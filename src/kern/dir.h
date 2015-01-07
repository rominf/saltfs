#ifndef __SALTFS_DIR_H__
#define __SALTFS_DIR_H__

#include "internal.h"
#include <linux/fs.h>

struct salt_item_spec {
	char *name;
	char *(*list_cmd)(struct salt_inode const *si);
	enum salt_dir_entry_type next_item_type;
	umode_t mode;
//	char *dir_name;
//	char *list_cmd_template;
};

//static struct inode *salt_alloc_inode(struct super_block *sb);

void update_current_time(struct inode *inode);
void salt_fill_salt_inode(struct inode *dir, char const *name,
		enum salt_dir_entry_type const type, struct inode *parent);
void salt_fill_dir(struct salt_inode *si, struct dentry *dir, int const ino,
		enum salt_dir_entry_type const next_item_type);

extern struct file_operations const salt_dir_operations;
extern struct salt_item_spec const salt_items_spec[];

#endif /*__SALTFS_DIR_H__*/
