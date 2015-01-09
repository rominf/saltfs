#include "user.h"

#include <asm/uaccess.h>
#include <linux/idr.h>
#include <linux/seq_file.h>

void salt_output_show(struct seq_file *m, struct salt_userspace_output const *salt_output)
{
	int i;
	for (i = 0; i < salt_output->line_count; i++)
		seq_printf(m, "%s\n", salt_output->lines[i]);
}

void salt_output_show_ino(struct seq_file *m, int ino)
{
	salt_output_show(m, idr_find(&salt_output_idr, ino));
}
