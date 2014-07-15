#ifndef HASH_H
#define    HASH_H

#include "uv.h"

#define ENDIAN_BIG			0
#define ENDIAN_LITTLE		1

typedef uint32_t (*hash_func)(const void *key, size_t length);
hash_func hash;

enum hashfunc_type {
    JENKINS_HASH=0, MURMUR3_HASH
};

int hash_init(enum hashfunc_type type);

#endif    /* HASH_H */

