#include <iostream>
#include <random>
#include <unistd.h>
#include "pthread.h"
#include "semaphore.h"

using namespace std;

struct Thread {
    unsigned int index;
    pthread_t thread;
};

struct Summer : Thread {
    unsigned int a;
    unsigned int b;
};

const int THREAD_NUM = 20;

random_device rd;
mt19937 mt(rd());
uniform_int_distribution<unsigned int> dist(0, UINT32_MAX);

pthread_rwlock_t rwlock;
pthread_mutex_t mutex;
pthread_cond_t ready_to_read;
Thread generators[THREAD_NUM];
Summer summers[THREAD_NUM];

unsigned int buffer[THREAD_NUM];
unsigned int front = 0;
unsigned int rear = 0;

void *Generate(void *params) {
    auto index = *(unsigned int *) params;
    auto time = dist(mt) % 7 + 1;
    sleep(time);
    auto number = dist(mt) % THREAD_NUM + 1;
    pthread_rwlock_wrlock(&rwlock);
    buffer[rear] = number;
    rear = (rear + 1) % THREAD_NUM;
    pthread_rwlock_unlock(&rwlock);
    printf("Thread %d slept %d and wrote number %d\n", index, time, number);
    if ((20 + rear - front) % 20 >= 2)
        pthread_cond_broadcast(&ready_to_read);
    return nullptr;
}

void* Sum(void* params) {
    auto val = (Summer *) params;
    auto time = dist(mt) % 4 + 3;
    sleep(time);
    auto res = val->a + val->b;
    pthread_rwlock_wrlock(&rwlock);
    buffer[rear] = res;
    rear = (rear + 1) % THREAD_NUM;
    pthread_rwlock_unlock(&rwlock);
    printf("Summer %d slept %d and wrote number %d\n", val->index, time, res);
    if ((20 + rear - front) % 20 >= 2)
        pthread_cond_broadcast(&ready_to_read);
    return nullptr;
}

int main() {
    pthread_mutex_init(&mutex, nullptr);
    pthread_cond_init(&ready_to_read, nullptr);

    for (unsigned int i = 0; i < THREAD_NUM; ++i) {
        generators[i].index = i;
        pthread_create(&generators[i].thread, nullptr, &Generate, (void *) &generators[i].index);
        printf("Thread %d created\n", i);
    }


    for (unsigned int i = 0; i < THREAD_NUM - 1; ++i) {
        while ((20 + rear - front) % 20 < 2)
            pthread_cond_wait(&ready_to_read, &mutex);
        summers[i].index = i;
        pthread_rwlock_rdlock(&rwlock);
        auto a = buffer[front];
        front = (front + 1) % THREAD_NUM;
        auto b = buffer[front];
        front = (front + 1) % THREAD_NUM;
        pthread_rwlock_unlock(&rwlock);
        summers[i].a = a;
        summers[i].b = b;
        pthread_create(&summers[i].thread, nullptr, &Sum, (void*) &summers[i]);
        printf("Summer %d created to sum numbers %d and %d\n", i, a, b);
    }


    for (auto thread: generators)
        pthread_join(thread.thread, nullptr);
    for (auto summer : summers)
        pthread_join(summer.thread, nullptr);

    pthread_rwlock_destroy(&rwlock);

    printf("Result: %d", buffer[front]);
}