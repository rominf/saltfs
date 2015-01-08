#include "dir.h"
#include "internal.h"
#include "string.h"
#include "user.h"

#include <linux/fs.h>
#include <linux/idr.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/slab.h>
#include <linux/utsname.h>


static int salt_grain_show(struct seq_file *m, void *v)
{
	struct inode *inode = (struct inode *)(m->private);
	struct salt_inode *si = SALT_I(inode);
	int i, ino = inode->i_ino;
	char const *grain = parent(si, Salt_grain)->name;
	char const *minion = parent(si, Salt_minion)->name;
	char *cmd = vstrcat(SALT_FISH_SET_MINION(minion),
			"__fish_salt_grain_read ", grain, NULL);
	pr_debug("saltfs: showing grain '%s', ino=%d, cmd='%s'\n", grain, ino, cmd);
	salt_list(cmd, ino);
	kfree(cmd);
	salt_output = (struct salt_userspace_output *)idr_find(&salt_output_idr, ino);
	for (i = 0; i < salt_output->line_count; i++)
		seq_printf(m, "%s\n", salt_output->lines[i]);
	return 0;
}

static int salt_grain_open(struct inode *inode, struct file *file)
{
	pr_debug("saltfs: open grain '%s'\n", SALT_I(inode)->name);
	return single_open(file, salt_grain_show, (void *)inode);
}

struct file_operations const salt_grain_fops = {
		.open		= salt_grain_open,
		.read		= seq_read,
		.llseek		= seq_lseek,
		.release	= single_release,
};
