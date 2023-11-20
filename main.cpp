#include "utils.h"
#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <ncurses.h>
#include <unistd.h>
#include <string.h>
#include <iostream>
#include <vector>

typedef void (*PainterFn)(int, int, uint16_t);

void Init_colorpair()
{
    for (int i = 0; i < COLORS; i++) {
        init_pair(i, i, i);
    }
}

void PaintGray(int x, int y, uint16_t buf)
{
    mvaddch(y, x, (uint8_t)(buf & 0xff));
}

void PaintColor(int x, int y, uint16_t buf)
{
    uint8_t color = (buf & 0xff00) >> 8;
    attron(COLOR_PAIR(color));
    mvaddch(y, x, (uint8_t)(buf & 0xff));
    attroff(COLOR_PAIR(color));
}

int main(int argc, char **argv)
{
    if (argc != 3) {
        printf("Usage: %s [option] [path/to/video/file]\nOption:\n\t-I: Image\n\t-V: Video\n", argv[0]);
        return 0;
    }
    enum {
        TYPE_IMAGE,
        TYPE_VIDEO,
        TYPE_ALL
    } input_type;
    WINDOW *mainscr = NULL;
    bool loop = true;
    bool paused = false;
    int maxh, maxw;
    AsciiAnim anim;

    bool color_support = false;
    PainterFn painterfn = PaintGray;

    const uint16_t* buf = NULL;
    mainscr = initscr();
    if (has_colors()) {
        start_color();
        if (COLORS == 256) {
            Init_colorpair();
            color_support = true;
            painterfn = PaintColor;
        }
    }
    if (!mainscr) {
        printf("Error, exit.\n");
    }
    getmaxyx(mainscr, maxh, maxw);
    curs_set(0);
    noecho();
    keypad(stdscr, TRUE);
    anim.Init(maxw, maxh);

    printw("Loading file %s ...\n", argv[2]);
    refresh();
    if (!strncmp(argv[1], "-I", 2)) {
        ImageFile2Ascii(argv[2], maxw, maxh, anim, color_support);
        input_type = TYPE_IMAGE;
    } else {
        VideoFile2Ascii(argv[2], maxw, maxh, anim, color_support);
        input_type = TYPE_VIDEO;
    }
    printw("Press ANY to start.\n");
    refresh();
    fgetc(stdin);
    printw("Start playing...");
    refresh();

    while (loop) {
        buf = anim.LoopGetFrame();
        switch (input_type) {
            case TYPE_IMAGE:
                clear();
                break;
            case TYPE_VIDEO:
                if (!paused) {
                    timeout(33);
                } else {
                    timeout(-1);
                }
                break;
            default:
                break;
        }
        for (int y = 0; y < maxh; y++) {
            for (int x = 0; x < maxw; x++) {
                painterfn(x, y, buf[y * maxw + x]);
            }
        }
        refresh();
        switch (getch()){
            case 'q':
                loop = false;
                break;
            case ' ':
                paused = !paused;
                break;
        }
    }
    endwin();
    return 0;
}
