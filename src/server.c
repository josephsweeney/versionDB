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
  write(1, output, strlen(output));
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
  char delimiter = ',';

  /* TYPE */

  if(eat_prefix(sid, "TYPE:") == -1) {
    printf("prefix wasn't correct\n");
    return NULL;
  }
  RequestType type;
  
  int bytes = 0;
  int max_len = 16;
  char type_str[max_len];
  int type_sz = sizeof(type);
  while(type_str[bytes] != delimiter) {
    bytes += recv(sid, type_str+bytes, 1, 0);
    if(bytes == max_len) {
      // BAD REQUEST
      
      return NULL;
    }
  }
  type_str[bytes] = '\0';

  if(strcmp(type_str, "WRITE") == 0) {
    type = WRITE;
  } else if(strcmp(type_str, "READ") == 0) {
    type = READ;
  } else if(strcmp(type_str, "READ_TIME") == 0) {
    type = READ_TIME;
  } else {
    // UNSUPPORTED TYPE
    return NULL;
  }
  

  /* ID */
  
  char *id = id_get(sid);
  if(id == NULL) {
    return NULL;
  }

  /* WRITE_LEN */
  
  u64 write_len = 0;
  if(type == WRITE) {
    if(eat_prefix(sid, "BYTES:") == -1) {
      printf("prefix wasn't correct\n");
      free(id);
      return NULL;
    }
    
    bytes = 0;
    int size = 32;
    char *buf = (char*)malloc(sizeof(char)*size);
    while(buf[bytes] != '\0') {
      bytes += recv(sid, buf+bytes, 1, 0);
      if(buf[bytes] < '0' || buf[bytes] > '9') {
	free(buf);
	free(id);
	return NULL;
      }
      if(bytes == size) {
	size *= 2;
	buf = (char*)realloc(buf, sizeof(char)*size);
      }
    }
    write_len = (u64)strtoull(buf, (char**)NULL, 10);
    free(buf);
  }

  /* TIME */
  
  u64 time = 0;
  if(type == READ_TIME) {
    if(eat_prefix(sid, "TIME:") == -1) {
      printf("prefix wasn't correct\n");
      free(id);
      return NULL;
    }
    bytes = 0;
    int size = 32;
    char *buf = (char*)malloc(sizeof(char)*size);
    while(buf[bytes] != '\0') {
      bytes += recv(sid, buf+bytes, 1, 0);
      if(buf[bytes] < '0' || buf[bytes] > '9') {
	free(buf);
	free(id);
	return NULL;
      }
      if(bytes == size) {
	size *= 2;
	buf = (char*)realloc(buf, sizeof(char)*size);
      }
    }
    time = (u64)strtoull(buf, (char**)NULL, 10);
    free(buf);
  }

  RequestInfo *info = (RequestInfo*)malloc(sizeof(RequestInfo));

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
  if(eat_prefix(sid, "ID:") == -1) {
    printf("prefix wasn't correct\n");
    return NULL;
  }
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
    if(id[bytes] == ',') {
      // Hit the end of the id
      id[bytes] = '\0';
      break;
    }
  }

  return id;
}

int eat_prefix(int sid, char *prefix) {
  char buf[32];
  int pre_len = strlen(prefix);
  buf[0] = 'a';
  int bytes = 0;
  while(buf[bytes] != ':') {
    bytes += recv(sid, buf+bytes, 1, 0);
    if(bytes > pre_len || buf[bytes] != prefix[bytes]) {
      return -1;
    }
  }
  return 0;
}
