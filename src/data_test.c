#include <stdio.h>
#include "data.h"



int main()
{

  printf("Testing read and write.\n");
  
  char *data = "We made it.";

  /* write("test1", data, 12); */

 BYTE buffer[100];

  /* read("test1", buffer, 100); */

  add_data("conf", (BYTE*)data, strlen(data)+1);

  printf("Wrote: %s\n", data);

  get_data("conf", buffer, 100);

  printf("Read: %s\n", (char*)buffer);

}
