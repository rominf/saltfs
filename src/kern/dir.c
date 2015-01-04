#include "dir.h"
#include "internal.h"
#include "user.h"

#include <asm/uaccess.h>
#include <linux/bitops.h>
#include <linux/completion.h>
#include <linux/errno.h>
#include <linux/idr.h>
#include <linux/init.h>
#include <linux/mm.h>
#include <linux/module.h>
#include <linux/mount.h>
#include <linux/namei.h>
#include <linux/printk.h>
#include <linux/proc_fs.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <linux/stat.h>
#include <linux/time.h>
#include <linux/types.h>



DEFINE_SPINLOCK(salt_subdir_lock);

//#define SALT_OUTPUT_DIR /tmp/
#define SALT_OUTPUT_DIR /proc/
#define SALT_OUTPUT_FILE "/proc/saltfs"
//#define SALT_OUTPUT_FILE(WHAT) (SALT_OUTPUT_DIR ## WHAT)
//#define SALT_OUTPUT_MINIONS_FILE SALT_OUTPUT_DIR ## minions

//static int proc_pid_instantiate(struct inode *dir,
//				   struct dentry * dentry,
//				   struct task_struct *task, const void *ptr)
//{
//	struct inode *inode;
//
//	inode = proc_pid_make_inode(dir->i_sb, task);
//	if (!inode)
//		goto out;
//
//	inode->i_mode = S_IFDIR|S_IRUGO|S_IXUGO;
//	inode->i_op = &proc_tgid_base_inode_operations;
//	inode->i_fop = &proc_tgid_base_operations;
//	inode->i_flags|=S_IMMUTABLE;
//
//	set_nlink(inode, 2 + pid_entry_count_dirs(tgid_base_stuff,
//						  ARRAY_SIZE(tgid_base_stuff)));
//
//	d_set_d_op(dentry, &pid_dentry_operations);
//
//	d_add(dentry, inode);
//	/* Close the race of the process dying before we return the dentry */
//	if (pid_revalidate(dentry, 0))
//		return 0;
//out:
//	return -ENOENT;
//}

bool salt_fill_cache_minion(struct file *file, struct dir_context *ctx,
	const char *name, int len)
{
	struct dentry *child, *dir = file->f_path.dentry;
	struct qstr qname = QSTR_INIT(name, len);
	struct inode *inode;
	unsigned type;
	ino_t ino;

	pr_debug("saltfs: salt_fill_cache_minion: variables inited\n");

	child = d_hash_and_lookup(dir, &qname);
	if (!child) {
		child = d_alloc(dir, &qname);
		pr_debug("saltfs: salt_fill_cache_minion: allocated child\n");
	}
	pr_debug("saltfs: salt_fill_cache_minion: trying to init inode and etc.\n");
	inode = child->d_inode;
	pr_debug("saltfs: salt_fill_cache_minion: inited inode %p\n", inode);
	ino = inode->i_ino;
	type = inode->i_mode >> 12;
	pr_debug("saltfs: salt_fill_cache_minion: trying to dput child\n");
	dput(child);

	pr_debug("saltfs: salt_fill_cache_minion: before dir_emit\n");

	return dir_emit(ctx, name, len, ino, type);
}

