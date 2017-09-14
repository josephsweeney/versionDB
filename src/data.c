#include<stdio.h>


int write(char *id, int byteCount, char *data) {
  FILE* fd = fopen(id, "wb");

  if(!fd) {
    return 0;
  }

  fwrite(data, 1, byteCount, fd);
  fclose(fd);

  return 1;
}

int read(char *id, char buffer[]) {
  FILE* fd = fopen(id, "rb");

  if(!fd) {
    return 0;
  }

  while(1) {
    int bytes_read = fread(buffer++, 1, 1, fd);
    if(bytes_read < 1) 
      break;
  }
    
  
  /* fwrite(data, 1, byteCount, fd); */

  fclose(fd);

  return 1;
}

int main()
{

  printf("Testing read and write.\n");
  
  char* data = "We made it.";

  write("test1", 12, data);

  char buffer[100];

  read("test1", buffer);

  printf("%s", buffer);
  printf("\n");

}
