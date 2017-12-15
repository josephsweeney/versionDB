#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <fcntl.h>
#include <dirent.h>
#include "sha1.h"
#include "data.h"
#include "commit.h"


int vdb_size(char *path) {
  struct stat st;

  if(stat(path, &st) == 0)
    return st.st_size;

  return -1; 

}

int buf_size(char *id) {
  int hash_size = SHA1_BLOCK_SIZE;
  int hash_str_size = hash_size*2+1;

  rlock_ref(id);
  Commit commit = get_commit_from_id(id);
  unlock_ref(id);

  // Get data filepath from commit
  BYTE *data_hash = commit.data;
  char hash_str[hash_str_size];
    
  hash_to_str(data_hash, hash_str);
  char data_path[hash_str_size + 20];
  get_file_path(data_path, hash_str);

  return vdb_size(data_path);
}

int buf_size_from_hash(char *hash_str) {
  int hash_size = SHA1_BLOCK_SIZE;
  int hash_str_size = hash_size*2+1;

  char data_path[hash_str_size + 20];
  get_file_path(data_path, hash_str);

  return vdb_size(data_path);
}


void output_refs(int fd) {
  DIR *d;
  struct dirent *dir;
  d = opendir("db/refs/");
  if (d) {
    while ((dir = readdir(d)) != NULL) {
      int len = dir->d_namlen;
      int bytes = 0;
      while(bytes < len) {
	bytes += write(fd, dir->d_name + bytes, len-bytes);
      }
    }
    closedir(d);
  }

}

void output_history(int fd, char *id) {
  int hash_size = SHA1_BLOCK_SIZE;
  int hash_str_size = hash_size*2+1;

  int locked = rlock_ref(id);
  Commit commit = get_commit_from_id(id);
  if(locked) unlock_ref(id);

  BYTE *data_hash;
  char hash_str[hash_str_size];
  while(commit.has_parent) {
    commit = get_commit_from_hash(commit.parent);
    data_hash = commit.data;
    hash_to_str(data_hash, hash_str);
    int bytes = 0;
    while(bytes < hash_str_size) {
      bytes += write(fd, hash_str + bytes, hash_str_size-bytes);
    }
    write(fd, "\n", 1);
  }
}


int vdb_write(char *id, BYTE *data, size_t byte_count) {
  int fd = open(id, O_WRONLY|O_CREAT|O_TRUNC, S_IRUSR|S_IWUSR);

  if(fd == -1) {
    printf("Couldn't open file %s to write.\n", id);
    return(0);
  }
  int written = 0;
  while(written < byte_count) {
    written += write(fd, data - written, byte_count - written);
  }
  close(fd);

  return 1;
}

