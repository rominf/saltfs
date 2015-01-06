//char **salt_output;

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
#define SALT_OUTPUT_FILE "/proc/saltfs"

//struct kmem_cache *salt_output_cachep;

struct salt_userspace_output salt_output = {
		.lines = NULL,
		.line_count = 0,
};

int salt_list(char const salt_list_cmd[])
{
	int result;
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
	salt_list_cmd_full = vstrcat(
			"complete --do-complete='salt_common --' >/dev/null; and ",
			salt_list_cmd, " >"
					SALT_OUTPUT_FILE,
//					"/tmp/saltfs",
			(char *)NULL
	);
	argv[2] = salt_list_cmd_full;
	pr_debug("saltfs: executing usermodehelper; argv: '%s', '%s', '%s'\n",
			argv[0], argv[1], argv[2]);
	result = call_usermodehelper(argv[0], argv, envp, UMH_WAIT_PROC);
	kfree(salt_list_cmd_full);
	return result;
}

static void proc_input_flush_line(char *line, unsigned int const line_length)
{
	if (line_length) {
		line[line_length] = '\0';
		salt_output.lines[salt_output.line_count] =
				(char *)kcalloc(line_length + 1, sizeof(char), GFP_KERNEL);
		salt_output.lines[salt_output.line_count] =
				strncpy(salt_output.lines[salt_output.line_count], line, line_length);
		pr_debug("saltfs: read '%s'\n", salt_output.lines[salt_output.line_count]);
		salt_output.line_count++;
	}
}

static ssize_t proc_input(struct file *filp, const char *buff, size_t len, loff_t *off)
{
	int i;
	unsigned int line_length;
	char *line;
	line = (char *)kcalloc(SALT_OUTPUT_MAX_LINE_LENGTH, sizeof(char), GFP_KERNEL);
	line_length = 0;
	pr_debug("saltfs: start reading from proc\n");
	for (i = 0; line_length < SALT_OUTPUT_MAX_LINE_LENGTH - 1 && i < len; i++, line_length++) {
		get_user(line[line_length], buff + i);
		if (line[line_length] == '\n') {
			proc_input_flush_line(line, line_length);
			line_length = -1;
		}
	}
	proc_input_flush_line(line, line_length);
	kfree(line);
	pr_debug("saltfs: read finished");
	return len;
}

static const struct file_operations proc_fops = {
	.owner = THIS_MODULE,
	.write = proc_input,
	.llseek = seq_lseek,
};

void init_proc(void)
{
	pr_debug("saltfs: init proc started\n");
	salt_output.lines = (char **)kcalloc(SALT_OUTPUT_MAX_LINE_COUNT,
			sizeof(char *), GFP_KERNEL);
	proc_create(NAME, 0, NULL, &proc_fops);
	pr_debug("saltfs: inited proc\n");
//	salt_output_cachep = kmem_cache_create(
//			"salt_output",  SALT_OUTPUT_LINE_LENGTH, ARCH_MIN_TASKALIGN)
}

void shutdown_proc(void)
{
	int i;
	pr_debug("saltfs: shutdown proc started\n");

	remove_proc_entry(NAME, NULL);
	for (i = 0; i < salt_output.line_count; i++) {
		kfree(salt_output.lines[i]);
	}
	kfree(salt_output.lines);

	pr_debug("saltfs: shutdown proc\n");
}
