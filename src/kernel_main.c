
#include <stdint.h>
#include "scancodes.txt"
#include "page.h"
#include "rprintf.h"
#include "paging.h"

#define MULTIBOOT2_HEADER_MAGIC         0xe85250d6

#define V_mem 0xB8000
#define width 80
#define hight 25

static volatile unsigned short *vram = (unsigned short *)V_mem;
static unsigned int cursor_pos = 0; 
static const unsigned char default_attr = 7; 



extern int vga_putc(int c);
extern uint8_t _end_kernel;

extern uint32_t pd[1024];  // from paging.c

const unsigned int multiboot_header[]  __attribute__((section(".multiboot"))) = {MULTIBOOT2_HEADER_MAGIC, 0, 16, -(16+MULTIBOOT2_HEADER_MAGIC), 0, 12};

uint8_t inb (uint16_t _port) {
    uint8_t rv;
    __asm__ __volatile__ ("inb %1, %0" : "=a" (rv) : "dN" (_port));
    return rv;
}

void outb (uint16_t _port, uint8_t val) {
__asm__ __volatile__ ("outb %0, %1" : : "a" (val), "dN" (_port) );
}

int vga_putc(int c) {
    putc(c);
    return c;
}


void putc(int data) {
    unsigned char ch = (unsigned char)(data & 0xFF);


    if (ch == '\n') {
        cursor_pos = (cursor_pos / width + 1) * width;
    } else {
        vram[cursor_pos] = (unsigned short)(ch | (default_attr << 8));
        cursor_pos++; 
    }
      

    if (cursor_pos >= width * hight) {
        for (unsigned int row = 1; row < hight; row++) {
            for (unsigned int col = 0; col < width; col++) {
                vram[(row - 1) * width + col] = vram[row * width + col];
            }
        }

        unsigned short blank = ' ' | (default_attr << 8);
        for (unsigned int col = 0; col < width; col++) {
            vram[(hight - 1) * width + col] = blank;
        }

        cursor_pos = (hight - 1) * width;
    }
        
}

void puts(const char *str){
    while(*str){
        putc(*str++);
    }
}

char nibble_to_hex(uint8_t nibble) {
    return (nibble < 10) ? ('0' + nibble) : ('A' + (nibble - 10));
}

static void identity_map_range(uint32_t start, uint32_t end) {
    for (uint32_t addr = start & ~0xFFFu; addr < (end + 0xFFFu) & ~0xFFFu; addr += 4096) {
        struct ppage tmp = {0};
        tmp.physical_addr = (void*)(uintptr_t)addr;
        map_pages((void*)(uintptr_t)addr, &tmp, pd);
    }
}



void main() {

     init_pfa_list();

    for (int i=0;i<1024;i++) pd[i] = 0;

    identity_map_range(0x00100000u, (uint32_t)(uintptr_t)&_end_kernel);

    uint32_t esp;
    asm volatile("mov %%esp, %0" : "=r"(esp));
    identity_map_range((esp - 0x8000) & ~0xFFFu, (esp + 0x1000 + 0xFFFu) & ~0xFFFu);

    identity_map_range(0x000B8000u, 0x000B9000u);

    loadPageDirectory(pd);
    enablePaging();

    esp_printf(vga_putc, "Paging enabled. PD=%x ESP=%x END=%x\n", pd, esp, &_end_kernel);

    /* putc('A');
    
    esp_printf(putc, "\nCurrent execution: %d", inb);

    for(int i = 0; i < 23; i++){
        esp_printf(putc,"Line %d\n", i+1);
    }
    init_pfa_list();  
    esp_printf(vga_putc, "Page Frame Allocator Initialized!\n");
    print_pfa_state();

     Allocate 3 pages
    struct ppage *allocated = allocate_physical_pages(3);
    esp_printf(vga_putc, "\nAllocated 3 pages:\n");
    struct ppage *cur = allocated;
    while (cur) {
        esp_printf(vga_putc, "  Allocated page addr: %x\n", cur->physical_addr);
        cur = cur->next;
    }

     Print free list after allocation
    print_pfa_state();

    
    free_physical_pages(allocated);
    esp_printf(vga_putc, "\nFreed 3 pages back to free list.\n")
     Final free list check
    print_pfa_state();  */
    
    while (1) {
        uint8_t status = inb(0x64);

        if (status & 1) {
            uint8_t scancode = inb(0x60);

            if(scancode >= 0x80){
                continue;
            }
            
            putc('[');
            putc(nibble_to_hex((scancode >> 4) & 0xF));
            putc(nibble_to_hex(scancode & 0xF));
            putc(']');
            putc(' ');
                    
            if (keyboard_map[scancode]) {
                putc(keyboard_map[scancode]);
            }
            putc('\n');
        }
    }
}
