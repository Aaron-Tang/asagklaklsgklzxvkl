#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include "traffic.h"

extern struct intersection isection;

/**
 * Populate the car lists by parsing a file where each line has
 * the following structure:
 *
 * <id> <in_direction> <out_direction>
 *
 * Each car is added to the list that corresponds with 
 * its in_direction
 * 
 * Note: this also updates 'inc' on each of the lanes
 */
void parse_schedule(char *file_name) {
    int id;
    struct car *cur_car;
    struct lane *cur_lane;
    enum direction in_dir, out_dir;
    FILE *f = fopen(file_name, "r");

    /* parse file */
    while (fscanf(f, "%d %d %d", &id, (int*)&in_dir, (int*)&out_dir) == 3) {

        /* construct car */
        cur_car = malloc(sizeof(struct car));
        cur_car->id = id;
        cur_car->in_dir = in_dir;
        cur_car->out_dir = out_dir;

        /* append new car to head of corresponding list */
        cur_lane = &isection.lanes[in_dir];
        cur_car->next = cur_lane->in_cars;
        cur_lane->in_cars = cur_car;
        cur_lane->inc++;
    }

    fclose(f);
}

/**
 * TODO: Fill in this function
 *
 * Do all of the work required to prepare the intersection
 * before any cars start coming
 * 
 */
void init_intersection() {
    int i;
    pthread_mutex_t quadrant_mutex_1 = PTHREAD_MUTEX_INITIALIZER;
    pthread_mutex_t quadrant_mutex_2 = PTHREAD_MUTEX_INITIALIZER;
    pthread_mutex_t quadrant_mutex_3 = PTHREAD_MUTEX_INITIALIZER;
    pthread_mutex_t quadrant_mutex_4 = PTHREAD_MUTEX_INITIALIZER;

    isection.quad[0] = quadrant_mutex_1;
    isection.quad[1] = quadrant_mutex_2;
    isection.quad[2] = quadrant_mutex_3;
    isection.quad[3] = quadrant_mutex_4;

    // OFFICE HOURS
    for (i = 0; i < 4; i++) {
        // initialize locks
        pthread_mutex_t lane_mutex = PTHREAD_MUTEX_INITIALIZER;
        pthread_cond_t prod_cv = PTHREAD_COND_INITIALIZER;
        pthread_cond_t cons_cv = PTHREAD_COND_INITIALIZER;

        // create a new lane
        struct lane *new_lane;
        new_lane = malloc(sizeof(struct lane));
        new_lane->lock = lane_mutex;
        new_lane->producer_cv = prod_cv;
        new_lane->consumer_cv = cons_cv;
        // new_lane->in_cars = NULL;
        // new_lane->out_cars = NULL;
        new_lane->inc = 0;
        new_lane->passed = 0;
        new_lane->head = 0;
        new_lane->tail = 0;
        new_lane->capacity = LANE_LENGTH;
        new_lane->in_buf = 0;
        // DOUBLE CHECK THIS AT OFFICE HOURS
        struct car *buffer = malloc(LANE_LENGTH * sizeof(struct car));
        new_lane->buffer = &buffer;

        // add new lane to lanes array
        isection.lanes[i] = *new_lane;
    }
}

/**
 * TODO: Fill in this function
 *
 * Populates the corresponding lane with cars as room becomes
 * available. Ensure to notify the cross thread as new cars are
 * added to the lane.
 * 
 */
void *car_arrive(void *arg) {
    struct lane *l = arg;
    pthread_mutex_lock(&l->lock);
    int i;

    for (i = 0; i < l->inc; i++){
        printf("Iteration: %d\n", i);
        while(l->in_buf == l->capacity) {
            pthread_cond_wait(&l->producer_cv, &l->lock);
        }
        l->in_cars[i].next = NULL;
        printf("Tail %d, Car: %d\n", l->tail, l->in_cars[i].id);
        l->buffer[l->tail] = &l->in_cars[i];
        if (l->tail == l->capacity - 1)
            l->tail = 0;
        l->tail += 1;
        l->in_buf += 1;
        pthread_cond_signal(&l->consumer_cv);
    }

    // might be broadcast
    pthread_mutex_unlock(&l->lock);
    return NULL;
}

