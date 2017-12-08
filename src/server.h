#ifndef SERVER_H
#define SERVER_H

/*************************** HEADER FILES ***************************/
#include <stddef.h>
#include "commit.h" // For the u64 type

/****************************** MACROS ******************************/

/**************************** DATA TYPES ****************************/

typedef enum {
  WRITE=0,
  READ=1,
  READ_TIME=2
} RequestType;

typedef struct {
  RequestType type;
  char *id;
  u64 write_len;
  u64 time;
} RequestInfo;


/*********************** FUNCTION DECLARATIONS **********************/

void* accept_loop(void* in);


RequestInfo* request_info_get(int sid);
void         request_info_destroy(RequestInfo* info);

char* id_get(int sid);

int eat_prefix(int sid, char *prefix);

#endif
