#include <cstdio>
#include <cstdlib>
#include <ncurses.h>
#include <unistd.h>

int main(int argc, char **argv)
{
    printf("Hello world\n");
    WINDOW *mainscr = initscr();
    curs_set(0);
    noecho();
    keypad(stdscr, TRUE);
    mousemask(BUTTON1_RELEASED,0);
    MEVENT event;
    int i, j;
    uint64_t counter = 0;
    bool loop = true;
    while (loop) {
        clear();
        counter++;
        for(i = 0; i < 10; i++) {
            for(j = 0; j < 20; j++) {
                if (j/2 == i) {
                    mvaddch(i, j, '-');
                } else {
                    mvaddch(i, j, ' ');
                }
                //mvaddch(i, j, '0');
            }
        }
        printw("\n%lu", counter);
        refresh();
        switch (getch()){
            case 'q':
                loop = false;
                break;
        }
        usleep(1000);
    }
    endwin();
    return 0;
}
