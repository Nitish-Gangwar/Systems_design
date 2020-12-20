#include<bits/stdc++.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include "KVClientLibrary.h"
#include "KVClientLibrary.cpp"
#define MAX 1024
#define MAXI 514
char config_param[MAX][MAX];
char listening_port[MAX]; //VARIABLE SPECIFIES THE PORT NUMBER OVER WHICH SERVER IS LISTENING
int clients_per_thread;//NUMBER OF CLIENTS THAT EACH WORKER THREAD HANDLES
int cache_size;		// DEFINES THE CACHE SIZE
char cache_replacement[MAX];//CACHE REPLACEMENT ALGORITHM BEIGN USED
int thread_pool_size_initial;	//INTIAL THREAD POOL SIZE
int thread_pool_growth;		//THREAD POOL INCREMENT ONCE THREADS ENDS

struct timespec start, finish;
double elapse;
int str_to_int(char *str)
{
	
	int i;
	int value=0;
	i=0;
	while(str[i]!='\0')
	{
		value=value*10+(str[i]-'0');
		i++;
	}
	return value;
}
void read_config_file()
{
	int i,j;
	FILE *f1=fopen("server.conf","r");
	if(f1==NULL)
	{
		printf("File didn't opened\n");
		return;
	}
	char buffer[MAX];
	int count=0;
	//printf("going to  read the file\n\n");
	while(fgets(buffer,MAX,f1)!=NULL)
	{
		i=0;
		int index=0;
		while(buffer[i]!='\0' && buffer[i]!='\n')
		{
			
			if(buffer[i]!='=')
			{
				
				config_param[count][index]=buffer[i];
				
				index++;
			}
			else
			{
				
				config_param[count][index]='\0';
				
				count++;
				index=0;
			}
			i++;
		}
		config_param[count][index]='\0';
		
		count++;
		
	}
	fclose(f1);
	
	for(i=0;i<count;i+=2)
	{
		if(strcmp(config_param[i],"LISTENING_PORT")==0)
		{
			j=0;
			while(config_param[i+1][j]!='\0')
			{
				listening_port[j]=config_param[i+1][j];
				j++;
			}
			listening_port[j]='\0';
			//listening_port=str_to_int(config_param[i+1]);
			//printf("LISTENING_PORT=%s\n",listening_port);
		}
		else if(strcmp(config_param[i],"CLIENTS_PER_THREAD")==0)
		{
			clients_per_thread=str_to_int(config_param[i+1]);
			//printf("CLIENTS_PER_THREAD=%d\n",clients_per_thread);
		}
		else if(strcmp(config_param[i],"CACHE_SIZE")==0)
		{
			cache_size=str_to_int(config_param[i+1]);
			//printf("CACHE_SIZE=%d\n",cache_size);
		}
		else if(strcmp(config_param[i],"CACHE_REPLACEMENT")==0)
		{
			j=0;
			while(config_param[i+1][j]!='\0')
			{
				cache_replacement[j]=config_param[i+1][j];
				j++;
			}
			cache_replacement[j]='\0';
			//printf("CACHE_REPLACEMENT=%s\n",cache_replacement);
		}
		else if(strcmp(config_param[i],"THREAD_POOL_SIZE_INITIAL")==0)
		{
			thread_pool_size_initial=str_to_int(config_param[i+1]);
			//printf("thread_pool_size_initial=%d\n",thread_pool_size_initial);
		}
		else if(strcmp(config_param[i],"THREAD_POOL_GROWTH")==0)
		{
			thread_pool_growth=str_to_int(config_param[i+1]);
			//printf("thread_pool_growth=%d\n",thread_pool_growth);
		}
	}
	
	return;
}


int main()
{
	int i,s;
	int sock_fd = socket(AF_INET, SOCK_STREAM, 0);

	struct addrinfo hints, *result;
	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	
	read_config_file();
	
	s = getaddrinfo(NULL, listening_port, &hints, &result);
	if (s != 0) {
	        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
        	exit(1);
	}
	connect(sock_fd, result->ai_addr, result->ai_addrlen);
	char temp_array[4][MAXI];
	char buffer[518];
	char message[MAXI];
	clock_gettime(CLOCK_MONOTONIC, &start);
	double no_of_requests=0;
	while(fgets(buffer, 518, stdin) != NULL)
	{
		no_of_requests++;
		if(strlen(buffer)<=3)
		continue;
		if(strcmp(buffer,"stop\n")==0)
		{
			message[0]='s';
			message[1]='t';
			message[2]='o';
			message[3]='p';
			message[4]='\0';
			write(sock_fd, message,5);
			break;
		}
		generate_tokens(buffer,temp_array);
		MessageEncoding(message,temp_array);
		if(strcmp(message,"ERROR")==0)
		{
			fflush(stdin);
			fflush(stdout);
			continue;
		}
		write(sock_fd, message, strlen(message));
		char resp[1000];
		fflush(stdin);
		fflush(stdout);
		
		int len = read(sock_fd, resp,513);
		
		if(resp[0]==(char)200)
		{
			//cout<<"length of the message is "<<len<<endl;
			//cout<<"lets print what client has got in response\n";
			if(len==257)
			{
				for(i=1;i<=256;i++)
				{
					cout<<resp[i];
				}
				cout<<"\n";
			}
			cout<<"SUCCESS : 200"<<endl;
		}
		else if(resp[0]==(char)240)
		{
			cout<<"ERROR : 240"<<endl;
		}
		resp[len] = '\0';
		//cout<<"data at client is "<<resp<<endl;
		fflush(stdin);
		fflush(stdout);
		
	}
	clock_gettime(CLOCK_MONOTONIC, &finish);
	elapse = (finish.tv_sec - start.tv_sec);
	elapse += (finish.tv_nsec - start.tv_nsec) / 1000000000.0;
	cout<<"Throughput = "<<no_of_requests/elapse<<endl;
return 0;
}

