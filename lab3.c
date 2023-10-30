#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>

#define DEFAULT_ELEMENTS 8

typedef struct {
    int N; // Size of the matrix/array
    int *array;
    bool stop; // Used to stop the simulations
    int *thread_ids;
} Data;

typedef struct {
    Data *data;
    int thread_id;
} ThreadArgs;

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

// Functions shared for both problems
void validate_input(int argc, char *argv[], void *arg) {
    Data *data = (Data *)arg;
    data->N = DEFAULT_ELEMENTS; // Default size
    if (argc > 1)
    {
        char *ptr;
        long input = strtol(argv[1], &ptr, 10);
        if (*ptr == '\0')
        {
            data->N = (int)input;
        }
        else
        {
            printf("Using Default Size of %d\n", data->N);
        }
    }
    else
    {
        printf("Using Default Size of %d\n", data->N);
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

ThreadArgs* create_threads(pthread_t *threads, Data *data, int M, void *(*start_routine) (void *)) {
    ThreadArgs* args = malloc(M * sizeof(ThreadArgs));
    for (int i = 0; i < M; i++) {
        args[i].data = data;
        args[i].thread_id = i;
        pthread_create(&threads[i], NULL, start_routine, &args[i]);
    }
    return args;
}


void join_threads(pthread_t *threads, int M)
{
    for (int i = 0; i < M; i++)
    {
        pthread_join(threads[i], NULL);
    }
}

// Fill matrix for problem 1
void fill_matrix(void *arg)
{
    Data *data = (Data *)arg;
    for (int i = 0; i <  data->N; i++)
    {
        for (int j = 0; j <  data->N; j++)
        {
            data->array[i *  data->N + j] = rand() % 2;
        }
    }
}

// Fill array for problem 2
void fill_array(void *arg)
{
    Data *data = (Data *)arg;
    for (int i = 0; i < data->N; i++)
    {
        data->array[i] = rand() % 100;
    }
}

// Print matrix for problem 1
void print_matrix(void *arg)
{
    Data *data = (Data *)arg;
    for (int i = 0; i < data->N; i++)
    {
        for (int j = 0; j < data->N; j++)
        {
            printf("%d ", data->array[i * data->N + j]);
        }
        printf("\n");
    }
}

// Print array for problem 2
void print_array(void *arg)
{
    Data *data = (Data *)arg;
    for (int i = 0; i < data->N; i++)
    {
        printf("%d ", data->array[i]);
    }
    printf("\n");
}

// Stop function for problem 1
void *check_stop_criteria(void *arg)
{
    Data *data = (Data *)arg;
    while (1) // Keep running until stop is true
    {
        bool all_zero = true;
        bool all_one = true;

        for (int i = 0; i < data->N; i++)
        {
            for (int j = 0; j < data->N; j++)
            {
                if (data->array[i * data->N + j] != 0)
                {
                    all_zero = false;
                }
                if (data->array[i * data->N + j] != 1)
                {
                    all_one = false;
                }
            }
        }

        data->stop = all_zero || all_one;

        if (data->stop) break; // Exit the loop if stop is true
    }

    return NULL;
}

// Stop function for problem 2
void *check_sorted(void *arg)
{
    Data *data = (Data *)arg;
    while (!data->stop)
    {
        // Check if the array is sorted
        int sorted = 1;
        for (int i = 0; i < data->N - 1; i++)
        {
            if (data->array[i] > data->array[i + 1])
            {
                sorted = 0;
            }
        }
        if (sorted)
        {
            data->stop = true;
            pthread_exit(NULL);
            break;
        }
    }
    return NULL;
}

void *flipNumbers(void *arg)
{
    ThreadArgs *args = (ThreadArgs *)arg;
    Data *data = args->data;

    int i, j;

    while (!data->stop) // Keep running until the stopping criteria is met
    {
        // Generate a column i and a row j randomly
        i = rand() % data->N;
        j = rand() % data->N;

        // Count the number of 0s and the total number of neighbors
        int count = 0;
        int neighbors = 0;
        for (int x = -1; x <= 1; x++)
        {
            for (int y = -1; y <= 1; y++)
            {
                if (x != 0 || y != 0) // Exclude the element itself
                {
                    if (i + x >= 0 && i + x < data->N && j + y >= 0 && j + y < data->N) // Check boundaries
                    {
                        if (data->array[(i + x) * data->N + j + y] == 0)
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
            printf("Thread %d is changing the element at (%d, %d) to 0 from %d\n", args->thread_id, i, j, data->array[i * data->N + j]);
            print_matrix(&data);
            data->array[i * data->N + j] = 0;
        }
        else if (count < neighbors / 2)
        {
            printf("Thread %d is changing the element at (%d, %d) to 1 from %d\n", args->thread_id, i, j, data->array[i * data->N + j]);
            print_matrix(&data);
            data->array[i * data->N + j] = 1;
        }
        pthread_mutex_unlock(&lock); // Unlock the mutex after changing the shared variable
    }

    return NULL;
}

void *insertionSort(void *arg)
{
    ThreadArgs *args = (ThreadArgs *)arg;
    Data *data = args->data;
    while (! data->stop)
    {
        // Generate an index i randomly
        int i = rand() %  data->N;

        pthread_mutex_lock(&lock); // Lock the mutex before accessing the shared variable

        // Insert array[i] into the sorted sequence array[0..i-1]
        int key =  data->array[i];
        int j = i - 1;
        while (j >= 0 &&  data->array[j] > key)
        {
            data->array[j + 1] =  data->array[j];
            j = j - 1;
        }
        data->array[j + 1] = key;

        printf("Thread %d has inserted %d at position %d in the array.\n", args->thread_id, key, j+1);

        pthread_mutex_unlock(&lock); // Unlock the mutex after accessing the shared variable
    }
    return NULL;
}

void problem1(int M, pthread_t *threads, pthread_t stop_thread, void *arg) {
    Data *data = (Data *)arg;
    data->array = malloc(data->N * data->N * sizeof(int));

    // Randomly fill the matrix with 0s and 1s
    fill_matrix(data);
    printf("The starting matrix:\n");
    print_matrix(data);

    // Create a separate thread for checking the stopping criteria
    pthread_create(&stop_thread, NULL, check_stop_criteria, data);

    // Create M threads for problem 1
    ThreadArgs* args = create_threads(threads, data, M, flipNumbers);

    // Join all threads
    join_threads(threads, M);
    pthread_join(stop_thread, NULL);

    printf("The ending matrix:\n");
    print_matrix(data);
    data->stop = false;
    free(data->array);
    free(args);
}

void problem2(int M, pthread_t *threads, pthread_t stop_thread, void *arg) {
    Data *data = (Data *)arg;
    data->array = malloc(data->N * sizeof(int));

    // Randomly fill the array with numbers
    fill_array(data);
    printf("Original Array: ");
    print_array(data);
    printf("Original Array: ");

    pthread_create(&stop_thread, NULL, check_sorted, data);

    // Create M threads for problem 2
    ThreadArgs* args = create_threads(threads, data, M, insertionSort);

    // Join all threads
    join_threads(threads, M);
    pthread_join(stop_thread, NULL);

    printf("Sorted Array: ");
    print_array(data);

    free(data->array);
    free(args);
}

int main(int argc, char *argv[]) {
    Data data;
    data.N = DEFAULT_ELEMENTS; // Default size
    data.stop = false;

    validate_input(argc, argv, &data);

    int choice;
    do {
        choice = display_menu();

        int M;
        printf("Enter the number of threads: ");
        scanf("%d", &M);

        data.thread_ids = malloc(M * sizeof(int)); // Dynamically allocate memory for thread_ids
        pthread_t threads[M];
        pthread_t stop_thread;

        switch (choice) {
            case 1:
                problem1(M, threads, stop_thread, &data);
                data.stop = false;
                break;
            case 2:
                problem2(M, threads, stop_thread, &data);
                data.stop = false;
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

