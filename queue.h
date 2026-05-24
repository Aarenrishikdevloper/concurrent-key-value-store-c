//
// Created by Rishik Kashyap on 03-05-2026.
//

#ifndef KEY_VALUE_STORE_QUEUE_H
#define KEY_VALUE_STORE_QUEUE_H
#include "lab3.h"
typedef struct  task task_t;
struct task {
    task_t *next;
    int fd;
};
typedef struct queue {
  task_t *head;
  task_t *tail;
    int count;

}queue_t;
//add a new task to the nd of the queuue
void enequeue(queue_t *q, task_t *t );
//fetch  a task from the head  of the queue
task_t * dequeue(queue_t *q);
//get number of tasks in the queue
int queue_count(queue_t *q);


#endif //KEY_VALUE_STORE_QUEUE_H
