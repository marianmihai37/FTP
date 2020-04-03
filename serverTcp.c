#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>
#include <libgen.h>
#include <sys/stat.h>
#include <dirent.h>
#include <pthread.h>

#define _GNU_SOURCE
#define PORT 2728
#define buffer 1000
#define MAX_SIZE_FILE 11000000 //11mb

extern int errno;	

typedef struct thData{
	int idThread; 
	int cl; 
}thData;


struct User{
  int Conectat;
  char* Username;
  char* Password;
  int Downloading;
  char* Path;
}Lista_Useri[100];

static void *treat(void *); /* functia executata de fiecare thread ce realizeaza comunicarea cu clientii */

const char* InterpretareComanda(void *arg);
int main ()
{
  //initialise list
  for(int i=0;i<100;i++)
  {
    Lista_Useri[i].Username = (char*) malloc (30);
    Lista_Useri[i].Password = (char*) malloc (30);
    Lista_Useri[i].Conectat = 0;
    Lista_Useri[i].Downloading = 0;
    Lista_Useri[i].Path = (char*) malloc (100);
  }

  struct sockaddr_in server;	// structura folosita de server
  struct sockaddr_in from;	
  int nr;		//mesajul primit de trimis la client 
  int sd;		//descriptorul de socket 
  int pid;
  pthread_t th[100];    //Identificatorii thread-urilor care se vor crea
	int i=0;
  

  /* crearea unui socket */
  if ((sd = socket (AF_INET, SOCK_STREAM, 0)) == -1)
    {
      perror ("[server]Eroare la socket().\n");
      return errno;
    }
  /* utilizarea optiunii SO_REUSEADDR */
  int on=1;
  setsockopt(sd,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on));
  
  /* pregatirea structurilor de date */
  bzero (&server, sizeof (server));
  bzero (&from, sizeof (from));
  
  /* umplem structura folosita de server */
  /* stabilirea familiei de socket-uri */
    server.sin_family = AF_INET;	
  /* acceptam orice adresa */
    server.sin_addr.s_addr = htonl (INADDR_ANY);
  /* utilizam un port utilizator */
    server.sin_port = htons (PORT);
  
  /* atasam socketul */
  if (bind (sd, (struct sockaddr *) &server, sizeof (struct sockaddr)) == -1)
    {
      perror ("[server]Eroare la bind().\n");
      return errno;
    }

  /* punem serverul sa asculte daca vin clienti sa se conecteze */
  if (listen (sd, 2) == -1)
    {
      perror ("[server]Eroare la listen().\n");
      return errno;
    }
  /* servim in mod concurent clientii...folosind thread-uri */
  printf ("[server]Asteptam la portul %d...\n",PORT);
  fflush (stdout);

  while (1)
  {
    int client;
    thData * td; //parametru functia executata de thread     
    int length = sizeof (from);
    
    // client= malloc(sizeof(int));
    /* acceptam un client (stare blocanta pina la realizarea conexiunii) */
    if ( (client = accept (sd, (struct sockaddr *) &from, &length)) < 0)
    {
      perror ("[server]Eroare la accept().\n");
      continue;
    }
	
        /* s-a realizat conexiunea, se astepta mesajul */
    
    // int idThread; //id-ul threadului
    // int cl; //descriptorul intors de accept

    td=(struct thData*)malloc(sizeof(struct thData));	
    td->idThread=i++;
    td->cl=client;

    pthread_create(&th[i], NULL, &treat, td);	
  }				/* while */
}				/* main */

static void *treat(void * arg)
{		
		struct thData tdL; 
		tdL = *((struct thData*)arg);	
		printf ("[thread]- %d - Asteptam comanda de la client\n", tdL.idThread);
		fflush (stdout);		 
		pthread_detach(pthread_self());	
    	
    do{
      const char* rasp = InterpretareComanda((struct thData*)arg);
      if(strcmp(rasp,"exit") == 0)
      {
        Lista_Useri[tdL.idThread].Conectat = 0;
        free(Lista_Useri[tdL.idThread].Username);
        free(Lista_Useri[tdL.idThread].Password);
        free(Lista_Useri[tdL.idThread].Path);
        printf ("[Server] S-a deconectat clientul cu thread-ul %d.\n",tdL.idThread);
        printf ("---------------------------------------------\n");
        fflush (stdout);
        close ((intptr_t)arg);
        return(NULL);	
      }
    }while(1<2);
};

