#include<stdio.h>
#include<string.h>
#include<sys/stat.h>
#include "sha1.h"


int write(char *id, int byteCount, BYTE *data) {
  FILE* fd = fopen(id, "wb");

  if(!fd) {
    return 0;
  }

  fwrite(data, 1, byteCount, fd);
  fclose(fd);

  return 1;
}

int read(char *id, BYTE buffer[], int size) {
  FILE* fd = fopen(id, "rb");

  if(!fd) {
    return 0;
  }

  int i = 0;
  while(1) {
    int bytes_read = fread(buffer + i, 1, 1, fd);
    i++;
    if(i >= size) {
      printf("Not enough memory given for buffer.\n");
      return 0;
    }
    if(bytes_read < 1) 
      break;
  }
      
  fclose(fd);

  return 1;
}

void hash_to_str(BYTE *hash, char *dst) {
  dst[SHA1_BLOCK_SIZE] = '\0';
  for(int i = 0; i < SHA1_BLOCK_SIZE; i += sizeof(unsigned int)) {
    snprintf(&dst[i*2], 9, "%x", *((unsigned int*)hash)+i); 
  }
}

void get_file_path(char *dest, char *hashstr) {
  int hash_size = SHA1_BLOCK_SIZE;
  int hash_str_size = hash_size*2+1;
  char filepath[16+hash_str_size+1];
  sprintf(filepath, "db/objects/");

  // Separate the hash string into a directory name, and a file name
  int dir_size  = (hash_size/10)*2+1;
  int name_size = (9*hash_size/10)*2+1;

  char dir[dir_size];
  char name[name_size];

  snprintf(dir, dir_size, "%s", hashstr);
  snprintf(name, name_size, "%s", hashstr + dir_size - 1);

  strcat(filepath, dir);
  sprintf(filepath, "%s/%s", filepath, name);
  sprintf(dest, "%s", filepath);
}

void get_data(char *id, BYTE *buf, size_t size) {
  // Get ref from id

  // Get commit from ref

  // Get hash from commit

  // Get filepath from hash

  // Read data from filepath
  /* read(filepath, buf, size); */
  
}

int add_data(char* id, BYTE *data, size_t bytes) {
  char path[16];
  sprintf(path, "db/objects/");
  SHA1_CTX ctx;
  int hash_size = SHA1_BLOCK_SIZE;
  int hash_str_size = hash_size*2+1;
  char filepath[16+hash_str_size+1];
  BYTE hash[hash_size];

  // Hash the data
  sha1_init(&ctx);
  sha1_update(&ctx, data, bytes);
  sha1_final(&ctx, hash);

  // Make the hash into a string
  char hashstr[hash_str_size];
  hash_to_str(hash, hashstr);

  // Separate the hash string into a directory name, and a file name
  int dir_size  = (hash_size/10)*2+1;
  int name_size = (9*hash_size/10)*2+1;

  char dir[dir_size];
  char name[name_size];

  snprintf(dir, dir_size, "%s", hashstr);
  snprintf(name, name_size, "%s", hashstr + dir_size - 1);

  // Make the directory
  strcat(path, dir);
  mkdir(path, S_IRWXU);

  // Write to the file
  sprintf(filepath, "%s/", path);
  strcat(filepath, name);
  write(filepath, bytes, data);

  // Make commit
  /* makeCommit(id, hash); */
  
  return 0;
}

int main()
{

  printf("Testing read and write.\n");
  
  char *data = "We made it.";

  /* write("test1", 12, data); */

  char buffer[100];

  /* read("test1", buffer, 100); */

  add_data("conf", (BYTE*)data, strlen(data)+1);

  /* get_data("conf", buffer, 100); */

}
