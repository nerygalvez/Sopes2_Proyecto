/**
*
*	Creando el modulo de procesos
*
*/


/**
*   Los estados se extraen de 
*   /usr/src/linux-headers-4.4.0-127/include/linux/sched.h
*   #define TASK_RUNNING		    0
*   #define TASK_INTERRUPTIBLE	    1
*   #define TASK_UNINTERRUPTIBLE	2
*   #define __TASK_STOPPED		    4
*   #define __TASK_TRACED		    8
*   #define EXIT_DEAD		        16
*   #define EXIT_ZOMBIE		        32
*/

#include <linux/module.h> 

#include <linux/kernel.h>

#include <linux/init.h>

#include <linux/list.h>
#include <linux/types.h>
#include <linux/slab.h>
#include <linux/sched.h>
#include <linux/string.h>
#include <linux/fs.h>
#include <linux/seq_file.h>
#include <linux/proc_fs.h>
#include <asm/uaccess.h> 
#include <linux/hugetlb.h>
#include <linux/sched/signal.h>
#include <linux/sched.h>

#define FileProc "201403525_procesos"
#define Carne "201403525"
#define Nombre "Nery Gonzalo Galvez Gomez"
#define SO "Ubuntu 18.04.4 LTS"
#define Curso "Sistemas Operativos 2"

int contador_procesos = 0; //Para ver cuántos procesos hay
int ejecucion = 0, suspendidos = 0, detenidos = 0, zombies = 0; //Para contar los tipos de procesos

static void obtener_estado(int estado, char cadena_estado[30]){
    contador_procesos++; //Aumento un proceso

    if (estado == 0){
            //seq_printf(m, "\"Ejecutandose\"}");
            ejecucion++;
            strcpy(cadena_estado, "Running");
    }else if (estado == 1){
            //seq_printf(m, "\"Hibernado\"}");
            suspendidos++;
            strcpy(cadena_estado, "Sleeping");
    }else if (estado == 2){
            //seq_printf(m, "\"U\"}");
            suspendidos++;
            strcpy(cadena_estado, "Sleeping");
    }else if (estado == 4){
            //seq_printf(m, "\"Detenido\"}");
            detenidos++;
            strcpy(cadena_estado, "Stopped");
    }else if (estado == 8){
            //seq_printf(m, "\"T\"}");
            detenidos++;
            strcpy(cadena_estado, "Stopped");
    }else if (estado == 16){
            //seq_printf(m, "\"Muerto\"}");
            detenidos++;
            strcpy(cadena_estado, "Stopped");
    }else if (estado == 32){
            //seq_printf(m, "\"Zombie\"}");
            zombies++;
            strcpy(cadena_estado, "Zombie");
    }else{
        zombies++;
        strcpy(cadena_estado, "Zombie"); //Si no es ninguno devuelvo uno por defecto
    }
}


struct task_struct *task;
struct task_struct *task_child;
struct list_head *list;
unsigned long size; 

struct sysinfo i;

