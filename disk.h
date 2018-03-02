#ifndef DISK_H
#define DISK_H

#define DISK_BLOCK_SIZE 4096
#include "fs.h"
FILE *diskfile;
FILE *logfile;
int nblocks;
int nreads;
int nwrites;

int writeLog(char *message);
int disk_init(const char *filename, char *mode,int n );
int  disk_size();

void disk_read( int blocknum, union fs_block *data );
void disk_write( int blocknum, union fs_block *data );
void disk_close();


#endif

