//
// Created by Rishik Kashyap on 03-05-2026.
//
#include "kvstore.h"
//Mutex to protect key value-store
static  pthread_mutex_t kvstore_mutx = PTHREAD_MUTEX_INITIALIZER;
char*kv_read(kvstore_t *kv, char *key) {
    pthread_mutex_lock(&kvstore_mutx);
    char*result = NULL;
    for (int i=0; i < TABLE_MAX; i++) {
        if (kv->keys[i].stat == 1 && strcmp(kv->keys[i].key, key) == 0) {
            //key found
            result =kv->value[i];
            break;
        }
    }
    pthread_mutex_unlock(&kvstore_mutx);
    return result;

}
int kv_write(kvstore_t *kv, char *key, char *value) {
    //Monitor entry - acquire lock
    pthread_mutex_lock(&kvstore_mutx);
    int result =0;
    int key_index = -1;
    int free_index = -1;
    //seach for existing key or fina free slot
    for (int i=0; i < TABLE_MAX; i++) {
        if (kv->keys[i].stat == 1 && strcmp(kv->keys[i].key, key) == 0) {
            key_index = i;
            break;

        }
        if (kv->keys[i].stat == 0 && free_index == -1) {
            free_index = i;
        }
    }
    if (key_index != -1) {
        //key exist overwrite the value
        //free old value
        free(kv->value[key_index]);
        //Allocate and copy new value
        int val_len = strlen(value);
        kv->value[key_index] = (char*)malloc(val_len + 1);
        strcpy(kv->value[key_index], value);
        result = 0;
    }
    else if (free_index != -1) {
        //keys does not exist insert new kv pair
        kv->keys[free_index].stat = 1;
        strcpy(kv->keys[free_index].key, key);
        int val_len = strlen(value);
        kv->value[free_index]  = (char*)malloc(val_len + 1);
        strcpy(kv->value[free_index], value);
        result = 0;

    }else {
        result = -1;
    }
    //Monitor exist - realeas lock
    pthread_mutex_unlock(&kvstore_mutx);
    return result;
}
//delete a kv pair from the kv store
   void kv_delete(kvstore_t *kv, char *key) {
    //monitor entry - accquire lock
    pthread_mutex_lock(&kvstore_mutx);
    for (int i=0; i < TABLE_MAX; i++) {
        if (kv->keys[i].stat == 1 && strcmp(kv->keys[i].key, key) == 0) {
            kv->keys[i].stat = 0;
            free(kv->value[i]);
            kv->value[i] = NULL;
            break;
        }
    }
    pthread_mutex_unlock(&kvstore_mutx);
}
//print kv-store content using stdout
void kv_dump(kvstore_t *kv) {
    pthread_mutex_lock(&kvstore_mutx);
    printf("=== Key Values store  dump ===\n ");
    int count = 0;
    for (int i=0; i < TABLE_MAX; i++) {
        if (kv->keys[i].stat == 1) {
            printf("[%d] key=\"%s\", value=\"%s\"\n", i, kv->keys[i].key, kv->value[i]);
            count++;
        }
    }
    printf("Total entities: %d\n",count);
    printf("=================\n");
}

