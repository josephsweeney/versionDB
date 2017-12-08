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

char *recv_request(int sid);

RequestInfo* request_info_get(char *request);
void         request_info_destroy(RequestInfo* info);

// adjusts req to point after prefix
int eat_prefix(char** req, char *prefix); 
// returns the char* after the first char c
char* get_past(char* req, char c);
// returns a copy from req to char c not including c
// if check_end is not 0, we will also copy to the 0 byte instead of c
char* copy_to(char* req, char c, int check_end);

#endif
