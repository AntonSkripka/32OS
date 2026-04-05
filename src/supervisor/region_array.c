#include "region_array.h"

__attribute__((section(".region_array")))
region_t region_array[MAX_REGIONS];

void region_init_all(void) {
    for (int i = 0; i < MAX_REGIONS; i++) {
        region_array[i].status = STATUS_EMPTY;
        region_array[i].id = i;
    }

    region_array[0].status     = STATUS_ACTIVE;
    region_array[0].code_base  = 0xFFFFFFFF80800000;
    region_array[0].state_base = 0xFFFFFFFF80E00000; 
    region_array[0].state_limit = 0xFFFFFFFF80200000;
    region_array[0].access_key  = 0xABCDE001;
}

int region_register(uint64_t id, uint64_t c_base, uint64_t s_base, uint64_t s_limit) {
    if (id >= MAX_REGIONS || region_array[id].status != STATUS_EMPTY) {
        return -1;
    }
    
    region_array[id].status      = STATUS_ACTIVE;
    region_array[id].code_base   = c_base;
    region_array[id].state_base  = s_base;
    region_array[id].state_limit = s_limit;
    
    return 0;
}