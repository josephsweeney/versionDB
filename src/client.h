#include <stddef.h>
#include "data.h"

#define MAX_U64_LEN 20




void readline(char* dest);

void send_request(int sid, char *req);

void send_write(int sid, char* id, char *buf, int len);
void send_write_from_input(int sid, char *id);
void send_read(int sid, char* id);
void send_read_time(int sid, char* id, u64 time);
void send_read_hash(int sid, char* hash);
void send_ls(int sid);
void send_history(int sid, char* id);


void print_result(int sid);
