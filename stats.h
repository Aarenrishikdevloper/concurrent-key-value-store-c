//
// Created by Rishik Kashyap on 03-05-2026.
//

#ifndef KEY_VALUE_STORE_STATS_H
#define KEY_VALUE_STORE_STATS_H
#include "lab3.h"
void inc_writes();
void inc_reads();
void inc_deletes();
void inc_fails();
int get_writes();
int get_reads();
int get_deletes();
int get_fails();
#endif //KEY_VALUE_STORE_STATS_H
