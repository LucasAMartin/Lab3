#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>

#define DEFAULT_ELEMENTS 8

int N; // Size of the matrix/array
int *array;
bool stop = false; // Used to stop the simulations
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

// Functions shared for both problems
void validate_input(int argc, char *argv[]) {
    N = DEFAULT_ELEMENTS; // Default size
    if (argc > 1)
    {
        char *ptr;
        long input = strtol(argv[1], &ptr, 10);
        if (*ptr == '\0')
        {
            N = (int)input;
        }
        else
        {
            printf("Using Default Size of %d\n", N);
        }
    }
    else
    {
        printf("Using Default Size of %d\n", N);
    }
}

int display_menu() {
    int choice;
    printf("Menu:\n");
    printf("1. Problem 1\n");
    printf("2. Problem 2\n");
    printf("3. Exit\n");
    printf("Enter your choice: ");
    scanf("%d", &choice);
    return choice;
}

void create_threads(pthread_t *threads, int *thread_ids, int M, void *(*start_routine) (void *))
{
    for (int i = 0; i < M; i++)
    {
        thread_ids[i] = i;
        pthread_create(&threads[i], NULL, start_routine, &thread_ids[i]);
    }
}

void join_threads(pthread_t *threads, int M)
{
    for (int i = 0; i < M; i++)
    {
        pthread_join(threads[i], NULL);
    }
}

// Fill matrix for problem 1
void fill_matrix()
{
    for (int i = 0; i < N; i++)
    {
        for (int j = 0; j < N; j++)
        {
            array[i * N + j] = rand() % 2;
        }
    }
}

// Fill array for problem 2
void fill_array()
{
    for (int i = 0; i < N; i++)
    {
        array[i] = rand() % 100;
    }
}

// Print matrix for problem 1
void print_matrix()
{
    for (int i = 0; i < N; i++)
    {
        for (int j = 0; j < N; j++)
        {
            printf("%d ", array[i * N + j]);
        }
        printf("\n");
    }
}

// Print array for problem 2
void print_array()
{
    for (int i = 0; i < N; i++)
    {
        printf("%d ", array[i]);
    }
    printf("\n");
}

// Stop function for problem 1
void *check_stop_criteria(void *arg)
{
    while (1) // Keep running until stop is true
    {
        bool all_zero = true;
        bool all_one = true;

        for (int i = 0; i < N; i++)
        {
            for (int j = 0; j < N; j++)
            {
                if (array[i * N + j] != 0)
                {
                    all_zero = false;
                }
                if (array[i * N + j] != 1)
                {
                    all_one = false;
                }
            }
        }

        stop = all_zero || all_one;

        if (stop) break; // Exit the loop if stop is true
    }

    return NULL;
}

// Stop function for problem 2
void *check_sorted(void *arg)
{
    while (!stop)
    {
        // Check if the array is sorted
        int sorted = 1;
        for (int i = 0; i < N - 1; i++)
        {
            if (array[i] > array[i + 1])
            {
                sorted = 0;
            }
        }
        if (sorted)
        {
            stop = true;
            pthread_exit(NULL);
            break;
        }
    }
    return NULL;
}

void *flipNumbers(void *arg)
{
    int i, j;
    int thread_id = *(int *)arg;

    while (!stop) // Keep running until the stopping criteria is met
    {
        // Generate a column i and a row j randomly
        i = rand() % N;
        j = rand() % N;

        // Count the number of 0s and the total number of neighbors
        int count = 0;
        int neighbors = 0;
        for (int x = -1; x <= 1; x++)
        {
            for (int y = -1; y <= 1; y++)
            {
                if (x != 0 || y != 0) // Exclude the element itself
                {
                    if (i + x >= 0 && i + x < N && j + y >= 0 && j + y < N) // Check boundaries
                    {
                        if (array[(i + x) * N + j + y] == 0)
                        {
                            count++;
                        }
                        neighbors++;
                    }
                }
            }
        }

        pthread_mutex_lock(&lock); // Lock the mutex before changing the shared variable
        // Change the value of the matrix element based on the majority of its neighbors
        if (count > neighbors / 2)
        {
            printf("Thread %d is changing the element at (%d, %d) to 0 from %d\n", thread_id, i, j, array[i * N + j]);
            print_matrix();
            array[i * N + j] = 0;
        }
        else if (count < neighbors / 2)
        {
            printf("Thread %d is changing the element at (%d, %d) to 1 from %d\n", thread_id, i, j, array[i * N + j]);
            print_matrix();
            array[i * N + j] = 1;
        }
        pthread_mutex_unlock(&lock); // Unlock the mutex after changing the shared variable
    }

    return NULL;
}

void *insertionSort(void *arg)
{
    int thread_id = *(int *)arg;
    while (!stop)
    {
        // Generate an index i randomly
        int i = rand() % N;

        pthread_mutex_lock(&lock); // Lock the mutex before accessing the shared variable

        // Insert array[i] into the sorted sequence array[0..i-1]
        int key = array[i];
        int j = i - 1;
        while (j >= 0 && array[j] > key)
        {
            array[j + 1] = array[j];
            j = j - 1;
        }
        array[j + 1] = key;

        printf("Thread %d has inserted %d at position %d in the array.\n", thread_id, key, j+1);

        pthread_mutex_unlock(&lock); // Unlock the mutex after accessing the shared variable
    }
    return NULL;
}

void problem1(int M, pthread_t *threads, int *thread_ids, pthread_t stop_thread) {
    array = malloc(N * N * sizeof(int));

    // Randomly fill the matrix with 0s and 1s
    fill_matrix();
    printf("The starting matrix:\n");
    print_matrix();

    // Create a separate thread for checking the stopping criteria
    pthread_create(&stop_thread, NULL, check_stop_criteria, NULL);

    // Create M threads for problem 1
    create_threads(threads, thread_ids, M, flipNumbers);

    // Join all threads
    join_threads(threads, M);
    pthread_join(stop_thread, NULL);

    printf("The ending matrix:\n");
    print_matrix();
    stop = false;
    free(array);
}

void problem2(int M, pthread_t *threads, int *thread_ids, pthread_t stop_thread) {
    array = malloc(N * sizeof(int));

    // Randomly fill the array with numbers
    fill_array();
    printf("Original Array: ");
    print_array();

    pthread_create(&stop_thread, NULL, check_sorted, NULL);

    // Create M threads for problem 2
    create_threads(threads, thread_ids, M, insertionSort);

    // Join all threads
    join_threads(threads, M);
    pthread_join(stop_thread, NULL);

    printf("Sorted Array: ");
    print_array();

    free(array);
}

int main(int argc, char *argv[]) {
    validate_input(argc, argv);

    int choice;
    do {
        choice = display_menu();

        int M;
        printf("Enter the number of threads: ");
        scanf("%d", &M);

        pthread_t threads[M];
        int *thread_ids = malloc(M * sizeof(int)); // Dynamically allocate memory for thread_ids
        pthread_t stop_thread;

        switch (choice) {
            case 1:
                problem1(M, threads, thread_ids, stop_thread);
                free(thread_ids); // Free the memory for thread_ids after all threads have finished executing
                stop = false;
                break;
            case 2:
                problem2(M, threads, thread_ids, stop_thread);
                free(thread_ids); // Free the memory for thread_ids after all threads have finished executing
                stop = false;
                break;

            case 3:
                printf("Exiting...\n");
                break;

            default:
                printf("Invalid choice. Please enter a number between 1 and 3.\n");
        }
    } while (choice != 3);

    return 0;
}

