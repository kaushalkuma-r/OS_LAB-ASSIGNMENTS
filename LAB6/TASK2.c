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

void *mythread(void *arg)
{
    pthread_mutex_lock(&safe);
    arr[index1] = rand() % 100000;
    printf("The element : %d is on the index: %d\n", arr[index1], index1);
    index1 += 1;

    pthread_mutex_unlock(&safe);

    return NULL;
}
// void *consume_rand()
// {
//     for(int i=0;i<5;i++){
//         pthread_mutex_lock(&lock);
//         int n= rand()%50;
//         printf("n : %d value: %d\n", n, arr[n]);
//         pthread_mutex_unlock(&lock);
//     }
// }   
void* ind(void *args){

    int idx = *((int *) args);
    printf("Index of the element : %d is : %d \n" , arr[idx] , idx);
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
    printf("\n");
    // // consume_rand();
    // pthread_t t1,t2,t3;
    // pthread_create(t1,NULL,consume_rand(),NULL);
    // pthread_create(t2,NULL,)
    pthread_t c1;
    int foo = 32;
    pthread_create(&c1 , NULL , ind, &foo);
    pthread_join(c1, NULL);
    foo = rand() % 50;
    pthread_create(&c1 , NULL , ind, &foo);
    pthread_join(c1, NULL);
    return 1;
}
