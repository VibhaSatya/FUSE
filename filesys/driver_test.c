#define FUSE_USE_VERSION 31

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef linux
/* For pread()/pwrite()/utimensat() */
#define _XOPEN_SOURCE 700
#endif

#include "common.h"
#include <fuse.h>
#include "disk.h"
#include "namei.h"
#include <sys/stat.h>
#include <sys/types.h>
/*
Compile with:
gcc -Wall driver_test.c `pkg-config fuse3 --cflags --libs` -o driver_test
*/

const char *filename;
const char *filecontent;
char * inter_path; 

static void *fs_buf;
static size_t fs_size;
struct file_descriptor_table *fdtable=NULL; //file descriptor table for this process

typedef enum {
	READ,
	WRITE, 
	READ_WRITE
} access_mode;


struct file_descriptor_entry {
	int fd;
	access_mode mode;
	int inode_number;
	int byte_offset;
};

struct file_descriptor_table {
	int pid;
	int total_descriptors;
	int used_descriptors;
	struct file_descriptor_entry *entries[20];
}; 

static void *file_init(struct fuse_conn_info *conn,
			struct fuse_config *cfg)
{
	printf("\nthis is hello init\n");
       // writeLog("Inside file_init");
	(void) conn;
	cfg->kernel_cache = 1;
	return NULL;
}

static int file_getattr(const char *path, struct stat *stbuf,
			struct fuse_file_info *fi)
{
	printf("\nInside getattr\n");
        writeLog("Inside getattr");
	
	(void) fi;
	int res = 0;
        
	memset(stbuf, 0, sizeof(struct stat));
	struct inode inode;
	if(get_inode(namei(path), &inode) == -1) {
		fprintf(stderr, "failed to get inode\n");
		res = -ENOENT;
		return res;
	}

	stbuf->st_ino = inode.inum;
	if (strcmp(path, "/") == 0) {
		printf("ioctl stbug-st_mode %d %d",  S_IFDIR | 0755, S_IFREG | 0644 );
		printf("read stbug-st_mode %d %d",  S_IFDIR | 0755, S_IFREG | 0644 );
		stbuf->st_mode = S_IFDIR | 0755;
		stbuf->st_nlink = 2;
	} else if (strcmp(path+1, filename) == 0) {
		stbuf->st_mode = S_IFREG | 0777;
		stbuf->st_nlink = 1;
		stbuf->st_size = strlen(filecontent);
	} else
		res = -ENOENT;
	printf("\nEnd of getattr\n");
	return res;
	
	



}

static int file_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
    off_t offset, struct fuse_file_info *fi,enum fuse_readdir_flags flags) {
       // writeLog("Inside readdir!!");
	(void) offset;
	(void) fi;
	(void) flags;
	if (strcmp(path, "/") != 0)
		return -ENOENT;

	filler(buf, ".", NULL, 0, 0);
	filler(buf, "..", NULL, 0, 0);
	filler(buf,filename, NULL, 0, 0);

	return 0; 
}
struct file_descriptor_entry * allocate_file_descriptor_entry(int pid) {

	// Allocate file descriptor table for the process
	if(fdtable==NULL)
	{
	fdtable=(struct file_descriptor_table*)malloc(sizeof(struct file_descriptor_table));
	fdtable->pid=pid;
        fdtable->total_descriptors=20; //setting a max of 20 open files per process
	fdtable->used_descriptors=0;
	int i;
        for(i=0;i<20;i++)
	{
	fdtable->entries[i]=(struct file_descriptor_entry*)malloc(sizeof(struct file_descriptor_entry));
	fdtable->entries[i]->fd=i;
	fdtable->entries[i]->inode_number=0; //setting an invalid number to show that the fd is free
	fdtable->entries[i]->byte_offset=0;
	}
	}

	//to find an available fd
        int i;
	for(i=0;i<20;i++)
	{
	if(fdtable->entries[i]->inode_number==0)
		{
		 
		 fdtable->used_descriptors++;
		 return fdtable->entries[i];


		}

	}
	return NULL; //indicates no available file entries
	}


