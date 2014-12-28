#include "defines.h"

#include <linux/buffer_head.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>

#include "saltfs.h"
#include "super.h"
/* #include "inode.h" */

//static const struct address_space_operations saltfs_aops = {
//		.readpage       = simple_readpage,
//		.write_begin    = simple_write_begin,
//		.write_end      = simple_write_end,
//		.set_page_dirty = __set_page_dirty_no_writeback,
//};
//
//struct inode *saltfs_get_inode(struct super_block *sb, const struct inode *dir, umode_t mode, dev_t dev)
//{
//	struct inode *inode = new_inode(sb);
//
//	if (inode) {
//		inode->i_ino = get_next_ino();
//		inode_init_owner(inode, dir, mode);
//		inode->i_mapping->a_ops = &saltfs_aops;
//		mapping_set_gfp_mask(inode->i_mapping, GFP_HIGHUSER);
//		mapping_set_unevictable(inode->i_mapping);
//		inode->i_atime = inode->i_mtime = inode->i_ctime = CURRENT_TIME;
//		switch (mode & S_IFMT) {
//			default:
//				init_special_inode(inode, mode, dev);
//				break;
//			case S_IFREG:
//				inode->i_op = &saltfs_file_inode_operations;
//				inode->i_fop = &saltfs_file_operations;
//				break;
//			case S_IFDIR:
//				inode->i_op = &saltfs_dir_inode_operations;
//				inode->i_fop = &simple_dir_operations;
//
//				/* directory inodes start off with i_nlink == 2 (for "." entry) */
//				inc_nlink(inode);
//				break;
//			case S_IFLNK:
//				inode->i_op = &page_symlink_inode_operations;
//				break;
//		}
//	}
//	return inode;
//}


static void saltfs_put_super(struct super_block *sb)
{
	pr_debug("saltfs: super block destroyed\n");
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
		pr_err("saltfs: cannot create root\n");
		return -ENOMEM;
	}

	return 0;
}

static struct dentry *saltfs_mount(struct file_system_type *type, int flags,
		char const *dev, void *data)
{
	struct dentry *const entry = mount_nodev(type, flags, data, saltfs_fill_sb);
	if (IS_ERR(entry))
		pr_err("saltfs: mounting failed\n");
	else
		pr_info("saltfs: mounted\n");
	return entry;
}

static void saltfs_kill_sb(struct super_block *sb)
{
	pr_debug("saltfs: trying to umount\n");
	generic_shutdown_super(sb);
	pr_info("saltfs: umounted\n");
}

static struct file_system_type saltfs_type = {
		.owner = THIS_MODULE,
		.name = NAME,
		.mount = saltfs_mount,
		.kill_sb = saltfs_kill_sb,
//		.fs_flags = FS_REQUIRES_DEV
};

static int __init saltfs_init(void)
{
	static unsigned long once;
	int ret;

	if (test_and_set_bit(0, &once)) {
		pr_err("saltfs: filesystem is already mounted, refusing to mount it second time\n");
		return 0;
	}

	ret = register_filesystem(&saltfs_type);
	if (ret != 0) {
//		saltfs_inode_cache_destroy();
		pr_err("saltfs: cannot register filesystem\n");
		return ret;
	}

	pr_debug("saltfs: module loaded\n");

	return 0;
}

static void __exit saltfs_shutdown(void)
{
	int ret;
	pr_debug("saltfs: trying to unload module\n");
	ret = unregister_filesystem(&saltfs_type);
	if (ret != 0)
		pr_err("saltfs: cannot unregister filesystem\n");
//
//	saltfs_inode_cache_destroy();

	pr_debug("saltfs: module unloaded\n");
}

module_init(saltfs_init);
module_exit(saltfs_shutdown);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Roman Inflianskas");
