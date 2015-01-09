struct salt_userspace_output;
struct seq_file;

int salt_output_show(struct seq_file *m, struct salt_userspace_output const *salt_output);
int salt_output_show_ino(struct seq_file *m, int ino);
