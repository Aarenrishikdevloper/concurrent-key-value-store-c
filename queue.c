//
// Created by Rishik Kashyap on 03-05-2026.
//
#include "queue.h"
//Monitor: mutex and conditional variable  for the queue
static pthread_mutex_t queue_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t queue_cond = PTHREAD_COND_INITIALIZER;
void enequeue(queue_t *q, task_t *t) {
    pthread_mutex_lock(&queue_mutex);
    t->next = NULL;
    if (q->tail == NULL) {
        //empty queue
        q->head = t;
        q->tail = t;
    }else {
        q->tail->next = t;
        q->tail = t;
    }
    q->count++;
    pthread_cond_signal(&queue_cond);
    pthread_mutex_unlock(&queue_mutex);
}
task_t *dequeue(queue_t *q) {
    //Monitor entry - acquire lock
    pthread_mutex_lock(&queue_mutex);
    //wait while the queue is empty
    while (q->head == NULL) {
        pthread_cond_wait(&queue_cond, &queue_mutex);
    }
    //deque from head
    task_t* task = q->head;
    q->head = task->next;
    if (q->head == NULL) {
        //queue is not empty
        q->tail = NULL;
    }
    q->count--;
    pthread_mutex_unlock(&queue_mutex);
    return task;


}

//return the number of tasks ihn the queue
int queue_count(queue_t *q) {
    pthread_mutex_lock(&queue_mutex);
    int count = q->count;
    pthread_mutex_unlock(&queue_mutex);
    return count;
}
