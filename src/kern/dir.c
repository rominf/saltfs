#include "dir.h"
#include "internal.h"
#include "user.h"
#include "string.h"

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


static int const refresh_delay = 8;  /* in seconds */

extern struct salt_item_spec const salt_items_spec[];

static char *list_cmd_root(struct salt_inode const *si) {
	pr_debug("saltfs: get list_cmd_root string\n");
	return vstrcat("",
			(char *)NULL);
}

static char *list_cmd_minion(struct salt_inode const *si) {
	pr_debug("saltfs: get list_cmd_minion string\n");
	return vstrcat("__fish_salt_list_minion accepted",
			(char *)NULL);
//	return salt_items_spec[Salt_minion].list_cmd_template;
}

static char *list_cmd_module(struct salt_inode const *si) {
	char *minion = si->name;
	pr_debug("saltfs: get list_cmd_module string; minion=%s\n", minion);
	return vstrcat("set -g __fish_salt_program salt; and ",
			"set -g __fish_salt_extracted_minion ", minion, "; and ",
//			"__fish_salt_list_minion accepted",
//			"__fish_salt_list_module | sed '2,$d'",
			"__fish_salt_list_funtion acl | sed '2,$d'",
//			"salt --out txt minion test.ping",
			(char *)NULL);
}

struct salt_item_spec const salt_items_spec[] = {
		{
				.name = "root",
				.list_cmd = list_cmd_root,
				.next_item_type = Salt_minion,
		},
		{
				.name = "minion",
				.list_cmd = list_cmd_minion,
				.next_item_type = Salt_module,
		},
		{
				.name = "module",
				.list_cmd = list_cmd_module,
				.next_item_type = Salt_module,
		},
};

void salt_fill_salt_inode(
		struct inode *dir,
		char const *name,
		enum salt_dir_entry_type const type,
		struct inode *parent
)
{
	struct salt_inode *ei;
	unsigned int len = strlen(name);

	ei = SALT_I(dir);
	ei->name = kmalloc(len + 1, GFP_KERNEL);
	memcpy(ei->name, name, len + 1);
	ei->type = type;
	ei->parent = SALT_I(parent);
	pr_debug("saltfs: new salt_inode filled; i_ino=%zu, name=%s, type=%d\n",
			dir->i_ino, ei->name, ei->type);
}

void update_current_time(struct inode *inode)
{
	inode->i_atime = inode->i_mtime = inode->i_ctime = CURRENT_TIME;
}

static struct inode *salt_create_inode(struct inode *dir, struct dentry *dentry,
		enum salt_dir_entry_type const type)
{
	struct inode *inode;

	inode = new_inode(dir->i_sb);

	pr_debug("saltfs: new inode created\n");

	if (!inode)
		goto out;

	inode->i_ino = get_next_ino();
	inode->i_op = &simple_dir_inode_operations;
	inode->i_fop = &salt_dir_operations;
	inode_init_owner(inode, NULL, S_IFDIR | 0770);
//	inode->i_mode = S_IFDIR|S_IRUGO|S_IXUGO;
	inode->i_flags |= S_IMMUTABLE;
	update_current_time(inode);

	pr_debug("saltfs: new inode filled; i_ino=%zu\n", inode->i_ino);

out:
	return inode;
//	return -ENOENT;
}

bool salt_fill_cache_item(struct dentry *dir, char const *name, int len,
		enum salt_dir_entry_type const type)
{
	struct dentry *child;
	struct qstr qname = QSTR_INIT(name, len);
	struct inode *inode, *parent_inode = dir->d_inode;
//	unsigned type;
//	ino_t ino;

	pr_debug("saltfs: salt_fill_cache_item: name='%s', i_ino: %zu; variables inited\n",
			name, parent_inode->i_ino);

	child = d_hash_and_lookup(dir, &qname);
	if (!child) {
		child = d_alloc(dir, &qname);
		pr_debug("saltfs: salt_fill_cache_item: allocated child\n");

		inode = salt_create_inode(parent_inode, dir, type);
		salt_fill_salt_inode(inode, name, type, parent_inode);

		d_add(child, inode);
		pr_debug("saltfs: new dentry cached\n");
	}
//	pr_debug("saltfs: salt_fill_cache_item: trying to init inode and etc.\n");
//	inode = child->d_inode;
//	pr_debug("saltfs: salt_fill_cache_item: inited inode %p\n", inode);
//	ino = inode->i_ino;
//	type = inode->i_mode >> 12;
//	pr_debug("saltfs: salt_fill_cache_item: trying to dput child\n");
//	dput(child);
//
//	pr_debug("saltfs: salt_fill_cache_item: before dir_emit\n");

	return 0;
}

void salt_fill_dir(struct salt_inode *si, struct dentry *dir, int const ino,
		enum salt_dir_entry_type const next_item_type)
{
	int i;
	char *next_item_list_cmd;
	char *salt_item;
	struct salt_userspace_output *salt_output;
	next_item_list_cmd = salt_items_spec[next_item_type].list_cmd(si);
	pr_debug("saltfs: salt_readdir: next_item: list_cmd='%s'\n",
			next_item_list_cmd);
	salt_list(next_item_list_cmd, ino);
	kfree(next_item_list_cmd);

	salt_output = (struct salt_userspace_output *)idr_find(&salt_output_idr, ino);
	for (i = 0; i < salt_output->line_count; i++) {
		salt_item = salt_output->lines[i];
		//		pr_debug("saltfs: caching item '%s'\n", salt_item);
		salt_fill_cache_item(
				dir, salt_item, strlen(salt_item), next_item_type);
		//		pr_debug("saltfs: cached item '%s'\n", salt_item);
	}
}

static int salt_readdir(struct file *file, struct dir_context *ctx)
{
	struct inode *inode = file->f_inode;
	struct salt_inode *si = SALT_I(inode);
	enum salt_dir_entry_type next_item_type = salt_items_spec[si->type].next_item_type;

#ifdef DEBUG
	int const path_len = 1024;
	char buf[path_len];
	char *path;

	path = dentry_path_raw(file->f_path.dentry, buf, path_len - 1);
	pr_debug("saltfs: salt_readdir: called with dir '%s', type %d, i_ino=%zu\n",
			path, si->type, inode->i_ino);
#endif

	pr_debug("saltfs: salt_readdir: next_item: name=%s, type=%d\n",
			salt_items_spec[next_item_type].name, next_item_type);

	if (CURRENT_TIME.tv_sec - inode->i_atime.tv_sec > refresh_delay) {
		pr_debug("saltfs: salt_readdir: cache invalidation\n");
		update_current_time(inode);
		salt_fill_dir(si, file->f_path.dentry, inode->i_ino, next_item_type);
	}

	return dcache_readdir(file, ctx);
}

ssize_t salt_read_dir(struct file *filp, char __user *buf, size_t siz, loff_t *ppos)
{
	pr_debug("saltfs: read_dir\n");
	return -EISDIR;
}


struct file_operations const salt_dir_operations = {
		.open			= dcache_dir_open,
		.llseek			= generic_file_llseek,
		.read			= generic_read_dir,
		.iterate		= salt_readdir,
};

