#include "paging.h"
#include "region_array.h"
#include "apic.h"

#define TABLES_PER_REGION 8

/* ===== Shared Kernel Structures ===== */
__attribute__((section(".paging"), aligned(4096)))
uint64_t kernel_pdpt[512];

__attribute__((section(".paging"), aligned(4096)))
uint64_t kernel_pd[512];

__attribute__((section(".paging"), aligned(4096)))
uint64_t kernel_devices_pt[512];

/* ===== Per-Region Structures ===== */
__attribute__((section(".paging"), aligned(4096)))
uint64_t region_pml4s[MAX_REGIONS][512];

__attribute__((section(".paging"), aligned(4096)))
uint64_t region_pdpts[MAX_REGIONS][512];

__attribute__((section(".paging"), aligned(4096)))
uint64_t region_pds[MAX_REGIONS][512];

__attribute__((section(".paging"), aligned(4096)))
uint64_t region_pts[MAX_REGIONS][TABLES_PER_REGION][512];

/* ===== Helper Functions ===== */
void paging_map_user_page(int reg_id, uint64_t virt, uint64_t phys, uint64_t flags)
{
    uint64_t pml4_idx = PML4_IDX(virt);
    uint64_t pdpt_idx = PDPT_IDX(virt);
    uint64_t pd_idx = PD_IDX(virt);
    uint64_t pt_idx = PT_IDX(virt);

    if (!(region_pml4s[reg_id][pml4_idx] & PAGE_PRESENT))
    {
        region_pml4s[reg_id][pml4_idx] = V2P(region_pdpts[reg_id]) |
                                         PAGE_PRESENT | PAGE_WRITE | PAGE_USER;
    }

    if (!(region_pdpts[reg_id][pdpt_idx] & PAGE_PRESENT))
    {
        region_pdpts[reg_id][pdpt_idx] = V2P(region_pds[reg_id]) |
                                         PAGE_PRESENT | PAGE_WRITE | PAGE_USER;
    }

    int pt_pool_idx = pd_idx % TABLES_PER_REGION;

    if (!(region_pds[reg_id][pd_idx] & PAGE_PRESENT))
    {
        region_pds[reg_id][pd_idx] = V2P(region_pts[reg_id][pt_pool_idx]) |
                                     PAGE_PRESENT | PAGE_WRITE | PAGE_USER;
    }

    region_pts[reg_id][pt_pool_idx][pt_idx] = phys | flags | PAGE_PRESENT | PAGE_USER;
}

void memset(void *dest, uint64_t val, size_t bytes)
{
    uint64_t *p = (uint64_t *)dest;
    size_t count = bytes / sizeof(uint64_t);

    while (count--)
    {
        *p++ = val;
    }
}

void paging_init_static()
{
    memset(kernel_pdpt, 0, 4096);
    memset(kernel_pd, 0, 4096);
    memset(kernel_devices_pt, 0, 4096);

    uint64_t kernel_pdpt_phys = V2P((uint64_t)kernel_pdpt);
    uint64_t kernel_pd_phys = V2P((uint64_t)kernel_pd);
    uint64_t kernel_devices_pt_phys = V2P((uint64_t)kernel_devices_pt);

    {
        kernel_pdpt[PDPT_IDX(0xFFFFFFFF80000000)] = kernel_pd_phys | PAGE_PRESENT | PAGE_WRITE;

        for (int pde_idx = 0; pde_idx < 8; pde_idx++)
        {
            uint64_t phys_addr = (uint64_t)pde_idx * 0x200000;
            kernel_pd[pde_idx] = phys_addr | PAGE_PRESENT | PAGE_WRITE | PAGE_HUGE;
        }

        kernel_pd[PD_IDX(APIC_VIRT_BASE)] = kernel_devices_pt_phys | PAGE_PRESENT | PAGE_WRITE | PAGE_PCD | PAGE_PWT;
        kernel_devices_pt[PT_IDX(KERNEL_OFFSET + 0xB8000)] = 0xB8000 | PAGE_PRESENT | PAGE_WRITE;
        kernel_devices_pt[PT_IDX(APIC_VIRT_BASE)] = APIC_DEFAULT_BASE | PAGE_PRESENT | PAGE_WRITE | PAGE_PCD | PAGE_PWT;
    }

    for (int r = 0; r < MAX_REGIONS; r++)
    {
        memset(region_pml4s[r], 0, 4096);
        memset(region_pdpts[r], 0, 4096);
        memset(region_pds[r], 0, 4096);
        memset(region_pts[r], 0, 4096 * TABLES_PER_REGION);

        region_pml4s[r][511] = kernel_pdpt_phys | PAGE_PRESENT | PAGE_WRITE;

        if (region_array[r].status == STATUS_ACTIVE)
        {
            if (r != 0)
            {
                uint64_t code_base = region_array[r].code_base;
                for (uint64_t off = 0; off < 0x200000; off += 4096)
                {
                    paging_map_user_page(r, code_base + off, code_base + off, PAGE_WRITE);
                }
            }
        }
    }
}