#include "worker.h"
#include "stats.h"
#include <zlib.h>

extern  int if_sleep;
extern  void cleanup(int sig);
extern void trace(char *msg);
//response a failure to dbclient
void resp_fail(int sock , struct request *rq, char * fmt, ...) {
    inc_fails();

    if (fmt != NULL) {
        va_list ap;
        va_start(ap, fmt);
        // Print "ERROR: " prefix so failures are clearly visible
        fprintf(stderr, "ERROR: ");
        vfprintf(stderr, fmt, ap);
        va_end(ap);             // FIXED: va_end now correctly after vfprintf
    }
    rq->op_status = 'X';
    write(sock , rq , sizeof(*rq));
}
//response a sucess to db clinet
void resp_okay(int sock, struct request *rq, int len, void*data) {
    rq->op_status = 'K';
    sprintf(rq->len, "%d",len);
    write(sock, rq, sizeof(*rq));
    if (len > 0) {
            write(sock , data , len);
    }

}

//handle writes
void handle_writes(int sock, struct request *rq, kvstore_t *kv) {
    inc_writes();
     int n, bytes = 0, len = atoi(rq->len);
     char buf[len + 1];
    for (void*ptr = buf, *max = ptr+len; ptr < max;) {
         n = read(sock,ptr, (char*)max -(char*)ptr);
        if (n < 0) {
            resp_fail(sock, rq, "write(%s): receive: %s\n", rq->name, strerror(errno));
            return;
        }
        if (n == 0) {
            // FIXED: removed extra args len, bytes
            resp_fail(sock, rq, "write(%s): connection closed\n", rq->name);
            return;
        }
        ptr = (char *) ptr + n;
        bytes += n;
    }
    buf[len] = '\0';
    char msg[1024];
    sprintf(msg, "W %s = %d %d", rq->name, bytes, (int)crc32(0, (unsigned char *)buf, bytes));
    trace(msg);
    int ret  = kv_write(kv, rq->name, buf);
    if (ret == 0) {
        resp_okay(sock, rq, 0 , NULL);
    }else {
        resp_fail(sock, rq, "write(%s, %d):KV-store full\n", rq->name, len);
    }


}
//handles read
void handle_read(int sock, struct request *rq,  kvstore_t *kv) {
    inc_reads();
    char *key = rq->name;
    char*ret = kv_read(kv, key);
    if (ret == NULL) {
        resp_fail(sock, rq, "not found: %S\n", rq->name);
    }
    else {
        int size = strlen(ret);
        resp_okay(sock, rq, size, ret);
    }

}
void  handle_delete(int sock, struct request *rq, kvstore_t *kv) {
    inc_deletes();
    char *key = rq->name;
    kv_delete(kv, key);
    char msg[1024];
    sprintf(msg, "D %s \n", rq->name);
    trace(msg);
    resp_okay(sock, rq, 0 , NULL);

}
void dispatch(int sock, kvstore_t *kv) {
    struct request rq;
    int n = read(sock, &rq, sizeof(rq));
    if (n != sizeof(rq)) {
        resp_fail(sock, &rq, "read failed %d : %s\n", n, strerror(errno));
    }
    else {
        usleep((random() % if_sleep) * 1000);
        if (rq.op_status == 'W') {
            handle_writes(sock, &rq, kv);
        }
        else if (rq.op_status == 'R') {
            handle_read(sock, &rq, kv);
        }
        else if (rq.op_status == 'D') {
            handle_delete(sock, &rq, kv);
        }
        else if (rq.op_status == 'Q') {
           cleanup(0);
        }
        else {
            resp_fail(sock , &rq, "unknown op: %c\n", rq.op_status);
        }
    }
    close(sock);
}
void *worker(void *args) {
    queue_t *wq = ((args_t *)args)->q;
    kvstore_t *kv =((args_t *)args)->kv;
    int first_null  = 0;
    for (;;) {
        task_t *w = dequeue(wq);
        if (w != NULL ) {
            dispatch(w->fd, kv);
            free(w);
        }else {
            if (first_null == 0) {
                printf("[INFO] worker thread  dequeue null....\n");
                first_null = 1;
            }
            sleep(1);
        }

    }
}