#include "fs.h"
#include "mm.h"

volatile unsigned short* vidmem = (volatile unsigned short*) 0xB8000;
int cursor_row = 0;
int cursor_col = 0;

void vga_put(int row, int col, char c, char color) {
    vidmem[row * 80 + col] = (color << 8) | c;
}

void draw_hline(int row, int col, int width, char color) {
    int i;
    for(i = 0; i < width; i++)
        vga_put(row, col+i, '-', color);
}

void draw_vline(int col, int row, int height, char color) {
    int i;
    for(i = 0; i < height; i++)
        vga_put(row+i, col, '|', color);
}

void clear_screen() {
    int i;
    for(i = 0; i < 80*25; i++)
        vidmem[i] = (0x17 << 8) | ' ';
    cursor_row = 6;
    cursor_col = 2;
}

void draw_ui() {
    int i;

    /* Top bar - dark blue */
    for(i = 0; i < 80; i++)
        vga_put(0, i, ' ', 0x1F);

    /* Title */
    char* title = "  INDUS OS v0.6  |  Pakistan  |  Banaya: Azeem Zafar";
    for(i = 0; title[i]; i++)
        vga_put(0, i, title[i], 0x1F);

    /* Top right - version */
    char* ver = "[v0.6]";
    for(i = 0; ver[i]; i++)
        vga_put(0, 74+i, ver[i], 0x1E);

    /* Second bar */
    for(i = 0; i < 80; i++)
        vga_put(1, i, '-', 0x1B);

    /* Side borders */
    for(i = 2; i < 23; i++) {
        vga_put(i, 0, '|', 0x1B);
        vga_put(i, 79, '|', 0x1B);
    }

    /* Work area */
    for(i = 2; i < 23; i++) {
        int j;
        for(j = 1; j < 79; j++)
            vga_put(i, j, ' ', 0x17);
    }

    /* Bottom bar */
    for(i = 0; i < 80; i++)
        vga_put(23, i, '-', 0x1B);

    /* Status bar */
    for(i = 0; i < 80; i++)
        vga_put(24, i, ' ', 0x70);

    char* status = " IndusOS Ready | Type 'help' for commands | Pakistan 🇵🇰";
    for(i = 0; status[i] && i < 79; i++)
        vga_put(24, i, status[i], 0x70);

    /* Welcome message */
    char* w1 = "  ___   _____  ____  ____  __  __    ___  ____";
    char* w2 = " / _ |/_  __// __/ / __/ / / / /   / _ |/ __/";
    char* w3 = "/ __ |  / /  / _/  / _/  / /_/ /   / __ |\\__ \\";
    char* w4 = "/_/ |_| /_/  /___/ /___/  \\____/   /_/ |_|/___/";

    for(i = 0; w1[i]; i++) vga_put(2, i+16, w1[i], 0x1A);
    for(i = 0; w2[i]; i++) vga_put(3, i+16, w2[i], 0x1A);
    for(i = 0; w3[i]; i++) vga_put(4, i+16, w3[i], 0x1A);
    for(i = 0; w4[i]; i++) vga_put(5, i+16, w4[i], 0x1A);
}

void print_char(char c, char color) {
    if(c == '\n') {
        cursor_row++;
        cursor_col = 2;
        if(cursor_row >= 23) {
            /* Scroll up */
            int i, j;
            for(i = 6; i < 23; i++)
                for(j = 1; j < 79; j++)
                    vidmem[i*80+j] = vidmem[(i+1)*80+j];
            for(j = 1; j < 79; j++)
                vga_put(22, j, ' ', 0x17);
            cursor_row = 22;
        }
        return;
    }
    if(cursor_col >= 79) { cursor_col = 2; cursor_row++; }
    vga_put(cursor_row, cursor_col, c, color);
    cursor_col++;
}

void print_string(char* str, char color) {
    int i = 0;
    while(str[i] != 0) { print_char(str[i], color); i++; }
}

void print_at(int row, int col, char* str, char color) {
    int i = 0;
    while(str[i] != 0) {
        vga_put(row, col+i, str[i], color);
        i++;
    }
}

void newline() {
    cursor_row++;
    cursor_col = 2;
    if(cursor_row >= 23) {
        int i, j;
        for(i = 6; i < 23; i++)
            for(j = 1; j < 79; j++)
                vidmem[i*80+j] = vidmem[(i+1)*80+j];
        for(j = 1; j < 79; j++)
            vga_put(22, j, ' ', 0x17);
        cursor_row = 22;
    }
}

unsigned char port_read(unsigned short port) {
    unsigned char result;
    __asm__("in %%dx, %%al" : "=a" (result) : "d" (port));
    return result;
}

char keys[] = {
    0,0,'1','2','3','4','5','6','7','8','9','0','-','=',
    '\b','\t','q','w','e','r','t','y','u','i','o','p','[',']','\n',
    0,'a','s','d','f','g','h','j','k','l',';','\'','`',
    0,'\\','z','x','c','v','b','n','m',',','.','/',
    0,0,0,' '
};

char get_key() {
    unsigned char status = port_read(0x64);
    if(!(status & 0x01)) return 0;
    unsigned char keycode = port_read(0x60);
    if(keycode & 0x80) return 0;
    if(keycode < sizeof(keys)) return keys[keycode];
    return 0;
}

