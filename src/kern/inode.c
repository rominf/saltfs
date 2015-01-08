#include "inode.h"
#include "internal.h"

#include <linux/slab.h>

static struct kmem_cache *salt_inode_cachep;

struct inode *salt_alloc_inode(struct super_block *sb)
{
	struct salt_inode *si;
	struct inode *inode;

	si = (struct salt_inode *)kmem_cache_alloc(salt_inode_cachep, GFP_KERNEL);
	if (!si)
		return NULL;
	si->sde = NULL;
	inode = &si->vfs_inode;
	inode->i_mtime = inode->i_atime = inode->i_ctime = CURRENT_TIME;
	return inode;
}

static void init_once(void *foo)
{
	struct salt_inode *ei = (struct salt_inode *)foo;
	inode_init_once(&ei->vfs_inode);
}

void __init salt_init_inodecache(void)
{
	salt_inode_cachep = kmem_cache_create(
			"salt_inode_cache", sizeof(struct salt_inode), 0,
			(SLAB_RECLAIM_ACCOUNT | SLAB_MEM_SPREAD | SLAB_PANIC), init_once
	);
}
