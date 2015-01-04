#ifndef __INODE_H__
#define __INODE_H__

#include <linux/fs.h>

//static struct inode *salt_alloc_inode(struct super_block *sb);
void salt_init_inodecache(void);
struct inode *salt_alloc_inode(struct super_block *sb);

#endif /*__INODE_H__*/
