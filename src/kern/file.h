struct salt_userspace_output;
struct seq_file;

void salt_output_show(struct seq_file *m, struct salt_userspace_output const *salt_output);
void salt_output_show_ino(struct seq_file *m, int ino);
