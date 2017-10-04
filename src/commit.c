#include <sys/time.h>
#include <string.h>
#include <stdio.h>
#include "commit.h"

void commit_init(Commit *commit, BYTE data[], BYTE parent[]) {
  memcpy(commit->data,   data,   SHA1_BLOCK_SIZE);
  if(parent != NULL) {
    memcpy(commit->parent, parent, SHA1_BLOCK_SIZE);
  } else {
    commit->parent = NULL;
  }

  struct timeval tp;
  gettimeofday(&tp, NULL);
  u64 ms = tp.tv_sec * 1000 + tp.tv_usec / 1000;
  printf("ms: %llu\n", ms);
  
  commit->time = ms;
}