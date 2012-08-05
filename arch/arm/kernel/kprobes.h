/*
 * arch/arm/kernel/kprobes.h
 *
 * Contents moved from arch/arm/include/asm/kprobes.h which is
 * Copyright (C) 2006, 2007 Motorola Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 */

#ifndef _ARM_KERNEL_KPROBES_H
#define _ARM_KERNEL_KPROBES_H

/*
 * This undefined instruction must be unique and
 * reserved solely for kprobes' use.
 */
#define KPROBE_BREAKPOINT_INSTRUCTION	0xe7f001f8

enum kprobe_insn {
	INSN_REJECTED,
	INSN_GOOD,
	INSN_GOOD_NO_SLOT
};

typedef enum kprobe_insn (kprobe_decode_insn_t)(kprobe_opcode_t,
						struct arch_specific_insn *);

#ifdef CONFIG_THUMB2_KERNEL

enum kprobe_insn thumb16_kprobe_decode_insn(kprobe_opcode_t,
						struct arch_specific_insn *);
enum kprobe_insn thumb32_kprobe_decode_insn(kprobe_opcode_t,
						struct arch_specific_insn *);

#else /* !CONFIG_THUMB2_KERNEL */

enum kprobe_insn arm_kprobe_decode_insn(kprobe_opcode_t,
					struct arch_specific_insn *);
#endif

void __init arm_kprobe_decode_init(void);

extern kprobe_check_cc * const kprobe_condition_checks[16];

#endif /* _ARM_KERNEL_KPROBES_H */
