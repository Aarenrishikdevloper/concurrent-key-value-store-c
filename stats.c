#include "stats.h"
int n_writes = 0;
int n_deletes = 0;
int n_fails = 0;
int n_reads = 0;

static pthread_mutex_t stats_mutex = PTHREAD_MUTEX_INITIALIZER;
void inc_writes() {
    pthread_mutex_lock(&stats_mutex);
    n_writes++;
    pthread_mutex_unlock(&stats_mutex);

}

void inc_deletes() {
    pthread_mutex_lock(&stats_mutex);
    n_deletes++;
    pthread_mutex_unlock(&stats_mutex);
}

void inc_fails() {
    pthread_mutex_lock(&stats_mutex);
    n_fails++;
    pthread_mutex_unlock(&stats_mutex);
}
void inc_reads() {
    pthread_mutex_lock(&stats_mutex);
    n_reads++;
    pthread_mutex_unlock(&stats_mutex);
}

int get_writes() {
    pthread_mutex_lock(&stats_mutex);
    int val = n_writes;
    pthread_mutex_unlock(&stats_mutex);
    return val;
}
int get_deletes() {
    pthread_mutex_lock(&stats_mutex);
    int val = n_deletes;
    pthread_mutex_unlock(&stats_mutex);
    return val;
}
int get_fails() {
    pthread_mutex_lock(&stats_mutex);
    int val = n_fails;
    pthread_mutex_unlock(&stats_mutex);
}

int get_reads() {
    pthread_mutex_lock(&stats_mutex);
    int val = n_reads;
    pthread_mutex_unlock(&stats_mutex);
    return val;
}