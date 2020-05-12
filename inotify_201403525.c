#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/inotify.h>
#include <limits.h>
#include <time.h>
 
#define MAX_EVENTS 1024 /*Max. number of events to process at one go*/
#define LEN_NAME 16 /*Assuming that the length of the filename won't exceed 16 bytes*/
#define EVENT_SIZE  ( sizeof (struct inotify_event) ) /*size of one event*/
#define BUF_LEN     ( MAX_EVENTS * ( EVENT_SIZE + LEN_NAME )) /*buffer to store the data of events*/
 
int main( int argc, char **argv ) 
{
  int length, i = 0, wd;
  int fd;
  char buffer[BUF_LEN];
 
  /* Initialize Inotify*/
  fd = inotify_init();
  if ( fd < 0 ) {
    perror( "No se pudo iniciar inotify");
  }
 
  /* add watch to starting directory */
  wd = inotify_add_watch(fd, "/", IN_CREATE | IN_MODIFY | IN_DELETE); 
 
  if (wd == -1)
    {
      printf("No se pudo agregar reloj %s\n","/");
    }
  else
    {
      printf("Watching:: %s\n","/");
    }
 
         FILE *fptr;

  /* do it forever*/
  while(1)
    {
      i = 0;
      length = read(fd, buffer, BUF_LEN );  
 
      if ( length < 0 ) {
        perror( "read" );
      }  
 
      while ( i < length ) {


                fptr = fopen("/home/ing-usac-201403525/Sopes2/Sopes2_Proyecto/201403525.log","a");

        

                if(fptr == NULL)
                {
                    printf("No se pudo cargar el archivo para el log!");   
                    return -1;           
                }
                   
        struct inotify_event *event = ( struct inotify_event * ) &buffer[ i ];
        if ( event->len ) {

          time_t rawtime;
          struct tm *info;
          time( &rawtime );
          info = localtime( &rawtime );
          //printf("Current local time and date: %s", asctime(info));
          
          if ( event->mask & IN_CREATE) {
            if (event->mask & IN_ISDIR)
              {
                printf( "La carpeta %s fue creada. --- %s\n", event->name, asctime(info) );     
               fprintf(fptr,"La carpeta %s fue creada. --- %s\n", event->name, asctime(info));  
              }
            else
              {
                printf( "El archivo %s fue creado con WD %d --- %s\n", event->name, event->wd, asctime(info));  
                fprintf(fptr,"El archivo %s fue creado con WD %d --- %s\n", event->name, event->wd, asctime(info));   
              }   
          }
           
          if ( event->mask & IN_MODIFY) {
            if (event->mask & IN_ISDIR){
              printf( "La carpeta %s fue modificada. --- %s\n", event->name , asctime(info));    
              fprintf(fptr,"La carpeta %s fue modificada. --- %s\n", event->name , asctime(info));     
            }
            else{
              printf( "El archivo %s fue modificado. %d --- %s\n", event->name, event->wd, asctime(info));   
              fprintf(fptr,"El archivo %s fue modificado.%d --- %s\n", event->name, event->wd, asctime(info));  
            }   
          }
           
          if ( event->mask & IN_DELETE) {
            if (event->mask & IN_ISDIR){
              printf( "La carpeta %s fue eliminada.\n --- %s", event->name , asctime(info));   
              fprintf(fptr,"La carpeta %s fue eliminada.\n --- %s", event->name, asctime(info)); 
            }    
            else{
              printf( "El archivo %s fue eliminado.  %d\n --- %s", event->name, event->wd, asctime(info));     
              fprintf(fptr,"El archivo %s fue eliminado.  %d\n --- %s", event->name, event->wd, asctime(info));  
            }
          }  
 
 
          i += EVENT_SIZE + event->len;
          
        }
        fclose(fptr);
      }
    }
 
  /* Clean up*/
  inotify_rm_watch( fd, wd );
  close( fd );
   
  return 0;
}
