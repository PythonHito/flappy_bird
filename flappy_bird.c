#include <math.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include "ncurses.h"

#define JUMP_ACCEL 6f
#define G 9.81f

#define NUM_PIPES 5
#define PIPE_SPEED 500.0f
#define PIPE_CYCLE_MAX 200.0f
#define PIPE_GIRTH 30.0f
#define PIPE_HOLE_WIDTH 50.0f

#define FLAPPY_X 100

#define FPS 30

typedef struct flappy_bird {
    float height;
    float height_accel;
} flappy_bird_t;

#define BIRD_UP '^'
#define BIRD_STRAIGHT '>'
#define BIRD_DOWN 'v'

void iter_sim(flappy_bird_t *flappy, float *pipe_cycle, int *current_pipe_hole){
    flappy->height += flappy->height_accel;
    flappy->height_accel -= G/FPS;

    *pipe_cycle -= PIPE_SPEED/FPS;
    if (*pipe_cycle < 0) *current_pipe_hole = *current_pipe_hole + 1 % NUM_PIPES;
}

void process_frame(flappy_bird_t *flappy_bird) {
    move(FLAPPY_X, flappy_bird->height);

    if (flappy_bird->height_accel < -2) addch(BIRD_DOWN);
    else if (flappy_bird->height_accel > 2) addch(BIRD_UP);
    else addch(BIRD_STRAIGHT);
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
    //timeout(33); // SCREW, MAGIC BE HERE
    noecho();

    flappy_bird_t flappy_bird;
    flappy_bird.height = 500;
    flappy_bird.height_accel = 0;

    float pipe_cycle = PIPE_CYCLE_MAX;

    // Init circular buffer for pipe
    float pipe_hole_buffer[NUM_PIPES];
    srand(time(NULL));
    for (int i = 0; i < NUM_PIPES; i++) pipe_hole_buffer[i] = 1000*rand();

    int current_pipe_hole = 0;

    while (true){
        iter_sim(&flappy_bird, &pipe_cycle, &current_pipe_hole);
        process_frame(&flappy_bird);

        if (flappy_bird.height < 0) return 0; // LOSER

        _Bool in_pipe_x = (abs(FLAPPY_X-pipe_cycle) < PIPE_GIRTH);
        _Bool in_pipe_y = (abs(pipe_hole_buffer[current_pipe_hole]-flappy_bird.height) < PIPE_HOLE_WIDTH);

        //if (in_pipe_x && in_pipe_y) return 0; // LOSER

        // OK FINE, YOU HAVEN'T LOST YET

        msleep(33);

        // THERE IS NO WINNING, ONLY DELAYED DEATH
    }
}
