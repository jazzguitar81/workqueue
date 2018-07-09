#include <linux/debugfs.h>
#include <linux/err.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/kref.h>
#include <linux/module.h>
#include <linux/seq_file.h>
#include <linux/slab.h>

#define DRIVER_DESC	"Work Queue Test Kernel Module"

struct test_work {
	int	index;
	int	type;
	int	option;

	struct kref kref;
};
struct test_work *tw;

enum {
	TYPE_NONE,
	TYPE_1,
	TYPE_2,
	TYPE_3,
};

/********** debugfs **********/
static struct dentry *dent;

static int type_show(struct seq_file *s, void *unused)
{
	struct test_work *tw = s->private;

	switch (tw->type) {
	case TYPE_1:
		seq_puts(s, "type-1");
		break;
	case TYPE_2:
		seq_puts(s, "type-2");
		break;
	case TYPE_3:
		seq_puts(s, "type-3");
		break;
	default:
		seq_puts(s, "No value");
		break;
	}

	return 0;
}

static int type_open(struct inode *inode, struct file *file)
{
	return single_open(file, type_show, inode->i_private);
}

static ssize_t type_write(struct file *file, const char __user *ubuf,
			  size_t count, loff_t *ppos)
{
	pr_info("%s: called\n", __func__);
	return count;
}

static const struct file_operations type_fops = {
	.open	= type_open,
	.read	= seq_read,
	.write	= type_write,
	.llseek = seq_lseek,
	.release = single_release,
};

static int start_show(struct seq_file *s, void *unused)
{
	pr_info("%s: called\n", __func__);

	return 0;
}

static int start_open(struct inode *inode, struct file *file)
{
	return single_open(file, start_show, inode->i_private);
}

static const struct file_operations start_fops = {
	.open = start_open,
	.release = single_release,
	.read = seq_read,
	.llseek = seq_lseek,
};

static void work_debugfs_init(struct test_work *tw)
{
	struct dentry *file;

	dent = debugfs_create_dir("test_work", 0);
	if (IS_ERR_OR_NULL(dent)) {
		pr_err("%s: failed to debugfs dir create\n", __func__);
		return;
	}

	file = debugfs_create_file("type", 0666, dent, tw, &type_fops);
	if (IS_ERR_OR_NULL(file)) {
		pr_err("%s: failed to debugfs file(type) create\n", __func__);
		debugfs_remove_recursive(dent);
	}

	file = debugfs_create_file("start", 0644, dent, NULL, &start_fops);
	if (IS_ERR_OR_NULL(file)) {
		pr_err("%s: failed to debugfs file(start) create\n", __func__);
		debugfs_remove_recursive(dent);
	}

	return;
}

static void work_debugfs_remove(void)
{
	debugfs_remove_recursive(dent);
}
/********** debugfs **********/

static void cleanup_wq(struct kref *refcount)
{
	struct test_work *tw;

	tw = container_of(refcount, struct test_work, kref);
	kfree(tw);
}

static int __init init_wq(void)
{
	tw = kzalloc(sizeof(struct test_work), GFP_KERNEL);
	if (!tw)
		return -ENOMEM;

	work_debugfs_init(tw);
	kref_init(&tw->kref);

	pr_info("%s: test workqueu module init\n", __func__);

	return 0;
}

static void __exit exit_wq(void)
{
	pr_info("%s: called\n", __func__);
	kref_put(&tw->kref, cleanup_wq);
	work_debugfs_remove();
}

module_init(init_wq);
module_exit(exit_wq);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION(DRIVER_DESC);