int vdb_read(char *id, BYTE buffer[], int size) {
  int fd = open(id, O_RDONLY);

  if(fd == -1) {
    printf("Couldn't open file %s to read.\n", id);
    return(0);
  }
  int total_read = 0;
  int bytes_read = 1;
  while(1) {
    bytes_read = read(fd, buffer + total_read, size - total_read);
    total_read += bytes_read;
    
    if(total_read > size) {
      printf("Not enough memory given for id %s.\n", id);
      printf("    Read %d bytes, given %d bytes for buffer\n", total_read, size);
      return(0);
    }
    if(total_read == size)
      break;
  }
  close(fd);

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


int get_current_commit(char *id, BYTE *hash) {
  int hash_size = SHA1_BLOCK_SIZE;
  int hash_str_size = hash_size*2+1;

  int idlen = strlen(id);
  char refpath[10 + idlen];
  sprintf(refpath, "db/refs/");
  strcat(refpath, id);

  if(!vdb_read(refpath, hash, SHA1_BLOCK_SIZE)) {
    hash = NULL;
    return 0;

  }

  return 1;
}

void make_commit(char *id, BYTE *hash) {
  Commit commit;
  int hash_size = SHA1_BLOCK_SIZE;

  BYTE parent[hash_size];

  int locked = wlock_ref(id);

  get_current_commit(id, parent);

  commit_init(&commit, hash, parent);

  SHA1_CTX ctx;
  BYTE commit_hash[hash_size];

  // Hash the commit
  sha1_init(&ctx);
  sha1_update(&ctx, (BYTE*)&commit, sizeof(Commit));
  sha1_final(&ctx, commit_hash);

  // Add Commit to object db
  add_data_to_objects(commit_hash, (BYTE *)&commit, sizeof(Commit));

  // Add commit to hash to id's ref
  int idlen = strlen(id);
  char refpath[10 + idlen];
  sprintf(refpath, "db/refs/");
  strcat(refpath, id);

  
  vdb_write(refpath, commit_hash, hash_size);
  if(locked) unlock_ref(id);
}

Commit get_commit_from_id(char *id) {
  // Get ref from id
  int idlen = strlen(id);
  char refpath[10 + idlen];
  sprintf(refpath, "db/refs/");
  strcat(refpath, id);

  // Get commit hash from ref
  int hash_size = SHA1_BLOCK_SIZE;
  int hash_str_size = hash_size*2+1;

  BYTE commit_hash[hash_size];

  vdb_read(refpath, commit_hash, hash_size);
  
  // Get commit from hash
  char hash_str[hash_str_size];
  hash_to_str(commit_hash, hash_str);
  char objpath[hash_str_size + 20];
  get_file_path(objpath, hash_str);

  Commit commit;

  vdb_read(objpath, (BYTE*)&commit, sizeof(Commit));

  return commit;
}

Commit get_commit_from_hash(BYTE *hash) {
  int hash_size = SHA1_BLOCK_SIZE;
  int hash_str_size = hash_size*2+1;

  // Get object from hash
  char hash_str[hash_str_size];
  hash_to_str(hash, hash_str);
  char objpath[hash_str_size + 20];
  get_file_path(objpath, hash_str);

  Commit commit;

  vdb_read(objpath, (BYTE*)&commit, sizeof(Commit));

  return commit;
}

Commit get_commit_from_hash_str(char *hash_str) {
  int hash_size = SHA1_BLOCK_SIZE;
  int hash_str_size = hash_size*2+1;

  // Get object from hash
  char objpath[hash_str_size + 20];
  get_file_path(objpath, hash_str);

  Commit commit;

  vdb_read(objpath, (BYTE*)&commit, sizeof(Commit));

  return commit;
}


int get_object_from_hash_str(char *hash_str, BYTE *buf, int sz) {
  int hash_size = SHA1_BLOCK_SIZE;
  int hash_str_size = hash_size*2+1;

  // Get object from hash
  char objpath[hash_str_size + 20];
  get_file_path(objpath, hash_str);

  return vdb_read(objpath, buf, sz);
}

int get_data(char *id, BYTE *buf, size_t size) {
  int hash_size = SHA1_BLOCK_SIZE;
  int hash_str_size = hash_size*2+1;

  int locked = rlock_ref(id);
  Commit commit = get_commit_from_id(id);
  if(locked) unlock_ref(id);


  // Get data filepath from commit
  BYTE *data_hash = commit.data;
  char hash_str[hash_str_size];
    
  hash_to_str(data_hash, hash_str);
  char data_path[hash_str_size + 20];
  get_file_path(data_path, hash_str);

  // Read data from filepath
  vdb_read(data_path, buf, size);

  return 0;
}

int get_data_at_time(char* id, BYTE *buf, size_t size, u64 timestamp) {
  /* !!!!!!!!!!!!!!!!!!!!!!!! 
          NEEDS TESTING 
     !!!!!!!!!!!!!!!!!!!!!!!! */
  
  int hash_size = SHA1_BLOCK_SIZE;
  int hash_str_size = hash_size*2+1;

  int locked = rlock_ref(id);
  Commit commit = get_commit_from_id(id);
  if(locked) unlock_ref(id);

  while(commit.time > timestamp && commit.has_parent) {
    commit = get_commit_from_hash(commit.parent);
  }

  // Get data filepath from commit
  BYTE *data_hash = commit.data;
  char hash_str[hash_str_size];
    
  hash_to_str(data_hash, hash_str);
  char data_path[hash_str_size + 100];
  get_file_path(data_path, hash_str);

  // Read data from filepath
  vdb_read(data_path, buf, size);

  return 0;
}

int add_data_to_objects(BYTE *hash, BYTE *data, size_t size) {
  char path[16];
  sprintf(path, "db/objects/");
  SHA1_CTX ctx;
  int hash_size = SHA1_BLOCK_SIZE;
  int hash_str_size = hash_size*2+1;
  char filepath[16+hash_str_size+1];

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
  vdb_write(filepath, data, size);

  return 0;
}


int add_data(char* id, BYTE *data, size_t bytes) {
  SHA1_CTX ctx;
  int hash_size = SHA1_BLOCK_SIZE;
  BYTE hash[hash_size];

  // Hash the data
  sha1_init(&ctx);
  sha1_update(&ctx, data, bytes);
  sha1_final(&ctx, hash);

  // Add the data to the objects db
  add_data_to_objects(hash, data, bytes);

  // Make commit
  make_commit(id, hash);
  
  return 0;
}


int lock_ref(char *id, int lock_type) {
  int idlen = strlen(id);
  char refpath[10 + idlen];
  sprintf(refpath, "db/refs/");
  strcat(refpath, id);

  int fd = open(refpath, O_RDONLY);

  if(fd == -1) {
    printf("Couldn't open file %s for locking.\n", refpath);
    close(fd);
    return 0;
  }
  else {
    if(flock(fd, lock_type) < 0) {
      printf("flock failed on id %s\n", id);
      close(fd);
      return 0;
    }

    close(fd);
    return 1;
  }

}

int rlock_ref(char* id) {
  /* printf("RLocking id %s\n", id); */
  return lock_ref(id, LOCK_SH);
}
int wlock_ref(char* id) {
  /* printf("Locking id %s\n", id); */
  return lock_ref(id, LOCK_EX);
}
int unlock_ref(char* id) {
  /* printf("Unlocked id %s\n", id); */
  return lock_ref(id, LOCK_UN);
}
