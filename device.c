#include "device.h"

FILE *FL;

/*
    Bloque 0, Metadata(Tamaño, Bloques, Bloques libres, Palabras en 0, Entero Actual, BloqueInit Bitmap)
    Bloque 1, Mapa de bits

    Test
    2Gb-2048Mb-2097152Kb

    -sff -create dev1.fs -s 50Gb

    50*1024*1024/4 = x/1024/1024/8

    
*/

int create_disk(char* path, int size){
    if(size >= 12){
        FL = fopen(path, "wb+");
        if(FL == NULL){
            return 0;
        }
        format_disk(size);
        fldisk();
        return 1;
    }
    return -1;
}

int open_disk(char* path, int size){
    FL = fopen(path, "rb+");
    if(FL == NULL){
        return create_disk(path, size);
    }
    return 1;
}

int close_disk(){
    if(FL == NULL){
        return 0;
    }
    fldisk();
    fclose(FL);
    return 1;
}

int read_block_disk(char buff[], int block) {
    fseek(FL, block*BLOCK_SIZE, SEEK_SET);
    return fread(buff, 1, BLOCK_SIZE, FL);
}

int write_block_disk(char buff[], int block) {
    fseek(FL, block*BLOCK_SIZE, SEEK_SET);	
    return fwrite(buff, 1, BLOCK_SIZE, FL);
}

void format_disk(int size){
    int blocks;
    if((size / 4) % 4 == 0){
        blocks = size / 4;
    }else{
        blocks = (size / 4) + 1;
    }
    printf("Bloques: %d\n", blocks);
    int enteros = blocks/32;
    printf("Arreglos de Enteros: %d\n", enteros);
    unsigned int bitmap[enteros];
    int abarcaB = enteros*4;
    printf("Arreglo en Bytes: %d\n", abarcaB);
    int abarcaBlocks = abarcaB/4096;
    printf("Arreglo en Bloques: %d\n", abarcaBlocks);
    int bloquelib = blocks- abarcaBlocks -1;
    printf("Bloques Libres Iniciales: %d\n", bloquelib);
    int palabrasEn0 = 0;
    int sizeGB = size/1024/1024;
    int count_block_dir = BASE_DIR_BLOCKS*sizeGB;
    printf("Bloques de Directorio: %d\n", count_block_dir);
    int i;
    for(i=0;i<enteros;i++){
        bitmap[i] = UINT_MAX;
    }
    
    int EnterosAModificar = abarcaBlocks/32;
    int j;
    for(i=0; i <= EnterosAModificar; i++){
        for(j = 0; j < abarcaBlocks + 1 + count_block_dir; j++)
        {
            //printf("bit %d: %u\n",j,BIT_GET(bitmap[i],j));
            BIT_CLEAR(bitmap[i], j);
        }
    }
    /*for(int j = 0; j < 32; j++)
        {
            printf("nbit %d: %u\n",j,BIT_GET(bitmap[0],j));
        }

    printf("%u\n", bitmap[0]);*/

    int first_block_dir = abarcaBlocks + 2;

    fseek(FL, 1*BLOCK_SIZE, SEEK_SET);	
    fwrite(bitmap, sizeof(int), enteros, FL);
    fseek(FL, 0*BLOCK_SIZE, SEEK_SET);
    fwrite(&size, sizeof(int), 1, FL);
    fwrite(&blocks, sizeof(int), 1, FL);
    fwrite(&bloquelib, sizeof(int), 1, FL);
    fwrite(&palabrasEn0, sizeof(int), 1, FL);
    fwrite(&first_block_dir, sizeof(int), 1, FL);
    fwrite(&count_block_dir, sizeof(int), 1, FL);
    fwrite(&palabrasEn0, sizeof(int), 1, FL);
}

void fldisk()
{
    fflush(FL);
}