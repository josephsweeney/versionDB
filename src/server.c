#include <sys/socket.h>
#include <sys/types.h>
#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <sys/un.h>
#include "data.h"
#include "server.h"

// This globals is to unlink and close the socket when interrupted
int global_sid = -1;
char * global_host = NULL;

void error(int status, char* file, int line)
{
  if (status < 0) {
    printf("error in file %s at line %d: [%s]\n", file, line, strerror(errno));
    exit(-1);
  }
}

void int_handler(int sig) {
  if(global_sid != -1)
    close(global_sid);
  if(global_host != NULL)
    unlink(global_host);

  char *output = "\nStopped the server!\n";
  int len = strlen(output);
  write(1, output, len);
  exit(0);
}

int main(int argc, char *argv[]) {
  int st; // Status for all the calls
  signal(SIGINT, int_handler);
  if(argc < 3) {
    printf("Please give number of workers, then name of host");
    exit(0);
  }
  int num_threads = atoi(argv[1]);
  char *host = argv[2];
  int sd = socket(PF_LOCAL, SOCK_STREAM, 0);
  error(sd, __FILE__, __LINE__);
  global_sid = sd;
  
  struct sockaddr_un addr;
  memset(&addr, 0, sizeof(addr));
  addr.sun_family = PF_LOCAL;
  strncpy(addr.sun_path, host, sizeof(addr.sun_path)-1);
  st = bind(sd, (struct sockaddr*)&addr, sizeof(addr));
  error(st, __FILE__, __LINE__);
  global_host = host;

  st = listen(sd, 20);
  error(st, __FILE__, __LINE__);

  pthread_t tids[num_threads];
  
  for(int i = 0; i<num_threads; i++) {
    pthread_create(&tids[i], NULL, accept_loop, (void *)&sd);
  }

  void *val;
  for(int i = 0; i<num_threads; i++) {
    pthread_join(tids[i], &val);
  }

  close(sd);
  unlink(host);
  
  return 0;
}


void *accept_loop(void* in) {
  int listen_socket = *(int*)in;
  int sid;
  struct sockaddr_un client;
  socklen_t client_sz;
  while(1) {
    sid = accept(listen_socket, (struct sockaddr*)&client, &client_sz);
    RequestInfo *info = request_info_get(sid);
    if(info == NULL) {
      // Bad Request,
      // TODO: Send back an error
      continue; 
    }
    switch(info->type) {
      case WRITE:
	// write data
	printf("Write\n");
	break;
      case READ:
	//read data
	printf("Read\n");
	break;
      case READ_TIME:
	// read data at time
	printf("Read at time\n");
	break;
      default:
	printf("Unknown type given");
    }
    request_info_destroy(info);
  }
}


RequestInfo* request_info_get(int sid) {
  RequestInfo *info = (RequestInfo*)malloc(sizeof(RequestInfo));
  
  int bytes = 0;
  RequestType type;
  int type_sz = sizeof(type);
  while(bytes < type_sz) {
    bytes += recv(sid, (&type)+bytes, type_sz-bytes, 0);
  }

  char *id = id_get(sid);
  if(id == NULL) {
    return NULL;
  }

  u64 write_len = 0;
  if(type == WRITE) {
    int write_sz = sizeof(write_len);
    bytes = 0;
    while(bytes < write_sz) {
      bytes += recv(sid, (&write_len)+bytes, write_sz-bytes, 0);
    }
  }

  u64 time = 0;
  if(type == READ_TIME) {
    int time_sz = sizeof(time);
    bytes = 0;
    while(bytes < time_sz) {
      bytes += recv(sid, (&time)+bytes, time_sz-bytes, 0);
    }
  }

  info->type = type;
  info->id = id;
  info->write_len = write_len;
  info->time = time;

  return info;
}

void request_info_destroy(RequestInfo *info) {
  free(info->id);
  free(info);
}

char *id_get(int sid) {
  int initial = 20;
  int sz = initial;
  char *id = (char*)malloc(sz*sizeof(char));
  id[0] = 'a'; // init for while loop check

  int bytes = 0;
  while(*(id+bytes) != '\0') {
    bytes += recv(sid, id + bytes, 1, 0);
    if(bytes == sz) {
      id = realloc(id, (sz + initial)*sizeof(char));
    }
    if(bytes > 1024) {
      // Calling that id too big
      free(id);
      printf("Someone tried to give me a big id\n");
      return NULL;
    }
  }

  return id;
}
