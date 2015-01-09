#include "dir.h"
#include "file.h"
#include "string.h"
#include "user.h"

#include <asm/uaccess.h>
#include <linux/idr.h>
#include <linux/seq_file.h>
#include <linux/slab.h>


char const *function_name(struct salt_dir_entry const *sde)
{
	return vstrcat(parent(sde, Salt_module)->name, ".",
			parent(sde, Salt_function)->name, NULL);
}

int salt_function_call(char const *minion, char const *function, char const *args, int const ino)
{
	int i;
	char *cmd = vstrcat("salt ", minion, " ", function, " ", args, NULL);
	pr_debug("saltfs: showing function '%s', ino=%d, cmd='%s'\n", function, ino, cmd);
	salt_list(cmd, ino);
	kfree(cmd);
	salt_output = (struct salt_userspace_output *)idr_find(&salt_output_idr, ino);
	for (i = 0; i < salt_output->line_count; i++)
		pr_info("saltfs: result: %s\n", salt_output->lines[i]);
	return 0;
}

int salt_function_call_parent_minion(struct inode const *inode, char const *function, char const *args)
{
	char const *minion = parent(SDE(inode), Salt_minion)->name;
	return salt_function_call(minion, function, args, inode->i_ino);
}

int salt_function_call_inode(struct inode const *inode, char const *args)
{
	char const *function = function_name(SDE(inode));
	int ret = salt_function_call_parent_minion(inode, function, args);
	kfree(function);
	return ret;
}

static ssize_t salt_function_write(struct file *file,
		char const *buf, size_t count, loff_t *offset)
{
	int i;
	struct inode *inode = file->f_inode;
	char *args = (char *)kcalloc(count + 1, sizeof(char), GFP_KERNEL);
	for (i = 0; i < count; i++)
		get_user(args[i], buf + i);
	if (args[i] == '\n')
		args[i] = '\0';
	salt_function_call_inode(inode, args);
	return count;
}

static int salt_function_show(struct seq_file *m, void *v)
{
	struct inode *inode = (struct inode *)(m->private);
	struct salt_dir_entry *sde = SDE(inode);
	int ino = inode->i_ino;
	char const *minion = parent(sde, Salt_minion)->name;
	char const *function = function_name(sde);
	char *cmd = vstrcat(SALT_FISH_SET_MINION(minion),
			"salt ", minion, " sys.doc ", function, NULL);
	pr_debug("saltfs: showing function doc '%s', ino=%d, cmd='%s'\n",
			function, ino, cmd);
	salt_list(cmd, ino);
	kfree(function);
	kfree(cmd);
	salt_output_show_ino(m, ino);
	return 0;
}

bool is_touch(struct file const *file)
{
	return (file->f_flags & (O_WRONLY|O_CREAT|O_NOCTTY|O_NONBLOCK));
}

static int salt_function_open(struct inode *inode, struct file *file)
{
	pr_debug("saltfs: open function '%s', flags=%d\n",
			SDE(inode)->name, file->f_flags);
	if (is_touch(file))
		salt_function_call_inode(file->f_inode, NULL);
	return single_open(file, salt_function_show, inode);
}

struct file_operations const salt_function_fops = {
		.open       = salt_function_open,
		.read       = seq_read,
		.write      = salt_function_write,
		.llseek		= seq_lseek,
		.release	= single_release,
};
