#include <linux/module.h>
#include <linux/init.h>
#include <linux/poll.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/mm.h>
#include <linux/idr.h>
#include <linux/sched.h>
#include <linux/sched/signal.h>
#include <linux/string.h>
#include <linux/kobject.h>
#include <linux/cdev.h>
#include <linux/uio_driver.h>
#include <linux/nsproxy.h>
#include "docker_container.h"

#define DOCKER_MAX_DEVICE	(64)

static int docker_major;
static struct cdev *docker_cdev;
struct class *docker_class;

const char docker_name[] = "docker_test";

static int docker_open(struct inode *inode, struct file *filep)
{
	struct pid_namespace *pid_ns = NULL;
	struct task_struct *tsk = NULL;
	struct pid *p = NULL;
	pid_t tgid;

	pr_err("current->tgid = %d\n", current->tgid);

	/*
	pid_ns = task_active_pid_ns(current);
	if (pid_ns == NULL) {
		pr_err("get task active pid ns failed\n");
		return -ENOMEM;
	}
	tgid = task_tgid_nr_ns(current, pid_ns);
	*/
	tgid = task_tgid_vnr(current);
	pr_err("container tgid = %d\n", tgid);
	pr_err("current tgid = %d\n", current->tgid);
	pr_err("mnt_ns = %p\n", current->nsproxy->mnt_ns);
	pr_err("pid_ns = %p\n", current->nsproxy->pid_ns_for_children);

	p = find_pid_ns(1, current->nsproxy->pid_ns_for_children);
	if (!p) {
		pr_err("find pid ns failed\n");
		return 0;
	}
	tsk = pid_task(p, PIDTYPE_PID);
	if (!tsk) {
		pr_err("tsk is NULL");
		return 0;
	}
	pr_err("tsk tgid = %d, tsk->comm = %s\n", tsk->tgid, tsk->comm);

	return 0;
}

static int docker_release(struct inode *inode, struct file *filep)
{

	return 0;
}

static const struct file_operations docker_fops = {
	.owner		= THIS_MODULE,
	.open		= docker_open,
	.release	= docker_release,
	/*
	.read		= uio_read,
	.write		= uio_write,
	.mmap		= uio_mmap,
	.poll		= uio_poll,
	.fasync		= uio_fasync,
	.llseek		= noop_llseek,
	*/
};

int docker_register(int i)
{
	device_create(docker_class, NULL, MKDEV(docker_major, i),
		NULL, "docker_dev");
	return 0;
}

int docker_unregister(int i)
{
	device_destroy(docker_class, MKDEV(docker_major, i));
	return 0;
}

static int docker_cdev_init(void)
{
	struct cdev *cdev = NULL;
	dev_t docker_dev = 0;
	int ret;

	docker_class = class_create(THIS_MODULE, docker_name);
	if (IS_ERR(docker_class))
		return -EFAULT;

	ret = alloc_chrdev_region(&docker_dev, 0, DOCKER_MAX_DEVICE, docker_name);
	if (ret)
		goto out;

	ret = -ENOMEM;
	cdev = cdev_alloc();
	if (!cdev)
		goto out_unregister;
	cdev->owner = THIS_MODULE;
	cdev->ops = &docker_fops;
	kobject_set_name(&cdev->kobj, "%s", docker_name);

	ret = cdev_add(cdev, docker_dev, DOCKER_MAX_DEVICE);
	if (ret)
		goto out_put;
	docker_major = MAJOR(docker_dev);
	docker_cdev = cdev;

	docker_register(0);

	pr_err("host mnt_ns = %p\n", current->nsproxy->mnt_ns);
	pr_err("host pid_ns = %p\n", current->nsproxy->pid_ns_for_children);

	return 0;

out_put:
	kobject_put(&cdev->kobj);
out_unregister:
	unregister_chrdev_region(docker_dev, DOCKER_MAX_DEVICE);
out:
	class_destroy(docker_class);
	return -ENODEV;
}

static int docker_cdev_exit(void)
{
	docker_unregister(0);
	class_destroy(docker_class);
	unregister_chrdev_region(MKDEV(docker_major, 0), DOCKER_MAX_DEVICE);
	cdev_del(docker_cdev);
}

static int docker_test_init(void)
{
	docker_cdev_init();
}
static void docker_test_exit(void)
{
	docker_cdev_exit();
}
module_init(docker_test_init);
module_exit(docker_test_exit);

MODULE_LICENSE("Dual BSD/GPL");