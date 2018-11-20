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

    //read input file
    FILE *pInputFile = fopen("input.txt", "r");
    if (pInputFile == NULL) {
        puts("couldn't find input file");
        exit(0);
    }

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


    //initialize output matrixC
    cRows = aRows;
    cColumns = bColumns;

    matrixC = (int **) malloc(cRows * sizeof(int *));

    for (int i = 0; i < aRows; i++) {
        matrixC[i] = (int *) malloc(cColumns * sizeof(int));
    }

    //open output file
    FILE *pOut = fopen("output.txt", "w");
    clock_t begin, end;

    //run first code variation
    begin = clock();
    variation1();
    end = clock();

    //calc running time of variation1
    double variation1_time_spent = (double) (end - begin) / CLOCKS_PER_SEC;

    //output the array
    for (int i = 0; i < cRows; i++) {
        for (int j = 0; j < cColumns; j++) {
            fprintf(pOut, "%d ", matrixC[i][j]);
        }
        fprintf(pOut, "\n");
    }

    //output running time of variation1
    fprintf(pOut, "END1 [%lf]\n", variation1_time_spent);

    //run second code variation
    begin = clock();
    variation2();
    end = clock();

    //calc running time of variation2
    double variation2_time_spent = (double) (end - begin) / CLOCKS_PER_SEC;

    //output the array
    for (int i = 0; i < cRows; i++) {
        for (int j = 0; j < cColumns; j++) {
            fprintf(pOut, "%d ", matrixC[i][j]);
        }
        fprintf(pOut, "\n");
    }

    //output running time of variation2
    fprintf(pOut, "END2 [%lf]", variation2_time_spent);

    //close output file
    fclose(pOut);

    free(matrixA);
    free(matrixB);
    free(matrixC);

    return 0;
}

//data handed to each thread in variation1
//specifies which element in matrixC to calculate
struct ThreadData {
    int aRowIndex;
    int bColumnIndex;
};

/**
 * matrix multiplication is done by creating a new thread
 * for each element in output matrixC to calculate its value.
 */
void variation1() {

    //return code of thread creation
    int rc;

    //attributes used for thread creation
    pthread_attr_t attr;

    //create a unique identifier for each thread (number of threads = number of elements in output matrixC)
    pthread_t thread[aRows * bColumns];

    //Initialize and set thread detached attribute to specify that it will be
    //joined later after creation
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

    //for each element in output matrix create a thread and give it
    //needed data to know which element to calculate
    //i.e:location of element in matrixC
    for (int i = 0; i < aRows; i++) {
        for (int j = 0; j < bColumns; j++) {

            //create a ThreadData and initialize it with location of element in matrixC
            //(row index from matrixA and column index from matrixB)
            struct ThreadData *threadData = malloc(sizeof(struct ThreadData));
            threadData->aRowIndex = i;
            threadData->bColumnIndex = j;

            //create the thread
            rc = pthread_create(&thread[i * bColumns + j], &attr, calculateElement, (void *) threadData);

            if (rc) {
                printf("ERROR; return code from pthread_create() is %d\n", rc);
                exit(-1);
            }
        }

    }


    //Free attr and join created threads with main thread
    pthread_attr_destroy(&attr);
    for (int i = 0; i < aRows * bColumns; i++) {
        rc = pthread_join(thread[i], NULL);
        if (rc) {
            printf("ERROR; return code from pthread_join() is %d\n", rc);
            exit(-1);
        }

//        printf("Main: completed join with thread %ld having a status of % ld\n", i, (long) status);
    }

}

/**
 * routine for each thread in variation1
 *
 * calculates each element in output matrixC
 *
 * @param args ThreadData*
 */
void *calculateElement(void *args) {

    struct ThreadData *data = (struct ThreadData *) args;

    //retrieve values from ThreadData*
    int rowIndex = data->aRowIndex;
    int columnIndex = data->bColumnIndex;

    //calculate element
    int sum = 0;
    for (int i = 0; i < aColumns; i++)
        sum += matrixA[rowIndex][i] * matrixB[i][columnIndex];

    //save it in matrixC
    matrixC[rowIndex][columnIndex] = sum;

    free(data);

    pthread_exit(NULL);
}

/**
 * matrix multiplication is done by creating a new thread
 * for each row in output matrixC to calculate its values.
 */
void variation2() {

    //return code of thread creation
    int rc;

    //attributes used for thread creation
    pthread_attr_t attr;

    //create a unique identifier for each thread (number of threads = number of rows in output matrixC)
    pthread_t thread[cRows];

    //Initialize and set thread detached attribute to specify that it will be
    //joined later after creation
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

    //for each row in matrixC create a thread to calculate it's values
    for (int i = 0; i < cRows; i++) {

        //create thread
        rc = pthread_create(&thread[i], &attr, calculateRow, (void *) i);
        if (rc) {
            printf("ERROR; return code from pthread_create() is %d\n", rc);
            exit(-1);
        }

    }

    //Free attr and join created threads with main thread
    pthread_attr_destroy(&attr);
    for (int i = 0; i < cRows; i++) {
        rc = pthread_join(thread[i], NULL);
        if (rc) {
            printf("ERROR; return code from pthread_join() is %d\n", rc);
            exit(-1);
        }

//        printf("Main: completed join with thread %ld having a status of % ld\n", i, (long) status);
    }
}

/**
 * routine for each thread in variation2
 *
 * calculates each row in output matrixC
 *
 * @param args row index
 */
void *calculateRow(void *args) {

    int cRowToCalculate = (int) args;

    //calculate row elements
    int sum;
    for (int i = 0; i < cColumns; i++) {
        sum = 0;
        for (int j = 0; j < aColumns; j++)
            sum += matrixA[cRowToCalculate][j] * matrixB[j][i];
        matrixC[cRowToCalculate][i] = sum;

    }

    pthread_exit(NULL);
}