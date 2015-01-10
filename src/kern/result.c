#include "dir.h"
#include "file.h"
#include "function.h"
#include "string.h"

#include <asm/uaccess.h>
#include <linux/seq_file.h>
#include <linux/slab.h>

extern int salt_last_result_ino;

static int salt_result_show(struct seq_file *m, void *v)
{
	if (salt_last_result_ino) {
		pr_debug("saltfs: showing result for inode=%d\n", salt_last_result_ino);
		return salt_output_show_ino(m, salt_last_result_ino);
	}
	return EPERM;
}

static int salt_result_open(struct inode *inode, struct file *file)
{
	pr_debug("saltfs: open result\n");
	return single_open(file, salt_result_show, (void *)inode);
}

struct file_operations const salt_result_fops = {
		.open    = salt_result_open,
		.read    = seq_read,
		.llseek  = seq_lseek,
		.release = single_release,
};
