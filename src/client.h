#include <stddef.h>

#define MAX_U64_LEN 20

typedef unsigned long long u64;

void send_request(int sid, char *req);

void send_write(int sid, char* id, char *buf, int len);
void send_read(int sid, char* id);
void send_read_time(int sid, char* id, u64 time);


