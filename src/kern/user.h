#ifndef __USER_H__
#define __USER_H__

#define SALT_LIST_CMD_FULL_MAX_LENGTH 128

struct salt_userspace_output {
	char **lines;
	u8 line_count;
};

extern struct salt_userspace_output salt_output;

int salt_list(char const salt_list_cmd[]);
void init_proc(void);
void shutdown_proc(void);

#endif /*__USER_H__*/
