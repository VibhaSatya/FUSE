#include "common.h"
#include "disk.h"

static int check_data_block_for_next_entry(struct dirBlock *dir_block, char *next_file) { // returns -1 if entry not found
	//finds the inode number of given file name in a given directory
        int len = DIR_LEN;
	int i;
	for(i = 0; i < len; i++){
		if(strcmp(next_file, dir_block->names[i]) == 0){
			int inumber = dir_block->inum[i];
			writeLog("Found next inode number");
			return inumber;
		}
	}
	return -1;
} 

int get_inode(int inode_num,struct inode *inode)
{
 //returns the inode from ilist given its number
 printf("\nInside get_inode\n");
 inode= &ilist[inode_num-1];
 if(inode->valid==0) return 1;
 else return 0;

}
int namei(const char *path) {
        //accepts only absolute paths
	writeLog("namei called");
	printf("\nInside namei\n");
	if(path == NULL || path[0] != '/'){
		fprintf(stderr, "Invalid path name\n");
		writeLog("namei invalid path name");
		return -1;
	}

	char *dup_path = strdup(path);
        printf("\nBefore strtok\n");
	char *next_file = strtok(dup_path, "/");
        printf("\nAfter strtok\n");
	struct inode next_inode;
        printf("\nBefore get_inode\n");
	int next_inode_success = get_inode(HOME_INODE_NO, &next_inode);
	struct dirBlock dir_block;
	int block_id;

	while(next_file != NULL && next_inode_success == 0) {

		int next_inode_number = -1;
		int i;
                printf("Inside");
                int len=DIR_LEN;
		for(i = 0; i < len; ++i) {
			union fs_block *block=(union fs_block*)malloc(sizeof(union fs_block));
		        disk_read(next_inode.blocknum,block);
			dir_block=block->dir;
			next_inode_number = check_data_block_for_next_entry(&dir_block, next_file);
			if(next_inode_number != -1) {
				break;
			}
		}

		if(next_inode_number == -1) {
			free(dup_path);
			return -1;
		}

		next_file = strtok(NULL, '/');
		next_inode_success = get_inode(next_inode_number, &next_inode);
	}

	free(dup_path);
	if(next_inode_success == -1){
		fprintf(stderr, "ERROR. Exiting namei..\n");
		return -1;
	}
	writeLog("got inode number");
        printf("\nend of namei\n");
	return next_inode.inum;
}
