#include <linux/proc_fs.h>
#include <linux/proc_ns.h>
#include <linux/spinlock.h>
#include <linux/atomic.h>
#include <linux/binfmts.h>

#define NAME "saltfs"
// only root should be able to access saltfs
#define SALTFS_DEFAULT_MODE 0700

#define SALT_ROOT_INO 1

enum salt_dir_entry_type {
	Salt_minion, Salt_module, Salt_function, Salt_grain
};

struct salt_dir_entry {
	unsigned int low_ino;
	umode_t mode;
	nlink_t nlink;
	kuid_t uid;
	kgid_t gid;
	loff_t size;
	u8 namelen;
	char name[];
};

struct salt_inode {
	enum salt_dir_entry_type type;
	struct salt_dir_entry *sde;
	struct inode vfs_inode;
	u8 namelen;
	char name[];
};

static inline struct salt_inode *SALT_I(const struct inode *inode)
{
	return container_of(inode, struct salt_inode, vfs_inode);
}

static inline struct salt_dir_entry *SDE(const struct inode *inode)
{
	return SALT_I(inode)->sde;
}
