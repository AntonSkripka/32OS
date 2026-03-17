#include "paging.h"
#include "region_array.h"
#define TABLES_PER_REGION 8

__attribute__((section(".paging"), aligned(4096))) 
uint64_t region_pml4s[MAX_REGIONS][512];

__attribute__((section(".paging"), aligned(4096))) 
uint64_t region_pdpts[MAX_REGIONS][512];

__attribute__((section(".paging"), aligned(4096))) 
uint64_t region_pds[MAX_REGIONS][512];

__attribute__((section(".paging"), aligned(4096))) 
uint64_t region_pts[MAX_REGIONS][TABLES_PER_REGION][512];

void paging_map_region_page(int reg_id, uint64_t virt, uint64_t phys, uint64_t flags) {
    uint64_t pd_idx = PD_IDX(virt);
    uint64_t pt_idx = PT_IDX(virt);

    if (pd_idx >= TABLES_PER_REGION) {
        return; 
    }
    
    int pool_idx = pd_idx % TABLES_PER_REGION; 

    if (!(region_pds[reg_id][pd_idx] & PAGE_PRESENT)) {
        uint64_t table_addr = (uintptr_t)region_pts[reg_id][pool_idx];
        region_pds[reg_id][pd_idx] = (table_addr & 0x000FFFFFFFFFF000ULL) | PAGE_PRESENT | PAGE_WRITE | PAGE_USER;
    }

    region_pts[reg_id][pool_idx][pt_idx] = phys | flags | PAGE_PRESENT;
}

void memset(void *dest, uint64_t val, size_t bytes) {
    uint64_t *p = (uint64_t *)dest;
    size_t count = bytes / sizeof(uint64_t);

    while (count--) {
        *p++ = val;
    }
}

void paging_init_static() {
    for (int r = 0; r < MAX_REGIONS; r++) {
        memset(region_pml4s[r], 0, 4096);
        memset(region_pdpts[r], 0, 4096);
        memset(region_pds[r], 0, 4096);
        memset(region_pts[r], 0, 4096 * TABLES_PER_REGION);

        region_pml4s[r][0] = (uintptr_t)region_pdpts[r] | PAGE_PRESENT | PAGE_WRITE | PAGE_USER;
        region_pdpts[r][0] = (uintptr_t)region_pds[r]   | PAGE_PRESENT | PAGE_WRITE | PAGE_USER;

        for (uint64_t addr = 0; addr < 0x800000; addr += PAGE_SIZE_4K) {
            paging_map_region_page(r, addr, addr, PAGE_WRITE); 
        }

        if (region_array[r].status == STATUS_ACTIVE) {
            for (uint64_t off = 0; off < 0x200000; off += PAGE_SIZE_4K) {
                paging_map_region_page(r, region_array[r].code_base + off, 
                                       region_array[r].code_base + off, PAGE_USER);
            }
            
            for (uint64_t off = 0; off < region_array[r].state_limit; off += PAGE_SIZE_4K) {
                paging_map_region_page(r, region_array[r].state_base + off, 
                                       region_array[r].state_base + off, PAGE_WRITE | PAGE_USER | (1ULL << 63));
            }
        }
    }
}