static int proc_llenar_archivo(struct seq_file *m, void *v) {
    #define S(x) ((x) << (PAGE_SHIFT -10))
	si_meminfo(&i);


    //Reinicio los contadores
    contador_procesos = 0;
    ejecucion = 0, suspendidos = 0, detenidos = 0, zombies = 0;

    seq_printf(m, "{\n"); //Inicio del json

    seq_printf(m, "\t\"Procesos\" : [\n"); //Inicio del arreglo de procesos

    //Imprimo la informacion de cada uno de los procesos
    for_each_process(task){
        if (contador_procesos != 0) //Si no es el primer proceso pongo una coma para separar los procesos
            seq_printf(m, ",\n");
        
        //Inicio del proceso
        //Con este if compruebo si el proceso tiene el scruct mm que sera para obtener la memoria del proceso
        if(task->mm){
            down_read(&task->mm->mmap_sem); 
            size = (task->mm->mmap->vm_end - task->mm->mmap->vm_start );            
            up_read(&task->mm->mmap_sem);
        }else{
            size=0;
        }


        char cadena_estado[30] = "";
        obtener_estado(task->state, cadena_estado);
        seq_printf(m, "\t\t{\"PID\":%d , \"Nombre\":\"%s\" , \"Usuario\":%d, \"Ram\":%llu , \"Cpu\":%d , \"Estado\":\"%s\"" ,
                   task->pid, task->comm, task->cred->uid, size*100/S(i.totalram), task->cpuset_mem_spread_rotor, cadena_estado);

        //Ahora veo los hijos del proceso
        int contador_hijos = 0;
        seq_printf(m, ", \"Hijos\" : [\n\t\t\t\t"); //Inicio del arreglo de hijos del proceso
        list_for_each(list, &task->children){

            if (contador_hijos != 0) //Si no es el primer proceso hijo pongo una coma para separar los procesos hijos
                seq_printf(m, ",\n");

            task_child = list_entry(list, struct task_struct, sibling);

            //Inicio del proceso
            //Con este if compruebo si el proceso tiene el scruct mm que sera para obtener la memoria del proceso
		    if(task_child->mm){
		        down_read(&task_child->mm->mmap_sem); 
		        size = (task_child->mm->mmap->vm_end - task_child->mm->mmap->vm_start );            
		        up_read(&task_child->mm->mmap_sem);
	        }else{
		        size=0;
	        
            }

            obtener_estado(task_child->state, cadena_estado);
            seq_printf(m, "\t\t\t\t{\"PID\":%d , \"Nombre\":\"%s\" , \"Usuario\":%d, \"Ram\":%llu , \"Cpu\":%d , \"Estado\":\"%s\"" ,
                   task_child->pid, task_child->comm, task_child->cred->uid, size*100/S(i.totalram), task_child->cpuset_mem_spread_rotor, cadena_estado);

            seq_printf(m, ",\"Hijos\" : [] }"); //fin del proceso hijo

            contador_hijos++; //Aumento el número de hijos
        }

        seq_printf(m, "\t\t\t\t]\n\t\t}"); //fin del arreglo de hijos del proceso y fin del proceso
    }
    
    seq_printf(m, "\n\t\t]\n"); //fin del arreglo de procesos

    seq_printf(m, "\t,\"Total\" : %d, \"Ejecucion\" : %d, \"Suspendidos\" : %d, \"Detenidos\" : %d, \"Zombies\" : %d \n"
                , contador_procesos, ejecucion, suspendidos, detenidos, zombies); //Pongo los otros atributos del json


    seq_printf(m, "}\n"); //fin del json

    return 0;
}

static int proc_al_abrir_archivo(struct inode *inode, struct  file *file) {
  return single_open(file, proc_llenar_archivo, NULL);
}

static struct file_operations myops =
{
        .owner = THIS_MODULE,
        .open = proc_al_abrir_archivo,
        .read = seq_read,
        .llseek = seq_lseek,
        .release = single_release,
};



/**
*	Defino que es lo que se va a hacer al cargar el modulo
*/
static int iniciar(void)
{
	proc_create(FileProc,0,NULL,&myops);
    printk(KERN_INFO "Carne: %s\n", Carne);

    /**
    * Si no se devuelve 0 significa que initmodule ha fallado y no ha podido cargarse.
    */
    return 0;
}

/**
*	Defino que es lo que se va a hacer al terminar el modulo
*/
static void terminar(void)
{
	remove_proc_entry(FileProc,NULL);
	printk(KERN_INFO "Curso: %s\n", Curso);
}


/*
 * Indicamos cuales son las funciones de inicio y fin
 */
module_init(iniciar);
module_exit(terminar);

/*
 * Documentacion del modulo
 */
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Nery Galvez - 201403525");
MODULE_DESCRIPTION("Modulo con informacion de los procesos");
MODULE_SUPPORTED_DEVICE("TODOS");
