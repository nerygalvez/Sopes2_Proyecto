
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <linux/inotify.h>
#include <time.h>
#include <string.h>

#define EVENT_SIZE ( sizeof (struct inotify_event) )
#define EVENT_BUF_LEN ( 1024 * (EVENT_SIZE) + 16)
#define RED "\e[31m"
#define YELLOW "\e[33m"
#define GREEN "\e[32m"
#define ENDC "\e[00m"
FILE * get_log();

int main() {
  FILE *log = get_log();
  int length, i;
  int fd;
  int wd;
  char buffer[EVENT_BUF_LEN];
  char h_evento [128] = {0};
  
  /*instancia inotify*/
  fd = inotify_init();
  
  /*checking for error*/
  if ( fd < 0) {
    perror ("inotify_init");
  }
    
    /*agregando directorio*/
  wd = inotify_add_watch(fd, "/home/wxjoy/Desarrollo/SO2/test", IN_DELETE | IN_CREATE | IN_MODIFY);
    
  while(1){
    /*determina que pasa*/
    
    length = read( fd, buffer, EVENT_BUF_LEN);
    i=0;
    
    if( length < 0){
      perror ("read");
    }
    
    /*leyendo la lista de eventos*/
    time_t t = time(0);
    struct tm *tlocal = localtime (&t);
    while( i < length){
      struct inotify_event *event = ( struct inotify_event *)&buffer[i];
      if( event->len){
        
        strftime (h_evento, 128, "%d/%m/%Y %H:%M:%S", tlocal);
        if( event->mask & IN_DELETE){
          printf("%s", RED);
          if( event->mask & IN_ISDIR){
            printf ("%s, se ha borrado el directorio: %s\n", h_evento, event->name);
            fprintf (log, "%s, Se ha borrado el directorio: %s\n", h_evento, event->name);
          }else{
            printf ("%s, se ha borrado el archivo: %s\n", h_evento, event->name);
            fprintf (log, "%s, se ha borrado el archivo: %s\n", h_evento, event->name);
          }
          printf("%s", ENDC);
        }else if(event->mask & IN_CREATE){
          printf("%s", GREEN);
          if( event->mask & IN_ISDIR){
            printf ("%s, se ha creado el directorio: %s\n",h_evento, event->name);
            fprintf (log, "%s, se ha creado el directorio: %s\n", h_evento, event->name);
          }else{
            printf ("%s, se ha creado el archivo: %s\n", h_evento, event->name);
            fprintf (log, "%s, se ha creado el archivo: %s\n", h_evento, event->name);
          }
          printf("%s", ENDC);
        }else {
          printf("%s", YELLOW);
          if(event->mask & IN_ISDIR){
            printf ("%s, se ha modificado el directorio: %s\n", h_evento, event->name);
            fprintf (log, "%s, Se ha modificado el directorio: %s\n", h_evento, event->name);
          } else{
            printf ("%s, se ha modificado el archivo: %s\n", h_evento, event->name);
            fprintf (log, "%s, se ha borrado modificado el archivo: %s\n", h_evento, event->name);
          }
          printf("%s", ENDC);
        }
      }
      i += EVENT_SIZE + event->len;
      fflush (log);
    }
    
  }           
  
  inotify_rm_watch(fd, wd);
  close(fd);
  
}

FILE * get_log(){
  time_t t = time(0);
  struct tm *tl = localtime (&t);
  char output[128];
  strftime(output,128,"%d-%m-%Y %H:%M:%S", tl);
  char nom[134]={0};
  strcat (nom, output);
  strcat (nom, ".log");
  printf("%s\n", nom);
  return fopen (nom, "a+");
}