#include "dir.h"
#include "internal.h"
#include "inode.h"
#include "saltfs.h"
#include "super.h"
#include "user.h"

#include <linux/buffer_head.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>

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


static void salt_put_super(struct super_block *sb)
{
	pr_debug("saltfs: super block destroyed\n");
}

static struct super_operations const salt_super_ops = {
		.alloc_inode = salt_alloc_inode,
		.put_super   = salt_put_super,
};

static int saltfs_fill_sb(struct super_block *sb, void *data, int silent)
{
	struct inode *root = NULL;
	struct salt_inode *ei;

	sb->s_flags |= MS_NODIRATIME | MS_NOSUID | MS_NOEXEC;
	sb->s_blocksize = 1024;
	sb->s_blocksize_bits = 10;
	sb->s_time_gran = 1;
	sb->s_magic = SALTFS_MAGIC;
	sb->s_op = &salt_super_ops;

	root = new_inode(sb);
	ei = SALT_I(root);
	ei->type = Salt_root;
	root->i_ino = SALT_ROOT_INO;
	root->i_sb = sb;
	root->i_op = &simple_dir_inode_operations;
	root->i_fop = &salt_dir_operations;
	inode_init_owner(root, NULL, S_IFDIR | 0770);
	update_current_time(root);

	sb->s_root = d_make_root(root);
	if (!sb->s_root) {
		pr_err("saltfs: cannot create root\n");
		return -ENOMEM;
	}

	salt_fill_salt_inode(root, "/", Salt_root, root);
	salt_fill_dir(ei, sb->s_root, root->i_ino, Salt_root);

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

	pr_debug("saltfs: filesystem registered\n");

	init_proc();
	salt_init_inodecache();

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

	pr_debug("saltfs: filesystem unregistered\n");
//
//	saltfs_inode_cache_destroy();

	shutdown_proc();

	pr_debug("saltfs: module unloaded\n");
}

module_init(saltfs_init);
module_exit(saltfs_shutdown);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Roman Inflianskas");
