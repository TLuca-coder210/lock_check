#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

pthread_mutex_t mutex_A = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_B = PTHREAD_MUTEX_INITIALIZER;

void* thread_1_logic(void* arg) {
    pthread_mutex_lock(&mutex_A);
    sleep(1);
    pthread_mutex_lock(&mutex_B);
    
    pthread_mutex_unlock(&mutex_B);
    pthread_mutex_unlock(&mutex_A);
    return NULL;
}

void* thread_2_logic(void* arg) {
    pthread_mutex_lock(&mutex_B);
    sleep(1);
    pthread_mutex_lock(&mutex_A);
    
    pthread_mutex_unlock(&mutex_A);
    pthread_mutex_unlock(&mutex_B);
    return NULL;
}

int main() {
    pthread_t t1, t2;
    
    pthread_create(&t1, NULL, thread_1_logic, NULL);
    pthread_create(&t2, NULL, thread_2_logic, NULL);
    
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    
    return 0;
}
