#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>

void variation1();

void *calculateElement(void *args);

int **matrixA;
int aRows, aColumns;

int **matrixB;
int bRows, bColumns;

sem_t mutex;

int main() {

    //open input file in read mode
    FILE *pInputFile = fopen("input.txt", "r");

    //check if it exists
    if (pInputFile == NULL) {
        puts("couldn't find input file");
        exit(0);
    }

    //read matrixA then matrixB
    fscanf(pInputFile, "%d", &aRows);
    fscanf(pInputFile, "%d", &aColumns);

    matrixA = (int **) malloc(aRows * sizeof(int *));

    for (int i = 0; i < aRows; i++) {
        matrixA[i] = (int *) malloc(aColumns * sizeof(int));
        for (int j = 0; j < aColumns; j++)
            fscanf(pInputFile, "%d", &matrixA[i][j]);
    }


    fscanf(pInputFile, "%d", &bRows);
    fscanf(pInputFile, "%d", &bColumns);

    matrixB = (int **) malloc(bRows * sizeof(int *));

    for (int i = 0; i < bRows; i++) {
        matrixB[i] = (int *) malloc(bColumns * sizeof(int));
        for (int j = 0; j < bColumns; j++)
            fscanf(pInputFile, "%d", &matrixB[i][j]);
    }

    //close input file
    fclose(pInputFile);


//    for (int i = 0; i < aRows; i++) {
//        for (int j = 0; j < aColumns; j++) {
//            printf("%d ", matrixA[i][j]);
//        }
//        puts("");
//    }
//
//    for (int i = 0; i < bRows; i++) {
//        for (int j = 0; j < bColumns; j++) {
//            printf("%d ", matrixB[i][j]);
//        }
//        puts("");
//    }


    sem_init(&mutex, 0, 1);

    variation1();

    sem_destroy(&mutex);

    return 0;
}


struct ThreadData {
    int aRowIndex;
    int bColumnIndex;
};

//program 1 starts from here
//matrixA , matrixB are shared memory for all threads
void variation1() {

    int rc;
    pthread_attr_t attr;
    pthread_t thread[aRows * bColumns];
    void *status;

    //Initialize and set thread detached attribute
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

    for (int i = 0; i < aRows; i++) {
        for (int j = 0; j < bColumns; j++) {

            //struct to pass arguments to each thread
            struct ThreadData *threadData = malloc(sizeof(struct ThreadData));
            threadData->aRowIndex = i;
            threadData->bColumnIndex = j;

            //create thread
            rc = pthread_create(&thread[i * bColumns + j], &attr, calculateElement, (void *) threadData);

            if (rc) {
                printf("ERROR; return code from pthread_create() is %d\n", rc);
                exit(-1);
            }
        }

    }


    // Free attribute and wait for the other threads by using join
    pthread_attr_destroy(&attr);
    for (int i = 0; i < aRows * bColumns; i++) {
        rc = pthread_join(thread[i], &status);
        if (rc) {
            printf("ERROR; return code from pthread_join() is %d\n", rc);
            exit(-1);
        }

//        printf("Main: completed join with thread %ld having a status of % ld\n", i, (long) status);
    }

}


void *calculateElement(void *args) {

    struct ThreadData *data = (struct ThreadData *) args;

    sem_wait(&mutex);
    printf("row %d , column %d \n", data->aRowIndex, data->bColumnIndex);
    free(data);
    sem_post(&mutex);

}