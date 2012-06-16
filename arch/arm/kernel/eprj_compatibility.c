#include <linux/kallsyms.h>
#include <linux/module.h>

void __attribute__((noreturn)) __bug(const char *file, int line)
{
	printk(KERN_CRIT"EternityProject Compatibility: kernel BUG at %s:%d!\n", file, line);
	*(int *)0 = 0;

	/* Avoid "noreturn function does return" */
	for (;;);
}
EXPORT_SYMBOL(__bug);
