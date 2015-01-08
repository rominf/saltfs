#include "dir.h"
#include "file.h"
#include "function.h"
#include "internal.h"
#include "string.h"
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

#define SALT_OUTPUT_DIR /proc/
#define SALT_OUTPUT_FILE "/proc/saltfs"
//#define SALT_OUTPUT_FILE(WHAT) (SALT_OUTPUT_DIR ## WHAT)
//#define SALT_OUTPUT_MINIONS_FILE SALT_OUTPUT_DIR ## minions


static int const refresh_delay = 4;  /* in seconds */

extern struct salt_item_spec const salt_items_spec[];

struct salt_inode *parent_(struct salt_inode *si, enum salt_dir_entry_type const type) {
	while (si->type != type)
		si = si->parent;
	return si;
}

struct salt_inode const *parent(struct salt_inode const *si, enum salt_dir_entry_type const type) {
	while (si->type != type)
		si = si->parent;
	return si;
}

static char *list_cmd_root(struct salt_inode const *si) {
	pr_debug("saltfs: get list_cmd_root string\n");
	return vstrcat("", NULL);
}

static char *list_cmd_minion(struct salt_inode const *si) {
	pr_debug("saltfs: get list_cmd_minion string\n");
	return vstrcat("__fish_salt_list_minion accepted", NULL);
}

static char *list_cmd_module(struct salt_inode const *si) {
	char *minion = si->name;
	pr_debug("saltfs: get list_cmd_module string; minion=%s\n", minion);
	return vstrcat(SALT_FISH_SET_MINION(minion),
			"__fish_salt_list_module", NULL);
}

static char *list_cmd_function(struct salt_inode const *si) {
	char const *minion = parent(si, Salt_minion)->name,
			*module = parent(si, Salt_module)->name;
	pr_debug("saltfs: get list_cmd_funtion string; minion=%s, module=%s\n",
			minion, module);
	return vstrcat(SALT_FISH_SET_MINION(minion),
			"__fish_salt_list_function_without_module ", module, NULL);
}

static char *list_cmd_grain(struct salt_inode const *si) {
	char *minion = si->parent->name;
	pr_debug("saltfs: get list_cmd_grain string; minion=%s\n", minion);
	return vstrcat(SALT_FISH_SET_MINION(minion),
			"__fish_salt_list_grain", NULL);
}

struct salt_next_item_spec const salt_module_next_items[] = {
		{ .name = "grains", .type = Salt_grain,    },
		{ .name = "",       .type = Salt_function, },
};

struct salt_item_spec const salt_items_spec[] = {
		{
				.name = "root",
				.list_cmd = list_cmd_root,
//				.fops = NULL,
				.next_item_type = Salt_minion,
				.next_items = NULL,
				.mode = S_IFDIR,
		},
		{
				.name = "minion",
				.list_cmd = list_cmd_minion,
//				.proc_fops = NULL,
				.next_item_type = Salt_module,
				.next_items = NULL,
				.mode = S_IFDIR,
		},
		{
				.name = "module",
				.list_cmd = list_cmd_module,
//				.proc_fops = NULL,
				.next_item_type = Salt_function,
				.next_items = salt_module_next_items,
				.mode = S_IFDIR,
		},
		{
				.name = "function",
				.list_cmd = list_cmd_function,
				.fops = &salt_function_fops,
				.next_item_type = Salt_NULL,
				.next_items = NULL,
				.mode = S_IFREG,
		},
		{
				.name = "grain",
				.list_cmd = list_cmd_grain,
				.fops = &salt_grain_fops,
				.next_item_type = Salt_NULL,
				.next_items = NULL,
				.mode = S_IFREG,
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
	pr_debug("saltfs: new salt_inode filled; i_ino=%zu, name='%s', type=%d\n",
			dir->i_ino, ei->name, ei->type);
}

void update_current_time(struct inode *inode)
{
	inode->i_atime = (struct timespec){0, 0};
	inode->i_mtime = inode->i_ctime = CURRENT_TIME;
}

static struct inode *salt_create_inode(struct inode *dir, struct dentry *dentry,
		enum salt_dir_entry_type const type)
{
	struct inode *inode;
//	umode_t mode = (type == Salt_NULL) ? S_IFREG : S_IFDIR;

	inode = new_inode(dir->i_sb);

	pr_debug("saltfs: new inode created\n");

	if (!inode)
		goto out;

	inode->i_ino = get_next_ino();
	inode->i_op = &simple_dir_inode_operations;
	inode->i_fop = (salt_items_spec[type].fops)?
			salt_items_spec[type].fops : &salt_dir_operations;
	inode_init_owner(inode, NULL, salt_items_spec[type].mode | 0770);
//	inode->i_mode = S_IFDIR|S_IRUGO|S_IXUGO;
	if (salt_items_spec[type].mode == S_IFDIR)
		inode->i_flags |= S_IMMUTABLE;
	update_current_time(inode);
//	if (salt_items_spec[type].mode == S_IFREG)
//		inode->i_atime = CURRENT_TIME;

	pr_debug("saltfs: new inode filled; type=%d, i_ino=%zu, i_mode=%d\n",
			type, inode->i_ino, inode->i_mode);

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
		enum salt_dir_entry_type const type)
{
	int i = 0;
	char *next_item_list_cmd;
	char *salt_item;
	struct salt_userspace_output *salt_output;
	struct salt_next_item_spec const *next_item = salt_items_spec[type].next_items;
	enum salt_dir_entry_type next_item_type;

	if (next_item) {
		while (next_item->name[0] != '\0') {
			if (strcmp(next_item->name, si->name) == 0)
				break;
			next_item++;
		}
		next_item_type = next_item->type;
		pr_debug("saltfs: salt_readdir: next_item: name='%s', type=%d\n",
				next_item->name, next_item->type);
	} else {
		next_item_type = salt_items_spec[type].next_item_type;
	}

	next_item_list_cmd = salt_items_spec[next_item_type].list_cmd(si);
	pr_debug("saltfs: salt_readdir: next_item: list_cmd='%s'\n",
			next_item_list_cmd);

	salt_list(next_item_list_cmd, ino);
	kfree(next_item_list_cmd);

	salt_output = (struct salt_userspace_output *)idr_find(&salt_output_idr, ino);
	for (i = 0; i < salt_output->line_count; i++) {
		salt_item = salt_output->lines[i];
		salt_fill_cache_item(dir, salt_item, strlen(salt_item), next_item_type);
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

	pr_debug("saltfs: salt_readdir: next_item: name='%s', type=%d\n",
			salt_items_spec[next_item_type].name, next_item_type);

	if (CURRENT_TIME.tv_sec - inode->i_atime.tv_sec > refresh_delay) {
		pr_debug("saltfs: salt_readdir: cache invalidation\n");
		inode->i_atime = CURRENT_TIME;
		salt_fill_dir(si, file->f_path.dentry, inode->i_ino, si->type);
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