int str_starts(char* str, char* prefix) {
    int i = 0;
    while(prefix[i] != 0) {
        if(str[i] != prefix[i]) return 0;
        i++;
    }
    return 1;
}

int str_equal(char* a, char* b) {
    int i = 0;
    while(a[i] != 0 && b[i] != 0) {
        if(a[i] != b[i]) return 0;
        i++;
    }
    return a[i] == b[i];
}

char input_buffer[256];
int buf_pos = 0;

void show_prompt() {
    print_string("\n  > ", 0x1A);
}

void compare_and_run(char* cmd) {
    if(str_equal(cmd, "help")) {
        print_string("\n  Commands:\n", 0x1E);
        print_string("  help     - yeh message\n", 0x1F);
        print_string("  clear    - screen saaf karo\n", 0x1F);
        print_string("  name     - developer info\n", 0x1F);
        print_string("  version  - OS version\n", 0x1F);
        print_string("  list     - files dikhao\n", 0x1F);
        print_string("  create   - file banao\n", 0x1F);
        print_string("  write    - file mein likho\n", 0x1F);
        print_string("  read     - file padho\n", 0x1F);
        print_string("  delete   - file hatao\n", 0x1F);
        print_string("  meminfo  - RAM info\n", 0x1F);
        return;
    }
    if(str_equal(cmd, "clear")) {
        clear_screen();
        draw_ui();
        cursor_row = 6;
        cursor_col = 2;
        return;
    }
    if(str_equal(cmd, "name")) {
        print_string("\n  Developer : Azeem Zafar\n", 0x1B);
        print_string("  Country   : Pakistan\n", 0x1B);
        print_string("  Project   : IndusOS\n", 0x1B);
        return;
    }
    if(str_equal(cmd, "version")) {
        print_string("\n  IndusOS v0.6 - Better UI Edition\n", 0x1E);
        print_string("  Built from scratch in Assembly + C\n", 0x1F);
        return;
    }
    if(str_equal(cmd, "list")) {
        int i, found = 0;
        print_string("\n  Files:\n", 0x1E);
        for(i = 0; i < MAX_FILES; i++) {
            if(filesystem[i].used) {
                print_string("    ", 0x1F);
                print_string(filesystem[i].name, 0x1A);
                print_string("\n", 0x1F);
                found++;
            }
        }
        if(!found) print_string("  Koi file nahi!\n", 0x1C);
        return;
    }
    if(str_starts(cmd, "create ")) {
        char* fname = cmd + 7;
        int result = fs_create(fname);
        if(result >= 0) {
            print_string("\n  File bani: ", 0x1A);
            print_string(fname, 0x1F);
            print_string("\n", 0x1F);
        } else {
            print_string("\n  Error!\n", 0x1C);
        }
        return;
    }
    if(str_starts(cmd, "read ")) {
        char* fname = cmd + 5;
        char* data = fs_read(fname);
        if(data) {
            print_string("\n  ", 0x1F);
            print_string(data, 0x1F);
            print_string("\n", 0x1F);
        } else {
            print_string("\n  File nahi mili!\n", 0x1C);
        }
        return;
    }
    if(str_starts(cmd, "delete ")) {
        char* fname = cmd + 7;
        fs_delete(fname);
        print_string("\n  Deleted!\n", 0x1A);
        return;
    }
    if(str_starts(cmd, "write ")) {
        char fname[32];
        char* rest = cmd + 6;
        int i = 0;
        while(rest[i] != ' ' && rest[i] != 0 && i < 31) {
            fname[i] = rest[i]; i++;
        }
        fname[i] = 0;
        char* text = rest + i + 1;
        fs_write(fname, text);
        print_string("\n  Likha gaya!\n", 0x1A);
        return;
    }
    if(str_equal(cmd, "meminfo")) {
        char buf[12];
        print_string("\n  RAM Info:\n", 0x1E);
        print_string("  Used: ", 0x1F);
        int_to_str(mm_get_used(), buf);
        print_string(buf, 0x1A);
        print_string(" bytes\n", 0x1F);
        print_string("  Free: ", 0x1F);
        int_to_str(mm_get_free(), buf);
        print_string(buf, 0x1A);
        print_string(" bytes\n", 0x1F);
        return;
    }
    print_string("\n  Command nahi pehchana! 'help' type karo\n", 0x1C);
}

void process_input() {
    input_buffer[buf_pos] = 0;
    compare_and_run(input_buffer);
    buf_pos = 0;
    show_prompt();
}

void kernel_main() {
    fs_init();
    mm_init();
    clear_screen();
    draw_ui();
    cursor_row = 6;
    cursor_col = 2;
    print_string("  Khush Amdeed! IndusOS mein.\n", 0x1A);
    show_prompt();

    while(1) {
        char c = get_key();
        if(c == 0) continue;
        if(c == '\n') {
            process_input();
        } else if(c == '\b') {
            if(buf_pos > 0) {
                buf_pos--;
                cursor_col--;
                vga_put(cursor_row, cursor_col, ' ', 0x17);
            }
        } else {
            if(buf_pos < 255) {
                input_buffer[buf_pos++] = c;
                print_char(c, 0x1F);
            }
        }
        int i;
        for(i = 0; i < 100000; i++) {}
    }
}
