/*
 * arch/arm/mach-tegra/include/mach/eternityproject.h
 *
 * Copyright (C) 2012, EternityProject Development
 *
 * Author:
 * 	Angelo G. Del Regno <kholk11@gmail.com>
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

#ifndef __MACH_TEGRA_ETERNITYPROJECT_H
#define __MACH_TEGRA_ETERNITYPROJECT_H
#endif

#include "../../board.h"	/* Thanks for the bad hacks, nVidia! */

#define EPRJ_DEBUGGING	1

/*
 * CPU Clocks Management
 *
 * TODO: Make everything better and use the code in there
 *	 to simplify Tegra clocks management.
 */
#define BASEFREQ	51
#define eprjc(c)	(BASEFREQ * c)		/* For cpu-tegra's CPU_DVFS */
#define eprjf(c)	(BASEFREQ * c * 1000)	/* For cpufreq tables */

/*
 * EDP Management
 */
#define FAC		100000
#define EEDP_MAX 	1700000			/* Normal EDP Clocks */
#define EPRJEDP1	(EEDP_MAX - FAC)
#define EPRJEDP2	(EPRJEDP1 - FAC)
#define EPRJEDP3	(EPRJEDP2 - FAC)

#define COOLDWN0	1000000			/* CPU is HOT! */
#define COOLDWN1	(COOLDWN0 + FAC)
#define COOLDWN2	(COOLDWN1 + FAC)
#define COOLDWN3	(COOLDWN2 + FAC)

#define CRITICAL	204000			/* CPU is MELTING! */

/*
 * TODO: GPU Clocks Management
 */


/*
 * TODO: EternityProject CPUFREQ Governor (eprjdemand)
 *	 -> Tegra highly specific things has to be there
 *	    to mantain the cpufreq governor code clean
 *	    and GENERIC!
 */
#if 0 /* Silence warnings until the code will be used */
#define EPRJDEMAND_GOVERNOR	"eprjdemand"
static void eternityproject_governor_enable(void)
{
#if EPRJ_DEBUGGING
	printk("EternityProject: Setting eprjdemand governor...\n");
#endif
	cpufreq_set_governor(EPRJDEMAND_GOVERNOR);
}
#endif

/*
 * EternityProject sysfs Tools
 */
/* sysfs names */
#define T3PARMS			"/sys/module/cpu_tegra3/parameters/"
#define CLUSTER			"/sys/kernel/cluster/"

/* Android API Levels */
#define ANDROID_API_ICS		15
#define ANDROID_API_JB		16

/* Debug */
#define EPRJ_CHATWITHME
#if defined(EPRJ_CHATWITHME)
#define EPRJ_PRINT(c) \
		pr_info(c)
#else
#define EPRJ_PRINT(c)
#endif

void eprj_hsmgr_35mm_os(unsigned short int); 	/* Headset Compatibility 	*/
void eprj_extreme_powersave(bool);		/* Powersave - LP Cluster Lock	*/
extern struct wake_lock eprj_wifi_lock;		/* WiFi Wakelock		*/
extern bool wifiwakelock_is_allowed;		/* WiFi Wakelock User Control 	*/

struct eprj_sysfs {
	struct attribute attr;
	int value;
};
