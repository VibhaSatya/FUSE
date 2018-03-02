/*
  FUSE: Filesystem in Userspace
  Copyright (C) 2001-2007  Miklos Szeredi <miklos@szeredi.hu>

  This program can be distributed under the terms of the GNU GPL.
  See the file COPYING.
*/

/** @file
 *
 * minimal example filesystem using high-level API
 *
 * Compile with:
 *
 *     gcc -Wall driver.c `pkg-config fuse3 --cflags --libs` -o driver
 *
 * ## Source code ##
 * \include hello.c
 */


#define FUSE_USE_VERSION 31

#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stddef.h>
#include <assert.h>
#include <stdlib.h>
#include <unistd.h>
#include "disk.h"
#include<time.h>
#include "fs.h"

#define DISK_MAGIC 0xdeadbeef

//#######################################################################################################################################################################################################
//DISK

static FILE *diskfile;
static FILE *logfile;
static int nblocks=0;
static int nreads=0;
static int nwrites=0;

const char* logFile ="logs.txt";
int writeLog(char *message)
{
	time_t rawtime;
  	struct tm * timeinfo;
	char *t=(char*)malloc(50);
	time ( &rawtime );
  	timeinfo = localtime ( &rawtime );
	strcpy(t,asctime (timeinfo));
	logfile = fopen(logFile,"a+");
	if(logfile==NULL) return -1;
	fwrite("\n\n",1,1,logfile);
	t[strlen(t)-1]=' ';
	fwrite(t,1,strlen(t),logfile);
	
	fwrite(message,1,strlen(message),logfile);
	
	fclose(logfile);
	return 1;	

}



int disk_init( const char *filename, int n )
{
	writeLog("Initializing disk");
	diskfile = fopen(filename,"r+");
	if(!diskfile) diskfile = fopen(filename,"w+");
	if(!diskfile) {writeLog("Initialization Failed"); return 0;}

	ftruncate(fileno(diskfile),n*DISK_BLOCK_SIZE);

	nblocks = n;
	nreads = 0;
	nwrites = 0;
	writeLog("Successful initialization");
	return 1;
}

int disk_size()
{
	return nblocks;
}

static void sanity_check( int blocknum, const void *data )
{
	if(blocknum<0) {
		printf("ERROR: blocknum (%d) is negative!\n",blocknum);
		abort();
	}

	if(blocknum>=nblocks) {
		printf("ERROR: blocknum (%d) is too big!\n",blocknum);
		abort();
	}

	if(!data) {
		printf("ERROR: null data pointer!\n");
		abort();
	}
}

void disk_read( int blocknum, char *data )
{
	//char* temp=malloc(sizeof(char)*DISK_BLOCK_SIZE);
	sanity_check(blocknum,data);
	fseek(diskfile,blocknum*DISK_BLOCK_SIZE,SEEK_SET);
	
	if(fread(data,DISK_BLOCK_SIZE,1,diskfile)==1) {
		nreads++;
	} else {
		printf("ERROR: couldn't access simulated disk: %s\n",strerror(errno));
		abort();
	}
}

void disk_write( int blocknum, const char *data )
{
	sanity_check(blocknum,data);

	fseek(diskfile,blocknum*DISK_BLOCK_SIZE,SEEK_SET);
        
	if(fwrite(data,DISK_BLOCK_SIZE,1,diskfile)==1) {
		nwrites++;
	} else {
		printf("ERROR: couldn't access simulated disk: %s\n",strerror(errno));
		abort();
	}
}

void disk_close()
{
	if(diskfile) {
		printf("%d disk block reads\n",nreads);
		printf("%d disk block writes\n",nwrites);
		fclose(diskfile);
		diskfile = 0;
	}
}

//#######################################################################################################################################################################################################
//FILE STRUCTURE
#define FS_MAGIC           0xf0f03410
#define INODES_PER_BLOCK   128
#define POINTERS_PER_INODE 5
#define POINTERS_PER_BLOCK 1024
#define NO_OF_BLOCKS 256
#define NO_INODE_BLOCKS 26
#define NO_OF_INODES INODES_PER_BLOCK*NO_INODE_BLOCKS

struct fs_superblock {
	int magic;
	int nblocks;
	int ninodeblocks;
	int ninodes;
};

struct fs_inode {
	int isvalid;
	int size;
	int direct[POINTERS_PER_INODE];
	int indirect;
};

union fs_block {
	struct fs_superblock super;
	struct fs_inode inode[INODES_PER_BLOCK];
	int pointers[POINTERS_PER_BLOCK];
	char data[DISK_BLOCK_SIZE];
};



//#######################################################################################################################################################################################################
//UTILITY FUNCTIONS
int super_init()
{
	
	struct fs_superblock *super=(struct fs_superblock*)malloc(sizeof(struct fs_superblock));
	super->magic=FS_MAGIC;
	super->nblocks=	NO_OF_BLOCKS;
	super->ninodeblocks=NO_INODE_BLOCKS;
	super->ninodes= NO_OF_INODES;
	if(fwrite(super,DISK_BLOCK_SIZE,1,diskfile)==1) {
		nwrites++;
	} else {
		printf("ERROR: couldn't access simulated disk: %s\n",strerror(errno));
		abort();
	}		
}
int fs_format()
{	


	disk_init("disk.dat",256);
	super_init();
	disk_close();
	return 0;
}

