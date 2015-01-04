#ifndef __USER_H__
#define __USER_H__

struct salt_userspace_output {
	char **lines;
	u8 line_count;
};

extern struct salt_userspace_output salt_output;

int prepare_minion_list(void);
void init_proc(void);
void shutdown_proc(void);

#endif /*__USER_H__*/
