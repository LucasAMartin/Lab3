#ifndef LAB3_H
#define LAB3_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <time.h>

#define DEFAULT_ELEMENTS 5

// Data structure to hold shared data
typedef struct
{
    int N;         // Size of the matrix/array
    int *array;    // Array to hold data
    bool stop;     // Used to stop the simulations
    int *thread_ids; // Thread IDs
} Data;

// Structure to pass thread arguments
typedef struct
{
    Data *data;   // Pointer to shared data
    int thread_id; // Thread ID
} ThreadArgs;

// Input functions
void validate_argument(int argc, char *argv[], void *arg);
int validate_input();
int display_menu();

// Thread management functions
ThreadArgs *create_threads(pthread_t *threads, Data *data, int M, void *(*start_routine)(void *));
void join_threads(pthread_t *threads, int M);

// Problem-specific functions for problem 1
void fill_matrix(void *arg);
void print_matrix(void *arg);
void *check_stop_criteria(void *arg);
void *flipNumbers(void *arg);

// Problem-specific functions for problem 2
void fill_array(void *arg);
void print_array(void *arg);
void *check_sorted(void *arg);
void *insertionSort(void *arg);

// Problem simulation functions
void problem1(int M, pthread_t *threads, pthread_t stop_thread, void *arg);
void problem2(int M, pthread_t *threads, pthread_t stop_thread, void *arg);

#endif
