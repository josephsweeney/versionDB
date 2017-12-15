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

  st = listen(sd, 10);
  error(st, __FILE__, __LINE__);

  /* pthread_t tids[num_threads]; */
  
  /* for(int i = 0; i<num_threads; i++) { */
  /*   pthread_create(&tids[i], NULL, accept_loop, (void *)&sd); */
  /* } */

  accept_loop(sd);

  close(sd);
  unlink(host);
  
  return 0;
}


void accept_loop(int listen_socket) {
  pthread_t tid;
  struct sockaddr_un client;
  socklen_t client_sz;
  while(1) {
    int *sid = (int*)malloc(sizeof(int));
    *sid = accept(listen_socket, (struct sockaddr*)&client, &client_sz);
    printf("\nCreating new thread!!!\n\n");
    pthread_create(&tid, NULL, process_request, (void *)sid);
    pthread_detach(tid);
  }
}

void* process_request(void *in) {
  int sid = *(int*)in;
  char *request;
  RequestInfo *info;
  while(1) {
    request = recv_request(sid);
    if(request == NULL) {
      // Bad Request,
      // TODO: Send back an error
      continue; 
    }
    printf("%s\n", request);
    info = request_info_get(request);
    if(info == NULL) {
      // Bad Request,
      // TODO: Send back an error
      continue; 
    }
    if(info->type == EXIT) {
      free(request);
      request_info_destroy(info);
      break;
    }
    BYTE *buf;
    int sz=0;
    switch(info->type) {
      case WRITE:
	// write data
	printf("Write\n");
	printf("ID: %s\n", info->id);
	printf("LEN: %llu\n", info->write_len);
	buf = (BYTE*)malloc(info->write_len);
	retrieve_data(sid, info, buf);
	add_data(info->id, buf, info->write_len);
        // Send response back (response is hash str of stored data)
	output_hash_str(sid, info->id);
	break;
      case READ:
	//read data
	printf("Read\n");
	printf("ID: %s\n", info->id);
	sz = buf_size(info->id);
	buf = (BYTE*)malloc(sz);
	get_data(info->id, buf, sz);
	printf("BUF of sz: %d:\n%s\nDone reading\n", sz,buf);
	send_bytes(sid, buf, sz);
	break;
      case READ_TIME:
	// read data at time
	printf("Read at time\n");
	printf("ID: %s\n", info->id);
	printf("TIME: %llu\n", info->time);
	// TODO: if old file is larger than current, this breaks
	sz = buf_size(info->id);
	buf = (BYTE*)malloc(sz);
	get_data_at_time(info->id, buf, sz, info->time);
	send_bytes(sid, buf, sz);
	break;
      case LS:
	// output all the refs
	printf("LS\n");
	output_refs(sid);
	break;
      case HISTORY:
	printf("History\nID: %s\n", info->id);
	output_history(sid, info->id);
	break;
      default:
	printf("Unknown type given");
    }
    free(request);
    request_info_destroy(info);
  }
  close(sid);
  free(in);
  printf("Finished request!!!!\n");
  pthread_exit(NULL);
}


RequestInfo* request_info_get(char *req) {
  char delimiter = ',';

  char *pos = req;

  /* TYPE */

  if(eat_prefix(&pos, "TYPE:") == -1) {
    printf("prefix wasn't correct\n");
    printf("%s != TYPE:\n", pos);
    return NULL;
  }
  RequestType type;

  if(strncmp(pos, "WRITE", 5) == 0) {
    type = WRITE;
  } else if(strncmp(pos, "READ_TIME", 9) == 0) {
    type = READ_TIME;
  } else if(strncmp(pos, "READ", 4) == 0) {
    type = READ;
  } else if(strncmp(pos, "EXIT", sizeof("EXIT")) == 0) {
    type = EXIT;
  } else if(strncmp(pos, "EXIT", sizeof("EXIT")) == 0) {
    type = LS;
  } else {
    // UNSUPPORTED TYPE
    printf("Request had unsupported type.\n%s\n", req);
    return NULL;
  }

  if(type == EXIT) {
    RequestInfo *info = (RequestInfo*)malloc(sizeof(RequestInfo));
    info->type = type;
    info->id = NULL;
    info->hash = NULL;
    return info;
  }

  if(type == LS) {
    RequestInfo *info = (RequestInfo*)malloc(sizeof(RequestInfo));
    info->type = type;
    info->id = NULL;
    info->hash = NULL;
    return info;
  }
  
  pos = get_past(pos, delimiter);
  

  /* ID */

  if(eat_prefix(&pos, "ID:") == -1) {
    if(eat_prefix(&pos, "HASH:") == 0 && type == READ) {
      RequestInfo *info = (RequestInfo*)malloc(sizeof(RequestInfo));
      info->type = type;
      info->id = NULL;
      char *hash = copy_to(pos, delimiter, 1);
      info->hash = hash;
      return info;
    }
    else {
      printf("prefix wasn't correct\n");
      return NULL;
    }
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
  info->hash = NULL;
  info->write_len = write_len;
  info->time = time;

  return info;
}

void request_info_destroy(RequestInfo *info) {
  if(info->id != NULL) 
    free(info->id);
  if(info->hash != NULL)
    free(info->hash);
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

int retrieve_data(int sid, RequestInfo *info, BYTE *buf) {
  int sz = info->write_len;
  int bytes = 0;
  int i = 0;
  while(bytes < sz) {
    int recv_bytes = recv(sid, buf+bytes, 1, 0);
    if(recv_bytes < 0)
      return -1;
    bytes += recv_bytes;
  }

  return 0;
}


void send_bytes(int sid, BYTE *buf, int size) {
  /* Outputs null terminated string: "SIZE:<size>" then that many bytes */
  char size_str[64];
  sprintf(size_str, "SIZE:%d", size);

  int len = strlen(size_str);

  int bytes = 0;
  while(bytes<(len+1)) {
    bytes += send(sid, size_str + bytes, len + 1 - bytes, 0);
  }

  len = size;
  bytes = 0;
  while(bytes<(len+1)) {
    bytes += send(sid, buf + bytes, len - bytes, 0);
  }
  
}
