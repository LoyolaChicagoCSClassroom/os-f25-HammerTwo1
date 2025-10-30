#include "paging.h"
#include <stdint.h>

uint32_t pd[1024] __attribute__((aligned(4096)));
static uint32_t pt_pool[32][1024] __attribute__((aligned(4096)));
static int next_pt = 0;

static void memclr32(uint32_t *p, uint32_t n) { for (uint32_t i=0;i<n;i++) p[i]=0; }

static uint32_t *get_or_create_pt(uint32_t pde_idx) {
    if (pd[pde_idx] & P_PRESENT) {
        return (uint32_t *)(pd[pde_idx] & 0xFFFFF000);
    }
    if (next_pt >= 32) { for(;;){} }  

    uint32_t *pt = &pt_pool[next_pt++][0];
    memclr32(pt, 1024);

    pd[pde_idx] = FRAME(pt) | P_PRESENT | P_RW;  
    return pt;
}

void *map_pages(void *vaddr, struct ppage *pglist, uint32_t *root_pd /* unused here */) {
    (void)root_pd;          
    uint8_t *va = (uint8_t*)vaddr;
    struct ppage *cur = pglist;

    while (cur) {
        uint32_t pde = PDE_INDEX(va);
        uint32_t pte = PTE_INDEX(va);
        uint32_t *pt = get_or_create_pt(pde);

        pt[pte] = FRAME((uint32_t)(uintptr_t)cur->physical_addr) | P_PRESENT | P_RW;
        va += PAGE_SIZE;
        cur = cur->next;
    }
    return vaddr;
}

void loadPageDirectory(uint32_t *root) {
    asm volatile("mov %0, %%cr3" :: "r"(root) : "memory");
}

void enablePaging(void) {
    asm volatile(
        "mov %%cr0, %%eax\n\t"
        "or  $0x80000001, %%eax\n\t"  
        "mov %%eax, %%cr0\n\t"
        :
        :
        : "eax","memory"
    );
}
