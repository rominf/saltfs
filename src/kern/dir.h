#ifndef __DIR_H__
#define __DIR_H__

#include <linux/fs.h>

enum salt_dir_entry_type;
//static struct inode *salt_alloc_inode(struct super_block *sb);

extern const struct file_operations salt_dir_operations;

void salt_fill_salt_inode(struct inode *dir, char const *name,
		enum salt_dir_entry_type const type, struct inode *parent);

#endif /*__DIR_H__*/
