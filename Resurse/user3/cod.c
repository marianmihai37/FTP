
const char* Encrypt_Password(char* Password) //Caesar Cypher Algorithm
{
  for(int i=0; (i<100 && Password[i] != '\0'); i++)
    Password[i]+=3;
  return Password;
}

const char* InterpretareComanda(int fd)
{
  char comanda[1024];		//comanda primita de la client 
  char msgrasp[buffer]=" "; //mesaj de raspuns pentru client

  bzero(comanda,1024);
  if ((read (fd, comanda, 1024) < 0))
  {
    perror ("Eroare la read() de la client.\n");
    return "exit";
  }      
  /*interpretam comanda si apelam functia necesara fiecarei comenzi */  
  if(Lista_Useri[fd].Downloading == 1)
  {
    DownloadFile(fd);
    strcpy(msgrasp, "[Server] Fisierul a fost incarcat!");
    printf ("[Descriptorul %d] Trimitem raspuns clientului: %s\n", fd, msgrasp);
    if (write (fd, msgrasp, strlen(msgrasp)+1) < 0)
    {
      perror ("[Server] Eroare la write() catre client.\n");
      return "exit";
    }
  }
  else
  {
    printf ("---------------------------------------------\n");
    printf ("[Descriptorul %d] Comanda receptionata: %s\n", fd, comanda);
    if(strcmp("help",comanda) == 0)
    {
      strcpy(msgrasp, Help());
      printf ("[Descriptorul %d] Trimitem raspuns clientului: %s\n", fd, msgrasp);
      if (write (fd, msgrasp, strlen(msgrasp)+1) < 0)
      {
        perror ("[Server] Eroare la write() catre client.\n");
        return "exit";
      }
    }
    else if(strcmp("login",comanda) == 0)
    {    
      LogIn(fd);
    }
    else if(strcmp("sendfile",comanda) == 0)
    {
      strcpy(msgrasp,SendFile(fd));
      printf ("[Descriptorul %d] Trimitem raspuns clientului: %s\n", fd, msgrasp);    
      if (write (fd, msgrasp, strlen(msgrasp)+1) < 0)
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
        strcpy(Lista_Useri[fd].Username, user);
        LogIn_Password(fd);
      }
      else 
      {
        char* pw = strstr(comanda,"[password]:");
        if(pw != NULL) //a fost introdusa parola
        {
          pw = strstr(pw,":");
          strcpy(pw,pw+1);
          strcpy(Lista_Useri[fd].Password, Decriptare_Parola(pw));
          int cont_blocat = ContBlocat(fd, Lista_Useri[fd].Username);
          if(cont_blocat == 0)
          {
            int cont_valid = ContValid(fd,Lista_Useri[fd].Username,Lista_Useri[fd].Password);
            if(cont_valid == 1) //cont valid
            {
              Lista_Useri[fd].Conectat = 1;
              strcpy(msgrasp, "Conectat!\n");
              printf ("[Descriptorul %d] Trimitem raspuns clientului: %s\n", fd, msgrasp);
              if (write (fd, msgrasp, strlen(msgrasp)+1) < 0)
              {
                perror ("[Server] Eroare la write() catre client.\n");
                return "exit";
              }
            }
            else if(cont_valid == 0) // parola incorecta
            {
              strcpy(msgrasp, "Parola incorecta!\n");
              printf ("[Descriptorul %d] Trimitem raspuns clientului: %s\n", fd, msgrasp);
              if (write (fd, msgrasp, strlen(msgrasp)+1) < 0)
              {
                perror ("[Server] Eroare la write() catre client.\n");
                return "exit";
              }
            }     
            else //cont invalid
            {            
              strcpy(msgrasp, "Acest cont nu exista!\n");
              printf ("[Descriptorul %d] Trimitem raspuns clientului: %s\n", fd, msgrasp);
              if (write (fd, msgrasp, strlen(msgrasp)+1) < 0)
              {
                perror ("[Server] Eroare la write() catre client.\n");
                return "exit";
              }
            }
          }
          else //cont blocat
          {
            Lista_Useri[fd].Conectat = 0;
            strcpy(msgrasp, "Cont blocat!\n");
            printf ("[Descriptorul %d] Trimitem raspuns clientului: %s\n", fd, msgrasp);
            if (write (fd, msgrasp, strlen(msgrasp)+1) < 0)
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
            strcpy(Lista_Useri[fd].Path, sendf);
            if(access(Lista_Useri[fd].Path, F_OK) != -1)
            {
              Lista_Useri[fd].Downloading = 1;
              strcpy(msgrasp, "[Info File]: Fisierul poate fi primit!");
            }
            else
            {
              Lista_Useri[fd].Downloading = 0;
              strcpy(msgrasp, "[Info File]: Fisierul nu exista!");
            }
                      
            printf ("[Descriptorul %d] Trimitem raspuns clientului: %s\n", fd, msgrasp);
            if (write (fd, msgrasp, strlen(msgrasp)+1) < 0)
            {
              perror ("[Server] Eroare la write() catre client.\n");
              return "exit";
            }
          }
          else
          {          
            //printf(comanda); //[SendFile]          
            strcpy(msgrasp, "Comanda invalida! Tastati help pentru comenzile disponibile!\n");
            printf ("[Descriptorul %d] Trimitem raspuns clientului: %s\n", fd, msgrasp);
            if (write (fd, msgrasp, strlen(msgrasp)+1) < 0)
            {
              perror ("[Server] Eroare la write() catre client.\n");
              return "exit";
            }           
          }        
        }        
      }   
    } 
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
    printf("[File Size]: %lu bytes\n", fsize);
    fclose(fs);

    FILE *fp = fopen(path,"rb");
    if(fp == NULL)
    {
        perror("File open error");
    }  
    while(1)
    {
        /* First read file in chunks of 1024 bytes */
        unsigned char buff[1024]={0};
        int nread = fread(buff,1,1024,fp);
        //printf("Bytes read %d \n", nread);        

        /* If read was success, send data. */
        if(nread > 0)
        {       
            if (write (sd, buff, nread) <= 0)
            {
              perror ("[client]Eroare la write() spre server.\n");
            }
        }
        if (nread < 1024)
        {
            if (feof(fp))
            {
                printf("[Info File]: Fisierul a fost trimis la server ! \n");
                fflush(stdout);
            }
            if (ferror(fp))
                perror("Error reading\n");
            break;
        }
    }
}

