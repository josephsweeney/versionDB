/*************************** HEADER FILES ***************************/
#include <stddef.h>
#include "commit.h"

/****************************** MACROS ******************************/
#ifndef LOCK_SH
#define   LOCK_SH   1    /* shared lock */
#endif
#ifndef LOCK_EX
#define   LOCK_EX   2    /* exclusive lock */
#endif
#ifndef LOCK_NB
#define   LOCK_NB   4    /* don't block when locking */
#endif
#ifndef LOCK_UN
#define   LOCK_UN   8    /* unlock */
#endif

/**************************** DATA TYPES ****************************/


/*********************** FUNCTION DECLARATIONS **********************/

int vdb_size(char *path);  // Returns the size the of a file given file path
int buf_size(char *id);  // Same, but give the id
int buf_size_from_hash(char *hash_str);   // Same but give hash_str of object

void output_refs(int fd); // Writes all the refs to a file descriptor

// Writes out the hash_str's of id to an fd (separated by newlines)
void output_history(int fd, char *id);

void output_hash_str(int fd, char *id);

int vdb_write(char *id, BYTE *data, size_t byteCount);
int vdb_read(char *id, BYTE buffer[], int size);
void hash_to_str(BYTE *hash, char *dst);
void get_file_path(char *dest, char *hashstr);

int get_data(char *id, BYTE *buf, size_t size);

// mallocs buf, and sets size
int get_data_at_time(char* id, BYTE **buf, int *size, u64 timestamp);
int get_object_from_hash_str(char *hash_str, BYTE *buf, int sz);

int add_data_to_objects(BYTE *hash, BYTE *data, size_t size);
int add_data(char* id, BYTE *data, size_t bytes);

Commit get_commit_from_id(char *id);
Commit get_commit_from_hash(BYTE *hash);
Commit get_commit_from_hash_str(char *hash_str);
void make_commit(char* id, BYTE *hash);

int lock_ref(char *id, int lock_type);
int rlock_ref(char* id);
int wlock_ref(char* id);
int unlock_ref(char* id);
