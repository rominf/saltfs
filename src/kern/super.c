#include "defines.h"

#include <linux/buffer_head.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>

#include "saltfs.h"
#include "super.h"
/* #include "inode.h" */

//static struct saltfs_super_block *saltfs_super_block_read(struct super_block *sb)
//{
//	struct saltfs_super_block *result = (struct saltfs_super_block *)
//			kzalloc(sizeof(struct saltfs_super_block),
//					GFP_NOFS);
//	struct saltfs_disk_super_block *dsb = NULL;
//	struct buffer_head *bh = NULL;
//
//	if (!result) {
//		pr_err("saltfs cannot allocate super block\n");
//		return NULL;
//	}
//
//	bh = sb_bread(sb, 0);
//	if (!bh) {
//		pr_err("cannot read 0 block\n");
//		goto free_memory;
//	}
//
//	dsb = (struct saltfs_disk_super_block *)bh->b_data;
//	saltfs_super_block_fill(result, dsb);
//	brelse(bh);
//
//	if (result->asb_magic != AUFS_MAGIC) {
//		pr_err("wrong magic number %u\n", (unsigned) result->asb_magic);
//		goto free_memory;
//	}
//
//	pr_debug("saltfs super block info:\n"
//					"\tmagic           = %u\n"
//					"\tinode blocks    = %u\n"
//					"\tblock size      = %u\n"
//					"\troot inode      = %u\n"
//					"\tinodes in block = %u\n",
//			(unsigned) result->asb_magic,
//			(unsigned) result->asb_inode_blocks,
//			(unsigned) result->asb_block_size,
//			(unsigned) result->asb_root_inode,
//			(unsigned) result->asb_inodes_in_block);
//
//	return result;
//
//	free_memory:
//	kfree(result);
//	return NULL;
//}

static void saltfs_put_super(struct super_block *sb)
{
	pr_debug("saltfs super block destroyed\n");
}

static struct super_operations const saltfs_super_ops = {
		.put_super = saltfs_put_super,
};

static int saltfs_fill_sb(struct super_block *sb, void *data, int silent)
{
	struct inode *root = NULL;

	sb->s_magic = SALTFS_MAGIC;
	sb->s_op = &saltfs_super_ops;

	root = new_inode(sb);

	root->i_ino = 0;
	root->i_sb = sb;
	root->i_atime = root->i_mtime = root->i_ctime = CURRENT_TIME;
	inode_init_owner(root, NULL, S_IFDIR);

	sb->s_root = d_make_root(root);
	if (!sb->s_root) {
		pr_err("saltfs cannot create root\n");
		return -ENOMEM;
	}

	return 0;
}

static struct dentry *saltfs_mount(struct file_system_type *type, int flags,
		char const *dev, void *data)
{
	struct dentry *const entry = mount_bdev(type, flags, dev,
			data, saltfs_fill_sb);
	if (IS_ERR(entry))
		pr_err("saltfs mounting failed\n");
	else
		pr_debug("saltfs mounted\n");
	return entry;
}

static struct file_system_type saltfs_type = {
		.owner = THIS_MODULE,
		.name = NAME,
		.mount = saltfs_mount,
		.kill_sb = kill_block_super,
		.fs_flags = FS_REQUIRES_DEV
};

static int __init saltfs_init(void)
{
	int ret = register_filesystem(&saltfs_type);
	if (ret != 0) {
//		saltfs_inode_cache_destroy();
		pr_err("cannot register filesystem\n");
		return ret;
	}

	pr_debug("saltfs module loaded\n");

	return 0;
}

static void __exit saltfs_fini(void)
{
	int const ret = unregister_filesystem(&saltfs_type);
	if (ret != 0)
		pr_err("cannot unregister filesystem\n");
//
//	saltfs_inode_cache_destroy();

	pr_debug("saltfs module unloaded\n");
}

module_init(saltfs_init);
module_exit(saltfs_fini);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Roman Inflianskas");
