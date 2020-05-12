#include <linux/version.h>
#include <linux/module.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/highmem.h>
#include <asm/unistd.h>
//#include <sys/syscall.h>
#include <linux/module.h>  // Needed by all modules
#include <linux/kernel.h>  // Needed for KERN_INFO
#include <linux/fs.h>      // Needed by filp
#include <asm/uaccess.h>   // Needed by segment descriptors
MODULE_LICENSE("GPL");

// IOCTL commands
#define IOCTL_PATCH_TABLE 0x00000001
#define IOCTL_FIX_TABLE 0x00000004



//direccion de systable, se encuentra con el siguiente comando: cat /proc/kallsyms  | grep sys_call
// tomar la direccion de sys_call_table
//realizar cada vez que se reinicie el sistema puesto que la direccion cambia

//unsigned long *sys_call_table = (unsigned long*)0xffffffffbd114940;
unsigned long *sys_call_table = (unsigned long*)0xffffffff92e00220;
//puntero de la funcion del sys_openat
asmlinkage int (*real_open)(const char* __user, int, int);

asmlinkage int (*original_sys_unlink) (const char *pathname);

/*return -1. this will prevent any process from unlinking any file*/
asmlinkage int hacked_sys_unlink(const char *pathname)
{
	printk("ENTRO AL KACKED??");
	printk(".....%s",pathname);
	printk(".....%i",*pathname);
    printk("RM_CATCHED: unlink( \"%s\" )\n", pathname);
    //return original_sys_unlink(pathname);
	// Create variables
    struct file *f;
    char buf[128];
    mm_segment_t fs;
    int i;
    // Init the buffer with 0
    for(i=0;i<128;i++)
        buf[i] = 0;
    // To see in /var/log/messages that the module is operating
    printk(KERN_INFO "My module is loaded\n");
    // I am using Fedora and for the test I have chosen following file
    // Obviously it is much smaller than the 128 bytes, but hell with it =)
    f = filp_open(pathname, O_RDONLY, 0);
    if(f == NULL)
        printk(KERN_ALERT "filp_open error!!.\n");
    else{
        // Get current segment descriptor
        fs = get_fs();
        // Set segment descriptor associated to kernel space
        set_fs(get_ds());
        // Read the file
        f->f_op->read(f, buf, 128, &f->f_pos);
        // Restore segment descriptor
        set_fs(fs);
        // See what we read from file
        printk(KERN_INFO "buf:%s\n",buf);
    }
    filp_close(f,NULL);
	return -1;
}
        

//Reemplazando la llamada original con la llamada modificada
asmlinkage int custom_open(const char* __user file_name, int flags, int mode)
{
	printk("interceptor: open(\"%s\", %X, %X)\n", file_name,flags,mode);
	return real_open(file_name,flags,mode);
}

/*
Modificando la pagina de la memoria para escritura
Esto es un poco riesgoso ya que se modifica el bit de proteccion a nivel de arquitectura
*/
int make_rw(unsigned long address)
{
	unsigned int level;
	pte_t *pte = lookup_address(address, &level);
	if(pte->pte &~ _PAGE_RW){
		pte->pte |= _PAGE_RW;
		printk("RW seteado\n");
	}
	return 0;
}

/* Protegiendo la pagina contra escritura*/
int make_ro(unsigned long address)
{
	unsigned int level;
	pte_t *pte = lookup_address(address, &level);
	pte->pte = pte->pte &~ _PAGE_RW;
	printk("RO seteado\n");
	return 0;
}


static int __init init_my_module(void)
{
	//para este ejemplo se utiliza la llamada al sistema openat para abir archivos
	printk(KERN_INFO "Inside kernel space\n");
	//cambiando permisos de la pagina
	make_rw((unsigned long)sys_call_table);
	printk(KERN_INFO "cambiando permisos de la pagina\n");
	//guardando el valor de memoria de la llamada original
	original_sys_unlink = (void *)sys_call_table[__NR_unlink];
	printk(KERN_INFO "guardando el valor de memoria de la llamada original\n");
	//insertando nuestra funcion a la direccion de memoria de openat
	*(sys_call_table + __NR_unlink) = (unsigned long) hacked_sys_unlink;
	printk(KERN_INFO "insertando nuestra funcion a la direccion de memoria de openat\n");
	printk("hizo el cambio de pagina \n");
	return 0;
}

static void __exit cleanup_my_module(void)
{
	
	//cambiando la direccion de memoria a modo de escritura
	make_rw((unsigned long)sys_call_table);
	//regresando la funcion original a la direccion de la llamada
	*(sys_call_table + __NR_unlink) = (unsigned long)original_sys_unlink;
	//cambiando la direccion de memoria a modo de lectura. 
	make_ro((unsigned long)sys_call_table);
	printk(KERN_INFO "Exiting kernel space\n");
	return;
}

module_init(init_my_module);
module_exit(cleanup_my_module);