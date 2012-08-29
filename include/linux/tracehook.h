/*
 * Tracing hooks
 *
 * Copyright (C) 2008-2009 Red Hat, Inc.  All rights reserved.
 *
 * This copyrighted material is made available to anyone wishing to use,
 * modify, copy, or redistribute it subject to the terms and conditions
 * of the GNU General Public License v.2.
 *
 * This file defines hook entry points called by core code where
 * user tracing/debugging support might need to do something.  These
 * entry points are called tracehook_*().  Each hook declared below
 * has a detailed kerneldoc comment giving the context (locking et
 * al) from which it is called, and the meaning of its return value.
 *
 * Each function here typically has only one call site, so it is ok
 * to have some nontrivial tracehook_*() inlines.  In all cases, the
 * fast path when no tracing is enabled should be very short.
 *
 * The purpose of this file and the tracehook_* layer is to consolidate
 * the interface that the kernel core and arch code uses to enable any
 * user debugging or tracing facility (such as ptrace).  The interfaces
 * here are carefully documented so that maintainers of core and arch
 * code do not need to think about the implementation details of the
 * tracing facilities.  Likewise, maintainers of the tracing code do not
 * need to understand all the calling core or arch code in detail, just
 * documented circumstances of each call, such as locking conditions.
 *
 * If the calling core code changes so that locking is different, then
 * it is ok to change the interface documented here.  The maintainer of
 * core code changing should notify the maintainers of the tracing code
 * that they need to work out the change.
 *
 * Some tracehook_*() inlines take arguments that the current tracing
 * implementations might not necessarily use.  These function signatures
 * are chosen to pass in all the information that is on hand in the
 * caller and might conceivably be relevant to a tracer, so that the
 * core code won't have to be updated when tracing adds more features.
 * If a call site changes so that some of those parameters are no longer
 * already on hand without extra work, then the tracehook_* interface
 * can change so there is no make-work burden on the core code.  The
 * maintainer of core code changing should notify the maintainers of the
 * tracing code that they need to work out the change.
 */

#ifndef _LINUX_TRACEHOOK_H
#define _LINUX_TRACEHOOK_H	1

#include <linux/sched.h>
#include <linux/ptrace.h>
#include <linux/security.h>
struct linux_binprm;

/*
 * ptrace report for syscall entry and exit looks identical.
 */
static inline void ptrace_report_syscall(struct pt_regs *regs)
{
	int ptrace = current->ptrace;

	if (!(ptrace & PT_PTRACED))
		return;

	ptrace_notify(SIGTRAP | ((ptrace & PT_TRACESYSGOOD) ? 0x80 : 0));

	/*
	 * this isn't the same as continuing with a signal, but it will do
	 * for normal use.  strace only continues with a signal if the
	 * stopping signal is not SIGTRAP.  -brl
	 */
	if (current->exit_code) {
		send_sig(current->exit_code, current, 1);
		current->exit_code = 0;
	}
}

/**
 * tracehook_report_syscall_entry - task is about to attempt a system call
 * @regs:		user register state of current task
 *
 * This will be called if %TIF_SYSCALL_TRACE has been set, when the
 * current task has just entered the kernel for a system call.
 * Full user register state is available here.  Changing the values
 * in @regs can affect the system call number and arguments to be tried.
 * It is safe to block here, preventing the system call from beginning.
 *
 * Returns zero normally, or nonzero if the calling arch code should abort
 * the system call.  That must prevent normal entry so no system call is
 * made.  If @task ever returns to user mode after this, its register state
 * is unspecified, but should be something harmless like an %ENOSYS error
 * return.  It should preserve enough information so that syscall_rollback()
 * can work (see asm-generic/syscall.h).
 *
 * Called without locks, just after entering kernel mode.
 */
static inline __must_check int tracehook_report_syscall_entry(
	struct pt_regs *regs)
{
	ptrace_report_syscall(regs);
	return 0;
}

/**
 * tracehook_report_syscall_exit - task has just finished a system call
 * @regs:		user register state of current task
 * @step:		nonzero if simulating single-step or block-step
 *
 * This will be called if %TIF_SYSCALL_TRACE has been set, when the
 * current task has just finished an attempted system call.  Full
 * user register state is available here.  It is safe to block here,
 * preventing signals from being processed.
 *
 * If @step is nonzero, this report is also in lieu of the normal
 * trap that would follow the system call instruction because
 * user_enable_block_step() or user_enable_single_step() was used.
 * In this case, %TIF_SYSCALL_TRACE might not be set.
 *
 * Called without locks, just before checking for pending signals.
 */
