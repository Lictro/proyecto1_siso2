#include "filesystem.h"

struct Metadata METADATA;
struct Entry_Directory* ENTRIES;

uint32_t * BITMAP;
uint32_t SIZE_BITMAP;
int IS_OPEN;

char CURR_DIR[256];

void create_partition(char* path, int size){
    create_disk(path,size);
}

void load_metadata(){
    char* buffer = (char*)malloc(BLOCK_SIZE*sizeof(char));
    read_block_disk(buffer,0);
    int pos = 0;
    memcpy(&METADATA.size, &buffer[pos], 4);
    pos+=4;
    memcpy(&METADATA.blocks, &buffer[pos], 4);
    pos+=4;
    memcpy(&METADATA.free_blocks, &buffer[pos], 4);
    pos+=4;
    memcpy(&METADATA.zero_words, &buffer[pos], 4);
    pos+=4;
    memcpy(&METADATA.first_block_dir, &buffer[pos], 4);
    pos+=4;
    memcpy(&METADATA.count_block_dir, &buffer[pos], 4);
    pos+=4;
    memcpy(&METADATA.count_entries, &buffer[pos], 4);
    //cout<<"Tamaño: "<<METADATA.size<<endl;
    //cout<<"# Bloques: "<<METADATA.blocks<<endl;
    //cout<<"# Bloques Libres: "<<METADATA.free_blocks<<endl;
    //cout<<"Palabras en 0: "<<METADATA.zero_words<<endl;
    //cout<<"Primer Bloque Directorio: "<<METADATA.first_block_dir<<endl;
    //cout<<"Bloques de Directorio: "<<METADATA.count_block_dir<<endl;
    //cout<<"# de Entradas: "<<METADATA.count_entries<<endl;

    ENTRIES = (struct Entry_Directory*)malloc(METADATA.count_block_dir*4096/ENTRY_SIZE_P*ENTRY_SIZE);
}

void load_bitmap(){
    SIZE_BITMAP = METADATA.blocks/32;
    BITMAP = (uint32_t*)malloc(SIZE_BITMAP*sizeof(uint32_t));
    //cout<<"Mapa de Bits: "<<SIZE_BITMAP<<endl;
    int read_words = 0;
    int i;
    for(i = 1; i <= SIZE_BITMAP/1024; i++){
        ////cout<<"Bloque "<<i<<endl;
        char* buffer = (char*)malloc(BLOCK_SIZE*sizeof(char));
        read_block_disk(buffer,i);
        for(int j = 0; j < 1024; j++){
            memcpy(&BITMAP[read_words], &buffer[j*4], 4);
            read_words++;
        }
    }
}

void load_entrys(){
    int previos_block = 0;
    int pos = 0;
    char* buffer = (char*)malloc(BLOCK_SIZE*sizeof(char));
    read_block_disk(buffer,METADATA.first_block_dir);
    struct Entry_Directory entry;
    int i;
    for(i = 0; i < METADATA.count_entries; i++){
        if(previos_block!=i/16){
            //cout<<"RELOAD "<<i<<endl;
            previos_block = i/16;
            buffer = (char*)malloc(BLOCK_SIZE*sizeof(char));
            read_block_disk(buffer,METADATA.first_block_dir + previos_block);
            pos = 0;
        }
        memcpy(&entry.path, &buffer[pos], 235);
        pos+=235;
        memcpy(&entry.size, &buffer[pos], 4);
        pos+=4;
        memcpy(&entry.is_dir, &buffer[pos], 1);
        pos+=1;
        memcpy(&entry.create_date, &buffer[pos], 4);
        pos+=4;
        memcpy(&entry.modi_date, &buffer[pos], 4);
        pos+=4;
        memcpy(&entry.index_block, &buffer[pos], 4);
        pos+=4;
        ENTRIES[i] = entry;
    }
}

void save_metadata(){
    char* buffer = (char*)malloc(BLOCK_SIZE*sizeof(char));
    int pos = 0;
    memcpy(&buffer[pos],&METADATA.size,4);
    pos+=4;
    memcpy(&buffer[pos],&METADATA.blocks,4);
    pos+=4;
    memcpy(&buffer[pos],&METADATA.free_blocks,4);
    pos+=4;
    memcpy(&buffer[pos],&METADATA.zero_words,4);
    pos+=4;
    memcpy(&buffer[pos],&METADATA.first_block_dir,4);
    pos+=4;
    memcpy(&buffer[pos],&METADATA.count_block_dir,4);
    pos+=4;
    memcpy(&buffer[pos],&METADATA.count_entries,4);
    pos+=4;
    write_block_disk(buffer,0);
}

void save_bitmap(){
    int i;
    int j;
    for(i = 1; i <= SIZE_BITMAP/1024; i++){
        char* buffer = (char*)malloc(BLOCK_SIZE*sizeof(char));
        int pos = 0;
        for(j = 0; j < 1024; j++){
            memcpy(&buffer[pos],&BITMAP[((i-1)*1024)+j],4);
            pos+=4;
        }
        write_block_disk(buffer,i);
    }
}

