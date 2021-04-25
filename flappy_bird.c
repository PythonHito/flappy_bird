#include <math.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include "ncurses.h"

#define TERM_HEIGHT 50
#define TERM_WIDTH 100

#define JUMP_ACCEL 25.0f
#define G 30.81f

#define NUM_PIPES 5
#define PIPE_SPEED 75.0f
#define PIPE_CYCLE_MAX 100.0f
#define PIPE_GIRTH 10
#define PIPE_HOLE_WIDTH 8
#define PIPE_GRAPHIC '|'

#define FPS 30

#define BIRD_UP '^'
#define BIRD_STRAIGHT '>'
#define BIRD_DOWN 'v'

#define FLAPPY_X 20

typedef struct flappy_bird {
    float height;
    float height_accel;
} flappy_bird_t;

void iter_sim(flappy_bird_t *flappy, float *pipe_cycle, int *current_pipe_hole){
    flappy->height += flappy->height_accel/FPS;
    flappy->height_accel -= G/FPS;

    *pipe_cycle -= PIPE_SPEED/FPS;
}

void draw_pipe(int column_x, int hole_y) {
    for (int i = column_x-(PIPE_GIRTH/2); i < column_x+(PIPE_GIRTH/2); i++)
    for (int j = 0; j <= TERM_HEIGHT; j++)
        if ((j > hole_y+(PIPE_HOLE_WIDTH/2)) || (j < hole_y-(PIPE_HOLE_WIDTH/2)))
            mvaddch(TERM_HEIGHT-j, i, PIPE_GRAPHIC);
}

void process_frame(flappy_bird_t *flappy_bird, float pipe_cycle, float *pipe_hole_buffer, int current_pipe_hole) {
    clear();
    
    move(TERM_HEIGHT-(int)flappy_bird->height, FLAPPY_X);
    if (flappy_bird->height_accel < -4) addch(BIRD_DOWN);
    else if (flappy_bird->height_accel > 4) addch(BIRD_UP);
    else addch(BIRD_STRAIGHT);

    for (int i = 0; i < NUM_PIPES; i++){
        draw_pipe(pipe_cycle + PIPE_CYCLE_MAX*i, pipe_hole_buffer[(i+current_pipe_hole)%NUM_PIPES]);
    }

    refresh();
}

// STACK OVERFLOW: https://stackoverflow.com/questions/1157209/is-there-an-alternative-sleep-function-in-c-to-milliseconds
/* msleep(): Sleep for the requested number of milliseconds. */
int msleep(long msec)
{
    struct timespec ts;
    int res;

    if (msec < 0)
    {
        errno = EINVAL;
        return -1;
    }

    ts.tv_sec = msec / 1000;
    ts.tv_nsec = (msec % 1000) * 1000000;

    do {
        res = nanosleep(&ts, &ts);
    } while (res && errno == EINTR);

    return res;
}

int main(){

    // GRAPHICS INIT
    initscr();
    raw();
    timeout(33); // SCREW, MAGIC BE HERE
    noecho();

    flappy_bird_t flappy_bird;
    flappy_bird.height = TERM_HEIGHT/2;
    flappy_bird.height_accel = 0;

    float pipe_cycle = PIPE_CYCLE_MAX;

    // Init circular buffer for pipe
    float pipe_hole_buffer[NUM_PIPES];
    srand(time(NULL));
    for (int i = 0; i < NUM_PIPES; i++) pipe_hole_buffer[i] = rand()%TERM_HEIGHT;

    int current_pipe_hole = 0;

    int score = 0;

    while (true){
        
        int ch = getch();
        if (ch == 32) // SPACE
            flappy_bird.height_accel = JUMP_ACCEL;

        iter_sim(&flappy_bird, &pipe_cycle, &current_pipe_hole);
        if (pipe_cycle < 0){
            score++;
            pipe_cycle = PIPE_CYCLE_MAX;
            current_pipe_hole = current_pipe_hole + 1 % NUM_PIPES;
        }
        process_frame(&flappy_bird, pipe_cycle, pipe_hole_buffer, current_pipe_hole);

        if (flappy_bird.height < 0.0f) break; // LOSER

        _Bool in_pipe_x = (abs(FLAPPY_X-pipe_cycle) <= PIPE_GIRTH);
        _Bool in_pipe_y = (abs(pipe_hole_buffer[current_pipe_hole]-flappy_bird.height) > PIPE_HOLE_WIDTH);

        if (in_pipe_x && in_pipe_y) break; // LOSER

        // OK FINE, YOU HAVEN'T LOST YET

        msleep(33);

        // THERE IS NO WINNING, ONLY DELAYED DEATH
    }

    endwin();
    puts("YOU SUCK\n");
    printf("Score: %d", score);
}
