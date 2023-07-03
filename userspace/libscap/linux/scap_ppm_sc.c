/*
Copyright (C) 2023 The Falco Authors.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

*/

#include "scap.h"
#include "scap-int.h"
#include <string.h>

/*
 * When adding a new event, a new line should be added with the list of ppm_sc codes mapping that event.
 * Events that are not mapped to any ppm_sc (ie: "container", "useradded"..., have NULL entries.
 *
 * If adding a specific event mapping an existing generic event, remember to
 * remove the generic events from the first 2 lines.
 *
 * NOTE: first 2 lines are automatically bumped by syscalls-bumper.
 */
static const ppm_sc_code *g_events_to_sc_map[] = {
	[PPME_GENERIC_E] = (ppm_sc_code[]){ PPM_SC_RESTART_SYSCALL, PPM_SC_EXIT, PPM_SC_TIME,  PPM_SC_MKNOD, PPM_SC_GETPID, PPM_SC_SYNC, PPM_SC_TIMES, PPM_SC_ACCT, PPM_SC_UMASK, PPM_SC_USTAT, PPM_SC_GETPPID, PPM_SC_GETPGRP, PPM_SC_SETHOSTNAME, PPM_SC_GETRUSAGE, PPM_SC_GETTIMEOFDAY, PPM_SC_SETTIMEOFDAY, PPM_SC_READLINK, PPM_SC_SWAPON, PPM_SC_REBOOT, PPM_SC_TRUNCATE, PPM_SC_FTRUNCATE, PPM_SC_GETPRIORITY, PPM_SC_SETPRIORITY, PPM_SC_STATFS, PPM_SC_FSTATFS, PPM_SC_SETITIMER, PPM_SC_GETITIMER, PPM_SC_UNAME, PPM_SC_VHANGUP, PPM_SC_WAIT4, PPM_SC_SWAPOFF, PPM_SC_SYSINFO, PPM_SC_FSYNC, PPM_SC_SETDOMAINNAME, PPM_SC_ADJTIMEX, PPM_SC_INIT_MODULE, PPM_SC_DELETE_MODULE, PPM_SC_GETPGID, PPM_SC_SYSFS, PPM_SC_PERSONALITY, PPM_SC_MSYNC, PPM_SC_GETSID, PPM_SC_FDATASYNC, PPM_SC_SCHED_SETSCHEDULER, PPM_SC_SCHED_GETSCHEDULER, PPM_SC_SCHED_YIELD, PPM_SC_SCHED_GET_PRIORITY_MAX, PPM_SC_SCHED_GET_PRIORITY_MIN, PPM_SC_SCHED_RR_GET_INTERVAL, PPM_SC_MREMAP, PPM_SC_ARCH_PRCTL, PPM_SC_RT_SIGACTION, PPM_SC_RT_SIGPROCMASK, PPM_SC_RT_SIGPENDING, PPM_SC_RT_SIGTIMEDWAIT, PPM_SC_RT_SIGQUEUEINFO, PPM_SC_RT_SIGSUSPEND, PPM_SC_CAPGET, PPM_SC_SETREUID, PPM_SC_SETREGID, PPM_SC_GETGROUPS, PPM_SC_SETGROUPS, PPM_SC_SETFSUID, PPM_SC_SETFSGID, PPM_SC_PIVOT_ROOT, PPM_SC_MINCORE, PPM_SC_MADVISE, PPM_SC_GETTID, PPM_SC_SETXATTR, PPM_SC_LSETXATTR, PPM_SC_FSETXATTR, PPM_SC_GETXATTR, PPM_SC_LGETXATTR, PPM_SC_FGETXATTR, PPM_SC_LISTXATTR, PPM_SC_LLISTXATTR, PPM_SC_FLISTXATTR, PPM_SC_REMOVEXATTR, PPM_SC_LREMOVEXATTR, PPM_SC_FREMOVEXATTR,PPM_SC_SCHED_SETAFFINITY, PPM_SC_SCHED_GETAFFINITY, PPM_SC_SET_THREAD_AREA, PPM_SC_GET_THREAD_AREA, PPM_SC_IO_SETUP, PPM_SC_IO_DESTROY, PPM_SC_IO_GETEVENTS, PPM_SC_IO_SUBMIT, PPM_SC_IO_CANCEL, PPM_SC_EXIT_GROUP, PPM_SC_REMAP_FILE_PAGES, PPM_SC_SET_TID_ADDRESS, PPM_SC_TIMER_CREATE, PPM_SC_TIMER_SETTIME, PPM_SC_TIMER_GETTIME, PPM_SC_TIMER_GETOVERRUN, PPM_SC_TIMER_DELETE, PPM_SC_CLOCK_SETTIME, PPM_SC_CLOCK_GETTIME, PPM_SC_CLOCK_GETRES, PPM_SC_CLOCK_NANOSLEEP,PPM_SC_UTIMES, PPM_SC_MQ_OPEN, PPM_SC_MQ_UNLINK, PPM_SC_MQ_TIMEDSEND, PPM_SC_MQ_TIMEDRECEIVE, PPM_SC_MQ_NOTIFY, PPM_SC_MQ_GETSETATTR, PPM_SC_KEXEC_LOAD, PPM_SC_WAITID, PPM_SC_ADD_KEY, PPM_SC_REQUEST_KEY, PPM_SC_KEYCTL, PPM_SC_IOPRIO_SET, PPM_SC_IOPRIO_GET, PPM_SC_INOTIFY_ADD_WATCH, PPM_SC_INOTIFY_RM_WATCH, PPM_SC_MKNODAT, PPM_SC_FUTIMESAT, PPM_SC_READLINKAT, PPM_SC_FACCESSAT, PPM_SC_SET_ROBUST_LIST, PPM_SC_GET_ROBUST_LIST, PPM_SC_TEE, PPM_SC_VMSPLICE, PPM_SC_GETCPU, PPM_SC_EPOLL_PWAIT, PPM_SC_UTIMENSAT, PPM_SC_TIMERFD_SETTIME, PPM_SC_TIMERFD_GETTIME, PPM_SC_RT_TGSIGQUEUEINFO, PPM_SC_PERF_EVENT_OPEN, PPM_SC_FANOTIFY_INIT, PPM_SC_CLOCK_ADJTIME, PPM_SC_SYNCFS, PPM_SC_MSGSND, PPM_SC_MSGRCV, PPM_SC_MSGGET, PPM_SC_MSGCTL, PPM_SC_SHMDT, PPM_SC_SHMGET, PPM_SC_SHMCTL, PPM_SC_STATFS64, PPM_SC_FSTATFS64, PPM_SC_FSTATAT64, PPM_SC_BDFLUSH, PPM_SC_SIGPROCMASK, PPM_SC_IPC, PPM_SC__NEWSELECT, PPM_SC_SGETMASK, PPM_SC_SSETMASK, PPM_SC_SIGPENDING, PPM_SC_OLDUNAME, PPM_SC_SIGNAL, PPM_SC_NICE, PPM_SC_STIME, PPM_SC_WAITPID, PPM_SC_SHMAT, PPM_SC_RT_SIGRETURN, PPM_SC_FALLOCATE, PPM_SC_NEWFSTATAT, PPM_SC_FINIT_MODULE, PPM_SC_SIGALTSTACK, PPM_SC_GETRANDOM, PPM_SC_FADVISE64, PPM_SC_SOCKETCALL, PPM_SC_FSPICK, PPM_SC_FSMOUNT, PPM_SC_FSOPEN, PPM_SC_OPEN_TREE, PPM_SC_MOVE_MOUNT, PPM_SC_MOUNT_SETATTR, PPM_SC_MEMFD_SECRET, PPM_SC_IOPERM, PPM_SC_KEXEC_FILE_LOAD, PPM_SC_PIDFD_OPEN, PPM_SC_PIDFD_SEND_SIGNAL, PPM_SC_PKEY_ALLOC, PPM_SC_PKEY_MPROTECT, PPM_SC_PKEY_FREE, PPM_SC_LANDLOCK_CREATE_RULESET, PPM_SC_QUOTACTL_FD, PPM_SC_LANDLOCK_RESTRICT_SELF, PPM_SC_LANDLOCK_ADD_RULE, PPM_SC_EPOLL_PWAIT2, PPM_SC_MIGRATE_PAGES, PPM_SC_MOVE_PAGES, PPM_SC_PREADV2, PPM_SC_PWRITEV2, PPM_SC_QUERY_MODULE, PPM_SC_STATX, PPM_SC_SET_MEMPOLICY, PPM_SC_FANOTIFY_MARK, PPM_SC_SYNC_FILE_RANGE, PPM_SC_READAHEAD, PPM_SC_PROCESS_MRELEASE, PPM_SC_MBIND, PPM_SC_PROCESS_MADVISE, PPM_SC_MEMBARRIER, PPM_SC_MODIFY_LDT, PPM_SC_SEMTIMEDOP, PPM_SC_NAME_TO_HANDLE_AT, PPM_SC_KCMP, PPM_SC_EPOLL_CTL_OLD, PPM_SC_EPOLL_WAIT_OLD, PPM_SC_FUTEX_WAITV, PPM_SC_CREATE_MODULE, PPM_SC__SYSCTL, PPM_SC_LOOKUP_DCOOKIE, PPM_SC_IOPL, PPM_SC_IO_PGETEVENTS, PPM_SC_GETPMSG, PPM_SC_SCHED_SETATTR, PPM_SC_GET_KERNEL_SYMS, PPM_SC_RSEQ, PPM_SC_CLOSE_RANGE, PPM_SC_GET_MEMPOLICY, PPM_SC_SCHED_GETATTR, PPM_SC_NFSSERVCTL, PPM_SC_SET_MEMPOLICY_HOME_NODE, PPM_SC_FACCESSAT2, PPM_SC_EPOLL_CTL, PPM_SC_PROCESS_VM_WRITEV, PPM_SC_SCHED_GETPARAM, PPM_SC_PSELECT6, PPM_SC_SCHED_SETPARAM, PPM_SC_PROCESS_VM_READV, PPM_SC_PAUSE, PPM_SC_UTIME, PPM_SC_SYSLOG, PPM_SC_USELIB, PPM_SC_ALARM, PPM_SC_SIGSUSPEND, PPM_SC_IDLE, PPM_SC_S390_RUNTIME_INSTR, PPM_SC_SIGRETURN, PPM_SC_S390_GUARDED_STORAGE, PPM_SC_TIMERFD, PPM_SC_S390_PCI_MMIO_READ, PPM_SC_SIGACTION, PPM_SC_S390_PCI_MMIO_WRITE, PPM_SC_READDIR, PPM_SC_S390_STHYI,  -1},
	[PPME_GENERIC_X] = (ppm_sc_code[]){ PPM_SC_RESTART_SYSCALL, PPM_SC_EXIT, PPM_SC_TIME,  PPM_SC_MKNOD, PPM_SC_GETPID, PPM_SC_SYNC, PPM_SC_TIMES, PPM_SC_ACCT, PPM_SC_UMASK, PPM_SC_USTAT, PPM_SC_GETPPID, PPM_SC_GETPGRP, PPM_SC_SETHOSTNAME, PPM_SC_GETRUSAGE, PPM_SC_GETTIMEOFDAY, PPM_SC_SETTIMEOFDAY, PPM_SC_READLINK, PPM_SC_SWAPON, PPM_SC_REBOOT, PPM_SC_TRUNCATE, PPM_SC_FTRUNCATE, PPM_SC_GETPRIORITY, PPM_SC_SETPRIORITY, PPM_SC_STATFS, PPM_SC_FSTATFS, PPM_SC_SETITIMER, PPM_SC_GETITIMER, PPM_SC_UNAME, PPM_SC_VHANGUP, PPM_SC_WAIT4, PPM_SC_SWAPOFF, PPM_SC_SYSINFO, PPM_SC_FSYNC, PPM_SC_SETDOMAINNAME, PPM_SC_ADJTIMEX, PPM_SC_INIT_MODULE, PPM_SC_DELETE_MODULE, PPM_SC_GETPGID, PPM_SC_SYSFS, PPM_SC_PERSONALITY, PPM_SC_MSYNC, PPM_SC_GETSID, PPM_SC_FDATASYNC, PPM_SC_SCHED_SETSCHEDULER, PPM_SC_SCHED_GETSCHEDULER, PPM_SC_SCHED_YIELD, PPM_SC_SCHED_GET_PRIORITY_MAX, PPM_SC_SCHED_GET_PRIORITY_MIN, PPM_SC_SCHED_RR_GET_INTERVAL, PPM_SC_MREMAP, PPM_SC_ARCH_PRCTL, PPM_SC_RT_SIGACTION, PPM_SC_RT_SIGPROCMASK, PPM_SC_RT_SIGPENDING, PPM_SC_RT_SIGTIMEDWAIT, PPM_SC_RT_SIGQUEUEINFO, PPM_SC_RT_SIGSUSPEND, PPM_SC_CAPGET, PPM_SC_SETREUID, PPM_SC_SETREGID, PPM_SC_GETGROUPS, PPM_SC_SETGROUPS, PPM_SC_SETFSUID, PPM_SC_SETFSGID, PPM_SC_PIVOT_ROOT, PPM_SC_MINCORE, PPM_SC_MADVISE, PPM_SC_GETTID, PPM_SC_SETXATTR, PPM_SC_LSETXATTR, PPM_SC_FSETXATTR, PPM_SC_GETXATTR, PPM_SC_LGETXATTR, PPM_SC_FGETXATTR, PPM_SC_LISTXATTR, PPM_SC_LLISTXATTR, PPM_SC_FLISTXATTR, PPM_SC_REMOVEXATTR, PPM_SC_LREMOVEXATTR, PPM_SC_FREMOVEXATTR,PPM_SC_SCHED_SETAFFINITY, PPM_SC_SCHED_GETAFFINITY, PPM_SC_SET_THREAD_AREA, PPM_SC_GET_THREAD_AREA, PPM_SC_IO_SETUP, PPM_SC_IO_DESTROY, PPM_SC_IO_GETEVENTS, PPM_SC_IO_SUBMIT, PPM_SC_IO_CANCEL, PPM_SC_EXIT_GROUP, PPM_SC_REMAP_FILE_PAGES, PPM_SC_SET_TID_ADDRESS, PPM_SC_TIMER_CREATE, PPM_SC_TIMER_SETTIME, PPM_SC_TIMER_GETTIME, PPM_SC_TIMER_GETOVERRUN, PPM_SC_TIMER_DELETE, PPM_SC_CLOCK_SETTIME, PPM_SC_CLOCK_GETTIME, PPM_SC_CLOCK_GETRES, PPM_SC_CLOCK_NANOSLEEP,PPM_SC_UTIMES, PPM_SC_MQ_OPEN, PPM_SC_MQ_UNLINK, PPM_SC_MQ_TIMEDSEND, PPM_SC_MQ_TIMEDRECEIVE, PPM_SC_MQ_NOTIFY, PPM_SC_MQ_GETSETATTR, PPM_SC_KEXEC_LOAD, PPM_SC_WAITID, PPM_SC_ADD_KEY, PPM_SC_REQUEST_KEY, PPM_SC_KEYCTL, PPM_SC_IOPRIO_SET, PPM_SC_IOPRIO_GET, PPM_SC_INOTIFY_ADD_WATCH, PPM_SC_INOTIFY_RM_WATCH, PPM_SC_MKNODAT, PPM_SC_FUTIMESAT, PPM_SC_READLINKAT, PPM_SC_FACCESSAT, PPM_SC_SET_ROBUST_LIST, PPM_SC_GET_ROBUST_LIST, PPM_SC_TEE, PPM_SC_VMSPLICE, PPM_SC_GETCPU, PPM_SC_EPOLL_PWAIT, PPM_SC_UTIMENSAT, PPM_SC_TIMERFD_SETTIME, PPM_SC_TIMERFD_GETTIME, PPM_SC_RT_TGSIGQUEUEINFO, PPM_SC_PERF_EVENT_OPEN, PPM_SC_FANOTIFY_INIT, PPM_SC_CLOCK_ADJTIME, PPM_SC_SYNCFS, PPM_SC_MSGSND, PPM_SC_MSGRCV, PPM_SC_MSGGET, PPM_SC_MSGCTL, PPM_SC_SHMDT, PPM_SC_SHMGET, PPM_SC_SHMCTL, PPM_SC_STATFS64, PPM_SC_FSTATFS64, PPM_SC_FSTATAT64, PPM_SC_BDFLUSH, PPM_SC_SIGPROCMASK, PPM_SC_IPC, PPM_SC__NEWSELECT, PPM_SC_SGETMASK, PPM_SC_SSETMASK, PPM_SC_SIGPENDING, PPM_SC_OLDUNAME, PPM_SC_SIGNAL, PPM_SC_NICE, PPM_SC_STIME, PPM_SC_WAITPID, PPM_SC_SHMAT, PPM_SC_RT_SIGRETURN, PPM_SC_FALLOCATE, PPM_SC_NEWFSTATAT, PPM_SC_FINIT_MODULE, PPM_SC_SIGALTSTACK, PPM_SC_GETRANDOM, PPM_SC_FADVISE64, PPM_SC_SOCKETCALL, PPM_SC_FSPICK, PPM_SC_FSMOUNT, PPM_SC_FSOPEN, PPM_SC_OPEN_TREE, PPM_SC_MOVE_MOUNT, PPM_SC_MOUNT_SETATTR, PPM_SC_MEMFD_SECRET, PPM_SC_IOPERM, PPM_SC_KEXEC_FILE_LOAD, PPM_SC_PIDFD_OPEN, PPM_SC_PIDFD_SEND_SIGNAL, PPM_SC_PKEY_ALLOC, PPM_SC_PKEY_MPROTECT, PPM_SC_PKEY_FREE, PPM_SC_LANDLOCK_CREATE_RULESET, PPM_SC_QUOTACTL_FD, PPM_SC_LANDLOCK_RESTRICT_SELF, PPM_SC_LANDLOCK_ADD_RULE, PPM_SC_EPOLL_PWAIT2, PPM_SC_MIGRATE_PAGES, PPM_SC_MOVE_PAGES, PPM_SC_PREADV2, PPM_SC_PWRITEV2, PPM_SC_QUERY_MODULE, PPM_SC_STATX, PPM_SC_SET_MEMPOLICY, PPM_SC_FANOTIFY_MARK, PPM_SC_SYNC_FILE_RANGE, PPM_SC_READAHEAD, PPM_SC_PROCESS_MRELEASE, PPM_SC_MBIND, PPM_SC_PROCESS_MADVISE, PPM_SC_MEMBARRIER, PPM_SC_MODIFY_LDT, PPM_SC_SEMTIMEDOP, PPM_SC_NAME_TO_HANDLE_AT, PPM_SC_KCMP, PPM_SC_EPOLL_CTL_OLD, PPM_SC_EPOLL_WAIT_OLD, PPM_SC_FUTEX_WAITV, PPM_SC_CREATE_MODULE, PPM_SC__SYSCTL, PPM_SC_LOOKUP_DCOOKIE, PPM_SC_IOPL, PPM_SC_IO_PGETEVENTS, PPM_SC_GETPMSG, PPM_SC_SCHED_SETATTR, PPM_SC_GET_KERNEL_SYMS, PPM_SC_RSEQ, PPM_SC_CLOSE_RANGE, PPM_SC_GET_MEMPOLICY, PPM_SC_SCHED_GETATTR, PPM_SC_NFSSERVCTL, PPM_SC_SET_MEMPOLICY_HOME_NODE, PPM_SC_FACCESSAT2, PPM_SC_EPOLL_CTL, PPM_SC_PROCESS_VM_WRITEV, PPM_SC_SCHED_GETPARAM, PPM_SC_PSELECT6, PPM_SC_SCHED_SETPARAM, PPM_SC_PROCESS_VM_READV, PPM_SC_PAUSE, PPM_SC_UTIME, PPM_SC_SYSLOG, PPM_SC_USELIB, PPM_SC_ALARM, PPM_SC_TIMERFD, PPM_SC_S390_PCI_MMIO_READ, PPM_SC_SIGACTION, PPM_SC_S390_PCI_MMIO_WRITE, PPM_SC_READDIR, PPM_SC_S390_STHYI, PPM_SC_SIGSUSPEND, PPM_SC_IDLE, PPM_SC_S390_RUNTIME_INSTR, PPM_SC_SIGRETURN, PPM_SC_S390_GUARDED_STORAGE,  -1},
	[PPME_SYSCALL_OPEN_E] = (ppm_sc_code[]){PPM_SC_OPEN, -1},
	[PPME_SYSCALL_OPEN_X] = (ppm_sc_code[]){PPM_SC_OPEN, -1},
	[PPME_SYSCALL_CLOSE_E] = (ppm_sc_code[]){PPM_SC_CLOSE, -1},
	[PPME_SYSCALL_CLOSE_X] = (ppm_sc_code[]){PPM_SC_CLOSE, -1},
	[PPME_SYSCALL_READ_E] = (ppm_sc_code[]){PPM_SC_READ, -1},
	[PPME_SYSCALL_READ_X] = (ppm_sc_code[]){PPM_SC_READ, -1},
	[PPME_SYSCALL_WRITE_E] = (ppm_sc_code[]){PPM_SC_WRITE, -1},
	[PPME_SYSCALL_WRITE_X] = (ppm_sc_code[]){PPM_SC_WRITE, -1},
	[PPME_SYSCALL_BRK_1_E] = (ppm_sc_code[]){PPM_SC_BRK, -1},
	[PPME_SYSCALL_BRK_1_X] = (ppm_sc_code[]){PPM_SC_BRK, -1},
	[PPME_SYSCALL_EXECVE_8_E] = (ppm_sc_code[]){PPM_SC_EXECVE, -1},
	[PPME_SYSCALL_EXECVE_8_X] = (ppm_sc_code[]){PPM_SC_EXECVE, -1},
	[PPME_SYSCALL_CLONE_11_E] = (ppm_sc_code[]){PPM_SC_CLONE, -1},
	[PPME_SYSCALL_CLONE_11_X] = (ppm_sc_code[]){PPM_SC_CLONE, -1},
	[PPME_SYSCALL_PRCTL_E] = (ppm_sc_code[]){PPM_SC_PRCTL, -1},
	[PPME_SYSCALL_PRCTL_X] = (ppm_sc_code[]){PPM_SC_PRCTL, -1},
	[PPME_PROCEXIT_E] = (ppm_sc_code[]){PPM_SC_SCHED_PROCESS_EXIT, -1},
	[PPME_PROCEXIT_X] = NULL,
	[PPME_SOCKET_SOCKET_E] = (ppm_sc_code[]){PPM_SC_SOCKET, -1},
	[PPME_SOCKET_SOCKET_X] = (ppm_sc_code[]){PPM_SC_SOCKET, -1},
	[PPME_SOCKET_BIND_E] = (ppm_sc_code[]){PPM_SC_BIND, -1},
	[PPME_SOCKET_BIND_X] = (ppm_sc_code[]){PPM_SC_BIND, -1},
	[PPME_SOCKET_CONNECT_E] = (ppm_sc_code[]){PPM_SC_CONNECT, -1},
	[PPME_SOCKET_CONNECT_X] = (ppm_sc_code[]){PPM_SC_CONNECT, -1},
	[PPME_SOCKET_LISTEN_E] = (ppm_sc_code[]){PPM_SC_LISTEN, -1},
	[PPME_SOCKET_LISTEN_X] = (ppm_sc_code[]){PPM_SC_LISTEN, -1},
	[PPME_SOCKET_ACCEPT_E] = (ppm_sc_code[]){PPM_SC_ACCEPT, -1},
	[PPME_SOCKET_ACCEPT_X] = (ppm_sc_code[]){PPM_SC_ACCEPT, -1},
	[PPME_SOCKET_SEND_E] = (ppm_sc_code[]){PPM_SC_SEND, -1},
	[PPME_SOCKET_SEND_X] = (ppm_sc_code[]){PPM_SC_SEND, -1},
	[PPME_SOCKET_SENDTO_E] = (ppm_sc_code[]){PPM_SC_SENDTO, -1},
	[PPME_SOCKET_SENDTO_X] = (ppm_sc_code[]){PPM_SC_SENDTO, -1},
	[PPME_SOCKET_RECV_E] = (ppm_sc_code[]){PPM_SC_RECV, -1},
	[PPME_SOCKET_RECV_X] = (ppm_sc_code[]){PPM_SC_RECV, -1},
	[PPME_SOCKET_RECVFROM_E] = (ppm_sc_code[]){PPM_SC_RECVFROM, -1},
	[PPME_SOCKET_RECVFROM_X] = (ppm_sc_code[]){PPM_SC_RECVFROM, -1},
	[PPME_SOCKET_SHUTDOWN_E] = (ppm_sc_code[]){PPM_SC_SHUTDOWN, -1},
	[PPME_SOCKET_SHUTDOWN_X] = (ppm_sc_code[]){PPM_SC_SHUTDOWN, -1},
	[PPME_SOCKET_GETSOCKNAME_E] = (ppm_sc_code[]){PPM_SC_GETSOCKNAME, -1},
	[PPME_SOCKET_GETSOCKNAME_X] = (ppm_sc_code[]){PPM_SC_GETSOCKNAME, -1},
	[PPME_SOCKET_GETPEERNAME_E] = (ppm_sc_code[]){PPM_SC_GETPEERNAME, -1},
	[PPME_SOCKET_GETPEERNAME_X] = (ppm_sc_code[]){PPM_SC_GETPEERNAME, -1},
	[PPME_SOCKET_SOCKETPAIR_E] = (ppm_sc_code[]){PPM_SC_SOCKETPAIR, -1},
	[PPME_SOCKET_SOCKETPAIR_X] = (ppm_sc_code[]){PPM_SC_SOCKETPAIR, -1},
	[PPME_SOCKET_SETSOCKOPT_E] = (ppm_sc_code[]){PPM_SC_SETSOCKOPT, -1},
	[PPME_SOCKET_SETSOCKOPT_X] = (ppm_sc_code[]){PPM_SC_SETSOCKOPT, -1},
	[PPME_SOCKET_GETSOCKOPT_E] = (ppm_sc_code[]){PPM_SC_GETSOCKOPT, -1},
	[PPME_SOCKET_GETSOCKOPT_X] = (ppm_sc_code[]){PPM_SC_GETSOCKOPT, -1},
	[PPME_SOCKET_SENDMSG_E] = (ppm_sc_code[]){PPM_SC_SENDMSG, -1},
	[PPME_SOCKET_SENDMSG_X] = (ppm_sc_code[]){PPM_SC_SENDMSG, -1},
	[PPME_SOCKET_SENDMMSG_E] = (ppm_sc_code[]){PPM_SC_SENDMMSG, -1},
	[PPME_SOCKET_SENDMMSG_X] = (ppm_sc_code[]){PPM_SC_SENDMMSG, -1},
	[PPME_SOCKET_RECVMSG_E] = (ppm_sc_code[]){PPM_SC_RECVMSG, -1},
	[PPME_SOCKET_RECVMSG_X] = (ppm_sc_code[]){PPM_SC_RECVMSG, -1},
	[PPME_SOCKET_RECVMMSG_E] = (ppm_sc_code[]){PPM_SC_RECVMMSG, -1},
	[PPME_SOCKET_RECVMMSG_X] = (ppm_sc_code[]){PPM_SC_RECVMMSG, -1},
	[PPME_SOCKET_ACCEPT4_E] = (ppm_sc_code[]){PPM_SC_ACCEPT4, -1},
	[PPME_SOCKET_ACCEPT4_X] = (ppm_sc_code[]){PPM_SC_ACCEPT4, -1},
	[PPME_SYSCALL_CREAT_E] = (ppm_sc_code[]){PPM_SC_CREAT, -1},
	[PPME_SYSCALL_CREAT_X] = (ppm_sc_code[]){PPM_SC_CREAT, -1},
	[PPME_SYSCALL_PIPE_E] = (ppm_sc_code[]){PPM_SC_PIPE, -1},
	[PPME_SYSCALL_PIPE_X] = (ppm_sc_code[]){PPM_SC_PIPE, -1},
	[PPME_SYSCALL_EVENTFD_E] = (ppm_sc_code[]){PPM_SC_EVENTFD, -1},
	[PPME_SYSCALL_EVENTFD_X] = (ppm_sc_code[]){PPM_SC_EVENTFD, -1},
	[PPME_SYSCALL_FUTEX_E] = (ppm_sc_code[]){PPM_SC_FUTEX, -1},
	[PPME_SYSCALL_FUTEX_X] = (ppm_sc_code[]){PPM_SC_FUTEX, -1},
	[PPME_SYSCALL_STAT_E] = (ppm_sc_code[]){PPM_SC_STAT, -1},
	[PPME_SYSCALL_STAT_X] = (ppm_sc_code[]){PPM_SC_STAT, -1},
	[PPME_SYSCALL_LSTAT_E] = (ppm_sc_code[]){PPM_SC_LSTAT, -1},
	[PPME_SYSCALL_LSTAT_X] = (ppm_sc_code[]){PPM_SC_LSTAT, -1},
	[PPME_SYSCALL_FSTAT_E] = (ppm_sc_code[]){PPM_SC_FSTAT, -1},
	[PPME_SYSCALL_FSTAT_X] = (ppm_sc_code[]){PPM_SC_FSTAT, -1},
	[PPME_SYSCALL_STAT64_E] = (ppm_sc_code[]){PPM_SC_STAT64, -1},
	[PPME_SYSCALL_STAT64_X] = (ppm_sc_code[]){PPM_SC_STAT64, -1},
	[PPME_SYSCALL_LSTAT64_E] = (ppm_sc_code[]){PPM_SC_LSTAT64, -1}, // lstat64 -> is not impl by supported archs
	[PPME_SYSCALL_LSTAT64_X] = (ppm_sc_code[]){PPM_SC_LSTAT64, -1}, // lstat64 -> is not impl by supported archs
	[PPME_SYSCALL_FSTAT64_E] = (ppm_sc_code[]){PPM_SC_FSTAT64, -1},
	[PPME_SYSCALL_FSTAT64_X] = (ppm_sc_code[]){PPM_SC_FSTAT64, -1},
	[PPME_SYSCALL_EPOLLWAIT_E] = (ppm_sc_code[]){PPM_SC_EPOLL_WAIT, -1},
	[PPME_SYSCALL_EPOLLWAIT_X] = (ppm_sc_code[]){PPM_SC_EPOLL_WAIT, -1},
	[PPME_SYSCALL_POLL_E] = (ppm_sc_code[]){PPM_SC_POLL, -1},
	[PPME_SYSCALL_POLL_X] = (ppm_sc_code[]){PPM_SC_POLL, -1},
	[PPME_SYSCALL_SELECT_E] = (ppm_sc_code[]){PPM_SC_SELECT, -1},
	[PPME_SYSCALL_SELECT_X] = (ppm_sc_code[]){PPM_SC_SELECT, -1},
	[PPME_SYSCALL_NEWSELECT_E] = (ppm_sc_code[]){PPM_SC_SELECT, -1},
	[PPME_SYSCALL_NEWSELECT_X] = (ppm_sc_code[]){PPM_SC_SELECT, -1},
	[PPME_SYSCALL_LSEEK_E] = (ppm_sc_code[]){PPM_SC_LSEEK, -1},
	[PPME_SYSCALL_LSEEK_X] = (ppm_sc_code[]){PPM_SC_LSEEK, -1},
	[PPME_SYSCALL_LLSEEK_E] = (ppm_sc_code[]){PPM_SC__LLSEEK, -1},
	[PPME_SYSCALL_LLSEEK_X] = (ppm_sc_code[]){PPM_SC__LLSEEK, -1},
	[PPME_SYSCALL_IOCTL_2_E] = (ppm_sc_code[]){PPM_SC_IOCTL, -1},
	[PPME_SYSCALL_IOCTL_2_X] = (ppm_sc_code[]){PPM_SC_IOCTL, -1},
	[PPME_SYSCALL_GETCWD_E] = (ppm_sc_code[]){PPM_SC_GETCWD, -1},
	[PPME_SYSCALL_GETCWD_X] = (ppm_sc_code[]){PPM_SC_GETCWD, -1},
	[PPME_SYSCALL_CHDIR_E] = (ppm_sc_code[]){PPM_SC_CHDIR, -1},
	[PPME_SYSCALL_CHDIR_X] = (ppm_sc_code[]){PPM_SC_CHDIR, -1},
	[PPME_SYSCALL_FCHDIR_E] = (ppm_sc_code[]){PPM_SC_FCHDIR, -1},
	[PPME_SYSCALL_FCHDIR_X] = (ppm_sc_code[]){PPM_SC_FCHDIR, -1},
	[PPME_SYSCALL_MKDIR_E] = (ppm_sc_code[]){PPM_SC_MKDIR, -1},
	[PPME_SYSCALL_MKDIR_X] = (ppm_sc_code[]){PPM_SC_MKDIR, -1},
	[PPME_SYSCALL_RMDIR_E] = (ppm_sc_code[]){PPM_SC_RMDIR, -1},
	[PPME_SYSCALL_RMDIR_X] = (ppm_sc_code[]){PPM_SC_RMDIR, -1},
	[PPME_SYSCALL_OPENAT_E] = (ppm_sc_code[]){PPM_SC_OPENAT, -1},
	[PPME_SYSCALL_OPENAT_X] = (ppm_sc_code[]){PPM_SC_OPENAT, -1},
	[PPME_SYSCALL_LINK_E] = (ppm_sc_code[]){PPM_SC_LINK, -1},
	[PPME_SYSCALL_LINK_X] = (ppm_sc_code[]){PPM_SC_LINK, -1},
	[PPME_SYSCALL_LINKAT_E] = (ppm_sc_code[]){PPM_SC_LINKAT, -1},
	[PPME_SYSCALL_LINKAT_X] = (ppm_sc_code[]){PPM_SC_LINKAT, -1},
	[PPME_SYSCALL_UNLINK_E] = (ppm_sc_code[]){PPM_SC_UNLINK, -1},
	[PPME_SYSCALL_UNLINK_X] = (ppm_sc_code[]){PPM_SC_UNLINK, -1},
	[PPME_SYSCALL_UNLINKAT_E] = (ppm_sc_code[]){PPM_SC_UNLINKAT, -1},
	[PPME_SYSCALL_UNLINKAT_X] = (ppm_sc_code[]){PPM_SC_UNLINKAT, -1},
	[PPME_SYSCALL_PREAD_E] = (ppm_sc_code[]){PPM_SC_PREAD64, -1},
	[PPME_SYSCALL_PREAD_X] = (ppm_sc_code[]){PPM_SC_PREAD64, -1},
	[PPME_SYSCALL_PWRITE_E] = (ppm_sc_code[]){PPM_SC_PWRITE64, -1},
	[PPME_SYSCALL_PWRITE_X] = (ppm_sc_code[]){PPM_SC_PWRITE64, -1},
	[PPME_SYSCALL_READV_E] = (ppm_sc_code[]){PPM_SC_READV, -1},
	[PPME_SYSCALL_READV_X] = (ppm_sc_code[]){PPM_SC_READV, -1},
	[PPME_SYSCALL_WRITEV_E] = (ppm_sc_code[]){PPM_SC_WRITEV, -1},
	[PPME_SYSCALL_WRITEV_X] = (ppm_sc_code[]){PPM_SC_WRITEV, -1},
	[PPME_SYSCALL_PREADV_E] = (ppm_sc_code[]){PPM_SC_PREADV, -1},
	[PPME_SYSCALL_PREADV_X] = (ppm_sc_code[]){PPM_SC_PREADV, -1},
	[PPME_SYSCALL_PWRITEV_E] = (ppm_sc_code[]){PPM_SC_PWRITEV, -1},
	[PPME_SYSCALL_PWRITEV_X] = (ppm_sc_code[]){PPM_SC_PWRITEV, -1},
	[PPME_SYSCALL_DUP_E] = (ppm_sc_code[]){PPM_SC_DUP, -1},
	[PPME_SYSCALL_DUP_X] = (ppm_sc_code[]){PPM_SC_DUP, -1},
	[PPME_SYSCALL_SIGNALFD_E] = (ppm_sc_code[]){PPM_SC_SIGNALFD, -1},
	[PPME_SYSCALL_SIGNALFD_X] = (ppm_sc_code[]){PPM_SC_SIGNALFD, -1},
	[PPME_SYSCALL_KILL_E] = (ppm_sc_code[]){PPM_SC_KILL, -1},
	[PPME_SYSCALL_KILL_X] = (ppm_sc_code[]){PPM_SC_KILL, -1},
	[PPME_SYSCALL_TKILL_E] = (ppm_sc_code[]){PPM_SC_TKILL, -1},
	[PPME_SYSCALL_TKILL_X] = (ppm_sc_code[]){PPM_SC_TKILL, -1},
	[PPME_SYSCALL_TGKILL_E] = (ppm_sc_code[]){PPM_SC_TGKILL, -1},
	[PPME_SYSCALL_TGKILL_X] = (ppm_sc_code[]){PPM_SC_TGKILL, -1},
	[PPME_SYSCALL_NANOSLEEP_E] = (ppm_sc_code[]){PPM_SC_NANOSLEEP, -1},
	[PPME_SYSCALL_NANOSLEEP_X] = (ppm_sc_code[]){PPM_SC_NANOSLEEP, -1},
	[PPME_SYSCALL_TIMERFD_CREATE_E] = (ppm_sc_code[]){PPM_SC_TIMERFD_CREATE, -1},
	[PPME_SYSCALL_TIMERFD_CREATE_X] = (ppm_sc_code[]){PPM_SC_TIMERFD_CREATE, -1},
	[PPME_SYSCALL_INOTIFY_INIT_E] = (ppm_sc_code[]){PPM_SC_INOTIFY_INIT, -1},
	[PPME_SYSCALL_INOTIFY_INIT_X] = (ppm_sc_code[]){PPM_SC_INOTIFY_INIT, -1},
	[PPME_SYSCALL_GETRLIMIT_E] = (ppm_sc_code[]){PPM_SC_GETRLIMIT, PPM_SC_UGETRLIMIT, -1},
	[PPME_SYSCALL_GETRLIMIT_X] = (ppm_sc_code[]){PPM_SC_GETRLIMIT, PPM_SC_UGETRLIMIT, -1},
	[PPME_SYSCALL_SETRLIMIT_E] = (ppm_sc_code[]){PPM_SC_SETRLIMIT, -1},
	[PPME_SYSCALL_SETRLIMIT_X] = (ppm_sc_code[]){PPM_SC_SETRLIMIT, -1},
	[PPME_SYSCALL_PRLIMIT_E] = (ppm_sc_code[]){PPM_SC_PRLIMIT64, -1},
	[PPME_SYSCALL_PRLIMIT_X] = (ppm_sc_code[]){PPM_SC_PRLIMIT64, -1},
	[PPME_SCHEDSWITCH_1_E] = (ppm_sc_code[]){PPM_SC_SCHED_SWITCH, -1},
	[PPME_SCHEDSWITCH_1_X] = NULL,
	[PPME_DROP_E] = NULL,
	[PPME_DROP_X] = NULL,
	[PPME_SYSCALL_FCNTL_E] = (ppm_sc_code[]){PPM_SC_FCNTL, PPM_SC_FCNTL64, -1},
	[PPME_SYSCALL_FCNTL_X] = (ppm_sc_code[]){PPM_SC_FCNTL, PPM_SC_FCNTL64, -1},
	[PPME_SCHEDSWITCH_6_E] = (ppm_sc_code[]){PPM_SC_SCHED_SWITCH, -1},
	[PPME_SCHEDSWITCH_6_X] = NULL,
	[PPME_SYSCALL_EXECVE_13_E] = (ppm_sc_code[]){PPM_SC_EXECVE, -1},
	[PPME_SYSCALL_EXECVE_13_X] = (ppm_sc_code[]){PPM_SC_EXECVE, -1},
	[PPME_SYSCALL_CLONE_16_E] = (ppm_sc_code[]){PPM_SC_CLONE, -1},
	[PPME_SYSCALL_CLONE_16_X] = (ppm_sc_code[]){PPM_SC_CLONE, -1},
	[PPME_SYSCALL_BRK_4_E] = (ppm_sc_code[]){PPM_SC_BRK, -1},
	[PPME_SYSCALL_BRK_4_X] = (ppm_sc_code[]){PPM_SC_BRK, -1},
	[PPME_SYSCALL_MMAP_E] = (ppm_sc_code[]){PPM_SC_MMAP, -1},
	[PPME_SYSCALL_MMAP_X] = (ppm_sc_code[]){PPM_SC_MMAP, -1},
	[PPME_SYSCALL_MMAP2_E] = (ppm_sc_code[]){PPM_SC_MMAP2, -1},
	[PPME_SYSCALL_MMAP2_X] = (ppm_sc_code[]){PPM_SC_MMAP2, -1},
	[PPME_SYSCALL_MUNMAP_E] = (ppm_sc_code[]){PPM_SC_MUNMAP, -1},
	[PPME_SYSCALL_MUNMAP_X] = (ppm_sc_code[]){PPM_SC_MUNMAP, -1},
	[PPME_SYSCALL_SPLICE_E] = (ppm_sc_code[]){PPM_SC_SPLICE, -1},
	[PPME_SYSCALL_SPLICE_X] = (ppm_sc_code[]){PPM_SC_SPLICE, -1},
	[PPME_SYSCALL_PTRACE_E] = (ppm_sc_code[]){PPM_SC_PTRACE, -1},
	[PPME_SYSCALL_PTRACE_X] = (ppm_sc_code[]){PPM_SC_PTRACE, -1},
	[PPME_SYSCALL_IOCTL_3_E] = (ppm_sc_code[]){PPM_SC_IOCTL, -1},
	[PPME_SYSCALL_IOCTL_3_X] = (ppm_sc_code[]){PPM_SC_IOCTL, -1},
	[PPME_SYSCALL_EXECVE_14_E] = (ppm_sc_code[]){PPM_SC_EXECVE, -1},
	[PPME_SYSCALL_EXECVE_14_X] = (ppm_sc_code[]){PPM_SC_EXECVE, -1},
	[PPME_SYSCALL_RENAME_E] = (ppm_sc_code[]){PPM_SC_RENAME, -1},
	[PPME_SYSCALL_RENAME_X] = (ppm_sc_code[]){PPM_SC_RENAME, -1},
	[PPME_SYSCALL_RENAMEAT_E] = (ppm_sc_code[]){PPM_SC_RENAMEAT, -1},
	[PPME_SYSCALL_RENAMEAT_X] = (ppm_sc_code[]){PPM_SC_RENAMEAT, -1},
	[PPME_SYSCALL_SYMLINK_E] = (ppm_sc_code[]){PPM_SC_SYMLINK, -1},
	[PPME_SYSCALL_SYMLINK_X] = (ppm_sc_code[]){PPM_SC_SYMLINK, -1},
	[PPME_SYSCALL_SYMLINKAT_E] = (ppm_sc_code[]){PPM_SC_SYMLINKAT, -1},
	[PPME_SYSCALL_SYMLINKAT_X] = (ppm_sc_code[]){PPM_SC_SYMLINKAT, -1},
	[PPME_SYSCALL_FORK_E] = (ppm_sc_code[]){PPM_SC_FORK, -1},
	[PPME_SYSCALL_FORK_X] = (ppm_sc_code[]){PPM_SC_FORK, -1},
	[PPME_SYSCALL_VFORK_E] = (ppm_sc_code[]){PPM_SC_VFORK, -1},
	[PPME_SYSCALL_VFORK_X] = (ppm_sc_code[]){PPM_SC_VFORK, -1},
	[PPME_PROCEXIT_1_E] = (ppm_sc_code[]){PPM_SC_SCHED_PROCESS_EXIT, -1},
	[PPME_PROCEXIT_1_X] = NULL,
	[PPME_SYSCALL_SENDFILE_E] = (ppm_sc_code[]){PPM_SC_SENDFILE, PPM_SC_SENDFILE64, -1},
	[PPME_SYSCALL_SENDFILE_X] = (ppm_sc_code[]){PPM_SC_SENDFILE, PPM_SC_SENDFILE64, -1},
	[PPME_SYSCALL_QUOTACTL_E] = (ppm_sc_code[]){PPM_SC_QUOTACTL, -1},
	[PPME_SYSCALL_QUOTACTL_X] = (ppm_sc_code[]){PPM_SC_QUOTACTL, -1},
	[PPME_SYSCALL_SETRESUID_E] = (ppm_sc_code[]){PPM_SC_SETRESUID, PPM_SC_SETRESUID32,  -1},
	[PPME_SYSCALL_SETRESUID_X] = (ppm_sc_code[]){PPM_SC_SETRESUID, PPM_SC_SETRESUID32,  -1},
	[PPME_SYSCALL_SETRESGID_E] = (ppm_sc_code[]){PPM_SC_SETRESGID,  PPM_SC_SETRESGID32, -1},
	[PPME_SYSCALL_SETRESGID_X] = (ppm_sc_code[]){PPM_SC_SETRESGID, PPM_SC_SETRESGID32, -1},
	[PPME_SCAPEVENT_E] = NULL,
	[PPME_SCAPEVENT_X] = NULL,
	[PPME_SYSCALL_SETUID_E] = (ppm_sc_code[]){PPM_SC_SETUID, PPM_SC_SETUID32, -1},
	[PPME_SYSCALL_SETUID_X] = (ppm_sc_code[]){PPM_SC_SETUID, PPM_SC_SETUID32, -1},
	[PPME_SYSCALL_SETGID_E] = (ppm_sc_code[]){PPM_SC_SETGID, PPM_SC_SETGID32, -1},
	[PPME_SYSCALL_SETGID_X] = (ppm_sc_code[]){PPM_SC_SETGID, PPM_SC_SETGID32, -1},
	[PPME_SYSCALL_GETUID_E] = (ppm_sc_code[]){PPM_SC_GETUID, PPM_SC_GETUID32, -1},
	[PPME_SYSCALL_GETUID_X] = (ppm_sc_code[]){PPM_SC_GETUID, PPM_SC_GETUID32, -1},
	[PPME_SYSCALL_GETEUID_E] = (ppm_sc_code[]){PPM_SC_GETEUID, PPM_SC_GETEUID32,  -1},
	[PPME_SYSCALL_GETEUID_X] = (ppm_sc_code[]){PPM_SC_GETEUID, PPM_SC_GETEUID32, -1},
	[PPME_SYSCALL_GETGID_E] = (ppm_sc_code[]){PPM_SC_GETGID, PPM_SC_GETGID32, -1},
	[PPME_SYSCALL_GETGID_X] = (ppm_sc_code[]){PPM_SC_GETGID, PPM_SC_GETGID32, -1},
	[PPME_SYSCALL_GETEGID_E] = (ppm_sc_code[]){PPM_SC_GETEGID, PPM_SC_GETEGID32, -1},
	[PPME_SYSCALL_GETEGID_X] = (ppm_sc_code[]){PPM_SC_GETEGID, PPM_SC_GETEGID32, -1},
	[PPME_SYSCALL_GETRESUID_E] = (ppm_sc_code[]){PPM_SC_GETRESUID,  PPM_SC_GETRESUID32, -1},
	[PPME_SYSCALL_GETRESUID_X] = (ppm_sc_code[]){PPM_SC_GETRESUID, PPM_SC_GETRESUID32, -1},
	[PPME_SYSCALL_GETRESGID_E] = (ppm_sc_code[]){PPM_SC_GETRESGID, PPM_SC_GETRESGID32, -1},
	[PPME_SYSCALL_GETRESGID_X] = (ppm_sc_code[]){PPM_SC_GETRESGID, PPM_SC_GETRESGID32, -1},
	[PPME_SYSCALL_EXECVE_15_E] = (ppm_sc_code[]){PPM_SC_EXECVE, -1},
	[PPME_SYSCALL_EXECVE_15_X] = (ppm_sc_code[]){PPM_SC_EXECVE, -1},
	[PPME_SYSCALL_CLONE_17_E] = (ppm_sc_code[]){PPM_SC_CLONE, -1},
	[PPME_SYSCALL_CLONE_17_X] = (ppm_sc_code[]){PPM_SC_CLONE, -1},
	[PPME_SYSCALL_FORK_17_E] = (ppm_sc_code[]){PPM_SC_FORK, -1},
	[PPME_SYSCALL_FORK_17_X] = (ppm_sc_code[]){PPM_SC_FORK, -1},
	[PPME_SYSCALL_VFORK_17_E] = (ppm_sc_code[]){PPM_SC_VFORK, -1},
	[PPME_SYSCALL_VFORK_17_X] = (ppm_sc_code[]){PPM_SC_VFORK, -1},
	[PPME_SYSCALL_CLONE_20_E] = (ppm_sc_code[]){PPM_SC_CLONE, -1},
	[PPME_SYSCALL_CLONE_20_X] = (ppm_sc_code[]){PPM_SC_CLONE, -1},
	[PPME_SYSCALL_FORK_20_E] = (ppm_sc_code[]){PPM_SC_FORK, -1},
	[PPME_SYSCALL_FORK_20_X] = (ppm_sc_code[]){PPM_SC_FORK, -1},
	[PPME_SYSCALL_VFORK_20_E] = (ppm_sc_code[]){PPM_SC_VFORK, -1},
	[PPME_SYSCALL_VFORK_20_X] = (ppm_sc_code[]){PPM_SC_VFORK, -1},
	[PPME_CONTAINER_E] = NULL,
	[PPME_CONTAINER_X] = NULL,
	[PPME_SYSCALL_EXECVE_16_E] = (ppm_sc_code[]){PPM_SC_EXECVE, -1},
	[PPME_SYSCALL_EXECVE_16_X] = (ppm_sc_code[]){PPM_SC_EXECVE, -1},
	[PPME_SIGNALDELIVER_E] = (ppm_sc_code[]){PPM_SC_SIGNAL_DELIVER, -1},
	[PPME_SIGNALDELIVER_X] = NULL,
	[PPME_PROCINFO_E] = NULL,
	[PPME_PROCINFO_X] = NULL,
	[PPME_SYSCALL_GETDENTS_E] = (ppm_sc_code[]){PPM_SC_GETDENTS, -1},
	[PPME_SYSCALL_GETDENTS_X] = (ppm_sc_code[]){PPM_SC_GETDENTS, -1},
	[PPME_SYSCALL_GETDENTS64_E] = (ppm_sc_code[]){PPM_SC_GETDENTS64, -1},
	[PPME_SYSCALL_GETDENTS64_X] = (ppm_sc_code[]){PPM_SC_GETDENTS64, -1},
	[PPME_SYSCALL_SETNS_E] = (ppm_sc_code[]){PPM_SC_SETNS, -1},
	[PPME_SYSCALL_SETNS_X] = (ppm_sc_code[]){PPM_SC_SETNS, -1},
	[PPME_SYSCALL_FLOCK_E] = (ppm_sc_code[]){PPM_SC_FLOCK, -1},
	[PPME_SYSCALL_FLOCK_X] = (ppm_sc_code[]){PPM_SC_FLOCK, -1},
	[PPME_CPU_HOTPLUG_E] = NULL,
	[PPME_CPU_HOTPLUG_X] = NULL,
	[PPME_SOCKET_ACCEPT_5_E] = (ppm_sc_code[]){PPM_SC_ACCEPT, -1},
	[PPME_SOCKET_ACCEPT_5_X] = (ppm_sc_code[]){PPM_SC_ACCEPT, -1},
	[PPME_SOCKET_ACCEPT4_5_E] = (ppm_sc_code[]){PPM_SC_ACCEPT4, -1},
	[PPME_SOCKET_ACCEPT4_5_X] = (ppm_sc_code[]){PPM_SC_ACCEPT4, -1},
	[PPME_SYSCALL_SEMOP_E] = (ppm_sc_code[]){PPM_SC_SEMOP, -1},
	[PPME_SYSCALL_SEMOP_X] = (ppm_sc_code[]){PPM_SC_SEMOP, -1},
	[PPME_SYSCALL_SEMCTL_E] = (ppm_sc_code[]){PPM_SC_SEMCTL, -1},
	[PPME_SYSCALL_SEMCTL_X] = (ppm_sc_code[]){PPM_SC_SEMCTL, -1},
	[PPME_SYSCALL_PPOLL_E] = (ppm_sc_code[]){PPM_SC_PPOLL, -1},
	[PPME_SYSCALL_PPOLL_X] = (ppm_sc_code[]){PPM_SC_PPOLL, -1},
	[PPME_SYSCALL_MOUNT_E] = (ppm_sc_code[]){PPM_SC_MOUNT, -1},
	[PPME_SYSCALL_MOUNT_X] = (ppm_sc_code[]){PPM_SC_MOUNT, -1},
	[PPME_SYSCALL_UMOUNT_E] = (ppm_sc_code[]){PPM_SC_UMOUNT, -1},
	[PPME_SYSCALL_UMOUNT_X] = (ppm_sc_code[]){PPM_SC_UMOUNT, -1},
	[PPME_K8S_E] = NULL,
	[PPME_K8S_X] = NULL,
	[PPME_SYSCALL_SEMGET_E] = (ppm_sc_code[]){PPM_SC_SEMGET, -1},
	[PPME_SYSCALL_SEMGET_X] = (ppm_sc_code[]){PPM_SC_SEMGET, -1},
	[PPME_SYSCALL_ACCESS_E] = (ppm_sc_code[]){PPM_SC_ACCESS, -1},
	[PPME_SYSCALL_ACCESS_X] = (ppm_sc_code[]){PPM_SC_ACCESS, -1},
	[PPME_SYSCALL_CHROOT_E] = (ppm_sc_code[]){PPM_SC_CHROOT, -1},
	[PPME_SYSCALL_CHROOT_X] = (ppm_sc_code[]){PPM_SC_CHROOT, -1},
	[PPME_TRACER_E] = NULL,
	[PPME_TRACER_X] = NULL,
	[PPME_MESOS_E] = NULL,
	[PPME_MESOS_X] = NULL,
	[PPME_CONTAINER_JSON_E] = NULL,
	[PPME_CONTAINER_JSON_X] = NULL,
	[PPME_SYSCALL_SETSID_E] = (ppm_sc_code[]){PPM_SC_SETSID, -1},
	[PPME_SYSCALL_SETSID_X] = (ppm_sc_code[]){PPM_SC_SETSID, -1},
	[PPME_SYSCALL_MKDIR_2_E] = (ppm_sc_code[]){PPM_SC_MKDIR, -1},
	[PPME_SYSCALL_MKDIR_2_X] = (ppm_sc_code[]){PPM_SC_MKDIR, -1},
	[PPME_SYSCALL_RMDIR_2_E] = (ppm_sc_code[]){PPM_SC_RMDIR, -1},
	[PPME_SYSCALL_RMDIR_2_X] = (ppm_sc_code[]){PPM_SC_RMDIR, -1},
	[PPME_NOTIFICATION_E] = NULL,
	[PPME_NOTIFICATION_X] = NULL,
	[PPME_SYSCALL_EXECVE_17_E] = (ppm_sc_code[]){PPM_SC_EXECVE, -1},
	[PPME_SYSCALL_EXECVE_17_X] = (ppm_sc_code[]){PPM_SC_EXECVE, -1},
	[PPME_SYSCALL_UNSHARE_E] = (ppm_sc_code[]){PPM_SC_UNSHARE, -1},
	[PPME_SYSCALL_UNSHARE_X] = (ppm_sc_code[]){PPM_SC_UNSHARE, -1},
	[PPME_INFRASTRUCTURE_EVENT_E] = NULL,
	[PPME_INFRASTRUCTURE_EVENT_X] = NULL,
	[PPME_SYSCALL_EXECVE_18_E] = (ppm_sc_code[]){PPM_SC_EXECVE, -1},
	[PPME_SYSCALL_EXECVE_18_X] = (ppm_sc_code[]){PPM_SC_EXECVE, -1},
	[PPME_PAGE_FAULT_E] = (ppm_sc_code[]){PPM_SC_PAGE_FAULT_USER, PPM_SC_PAGE_FAULT_KERNEL, -1},
	[PPME_PAGE_FAULT_X] = NULL,
	[PPME_SYSCALL_EXECVE_19_E] = (ppm_sc_code[]){PPM_SC_EXECVE, -1},
	[PPME_SYSCALL_EXECVE_19_X] = (ppm_sc_code[]){PPM_SC_EXECVE, -1},
	[PPME_SYSCALL_SETPGID_E] = (ppm_sc_code[]){PPM_SC_SETPGID, -1},
	[PPME_SYSCALL_SETPGID_X] = (ppm_sc_code[]){PPM_SC_SETPGID, -1},
	[PPME_SYSCALL_BPF_E] = (ppm_sc_code[]){PPM_SC_BPF, -1},
	[PPME_SYSCALL_BPF_X] = (ppm_sc_code[]){PPM_SC_BPF, -1},
	[PPME_SYSCALL_SECCOMP_E] = (ppm_sc_code[]){PPM_SC_SECCOMP, -1},
	[PPME_SYSCALL_SECCOMP_X] = (ppm_sc_code[]){PPM_SC_SECCOMP, -1},
	[PPME_SYSCALL_UNLINK_2_E] = (ppm_sc_code[]){PPM_SC_UNLINK, -1},
	[PPME_SYSCALL_UNLINK_2_X] = (ppm_sc_code[]){PPM_SC_UNLINK, -1},
	[PPME_SYSCALL_UNLINKAT_2_E] = (ppm_sc_code[]){PPM_SC_UNLINKAT, -1},
	[PPME_SYSCALL_UNLINKAT_2_X] = (ppm_sc_code[]){PPM_SC_UNLINKAT, -1},
	[PPME_SYSCALL_MKDIRAT_E] = (ppm_sc_code[]){PPM_SC_MKDIRAT, -1},
	[PPME_SYSCALL_MKDIRAT_X] = (ppm_sc_code[]){PPM_SC_MKDIRAT, -1},
	[PPME_SYSCALL_OPENAT_2_E] = (ppm_sc_code[]){PPM_SC_OPENAT, -1},
	[PPME_SYSCALL_OPENAT_2_X] = (ppm_sc_code[]){PPM_SC_OPENAT, -1},
	[PPME_SYSCALL_LINK_2_E] = (ppm_sc_code[]){PPM_SC_LINK, -1},
	[PPME_SYSCALL_LINK_2_X] = (ppm_sc_code[]){PPM_SC_LINK, -1},
	[PPME_SYSCALL_LINKAT_2_E] = (ppm_sc_code[]){PPM_SC_LINKAT, -1},
	[PPME_SYSCALL_LINKAT_2_X] = (ppm_sc_code[]){PPM_SC_LINKAT, -1},
	[PPME_SYSCALL_FCHMODAT_E] = (ppm_sc_code[]){PPM_SC_FCHMODAT, -1},
	[PPME_SYSCALL_FCHMODAT_X] = (ppm_sc_code[]){PPM_SC_FCHMODAT, -1},
	[PPME_SYSCALL_CHMOD_E] = (ppm_sc_code[]){PPM_SC_CHMOD, -1},
	[PPME_SYSCALL_CHMOD_X] = (ppm_sc_code[]){PPM_SC_CHMOD, -1},
	[PPME_SYSCALL_FCHMOD_E] = (ppm_sc_code[]){PPM_SC_FCHMOD, -1},
	[PPME_SYSCALL_FCHMOD_X] = (ppm_sc_code[]){PPM_SC_FCHMOD, -1},
	[PPME_SYSCALL_RENAMEAT2_E] = (ppm_sc_code[]){PPM_SC_RENAMEAT2, -1},
	[PPME_SYSCALL_RENAMEAT2_X] = (ppm_sc_code[]){PPM_SC_RENAMEAT2, -1},
	[PPME_SYSCALL_USERFAULTFD_E] = (ppm_sc_code[]){PPM_SC_USERFAULTFD, -1},
	[PPME_SYSCALL_USERFAULTFD_X] = (ppm_sc_code[]){PPM_SC_USERFAULTFD, -1},
	[PPME_PLUGINEVENT_E] = NULL,
	[PPME_PLUGINEVENT_X] = NULL,
	[PPME_CONTAINER_JSON_2_E] = NULL,
	[PPME_CONTAINER_JSON_2_X] = NULL,
	[PPME_SYSCALL_OPENAT2_E] = (ppm_sc_code[]){PPM_SC_OPENAT2, -1},
	[PPME_SYSCALL_OPENAT2_X] = (ppm_sc_code[]){PPM_SC_OPENAT2, -1},
	[PPME_SYSCALL_MPROTECT_E] = (ppm_sc_code[]){PPM_SC_MPROTECT, -1},
	[PPME_SYSCALL_MPROTECT_X] = (ppm_sc_code[]){PPM_SC_MPROTECT, -1},
	[PPME_SYSCALL_EXECVEAT_E] = (ppm_sc_code[]){PPM_SC_EXECVEAT, -1},
	[PPME_SYSCALL_EXECVEAT_X] = (ppm_sc_code[]){PPM_SC_EXECVEAT, -1},
	[PPME_SYSCALL_COPY_FILE_RANGE_E] = (ppm_sc_code[]){PPM_SC_COPY_FILE_RANGE, -1},
	[PPME_SYSCALL_COPY_FILE_RANGE_X] = (ppm_sc_code[]){PPM_SC_COPY_FILE_RANGE, -1},
	[PPME_SYSCALL_CLONE3_E] = (ppm_sc_code[]){PPM_SC_CLONE3, -1},
	[PPME_SYSCALL_CLONE3_X] = (ppm_sc_code[]){PPM_SC_CLONE3, -1},
	[PPME_SYSCALL_OPEN_BY_HANDLE_AT_E] = (ppm_sc_code[]){PPM_SC_OPEN_BY_HANDLE_AT, -1},
	[PPME_SYSCALL_OPEN_BY_HANDLE_AT_X] = (ppm_sc_code[]){PPM_SC_OPEN_BY_HANDLE_AT, -1},
	[PPME_SYSCALL_IO_URING_SETUP_E] = (ppm_sc_code[]){PPM_SC_IO_URING_SETUP, -1},
	[PPME_SYSCALL_IO_URING_SETUP_X] = (ppm_sc_code[]){PPM_SC_IO_URING_SETUP, -1},
	[PPME_SYSCALL_IO_URING_ENTER_E] = (ppm_sc_code[]){PPM_SC_IO_URING_ENTER, -1},
	[PPME_SYSCALL_IO_URING_ENTER_X] = (ppm_sc_code[]){PPM_SC_IO_URING_ENTER, -1},
	[PPME_SYSCALL_IO_URING_REGISTER_E] = (ppm_sc_code[]){PPM_SC_IO_URING_REGISTER, -1},
	[PPME_SYSCALL_IO_URING_REGISTER_X] = (ppm_sc_code[]){PPM_SC_IO_URING_REGISTER, -1},
	[PPME_SYSCALL_MLOCK_E] = (ppm_sc_code[]){PPM_SC_MLOCK, -1},
	[PPME_SYSCALL_MLOCK_X] = (ppm_sc_code[]){PPM_SC_MLOCK, -1},
	[PPME_SYSCALL_MUNLOCK_E] = (ppm_sc_code[]){PPM_SC_MUNLOCK, -1},
	[PPME_SYSCALL_MUNLOCK_X] = (ppm_sc_code[]){PPM_SC_MUNLOCK, -1},
	[PPME_SYSCALL_MLOCKALL_E] = (ppm_sc_code[]){PPM_SC_MLOCKALL, -1},
	[PPME_SYSCALL_MLOCKALL_X] = (ppm_sc_code[]){PPM_SC_MLOCKALL, -1},
	[PPME_SYSCALL_MUNLOCKALL_E] = (ppm_sc_code[]){PPM_SC_MUNLOCKALL, -1},
	[PPME_SYSCALL_MUNLOCKALL_X] = (ppm_sc_code[]){PPM_SC_MUNLOCKALL, -1},
	[PPME_SYSCALL_CAPSET_E] = (ppm_sc_code[]){PPM_SC_CAPSET, -1},
	[PPME_SYSCALL_CAPSET_X] = (ppm_sc_code[]){PPM_SC_CAPSET, -1},
	[PPME_USER_ADDED_E] = NULL,
	[PPME_USER_ADDED_X] = NULL,
	[PPME_USER_DELETED_E] = NULL,
	[PPME_USER_DELETED_X] = NULL,
	[PPME_GROUP_ADDED_E] = NULL,
	[PPME_GROUP_ADDED_X] = NULL,
	[PPME_GROUP_DELETED_E] = NULL,
	[PPME_GROUP_DELETED_X] = NULL,
	[PPME_SYSCALL_DUP2_E] = (ppm_sc_code[]){PPM_SC_DUP2, -1},
	[PPME_SYSCALL_DUP2_X] = (ppm_sc_code[]){PPM_SC_DUP2, -1},
	[PPME_SYSCALL_DUP3_E] = (ppm_sc_code[]){PPM_SC_DUP3, -1},
	[PPME_SYSCALL_DUP3_X] = (ppm_sc_code[]){PPM_SC_DUP3, -1},
	[PPME_SYSCALL_DUP_1_E] = (ppm_sc_code[]){PPM_SC_DUP, -1},
	[PPME_SYSCALL_DUP_1_X] = (ppm_sc_code[]){PPM_SC_DUP, -1},
	[PPME_SYSCALL_BPF_2_E] = (ppm_sc_code[]){PPM_SC_BPF, -1},
	[PPME_SYSCALL_BPF_2_X] = (ppm_sc_code[]){PPM_SC_BPF, -1},
	[PPME_SYSCALL_MLOCK2_E] = (ppm_sc_code[]){PPM_SC_MLOCK2, -1},
	[PPME_SYSCALL_MLOCK2_X] = (ppm_sc_code[]){PPM_SC_MLOCK2, -1},
	[PPME_SYSCALL_FSCONFIG_E] = (ppm_sc_code[]){PPM_SC_FSCONFIG, -1},
	[PPME_SYSCALL_FSCONFIG_X] = (ppm_sc_code[]){PPM_SC_FSCONFIG, -1},
	[PPME_SYSCALL_EPOLL_CREATE_E] = (ppm_sc_code[]){PPM_SC_EPOLL_CREATE, -1},
	[PPME_SYSCALL_EPOLL_CREATE_X] = (ppm_sc_code[]){PPM_SC_EPOLL_CREATE, -1},
	[PPME_SYSCALL_EPOLL_CREATE1_E] = (ppm_sc_code[]){PPM_SC_EPOLL_CREATE1, -1},
	[PPME_SYSCALL_EPOLL_CREATE1_X] = (ppm_sc_code[]){PPM_SC_EPOLL_CREATE1, -1},
	[PPME_SYSCALL_CHOWN_E] = (ppm_sc_code[]){PPM_SC_CHOWN, -1},
	[PPME_SYSCALL_CHOWN_X] = (ppm_sc_code[]){PPM_SC_CHOWN, -1},
	[PPME_SYSCALL_LCHOWN_E] = (ppm_sc_code[]){PPM_SC_LCHOWN, -1},
	[PPME_SYSCALL_LCHOWN_X] = (ppm_sc_code[]){PPM_SC_LCHOWN, -1},
	[PPME_SYSCALL_FCHOWN_E] = (ppm_sc_code[]){PPM_SC_FCHOWN, -1},
	[PPME_SYSCALL_FCHOWN_X] = (ppm_sc_code[]){PPM_SC_FCHOWN, -1},
	[PPME_SYSCALL_FCHOWNAT_E] = (ppm_sc_code[]){PPM_SC_FCHOWNAT, -1},
	[PPME_SYSCALL_FCHOWNAT_X] = (ppm_sc_code[]){PPM_SC_FCHOWNAT, -1},
	[PPME_SYSCALL_UMOUNT_1_E] = (ppm_sc_code[]){PPM_SC_UMOUNT, -1},
	[PPME_SYSCALL_UMOUNT_1_X] = (ppm_sc_code[]){PPM_SC_UMOUNT, -1},
	[PPME_SOCKET_ACCEPT4_6_E] = (ppm_sc_code[]){PPM_SC_ACCEPT4, -1},
	[PPME_SOCKET_ACCEPT4_6_X] = (ppm_sc_code[]){PPM_SC_ACCEPT4, -1},
	[PPME_SYSCALL_UMOUNT2_E] = (ppm_sc_code[]){PPM_SC_UMOUNT2, -1},
	[PPME_SYSCALL_UMOUNT2_X] = (ppm_sc_code[]){PPM_SC_UMOUNT2, -1},
	[PPME_SYSCALL_PIPE2_E] = (ppm_sc_code[]){PPM_SC_PIPE2, -1},
	[PPME_SYSCALL_PIPE2_X] = (ppm_sc_code[]){PPM_SC_PIPE2, -1},
	[PPME_SYSCALL_INOTIFY_INIT1_E] = (ppm_sc_code[]){PPM_SC_INOTIFY_INIT1, -1},
	[PPME_SYSCALL_INOTIFY_INIT1_X] = (ppm_sc_code[]){PPM_SC_INOTIFY_INIT1, -1},
	[PPME_SYSCALL_EVENTFD2_E] = (ppm_sc_code[]){PPM_SC_EVENTFD2, -1},
	[PPME_SYSCALL_EVENTFD2_X] = (ppm_sc_code[]){PPM_SC_EVENTFD2, -1},
	[PPME_SYSCALL_SIGNALFD4_E] = (ppm_sc_code[]){PPM_SC_SIGNALFD4, -1},
	[PPME_SYSCALL_SIGNALFD4_X] = (ppm_sc_code[]){PPM_SC_SIGNALFD4, -1},
	[PPME_ASYNCEVENT_E] = NULL,
	[PPME_ASYNCEVENT_X] = NULL,
	[PPME_SYSCALL_MEMFD_CREATE_E] = (ppm_sc_code[]){PPM_SC_MEMFD_CREATE,-1},
	[PPME_SYSCALL_MEMFD_CREATE_X] = (ppm_sc_code[]){PPM_SC_MEMFD_CREATE, -1},
	[PPME_SYSCALL_PIDFD_GETFD_E] = (ppm_sc_code[]){PPM_SC_PIDFD_GETFD, -1},
	[PPME_SYSCALL_PIDFD_GETFD_X] = (ppm_sc_code[]){PPM_SC_PIDFD_GETFD, -1},
	[PPME_SYSCALL_PIDFD_OPEN_E] = (ppm_sc_code[]){PPM_SC_PIDFD_OPEN, -1},
	[PPME_SYSCALL_PIDFD_OPEN_X] = (ppm_sc_code[]){PPM_SC_PIDFD_OPEN, -1},
};

