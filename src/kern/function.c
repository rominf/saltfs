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


char const *function_name(struct salt_dir_entry const *sde)
{
	return vstrcat(parent(sde, Salt_module)->name, ".",
			parent(sde, Salt_function)->name, NULL);
}

static ssize_t salt_function_write(struct file *file,
		char const *buf, size_t count, loff_t *offset)
{
	struct inode *inode = file->f_inode;
	struct salt_dir_entry *sde = SDE(inode);
	int i, ino = inode->i_ino;
	char const *minion = parent(sde, Salt_minion)->name;
	char const *function = function_name(sde);
	char *cmd, *args = (char *)kcalloc(count + 1, sizeof(char), GFP_KERNEL);
	for (i = 0; i < count; i++)
		get_user(args[i], buf + i);
	if (args[i] == '\n')
		args[i] = '\0';
	cmd = vstrcat("salt ", minion, " ", function, " ", args, NULL);
	pr_debug("saltfs: showing function '%s', ino=%d, cmd='%s'\n", function, ino, cmd);
	salt_list(cmd, ino);
	kfree(function);
	kfree(cmd);
	salt_output = (struct salt_userspace_output *)idr_find(&salt_output_idr, ino);
	for (i = 0; i < salt_output->line_count; i++)
		pr_info("saltfs: result: %s\n", salt_output->lines[i]);
	return count;
}

static int salt_function_show(struct seq_file *m, void *v)
{
	struct inode *inode = (struct inode *)(m->private);
	struct salt_dir_entry *sde = SDE(inode);
	int i, ino = inode->i_ino;
	char const *minion = parent(sde, Salt_minion)->name;
	char const *function = function_name(sde);
	char *cmd = vstrcat(SALT_FISH_SET_MINION(minion),
			"salt ", minion, " sys.doc ", function, NULL);
	pr_debug("saltfs: showing function doc '%s', ino=%d, cmd='%s'\n", function, ino, cmd);
	salt_list(cmd, ino);
	kfree(function);
	kfree(cmd);
	salt_output = (struct salt_userspace_output *)idr_find(&salt_output_idr, ino);
	for (i = 0; i < salt_output->line_count; i++)
		seq_printf(m, "%s\n", salt_output->lines[i]);
	return 0;
}

static int salt_function_open(struct inode *inode, struct file *file)
{
	pr_debug("saltfs: open function '%s', flags=%d\n",
			SDE(inode)->name, file->f_flags);
	return single_open(file, salt_function_show, inode);
}

struct file_operations const salt_function_fops = {
		.open       = salt_function_open,
		.read       = seq_read,
		.write      = salt_function_write,
		.llseek		= seq_lseek,
		.release	= single_release,
};
