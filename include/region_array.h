#ifndef _REGION_ARRAY_H
#define _REGION_ARRAY_H

#include "types.h"
#include "32os_conf.h"

typedef struct {
    uint64_t id;
    uint64_t status;
    
    uint64_t code_base;   
    uint64_t state_base;  
    uint64_t state_limit; 
    
    uint64_t mailbox_ptr;  
    uint64_t access_key;   
    
    uint8_t reserved[32];  
} __attribute__((packed)) region_t;

extern region_t region_array[128];

void region_init_all(void);
int  region_register(uint64_t id, uint64_t c_base, uint64_t s_base, uint64_t s_limit);

#endif