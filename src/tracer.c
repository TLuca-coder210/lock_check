#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdbool.h>
#include <fcntl.h>
#include <string.h>

#define DYLD_INTERPOSE(_replacement,_replacee) \
   __attribute__((used)) static struct{ const void* replacement; const void* replacee; } _interpose_##_replacee \
   __attribute__ ((section ("__DATA,__interpose"))) = { (const void*)&_replacement, (const void*)&_replacee };

__thread bool in_hook = false; 

__attribute__((constructor)) void init_hook() {
    int fd = open("test.log", O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if (fd != -1) close(fd);
}

void log_event(const char* action, pthread_mutex_t *mutex) {
    int fd = open("test.log", O_WRONLY | O_APPEND);
    if (fd != -1) {
        unsigned long long tid = (unsigned long long)pthread_self();
        unsigned long long mid = (unsigned long long)mutex;
        
        char buffer[256];
        int len = snprintf(buffer, sizeof(buffer), "%llu %s %llx\n", tid, action, mid);
        
        write(fd, buffer, len);
        close(fd);
    }
}

int my_pthread_mutex_lock(pthread_mutex_t *mutex) {
    if (in_hook) return pthread_mutex_lock(mutex);
    
    in_hook = true;
    log_event("REQUEST", mutex);
    int result = pthread_mutex_lock(mutex); 
    log_event("ACQUIRE", mutex);
    in_hook = false;
    
    return result;
}

int my_pthread_mutex_unlock(pthread_mutex_t *mutex) {
    if (in_hook) return pthread_mutex_unlock(mutex);
    
    in_hook = true;
    log_event("RELEASE", mutex);
    int result = pthread_mutex_unlock(mutex);
    in_hook = false;
    
    return result;
}

DYLD_INTERPOSE(my_pthread_mutex_lock, pthread_mutex_lock)
DYLD_INTERPOSE(my_pthread_mutex_unlock, pthread_mutex_unlock)