static int file_open(const char *path, struct fuse_file_info *fi)
{
	/*printf("\nthis is hello open\n");
        //writeLog("Inside file_open");	
	if (strcmp(path+1, filename) != 0)
		return -ENOENT;


	return 0;*/
        int oflag=fi->flags;
	struct file_descriptor_entry * fde;
	struct inode inod;
	int inode_number, pid;
	mode_t mode;

	// fuse does not support create flag
	inode_number = namei(path);
	if (inode_number == -1) {
		printf("\nFetching inode number failed\n");
		abort();
	}

	get_inode(inode_number, &inod);

	pid = getpid();
	
	fde = allocate_file_descriptor_entry(pid);

	// O_RDONLY, O_RDWR, O_WRONLY
	
	if (oflag & O_WRONLY) {
		// Check for write permissions
		fde->mode = WRITE;
	}
	else if (oflag & O_RDWR) {
		// Check for read/write permissions
		fde->mode = READ_WRITE;
	}
	else {
		fde->mode = READ;	
	}

	
	fde->inode_number = inode_number;
	

	update_inode_mem();
	return fde->fd;
}



void pread_new(const char *path, char *buf, size_t size, off_t offset)
{
	printf("\nInside pread\n");
      union fs_block *newblock=(union fs_block*)malloc(sizeof(union fs_block));
	int inum=namei(path);
	disk_read(ilist[inum].datablocknum,newblock);
	memcpy(buf, newblock->data, size);
	printf("\nNew block data %s from data block %d\n",newblock->data,ilist[inum].datablocknum);
	printf("\nEnd of pread\n");

}
static int file_read(const char *path, char *buf, size_t size, off_t offset,
		      struct fuse_file_info *fi){
    
      printf("\nInside read\n");
        //writeLog("Inside read");	
	size_t len;
	(void) fi;
	if(strcmp(path+1, filename) != 0)
		return -ENOENT;

	
	int inum=namei(path);
	
	union fs_block *newblock=(union fs_block*)malloc(sizeof(union fs_block));
	disk_read(ilist[inum-1].datablocknum,newblock);
	printf("\nNew block data %s\n",newblock->data);
	len = strlen(newblock->data);
	if (offset < len) {
		if (offset + size > len)
			size = len - offset;
		memcpy(buf, newblock->data + offset, size);
	} else
		size = 0;
	
	printf("\nEnd of read\n");
	return size;



	
}

static int file_resize(size_t new_size)  
{
	void *new_buf;
        //writeLog("Inside resize");

	if (new_size == fs_size)
		return 0;

	new_buf = realloc(fs_buf, new_size);
	if (!new_buf && new_size)
		return -ENOMEM;

	if (new_size > fs_size)
		memset(new_buf + fs_size, 0, new_size - fs_size);

	fs_buf = new_buf;
	fs_size = new_size;

	return 0;
}

static int file_expand(size_t new_size)
{
	//writeLog("Inside expand");	
	if (new_size > fs_size)
		return file_resize(new_size);
	return 0;
}

static int file_type(const char *path)
{
	writeLog("Inside type");	
	printf("path is : %s", path);
	if (strcmp(path, "/") == 0)
		return 1;
	inter_path=strdup("/");
	strcat(inter_path,filename);
	if (strcmp(path, inter_path) == 0)
		return 2;
	return 0;
}

