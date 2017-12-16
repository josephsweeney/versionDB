#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/un.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include "client.h"
#include "data.h"

int global_sd;

void int_handler(int sig) {
  send_request(global_sd, "TYPE:EXIT");
  close(global_sd);
  exit(0);
}


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

  global_sd = sid;
  signal(SIGINT, int_handler);
  if(argc >= 3) {
    if(strcmp(argv[2], "-r") == 0) {
      char *id = argv[3];
      send_read(sid, id);
      print_result(sid);
    }
    else if(strcmp(argv[2], "-w") == 0) {
      char *id = argv[3];
      // Read from stdin because it allows for use of redirects or plaintext
      // read until EOF given
      send_write_from_input(sid, id);
      print_result(sid);
    }
    else if(strcmp(argv[2], "-rt") == 0) {
      if(argc != 5) {
	printf("Error, please give ./client <path for socket> -rt <id> <time>\n");
	int_handler(0);
      }
      char *id = argv[3];
      u64 time = (u64)strtoull(argv[4], NULL, 10);
      send_read_time(sid, id, time);
      print_result(sid);
    }
    else if(strcmp(argv[2], "-rh") == 0) {
      if(argc != 4) {
	printf("Error, please give ./client <path for socket> -rh <hash>\n");
	int_handler(0);
      }
      send_read_hash(sid, argv[3]);
      print_result(sid);
    }
    else if(strcmp(argv[2], "ls") == 0) {
      if(argc != 3) {
	printf("Error, please give ./client <path for socket> ls\n");
	int_handler(0);
      }
      send_ls(sid);
      print_result(sid);
    }
    else if(strcmp(argv[2], "-history") == 0) {
      if(argc != 4) {
	printf("Error, please give ./client <path for socket> -history <id>\n");
	int_handler(0);
      }
      send_history(sid, argv[3]);
      print_result(sid);
    }
    else {
      printf("Error, please give ./client <path for socket> <request type>\n");
      printf("Request types:\n");
      printf("Read: ./client <path for socket> -r <id>\n");
      printf("Read at time: ./client <path for socket> -rt <id> <time>\n");
      printf("Read from hash: ./client <path for socket> -rh <hash>\n");
      printf("Write: ./client <path for socket> -w <id>\n");
      printf("    Then pass data in through stdin\n");
      printf("List ids: ./client <path for socket> ls\n");
      printf("History: ./client <path for socket> -history <id>\n");
    }
  }

  send_request(sid, "TYPE:EXIT");
  close(sid);
  
  return 0;
}



void readline(char* dest) {
  int i = 0;
  char curr = '\0';
  while((curr = fgetc(stdin)) != '\n') {
    dest[i++] = curr;
  }
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

  send_request(sid, buf);

  free(request);

}

void send_write_from_input(int sid, char *id) {
  int sz = 64;
  char *buf = (char*)malloc(sz);
  int i = 0;
  char curr = '\0';
  while((curr = fgetc(stdin)) != EOF) {
    buf[i++] = curr;
    if(i == sz-1) {
      sz *= 2;
      buf = (char*)realloc(buf, sz);
    }
  }
  send_write(sid, id, buf, i);
  free(buf);
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

void send_read_hash(int sid, char* hash) {
  int hash_len = strlen(hash);
  char *request = (char*)malloc(32+hash_len);
  sprintf(request, "TYPE:READ,HASH:%s", hash);

  send_request(sid, request);

  free(request);
}

void send_ls(int sid) {
  char *request = "TYPE:LS";

  send_request(sid, request);
}

void send_history(int sid, char* id) {
  int id_len = strlen(id);
  char *request = (char*)malloc(32+id_len);
  sprintf(request, "TYPE:HISTORY,ID:%s", id);

  send_request(sid, request);

  free(request);
}


void print_result(int sid) {
  char buf;
  while(1) {
    int bytes = recv(sid, &buf, 1, 0);
    if(bytes < 0)
      break;
    if(buf == (char)EOF){ break; }
    write(1, &buf, 1);
  }

}
