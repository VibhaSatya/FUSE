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
	int direct[POINTERS_PER_INODE];
};

struct {
int blocknum;
int inum;
int valid;

}ilist[NO_OF_INODES];

union fs_block {
	struct fs_superblock super;
	struct fs_inode inode[INODES_PER_BLOCK];
	int pointers[POINTERS_PER_BLOCK];
	char data[BLOCKSIZE];
};

void super_init();
void inodeBlocks_init();
int fs_format();
void fs_debug();
int fs_mount();
int fs_delete( int inumber );
int fs_read( int inumber, char *data, int length, int offset );
int fs_write( int inumber, const char *data, int length, int offset );