void fs_debug()
{
	union fs_block block;

	disk_read(0,block.data);

	printf("superblock:\n");
	printf("    %d blocks\n",block.super.nblocks);
	printf("    %d inode blocks\n",block.super.ninodeblocks);
	printf("    %d inodes\n",block.super.ninodes);
}


int fs_mount()
{
	
		
	return 0;
}

int fs_create()
{
	return 0;
}

int fs_delete( int inumber )
{
	return 0;
}

int fs_getsize( int inumber )
{
	return -1;
}

int fs_read( int inumber, char *data, int length, int offset )
{
	return 0;
}

int fs_write( int inumber, const char *data, int length, int offset )
{
	return 0;
}
//#######################################################################################################################################################################################################
/*
 * Command line options
 *
 * We can't set default values for the char* fields here because
 * fuse_opt_parse would attempt to free() them when the user specifies
 * different values on the command line.
 */
static struct options {
	const char *filename;
	const char *contents;
	int show_help;
} options;

#define OPTION(t, p)                           \
    { t, offsetof(struct options, p), 1 }
static const struct fuse_opt option_spec[] = {
	OPTION("--name=%s", filename),
	OPTION("--contents=%s", contents),
	OPTION("-h", show_help),
	OPTION("--help", show_help),
	FUSE_OPT_END
};

static void *hello_init(struct fuse_conn_info *conn,
			struct fuse_config *cfg)
{
	(void) conn;
	cfg->kernel_cache = 1;
	return NULL;
}

static int hello_getattr(const char *path, struct stat *stbuf,
			 struct fuse_file_info *fi)
{
	(void) fi;
	int res = 0;

	memset(stbuf, 0, sizeof(struct stat));
	if (strcmp(path, "/") == 0) {
		stbuf->st_mode = S_IFDIR | 0755;
		stbuf->st_nlink = 2;
	} else if (strcmp(path+1, options.filename) == 0) {
		stbuf->st_mode = S_IFREG | 0444;
		stbuf->st_nlink = 1;
		stbuf->st_size = strlen(options.contents);
	} else
		res = -ENOENT;

	return res;
}

static int hello_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
			 off_t offset, struct fuse_file_info *fi,
			 enum fuse_readdir_flags flags)
{
	(void) offset;
	(void) fi;
	(void) flags;

	if (strcmp(path, "/") != 0)
		return -ENOENT;

	filler(buf, ".", NULL, 0, 0);
	filler(buf, "..", NULL, 0, 0);
	filler(buf, options.filename, NULL, 0, 0);

	return 0;
}





static int hello_open(const char *path, struct fuse_file_info *fi)
{
	if (strcmp(path+1, options.filename) != 0)
		return -ENOENT;

	if ((fi->flags & O_ACCMODE) != O_RDONLY)
		return -EACCES;

	return 0;
}

static int hello_read(const char *path, char *buf, size_t size, off_t offset,
		      struct fuse_file_info *fi)
{
	size_t len;
	(void) fi;
	if(strcmp(path+1, options.filename) != 0)
		return -ENOENT;

	len = strlen(options.contents);
	if (offset < len) {
		if (offset + size > len)
			size = len - offset;
		memcpy(buf, options.contents + offset, size);
	} else
		size = 0;

	return size;
}

static struct fuse_operations hello_oper = {
	.init           = hello_init,
	.getattr	= hello_getattr,
	.readdir	= hello_readdir,
	.open		= hello_open,
	.read		= hello_read,
};

static void show_help(const char *progname)
{
	printf("usage: %s [options] <mountpoint>\n\n", progname);
	printf("File-system specific options:\n"
	       "    --name=<s>          Name of the \"hello\" file\n"
	       "                        (default: \"hello\")\n"
	       "    --contents=<s>      Contents \"hello\" file\n"
	       "                        (default \"Hello, World!\\n\")\n"
	       "\n");
}

int main(int argc, char *argv[])
{
	
	char* str=(char*)malloc(sizeof(char)*DISK_BLOCK_SIZE);
	char* temp=(char*)malloc(sizeof(char)*DISK_BLOCK_SIZE);
	strcpy(temp,"hi");
	fs_format();
	disk_write(4,temp);
	disk_read(4,str);
	printf("%s",str);
	struct fuse_args args = FUSE_ARGS_INIT(argc, argv);

	/* Set defaults -- we have to use strdup so that
	   fuse_opt_parse can free the defaults if other
	   values are specified */
	options.filename = strdup("hello");
	options.contents = strdup("Hello World!\n");

	/* Parse options */
	if (fuse_opt_parse(&args, &options, option_spec, NULL) == -1)
		return 1;

	/* When --help is specified, first print our own file-system
	   specific help text, then signal fuse_main to show
	   additional help (by adding `--help` to the options again)
	   without usage: line (by setting argv[0] to the empty
	   string) */
	if (options.show_help) {
		show_help(argv[0]);
		assert(fuse_opt_add_arg(&args, "--help") == 0);
		args.argv[0] = (char*) "";
	}
        disk_close();
	return fuse_main(args.argc, args.argv, &hello_oper, NULL);
}
