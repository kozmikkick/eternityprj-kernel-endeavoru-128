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

/*
 * CPU Clocks Management
 */
#define BASEFREQ 	51
#define FIXED1		(BASEFREQ * 2)
#define FIXED2		(BASEFREQ * 4)
#define eprj_ctegra(c)	(BASEFREQ * c)		/* For cpu-tegra */
#define eprj_tables(c)	(BASEFREQ * c * 1000);	/* For cpufreq tables */

#define eprjf(is_cpufreq, mult)				\
(							\
	{						\
		if (is_cpufreq == 1)			\
			eprj_tables(mult);		\
		else					\
			eprj_ctegra(mult);		\
	}						\
)
