
#include <stdlib.h>
#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>
#include <sys/time.h>
#include<time.h>
#include <stddef.h>
#include <assert.h>

#define BLOCKSIZE 4096
#define DISK_MAGIC 0xdeadbeef
#define FS_MAGIC   0xf0f03410
#define INODES_PER_BLOCK   128
#define POINTERS_PER_INODE 5
#define POINTERS_PER_BLOCK 1024
#define NO_OF_BLOCKS 256
#define NO_INODE_BLOCKS 26
#define NO_OF_INODES INODES_PER_BLOCK*NO_INODE_BLOCKS
