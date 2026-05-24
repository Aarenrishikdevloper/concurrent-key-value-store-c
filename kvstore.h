//
// Created by Rishik Kashyap on 03-05-2026.
//

#ifndef KEY_VALUE_STORE_KVSTORE_H
#define KEY_VALUE_STORE_KVSTORE_H
#include  "lab3.h"
typedef  struct  key_entry {
    int stat;
    char key[32];
}key_entry_t;
typedef  struct kvstore {
    key_entry_t keys[TABLE_MAX];
    char*value[TABLE_MAX];

}kvstore_t;
//key value store api
//reading a value form a key value store
char*kv_read(kvstore_t*kv, char *key);
//write a key-value pair into the key value store
int kv_write(kvstore_t *kv, char *key, char *value);
//delete a key-value pair from the kv-store
void kv_delete(kvstore_t *kv, char *key);
//pritn all the kv contents to stdout
void kv_dump(kvstore_t*kv);
#endif //KEY_VALUE_STORE_KVSTORE_H
