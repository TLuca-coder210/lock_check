#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#define MAX_THREADS 256
#define MAX_MUTEXES 1024

unsigned long long thread_keys[MAX_THREADS];
unsigned long long mutex_keys[MAX_MUTEXES];

int thread_waits_for[MAX_THREADS];
int mutex_held_by[MAX_MUTEXES];

typedef struct ThreadNode {
    struct ThreadNode *right;
    struct ThreadNode *left;
    unsigned long long thread_input_number;
    int index;
} binary_search_tree;

binary_search_tree *start = NULL;
int biggest_thread_index = 0;

int get_thread_index(unsigned long long thread){
    if(start == NULL){
        start = (binary_search_tree*)malloc(sizeof(binary_search_tree));
        start->thread_input_number = thread;
        start->index = biggest_thread_index++;
        start->right = NULL;
        start->left = NULL;
        return start->index;
    }
    binary_search_tree *current = start;
    binary_search_tree *parent = NULL;
    while(current != NULL){
        if(current->thread_input_number == thread){
            return current->index;
        }
        else{
            parent = current;
            if(current->thread_input_number > thread){
                current = current->left;
            }
            else{
                current = current->right;
            }
        }
    }
    binary_search_tree *new_node = (binary_search_tree*)malloc(sizeof(binary_search_tree));
    new_node->thread_input_number = thread;
    new_node->index = biggest_thread_index++;
    new_node->left = NULL;
    new_node->right = NULL;
    if(parent->thread_input_number > thread){
        parent->left = new_node;
    }
    else{
        parent->right = new_node;
    }
    return new_node->index;
}

typedef struct MutexNode{
    struct MutexNode *left;
    struct MutexNode *right;
    unsigned long long mutex_input_number;
    int index;
} mutex_binary_search_tree;

mutex_binary_search_tree *head = NULL;
int biggest_mutex_index = 0;

int get_mutex_index(unsigned long long mutex){
    if(head == NULL){
        head = (mutex_binary_search_tree*)malloc(sizeof(mutex_binary_search_tree));
        head->mutex_input_number = mutex;
        head->index = biggest_mutex_index++;
        head->left = NULL;
        head->right = NULL;
        return head->index;
    }
    mutex_binary_search_tree *current = head;
    mutex_binary_search_tree *parent = NULL;
    while(current != NULL){
        if(current->mutex_input_number == mutex){
            return current->index;
        }
        else{
            parent = current;
            if(current->mutex_input_number > mutex){
                current = current->left;
            }
            else{
                current = current->right;
            }
        }
    }
    mutex_binary_search_tree *new_node = (mutex_binary_search_tree*)malloc(sizeof(mutex_binary_search_tree));
    new_node->mutex_input_number = mutex;
    new_node->index = biggest_mutex_index++;
    new_node->left = NULL;
    new_node->right = NULL;
    if(parent->mutex_input_number > mutex){
        parent->left = new_node;
    }
    else{
        parent->right = new_node;
    }
    return new_node->index;
}

void check_deadlock(int start_thread){
    int current_thread = start_thread;
    while(true){
        int mutex = thread_waits_for[current_thread];
        if(mutex == -1){
            return;
        }
        int next_thread = mutex_held_by[mutex];
        if(next_thread == -1){
            return;
        }
        if(next_thread == start_thread){
            int trace_t = start_thread;
            do {
                int trace_m = thread_waits_for[trace_t];
                int nxt_t = mutex_held_by[trace_m];
                
                printf(" -> Thread %llu asteapta Mutex 0x%llx (tinut de Thread %llu)\n", 
                       thread_keys[trace_t], mutex_keys[trace_m], thread_keys[nxt_t]);
                
                trace_t = nxt_t;
            } while(trace_t != start_thread);
            exit(1);
        }
        current_thread = next_thread;
    }
}   

int main(int argc, char *argv[]){
    if(argc < 2){
        return 1;
    }
    FILE *file = fopen(argv[1], "r");
    if(file == NULL){
        return 1;
    }

    for(int index = 0; index < MAX_THREADS; index++){
        thread_waits_for[index] = -1;
    }
    for(int index = 0; index < MAX_MUTEXES; index++){
        mutex_held_by[index] = -1;
    }

    char line[256];
    int current_line = 0;
    while(fgets(line, sizeof(line), file)){
        unsigned long long thread;
        unsigned long long mutex;
        char action[32];
        
        if(sscanf(line, "%llu %31s %llx", &thread, action, &mutex) == 3){
            current_line += 1;
            
            int thread_index = get_thread_index(thread);
            int mutex_index = get_mutex_index(mutex);
            
            thread_keys[thread_index] = thread;
            mutex_keys[mutex_index] = mutex;
            
            if(strcmp(action, "REQUEST") == 0){
                thread_waits_for[thread_index] = mutex_index;
                check_deadlock(thread_index);
            }
            else if(strcmp(action, "ACQUIRE") == 0){
                thread_waits_for[thread_index] = -1;
                mutex_held_by[mutex_index] = thread_index;
            }
            else if(strcmp(action, "RELEASE") == 0){
                mutex_held_by[mutex_index] = -1;
            }
        }
    }
    
    fclose(file);
    return 0;
}
