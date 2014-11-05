#ifndef __SUPER_H__
#define __SUPER_H__

#include <linux/types.h>

struct saltfs_super_block {
	uint32_t	sb_magic;
	uint32_t	sb_inode_blocks;
	uint32_t	sb_block_size;
	uint32_t	sb_root_inode;
	uint32_t	sb_inodes_in_block;
};

static inline struct saltfs_super_block *SALTFS_SB(struct super_block *sb)
{
	return (struct saltfs_super_block *)sb->s_fs_info;
}

#endif /*__SUPER_H__*/
