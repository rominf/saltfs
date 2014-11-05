#include <linux/buffer_head.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>

//#include "aufs.h"
/* #include "inode.h" */
//#include "super.h"

static int __init saltfs_init(void)
{
//	ret = register_filesystem(&saltfs_type);
//	if (ret != 0) {
//		saltfs_inode_cache_destroy();
//		pr_err("cannot register filesystem\n");
//		return ret;
//	}

	pr_debug("saltfs module loaded\n");

	return 0;
}

static void __exit saltfs_fini(void)
{
//	int const ret = unregister_filesystem(&saltfs_type);
//	if (ret != 0)
//		pr_err("cannot unregister filesystem\n");
//
//	saltfs_inode_cache_destroy();

	pr_debug("saltfs module unloaded\n");
}

module_init(saltfs_init);
module_exit(saltfs_fini);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Roman Inflianskas");