static inline void tracehook_report_syscall_exit(struct pt_regs *regs, int step)
{
	if (step) {
		siginfo_t info;
		user_single_step_siginfo(current, regs, &info);
		force_sig_info(SIGTRAP, &info, current);
		return;
	}

	ptrace_report_syscall(regs);
}

/**
 * tracehook_tracer_task - return the task that is tracing the given task
 * @tsk:		task to consider
 *
 * Returns NULL if no one is tracing @task, or the &struct task_struct
 * pointer to its tracer.
 *
 * Must called under rcu_read_lock().  The pointer returned might be kept
 * live only by RCU.  During exec, this may be called with task_lock()
 * held on @task, still held from when tracehook_unsafe_exec() was called.
 */
static inline struct task_struct *tracehook_tracer_task(struct task_struct *tsk)
{
	if (tsk->ptrace & PT_PTRACED)
		return rcu_dereference(tsk->parent);
	return NULL;
}

/**
 * tracehook_report_vfork_done - vfork parent's child has exited or exec'd
 * @child:		child task, already running
 * @pid:		new child's PID in the parent's namespace
 *
 * Called after a %CLONE_VFORK parent has waited for the child to complete.
 * The clone/vfork system call will return immediately after this.
 * The @child pointer may be invalid if a self-reaping child died and
 * tracehook_report_clone() took no action to prevent it from self-reaping.
 *
 * Called with no locks held.
 */
static inline void tracehook_report_vfork_done(struct task_struct *child,
					       pid_t pid)
{
	ptrace_event(PTRACE_EVENT_VFORK_DONE, pid);
}

/**
 * tracehook_prepare_release_task - task is being reaped, clean up tracing
 * @task:		task in %EXIT_DEAD state
 *
 * This is called in release_task() just before @task gets finally reaped
 * and freed.  This would be the ideal place to remove and clean up any
 * tracing-related state for @task.
 *
 * Called with no locks held.
 */
static inline void tracehook_prepare_release_task(struct task_struct *task)
{
}

/**
 * tracehook_finish_release_task - final tracing clean-up
 * @task:		task in %EXIT_DEAD state
 *
 * This is called in release_task() when @task is being in the middle of
 * being reaped.  After this, there must be no tracing entanglements.
 *
 * Called with write_lock_irq(&tasklist_lock) held.
 */
static inline void tracehook_finish_release_task(struct task_struct *task)
{
	ptrace_release_task(task);
}

/**
 * tracehook_signal_handler - signal handler setup is complete
 * @sig:		number of signal being delivered
 * @info:		siginfo_t of signal being delivered
 * @ka:			sigaction setting that chose the handler
 * @regs:		user register state
 * @stepping:		nonzero if debugger single-step or block-step in use
 *
 * Called by the arch code after a signal handler has been set up.
 * Register and stack state reflects the user handler about to run.
 * Signal mask changes have already been made.
 *
 * Called without locks, shortly before returning to user mode
 * (or handling more signals).
 */
static inline void tracehook_signal_handler(int sig, siginfo_t *info,
					    const struct k_sigaction *ka,
					    struct pt_regs *regs, int stepping)
{
	if (stepping)
		ptrace_notify(SIGTRAP);
}

/**
 * tracehook_consider_ignored_signal - suppress short-circuit of ignored signal
 * @task:		task receiving the signal
 * @sig:		signal number being sent
 *
 * Return zero iff tracing doesn't care to examine this ignored signal,
 * so it can short-circuit normal delivery and never even get queued.
 *
 * Called with @task->sighand->siglock held.
 */
static inline int tracehook_consider_ignored_signal(struct task_struct *task,
						    int sig)
{
	return (task->ptrace & PT_PTRACED) != 0;
}

/**
 * tracehook_consider_fatal_signal - suppress special handling of fatal signal
 * @task:		task receiving the signal
 * @sig:		signal number being sent
 *
 * Return nonzero to prevent special handling of this termination signal.
 * Normally handler for signal is %SIG_DFL.  It can be %SIG_IGN if @sig is
 * ignored, in which case force_sig() is about to reset it to %SIG_DFL.
 * When this returns zero, this signal might cause a quick termination
 * that does not give the debugger a chance to intercept the signal.
 *
 * Called with or without @task->sighand->siglock held.
 */
