#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

void variation1();

void *calculateElement(void *args);

void variation2();

void *calculateRow(void *args);

int **matrixA;
int aRows, aColumns;

int **matrixB;
int bRows, bColumns;

int **matrixC;
int cRows, cColumns;

int main() {

    FILE *pInputFile = fopen("input.txt", "r");

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

    fclose(pInputFile);


    cRows = aRows;
    cColumns = bColumns;

    matrixC = (int **) malloc(cRows * sizeof(int *));

    for (int i = 0; i < aRows; i++) {
        matrixC[i] = (int *) malloc(cColumns * sizeof(int));
    }

    FILE *pOut = fopen("output.txt", "w");
    clock_t begin, end;

    begin = clock();
    variation1();
    end = clock();

    double variation1_time_spent = (double) (end - begin) / CLOCKS_PER_SEC;

    for (int i = 0; i < cRows; i++) {
        for (int j = 0; j < cColumns; j++) {
            fprintf(pOut, "%d ", matrixC[i][j]);
        }
        fprintf(pOut, "\n");
    }

    fprintf(pOut, "END1 [%lf]\n", variation1_time_spent);

    begin = clock();
    variation2();
    end = clock();

    double variation2_time_spent = (double) (end - begin) / CLOCKS_PER_SEC;

    for (int i = 0; i < cRows; i++) {
        for (int j = 0; j < cColumns; j++) {
            fprintf(pOut, "%d ", matrixC[i][j]);
        }
        fprintf(pOut, "\n");
    }

    fprintf(pOut, "END2 [%lf]", variation2_time_spent);


    fclose(pOut);

    return 0;
}

struct ThreadData {
    int aRowIndex;
    int bColumnIndex;
};

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

/**
 * calculates each element in output matrixC in a thread
 *
 * @param args struct that contains which row from matrixA to multiply and get sum with which column from matrixB
 */
void *calculateElement(void *args) {

    struct ThreadData *data = (struct ThreadData *) args;

    int rowIndex = data->aRowIndex;
    int columnIndex = data->bColumnIndex;


    int sum = 0;

    for (int i = 0; i < aColumns; i++)
        sum += matrixA[rowIndex][i] * matrixB[i][columnIndex];

    matrixC[rowIndex][columnIndex] = sum;


    free(data);

}

void variation2() {

    int rc;
    pthread_attr_t attr;
    pthread_t thread[cRows];
    void *status;

    //Initialize and set thread detached attribute
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

    for (int i = 0; i < cRows; i++) {

        //create thread
        rc = pthread_create(&thread[i], &attr, calculateRow, (void *) i);

        if (rc) {
            printf("ERROR; return code from pthread_create() is %d\n", rc);
            exit(-1);
        }

    }


    // Free attribute and wait for the other threads by using join
    pthread_attr_destroy(&attr);
    for (int i = 0; i < cRows; i++) {
        rc = pthread_join(thread[i], &status);
        if (rc) {
            printf("ERROR; return code from pthread_join() is %d\n", rc);
            exit(-1);
        }

//        printf("Main: completed join with thread %ld having a status of % ld\n", i, (long) status);
    }
}

/**
 * calculates each row of the output matrix c in a thread
 *
 * @param args takes matrixC row index to calculate
 */
void *calculateRow(void *args) {

    int cRowToCalculate = (int) args;

    int sum;
    for (int i = 0; i < cColumns; i++) {
        sum = 0;
        for (int j = 0; j < aColumns; j++)
            sum += matrixA[cRowToCalculate][j] * matrixB[j][i];
        matrixC[cRowToCalculate][i] = sum;

    }

}