//UTILITY FUNCTIONS
#include "common.h"
#include "disk.h"
#include "fs.h"
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
int icount=1; //inode number counter
for(i=1;i<=NO_INODE_BLOCKS;i++)
{
   //i=1 because 0 is superblock
   union fs_block *f=(union fs_block*)malloc(sizeof(union fs_block));
   int j;
   for(j=0;j<INODES_PER_BLOCK;j++)
      {
	 f->inode[j].isvalid=0; //not valid =>no data
	 ilist[icount].blocknum=i;
         ilist[icount].inum=icount;
         ilist[icount].valid=0;
         icount++;
      }
   writeLog("Initializing inode");   
   disk_write(i,f);
  

}

}
int fs_format()
{	
/*fs_format - Creates a new filesystem on the disk, destroying any data already present. Sets aside ten percent of the blocks for inodes, clears the inode table, and writes the superblock. Returns one on success, zero otherwise. Note that formatting a filesystem does not cause it to be mounted. Also, an attempt to format an already-mounted disk should do nothing and return failure. */       
	printf("\nDo you want to format disk? y/n ");
        char ch;
        scanf("%c",&ch);
	if(ch=='y')
	{
         writeLog("FORMATTING disk");
	 disk_init("disk.dat","w+",256);
	 super_init();
	 inodeBlocks_init();
         return 1;
	}
        else
	{
	disk_init("disk.dat","r+",256);
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
