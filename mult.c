#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>

int **A, **B, **C;
int N, numThreads;

// This struct helps define which rows each thread will work on
typedef struct {
    int start;
    int end;
} ThreadRange;

// Each thread runs this — it just handles the part of the matrix it's responsible for
void *multiply(void *arg) {
    ThreadRange *range = arg;
    for (int i = range->start; i < range->end; i++) {
        for (int j = 0; j < N; j++) {
            C[i][j] = 0;
            for (int k = 0; k < N; k++) {
                // Standard matrix multiplication logic
                C[i][j] += A[i][k] * B[k][j];
            }
        }
    }
    return NULL;
}

// Reads values from the given file and fills the matrix
void loadMatrix(const char *filename, int **matrix) {
    FILE *fp = fopen(filename, "r");
    if (!fp) exit(1); // If we can't open the file, just stop the program

    // Fill the matrix row by row
    for (int i = 0; i < N; i++)
        for (int j = 0; j < N; j++)
            fscanf(fp, "%d", &matrix[i][j]);
    fclose(fp);
}

// Writes the result matrix to the output file, one number per line
void saveMatrix(const char *filename, int **matrix) {
    FILE *f = fopen(filename, "w");
    if (!f) exit(1); // Again, just stop if something goes wrong opening the file

    for (int i = 0; i < N; i++)
        for (int j = 0; j < N; j++)
            fprintf(f, "%d\n", matrix[i][j]);
    fclose(f);
}

int main(int argc, char *argv[]) {
    // We expect 5 arguments (excluding the program name)
    if (argc != 6) {
        printf("Usage: %s <numThreads> <N> <fileA> <fileB> <fileC>\n", argv[0]);
        return 1;
    }

    // Read the thread count and matrix size from command line
    if (sscanf(argv[1], "%d", &numThreads) != 1 || sscanf(argv[2], "%d", &N) != 1) {
        printf("Invalid arguments for numThreads or N.\n");
        return 1;
    }

    // The project said to only allow 1 to 8 threads
    if (numThreads < 1 || numThreads > 8) {
        printf("Error: numThreads must be between 1 and 8.\n");
        return 1;
    }

    // Input and output file names
    const char *fileA = argv[3];
    const char *fileB = argv[4];
    const char *fileC = argv[5];

    // Allocate space for A, B, and C (each are N x N matrices)
    A = malloc(N * sizeof(int *));
    B = malloc(N * sizeof(int *));
    C = malloc(N * sizeof(int *));
    for (int i = 0; i < N; i++) {
        A[i] = malloc(N * sizeof(int));
        B[i] = malloc(N * sizeof(int));
        C[i] = malloc(N * sizeof(int));
    }

    // Load matrix data from files
    loadMatrix(fileA, A);
    loadMatrix(fileB, B);

    pthread_t threads[numThreads];
    ThreadRange ranges[numThreads];

    // Start timer before starting the actual multiplication
    struct timeval start, end;
    gettimeofday(&start, NULL);

    // Split the matrix rows evenly among all threads
    int rowsPerThread = N / numThreads;
    for (int i = 0; i < numThreads; i++) {
        // Figure out the starting and ending row for each thread
        ranges[i].start = i * rowsPerThread;
        // Last thread might need to pick up extra rows if N isn't evenly divisible
        ranges[i].end = (i == numThreads - 1) ? N : (i + 1) * rowsPerThread;

        // Start the thread and pass its range info
        pthread_create(&threads[i], NULL, multiply, &ranges[i]);
    }

    // Wait for all threads to finish before continuing
    for (int i = 0; i < numThreads; i++) {
        pthread_join(threads[i], NULL);
    }

    // End the timer now that multiplication is done
    gettimeofday(&end, NULL);
    double elapsed = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1e6;
    printf("Execution Time: %.6f seconds\n", elapsed);

    // Write the resulting matrix to the output file
    saveMatrix(fileC, C);

    // Free up all the memory we used for the matrices
    for (int i = 0; i < N; i++) {
        free(A[i]);
        free(B[i]);
        free(C[i]);
    }
    free(A);
    free(B);
    free(C);

    return 0;
}
