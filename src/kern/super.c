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


static void salt_put_super(struct super_block *sb)
{
	pr_debug("saltfs: super block destroyed\n");
}

static struct super_operations const salt_super_ops = {
		.alloc_inode = salt_alloc_inode,
		.drop_inode = generic_delete_inode,
		.put_super   = salt_put_super,
};

static int salt_fill_sb(struct super_block *sb, void *data, int silent)
{
	struct inode *root = NULL;

	sb->s_flags |= MS_NODIRATIME | MS_NOSUID | MS_NOEXEC;
	sb->s_blocksize = 1024;
	sb->s_blocksize_bits = 10;
	sb->s_time_gran = 1;
	sb->s_magic = SALTFS_MAGIC;
	sb->s_op = &salt_super_ops;

	root = new_inode(sb);
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
	d_set_d_op(sb->s_root, &salt_dentry_operations);

	pr_debug("saltfs: inited root inode\n");
	salt_dir_entry_create(root, "/", Salt_root, root);
	pr_debug("saltfs: filled root salt_dir_entry\n");
	salt_fill_dir(SDE(root), sb->s_root, root->i_ino, Salt_root);
	pr_debug("saltfs: filled root directory\n");

	return 0;
}

static struct dentry *salt_mount(struct file_system_type *type, int flags,
		char const *dev, void *data)
{
	struct dentry *const entry = mount_nodev(type, flags, data, salt_fill_sb);
	if (IS_ERR(entry))
		pr_err("saltfs: mounting failed\n");
	else
		pr_info("saltfs: mounted\n");
	return entry;
}

static void salt_kill_sb(struct super_block *sb)
{
	pr_debug("saltfs: trying to umount\n");
//	kill_anon_super(sb);
	generic_shutdown_super(sb);
//	kill_litter_super(sb);
	pr_info("saltfs: umounted\n");
}

static struct file_system_type salt_fs_type = {
		.owner = THIS_MODULE,
		.name = FS_NAME,
		.mount = salt_mount,
		.kill_sb = salt_kill_sb,
};

static int __init salt_init(void)
{
	static unsigned long once;
	int ret;

	if (test_and_set_bit(0, &once)) {
		pr_err("saltfs: filesystem is already mounted, refusing to mount it second time\n");
		return 0;
	}

	ret = register_filesystem(&salt_fs_type);
	if (ret != 0) {
		pr_err("saltfs: cannot register filesystem\n");
		return ret;
	}

	pr_debug("saltfs: filesystem registered\n");

	init_proc();
	salt_init_inodecache();

	pr_debug("saltfs: module loaded\n");

	return 0;
}

static void __exit salt_shutdown(void)
{
	int ret;
	pr_debug("saltfs: trying to unload module\n");
	ret = unregister_filesystem(&salt_fs_type);
	if (ret != 0)
		pr_err("saltfs: cannot unregister filesystem\n");

	pr_debug("saltfs: filesystem unregistered\n");

	shutdown_proc();

	pr_debug("saltfs: module unloaded\n");
}

module_init(salt_init);
module_exit(salt_shutdown);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Roman Inflianskas");
