/* Manticore: vim, but more gooder (hopefully) */

/* Includes */

#include<sys/ioctl.h>
#include<ctype.h>
#include<stdio.h>
#include<unistd.h>
#include<termios.h>
#include<stdlib.h>

/* Data */

struct editor_conf {
    int scr_rows;
    int scr_cols;
    struct termios orig_termios;
};

struct editor_conf E;

#define VER "1.0"

/* Terminal handling */


int get_win_size(int *rows, int  *cols) {
    struct winsize ws;

    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0) {
        return -1;
    }

    *cols = ws.ws_col;
    *rows = ws.ws_row;
    return 0;
}

void row_draw() {
    for (int y=0; y<E.scr_rows; ++y) {
        if (y == E.scr_rows/2) {
            char welcome[64];
            int wlen = snprintf(welcome, sizeof(welcome), "MANTICORE @ VER %s", VER);
            if (wlen > E.scr_cols) {
                wlen = E.scr_rows;
            }
            int padding = (E.scr_cols - wlen) / 2;
            if (padding) {
                write(STDOUT_FILENO, "-", 1);
                padding--;
            }
            while (padding--) {
                write(STDOUT_FILENO, " ", 1);
            }
            write(STDOUT_FILENO, welcome, wlen);
        } else {
            write(STDOUT_FILENO, "~", 1);
        }
        if (y < E.scr_rows - 1) {
            write(STDOUT_FILENO, "\r\n", 2);
        }
    }
}

void init_editor() {
    if (get_win_size(&E.scr_rows, &E.scr_cols) == -1) {
        E.scr_rows = 24;
        E.scr_cols = 80;
    }
}

void cls() {
    init_editor();
    write(STDOUT_FILENO, "\x1b[2J", 4);
    row_draw();
    write(STDOUT_FILENO, "\x1b[H", 3);

}


void raw_off() {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &E.orig_termios);
}

void raw_on() {
    tcgetattr(STDIN_FILENO, &E.orig_termios);
    //atexit(raw_off);
    struct termios raw=E.orig_termios;

    tcgetattr(STDIN_FILENO, &raw);

    raw.c_lflag &= ~(ECHO | ICANON | ISIG);
    raw.c_cc[VMIN] = 1;
    raw.c_cc[VTIME] = 0;

    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

/* Main method */

int main() {
    char c='\0';
    raw_on();
    while (1) {
        cls();
        if (read(STDIN_FILENO, &c, 1) == 1) {
            if (c == 'q') {
                break;
            } else if (c == 'i' || c == 'a') {
                raw_off();
            } else if (c == 'n') {
                raw_on();
            }
        }

    }
    write(STDOUT_FILENO, "\x1b[2J", 4);
    return 0;
}