char* Decriptare_Parola(char* Password)
{
  for(int i=0; (i<100 && Password[i] != '\0'); i++)
    Password[i]-=3;
  return Password;
}
void LogIn(void *arg)
{
  struct thData tdL; 
  tdL = *((struct thData*)arg);	
  if(Lista_Useri[tdL.idThread].Conectat == 1)
  {
    char msgrasp[100]=" ";
    bzero(msgrasp,100);
    strcpy(msgrasp,"Sunteti deja conectat!\n");
    printf ("[Thread %d] Trimitem raspuns clientului: %s\n", tdL.idThread, msgrasp);    
    if (write (tdL.cl, msgrasp, strlen(msgrasp)+1) < 0)
    {
      perror ("[Server] Eroare la write() catre client.\n");
    }
  }
  else
  {
    char msgrasp[100]=" ";
    bzero(msgrasp,100);
    strcpy(msgrasp, "\033[0;32m[LogIn] username: \033[0m");
    printf ("[Thread %d] Trimitem raspuns clientului: %s\n", tdL.idThread, msgrasp);
    if (write (tdL.cl, msgrasp, strlen(msgrasp)+1) < 0)
    {
      perror ("[Server] Eroare la write() catre client.\n");
    }
  }  
}
void LogIn_Password(void *arg)
{
  struct thData tdL; 
  tdL = *((struct thData*)arg);	
  char msgrasp[100]=" ";
  bzero(msgrasp,100);
  strcpy(msgrasp, "\033[0;32m[LogIn] password: \033[0m");
  printf ("[Thread %d] Trimitem raspuns clientului: %s\n", tdL.idThread, msgrasp);
  if (write (tdL.cl, msgrasp, strlen(msgrasp)+1) < 0)
  {
    perror ("[Server] Eroare la write() catre client.\n");
  }
}
int ContBlocat(void *arg,char* user)
{
  struct thData tdL; 
  tdL = *((struct thData*)arg);	
  char path[256];
  if(getcwd(path,sizeof(path)) == NULL)
    perror("[Server] getcwd() error.\n");
  else 
  {
    strcat(path,"/Documents/blacklist.txt");
    FILE *fis = fopen(path,"r");
    if(fis == NULL)
    {
      printf("Fisierul /Documents/blacklist.txt nu se poate deschide ! \n");
    } 
    else
    {
      char s[100];
      while(fscanf(fis, "%s", s) == 1)
      {  
        if(strcmp(s,user) == 0)
        {
          fclose(fis);
          return 1;
        }
      }
    }
    fclose(fis);
  }    
  return 0;
}
int ContValid(void *arg,char* Username, char* Password)
{
  struct thData tdL; 
  tdL = *((struct thData*)arg);	

  char path[256];
  if(getcwd(path,sizeof(path)) == NULL)
    perror("[Server] getcwd() error.\n");
  else 
  {
    strcat(path,"/Documents/whitelist.txt");
    FILE *fis = fopen(path,"r");
    if(fis == NULL)
    {
      printf("Fisierul /Documents/whitelist.txt nu se poate deschide ! \n");
    } 
    else
    {
      char s[100];
      char* user;
      char* pw;
      
      while(fscanf(fis, "%s", s) == 1)
      {  
        user=strtok(s,":");
        pw=strtok(NULL,":");   
           
        if(strcmp(Username,user) == 0)
        {
          if(strcmp(Password,Decriptare_Parola(pw)) == 0)
          {
            fclose(fis);
            return 1;
          }
          else
          {
            fclose(fis);
            return 0;
          }
        }
      }
    }
    fclose(fis);
  }    
  return -1;
}

const char* Help()
{
  char* msg;
  msg = (char*) malloc(buffer);
  strcpy(msg,"\033[0;31mComenzi disponibile: \033[0m\n");
  strcat(msg,"\033[0;32m login \033[0m (pentru a te conecta)\n");
  strcat(msg,"\033[0;32m sendfile \033[0m(trebuie sa fii conectat)\n");
  strcat(msg,"\033[0;32m myfiles \033[0m(pentru a vedea fisierele incarcate)\n");
  strcat(msg,"\033[0;32m download NUME_FISIER \033[0m (e.g: download fisier.txt). Fisierele se vor descarca in Downloads \n");
  strcat(msg,"\033[0;32m quit \033[0m (pentru a iesi)\n");
  return msg;
}

