/*
 * arch/arm/mach-tegra/eprj_sysfs_tools.c
 *   -- EternityProject sysfs Tools --
 *
 * Copyright (C) 2012, EternityProject Development
 *
 * Author:
 *      Angelo G. Del Regno <kholk11@gmail.com>
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * History:
 * Date		| Comment					| Author
 * 09/09/2012	  Cleanup, improved expandeability, better vars	  kholk <kholk11@gmail.com>
 * 09/09/2012	  Use for EternityProject HSMGR Modifications	  kholk <kholk11@gmail.com>
 * 08/08/2012	  Initial revision.				  kholk <kholk11@gmail.com>
 */

#include <linux/cpu.h>
#include <linux/delay.h>
#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/wakelock.h>

#include <asm/uaccess.h>
#include <mach/eternityproject.h>

static struct eprj_sysfs android_release = {
	.attr.name = "android_apirev",
	.attr.mode = 0644,
	.value = ANDROID_API_ICS, /* By default, we assume we are working with ICS */
};

static struct eprj_sysfs pwrmode = {
	.attr.name = "power_lock",
	.attr.mode = 0666,
	.value = 0, /* 0 = Don't lock in LP -- 1 = Lock in LP Mode */
};

struct attribute * eprjmanager[] = {
	&android_release.attr,
	&pwrmode.attr,
	NULL
};

typedef enum
{
	EPRJ_INVALID = 0,
	HSMGR_APIREV,
	POWER_LOCK
} eprj_attribute;

static eprj_attribute calling_attribute(const char *attrname)
{
	if (!strcmp(attrname, "android_apirev"))
		return HSMGR_APIREV;
	if (!strcmp(attrname, "power_lock"))
		return POWER_LOCK;

	return EPRJ_INVALID;
}

static ssize_t eprjsysfs_show(struct kobject *kobj, struct attribute *attr,
				char *buf)
{
	struct eprj_sysfs *entry = container_of(attr, struct eprj_sysfs, attr);
	return scnprintf(buf, PAGE_SIZE, "%d", entry->value);
}

/*
 * TODO (not urgent):	Use switch in eprjsysfs_store instead of
 *			a bunch of if statements. It's cleaner.
 */
static ssize_t eprjsysfs_store(struct kobject *kobj, struct attribute *attr,
				 const char *buf, size_t len)
{
	struct eprj_sysfs *entry = container_of(attr, struct eprj_sysfs, attr);
	eprj_attribute mynameis;

	sscanf(buf, "%d", &entry->value);
	mynameis = calling_attribute(entry->attr.name);

	switch (mynameis)
	{
		case HSMGR_APIREV:
			if (entry->value == ANDROID_API_JB) {	
				EPRJ_PRINT("EternityProject HSMGR: Android Jellybean detected.\n");
				eprj_hsmgr_35mm_os(ANDROID_API_JB);
			}
			break;
		case POWER_LOCK:
			eprj_extreme_powersave(entry->value);
			break;
		default:
			break;
	}

	return sizeof(int);
}

static struct sysfs_ops eprjsysfs_ops = {
	.show = eprjsysfs_show,
	.store = eprjsysfs_store,
};

static struct kobj_type eprjsysfs_type = {
	.sysfs_ops = &eprjsysfs_ops,
	.default_attrs = eprjmanager,
};

/* Thanks nVidia for that piggy bad thing. You're making me to act bad and lazy. */
void set_sysfs_param(char *param_path, char *name, char *value)
{
	struct file *gov_param = NULL;
	mm_segment_t old_fs;
	static char buf[128];
	loff_t offset = 0;

	/* change to KERNEL_DS address limit */
	old_fs = get_fs();
	set_fs(KERNEL_DS);

	sprintf(buf, "%s%s", param_path, name);
	gov_param = filp_open(buf, O_RDWR, 0);
	if (gov_param != NULL) {
		if (gov_param->f_op != NULL &&
			gov_param->f_op->write != NULL)
			gov_param->f_op->write(gov_param,
					      value,
					      strlen(value),
					      &offset);
		else
			pr_err("f_op might be null\n");

		filp_close(gov_param, NULL);
	}
	set_fs(old_fs);
}

void manage_auto_hotplug(bool active)
{
/*	if (active)
		set_sysfs_param(T3PARMS, "auto_hotplug", "1");
	else
		set_sysfs_param(T3PARMS, "auto_hotplug", "0");*/
}

/*
 * Tegra 3 - Force LP Cluster
 * TODO: Make a better implementation of that mess.
 */
void eprj_extreme_powersave(bool active)
{
	/* We can use set_cpu_possible(cpu, possible) for enforcing that, if needed. */
	if (active) {
		disable_nonboot_cpus(); /* Unstable here! */
		do { cpu_relax(); } while (num_online_cpus() > 1);
		set_sysfs_param(CLUSTER, "active", "lp");
		set_sysfs_param(T3PARMS, "auto_hotplug", "0");
	} else {
		enable_nonboot_cpus();
		msleep(100);
		set_sysfs_param(T3PARMS, "auto_hotplug", "1");
		set_sysfs_param(CLUSTER, "active", "g");
	};
};

/*
 * Actual implementation. Start the sysfs entry.
 */
struct kobject *eprj_sysfs_tools;
static int __init eprj_sysfs_tools_init(void)
{
	int ret = -1;
	printk("EternityProject sysfs Tools: Initialization...\n");
	eprj_sysfs_tools = kzalloc(sizeof(*eprj_sysfs_tools), GFP_KERNEL);
	if (eprj_sysfs_tools) {
		kobject_init(eprj_sysfs_tools, &eprjsysfs_type);
		if (kobject_add(eprj_sysfs_tools, NULL, "%s", "eprjmanager")) {
			printk("EternityProject sysfs Tools: creation failed.\n");
			kobject_put(eprj_sysfs_tools);
			eprj_sysfs_tools = NULL;
		}
	ret = 0;
	}
	if (!ret)
		printk("EternityProject sysfs Tools: Initialized!\n");
	return ret;
}

static void __exit eprj_sysfs_tools_exit(void)
{
	if (eprj_sysfs_tools) {
		kobject_put(eprj_sysfs_tools);
		kfree(eprj_sysfs_tools);
	}
}

late_initcall(eprj_sysfs_tools_init);
module_exit(eprj_sysfs_tools_exit);

MODULE_DESCRIPTION("EternityProject sysfs Tools for HTC One X");
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Angelo G. Del Regno - kholk - <kholk11@gmail.com>");
