#ifndef __SALTFS_USER_H__
#define __SALTFS_USER_H__

#include <linux/types.h>

#define SALT_LIST_CMD_FULL_MAX_LENGTH 128
#define SALT_FISH_SET_MINION(minion) \
		"set -g __fish_salt_extracted_minion ", minion, "; and "

struct salt_userspace_output {
	char **lines;
	unsigned int line_count;
};

extern struct salt_userspace_output *salt_output;
extern struct idr salt_output_idr;

int salt_list(char const salt_list_cmd[], int const ino);
void init_proc(void);
void shutdown_proc(void);

#endif /*__SALTFS_USER_H__*/