const char* MyFiles(void *arg)
{
  struct thData tdL; 
  tdL = *((struct thData*)arg);	

  if(Lista_Useri[tdL.idThread].Conectat == 0)
    return "\033[0;33m Trebuie sa fii conectat pentru a putea folosi aceasta comanda!\033[0m";

  char path[256];
  if(getcwd(path,sizeof(path)) == NULL)
    perror("[Server] getcwd() error.\n");
  else 
  {
    strcat(path,"/Resurse/");
    strcat(path,Lista_Useri[tdL.idThread].Username);
  }

  char* msg;
  msg = (char*) malloc(buffer);
  strcpy(msg,"\033[0;31mFisiere disponibile: \033[0m\n");
  DIR *dir;
  struct dirent *x;
  if((dir=opendir(path)) != NULL)
  {
    while((x = readdir(dir)) != NULL)
    {
      if(x->d_type == DT_REG)
      {
        strcat(msg,"\033[0;32m");
        strcat(msg,x->d_name);
        strcat(msg,"\033[0m");
        strcat(msg," - ");

        char path2[256];
        strcpy(path2,path);
        strcat(path2,"/");
        strcat(path2,x->d_name);
        unsigned long fsize=0;
        FILE *fs = fopen(path2,"r");
        if(fs == NULL)
        {
            perror("File open error");            
        }  
        else
        {
          fseek(fs, 0L, 2);
          fsize = ftell(fs);
          fclose(fs);
          char sizeOfFile[19];    
          sprintf(sizeOfFile,"%.1f", (float)fsize / 1024);   
          strcat(msg,sizeOfFile);
          strcat(msg, " Kbytes");
        }        
        strcat(msg,"\n");
      }
    }
    closedir(dir);
  }
  return msg;
}

const char* ReturnFileToClient(void *arg, char* fileName)
{
  struct thData tdL; 
  tdL = *((struct thData*)arg);	

  if(Lista_Useri[tdL.idThread].Conectat == 0)
    return "\033[0;33m Trebuie sa fii conectat pentru a putea folosi aceasta comanda!\033[0m";

  char path[256];
  if(getcwd(path,sizeof(path)) == NULL)
    perror("[Server] getcwd() error.\n");
  else 
  {
    strcat(path,"/Resurse/");
    strcat(path,Lista_Useri[tdL.idThread].Username);
  }

  char* msg;
  msg = (char*) malloc(buffer);
  DIR *dir;
  struct dirent *x;
  if((dir=opendir(path)) != NULL)
  {
    int exista=0;
    while((x = readdir(dir)) != NULL)
    {
      if(x->d_type == DT_REG)
      {
        if(strcmp(x->d_name, fileName) == 0)
        {
          exista=1;
          break;
        }
      }
    }
    if(exista == 1)
    {
      char path2[256];
      strcpy(path2,path);
      strcat(path2,"/");
      strcat(path2,fileName);
      unsigned long fsize=0;
      FILE *fs = fopen(path2,"r");
      if(fs == NULL)
      {
          perror("File open error");
      }  
      else
      {
        fseek(fs, 0L, 2);
        fsize = ftell(fs);
        fclose(fs);
        char sizeOfFile[19];    
        sprintf(sizeOfFile,"%ld", fsize); 
        strcpy(msg,"[Download File] \n");
        strcat(msg, fileName);
        strcat(msg," ");
        strcat(msg, sizeOfFile);
      }              
    }
    else
    {
      strcpy(msg, "[Download] Acest fisier nu exista!");
    }
    closedir(dir);
  }
  else
  {
    strcpy(msg, "[Download] Nu ai niciun fisier incarcat!");
  }
  return msg;
}

