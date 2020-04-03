#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <dirent.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <time.h>
#include <errno.h>
#include <limits.h>

int log_in = 0;

int ExistaUtilizator(char* utilizator)
{
	FILE *fis = fopen("utilizatori.txt","r");
	if(fis == NULL)
	{
		printf("Fisierul utilizatori.txt nu se poate deschide ! \n");
	} 
	else
	{
		char s[100];
		int ok=0;
		while(fscanf(fis, "%s", s) == 1 && ok == 0)
		{
			if(strcmp(s,utilizator) == 0)
			{
				fclose(fis);
				return 1;
			}
		}
	}
	fclose(fis);
	return 0;
}

void CautaFisier(char* path, const char* fisier, int socket)
{
	DIR *dir;
	dir = opendir(path);
	if(dir == 0) 
	{
		return;
	}
	while(1<2) 
	{
        	struct dirent *intrare;
        	const char *d_name;

        	intrare = readdir(dir);
        	if(intrare == 0)
		{
			break;
        	}
        	d_name = intrare->d_name;

		if((intrare->d_type & DT_DIR) == 0)
		{
			if(strcmp(d_name,fisier) == 0)
			{
				char rez[256];
				snprintf(rez,256,"%s/%s", path, d_name);
				write(socket,rez,256);
			}
		}
		else
		{
			if(strcmp(d_name, "..") != 0 && strcmp(d_name, ".") != 0) 
			{
				int lungime_path;
				char path2[PATH_MAX];

				lungime_path = snprintf(path2, PATH_MAX, "%s/%s", path, d_name);
				if (lungime_path >= PATH_MAX) {
			    		printf("Dimensiunea caii este prea mare. \n");
			    		return;
				}
				CautaFisier(path2,fisier, socket);
			}
		}
	}
	
	if (closedir(dir))
	{
		printf("Nu se poate inchide directorul %s. \n",path);
		return;
	}
}

char* Permisiuni(char *path)
{
	struct stat status;
	char* evaluare = malloc(1 + sizeof(char)*9);
	if(stat(path,&status) == -1)
	{
		printf("Eroare la stat. \n");
		return NULL;
	}
	else
	{
		mode_t mode = status.st_mode;
		evaluare[0] = (S_IRUSR & mode) ? 'r' : '-';
		evaluare[1] = (S_IWUSR & mode) ? 'w' : '-';
		evaluare[2] = (S_IXUSR & mode) ? 'x' : '-';
		evaluare[3] = (S_IRGRP & mode) ? 'r' : '-';
		evaluare[4] = (S_IWGRP & mode) ? 'w' : '-';
		evaluare[5] = (S_IXGRP & mode) ? 'x' : '-';
		evaluare[6] = (S_IROTH & mode) ? 'r' : '-';
		evaluare[7] = (S_IWOTH & mode) ? 'w' : '-';
		evaluare[8] = (S_IXOTH & mode) ? 'x' : '-';
		evaluare[9] = '\0';
		return evaluare;
	}
}

