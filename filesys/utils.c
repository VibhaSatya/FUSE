//UTILITY FUNCTIONS
#include "common.h"
#include "disk.h"
void super_init()
{
	inodeCount=0;
	union fs_block *block=(union fs_block*)malloc(sizeof(union fs_block));
	struct fs_superblock *super=(struct fs_superblock*)malloc(sizeof(struct fs_superblock));
	super->magic=FS_MAGIC;
	super->nblocks=	NO_OF_BLOCKS;
	super->ninodeblocks=NO_INODE_BLOCKS;
	super->ninodes= NO_OF_INODES;
        block->super=*super;
	if(fwrite(block,BLOCKSIZE,1,diskfile)==1) {writeLog("Writing superblock successful");
		nwrites++; 
	} else {
		printf("ERROR: couldn't access simulated disk: %s\n",strerror(errno));
		writeLog("Writing superblock failed");
		abort();
               
	}		
}
void inodeBlocks_init()
{
//initialize 26 inode blocks
int i;
int icount=0; //inode number counter
for(i=1;i<=NO_INODE_BLOCKS;i++)
{
   //i=1 because 0 is superblock
   union fs_block *f=(union fs_block*)malloc(sizeof(union fs_block));
   int j;
   for(j=0;j<INODES_PER_BLOCK;j++)
      {
	 f->inode[j].isvalid=0; //not valid =>no data
         f->inode[j].blocknum=-1;
         f->inode[j].inum=icount+1;
	 f->inode[j].datablocknum=-1; //initial state - does not point to any data block
	 ilist[icount].datablocknum=-1; //initial state
         ilist[icount].inum=icount+1; //because 0 is an invalid inode number
         ilist[icount].valid=0;
         icount++;
      }
   writeLog("Initializing inode");   
   disk_write(i,f);
   

}

}


void fill_ilist()
{
//fill inode list from disk into memory
int i;
int icount=0; //inode number counter
for(i=1;i<=NO_INODE_BLOCKS;i++)
{
   //i=1 because 0 is superblock
   union fs_block *f=(union fs_block*)malloc(sizeof(union fs_block));
   int j;
   disk_read(i,f);
   for(j=0;j<INODES_PER_BLOCK;j++)
      {
	
	 ilist[icount].datablocknum=f->inode[j].datablocknum; //initial state
         ilist[icount].inum=f->inode[j].inum;
         ilist[icount].valid=f->inode[j].isvalid;
         icount++;
      }
   writeLog("Filling ilist from disk into memory");   
  
}
}

void update_freeblock_list()
{
//updates the free block list
writeLog("Initializing free block list");
printf("\nInitializing free block list\n");
int i;
for(i=0;i<NO_OF_BLOCKS;i++)
{
if(i<27)
//for inode and super blocks
freeBlockList[i]=1;
else
freeBlockList[i]=0;
}
for(i=0;i<NO_OF_INODES;i++)
{

if(ilist[i].datablocknum!=-1)
{
printf("\nFound an occupied block in ilist\n");
freeBlockList[ilist[i].datablocknum]=1; //to indicate that it is occupied 
}
}

}

void update_inode_mem()
{
printf("\nUpdating inode memory\n");
//updating 26 inode blocks
int i;
int icount=0; //inode number counter
for(i=1;i<=NO_INODE_BLOCKS;i++)
{
   //i=1 because 0 is superblock
   union fs_block *f=(union fs_block*)malloc(sizeof(union fs_block));
   int j;
   for(j=0;j<INODES_PER_BLOCK;j++)
      {
	 f->inode[j].isvalid=ilist[icount].valid; 
         f->inode[j].blocknum=i;
         f->inode[j].inum=ilist[icount].inum;
	 f->inode[j].datablocknum=ilist[icount].datablocknum; 
         icount++;
      }
   writeLog("Updating inode block from ilist");   
   disk_write(i,f);
   

}


}
void home_init()
{
//initializing home directory
//inode num is 1 because 0 is an invalid inode number
struct dirBlock *home=(struct dirBlock*)malloc(sizeof(struct dirBlock));

//home->names[0]=(char*)malloc(sizeof(char)*2);
strcpy(home->names[0],".\0"); //current 
home->inum[0]=1;//inode number of home

//home->names[1]=(char*)malloc(sizeof(char)*3);
strcpy(home->names[1],"..\0"); //parent 
home->inum[1]=1;// in this case, inode number of home

int i;
for(i=2;i<DIR_LEN;i++)
{
//home->names[i]=(char*)malloc(sizeof(char)*20);
home->inum[i]=0; //invalid inode number
}

union fs_block *block=(union fs_block*)malloc(sizeof(union fs_block));
block->dir=*home;
writeLog("writing home directory block");
disk_write(NO_INODE_BLOCKS+1,block); //first data block to be written
ilist[0].inum=1;
ilist[0].valid=1;
ilist[0].datablocknum=NO_INODE_BLOCKS+1; //26 inode blocks 1 superblock (0 based indexing)
update_inode_mem();
printf("\nHome directory block number %d\n",NO_INODE_BLOCKS+1);
}

