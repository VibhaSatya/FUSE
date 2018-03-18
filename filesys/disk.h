#ifndef DISK_H
#define DISK_H

#include "common.h"

#define BLOCKSIZE 4096
#define NAMEI_ENTRY_SIZE 20
#define DISK_MAGIC 0xdeadbeef
#define FS_MAGIC   0xf0f03410
#define INODES_PER_BLOCK   128
#define POINTERS_PER_INODE 5
#define POINTERS_PER_BLOCK 1024
#define NO_OF_BLOCKS 256
#define NO_INODE_BLOCKS 26
#define NO_OF_INODES INODES_PER_BLOCK*NO_INODE_BLOCKS
#define DIR_LEN BLOCKSIZE/NAMEI_ENTRY_SIZE

#define HOME_INODE_NO 1

int inodeCount;
struct fs_superblock {
	int magic;
	int nblocks;
	int ninodeblocks;
	int ninodes;
};

struct fs_inode {
	int isvalid;
	int size;
	int blocknum;
        int inum;
	int datablocknum;
};

struct inode{
int datablocknum;
int inum;
int valid;

}ilist[NO_OF_INODES];

struct dirBlock
{
	char names[DIR_LEN][20];
	int inum[DIR_LEN];	

};

int freeBlockList[NO_OF_BLOCKS];
union fs_block {
	struct fs_superblock super;
	struct fs_inode inode[INODES_PER_BLOCK];
	int pointers[POINTERS_PER_BLOCK];
        struct dirBlock dir;
	char data[BLOCKSIZE];
};

//some utility functions
void super_init();
void inodeBlocks_init();
void fill_ilist();
void update_inode_mem();
void update_freeblock_list();
void print_home();

int fs_format();
void fs_debug();
int fs_mount();
int fs_delete( int inumber );
int fs_read( int inumber, char *data, int length, int offset );
int fs_write( int inumber, const char *data, int length, int offset );

FILE *diskfile;
FILE *logfile;
int nblocks;
int nreads;
int nwrites;

//disk operations
int writeLog(char *message);
int disk_init(const char *filename, char *mode,int n );
int  disk_size();
void disk_read( int blocknum, union fs_block *data );
void disk_write( int blocknum, union fs_block *data );
void disk_close();

#endif



