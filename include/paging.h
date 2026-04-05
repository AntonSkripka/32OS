#ifndef PAGING_H
#define PAGING_H

#include "types.h"

#define PAGE_PRESENT    (1ULL << 0)
#define PAGE_WRITE      (1ULL << 1)
#define PAGE_USER       (1ULL << 2)
#define PAGE_PWT        (1ULL << 3)
#define PAGE_PCD        (1ULL << 4)
#define PAGE_HUGE       (1ULL << 7)
#define PAGE_SIZE_4K    4096

#define KERNEL_OFFSET 0xFFFFFFFF80000000ULL

#define V2P(addr) ((uint64_t)(addr) - KERNEL_OFFSET)

#define PML4_IDX(v) (((v) >> 39) & 0x1FF)
#define PDPT_IDX(v) (((v) >> 30) & 0x1FF)
#define PD_IDX(v)   (((v) >> 21) & 0x1FF)
#define PT_IDX(v)   (((v) >> 12) & 0x1FF)

typedef uint64_t pt_entry_t;

void paging_init_static(void);
void paging_map_user_page(int reg_id, uint64_t virt, uint64_t phys, uint64_t flags);
void paging_map_region_page(int reg_id, uint64_t virt, uint64_t phys, uint64_t flags);
extern void paging_flush_load(uint64_t pml4_phys);

#endif