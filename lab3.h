//
// Created by Rishik Kashyap on 02-05-2026.
//

#ifndef  __LAB3_H__
#define __LAB3_H__
#include <stdio.h>
#include <stdlib.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <fcntl.h>
#include <signal.h>
#include <stdarg.h>
#include <dirent.h>
#include <zlib.h>

struct request {
    char op_status;
    char name[31];
    char len[8];
};
#define  TABLE_MAX 200

#endif