int salt_readdir_de(/*struct salt_dir_entry *de, */struct file *file, struct dir_context *ctx)
{
	int i;
	char *minion;

	if (!dir_emit_dots(file, ctx))
		return 0;

//	spin_lock(&salt_subdir_lock_subdir_lock);
//	dir_emit(ctx, "test", 4, SALT_ROOT_INO + 1, 0);
//	ctx->pos++;

	prepare_minion_list();
	for (i = 0; i < salt_output.line_count; i++) {
		minion = salt_output.lines[i];
		salt_fill_cache_minion(file, ctx++, minion, strlen(minion));
	}

//	spin_unlock(&salt_subdir_lock);
	return 1;

//	spin_lock(&saltfs_subdir_lock_subdir_lock);
//	de = de->subdir;
//	i = ctx->pos - 2;
//	for (;;) {
//		if (!de) {
//			spin_unlock(&proc_subdir_lock);
//			return 0;
//		}
//		if (!i)
//			break;
//		de = de->next;
//		i--;
//	}
//
//	do {
//		struct proc_dir_entry *next;
//		pde_get(de);
//		spin_unlock(&proc_subdir_lock);
//		if (!dir_emit(ctx, de->name, de->namelen,
//			    de->low_ino, de->mode >> 12)) {
//			pde_put(de);
//			return 0;
//		}
//		spin_lock(&proc_subdir_lock);
//		ctx->pos++;
//		next = de->next;
//		pde_put(de);
//		de = next;
//	} while (de);
//	spin_unlock(&proc_subdir_lock);
//	return 1;
}

int salt_readdir(struct file *file, struct dir_context *ctx)
{
//	struct inode *inode = file_inode(file);
	pr_debug("saltfs: readdir\n");
//	struct salt_inode = kmalloc(PATH_MAX, GFP_KERNEL);
	return salt_readdir_de(/*PDE(inode),*/file, ctx);
}

//struct inode *proc_get_inode(struct super_block *sb, struct proc_dir_entry *de)
//{
//	struct inode *inode = new_inode_pseudo(sb);
//
//	if (inode) {
//		inode->i_ino = de->low_ino;
//		inode->i_mtime = inode->i_atime = inode->i_ctime = CURRENT_TIME;
//		PROC_I(inode)->pde = de;
//
//		if (de->mode) {
//			inode->i_mode = de->mode;
//			inode->i_uid = de->uid;
//			inode->i_gid = de->gid;
//		}
//		if (de->size)
//			inode->i_size = de->size;
//		if (de->nlink)
//			set_nlink(inode, de->nlink);
//		WARN_ON(!de->proc_iops);
//		inode->i_op = de->proc_iops;
//		if (de->proc_fops) {
//			if (S_ISREG(inode->i_mode)) {
//#ifdef CONFIG_COMPAT
//				if (!de->proc_fops->compat_ioctl)
//					inode->i_fop =
//						&proc_reg_file_ops_no_compat;
//				else
//#endif
//				inode->i_fop = &proc_reg_file_ops;
//			} else {
//				inode->i_fop = de->proc_fops;
//			}
//		}
//	} else
//		pde_put(de);
//	return inode;
//}

ssize_t salt_read_dir(struct file *filp, char __user *buf, size_t siz, loff_t *ppos)
{
	pr_debug("saltfs: read_dir\n");
	return -EISDIR;
}

//bool salt_fill_cache(struct file *file, struct dir_context *ctx,
//		const char *name, int len, instantiate_t instantiate,
//		const void *ptr)
//{
//	struct dentry *child, *dir = file->f_path.dentry;
//	struct qstr qname = QSTR_INIT(name, len);
//	struct inode *inode;
//	unsigned type;
//	ino_t ino;
//
//	child = d_hash_and_lookup(dir, &qname);
//	if (!child) {
//		child = d_alloc(dir, &qname);
//		if (!child)
//			goto end_instantiate;
//		if (instantiate(dir->d_inode, child, task, ptr) < 0) {
//			dput(child);
//			goto end_instantiate;
//		}
//	}
//	inode = child->d_inode;
//	ino = inode->i_ino;
//	type = inode->i_mode >> 12;
//	dput(child);
//	return dir_emit(ctx, name, len, ino, type);
//
//	end_instantiate:
//	return dir_emit(ctx, name, len, 1, DT_UNKNOWN);
//}

const struct file_operations salt_dir_operations = {
		.open			= dcache_dir_open,
		.llseek			= generic_file_llseek,
//		.read			= salt_read_dir,
		.read			= generic_read_dir,
		.iterate		= salt_readdir,
};

