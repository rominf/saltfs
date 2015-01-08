#ifndef __SALTFS_INODE_H__
#define __SALTFS_INODE_H__

#include <linux/fs.h>

void salt_init_inodecache(void);
struct inode *salt_alloc_inode(struct super_block *sb);

#endif /*__SALTFS_INODE_H__*/
