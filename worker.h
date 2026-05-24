//
// Created by Rishik Kashyap on 06-05-2026.
//

#ifndef KEY_VALUE_STORE_WORKER_H
#define KEY_VALUE_STORE_WORKER_H
#include "lab3.h"
#include "queue.h"
#include "kvstore.h"
typedef  struct args {
    queue_t *q;
    kvstore_t *kv;
} args_t;
void *worker(void * args);
#endif //KEY_VALUE_STORE_WORKER_H
