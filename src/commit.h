#ifndef COMMIT_H
#define COMMIT_H

/*************************** HEADER FILES ***************************/
#include <stddef.h>
#include "sha1.h"

/****************************** MACROS ******************************/

/**************************** DATA TYPES ****************************/

typedef unsigned long long u64;
typedef struct {
  BYTE data[SHA1_BLOCK_SIZE];
  BYTE parent[SHA1_BLOCK_SIZE];
  u64 time;
  int has_parent;
} Commit;

/*********************** FUNCTION DECLARATIONS **********************/
void commit_init(Commit *commit, BYTE data[], BYTE parent[]);

#endif
