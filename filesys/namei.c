#include "namei.h"

int check_data_block_for_next_entry(struct dirBlock *dir_block, char *next_file) { // returns -1 if entry not found
	//finds the inode number of given file name in a given directory
        printf("\nInside check_data_block_for_next_entry\n");
        int len = DIR_LEN;
	int i;
	for(i = 0; i < len; i++){
		if(dir_block->names[i]!=NULL && (strcmp(next_file, dir_block->names[i]) == 0)){
			int inumber = dir_block->inum[i];
			writeLog("Found next inode number");
			printf("\nEnd of Inside check_data_block_for_next_entry- SUCCESS\n");
			printf("\nReturning inode number %d\n",inumber);
			return inumber;
		}
	}
	printf("\nEnd of Inside check_data_block_for_next_entry- FAILURE\n");
	return -1;
} 

int get_inode(int inode_num,struct inode *inode)
{
 //returns the inode from ilist given its number
 printf("\nInside get_inode\n");
 printf("\nInode number %d\n",inode_num);
 *inode= ilist[inode_num-1];
  printf("\nEnd of get_inode\n");
 if(inode->valid==0) return 1;
 else return 0;

}
int namei(const char *path) {
        //accepts only absolute paths
	writeLog("namei called");
	printf("\nInside namei\n");
	/*if(path == NULL || path[0] != '/'){
		fprintf(stderr, "Invalid path name\n");
		writeLog("namei invalid path name");
		return -1;
	}*/printf("\nInside print_home\n");
	struct inode next_inode;
	char *dup_path = strdup(path);
        printf("\nBefore strtok\n");
	char *next_file = strtok(dup_path, "/");
        printf("\nAfter strtok\n");
        printf("\nBefore get_inode\n");
	int next_inode_success = get_inode(HOME_INODE_NO, &next_inode);
	struct dirBlock dir_block;
	int block_id;
      
	while(next_file != NULL && next_inode_success == 0) {

		int next_inode_number = -1;
		int i;
               
                int len=DIR_LEN;
		for(i = 0; i < len; ++i) {
			union fs_block *block=(union fs_block*)malloc(sizeof(union fs_block));
			printf("\nnext_inode.datablocknum %d\n",next_inode.datablocknum);
		        disk_read(next_inode.datablocknum,block);
			dir_block=block->dir;
			printf("\nNext file %s \n",next_file);
			next_inode_number = check_data_block_for_next_entry(&dir_block, next_file);
			return next_inode.inum; //since we are considering only single hierarchy (this function could be easily extended for a multilevel directory system)
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
	printf("\nReturning inode number %d\n",next_inode.inum);
        printf("\nend of namei\n");
	return next_inode.inum; 
	
}
