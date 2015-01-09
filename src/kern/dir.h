#ifndef __SALTFS_DIR_H__
#define __SALTFS_DIR_H__

#include "internal.h"

#include <linux/fs.h>

struct salt_next_item_spec {
	enum salt_dir_entry_type const type;
	char const *name;
};

struct salt_item_spec {
	char *name;
	char *(*list_cmd)(struct salt_dir_entry const *si);
	struct file_operations const *fops;
	struct inode_operations const *iops;
	enum salt_dir_entry_type next_item_type;
	struct salt_next_item_spec const *next_items;
	umode_t mode;
};

struct salt_dir_entry const *parent(struct salt_dir_entry const *sde,
		enum salt_dir_entry_type const type);
void update_current_time(struct inode *inode);
void salt_dir_entry_create(struct inode *dir, char const *name,
		enum salt_dir_entry_type const type, struct inode *parent);
void salt_fill_dir(struct salt_dir_entry *sde, struct dentry *dir, int const ino,
		enum salt_dir_entry_type const type);
bool salt_fill_cache_item(struct dentry *dir, char const *name, int len,
		enum salt_dir_entry_type const type);

extern struct file_operations const salt_dir_operations;
extern struct dentry_operations const salt_dentry_operations;
extern struct salt_item_spec const salt_items_spec[];

#endif /*__SALTFS_DIR_H__*/
