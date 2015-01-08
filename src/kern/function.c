#include "dir.h"
#include "internal.h"
#include "string.h"
#include "user.h"

#include <asm/uaccess.h>
#include <linux/fs.h>
#include <linux/idr.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/slab.h>
#include <linux/utsname.h>


//void salt_function_call(char const *minion, char const)


static ssize_t salt_function_write(struct file *file,
		char const *buf, size_t count, loff_t *offset)
{
	struct inode *inode = file->f_inode;
	struct salt_inode *si = SALT_I(inode);
	int i, ino = inode->i_ino;
	char const *minion = parent(si, Salt_minion)->name;
	char const *module = parent(si, Salt_module)->name;
	char const *function = parent(si, Salt_function)->name;
	char *cmd, *args = (char *)kcalloc(count + 1, sizeof(char), GFP_KERNEL);
	for (i = 0; i < count; i++)
		get_user(args[i], buf + i);
	if (args[i] == '\n')
		args[i] = '\0';
	cmd = vstrcat("salt ", minion, " ", module, ".", function, " ", args, NULL);
	pr_debug("saltfs: showing function '%s', ino=%d, cmd='%s'\n", function, ino, cmd);
	salt_list(cmd, ino);
	kfree(cmd);
	salt_output = (struct salt_userspace_output *)idr_find(&salt_output_idr, ino);
	for (i = 0; i < salt_output->line_count; i++)
		pr_info("saltfs: result: %s\n", salt_output->lines[i]);
	return count;
}

static int salt_function_show(struct seq_file *m, void *v)
{
	pr_debug("saltfs: show function\n");
	seq_printf(m, "%s\n", "ololo");
	return 0;
}

static int salt_function_open(struct inode *inode, struct file *file)
{

	pr_debug("saltfs: open function '%s', flags=%d\n", SALT_I(inode)->name, file->f_flags);
	return single_open(file, salt_function_show, NULL);
}

struct file_operations const salt_function_fops = {
//		.open       = simple_open,
		.open       = salt_function_open,
		.read       = seq_read,
		.write      = salt_function_write,
		.llseek		= seq_lseek,
		.release	= single_release,
};
