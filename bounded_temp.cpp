#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <stdbool.h>

const int buffer_size = 2;
// int buffer[buffer_size];

struct buffer
{
    int value;
    struct buffer *next;
};

struct buffer *leftmost = NULL;
struct buffer *rightmost = NULL;

void print()
{
    buffer *temp = leftmost;
    while (temp != NULL)
    {
        printf("%d ", temp->value);
        temp = temp->next;
    }
    printf("\n");
}

int top = 0;

bool status = true;

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

int get_value(my_semaphore *sem)
{
    int value;
    pthread_mutex_lock(&sem->mutex);
    value = sem->value;
    pthread_mutex_unlock(&sem->mutex);
    return value;
}

my_semaphore full, empty, sema0;
void producer(void *ID)
{
    int producer_number = *((int *)ID);
    bool b = true;
    while (b || status)
    {
        b = false;
        int items_produced = (rand() % 100) + 1;

        sema_wait(&empty);
        sema_wait(&sema0);

        if (top >= buffer_size)
        {
            sema_signal(&empty);
            sema_signal(&sema0);
            return;
        }
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
        top++;

        printf("producer %d produced %d\n", producer_number, items_produced);

        print();

        // for (int i = 0; i < buffer_size; ++i)
        // {
        //     printf("%d ", buffer[i]);
        // }
        // printf("\n");

        sema_signal(&sema0);
        sema_signal(&full);

        sleep((rand() % 5) + 1);
    }
}

void consumer(void *ID)
{
    int consumer_number = *((int *)ID);

    bool b = true;
    while (b || status)
    {
        b = false;
        sema_wait(&full);
        sema_wait(&sema0);

        if (top <= 0)
        {
            sema_signal(&full);
            sema_signal(&sema0);
            return;
        }
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

        top--;

        printf("consumer %d consumed %d\n", consumer_number, items_consumed);
        print();
        sema_signal(&sema0);
        sema_signal(&empty);
        sleep((rand() % 5) + 1);
    }

    sema_signal(&empty);
}

int main()
{
    int n1 = 5;
    int n2 = 5;

    sema_init(&empty, buffer_size);
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