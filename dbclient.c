#include <stdio.h>
#include <stdlib.h>
#include <stdlib.h>
#include <unistd.h>
#include  <errno.h>
#include  <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <zlib.h>
#include <pthread.h>
#include <argp.h>
#include <assert.h>
#include  "lab3.h"
static struct argp_option options[] = {
    {"set", 'S', "KEY", 0 , "set Key to Value"},
    {"get", 'G', "KEY", 0 , "get Key to Value"},
    {"delete", 'D', "KEY", 0 , "delete Key to Value"},
    {"quit", 'Q', "KEY", 0 , "quit Key to Value"},

};
enum {OP_SET = 1, OP_GET =2, OP_DELETE = 3, OP_QUIT = 4};
struct args {
    int nthreads;
    int count;
    int port;
    int max;
    int op;
    char *key;
    char *val;
    char *logfile;
    FILE *logfp;
    struct sockaddr_in addr;
};
static  error_t parse_opt(int key, char *arg, struct argp_state *state) {
    struct args *a = state->input;
    switch (key) {
        case ARGP_KEY_INIT:
            a->nthreads = 1;
            a->count = 1000;
            a->port = 5600;
            a->max = 200;
            a->logfp = NULL;
            break;
        case 'l':
            a->logfile = arg;
            if ((a->logfp = fopen(arg, "w")) == NULL) {
                fprintf(stderr, "%s: can't open logfile %s\n",  arg, strerror(errno));
                exit(1);
            }
            break;
        case 'q':
            a->op = OP_QUIT;
            break;
        case 'G':
            a->op = OP_GET;
            if (strlen(arg) > 30)
                printf("key must be less than 30 characters\n"), argp_usage(state);
            a->key = arg;
            break;
        case 'S':
            a->op = OP_SET;
            if (strlen(arg) > 30)
                printf("key must be less than 30 characters\n"), argp_usage(state);
            a->key = arg;
            break;
        case 'D':
            a->op = OP_DELETE;
            if (strlen(arg) > 30)
                printf("key must be less than 30 characters\n"), argp_usage(state);
            a->key = arg;
            break;
        case 'n':
            a->count = atoi(arg);
            break;
        case 'p':
            a->port = atoi(arg);
            break;
        case ARGP_KEY_ARG:
            if (state->arg_num  == 0 && a->op == OP_SET) {
                a->val = arg;
                printf("val set to %s\n", arg);
            }else {
                argp_usage(state);
                break;
            }
    }
    return 0;
}
    static struct argp argp = {options, parse_opt,NULL, NULL};
    int do_coonect(struct sockaddr_in *addr) {
        int sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock < 0 || connect(sock, (struct sockaddr *) addr, sizeof(*addr)) < 0) {
            fprintf(stderr, " can't connect to socket:%s\n",strerror(errno) );
            exit(0);
        }
        return sock;
    }
    void do_del(struct args *args, char *name, char*result, int quiet) {
        int sock = do_coonect(&args->addr);
        struct request rq;
        snprintf(rq.name, sizeof(rq.name), "%s", name);
        rq.op_status = 'D';
        int val = write(sock, &rq, sizeof(rq));
        if ((val = read(sock, &rq, sizeof(rq))) < 0) {
            printf("DEL: REPLY: READ ERROR: %s\n", strerror(errno));
        }
        else if (val< sizeof(rq)) {
            printf("DEL: REPLY: SHORT READ ERROR: %s\n", val);
        }
        else if ( rq.op_status != 'K' && !quiet) {
            printf("DEL:FAILED (%c)\n", rq.op_status);
        }else if (!quiet) {
            printf("DEL:OK\n");
        }
        if (result != NULL) {
            *result = rq.op_status;
        }
        close(sock);



    }
void do_set(struct args *args, char *name, void *data, int len,  char*result, int quiet) {
        int sock = do_coonect(&args->addr);
        struct request rq;
        snprintf(rq.name, sizeof(rq.name), "%s", name);
        int val;
        rq.op_status = 'W';
        sprintf(rq.len , "%d", len);
         write(sock, &rq, sizeof(rq));
         write(sock,data, len);
        if ((val = read(sock, &rq, sizeof(rq))) < 0) {
            printf("Write: REPLY: READ ERROR: %s\n", strerror(errno));
        }
        else if (val< sizeof(rq)) {
            printf("Write: REPLY: SHORT READ ERROR: %s\n", val);
        }
        else if ( rq.op_status != 'K' && !quiet) {
            printf("Write:FAILED (%c)\n", rq.op_status);
        }else if (!quiet) {
            printf("Write:OK\n");
        }
        if (result != NULL) {
            *result = rq.op_status;
        }
        close(sock);

    }
void do_quit(struct args *args ) {
        int sock = do_coonect(&args->addr);
        struct request rq;
        rq.op_status = 'Q';
        write(sock, &rq, sizeof(rq));
    }



void do_get(struct args *args, char *name, void *data, int *len_p,  char*result) {
        int sock = do_coonect(&args->addr);
        struct request rq;
        snprintf(rq.name, sizeof(rq.name), "%s", name);
        int val;
        rq.op_status = 'R';
        sprintf(rq.len , "%d", 0);
        write(sock, &rq, sizeof(rq));
        if ((val = read(sock, &rq, sizeof(rq))) < 0) {
            printf("Read: REPLY: READ ERROR: %s\n", strerror(errno));
        }
        else if (val< sizeof(rq)) {
            printf("Read: REPLY: SHORT READ ERROR: %s\n", val);
        }
        else if ( rq.op_status != 'K') {
            printf("Read:FAILED (%c)\n", rq.op_status);
        }
       else {
           int len = atoi(rq.len);
           char buf[len];
           for (void*ptr = buf, *max = ptr + len; ptr < max;) {
               int n = read(sock, ptr,max-ptr);
                if (n < 0) {
                    printf("Read: REPLY: READ ERROR: %s\n", strerror(errno));
                    break;
                }
               ptr += n;
           }
           if (data != NULL) {
               memcpy(data,buf,len);
               *len_p = len;
           }  else {
               printf("\"%.*s\"\n", len, buf);
           }
       }
        if (result != NULL) {
            *result = rq.op_status;
        }
        close(sock);


    }
int main(int argc, char **argv) {
        struct args args;
        memset(&args, 0, sizeof(args));
        argp_parse(&argp, argc, argv,0,0, &args);
        args.addr = (struct sockaddr_in){
            .sin_family = AF_INET,
            .sin_port = htons(args.port),
            .sin_addr.s_addr = inet_addr("127.0.0.1"),
        };
        if (args.op == OP_SET)
            do_set(&args, args.key, args.val, strlen(args.val), NULL, 0);
        else if (args.op == OP_GET)
            do_get(&args, args.key, NULL,NULL,NULL);
        else if (args.op == OP_QUIT)
            do_quit(&args);
        else if (args.op == OP_DELETE)
            do_del(&args, args.key, NULL,0);
        else
            printf("Unknown operation %d\n", args.op);
    }

