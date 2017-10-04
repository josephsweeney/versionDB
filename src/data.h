/*************************** HEADER FILES ***************************/
#include <stddef.h>
#include "commit.h"

/****************************** MACROS ******************************/

/**************************** DATA TYPES ****************************/


/*********************** FUNCTION DECLARATIONS **********************/

int write(char *id, BYTE *data, size_t byteCount);
int read(char *id, BYTE buffer[], int size);
void hash_to_str(BYTE *hash, char *dst);
void get_file_path(char *dest, char *hashstr);

int get_data(char *id, BYTE *buf, size_t size);
int get_data_at_time(char* id, BYTE *buf, size_t size, u64 timestamp);

int add_data_to_objects(BYTE *hash, BYTE *data, size_t size);
int add_data(char* id, BYTE *data, size_t bytes);

Commit get_commit_from_id(char *id);
void make_commit(char* id, BYTE *hash);