static inline int tracehook_consider_fatal_signal(struct task_struct *task,
						  int sig)
{
	return (task->ptrace & PT_PTRACED) != 0;
}

/**
 * tracehook_force_sigpending - let tracing force signal_pending(current) on
 *
 * Called when recomputing our signal_pending() flag.  Return nonzero
 * to force the signal_pending() flag on, so that tracehook_get_signal()
 * will be called before the next return to user mode.
 *
 * Called with @current->sighand->siglock held.
 */
static inline int tracehook_force_sigpending(void)
{
	return 0;
}

/**
 * tracehook_get_signal - deliver synthetic signal to traced task
 * @task:		@current
 * @regs:		task_pt_regs(@current)
 * @info:		details of synthetic signal
 * @return_ka:		sigaction for synthetic signal
 *
 * Return zero to check for a real pending signal normally.
 * Return -1 after releasing the siglock to repeat the check.
 * Return a signal number to induce an artificial signal delivery,
 * setting *@info and *@return_ka to specify its details and behavior.
 *
 * The @return_ka->sa_handler value controls the disposition of the
 * signal, no matter the signal number.  For %SIG_DFL, the return value
 * is a representative signal to indicate the behavior (e.g. %SIGTERM
 * for death, %SIGQUIT for core dump, %SIGSTOP for job control stop,
 * %SIGTSTP for stop unless in an orphaned pgrp), but the signal number
 * reported will be @info->si_signo instead.
 *
 * Called with @task->sighand->siglock held, before dequeuing pending signals.
 */
static inline int tracehook_get_signal(struct task_struct *task,
				       struct pt_regs *regs,
				       siginfo_t *info,
				       struct k_sigaction *return_ka)
{
	return 0;
}

/**
 * tracehook_notify_jctl - report about job control stop/continue
 * @notify:		zero, %CLD_STOPPED or %CLD_CONTINUED
 * @why:		%CLD_STOPPED or %CLD_CONTINUED
 *
 * This is called when we might call do_notify_parent_cldstop().
 *
 * @notify is zero if we would not ordinarily send a %SIGCHLD,
 * or is the %CLD_STOPPED or %CLD_CONTINUED .si_code for %SIGCHLD.
 *
 * @why is %CLD_STOPPED when about to stop for job control;
 * we are already in %TASK_STOPPED state, about to call schedule().
 * It might also be that we have just exited (check %PF_EXITING),
 * but need to report that a group-wide stop is complete.
 *
 * @why is %CLD_CONTINUED when waking up after job control stop and
 * ready to make a delayed @notify report.
 *
 * Return the %CLD_* value for %SIGCHLD, or zero to generate no signal.
 *
 * Called with the siglock held.
 */
static inline int tracehook_notify_jctl(int notify, int why)
{
	return notify ?: (current->ptrace & PT_PTRACED) ? why : 0;
}

/**
 * tracehook_finish_jctl - report about return from job control stop
 *
 * This is called by do_signal_stop() after wakeup.
 */
static inline void tracehook_finish_jctl(void)
{
}

#ifdef TIF_NOTIFY_RESUME
/**
 * set_notify_resume - cause tracehook_notify_resume() to be called
 * @task:		task that will call tracehook_notify_resume()
 *
 * Calling this arranges that @task will call tracehook_notify_resume()
 * before returning to user mode.  If it's already running in user mode,
 * it will enter the kernel and call tracehook_notify_resume() soon.
 * If it's blocked, it will not be woken.
 */
static inline void set_notify_resume(struct task_struct *task)
{
	if (!test_and_set_tsk_thread_flag(task, TIF_NOTIFY_RESUME))
		kick_process(task);
}

/**
 * tracehook_notify_resume - report when about to return to user mode
 * @regs:		user-mode registers of @current task
 *
 * This is called when %TIF_NOTIFY_RESUME has been set.  Now we are
 * about to return to user mode, and the user state in @regs can be
 * inspected or adjusted.  The caller in arch code has cleared
 * %TIF_NOTIFY_RESUME before the call.  If the flag gets set again
 * asynchronously, this will be called again before we return to
 * user mode.
 *
 * Called without locks.
 */
static inline void tracehook_notify_resume(struct pt_regs *regs)
{
}
#endif	/* TIF_NOTIFY_RESUME */

#endif	/* <linux/tracehook.h> */
