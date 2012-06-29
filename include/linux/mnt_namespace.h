#ifndef _NAMESPACE_H_
#define _NAMESPACE_H_
#ifdef __KERNEL__

struct mnt_namespace;
struct fs_struct;

extern struct mnt_namespace *copy_mnt_ns(unsigned long, struct mnt_namespace *,
		struct fs_struct *);
extern void put_mnt_ns(struct mnt_namespace *ns);

extern const struct seq_operations mounts_op;
extern const struct seq_operations mountinfo_op;
extern const struct seq_operations mountstats_op;

#endif
#endif
