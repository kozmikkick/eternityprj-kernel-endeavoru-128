/*
 *  EternityProject Development, 02/08/2012
 *
 *  NULL-PokeCPU: Creates the sysfs attributes
 *  for endeavoru to not throw random errors
 *  due to those damn bad hTC libraries.
 *
 *  Example of solved errors:
 *  E/NvOmxCamera(  172): error in read /sys/htc/cpu_temp
 */

#include <linux/cpufreq.h>
#include <linux/kobject.h>
#include <linux/nct1008.h> /* for thermal temperature */

#define poke_attr(attrbute) 				\
static struct kobj_attribute attrbute##_attr = {	\
	.attr	= {					\
		.name = __stringify(attrbute),		\
		.mode = 0644,				\
	},						\
	.show	= attrbute##_show,			\
	.store	= attrbute##_store,			\
}

static char media_boost = 'N';

static int is_in_power_save = 0;

struct kobject *poke_kobj;

static ssize_t cpu_temp_show(struct kobject *kobj,
		struct kobj_attribute *attr, char *buf)
{
	long temperature;
	struct nct1008_data *thermal_data = get_pwr_data();
	nct1008_thermal_get_temp(thermal_data, &temperature);
	temperature /= 10;
	return sprintf(buf, "%ld.%ld\n", temperature/100, temperature%100);
}

static ssize_t cpu_temp_store(struct kobject *kobj,
		struct kobj_attribute *attr, const char *buf, size_t count)
{
	return 0;
}
poke_attr(cpu_temp);

static ssize_t media_boost_freq_show(struct kobject *kobj,
		struct kobj_attribute *attr, char *buf)
{
	return sprintf(buf, "%c\n", media_boost);
}

static ssize_t media_boost_freq_store(struct kobject *kobj,
		struct kobj_attribute *attr, const char *buf, size_t count)
{
	return 0;
}
poke_attr(media_boost_freq);


static ssize_t power_save_show(struct kobject *kobj,
		struct kobj_attribute *attr, char *buf)
{
	char value = 'N';

	if(is_in_power_save)
		value = 'Y';

	return sprintf(buf, "%c\n", value);
}

static ssize_t power_save_store(struct kobject *kobj,
		struct kobj_attribute *attr, const char *buf, size_t count)
{
	return count;
}
poke_attr(power_save);

static struct attribute * g[] = {
	&media_boost_freq_attr.attr,
	&cpu_temp_attr.attr,
	&power_save_attr.attr,
	NULL,
};

static struct attribute_group attr_group = {
	.attrs = g,
};

static int __init poke_init(void)
{
	pr_info("EternityProject: NULL-PokeCPU Compatibility init.\n");
        poke_kobj = kobject_create_and_add("htc", NULL);

        if (!poke_kobj)
		return -ENOMEM;

	return sysfs_create_group(poke_kobj, &attr_group);
}
late_initcall(poke_init);
