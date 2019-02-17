#ifndef filesystem_h
#define filesystem_h

#include "fuse.h"
#include "device.h"
#include <time.h>
#include "math.h"
#include "utils.h"
#include <string.h>
#include <errno.h>


#ifdef __cplusplus
extern "C" {
#endif

#define ENTRY_SIZE sizeof(struct Entry_Directory)
#define ENTRY_SIZE_P 256

struct Entry_Directory
{
    char path[235];
    int size;
    char is_dir;
    int create_date;
    int modi_date;
    int index_block;
};

struct Metadata
{
    int size;
    int blocks;
    int free_blocks;
    int zero_words;
    int first_block_dir;
    int count_block_dir;
    int count_entries;
};

void create_partition(char* path, int size);
void load_metadata();
void load_bitmap();
void load_entrys();
void save_metadata();
void save_bitmap();
void save_entrys();
int find_free_block();
void mountm(char* path);
void unmountm();
void create_file(char* filename);
void create_dir(char* filename);
void list_dir();
struct Entry_Directory create_dir_entry(char* filename,int is_dir);
void write_dir_entry(struct Entry_Directory entry);

void* filesystem_init(struct fuse_conn_info *conn);
struct Entry_Directory *filesystem_get_entry(const char *path);
int filesystem_getattr(const char *path, struct stat *statbuf);
int filesystem_mkdir(const char *path, mode_t mode);
int filesystem_readdir(const char *path, void *buffer, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi);
int filesystem_mknod(const char *path, mode_t mode, dev_t dev);
int filesystem_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fileInfo);
int filesystem_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fileInfo) ;
int filesystem_rename(const char *path, const char *newpath);
int filesystem_unlink(const char *path);
int filesystem_rmdir(const char *path);
int filesystem_statfs(const char *path, struct statvfs *statInfo);

#ifdef __cplusplus
}
#endif

#endif //sfs_h