static int file_do_write(const char *buf, size_t size, off_t offset)
{
	printf("\nin fioc do write");
        //writeLog("Inside do_write");	
	if (file_expand(offset + size))
		return -ENOMEM;

	memcpy(filecontent, buf, size);

	return size;
}
size_t pwrite_new(const char* path, const void *buf, size_t nbyte, off_t offset) {
	printf("\nInside pwrite\n");
	struct file_descriptor_entry * fde;
	struct inode inod;
	int pid;
	

	pid = getpid();
	fde = allocate_file_descriptor_entry(pid);

	if (fde == NULL) {
		printf("\nfile descriptor doesn't exist\n");
		return -1; // file descriptor doesn't exist
	}

	
	
	int k;
        //looking for a free inode
	for(k=0;k<NO_OF_INODES;k++)
		if(ilist[k].valid==0) break;
	int inode_number=k+1;
	fde->inode_number=inode_number;
	get_inode(fde->inode_number, &inod);
	ilist[fde->inode_number-1].valid=1;
	int i;
	printf("\nlooking for a free datablock\n");
	for(i=0;i<NO_OF_BLOCKS;i++)
	{
	if(freeBlockList[i]==0) break;
	}
	freeBlockList[i]=1;
	ilist[fde->inode_number-1].datablocknum=i;
	memcpy(filecontent, buf, nbyte);
	printf("\nwriting data into datablock %d\n",i);
	union fs_block *newblock=(union fs_block*)malloc(sizeof(union fs_block));
	memcpy(newblock->data,buf,nbyte);
	disk_write(ilist[fde->inode_number-1].datablocknum,newblock);
	
	printf("\ndisk write successful\n");
	printf("\nAdding entry to parent directory\n");
	union fs_block *parent=(union fs_block*)malloc(sizeof(union fs_block));
	disk_read(27,parent); //read home directory block
	int j;
	for(j=0;j<DIR_LEN;j++)
	{
	if(parent->dir.inum[j]==0) break;
        }
	printf("\nWRITE: found free entry\n");
	//parent->dir.names[j]=(char*)malloc(sizeof(char)*20);
	strcpy(parent->dir.names[j],path);
	parent->dir.inum[j]=fde->inode_number;
	disk_write(27,parent);
	printf("\nFinished writing parent into disk\n");
	update_inode_mem();
	update_freeblock_list();
	//print_home();
	printf("\nEnd of pwrite\n");
	
	return nbyte;
}
static int file_write(const char *path, const char *buf, size_t size,
		      off_t offset, struct fuse_file_info *fi)
{
	printf("\nInside write\n");
        //writeLog("Inside write");	
	// (void) fi;

	// if (fs_file_type(path) != 2)
	// 	return -EINVAL;
	// printf("\nabout to do stuff");
	// return file_do_write(buf, size, offset);

	int fd;
	int res;

	(void) fi;
	pwrite_new(path, buf, size, offset);

	printf("\nEnd of write\n");
	/*if(fi == NULL)
		fd = open(path, O_WRONLY);
	else
		fd = fi->fh;
	
	if (fd == -1)
		return -errno;

	res = file_do_write(buf, size, offset);
	if (res == -1)
		res = -errno;

	if(fi == NULL)
		close(fd);
	return res;*/
	//pwrite_new(fi->fh, buf, size, offset);
	//printf("\nEnd of write\n");


}
static int file_mkdir(const char *path, mode_t mode)
{
//only single level hierarchy implemented
printf("\nInside mkdir\n");
union fs_block *block=(union fs_block*)malloc(sizeof(union fs_block));
disk_read(27,block); //read home directory block
printf("\nAfter reading home directory block\n");
int i;
for(i=0;i<DIR_LEN;i++)
{
if(block->dir.inum[i]==0) break;
printf("\n%s\n",block->dir.names[i]);
if(strcmp(block->dir.names[i],path)==0)
{
printf("\nDirectory already exists. Going back to main...\n");
writeLog("Directory already exists. Going back to main...");
return -1;
}

}
printf("\nfound free entry\n");
if(i==DIR_LEN) {printf("\nmkdir failed\n");writeLog("mkdir failed");return -1;}
//found a free entry
strcpy(block->dir.names[i],path);

//to find a free inode number
printf("\nLooking for free inode number\n");
int j;
for(j=0;j<NO_OF_INODES;j++)
{
if(ilist[j].valid==0) break;

}
printf("\nFound %d\n",ilist[j].inum);
block->dir.inum[i]=ilist[j].inum;
ilist[j].valid=1;

//to find a free block to assign to the new directory
printf("\nLooking for free datablock\n");
int k;
for(k=0;k<NO_OF_BLOCKS;k++)
{
if(freeBlockList[k]==0) break;
}
printf("\nFound %d\n",k);
ilist[j].datablocknum=k;

//initializing new directory
printf("\ninitializing new directory\n");
//inode num is 1 because 0 is an invalid inode number
struct dirBlock *newdir=(struct dirBlock*)malloc(sizeof(struct dirBlock));
//newdir->names[0]=(char*)malloc(sizeof(char)*2);
strcpy(newdir->names[0],".\0"); //current 
newdir->inum[0]=block->dir.inum[i];//inode number of new directory

//newdir->names[1]=(char*)malloc(sizeof(char)*3);
strcpy(newdir->names[1],"..\0"); //parent 
newdir->inum[1]=1;// in this case, inode number of parent i.e. home

int x;
for(x=2;x<DIR_LEN;x++)
{
//newdir->names[x]=(char*)malloc(sizeof(char)*20);
newdir->inum[x]=0; //invalid inode number
}
printf("\nWriting the new datablock into memory\n");
union fs_block *newblock=(union fs_block*)malloc(sizeof(union fs_block));
newblock->dir=*newdir;
writeLog("writing new directory block");
disk_write(k,newblock); 
disk_write(NO_INODE_BLOCKS+1,block);
update_inode_mem();
update_freeblock_list();
printf("\nEnd of mkdir\n");
return 0;
}

