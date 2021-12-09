#include <stdio.h>
#include <pthread.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#define MAX 50

int *arr;
pthread_t array1[MAX];
int index1;
pthread_mutex_t safe;
// void *consume_rand()
// {
//     for(int i=0;i<5;i++){
//         pthread_mutex_lock(&lock);
//         int n= rand()%50;
//         printf("n : %d value: %d\n", n, arr[n]);
//         pthread_mutex_unlock(&lock);
//     }
// }   
void *mythread(void *arg)
{
    pthread_mutex_lock(&safe);
    arr[index1] = rand() % 100000;
    printf("The element : %d is on the index: %d\n", arr[index1], index1);
    index1 += 1;

    pthread_mutex_unlock(&safe);

    return NULL;
}

int main(int argc, char *argv[])
{
    srand(time(NULL));
    arr = (int *)malloc(sizeof(int) * MAX);
    index1 = 0;

    for (int i = 0; i < MAX; i++)
    {
        *(arr + i) = 0;
    };

    for (int i = 0; i < MAX; i++)
    {
        pthread_create(&(array1[i]), NULL, mythread, NULL);
    }
    for (int i = 0; i < MAX; i++)
    {
        pthread_join((array1[i]), NULL);
    }

    // for (int i = 0; i < MAX; i++)
    // {
    //     printf("%d ", *(arr + i));
    // };
    // printf("\n");

    return 1;
}
