#include "filesystem.h"
#include <stdlib.h>

static struct fuse_operations operations = {
    .init=filesystem_init,
    .getattr=filesystem_getattr,
    .mkdir=filesystem_mkdir,
    .readdir=filesystem_readdir,
    .mknod=filesystem_mknod,
    .write=filesystem_write,
    .read=filesystem_read,
    .rename=filesystem_rename,
    .unlink=filesystem_unlink,
    .rmdir=filesystem_rmdir,
    .statfs=filesystem_statfs
};

int main(int argc, char** argv){
    printf("Mounting disk...\n");
    
    if(open_disk(argv[1],50*1024*1024)){
        printf("SUCCESS...\n");
    }else{
        printf("ERROR...\n");
        exit(1);
    }
    for(int i = 1; i < argc; i++)
    {
		argv[i]=argv[i+1];
	}
	argc--;

    int fuse_stat=fuse_main(argc, argv, &operations, NULL);

    close_disk();

    return fuse_stat;
}