const char* SendFile(void *arg)
{
  struct thData tdL; 
  tdL = *((struct thData*)arg);	

  if(Lista_Useri[tdL.idThread].Conectat == 0)
    return "\033[0;33m Trebuie sa fii conectat pentru a putea folosi aceasta comanda!\033[0m";
  
  return "[Send file] Introdu calea absoluta catre fisier. \n[Send file] E.g : \033[0;33m/root/Desktop/Folder1/fisier.txt\033[0m";  
}

void* DownloadFile(void *arg)
{
  struct thData tdL; 
  tdL = *((struct thData*)arg);	

  char path[256];
  if(getcwd(path,sizeof(path)) == NULL)
    perror("[Server] getcwd() error.\n");
  else 
  {
    strcat(path,"/Resurse/");
    strcat(path,Lista_Useri[tdL.idThread].Username);
    //daca directorul nu exista, il cream
    struct stat st = {0};
    if(stat(path, &st) == -1)
      mkdir(path, 0700);
    //
    strcat(path,"/");
    strcat(path,basename(Lista_Useri[tdL.idThread].Path));
  }  
  char* recvBuff;
  recvBuff = (char*) malloc(MAX_SIZE_FILE);
  long bytesReceived = read(tdL.cl, recvBuff, 20);
  char* aux;
  long fileSize = strtol(recvBuff,&aux,10);
  
  if(strstr(recvBuff,"[Info File]:Error") != NULL)
  {
    printf("[Thread %d] Fisierul nu poate fi primit!\n", tdL.idThread);
  }
  else if(fileSize != 0)
  {
    printf("[Thread %d] File Name: \033[0;32m %s \033[0m \n",tdL.idThread, basename(Lista_Useri[tdL.idThread].Path));
    printf("[Thread %d] File Size: \033[0;32m %ld bytes \033[0m \n",tdL.idThread, fileSize);
    printf("[Thread %d] Receiving file...\n", tdL.idThread);
    
    FILE *fp;
    fp = fopen(path, "w"); 
    if(NULL == fp)
    {
        printf("Error opening file");
    }
    else
    {
      bzero(recvBuff,MAX_SIZE_FILE);
      //recvBuff = (char*) malloc(MAX_SIZE_FILE);
      bytesReceived = read(tdL.cl, recvBuff, fileSize);

      if(bytesReceived < 0)
      {
          printf("\n Read Error \n");
          fflush(stdout);
      }
      else 
      {
        printf("[Thread %d] Completed: \033[0;32m %.3f kb \033[0m\n",tdL.idThread, (float)(bytesReceived)/1024);   
        fwrite(recvBuff,1,bytesReceived,fp);   
        free(recvBuff);
        fflush(stdout);
      }   
      fclose(fp); 
    }    
  }  
  Lista_Useri[tdL.idThread].Downloading = 0;  
}
void* SendFileToClient(void *arg, char* fileName)
{
  struct thData tdL; 
  tdL = *((struct thData*)arg);	

  char path[256];
  if(getcwd(path,sizeof(path)) == NULL)
    perror("[Server] getcwd() error.\n");
  else 
  {
    strcat(path,"/Resurse/");
    strcat(path,Lista_Useri[tdL.idThread].Username);
    strcat(path,"/");
    strcat(path,fileName);
  }
    unsigned long fsize=0;
    FILE *fs = fopen(path,"r");
    if(fs == NULL)
    {
        perror("File open error");
    }  
    fseek(fs, 0L, 2);
    fsize = ftell(fs);
    fclose(fs);
    
    FILE *fp = fopen(path,"rb");
    if(fp == NULL)
    {
        perror("File open error");
    } 
    else
    {
      char* file;
      file = (char*) malloc(fsize);
      
      long nread = fread(file,1,fsize,fp);
      
      if(nread > 0)
      {       
        if (write (tdL.cl, file, fsize) <= 0)
        {
          perror ("[Server]Eroare la write() spre client.\n");
        }
        printf("[Thread %d] [Download File]: Fisierul a fost trimis la client (%ld bytes) ! \n", tdL.idThread, nread);
        
        fflush(stdout);
      }
      else
      {
        printf("[Thread %d] [Download File]: Fisierul NU a putut fi trimis la client ! \n", tdL.idThread);
        fflush(stdout);
      }
      fclose(fp); 
      free(file);
    }
}

