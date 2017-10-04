/*************************** HEADER FILES ***************************/
#include <stddef.h>

/****************************** MACROS ******************************/

/**************************** DATA TYPES ****************************/


/*********************** FUNCTION DECLARATIONS **********************/

int write(char *id, BYTE *data, size_t byteCount);
int read(char *id, BYTE buffer[], int size);
void hash_to_str(BYTE *hash, char *dst);
void get_file_path(char *dest, char *hashstr);
int get_data(char *id, BYTE *buf, size_t size);
int add_data_to_objects(BYTE *hash, BYTE *data, size_t size);
int add_data(char* id, BYTE *data, size_t bytes);
void make_commit(char* id, BYTE *hash);
