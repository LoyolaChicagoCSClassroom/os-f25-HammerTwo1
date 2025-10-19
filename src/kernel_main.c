
#include <stdint.h>
#include "scancodes.txt"
#include "page.h"
#include "rprintf.h"

#define MULTIBOOT2_HEADER_MAGIC         0xe85250d6

#define V_mem 0xB8000
#define width 80
#define hight 25

static volatile unsigned short *vram = (unsigned short *)V_mem;
static unsigned int cursor_pos = 0; 
static const unsigned char default_attr = 7; 

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


void main() {

    //putc('A');
    
    //esp_printf(putc, "\nCurrent execution: %d", inb);

    //for(int i = 0; i < 23; i++){
    //    esp_printf(putc,"Line %d\n", i+1);
    //}
    init_pfa_list();  // Initialize the page allocator
    esp_printf(vga_putc, "Page Frame Allocator Initialized!\n");
    print_pfa_state();

    // Allocate 3 pages
    struct ppage *allocated = allocate_physical_pages(3);
    esp_printf(vga_putc, "\nAllocated 3 pages:\n");
    struct ppage *cur = allocated;
    while (cur) {
        esp_printf(vga_putc, "  Allocated page addr: %x\n", cur->physical_addr);
        cur = cur->next;
    }

    // Print free list after allocation
    print_pfa_state();

    // Free those pages back
    free_physical_pages(allocated);
    esp_printf(vga_putc, "\nFreed 3 pages back to free list.\n");

    // Final free list check
    print_pfa_state();
    
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