static int file_rmdir(const char *path)
{
printf("\nInside rmdir\n");
//only single level hierarchy implemented
union fs_block *block=(union fs_block*)malloc(sizeof(union fs_block));
disk_read(27,block); //read home directory block
printf("\nAfter reading home directory block\n");
int i;
for(i=0;i<DIR_LEN;i++)
{
if(strcmp(block->dir.names[i],path)==0)
{
printf("\nFILE found\n");
int inum=block->dir.inum[i];
block->dir.inum[i]=0;
strcpy(block->dir.names[i],"\0");

//to update the ilist
ilist[inum-1].valid=0;
int blocknum=ilist[inum-1].datablocknum;
ilist[inum-1].datablocknum=-1;
disk_write(NO_INODE_BLOCKS+1,block);
update_inode_mem();
update_freeblock_list();
return 0;
}

}
printf("\nDirectory does not exist. Aborting...\n");
writeLog("Directory does not exist. Aborting...");
return -1;
}

void print_home()
{

union fs_block *block=(union fs_block*)malloc(sizeof(union fs_block));
disk_read(27,block); //read home directory block
//printf("\nInside print_home\n");
printf("\n########################################\n");
int i;
for(i=0;i<DIR_LEN;i++)
{
if(block->dir.inum[i]==0) continue;
if(strcmp(block->dir.names[i],"\0")!=0)
printf("\n%s\n",block->dir.names[i]);
}
printf("\n########################################\n");
}
static struct fuse_operations fs_oper = {
	.init       = file_init,
	.getattr	= file_getattr,
	.readdir	= file_readdir,
	.open       = file_open,
	.read		= file_read,
	.write      = file_write,
	.mkdir		= file_mkdir,
        .rmdir      = file_rmdir,
};

int main(int argc, char *argv[]){
        fs_format();
	char command[50];
	char *mkdir ="mkdir ";
	char *rmdir ="rmdir ";
	char *touch ="touch ";
	char *cat = "cat ";
	char *ls= "ls";
	int flag=0;
	while(1)
	{
        flag=0;
	printf("\n$");
	//scanf("%s",command);	
	//gets(command);
	char * c2=malloc(sizeof(char)*20);
	char * c3=malloc(sizeof(char)*20);
	scanf("%s",c2);
	//printf("\n%d %d\n",strlen(c2),strlen(c3));
	//mkdir
	int i;
	for(i=0;i<strlen(c2);i++)
		{if(c2[i]!=mkdir[i]) {flag=1; break;}}
	if(flag==0)
	{scanf("%s",c3);file_mkdir(c3,S_IFDIR);free(c2);free(c3);continue;}
	
	//rmdir
	flag=0;	
	for(i=0;i<strlen(c2);i++)
		{if(c2[i]!=rmdir[i]) {flag=1; break;}}
	if(flag==0)
	{scanf("%s",c3);file_rmdir(c3);free(c2);free(c3);continue;}
	
	//ls
	flag=0;	
	for(i=0;i<strlen(c2);i++)
		{if(c2[i]!=ls[i]) {flag=1; break;}}
	if(flag==0)
	{print_home(c3);free(c2);free(c3);continue;}
	
	//cat	
      	flag=0;	
	for(i=0;i<strlen(c2);i++)
		{if(c2[i]!=cat[i]) {flag=1; break;}}
	if(flag==0)
	{
	scanf("%s",c3);
	//cat >
	if(strcmp(c3,">")==0)
		{
		char * c4=malloc(sizeof(char)*20);
		scanf("%s",c4);
		
		filename = strdup(c4);
		printf("\n");
		char * data=malloc(sizeof(char)*100);
		scanf("%s",data);
		filecontent =strdup(data);
		pwrite_new(filename,filecontent,100,0);
	
		free(data);
		}
	//cat
	else   {
		
		filename = strdup(c3);
		char * buf=malloc(sizeof(char)*100);
		pread_new(filename,buf,100,0);
		printf("\n########################################\n");
		puts(buf);
		printf("\n########################################\n");
		free(buf);
		
		}
	free(c2);
	free(c3);
	
	
	continue;
	}

	}
	
        //file_mkdir("dir3",S_IFDIR);
	//print_home();
	//file_mkdir("dir4",S_IFDIR);
	//print_home();
	//file_rmdir("dir3");
	//print_home();
        //file_mkdir("dir4",S_IFDIR);
	
	//file_mkdir("dir2",S_IFDIR);
	
	
        //struct fuse_args args = FUSE_ARGS_INIT(argc, argv);
	//filename = strdup("file1");
	//filecontent =strdup( "I'm the content of the only file available there\n");

	//pwrite_new(filename,filecontent,100,0);
	//print_home();
	//char *buf=(char*)malloc(sizeof(char)*100);
	//pread_new(filename,buf,100,0);
	//printf("\n%s\n",buf);
        //writeLog("END FUSE");
	//return fuse_main(args.argc, args.argv, &fs_oper, NULL);
	//disk_close();
	return;
}
