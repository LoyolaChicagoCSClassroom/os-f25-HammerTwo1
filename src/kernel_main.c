
#include <stdint.h>

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

void putc(int data) {
    unsigned char ch = (unsigned char)(data & 0xFF);

    /* if (ch == '\r') {
        cursor_pos = (cursor_pos / width) * width;
        return;
    }

    if (ch == '\n') {
        cursor_pos = (cursor_pos / width + 1) * width;
    } else {
        
    }
        */
    vram[cursor_pos] = (unsigned short)(ch | (default_attr << 8));
        cursor_pos++;   

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

void main() {

    putc('A');
    
    esp_printf(putc, "Current execution level: %d", inb);

    for(int i = 0; i < 800; i++){
        esp_printf(putc,"Line %d", i+1);
    }

    while(1) {
        uint8_t status = inb(0x64);

        if(status & 1) {
            uint8_t scancode = inb(0x60);
        }
    }
}
