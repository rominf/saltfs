#ifndef __SALTFS_H__
#define __SALTFS_H__

#include <linux/types.h>

static uint32_t const SALTFS_MAGIC = 0x42424242;

struct saltfs_disk_super_block {
	__be32	dsb_magic;
	__be32	dsb_block_size;
	__be32	dsb_root_inode;
	__be32	dsb_inode_blocks;
};

struct saltfs_disk_inode {
	__be32	di_first;
	__be32	di_blocks;
	__be32	di_size;
	__be32	di_gid;
	__be32	di_uid;
	__be32	di_mode;
	__be64	di_ctime;
};

#endif /*__SALTFS_H__*/
