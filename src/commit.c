#include <sys/time.h>
#include <string.h>
#include <stdio.h>
#include "commit.h"

u64 get_time() {
  struct timeval tp;
  gettimeofday(&tp, NULL);
  u64 ms = tp.tv_sec * 1000 + tp.tv_usec / 1000;
  printf("ms: %llu\n", ms);
  return ms;
}

void commit_init(Commit *commit, BYTE data[], BYTE parent[]) {
  commit->has_parent = 0;

  memcpy(commit->data,   data,   SHA1_BLOCK_SIZE);
  if(parent != NULL) {
    memcpy(commit->parent, parent, SHA1_BLOCK_SIZE);
    commit->has_parent = 1;
  } 

  u64 ms = get_time();

  commit->time = ms;
}
