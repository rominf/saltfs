/* I'm using /proc/saltfs/inode + call_usermodehelper for executing
   salt and reading output.
   Linus Torvalds definitely hates me.
*/

#include "internal.h"
#include "user.h"
#include "string.h"

#include <asm/uaccess.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/slab.h>
#include <linux/string.h>

#define SALT_OUTPUT_MAX_LINE_COUNT 1024
#define SALT_OUTPUT_MAX_LINE_LENGTH 512
#define INT_MAX_STR_LENGTH 7
#define SALT_OUTPUT_PROC_ROOT "/proc/saltfs/"

//struct kmem_cache *salt_output_cachep;

DEFINE_SPINLOCK(salt_idr_output_lock);

struct salt_userspace_output *salt_output = NULL;
struct proc_dir_entry *salt_proc_root;
struct idr salt_output_idr;

struct salt_userspace_output *salt_output_alloc(void)
{
	struct salt_userspace_output *result =
			(struct salt_userspace_output *)kmalloc(sizeof(struct salt_userspace_output), GFP_KERNEL);
	result->lines = (char **)kcalloc(SALT_OUTPUT_MAX_LINE_COUNT,
			sizeof(char *), GFP_KERNEL);
	result->line_count = 0;
	pr_debug("saltfs: allocated salt_output\n");
	return result;
}

void salt_output_free(struct salt_userspace_output *salt_output)
{
	unsigned int i;
	if (salt_output) {
		pr_debug("saltfs: clear salt_output\n");
		for (i = 0; i < salt_output->line_count; i++) {
			kfree(salt_output->lines[i]);
		}
		kfree(salt_output->lines);
		kfree(salt_output);
	}
}

static void proc_input_flush_line(char *line, unsigned int const line_length,
		struct salt_userspace_output *salt_output)
{
	if (line_length) {
		line[line_length] = '\0';
		salt_output->lines[salt_output->line_count] =
				(char *)kcalloc(line_length + 1, sizeof(char), GFP_KERNEL);
		salt_output->lines[salt_output->line_count] =
				strncpy(salt_output->lines[salt_output->line_count], line, line_length);
		pr_debug("saltfs: read '%s'\n", salt_output->lines[salt_output->line_count]);
		salt_output->line_count++;
	}
}

static ssize_t proc_input(struct file *filp, const char *buff, size_t len, loff_t *off)
{
	int i, ino;
	unsigned int line_length;
	char *line;
	struct salt_userspace_output *salt_output;
	sscanf(filp->f_path.dentry->d_name.name, "%d", &ino);
	salt_output = (struct salt_userspace_output *)idr_find(&salt_output_idr, ino);
	line = (char *)kcalloc(SALT_OUTPUT_MAX_LINE_LENGTH, sizeof(char), GFP_KERNEL);
	line_length = 0;
	pr_debug("saltfs: start reading from proc %s%d\n", SALT_OUTPUT_PROC_ROOT, ino);
	for (i = 0; line_length < SALT_OUTPUT_MAX_LINE_LENGTH - 1 && i < len; i++, line_length++) {
		get_user(line[line_length], buff + i);
		if (line[line_length] == '\n') {
			proc_input_flush_line(line, line_length, salt_output);
			line_length = -1;
		}
	}
	proc_input_flush_line(line, line_length, salt_output);
	kfree(line);
	pr_debug("saltfs: read finished\n");
	return len;
}

static const struct file_operations proc_fops = {
		.owner = THIS_MODULE,
		.write = proc_input,
		.llseek = seq_lseek,
};

struct salt_userspace_output *init_proc_output(int const ino, char const *ino_str)
{
	struct salt_userspace_output *result = salt_output_alloc();
	idr_preload(GFP_KERNEL);
	spin_lock(&salt_idr_output_lock);
	idr_alloc(&salt_output_idr, result, ino, ino + 1, GFP_NOWAIT);
	spin_unlock(&salt_idr_output_lock);
	idr_preload_end();
//	if (id < 0)
//		error;
	return result;
}

int salt_list(char const salt_list_cmd[], int const ino)
{
	int result;
	char ino_str[INT_MAX_STR_LENGTH];
	char *salt_list_cmd_full;
	char *argv[] = {
//			"/bin/sh",
			"/usr/bin/fish",  /* Full path here */
			"-c",
			"",
			NULL
	};
	static char *envp[] = {
			"HOME=/",
			"TERM=linux",
			"PATH=/sbin:/bin:/usr/sbin:/usr/bin",
			NULL
	};
	sprintf(ino_str, "%d", ino);
	salt_list_cmd_full = vstrcat(
			"complete --do-complete='salt_common --' >/dev/null; and ",
			salt_list_cmd, " >"
					SALT_OUTPUT_PROC_ROOT, ino_str,
//					"/tmp/saltfs",
			(char *)NULL
	);
	argv[2] = salt_list_cmd_full;

	salt_output = (struct salt_userspace_output *)idr_find(&salt_output_idr, ino);
	if (!salt_output) {
		proc_create(ino_str, 0, salt_proc_root, &proc_fops);
	}
	salt_output_free(salt_output);
	salt_output = init_proc_output(ino, ino_str);

	pr_debug("saltfs: executing usermodehelper; argv: '%s', '%s', '%s'\n",
			argv[0], argv[1], argv[2]);
	result = call_usermodehelper(argv[0], argv, envp, UMH_WAIT_PROC);
	pr_debug("saltfs: executed usermodehelper\n");
	kfree(salt_list_cmd_full);
	return result;
}

void init_proc(void)
{
	idr_init(&salt_output_idr);
	pr_debug("saltfs: init proc started\n");
	salt_proc_root = proc_mkdir(NAME, NULL);
	pr_debug("saltfs: inited proc\n");
}

void shutdown_proc(void)
{
	pr_debug("saltfs: shutdown proc started\n");

	remove_proc_subtree(NAME, NULL);

	pr_debug("saltfs: shutdown proc\n");
}
