#include "fs.h"
#include "mm.h"

volatile unsigned short* vidmem = (volatile unsigned short*) 0xB8000;
int cursor_row = 0;
int cursor_col = 0;

void clear_screen() {
    int i;
    for(i = 0; i < 80*25; i++)
        vidmem[i] = (0x1F << 8) | ' ';
    cursor_row = 0;
    cursor_col = 0;
}

void print_char(char c, char color) {
    if(c == '\n') { cursor_row++; cursor_col = 0; return; }
    int offset = cursor_row * 80 + cursor_col;
    vidmem[offset] = (color << 8) | c;
    cursor_col++;
    if(cursor_col >= 80) { cursor_col = 0; cursor_row++; }
}

void print_string(char* str, char color) {
    int i = 0;
    while(str[i] != 0) { print_char(str[i], color); i++; }
}

void print_at(char* str, int col, int row, char color) {
    cursor_col = col; cursor_row = row;
    print_string(str, color);
}

void newline() { cursor_row++; cursor_col = 0; }

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

void compare_and_run(char* cmd) {
    if(str_equal(cmd, "help")) {
        print_string("Commands:\n", 0x1E);
        print_string("  help, clear, name, version\n", 0x1F);
        print_string("  list, create, read, write, delete\n", 0x1F);
        print_string("  meminfo, alloc\n", 0x1F);
        return;
    }
    if(str_equal(cmd, "clear")) {
        clear_screen();
        print_at("AzeemOS v0.5", 0, 0, 0x1E);
        cursor_row = 2; cursor_col = 0;
        return;
    }
    if(str_equal(cmd, "name")) {
        print_string("Banaya: Azeem Zafar\n", 0x1B);
        print_string("Pakistan ka pehla OS developer!\n", 0x1A);
        return;
    }
    if(str_equal(cmd, "version")) {
        print_string("AzeemOS v0.5\n", 0x1E);
        print_string("Built from scratch in Assembly + C\n", 0x1F);
        return;
    }
    if(str_equal(cmd, "list")) {
        int i, found = 0;
        print_string("Files:\n", 0x1E);
        for(i = 0; i < MAX_FILES; i++) {
            if(filesystem[i].used) {
                print_string("  ", 0x1F);
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
        if(result >= 0) { print_string("File bani!\n", 0x1A); }
        else { print_string("Error!\n", 0x1C); }
        return;
    }
    if(str_starts(cmd, "read ")) {
        char* fname = cmd + 5;
        char* data = fs_read(fname);
        if(data) { print_string(data, 0x1F); print_string("\n", 0x1F); }
        else { print_string("File nahi mili!\n", 0x1C); }
        return;
    }
    if(str_starts(cmd, "delete ")) {
        char* fname = cmd + 7;
        fs_delete(fname);
        print_string("Deleted!\n", 0x1A);
        return;
    }
    if(str_starts(cmd, "write ")) {
        char fname[32];
        char* rest = cmd + 6;
        int i = 0;
        while(rest[i] != ' ' && rest[i] != 0 && i < 31) { fname[i] = rest[i]; i++; }
        fname[i] = 0;
        char* text = rest + i + 1;
        fs_write(fname, text);
        print_string("Likha gaya!\n", 0x1A);
        return;
    }
    if(str_equal(cmd, "meminfo")) {
        char buf[12];
        print_string("Used: ", 0x1F);
        int_to_str(mm_get_used(), buf);
        print_string(buf, 0x1A);
        print_string(" bytes\n", 0x1F);
        print_string("Free: ", 0x1F);
        int_to_str(mm_get_free(), buf);
        print_string(buf, 0x1A);
        print_string(" bytes\n", 0x1F);
        return;
    }
    if(str_starts(cmd, "alloc ")) {
        char* sizestr = cmd + 6;
        unsigned int size = 0;
        int i = 0;
        while(sizestr[i] != 0) { size = size*10 + (sizestr[i]-'0'); i++; }
        void* ptr = kmalloc(size);
        if(ptr) print_string("Allocated!\n", 0x1A);
        else print_string("Error!\n", 0x1C);
        return;
    }
    print_string("Command nahi pehchana!\n", 0x1C);
}

void process_input() {
    input_buffer[buf_pos] = 0;
    newline();
    compare_and_run(input_buffer);
    buf_pos = 0;
    print_string("\n> ", 0x1A);
}

void kernel_main() {
    fs_init();
    mm_init();
    clear_screen();
    print_at("===================================================", 0, 0, 0x1E);
    print_at("         AZEEM OS v0.5  |  Pakistan               ", 0, 1, 0x1F);
    print_at("         Banaya: Azeem Zafar                       ", 0, 2, 0x1B);
    print_at("===================================================", 0, 3, 0x1E);
    cursor_row = 5;
    cursor_col = 0;
    print_string("Type 'help' for commands\n", 0x1F);
    print_string("\n> ", 0x1A);

    while(1) {
        char c = get_key();
        if(c == 0) continue;
        if(c == '\n') {
            process_input();
        } else if(c == '\b') {
            if(buf_pos > 0) {
                buf_pos--;
                cursor_col--;
                int offset = cursor_row * 80 + cursor_col;
                vidmem[offset] = (0x1F << 8) | ' ';
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
