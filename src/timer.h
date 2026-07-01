unsigned int tick = 0;

void port_out(unsigned short port, unsigned char data) {
    __asm__ __volatile__("out %%al, %%dx" : : "a"(data), "d"(port));
}

void timer_handler() {
    tick++;
    if(tick % 100 == 0) {           // Har 1 second mein
        unsigned short* vga = (unsigned short*)0xB8000;
        char buf[10];
        int_to_str(tick / 100, buf);
        for(int i = 0; buf[i]; i++) {
            vga[70 + i] = (0x1E << 8) | buf[i];   // Top right mein time
        }
    }
    port_out(0x20, 0x20);  // EOI
}

void timer_install() {
    port_out(0x43, 0x36);
    port_out(0x40, 0x9C);
    port_out(0x40, 0x2E);

    idt_set_gate(32, (unsigned int)timer_handler, 0x08, 0x8E);
}