int fs_format()
{	
/*fs_format - Creates a new filesystem on the disk, destroying any data already present. Sets aside ten percent of the blocks for inodes, clears the inode table, and writes the superblock. Returns one on success, zero otherwise. Note that formatting a filesystem does not cause it to be mounted. Also, an attempt to format an already-mounted disk should do nothing and return failure. */       
	printf("\nDo you want to format disk (y/n)? ");
        char ch;
        scanf("%c",&ch);
	if(ch=='y')
	{
         writeLog("FORMATTING disk");
	 disk_init("disk.dat","w+",256);
	 super_init();
	 inodeBlocks_init();
         home_init();
	 update_freeblock_list();
         printf("\nEnd of format\n");
         return 1;
	}
        else
	{
	printf("\nOpening disk file in r+ mode\n");
	disk_init("disk.dat","r+",256);
        //read inode blocks to fill ilist (in memory)
	printf("\nCalling fill_ilist\n");
        fill_ilist();
	printf("\nCalling freeblock_list\n");
	update_freeblock_list();
	
	
	printf("\nInside format\n");

	return 0;
	}
}

void fs_debug()
{
	union fs_block *block=(union fs_block*)malloc(sizeof(union fs_block));

	disk_read(0,block);

	printf("superblock:\n");
	printf("    %d blocks\n",block->super.nblocks);
	printf("    %d inode blocks\n",block->super.ninodeblocks);
	printf("    %d inodes\n",block->super.ninodes);
}


int fs_mount()
{
/*fs_mount - Examine the disk for a filesystem. If one is present, read the superblock, build a free block bitmap, and prepare the filesystem for use. Return one on success, zero otherwise. Note that a successful mount is a pre-requisite for the remaining calls.*/

	return 0;
}

int fs_create()
{
//fs_create - Create a new inode of zero length. On success, return the (positive) inumber. On failure, return zero. (Note that this implies zero cannot be a valid inumber.)
int i;
for(i=0;i<NO_OF_INODES;i++)
if(ilist[i].valid==0) return ilist[i].inum;

  return 0;
}

int fs_delete( int inumber )
{
//fs_delete - Delete the inode indicated by the inumber. Release all data and indirect blocks assigned to this inode and return them to the free block map. On success, return one. On failure, return 0.

        ilist[inumber-1].valid=0;
	return 1;
}

int fs_getsize( int inumber )
{
//fs_getsize - Return the logical size of the given inode, in bytes. Note that zero is a valid logical size for an inode! On failure, return -1.	
	return -1;
}

int fs_read( int inumber, char *data, int length, int offset )
{
/*fs_read - Read data from a valid inode. Copy "length" bytes from the inode into the "data" pointer, starting at "offset" in the inode. Return the total number of bytes read. The number of bytes actually read could be smaller than the number of bytes requested, perhaps if the end of the inode is reached. If the given inumber is invalid, or any other error is encountered, return 0.*/	
	return 0;
}

int fs_write( int inumber, const char *data, int length, int offset )
{
/*fs_write - Write data to a valid inode. Copy "length" bytes from the pointer "data" into the inode starting at "offset" bytes. Allocate any necessary direct and indirect blocks in the process. Return the number of bytes actually written. The number of bytes actually written could be smaller than the number of bytes request, perhaps if the disk becomes full. If the given inumber is invalid, or any other error is encountered, return 0.*/ 	
	return 0;
}
