#include "common.h"
#include "disk.h"

int check_data_block_for_next_entry(struct dirBlock *dir_block, char *next_file);
int get_inode(int inode_num,struct inode *inode);
int namei(const char *path);