#if defined(__GNUC__) || (__STDC_VERSION__ >=201112L)
_Static_assert(sizeof(g_events_to_sc_map) / sizeof(*g_events_to_sc_map) == PPM_EVENT_MAX, "Missing entries in g_events_to_sc_map table.");
#endif

int scap_get_modifies_state_ppm_sc(OUT uint8_t ppm_sc_array[PPM_SC_MAX])
{
	if(ppm_sc_array == NULL)
	{
		return SCAP_FAILURE;
	}

	/* Clear the array before using it.
	 * This is not necessary but just to be future-proof.
	 */
	memset(ppm_sc_array, 0, sizeof(*ppm_sc_array) * PPM_SC_MAX);

	uint8_t events_array[PPM_EVENT_MAX] = {0};
	// Collect EF_MODIFIES_STATE events
	for (int event_nr = 2; event_nr < PPM_EVENT_MAX; event_nr++)
	{
		if (g_event_info[event_nr].flags & EF_MODIFIES_STATE &&
		   (g_event_info[event_nr].category & EC_SYSCALL || g_event_info[event_nr].category & EC_TRACEPOINT))
		{
			events_array[event_nr] = 1;
		}
	}

	// Transform them into ppm_sc
	scap_get_ppm_sc_from_events(events_array, ppm_sc_array);

	// Append UF_NEVER_DROP syscalls too!
	for (int syscall_nr = 0; syscall_nr < SYSCALL_TABLE_SIZE; syscall_nr++)
	{
		if (g_syscall_table[syscall_nr].flags & UF_NEVER_DROP)
		{
			uint32_t code = g_syscall_table[syscall_nr].ppm_sc;
			ppm_sc_array[code] = 1;
		}
	}
	return SCAP_SUCCESS;
}