const char* InterpretareComanda(void *arg)
{  
	struct thData tdL; 
	tdL = *((struct thData*)arg);

  char comanda[buffer]=" ";		//comanda primita de la client 
  char msgrasp[buffer]=" "; //mesaj de raspuns pentru client

  if(Lista_Useri[tdL.idThread].Downloading == 1) //Acum e momentul sa se trimita fisierul catre client
  {
    DownloadFile((struct thData*)arg);
    Lista_Useri[tdL.idThread].Downloading = 0;
    bzero(msgrasp,buffer);
    strcpy(msgrasp, "[Server] Comanda terminata!");
    printf ("[Thread %d] Trimitem raspuns clientului: %s\n", tdL.idThread, msgrasp);
    if (write (tdL.cl, msgrasp, strlen(msgrasp)+1) < 0)
    {
      perror ("[Server] Eroare la write() catre client.\n");
      return "exit";
    }
  }
  else
  {
    bzero(comanda,buffer);
    if ((read (tdL.cl, comanda, buffer) < 0))
    {
      perror ("Eroare la read() de la client.\n");
      return "exit";
    }      
    // interpretam comanda si apelam functia necesara fiecarei comenzi
    printf ("---------------------------------------------\n");
    printf ("[Thread %d] Comanda receptionata: %s\n", tdL.idThread, comanda);
    if(strcmp("help",comanda) == 0)
    {
      strcpy(msgrasp, Help());
      printf ("[Thread %d] Trimitem raspuns clientului: %s\n", tdL.idThread, msgrasp);
      if (write (tdL.cl, msgrasp, strlen(msgrasp)+1) < 0)
      {
        perror ("[Server] Eroare la write() catre client.\n");
        return "exit";
      }
    }
    else if(strcmp("myfiles",comanda) == 0)
    { 
      strcpy(msgrasp,"");
      strcat(msgrasp, MyFiles((struct thData*)arg));
      printf ("[Thread %d] Trimitem raspuns clientului: %s\n", tdL.idThread, MyFiles((struct thData*)arg));
      if (write (tdL.cl, MyFiles((struct thData*)arg), strlen(MyFiles((struct thData*)arg))+1) < 0)
      {
        perror ("[Server] Eroare la write() catre client.\n");
        return "exit";
      }
    }
    else if(strcmp("login",comanda) == 0)
    {    
      LogIn((struct thData*)arg);
    }
    else if(strcmp("sendfile",comanda) == 0)
    {
      strcpy(msgrasp,SendFile((struct thData*)arg));
      printf ("[Thread %d] Trimitem raspuns clientului: %s\n", tdL.idThread, msgrasp);    
      if (write (tdL.cl, msgrasp, strlen(msgrasp)+1) < 0)
      {
        perror ("[Server] Eroare la write() catre client.\n");
        return "exit";
      }
    }    
    else if(strcmp("quit",comanda) == 0)
    {
      return "exit";
    }
    else
    {
      char* user = strstr(comanda,"[username]:");
      if(user != NULL) //a fost introdus username-ul
      {
        user = strstr(user,":");
        strcpy(user,user+1);
        strcpy(Lista_Useri[tdL.idThread].Username, user);
        LogIn_Password((struct thData*)arg);
      }
      else 
      {
        char* pw = strstr(comanda,"[password]:");
        if(pw != NULL) //a fost introdusa parola
        {
          pw = strstr(pw,":");
          strcpy(pw,pw+1);
          strcpy(Lista_Useri[tdL.idThread].Password, Decriptare_Parola(pw));
          int cont_blocat = ContBlocat((struct thData*)arg, Lista_Useri[tdL.idThread].Username);
          if(cont_blocat == 0)
          {
            int cont_valid = ContValid((struct thData*)arg,Lista_Useri[tdL.idThread].Username,Lista_Useri[tdL.idThread].Password);
            if(cont_valid == 1) //cont valid
            {
              Lista_Useri[tdL.idThread].Conectat = 1;
              strcpy(msgrasp, "Conectat!\n");
              printf ("[Thread %d] Trimitem raspuns clientului: %s\n", tdL.idThread, msgrasp);
              if (write (tdL.cl, msgrasp, strlen(msgrasp)+1) < 0)
              {
                perror ("[Server] Eroare la write() catre client.\n");
                return "exit";
              }
            }
            else if(cont_valid == 0) // parola incorecta
            {
              strcpy(msgrasp, "Parola incorecta!\n");
              printf ("[Thread %d] Trimitem raspuns clientului: %s\n", tdL.idThread, msgrasp);
              if (write (tdL.cl, msgrasp, strlen(msgrasp)+1) < 0)
              {
                perror ("[Server] Eroare la write() catre client.\n");
                return "exit";
              }
            }     
            else //cont invalid
            {            
              strcpy(msgrasp, "Acest cont nu exista!\n");
              printf ("[Thread %d] Trimitem raspuns clientului: %s\n", tdL.idThread, msgrasp);
              if (write (tdL.cl, msgrasp, strlen(msgrasp)+1) < 0)
              {
                perror ("[Server] Eroare la write() catre client.\n");
                return "exit";
              }
            }
          }
          else //cont blocat
          {
            Lista_Useri[tdL.idThread].Conectat = 0;
            strcpy(msgrasp, "Cont blocat!\n");
            printf ("[Thread %d] Trimitem raspuns clientului: %s\n", tdL.idThread, msgrasp);
            if (write (tdL.cl, msgrasp, strlen(msgrasp)+1) < 0)
            {
              perror ("[Server] Eroare la write() catre client.\n");
              return "exit";
            }
          }
        }
        else
        {
          char* sendf = strstr(comanda,"[Send file] Cale: ");
          if(sendf != NULL)
          {
            sendf = strstr(sendf,":");
            strcpy(sendf,sendf+2); //elimin :_
            strcpy(Lista_Useri[tdL.idThread].Path, sendf);
            if(access(Lista_Useri[tdL.idThread].Path, F_OK) != -1)
            {
              Lista_Useri[tdL.idThread].Downloading = 1;
              strcpy(msgrasp, "[Info File]: \033[0;32mCalea fisierului este corecta!\033[0m");
            }
            else
            {
              Lista_Useri[tdL.idThread].Downloading = 0;
              strcpy(msgrasp, "[Info File]: \033[0;32mFisierul nu exista!\033[0m");
            }
                      
            printf ("[Thread %d] Trimitem raspuns clientului: %s\n", tdL.idThread, msgrasp);
            if (write (tdL.cl, msgrasp, strlen(msgrasp)+1) < 0)
            {
              perror ("[Server] Eroare la write() catre client.\n");
              return "exit";
            }
          }
          else 
          {
            char* download = strstr(comanda,"download");
            if(download != NULL)
            {
              char* file_name = strstr(comanda,"download");
              
              if(file_name != NULL) 
              {
                file_name = strstr(file_name," ");
                if(file_name != NULL)
                {
                  strcpy(file_name,file_name+1);
                  const char* ret = ReturnFileToClient((struct thData*)arg,file_name);
                  strcpy(msgrasp,ret);
                  free((char*)ret);
                } 
                else
                {
                  strcpy(msgrasp, "[Download] Acest fisier nu exista!");
                }
              }
              
              printf ("[Thread %d] Trimitem raspuns clientului: %s\n", tdL.idThread, msgrasp);    
              if (write (tdL.cl, msgrasp, strlen(msgrasp)+1) < 0)
              {
                perror ("[Server] Eroare la write() catre client.\n");
                return "exit";
              }
            }
            else
            {     
              char* sendToClient = strstr(comanda,"[Download] Trimite fisier:"); 
              if(sendToClient != NULL)
              {
                char* file_name = strstr(sendToClient,":");
                strcpy(file_name,file_name+1);
                SendFileToClient((struct thData*)arg, file_name);
              }
              else
              {
                strcpy(msgrasp, "\033[0;33m Comanda invalida! Tastati help pentru comenzile disponibile!\n\033[0m");
                printf ("[Thread %d] Trimitem raspuns clientului: %s\n", tdL.idThread, msgrasp);
                if (write (tdL.cl, msgrasp, strlen(msgrasp)+1) < 0)
                {
                  perror ("[Server] Eroare la write() catre client.\n");
                  return "exit";
                }  
              }  
            }       
          }           
        }        
      }   
    } 
  } 
  
  return "continue";
}