void* DownloadFile(int fd)
{
  char path[256];
  if(getcwd(path,sizeof(path)) == NULL)
    perror("[Server] getcwd() error.\n");
  else 
  {
    strcat(path,"/Resurse/");
    strcat(path,Lista_Useri[fd].Username);
    //daca directorul nu exista, il cream
    struct stat st = {0};
    if(stat(path, &st) == -1)
      mkdir(path, 0700);
    //
    strcat(path,"/");
    strcat(path,basename(Lista_Useri[fd].Path));
  }  
  char recvBuff[1024];
  memset(recvBuff, '0', sizeof(recvBuff));
  int bytesReceived = 0;
  printf("File Name: \033[0;32m %s \033[0m \n",basename(Lista_Useri[fd].Path));
	printf("Receiving file...\n");
  FILE *fp;
  fp = fopen(path, "ab"); 
  if(NULL == fp)
  {
      printf("Error opening file");
  }
  long double sz=1;
  bzero(recvBuff,1024);
  bytesReceived = read(fd, recvBuff, 1024);

  if(bytesReceived < 0)
  {
      printf("\n Read Error \n");
      fflush(stdout);
  }
  else if(bytesReceived < 1024)
  {
    printf("Bytes : %lu \n", bytesReceived);
    sz++;
    printf("Received: \033[0;32m %llf Mb \033[0m\n",(sz/1024));
    fwrite(recvBuff,1,sizeof(recvBuff),fp);
  }
  else
  {
    while(bytesReceived >= 1024)
    {
      //printf("Bytes : %lu \n", bytesReceived);
      sz++;
      printf("Received: \033[0;32m %llf Mb \033[0m\n",(sz/1024));          
      fwrite(recvBuff,1,sizeof(recvBuff),fp);
      fflush(stdout);
      bzero(recvBuff,1024);
      bytesReceived = read(fd, recvBuff, 1024);
    }
    if(bytesReceived > 0)
    {
      //printf("Bytes : %lu \n", bytesReceived);
      sz++;
      printf("Received: \033[0;32m %llf Mb \033[0m\n",(sz/1024));          
      fwrite(recvBuff,1,sizeof(recvBuff),fp);
      fflush(stdout);
    }
  }
  printf("\nFile OK....Completed\n");
  Lista_Useri[fd].Downloading = 0;
}