int scap_get_events_from_ppm_sc(IN const uint8_t ppm_sc_array[PPM_SC_MAX], OUT uint8_t events_array[PPM_EVENT_MAX])
{
	if(ppm_sc_array == NULL || events_array == NULL)
	{
		return SCAP_FAILURE;
	}

	/* Clear the array before using it.
	 * This is not necessary but just to be future-proof.
	 */
	memset(events_array, 0, sizeof(*events_array) * PPM_EVENT_MAX);

	// Load associated events from event_table, skip generics
	for(int ev = 0; ev < PPM_EVENT_MAX; ev++)
	{
		const ppm_sc_code *sc_codes = g_events_to_sc_map[ev];
		while (sc_codes && *sc_codes != -1)
		{
			const ppm_sc_code sc_code = *sc_codes;
			sc_codes++;
			if(ppm_sc_array[sc_code])
			{
				events_array[ev] = 1;
				break;
			}
		}
	}

	return SCAP_SUCCESS;
}

int scap_get_ppm_sc_from_events(IN const uint8_t events_array[PPM_EVENT_MAX], OUT uint8_t ppm_sc_array[PPM_SC_MAX])
{
	if (events_array == NULL || ppm_sc_array == NULL)
	{
		return SCAP_FAILURE;
	}

	/* Clear the array before using it.
	 * This is not necessary but just to be future-proof.
	 */
	memset(ppm_sc_array, 0, sizeof(*ppm_sc_array) * PPM_SC_MAX);

	// Load associated ppm_sc from event_table
	for (int ev = 0; ev < PPM_EVENT_MAX; ev++)
	{
		if(!events_array[ev])
		{
			continue;
		}

		const ppm_sc_code *sc_codes = g_events_to_sc_map[ev];
		while (sc_codes && *sc_codes != -1)
		{
			ppm_sc_array[*sc_codes] = 1;
			sc_codes++;
		}
	}
	return SCAP_SUCCESS;
}

