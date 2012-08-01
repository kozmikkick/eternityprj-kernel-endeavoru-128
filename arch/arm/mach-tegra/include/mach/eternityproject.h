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
 * TODO: GPU Clocks Management
 */


/*
 * TODO: EternityProject CPUFREQ Governor (eprjdemand)
 *	 -> Tegra highly specific things has to be there
 *	    to mantain the cpufreq governor code clean
 *	    and GENERIC!
 */
