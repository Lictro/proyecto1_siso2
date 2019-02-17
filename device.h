#ifndef device_h
#define device_h

#include <stdio.h>
#include <stdlib.h> 
#include <limits.h>
#include "bits.h"

#ifdef __cplusplus
extern "C" {
#endif

#define BLOCK_SIZE 4096
#define BASE_DIR_BLOCKS 12

int create_disk(char* path, int size);
int open_disk(char* path,int size);
int is_open();
int close_disk();
int read_block_disk(char* buff, int block);
int write_block_disk(char* buff, int block);
void format_disk(int size);

void fldisk();

#ifdef __cplusplus
}
#endif

#endif //device_h