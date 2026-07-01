unsigned int tick = 0;
unsigned int seconds = 0;
unsigned int minutes = 0;
unsigned int hours = 0;

void port_write(unsigned short port, unsigned char data) {
    __asm__("out %%al, %%dx" : : "a" (data), "d" (port));
}

void update_clock() {
    volatile unsigned short* vga = (volatile unsigned short*) 0xB8000;
    char time_str[12];
    unsigned int h = hours;
    unsigned int m = minutes;
    unsigned int s = seconds;

    /* HH:MM:SS format */
    time_str[0] = (h / 10) + '0';
    time_str[1] = (h % 10) + '0';
    time_str[2] = ':';
    time_str[3] = (m / 10) + '0';
    time_str[4] = (m % 10) + '0';
    time_str[5] = ':';
    time_str[6] = (s / 10) + '0';
    time_str[7] = (s % 10) + '0';
    time_str[8] = 0;

    /* Top right corner pe print karo — column 72 */
    int i;
    for(i = 0; i < 8; i++) {
        vga[72 + i] = (0x1E << 8) | time_str[i];
    }
}

void timer_handler() {
    tick++;
    if(tick % 100 == 0) {
        seconds++;
        if(seconds >= 60) { seconds = 0; minutes++; }
        if(minutes >= 60) { minutes = 0; hours++; }
        if(hours >= 24)   { hours = 0; }
        update_clock();
    }
    port_write(0x20, 0x20);
}

void pic_install() {
    port_write(0x20, 0x11);
    port_write(0xA0, 0x11);
    port_write(0x21, 0x20);
    port_write(0xA1, 0x28);
    port_write(0x21, 0x04);
    port_write(0xA1, 0x02);
    port_write(0x21, 0x01);
    port_write(0xA1, 0x01);
    port_write(0x21, 0xFE);
    port_write(0xA1, 0xFF);
}

void timer_install() {
    pic_install();
    port_write(0x43, 0x36);
    port_write(0x40, 0x9C);
    port_write(0x40, 0x2E);
    idt_set_gate(32, (unsigned int) timer_handler, 0x08, 0x8E);
    update_clock();
}
