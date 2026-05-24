#include "lab3.h"
#include "worker.h"
#include "stats.h"
#include "queue.h"
#include "kvstore.h"

int listen_sock;
int port = 5600;

queue_t wq;
kvstore_t kv;
int debug = 0;
FILE *trace_fp;
pthread_mutex_t trm = PTHREAD_MUTEX_INITIALIZER;
int if_sleep = 1;

void cleanup(int sig) {
    int ret = close(listen_sock);
    if (ret != 0) {
        perror("close socket failure");
        exit(1);
    }
    if (trace_fp != NULL) {
        fclose(trace_fp);
    }
    exit(0);
}

void init(void) {
    if (signal(SIGINT, cleanup) == SIG_ERR) {
        printf("can't catch SIGINT\n");
    }
    for (int i = 0; i < TABLE_MAX; i++) {
        kv.keys[i].stat = 0;
    }
    if (debug) {
        trace_fp = fopen("./dbserver.trace", "w");
    }
}

void traces(char *msg) {
    pthread_mutex_lock(&trm);
    fprintf(trace_fp, "%s\n", msg);
    pthread_mutex_unlock(&trm);
}

void trace(char *msg) {
    if (!debug) { return; }
    traces(msg);
}

void *listen_thread(void *ignore) {
    listen_sock = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr = {
        .sin_family = AF_INET,
        .sin_port = htons(port),
        .sin_addr.s_addr = 0
    };
    if (bind(listen_sock, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind error");
        exit(1);
    }
    if (listen(listen_sock, 10) < 0) {   // fixed: backlog 2 -> 10
        perror("listen");
        exit(1);
    }
    while (1) {
        int fd = accept(listen_sock, NULL, NULL);
        task_t *newtask = (task_t *)malloc(sizeof(task_t));
        newtask->fd = fd;
        enequeue(&wq, newtask);
    }
}

int main(int argc, char **argv) {
    init();

    // FIXED: parse port BEFORE starting listener
    if (argc == 2) {
        port = atoi(argv[1]);
    }

    int num_thread = 4;
    char *str_nhtr = getenv("LAB3_NUM_THREAD");
    if (str_nhtr != NULL) {
        num_thread = atoi(str_nhtr);
        printf("[INFO] num_thread = %d\n", num_thread);
    }
    char *str_ifsleep = getenv("LAB3_if_sleep");
    if (str_ifsleep != NULL) {
        if_sleep = atoi(str_ifsleep);
        printf("[INFO] if_sleep = %d\n", if_sleep);
    }

    args_t args;
    args.q = &wq;
    args.kv = &kv;
    // FIXED: removed (void) args

    pthread_t listener;
    pthread_create(&listener, NULL, listen_thread, NULL);

    pthread_t workers[num_thread];
    for (int i = 0; i < num_thread; i++) {
        // FIXED: pass &args instead of NULL
        pthread_create(&workers[i], NULL, worker, &args);
    }

    for (;;) {
        char line[80], cmd[8];
        if (fgets(line, sizeof(line), stdin) == NULL) {
            exit(0);
        }
        int n = sscanf(line, "%7s", cmd);
        if (n < 1) continue;

        if (strcmp(cmd, "quit") == 0) {
            printf("quitting....\n");
            cleanup(0);
        } else if (strcmp(cmd, "stats") == 0) {
            printf("queued: %d\n", queue_count(&wq));
            printf("writes: %d\n", get_writes());
            printf("reads: %d\n", get_reads());
            printf("deletes: %d\n", get_deletes());
            printf("failure count: %d\n", get_fails());
        } else if (strcmp(cmd, "list") == 0) {
            kv_dump(&kv);
        } else if (strcmp(cmd, "help") == 0) {
            printf("stats: display the status of the dbserver\n");
            printf("list: list all existing kv-pairs in dbserver\n");
            printf("quit: terminate dbserver\n");
        } else {
            printf("unknown command %s\n", cmd);
        }
    }
}