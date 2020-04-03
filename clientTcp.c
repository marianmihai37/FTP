#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <arpa/inet.h>
#include <libgen.h>

#define buffer 1000
#define MAX_SIZE_FILE 11000000 //11mb

/* codul de eroare returnat de anumite apeluri */
extern int errno;
/* portul de conectare la server*/
int port;

void* SendFileToServer(int sd, char* path);
const char* Encrypt_Password(char* Password);
int main (int argc, char *argv[])
{  
  int sd;			// descriptorul de socket
  struct sockaddr_in server;	// structura folosita pentru conectare 
  char comanda[buffer];		// mesajul trimis
  char mesaj_primit[buffer]; // mesajul primit de la server
  char* path = (char*) malloc(100);
  /* exista toate argumentele in linia de comanda? */
  if (argc != 3)
  {
    printf ("[client] Sintaxa: %s <adresa_server> <port>\n", argv[0]);
    return -1;
  }

  /* stabilim portul */
  port = atoi (argv[2]);

  /* cream socketul */
  if ((sd = socket (AF_INET, SOCK_STREAM, 0)) == -1)
  {
    perror ("[client] Eroare la socket().\n");
    return errno;
  }

  /* umplem structura folosita pentru realizarea conexiunii cu serverul */
  /* familia socket-ului */
  server.sin_family = AF_INET;
  /* adresa IP a serverului */
  server.sin_addr.s_addr = inet_addr(argv[1]);
  /* portul de conectare */
  server.sin_port = htons (port);
  
  /* ne conectam la server */
  if (connect (sd, (struct sockaddr *) &server,sizeof (struct sockaddr)) == -1)
  {
    perror ("[client]Eroare la connect().\n");
    return errno;
  }

  char* size;
  char FilenameDownload[100];
  bzero(FilenameDownload,100);
  long fileSize=0;


  bzero (comanda, buffer);
  bzero (mesaj_primit, buffer);

  printf ("\033[0;32m Pentru a vizualiza toate comenzile disponibile tastati \033[0m\033[0;31mhelp \033[0m  \n");
  
  do
  {         
    if(strcmp(mesaj_primit, "\033[0;32m[LogIn] username: \033[0m") == 0)
    {
      fflush (stdout);    
      bzero (comanda, buffer);

      if (read (0, comanda, buffer) < 0)
      {
        perror ("[client]Eroare la read() de la server.\n");
        close (sd);
        return errno;
      }
      
      comanda[strlen(comanda)-1]=0;
      char tmp[100]="[username]:";
      strcat(tmp,comanda);
      strcpy(comanda,tmp);
      
      if (write (sd, comanda, strlen(comanda)+1) <= 0)
      {
        perror ("[client]Eroare la write() spre server.\n");
        close (sd);
        return errno;
      }
      bzero (mesaj_primit, buffer);
      if (read (sd, mesaj_primit, buffer) < 0)
      {
        perror ("[client]Eroare la read() de la server.\n");
        close (sd);
        return errno;
      }
      printf ("\n%s\n", mesaj_primit);
    }
    else if(strcmp(mesaj_primit, "\033[0;32m[LogIn] password: \033[0m") == 0)
    {
      fflush (stdout);
      bzero (comanda, buffer);

      char* pass = getpass("");
      strcpy(comanda, Encrypt_Password(pass));
      
      char tmp[100]="[password]:";
      strcat(tmp,comanda);
      strcpy(comanda,tmp);
      
      if (write (sd, comanda, strlen(comanda)+1) <= 0)
      {
        perror ("[client]Eroare la write() spre server.\n");
        return errno;
      }
      bzero (mesaj_primit, buffer);
      if (read (sd, mesaj_primit, buffer) < 0)
      {
        perror ("[client]Eroare la read() de la server.\n");
        return errno;
      }
      printf ("\n%s", mesaj_primit);
    }
    else if(strcmp(mesaj_primit, "[Send file] Introdu calea absoluta catre fisier. \n[Send file] E.g : \033[0;33m/root/Desktop/Folder1/fisier.txt\033[0m") == 0)
    {
      fflush (stdout);    
      bzero (comanda, buffer);

      if (read (0, comanda, buffer) < 0)
      {
        perror ("[client]Eroare la read() de la server.\n");
        close (sd);
        return errno;
      }
      comanda[strlen(comanda)-1]=0;
      //salvez calea fisierului in path
      strcpy(path,comanda);
      //
      char tmp[100]="[Send file] Cale: ";
      strcat(tmp,comanda);
      strcpy(comanda,tmp);
      
      if (write (sd, comanda, strlen(comanda)+1) <= 0)
      {
        perror ("[client]Eroare la write() spre server.\n");
        close (sd);
        return errno;
      }
      bzero (mesaj_primit, buffer);
      if (read (sd, mesaj_primit, buffer) < 0)
      {
        perror ("[client]Eroare la read() de la server.\n");
        close (sd);
        return errno;
      }
      /* afisam mesajul primit */
      printf ("\n%s\n", mesaj_primit);      
    }
    else if(strstr(mesaj_primit, "[Info File]:") != NULL)
    {              
      if(access(path, F_OK) != -1) //daca calea este corecta
      {
        //UPLOAD FILE
        SendFileToServer(sd, path);
        bzero (mesaj_primit, buffer);
        if (read (sd, mesaj_primit, buffer) < 0)
        {
          perror ("[client]Eroare la read() de la server.\n");
          close (sd);
          return errno;
        }
        
        printf ("\n%s\n", mesaj_primit); 
      }
      else 
      {
        printf ("*Introduceti o comanda: ");
        fflush (stdout);
        bzero (comanda, buffer);
        if (read (0, comanda, buffer) < 0)
        {
          perror ("[client]Eroare la read() de la server.\n");
          close (sd);
          return errno;
        }
        
        comanda[strlen(comanda)-1]=0; //stergem new line-ul de la sfarsitul string-ului
        
        if (write (sd, comanda, strlen(comanda)+1) <= 0)
        {
          perror ("[client]Eroare la write() spre server.\n");
          close (sd);
          return errno;
        }
        bzero (mesaj_primit, buffer);
        if (read (sd, mesaj_primit, buffer) < 0)
        {
          perror ("[client]Eroare la read() de la server.\n");
          close (sd);
          return errno;
        }
        
        printf ("\n%s\n", mesaj_primit);           
      }
    }   
    else if(strstr(mesaj_primit, "[Download File]") != NULL)
    {      
      int space=' ';
      char* size2 = strrchr(mesaj_primit, space);
      if(size2 && *(size2+1))
        size = size2+1;
      
      strcpy(FilenameDownload, mesaj_primit+17);
      for(int i=0; i<strlen(FilenameDownload); i++)
      {
        if(FilenameDownload[i] == ' ')
        {
          FilenameDownload[i] = '\0';
          break;
        }
      }
      
      char* aux;      
      fileSize = strtol(size,&aux,10);
      
      bzero(comanda,buffer);
      strcpy(comanda,"[Download] Trimite fisier:");
      strcat(comanda, FilenameDownload);
      if (write (sd, comanda, strlen(comanda)+1) <= 0)
      {
        perror ("[client]Eroare la write() spre server.\n");
        close (sd);
        return errno;
      }
      bzero(mesaj_primit,buffer);

      //DOWNLOAD FILE

      char path[256];
      strcpy(path,"/root/Downloads/");
      strcat(path, FilenameDownload);
      
      char* recvBuff;
      recvBuff = (char*) malloc(MAX_SIZE_FILE);

      if(fileSize != 0)
      {
        FILE *fp;
        fp = fopen(path, "w"); 
        if(NULL == fp)
        {
            printf("Error opening file");
        }

        bzero(recvBuff,MAX_SIZE_FILE);
        long bytesReceived = read(sd, recvBuff, fileSize);
        if(bytesReceived < 0)
        {
            printf("\n[Download File]Read Error \n");
            fflush(stdout);
        }
        else 
        { 
          printf("[Download File] Completed: \033[0;32m %.1f kb \033[0m\n", (float)(bytesReceived)/1024);  
          fwrite(recvBuff,1,bytesReceived,fp);
          free(recvBuff);
          fclose(fp);
        }
      }      
    }
    else //nimic de mai sus
    {
      printf ("*Introduceti o comanda: ");
      fflush (stdout);
      bzero (comanda, buffer);
      if (read (0, comanda, buffer) < 0)
      {
        perror ("[client]Eroare la read() de la server.\n");
        close (sd);
        return errno;
      }
      
      comanda[strlen(comanda)-1]=0; //stergem new line-ul de la sfarsitul string-ului
      // trimiterea comenzii la server
      if (write (sd, comanda, strlen(comanda)+1) <= 0)
      {
        perror ("[client]Eroare la write() spre server.\n");
        return errno;
      }   
      if(strcmp(comanda,"quit") != 0)
      {            
        bzero (mesaj_primit, buffer);
        if (read (sd, mesaj_primit, buffer) < 0)
        {
          perror ("[client]Eroare la read() de la server.\n");
          return errno;
        }
        
        if(strstr(mesaj_primit,"[Download File]") == NULL)
          printf ("\n%s\n", mesaj_primit);
      }   
    }        
  }while(strcmp(comanda,"quit") != 0);
  
  /* inchidem conexiunea, am terminat */
  close (sd);
}

