#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <stdbool.h>


bool status = true;

struct buffer
{
    int value;
    struct buffer *next;
};

struct buffer *leftmost = NULL;
struct buffer *rightmost = NULL;

void print(){
    buffer *temp = leftmost;
    while(temp!=NULL){
        printf("%d ",temp->value);
        temp = temp->next;
    }
    printf("\n");
}


typedef struct
{
    int value;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
} my_semaphore;

void sema_init(my_semaphore *sem, int value)
{
    sem->value = value;
    pthread_mutex_init(&sem->mutex, NULL);
    pthread_cond_init(&sem->cond, NULL);
}

void sema_wait(my_semaphore *sem)
{
    pthread_mutex_lock(&sem->mutex);
    while (sem->value == 0)
    {
        pthread_cond_wait(&sem->cond, &sem->mutex);
    }
    sem->value--;
    pthread_mutex_unlock(&sem->mutex);
}

void sema_signal(my_semaphore *sem)
{
    pthread_mutex_lock(&sem->mutex);
    sem->value++;
    pthread_cond_signal(&sem->cond);
    pthread_mutex_unlock(&sem->mutex);
}

my_semaphore full, sema0;

void producer(void *ID)
{
    int producer_number = *((int *)ID);
    bool b = true;
    // while (b || status)
    while(true)
    {
        b = false;
        int items_produced = (rand() % 100) + 1;

        sema_wait(&sema0);

        struct buffer *newBuffer = (struct buffer *)malloc(sizeof(struct buffer));
        newBuffer->value = items_produced;
        newBuffer->next = NULL;

        if (rightmost != NULL)
        {
            rightmost->next = newBuffer;
        }
        else
        {
            leftmost = newBuffer;
        }
        rightmost = newBuffer;

        printf("producer %d produced %d\n", producer_number, items_produced);
        print();

        sema_signal(&full);
        sema_signal(&sema0);

        sleep((rand() % 5) + 1);
    }
}

void consumer(void *ID)
{
    int consumer_number = *((int *)ID);

    bool b = true;
    // while (b || status)
    while(true)
    {
        b = false;
        sema_wait(&full);
        sema_wait(&sema0);

        // if (leftmost == NULL)
        // {
        //     sema_signal(&full);
        //     sema_signal(&sema0);
        //     return;
        // }

        int items_consumed = leftmost->value;
        struct buffer *temp = leftmost;

        if (leftmost == rightmost)
        {
            leftmost = NULL;
            rightmost = NULL;
        }
        else
        {
            leftmost = leftmost->next;
        }

        free(temp);

        

        printf("consumer %d consumed %d\n", consumer_number, items_consumed);
        print();

        sema_signal(&sema0);
        sleep((rand() % 5) + 1);
    }
}

int main()
{
    int n1 = 50;
    int n2 = 5;

    sema_init(&full, 0);
    sema_init(&sema0, 1);

    pthread_t pro[n1], cons[n2];

    int producer_arr[n1];
    int consumer_arr[n2];
    for (int i = 0; i < n1; i++)
    {
        producer_arr[i] = i;
    }
    for (int i = 0; i < n2; i++)
    {
        consumer_arr[i] = i;
    }

    for (int i = 0; i < n1; i++)
    {
        pthread_create(&pro[i], NULL, (void *(*)(void *))producer, (void *)&producer_arr[i]);
    }
    for (int i = 0; i < n2; i++)
    {
        pthread_create(&cons[i], NULL, (void *(*)(void *))consumer, (void *)&consumer_arr[i]);
    }

    sleep(5);
    status = false;

    for (int i = 0; i < n1; i++)
    {
        pthread_join(pro[i], NULL);
    }
    for (int i = 0; i < n2; i++)
    {
        pthread_join(cons[i], NULL);
    }

    return 0;
}