void save_entrys(){
    char* buffer;
    buffer = (char*)malloc(BLOCK_SIZE*sizeof(char));
    int previos_block = 0;
    int pos = 0;
    int i;
    for(i = 0; i < METADATA.count_entries; i++){
        if(pos == 4096){
            //cout<<"RELOAD "<<i<<endl;
            write_block_disk(buffer,METADATA.first_block_dir + previos_block);
            previos_block++;
            buffer = (char*)malloc(BLOCK_SIZE*sizeof(char));
            pos = 0;
        }
        memcpy(&buffer[pos],&ENTRIES[i].path,235);
        pos+=235;
        memcpy(&buffer[pos],&ENTRIES[i].size,4);
        pos+=4;
        memcpy(&buffer[pos],&ENTRIES[i].is_dir,1);
        pos+=1;
        memcpy(&buffer[pos],&ENTRIES[i].create_date,4);
        pos+=4;
        memcpy(&buffer[pos],&ENTRIES[i].modi_date,4);
        pos+=4;
        memcpy(&buffer[pos],&ENTRIES[i].index_block,4);
        pos+=4;
    }
    write_block_disk(buffer,METADATA.first_block_dir + previos_block);
}

int find_free_block(){
    int pos=0;
    int i;
    for(i = 0; i < 32; i++){
        ////cout<<BIT_GET(BITMAP[METADATA.zero_words],i)<<" "<<i<<endl;
        if(BIT_GET(BITMAP[METADATA.zero_words],i) != 0){
            pos = i;
            break;
        }
    }
    int free_block = (32*METADATA.zero_words) + pos;
    //cout<<"bL: "<<free_block<<endl;
    BIT_SET(BITMAP[METADATA.zero_words],free_block);
    if (BITMAP[METADATA.zero_words] == UINT32_MAX) {
        METADATA.zero_words++;
    }
    METADATA.free_blocks--;
    
    return free_block;
}

void mountm(char* path){
    if(open_disk(path,0)){
        IS_OPEN = 1;
        load_metadata();
        load_bitmap();
        load_entrys();
        strcpy(CURR_DIR, "/");
    }
}

void unmountm(){
    IS_OPEN = 0;
    //cout<<"Guardando entradas..."<<endl;
    save_entrys();
    //cout<<"Guardando bitmap..."<<endl;
    save_bitmap();
    //cout<<"Guardando metadata..."<<endl;
    save_metadata();
    close_disk();
}

void create_file(char* filename){
    if(1){
        struct Entry_Directory file_entry = create_dir_entry(filename,0);
        /*cout<<file_entry.path<<endl;        
        cout<<file_entry.index_block<<endl;
        cout<<file_entry.is_dir<<endl;
        cout<<file_entry.create_date<<endl;
        cout<<file_entry.modi_date<<endl;
        cout<<file_entry.size<<endl;*/

        write_dir_entry(file_entry);
    }else{
        //cout<<"Para crear un archivo o carpeta, primero tiene que abrir una particion."<<endl;
    }
}

void create_dir(char* filename){
    if(1){
        struct Entry_Directory file_entry = create_dir_entry(filename,1);
        /*cout<<file_entry.path<<endl;        
        cout<<file_entry.index_block<<endl;
        cout<<file_entry.is_dir<<endl;
        cout<<file_entry.create_date<<endl;
        cout<<file_entry.modi_date<<endl;
        cout<<file_entry.size<<endl;*/

        write_dir_entry(file_entry);
    }else{
        //cout<<"Para crear un archivo o carpeta, primero tiene que abrir una particion."<<endl;
    }
}

void list_dir(){
    //cout<<"Nombre       Tipo        Creacion      Ultima Modificacion       Tamaño"<<endl;
    //cout<<"-----------------------------------------------------------------------"<<endl;
    struct tm * create;
    struct tm * modi;
    int i;
    for(i = 0; i <  METADATA.count_entries; i++){
        time_t tc = ENTRIES[i].create_date;
        time_t tm = ENTRIES[i].modi_date;
        
        create = localtime (&tc);
        modi = localtime (&tm);
        printf("%s      %c      %d/%d/%d      %d/%d/%d      %d\n",ENTRIES[i].path,ENTRIES[i].is_dir,
            create->tm_mday,create->tm_mon,create->tm_year,
            modi->tm_mday,modi->tm_mon,modi->tm_year,
            ENTRIES[i].size);
    }
}

struct Entry_Directory create_dir_entry(char* filename,int is_dir){
    struct Entry_Directory entry;
    char src[50], dest[50];
    strcpy(src, CURR_DIR);
    strcat(src, filename);
    strcpy(entry.path, src);
    //printf("Final name |%s|\n", entry.path);
    if(is_dir){
        entry.is_dir = 'D';
    }else{
        entry.is_dir = 'F';
    }
    entry.index_block = find_free_block();
    time_t current_time;
	current_time = time(NULL);
    entry.create_date = current_time;
    entry.modi_date = current_time;
    entry.size = 4096;
    return entry;
}

void write_dir_entry(struct Entry_Directory entry){
    ENTRIES[METADATA.count_entries] = entry;
    METADATA.count_entries++;
}

