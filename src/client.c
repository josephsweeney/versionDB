#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/un.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "client.h"


int main(int argc, char* argv[]) {
  struct sockaddr_un server;

  if(argc < 2) {
    printf("Please give path for socket\n");
    exit(-1);
  }

  char *path = argv[1];

  int sid = socket(AF_UNIX, SOCK_STREAM, 0);

  server.sun_family = AF_UNIX;
  strcpy(server.sun_path, path);

  if (connect(sid, (struct sockaddr *) &server, sizeof(struct sockaddr_un)) < 0) {
    close(sid);
    printf("error connecting stream socket\n");
    exit(-1);
  }

  // TEST
  /* send_read(sid, "fun_data"); */
  /* printf("Sent read\n"); */

  /* send_write(sid, "more_fun_stuff", "wazzup", 6); */
  /* printf("Sent write\n"); */

  /* send_read_time(sid, "fun_data", 1356048000000); */
  /* printf("Sent read_time\n"); */

  if(argc > 2) {
    send_request(sid, argv[2]);
  }
  
  close(sid);
  
  return 0;
}

void send_request(int sid, char *req) {
  int len = strlen(req) + 1;
  int bytes = 0;
  while(bytes < len) {
    bytes += send(sid, req + bytes, len - bytes, 0);
  }
}

void send_write(int sid, char* id, char *buf, int len) {

  int id_len = strlen(id);
  char *request = (char*)malloc(32+id_len+MAX_U64_LEN);
  sprintf(request, "TYPE:WRITE,ID:%s,BYTES:%d", id, len);
  
  send_request(sid, request);

  free(request);

}

void send_read(int sid, char* id) {

  int id_len = strlen(id);
  char *request = (char*)malloc(32+id_len);
  sprintf(request, "TYPE:READ,ID:%s", id);

  send_request(sid, request);

  free(request);

}

void send_read_time(int sid, char* id, u64 time) {

  int id_len = strlen(id);
  char *request = (char*)malloc(32+id_len+MAX_U64_LEN);
  sprintf(request, "TYPE:READ_TIME,ID:%s,TIME:%llu", id, time);
  
  send_request(sid, request);

  free(request);

}


void send_type(int sid, char *type) {
  char *prefix = "TYPE:";
  int bytes = 0;
  int pre_sz = strlen(type);
  while(bytes < pre_sz) {
    bytes += send(sid, prefix + bytes, pre_sz - bytes, 0);
  }
  bytes = 0;
  int type_sz = strlen(type);
  while(bytes < type_sz) {
    bytes += send(sid, type + bytes, type_sz - bytes, 0);
  }

}

void send_char(int sid, char c) {
  int bytes = send(sid, &c, 1, 0);

  if(bytes == -1) {
    printf("failed to send %c\n", c);
    exit(-1);
  }
}


void send_id(int sid, char *id) {
  char *prefix = "ID:";
  int bytes = 0;
  int pre_sz = strlen(prefix);
  while(bytes < pre_sz) {
    bytes += send(sid, prefix + bytes, pre_sz - bytes, 0);
  }
  bytes = 0;
  int sz = strlen(id);
  while(bytes < sz) {
    bytes += send(sid, id+bytes, sz-bytes, 0);
  }

}