int main()
{
	int fd_login[2],fd_myfind[2];
	pid_t p_login, p_myfind;
	if(pipe(fd_login) == -1)
	{
		printf("Eroare la pipe() ! \n");
		exit(1);
	}
	if(pipe(fd_myfind) == -1)
	{
		printf("Eroare la pipe() ! \n");
		exit(1);
	}
	
	if((p_login=fork()) < 0)
	{
		printf("Eroare la fork() ! \n");
		exit(2);
	}
	if(p_login == 0)
	{
		char comanda[50];
		read(fd_login[0],comanda,50);
		char* cmd ;
		if((cmd = strstr(comanda,"login")) != NULL)
		{
			char* utilizator=strstr(comanda,":");
			if(utilizator == NULL)
			{
				write(fd_login[1],"failure",7);
			}
			else
			{
				memmove(utilizator,utilizator+1,strlen(utilizator));
				if(ExistaUtilizator(utilizator) == 1)
				{
					write(fd_login[1],"success",8);
				}
				else
				{
					write(fd_login[1],"failure",8);
				}
			}
		}
		else
		{
			write(fd_login[1],"failure",7);
		}
	}
	else
	{
		char comanda[50];
		char* cmd ;
		printf("*Trebuie sa va conectati (login:USERNAME): ");
		
		fgets(comanda,50,stdin);
		comanda[strlen(comanda)-1]=0;
		while(strcmp(comanda,"quit") != 0)
		{
			if((cmd = strstr(comanda,"login")) != NULL)
			{		
				if(log_in == 1)
					printf("Sunteti deja conectat(a) ! \n");
				else
				{			
					write(fd_login[1],cmd,strlen(cmd)+1);
					wait(NULL);
					char rezultat_fiu[8];
					read(fd_login[0],rezultat_fiu,8);
					if(strcmp(rezultat_fiu,"success") == 0)
					{
						log_in = 1;
						printf("V-ati conectat cu succes ! \n");
					}
					else
					{
						printf("Utilizator invalid. Exit...  \n");
						return 0;
					}
				}
				if(log_in == 1)
					printf("*Dati o comanda: ");
				else
					printf("*Trebuie sa va conectati (login:USERNAME): ");

				fgets(comanda,50,stdin);
				comanda[strlen(comanda)-1]=0;
			}
			else if((cmd = strstr(comanda,"myfind")) != NULL)
			{
				if(log_in == 0)
				{
					printf("Trebuie sa fiti conectat pentru a putea utiliza aceasta comanda ! \n");	
					if(log_in == 1)
						printf("*Dati o comanda: ");
					else
						printf("*Trebuie sa va conectati (login:USERNAME): ");

					fgets(comanda,50,stdin);
					comanda[strlen(comanda)-1]=0;
				}
				else
				{
					socketpair(PF_LOCAL, SOCK_STREAM, 0, fd_myfind);

					p_myfind = fork();
					if(p_myfind == 0)
					{
						char* fis = strstr(cmd," ");
						memmove(fis,fis+1,strlen(fis));
						close(fd_myfind[1]);

						char current_dir[100];
						getcwd(current_dir,sizeof(current_dir));
						CautaFisier(current_dir, fis, fd_myfind[0]);

						printf("Fisierul nu exista. Sesiune incheiata !\n");
						kill(p_myfind,SIGKILL);
					}
					else
					{
						close(fd_myfind[0]);
						char buff[1024];
						int n = read(fd_myfind[1],buff,sizeof(buff));
						printf("--------------------------\n");
						printf("Fisierul se gaseste la calea: '%.*s'\n",n,buff);

						char path[256];
						snprintf(path,256,"%.*s",n,buff);

						struct stat status;
						if(stat(path,&status) == -1)
							printf("Eroare la stat. \n");
						else
						{
							printf("iNode: %ld\n", (long)status.st_ino);
							printf("Dimensiune fisier: %lld bytes \n",(long long)status.st_size);
							printf("Ultima modificare: %s",ctime(&status.st_mtime));
							printf("Ultima accesare: %s", ctime(&status.st_atime));
							char *perm = Permisiuni(path);
							printf("Permisiuni: %s \n",perm);
							printf("--------------------------\n");
							kill(p_myfind,SIGKILL);
						}
					}
				}
				if(log_in == 1)
					printf("*Dati o comanda: ");
				else
					printf("*Trebuie sa va conectati (login:USERNAME): ");

				fgets(comanda,50,stdin);
				comanda[strlen(comanda)-1]=0;
			}
			else if((cmd = strstr(comanda,"mystat")) != NULL)
			{
				if(log_in == 0)
					printf("Trebuie sa fiti conectat pentru a putea utiliza aceasta comanda ! \n");
				else
				{
					printf("Functia mystat nu este implementata! \n");
				}
				if(log_in == 1)
					printf("*Dati o comanda: ");
				else
					printf("*Trebuie sa va conectati (login:USERNAME): ");

				fgets(comanda,50,stdin);
				comanda[strlen(comanda)-1]=0;	
			}
			else
			{
				printf("Introduceti o comanda valida! Pentru a inchide protocolul introduceti quit ! \n");
				if(log_in == 1)
					printf("*Dati o comanda: ");
				else
					printf("*Trebuie sa va conectati (login:USERNAME): ");

				fgets(comanda,50,stdin);
				comanda[strlen(comanda)-1]=0;
			}
			
		}
		printf("Sesiune incheiata ! \n");
		return 0;
	}
	return 0;
}
