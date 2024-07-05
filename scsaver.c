#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sys/ioctl.h>
#include <signal.h>

#define ALIVE "█"
#define DEAD " "

int WIDTH, HEIGHT;
int ***grid;
int ***new_grid;
int running = 1;

void clear_screen() {
    printf("\033[2J");
    printf("\033[H");
}

void get_terminal_size() {
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    WIDTH = w.ws_col;
    HEIGHT = w.ws_row - 1;//最下行にプロンプト表示
}

void init_grid() {
    grid = (int ***)malloc(HEIGHT * sizeof(int **));
    new_grid = (int ***)malloc(HEIGHT * sizeof(int **));
    for (int y = 0; y < HEIGHT; y++) {
        grid[y] = (int **)malloc(WIDTH * sizeof(int *));
        new_grid[y] = (int **)malloc(WIDTH * sizeof(int *));
        for (int x = 0; x < WIDTH; x++) {
            grid[y][x] = (int *)malloc(3 * sizeof(int));
            new_grid[y][x] = (int *)malloc(3 * sizeof(int));
            for (int c = 0; c < 3; c++) {
                grid[y][x][c] = rand() % 2;
            }
        }
    }
}

int count_neighbors(int x, int y, int color) {
    int count = 0;
    for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
            if (dx == 0 && dy == 0) continue;
            int nx = (x + dx + WIDTH) % WIDTH;
            int ny = (y + dy + HEIGHT) % HEIGHT;
            count += grid[ny][nx][color];
        }
    }
    return count;
}

void update_grid() {
    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            for (int c = 0; c < 3; c++) {
                int neighbors = count_neighbors(x, y, c);
                if (grid[y][x][c]) {
                    new_grid[y][x][c] = (neighbors == 2 || neighbors == 3);
                } else {
                    new_grid[y][x][c] = (neighbors == 3);
                }
            }
        }
    }
    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            for (int c = 0; c < 3; c++) {
                grid[y][x][c] = new_grid[y][x][c];
            }
        }
    }
}

void draw_grid() {
    clear_screen();
    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            int r = grid[y][x][0], g = grid[y][x][1], b = grid[y][x][2];
            if (r || g || b) {
                printf("\033[38;2;%d;%d;%dm%s\033[0m", r * 255, g * 255, b * 255, ALIVE);
            } else {
                printf("%s", DEAD);
            }
        }
        if (y < HEIGHT - 1) printf("\n");
    }
    fflush(stdout);
}

void free_grid() {
    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            free(grid[y][x]);
            free(new_grid[y][x]);
        }
        free(grid[y]);
        free(new_grid[y]);
    }
    free(grid);
    free(new_grid);
}

void signal_handler(int signum) {
    if (signum == SIGINT) {
        running = 0;
    }
}

int main() {
    signal(SIGINT, signal_handler);
    
    srand(time(NULL));
    get_terminal_size();
    init_grid();

    while (running) {
        draw_grid();
        update_grid();
        usleep(100000);
    }

    free_grid();
    printf("\033[0m\nプログラムを終了します。\n");
    return 0;
}