ppm_sc_code scap_ppm_sc_from_name(const char *name)
{
	int start = 0;
	int max = PPM_SC_MAX;
	const char *sc_name = name;

	if(name == NULL)
	{
		return -1;
	}

	for (int i = start; i < max; i++)
	{
		/* We need the strlen because all empty entries in the syscall_info_table are "", so
		 * if we pass a "" we will have a match!
		 */
		if(strlen(sc_name) !=0 && strcmp(sc_name, scap_get_ppm_sc_name(i)) == 0)
		{
			return i;
		}
	}
	return -1;
}

ppm_sc_code scap_native_id_to_ppm_sc(int native_id)
{
	if (native_id < 0 || native_id >= SYSCALL_TABLE_SIZE)
	{
		return PPM_SC_UNKNOWN;
	}
	return g_syscall_table[native_id].ppm_sc;
}

/* Here we must be sure that there is a 1:1 relation between syscall_id:ppm_sc 
 * otherwise there is the risk to return only the first occurrence
 */
int scap_ppm_sc_to_native_id(ppm_sc_code sc_code)
{
	for(int syscall_nr = 0; syscall_nr < SYSCALL_TABLE_SIZE; syscall_nr++)
	{
		if(g_syscall_table[syscall_nr].ppm_sc == sc_code)
		{
			return syscall_nr;
		}
	}
	return -1;
}
