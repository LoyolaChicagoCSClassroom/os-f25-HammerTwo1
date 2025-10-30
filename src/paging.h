
#ifndef PAGING_H
#define PAGING_H
#include <stdint.h>
#include "page.h"
#define PAGE_SIZE      4096u
#define PAGE_SHIFT     12
#define PDE_INDEX(v)   ((((uint32_t)(v)) >> 22) & 0x3FF)
#define PTE_INDEX(v)   ((((uint32_t)(v)) >> 12) & 0x3FF)

/* Flags (both PDE/PTE) */
#define P_PRESENT      0x001
#define P_RW           0x002
#define P_USER         0x004
#define P_PWT          0x008
#define P_PCD          0x010
#define P_ACCESSED     0x020
#define P_DIRTY        0x040   
#define P_PAGESIZE     0x080   
#define P_GLOBAL       0x100   

/* PA -> frame field */
#define FRAME(addr)    ((uint32_t)(addr) & 0xFFFFF000)

void *map_pages(void *vaddr, struct ppage *pglist, uint32_t *pd);
void loadPageDirectory(uint32_t *pd);
void enablePaging(void);

#endif
