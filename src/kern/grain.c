#include "dir.h"
#include "file.h"
#include "function.h"
#include "string.h"
#include "user.h"

#include <asm/uaccess.h>
#include <linux/seq_file.h>
#include <linux/slab.h>


static int salt_grain_show(struct seq_file *m, void *v)
{
	struct inode *inode = (struct inode *)(m->private);
	struct salt_dir_entry *sde = SDE(inode);
	int ino = inode->i_ino;
	char const *grain = parent(sde, Salt_grain)->name;
	char const *minion = parent(sde, Salt_minion)->name;
	char *cmd = vstrcat(SALT_FISH_SET_MINION(minion),
			"__fish_salt_grain_read ", grain, NULL);
	pr_debug("saltfs: showing grain '%s', ino=%d, cmd='%s'\n", grain, ino, cmd);
	salt_list(cmd, ino);
	kfree(cmd);
	salt_output_show_ino(m, ino);
	return 0;
}

static int salt_grain_open(struct inode *inode, struct file *file)
{
	pr_debug("saltfs: open grain '%s'\n", SDE(inode)->name);
	return single_open(file, salt_grain_show, (void *)inode);
}

static ssize_t salt_grain_write(struct file *file,
		char const *buf, size_t count, loff_t *offset)
{
	int i;
	struct inode *inode = file->f_inode;
	char const *key = SDE(inode)->name;
	char *args, *value = (char *)kcalloc(count + 1, sizeof(char), GFP_KERNEL);
	for (i = 0; i < count; i++)
		get_user(value[i], buf + i);
	if (value[i] == '\n')
		value[i] = '\0';
	args = vstrcat(key, " ", value, NULL);
	salt_function_call_parent_minion(inode, "grains.setval", args);
	kfree(args);
	return count;
}

struct file_operations const salt_grain_fops = {
		.open    = salt_grain_open,
		.read    = seq_read,
		.write   = salt_grain_write,
		.llseek  = seq_lseek,
		.release = single_release,
};