void* SendFileToServer(int sd, char* path)
{
    unsigned long fsize=0;
    FILE *fs = fopen(path,"r");
    if(fs == NULL)
    {
        perror("File open error");
    }  
    fseek(fs, 0L, 2);
    fsize = ftell(fs);
    fclose(fs);
    
    FILE *fp = fopen(path,"r");
    if(fp == NULL)
    {
        perror("File open error");
    } 
    else
    {
      char* file;
      file = (char*) malloc(MAX_SIZE_FILE);
      char sizeOfFile[19];
      bzero(sizeOfFile, sizeof(sizeOfFile));
      sprintf(sizeOfFile,"%ld", fsize);    
      printf("[File Size]:\033[0;32m %ld bytes\033[0m\n", fsize);
      
      if(fsize > MAX_SIZE_FILE)
      {
        printf("[Info]\033[0;32m Fisierul este prea mare! (Dimensiune maxima: 100MB)\033[0m");
        write(sd,"[Info File]:Error",20);
        fflush(stdout);
      }
      else
      {        
        write(sd, sizeOfFile, 20);
        long nread = fread(file,1,fsize,fp);
        
        if(nread > 0)
        {       
          if (write (sd, file, fsize) <= 0)
          {
            perror ("[client]Eroare la write() spre server.\n");
          }
          printf("[Info File]:\033[0;32m Fisierul a fost trimis la server (%ld bytes) ! \033[0m\n", nread);
          
          fflush(stdout);
        }
        else
        {
          printf("[Info File]:\033[0;32m Fisierul NU a putut fi trimis la server !\033[0m \n");
          write(sd,"[Info File]:Error",20);
          fflush(stdout);
        }
      }
      free(file);
      fclose(fp);      
    }
}

const char* Encrypt_Password(char* Password) //Caesar Cypher Algorithm
{
  for(int i=0; (i<100 && Password[i] != '\0'); i++)
    Password[i]+=3;
  return Password;
}