/**
 * TODO: Fill in this function
 *
 * Moves cars from a single lane across the intersection. Cars
 * crossing the intersection must abide the rules of the road
 * and cross along the correct path. Ensure to notify the
 * arrival thread as room becomes available in the lane.
 *
 * Note: After crossing the intersection the car should be added
 * to the out_cars list of the lane that corresponds to the car's
 * out_dir. Do not free the cars!
 *
 * 
 * Note: For testing purposes, each car which gets to cross the 
 * intersection should print the following three numbers on a 
 * new line, separated by spaces:
 *  - the car's 'in' direction, 'out' direction, and id.
 * 
 * You may add other print statements, but in the end, please 
 * make sure to clear any prints other than the one specified above, 
 * before submitting your final code. 
 */
void *car_cross(void *arg) {
    struct lane *l = arg;
    // pthread_mutex_lock(&l->lock);

    // while(l->in_buf == 0) {
    //     pthread_cond_wait(&l->consumer_cv, &l->lock);
    // }

    // // need to update new head
    // printf("HEAD: %d, Car: %d\n", l->head, l->buffer[l->head]->id);
    // struct car *cur_car = l->buffer[l->head];
 
    // if (l->head == l->capacity - 1)
    //     l->head = 0;
    // l->head += 1;


    // // Decrements in_buf because cur_car has left buffer
    // l->in_buf -= 1;

    // int *path = compute_path(cur_car->in_dir, cur_car->out_dir);
    // int i;
    // for (i = 0; i < (sizeof(path)/sizeof(int)); i++) {
    //     pthread_mutex_lock(&isection.quad[path[i]]);
    // }

    // printf("ID: %d || out_dir: %d || in_dir: %d\n", cur_car->id, cur_car->out_dir, cur_car->in_dir);

    // // adds cur_car to out_cars in exit lane
    // struct lane *exit_lane;

    // exit_lane = &isection.lanes[cur_car->out_dir];
    // cur_car->next = exit_lane->out_cars;
    // exit_lane->out_cars = cur_car;
    // exit_lane->passed++;

    // for (i = 0; i < (sizeof(path)/sizeof(int)); i++) {
    //     pthread_mutex_unlock(&isection.quad[path[i]]);
    // }
    // printf("Before Signal\n");
    // pthread_cond_signal(&l->producer_cv);
    // printf("After Signal\n");
    // pthread_mutex_unlock(&l->lock);

    // free(path);
    return NULL;
}

/**
 * TODO: Fill in this function
 *
 * Given a car's in_dir and out_dir return a sorted 
 * list of the quadrants the car will pass through.
 * 
 */
int *compute_path(enum direction in_dir, enum direction out_dir) {
    int *path = malloc(4 * sizeof(int));

    // OFFICE HOURS QUESTIONS: HOW TO U-TURN
    switch (in_dir) {
        case NORTH:
            switch (out_dir) {
                case NORTH:
                    path[0] = 0;
                    path[1] = 1;
                    path[2] = 2;
                    path[3] = 3;
                case EAST:
                    path[0] = 1;
                    path[1] = 2;
                    path[2] = 3;
                case SOUTH:
                    path[0] = 1;
                    path[1] = 2;
                case WEST:
                    path[0] = 1;
                default:
                    printf("NORTH\n");
            }
        case EAST:
            switch (out_dir) {
                case NORTH:
                    path[0] = 0;
                case EAST:
                    path[0] = 0;
                    path[1] = 1;
                    path[2] = 2;
                    path[3] = 3;
                case SOUTH:
                    path[0] = 0;
                    path[1] = 1;
                    path[2] = 2;
                case WEST:
                    path[0] = 0;
                    path[1] = 1;
                default:
                    printf("EAST\n");
            }
        case SOUTH:
            switch (out_dir) {
                case NORTH:
                    path[0] = 0;
                    path[1] = 3;
                case EAST:
                    path[0] = 3;
                case SOUTH:
                    path[0] = 0;
                    path[1] = 1;
                    path[2] = 2;
                    path[3] = 3;
                case WEST:
                    path[0] = 0;
                    path[1] = 1;
                    path[2] = 3;
                default:
                    printf("SOUTH\n");
            }
        case WEST:
            switch (out_dir) {
                case NORTH:
                    path[0] = 0;
                    path[1] = 2;
                    path[2] = 3;
                case EAST:
                    path[0] = 2;
                    path[1] = 3;
                case SOUTH:
                    path[0] = 2;
                case WEST:
                    path[0] = 0;
                    path[1] = 1;
                    path[2] = 2;
                    path[3] = 3;
                default:
                    printf("WEST\n");
            }
        default:
            printf("you done fucked up\n");
    }

    return path;
}