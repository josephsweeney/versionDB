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
    printf("Waiting\n");
    sid = accept(listen_socket, (struct sockaddr*)&client, &client_sz);
    char *request = recv_request(sid);
    if(request == NULL) {
      // Bad Request,
      // TODO: Send back an error
      continue; 
    }
    printf("%s\n", request);
    RequestInfo *info = request_info_get(request);
    if(info == NULL) {
      // Bad Request,
      // TODO: Send back an error
      continue; 
    }
    switch(info->type) {
      case WRITE:
	// write data
	printf("Write\n");
	printf("ID: %s\n", info->id);
	printf("LEN: %llu\n", info->write_len);
	break;
      case READ:
	//read data
	printf("Read\n");
	printf("ID: %s\n", info->id);
	break;
      case READ_TIME:
	// read data at time
	printf("Read at time\n");
	printf("ID: %s\n", info->id);
	printf("TIME: %llu\n", info->time);
	break;
      default:
	printf("Unknown type given");
    }
    request_info_destroy(info);
    close(sid);
    printf("Finished request\n");
  }
}


RequestInfo* request_info_get(char *req) {
  char delimiter = ',';

  char *pos = req;

  /* TYPE */

  if(eat_prefix(&pos, "TYPE:") == -1) {
    printf("prefix wasn't correct\n");
    return NULL;
  }
  RequestType type;

  if(strncmp(pos, "WRITE", 5) == 0) {
    type = WRITE;
  } else if(strncmp(pos, "READ_TIME", 9) == 0) {
    type = READ_TIME;
  } else if(strncmp(pos, "READ", 4) == 0) {
    type = READ;
  } else {
    // UNSUPPORTED TYPE
    printf("Request had unsupported type.\n%s\n", req);
    return NULL;
  }
  pos = get_past(pos, delimiter);
  

  /* ID */

  if(eat_prefix(&pos, "ID:") == -1) {
    printf("prefix wasn't correct\n");
    return NULL;
  }
  char *id = copy_to(pos, delimiter, 1);
  if(id == NULL) {
    printf("copy_to didn't work.\n%s\n", req);
    return NULL;
  }

  
  /* WRITE_LEN */
  
  u64 write_len = 0;
  if(type == WRITE) {
    pos = get_past(pos, delimiter);
    if(eat_prefix(&pos, "BYTES:") == -1) {
      printf("prefix wasn't correct\n");
      free(id);
      return NULL;
    }
    
    int size = 32;
    char *buf = copy_to(pos, delimiter, 1);   
    write_len = (u64)strtoull(buf, (char**)NULL, 10);
    free(buf);
  }

  /* TIME */
  
  u64 time = 0;
  if(type == READ_TIME) {
    pos = get_past(pos, delimiter);
    if(eat_prefix(&pos, "TIME:") == -1) {
      printf("prefix wasn't correct\n");
      free(id);
      return NULL;
    }
    int size = 32;
    char *buf = copy_to(pos, delimiter, 1);
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



int eat_prefix(char **req, char *prefix) {
  int pre_len = strlen(prefix);
  if(strncmp(*(req), prefix, pre_len) == 0) {
    *req += pre_len;
    return 0;
  } else {
    return -1;
  }
}

char *get_past(char* req, char c) {
  int i = 0;
  while(req[i] != c) {
    if(req[i] == '\0') {
      return NULL;
    }
    i++;
  }
  return &(req[i+1]);
}

char* copy_to(char* req, char c, int check_end) {
  int size = 32;
  char *buf = (char*)malloc(size);
  int i = 0;
  while(req[i] != c) {
    if(i == size-1) {
      size *= 2;
      buf = (char*)realloc(buf, size);
    }
    if(check_end && req[i] == '\0') {
      break;
    } else if(req[i] == '\0') {
      return NULL;
    }
    buf[i] = req[i];
    i++;
  }
  buf[i] = '\0';
  return buf;
}

char *recv_request(int sid) {
  int sz = 32;
  char *buf = (char*)malloc(sz);
  int bytes = 0;
  int i = 0;
  while(1) {
    bytes += recv(sid, buf+bytes, 1, 0);
    if(buf[i++] == '\0') {
      break;
    }
    if(bytes >= sz) {
      sz *= 2;
      buf = (char*)realloc(buf, sz);
    }
    if(bytes > 2048) {
      // Arbitrary limit but I don't expect a request to be this large
      free(buf);
      return NULL;
    }
  }
  return buf;
}