void init_file(){
    create_dir("user1");
    create_dir("user2");
    create_dir("user3");
    create_file("test1.txt");
    create_file("test2.txt");
    create_file("test3.txt");
    save_entrys();
    save_bitmap();
    save_metadata();
    fldisk();
}

void* filesystem_init(struct fuse_conn_info *conn) 
{
    printf("INICIANDO FILESYSTEM\n");
    load_metadata();
    load_bitmap();
    load_entrys();

    strcpy(CURR_DIR, "/");

    init_file();
    list_dir();
    return NULL;
}

struct Entry_Directory *filesystem_get_entry(const char *path){
    printf("D Buscando path %s\n",path);
    int i;
    struct Entry_Directory copy;
    for (i = 0; i < METADATA.count_entries; ++i)
    {
        if(strcmp(path,ENTRIES[i].path)){
            
            return &ENTRIES[i];
        }
    }
    return NULL;
}
//MODIFICAR 
void filesystem_getsize(int index_block, int *size, int *blocks){
    *blocks=1;
    *size=4096;
    char buffer[4096];
    read_block_disk(buffer,index_block);
    int i;
    int b;
    for(i = 0; i < 1024; i++){
        memcpy(&b, &buffer[i*4], 4);
        if(b == 0){
            break;
        }
        blocks++;
        size = size + 4096;
    }
}

int filesystem_getattr(const char *path, struct stat *statbuf){
    printf("D filesystem_getattr%s\n",path);

    if (strcmp(path, "/")==0)
    {
        statbuf->st_mode=S_IFDIR|0777;
        statbuf->st_uid=0;
        statbuf->st_gid=0;
        statbuf->st_nlink=1;
        statbuf->st_ino=0;
        statbuf->st_size=BLOCK_SIZE;
        statbuf->st_blksize=BLOCK_SIZE;
        statbuf->st_blocks=1;
    }else{
        struct Entry_Directory* entry = filesystem_get_entry(path);
        
        if(entry==NULL)
        {
            return -ENOENT;
        }
        printf("tipo %c\n", entry->is_dir);
        if (entry->is_dir == 'D') {
            printf("        dir entry path %s\n", entry->path);
            statbuf->st_mode=S_IFDIR|0777;
            statbuf->st_uid=0;
            statbuf->st_gid=0;
            statbuf->st_nlink=1;
            statbuf->st_ino=0;
            statbuf->st_size=0;
            statbuf->st_blksize=BLOCK_SIZE;
            statbuf->st_blocks=1;
        }else{
            printf("        file entry path %s\n", entry->path);
            int size, blocks;
            filesystem_getsize(entry->index_block, &size, &blocks);

            statbuf->st_mode=S_IFREG|0777;
            statbuf->st_nlink=1;
            statbuf->st_ino=0;
            statbuf->st_uid=0;
            statbuf->st_gid=0;
            statbuf->st_size=size; 
            statbuf->st_blksize=BLOCK_SIZE;
            statbuf->st_blocks=blocks;
        }
    }
    return 0;
}
int filesystem_mkdir(const char *path, mode_t mode){
    printf("D filesystem_mkdir%s\n",path);
    create_dir((char*)path);
    save_entrys();
    save_bitmap();
    save_metadata();
    fldisk();
    return -ENOENT;
}
int filesystem_readdir(const char *path, void *buffer, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi){
    printf("D filesystem_readdir%s\n",path);
    
    struct Entry_Directory directory[100];
    if(METADATA.count_entries > 0){
        if(strcmp(path, "/")==0){
            int i;
            for(i = 0; i <  METADATA.count_entries; i++){
                printf("Name %s\n", ENTRIES[i].path);
                char * pch;
                printf ("Splitting string \"%s\" into tokens:\n",ENTRIES[i].path);
                pch = strtok (ENTRIES[i].path,"/");
                printf ("%s\n",pch);
                if(filler(buffer, pch, NULL, 0)!=0)
                {
                    return -ENOMEM;
                }
            }
        }else{
            printf("NO ROOT\n");
        }
    }else{
        printf("There are nothing%s\n",path);
    }
    return 0;
}
int filesystem_mknod(const char *path, mode_t mode, dev_t dev){
    printf("D filesystem_mknod%s\n",path);
    return -ENOENT;
}
int filesystem_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fileInfo){
    printf("D filesystem_write%s\n",path);
    return -ENOENT;
}
int filesystem_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fileInfo){
    printf("D filesystem_read%s\n",path);
    return -ENOENT;
}
int filesystem_rename(const char *path, const char *newpath){
    printf("D filesystem_rename%s\n",path);
    return -ENOENT;
}
int filesystem_unlink(const char *path){
    printf("D filesystem_unlink%s\n",path);
    return -ENOENT;
}
int filesystem_rmdir(const char *path){
    printf("D filesystem_rmdir%s\n",path);
    return -ENOENT;
}
int filesystem_statfs(const char *path, struct statvfs *statInfo){
    printf("D filesystem_statfs%s\n",path);
    return -ENOENT;
}