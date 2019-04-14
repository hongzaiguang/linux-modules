#include <linux/list.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/sched.h>
#include <linux/list.h>

struct devdrv_manager_container_pid {
	struct list_head pid_list_node;

	pid_t tgid;
	unsigned long long start_time;
	unsigned long long real_start_time;
};

struct devdrv_manager_container_item {
	struct list_head pid_list_head;
	struct list_head item_list_node;

	struct mnt_namespace *mnt_ns;
	unsigned int dev_num;
	unsigned int dev_list[64];
};

struct devdrv_manager_container_table {
	struct list_head item_list_head;
};

struct devdrv_manager_container_table *devdrv_container_table = NULL;

int devdrv_manager_container_table_init(void)
{
	struct devdrv_manager_container_item *init_task_item = NULL;

	devdrv_container_table = kzalloc(sizeof(struct devdrv_manager_container_table), GFP_KERNEL);
	if (devdrv_container_table == NULL)
		return -ENOMEM;
	LIST_INIT_HEAD(&devdrv_container_table->item_list_head);

	init_task_item = kzalloc(sizeof(struct devdrv_manager_container_item), GFP_KERNEL);
	if (init_task_item == NULL) {
		kfree(devdrv_container_table);
		return -ENOMEM;
	}
	init_task_item->mnt_ns = init_task.mnt_ns;
	return 0;
}

int devdrv_manager_container_table_exit(void)
{
	kfree(devdrv_container_table);
	devdrv_container_table = NULL;
	return 0;
}