#ifndef __SALTFS_FUNCTION_H__
#define __SALTFS_FUNCTION_H__

extern struct file_operations const salt_function_fops;
extern int salt_function_call(char const *minion, char const *function,
		char const *args, int const ino);
extern int salt_function_call_parent_minion(struct inode const *inode,
		char const *function, char const *args);
extern int salt_function_call_inode(struct inode const *inode, char const *args);

extern int salt_last_result_ino;

#endif /*__SALTFS_FUNCTION_H__*/
