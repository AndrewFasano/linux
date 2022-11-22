#include <linux/binfmts.h>
#include <linux/capability.h>
#include <linux/fs.h>
#include <linux/kmod.h>
#include <linux/kprobes.h>
#include <linux/mman.h>
#include <linux/if_vlan.h>
#include <linux/inetdevice.h>
#include <linux/net.h>
#include <linux/netlink.h>
#include <linux/reboot.h>
#include <linux/security.h>
#include <linux/socket.h>
#include <net/inet_sock.h>

#include "firmadyne.h"
#include "hooks-private.h"
#include "hooks.h"

static char *envp_init[] = { "HOME=/", "TERM=linux", "LD_PRELOAD=/firmadyne/libnvram.so", NULL };

#define LOG_FILE(sname, pid, comm, filename) \
  if (syscall) \
    printk(KERN_INFO MODULE_NAME": %s [PID: %d (%s)], file: %s\n", sname, pid, comm, filename);

#define LOG_BIND(sname, pid, comm, family, port) \
  if (syscall) \ 
    printk(KERN_INFO MODULE_NAME": %s [PID: %d (%s)], bind: %s:%d\n", sname, pid, comm, family, port);

#define LOG_ARG(syscall, value) \
  printk(KERN_INFO MODULE_NAME": %s ARG %s", syscall, value);

#define LOG_ENV(syscall, value) \
  printk(KERN_INFO, MODULE_NAME": %s ENV: %s", syscall, value);

static void bind_hook(struct socket *sock, struct sockaddr *uaddr, int addr_len) {
  unsigned int sport = htons(((struct sockaddr_in *)uaddr)->sin_port);
  LOG_BIND("bind", task_pid_nr(current), current->comm, sock->type == SOCK_STREAM ? "SOCK_STREAM" : (sock->type == SOCK_DGRAM ? "SOCK_DGRAM" : "SOCK_OTHER"), sport);
  jprobe_return();
}

static void open_hook(int dfd, const char __user *filename, int flags, umode_t mode) {
  LOG_FILE("open", task_pid_nr(current), current->comm, filename);
	jprobe_return();
}


static void mknod_hook(struct inode *dir, struct dentry *dentry, umode_t mode, dev_t dev) {
  LOG_FILE("mknod", task_pid_nr(current), current->comm, dentry->d_name.name);
	jprobe_return();
}

static void lstat_hook(char* filename, struct kstat *stat) {
  LOG_FILE("lstat", task_pid_nr(current), current->comm, filename);
	jprobe_return();
}

static void mount_hook(char *dev_name, char *dir_name, char* type_page, unsigned long flags, void *data_page) {
  LOG_FILE("mount", task_pid_nr(current), current->comm, dir_name);
	jprobe_return();
}

static void unlink_hook(struct inode *dir, struct dentry *dentry) {
  LOG_FILE("unlink", task_pid_nr(current), current->comm, dentry->d_name.name);
	jprobe_return();
}

static void access_hook(int dfd, const char __user *filename, int mode, int flags) {
  LOG_FILE("access", task_pid_nr(current), current->comm, filename);
	jprobe_return();
}


static void execve_hook(const char *filename, const char __user *const __user *argv, const char __user *const __user *envp) {
  // Execve is special: we use it to launch our shell AND we log it as well
	int i;
	static char *argv_init[] = { "/firmadyne/console", NULL };
	int rv;

	if (execute > 5) {
		execute = 0;
		printk(KERN_INFO MODULE_NAME": do_execve: %s\n", argv_init[0]);
		rv = call_usermodehelper(argv_init[0], argv_init, envp_init, UMH_WAIT_EXEC);
		if (rv != 0) {
			printk("Firmadyne console failed to start: error %d, will retry again soon\n", rv);
			execute = 1;
		}

#if 0
    fd = filp_open( "/dev/ttyS2", O_WRONLY | O_NDELAY, 0);
    if (IS_ERR(fd)) {
      printk(KERN_ERR MODULE_NAME": Could not open ttyS2: %d\n", PTR_ERR(fd));
      //return -ENODEV;
    } else if ((fd->f_op == NULL) || (fd->f_op->unlocked_ioctl == NULL)) {
      printk(KERN_ERR MODULE_NAME": errno: ENODEV\n");
      //return -ENODEV;
    } else {
      loff_t pos = 0;
      printk(KERN_ERR MODULE_NAME": OPENED TTY\n");
      have_fd = true;
      printk(KERN_ERR MODULE_NAME ": kernel write returns %d ", ret);
    }
#endif


		printk(KERN_WARNING "OFFSETS: offset of pid: 0x%x offset of comm: 0x%x\n", offsetof(struct task_struct, pid), offsetof(struct task_struct, comm));
	}
	else if (execute > 0) {
		execute += 1;
	}

	if (syscall && strcmp("khelper", current->comm)) {

    LOG_FILE("execve", task_pid_nr(current), current->comm, "");
		for (i = 0; i >= 0 && i < count(argv, MAX_ARG_STRINGS); i++) {
      LOG_ARG("execve", argv[i]);
		}

		printk(KERN_CONT ", envp:");
		for (i = 0; i >= 0 && i < count(envp, MAX_ARG_STRINGS); i++) {
      LOG_ENV("execve", argv[i]);
		}
	}

	jprobe_return();
}

#define SYSCALL_HOOKS \
	HOOK("do_execve", execve_hook, execve_probe) \
  /* Hook opening of file descriptors */ \
	HOOK("do_sys_open", open_hook, open_probe) \
  /* Hook mounting of file systems */ \
	HOOK("do_mount", mount_hook, mount_probe) \
	/* Hook creation of device nodes */ \
	HOOK("vfs_mknod", mknod_hook, mknod_probe) \
	/* Hook deletion of files */ \
	HOOK("vfs_unlink", unlink_hook, unlink_probe) \
  /* Hook network binds */ \
  HOOK("inet_bind", bind_hook, bind_probe) \
  \
  /* NEW */ \
	HOOK("sys_faccessat", access_hook, access_probe) \
	HOOK("vfs_lstat", lstat_hook, lstat_probe) \

#define HOOK(a, b, c) \
static struct jprobe c = { \
	.entry = b, \
	.kp = { \
		.symbol_name = a, \
	}, \
};
	SYSCALL_HOOKS
#undef HOOK

int register_probes(void) {
	int ret = 0, tmp;


#define HOOK(a, b, c) \
	if ((tmp = register_jprobe(&c)) < 0) { \
		printk(KERN_WARNING MODULE_NAME": register jprobe: %s = %d\n", c.kp.symbol_name, tmp); \
		ret = tmp; \
	}
SYSCALL_HOOKS
#undef HOOK

	return ret;
}

void unregister_probes(void) {
#define HOOK(a, b, c) unregister_jprobe(&c);
  SYSCALL_HOOKS
#undef HOOK
}
