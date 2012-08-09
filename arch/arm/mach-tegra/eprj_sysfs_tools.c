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
 */

#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/wakelock.h>
#include <mach/eternityproject.h>

bool wifiwakelock_is_allowed = 0;
struct wake_lock eprj_wifi_lock;

struct eprj_sysfs android_release = {
	.attr.name = "android_apirev",
	.attr.mode = 0644,
	.value = ANDROID_API_ICS, /* By default, we assume we are working with ICS */
};

struct eprj_sysfs wifi_hwbug = {
	.attr.name = "wifi_wakelock",
	.attr.mode = 0666,
	.value = 0, /* 0 = Disallow wakelock -- 1 = Allow wakelock */
};

struct attribute * eprjmanager[] = {
	&android_release.attr,
	&wifi_hwbug.attr,
	NULL
};

static ssize_t eprjsysfs_show(struct kobject *kobj, struct attribute *attr,
				char *buf)
{
	struct eprj_sysfs *entry = container_of(attr, struct eprj_sysfs, attr);
	return scnprintf(buf, PAGE_SIZE, "%d", entry->value);
}

static ssize_t eprjsysfs_store(struct kobject *kobj, struct attribute *attr,
				 const char *buf, size_t len)
{
	struct eprj_sysfs *entry = container_of(attr, struct eprj_sysfs, attr);
	sscanf(buf, "%d", &entry->value);
	/* Is there any better way? */
	if ( (entry->value == ANDROID_API_JB ) && (entry->attr.name == "android_apirev") ) {
		printk("EternityProject HSMGR: Android Jellybean detected.\n");
		eprj_hsmgr_35mm_os(ANDROID_API_JB);
	};
	/* Okay, now that's getting really tedious .. and ridiculous. */
	if (entry->attr.name == "wifi_wakelock")
		wifiwakelock_is_allowed = entry->value;

	/*
	 * ToDo: Check if the wifi wakelock is being held, only if we're writing
	 *       "0" to wifi_wakelock. In case the wakelock is being held and we
	 *       write "0" here, we'll hold that for the entire "kernel-life".
	 *       If after that we write "1" here, we'll re-take the same wakelock
	 *       and we'll most probably panic the kernel.
	 *
	 *       This ToDo is URGENT.
	 */

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
	wake_lock_init(&eprj_wifi_lock, WAKE_LOCK_SUSPEND, "wifi_wakelock");
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
