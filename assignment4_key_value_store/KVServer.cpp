#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
#include <assert.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include<bits/stdc++.h>
#include "KVServerLibrary.cpp"
#include "KVStore.cpp"
#include "KVStore.h"
#define SERVERPORT 8888
#define SERVERBACKLOG 10
#define THREADSNO 20
#define EVENTS_BUFF_SZ 256
using namespace std;
static int serversock;
static int epoll_fd;
static pthread_t threads[THREADSNO];
#define MAXIV 514
#define MAX 1024
char config_param[MAX][MAX];
char listening_port[MAX]; //VARIABLE SPECIFIES THE PORT NUMBER OVER WHICH SERVER IS LISTENING
int clients_per_thread;//NUMBER OF CLIENTS THAT EACH WORKER THREAD HANDLES
int cache_size;		// DEFINES THE CACHE SIZE
char cache_replacement[MAX];//CACHE REPLACEMENT ALGORITHM BEING USED
int thread_pool_size_initial;	//INTIAL THREAD POOL SIZE
int thread_pool_growth;		//THREAD POOL INCREMENT ONCE THREADS ENDS

int LRUcacheused=0;
bool LRUcachefull=false;

int readercount=0,writercount=0,waitingreader=0,activewriter=0;
pthread_mutex_t mutexcache;
pthread_cond_t canreadcache,canwritecache;


pthread_mutex_t lock1;

//this is the structure for doubly linked list for implementing the lru cache
struct dll{
    string key;
    string value;
    struct dll* left;
    struct dll* right;
};


dll* headLRU=new(dll);
dll* endLRU;
//map for implementing the cache directly searching the cache for key
map<string,dll*> mymapLRU;

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
	printf("going to  read the file\n\n");
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
			printf("LISTENING_PORT=%s\n",listening_port);
		}
		else if(strcmp(config_param[i],"CLIENTS_PER_THREAD")==0)
		{
			clients_per_thread=str_to_int(config_param[i+1]);
			printf("CLIENTS_PER_THREAD=%d\n",clients_per_thread);
		}
		else if(strcmp(config_param[i],"CACHE_SIZE")==0)
		{
			cache_size=str_to_int(config_param[i+1]);
			printf("CACHE_SIZE=%d\n",cache_size);
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
			printf("CACHE_REPLACEMENT=%s\n",cache_replacement);
		}
		else if(strcmp(config_param[i],"THREAD_POOL_SIZE_INITIAL")==0)
		{
			thread_pool_size_initial=str_to_int(config_param[i+1]);
			printf("thread_pool_size_initial=%d\n",thread_pool_size_initial);
		}
		else if(strcmp(config_param[i],"THREAD_POOL_GROWTH")==0)
		{
			thread_pool_growth=str_to_int(config_param[i+1]);
			printf("thread_pool_growth=%d\n",thread_pool_growth);
		}
	}
	
	return;
}


void add_to_cache(string key,string value, map<string,dll*> &mymap, dll** head, dll** end, int* cused){

    dll* temp=*head;
    //pthread_mutex_lock(&lock1);
    if((*cused)==0)
    {//this is the case when cache is empty then insert at the start
    	
        (*head)->key = key;
        (*head)->value = value;
        (*head)->left=NULL;
        (*head)->right=NULL;
        *end=*head;
        //make_hash(mymap,key,*head);
        mymap[key]=*head;
        
        }
    else
    {	//this is the cache when cache is not empty then insert in the beginning
        dll* temp2=new(dll);
        temp->left=temp2;
        temp2->right=temp;
        temp2->left=NULL;
        temp2->key=key;
        temp2->value= value;
        *head=temp2;
        //make_hash(mymap,key,*head);
        mymap[key]=*head;
    }
    cout<<"value added to cache successfully\n";
    //pthread_mutex_unlock(&lock1);

}

void LRUCache(string key,string value, map<string,dll*> &mymapLRU,dll** head,dll** end,int csize)
{
    int i;
    if(mymapLRU.find(key)==mymapLRU.end())
    {
        //current key or random number is not available in the current hash map
        pthread_mutex_lock(&mutexcache);
        writercount++;
        //cout<<"going to write the cache successfully\n";
        while(readercount > 0 || activewriter > 0)
        	pthread_cond_wait(&canwritecache,&mutexcache);
        activewriter++;
        pthread_mutex_unlock(&mutexcache);
       //cout<<"After the condition check\n";
        if(!LRUcachefull)
        {
            //If the cache capacity is empty, more nodes are added
            add_to_cache(key,value,mymapLRU,head,end,&LRUcacheused);
            
            //pthread_mutex_lock(&lock1);
            
            LRUcacheused++;
            if(LRUcacheused==csize)
                LRUcachefull=true;
                
            //pthread_mutex_unlock(&lock1);
            
        }
        else
        {	//cache entry has to be written to metadata and to persistent storage
            //pthread_mutex_lock(&lock1);
            
            int len1=((*end)->key).length();
            int len2=((*end)->value).length();
            char key1[len1+1],value1[len2+1];
            for(i=0;i<len1;i++)
            {
            	key1[i]=((*end)->key)[i];
            }
            key1[i]='\0';
            for(i=0;i<len2;i++)
            {
            	value1[i]=((*end)->value)[i];
            }
            value1[i]='\0';
            //If the cache is fully , the least used element or element at last is removed
           
            cout<<"data is going to be written to persistent storage i.e."<<key1<<" "<<value1<<endl;
            insert_into_metadata(key1,value1);
            mymapLRU.erase(mymapLRU.find((*end)->key));
            dll* temp=(*end)->left;
            temp->right=NULL;
            delete *end;
            *end=temp;
            dll* temp2=new(dll);
            temp2->key=key;
            temp2->value=value;
            temp2->right=*head;
            temp2->left=NULL;
            (*head)->left=temp2;
            *head=temp2;
            //make_hash(mymapLRU,key,*head);
            mymapLRU[key]=*head;
            
            //pthread_mutex_unlock(&lock1);	
        }
        pthread_mutex_lock(&mutexcache);
        //activewriter--;
        if(--activewriter!=0)
        {
        	pthread_cond_signal(&canwritecache);
        }
        else
        {
        	pthread_cond_broadcast(&canreadcache);
        }
        pthread_mutex_unlock(&mutexcache);
    }
    else
    {
    	//cout<<"going to update the map\n";
        //Its a Cache Hit! Below code updates the position of the current key to be the head
	//pthread_mutex_lock(&lock1);
	/*pthread_mutex_lock(&mutexcache);
    	while(writercount>0)
    	{	
	    	pthread_cond_wait(&canreadcache,&mutexcache);
    	}
    	readercount++;
    	pthread_mutex_unlock(&mutexcache);*/
    	pthread_mutex_lock(&mutexcache);
        writercount++;
        //cout<<"going to update the cache successfully\n";
        while(readercount > 0 || activewriter > 0)
        	pthread_cond_wait(&canwritecache,&mutexcache);
        activewriter++;
        pthread_mutex_unlock(&mutexcache);
    	
        dll* temp;
        int len1=key.length();
    	char key1[len1+1];
    	
    	for(i=0;i<len1;i++)
    	{
    		key1[i]=key[i];
    	}
    	key1[i]='\0';
    	
    	
        
        temp=mymapLRU.find(key)->second;
        //cout<<"overwritting the value of key in both cache and persistent storage: "<<temp->value<<endl;
        //temp->value=value;
        mymapLRU.find(key)->second->value=value;
        int len2=(temp->value).length();
        
        char value1[len2+1];
        
        for(i=0;i<len2;i++)
    	{
    		value1[i]=(temp->value)[i];
    	}
        value1[i]='\0';
        
        insert_into_metadata(key1,value1);
        
        if(temp!=*head)
        {
		dll* left=temp->left;
		dll* right=temp->right;
		temp->right=*head;
		temp->left=NULL;
		(*head)->left=temp;
		*head=temp;
		left->right=right;
		if(right!=NULL)
			right->left=left;
		else
		    *end=left;
        }
        /*pthread_mutex_lock(&mutexcache);
        if(--readercount==0)
        	pthread_cond_signal(&canwritecache);
        pthread_mutex_unlock(&mutexcache);*/
        pthread_mutex_lock(&mutexcache);
        //activewriter--;
        if(--activewriter!=0)
        {
        	pthread_cond_signal(&canwritecache);
        }
        else
        {
        	pthread_cond_broadcast(&canreadcache);
        }
        pthread_mutex_unlock(&mutexcache);
    }
}
int accept_new_client(void)
{
	//cout<<"going to accept connection request\n";
    int clientsock;
    struct sockaddr_in addr;
    socklen_t addrlen = sizeof(addr);
    if ((clientsock = accept(serversock, (struct sockaddr *) &addr, &addrlen)) < 0) {
        return -1;
    }

    char ip_buff[INET_ADDRSTRLEN+1];
    if (inet_ntop(AF_INET, &addr.sin_addr, ip_buff, sizeof(ip_buff)) == NULL) {
        close(clientsock);
        return -1;
    }

    printf("*** [%p] Client connected from %s:%" PRIu16 "\n", (void *) pthread_self(),
           ip_buff, ntohs(addr.sin_port));

    struct epoll_event epevent;
    epevent.events = EPOLLIN | EPOLLET;
    epevent.data.fd = clientsock;

    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, clientsock, &epevent) < 0) {
        perror("epoll_ctl(2) failed attempting to add new client");
        close(clientsock);
        return -1;
    }
	//cout<<"file descriptor added successfully\n";
    return 0;
}

int handle_request(int clientfd) {
    char buf[514];
    struct sockaddr_in addr;
    socklen_t addrlen = sizeof(addr);
    ssize_t n;
    int s,j;
    if ((n = recv(clientfd,buf, sizeof(buf)-1, 0)) < 0) {
        return -1;
    }

    if (n == 0) {
        return 0;
    }

    buf[n] = '\0';
    //cout<<"content obtained at server is  "<<buf<<"\n";
    char temp_array[4][MAXIV];
    MessageDecoding(buf,temp_array); //server decoding the KVmessage send by the client
    
    if (getpeername(clientfd, (struct sockaddr *) &addr, &addrlen) < 0) {
        return -1;
    }

    char ip_buff[INET_ADDRSTRLEN+1];
    if (inet_ntop(AF_INET, &addr.sin_addr, ip_buff, sizeof(ip_buff)) == NULL) {
        return -1;
    }

    printf("*** [%p] [%s:%" PRIu16 "] -> server: %s", (void *) pthread_self(),
           ip_buff, ntohs(addr.sin_port), buf);
    
    char buffer[514];
    uint8_t code;
	//code = 200 for success
	//code = 240 for error
	if(buf[0]=='1')
	{
		//thats GET command
		
		char tempo[257];
		
		if(mymapLRU.find(temp_array[1])!=mymapLRU.end())
		{
			//cout<<"checking the value inside the cache\n";
			dll * temp=mymapLRU.find(temp_array[1])->second;
			string value_of_key=temp->value;
			code=200;
			int len=value_of_key.length();
			j=0;
			//cout<<"value of the asked key is "<<endl;
			for(j=0;j<len;j++)
			{
				tempo[j]=(char)value_of_key[j];
				//cout<<tempo[j]<<" ";
			}
			tempo[len]='\0';
			LRUCache(temp_array[1],temp_array[2], mymapLRU,&headLRU,&endLRU,cache_size);
		}
		else
		{
			//cout<<"value not found inside the cache going to search inside the file\n"; 
			if(check_from_file(temp_array[1],tempo))//this will check the key inside metadata and will fetch it from file
			{	//and places the value inside tempo
				code=200;
				LRUCache(temp_array[1],temp_array[2], mymapLRU,&headLRU,&endLRU,cache_size);
			}
			else
			{
				code=240;
			}
			//code for error
		}
		//pthread_mutex_unlock(&lock1);
		MessageEncoding(code,tempo,buffer,1); //server encoding the GET kVmessage response
	}
	else if(buf[0]=='2')
	{	//here temp_array[3] contains the key-value pair 
		//this is PUT command
		//cout<<"inside the PUT command\n";
		LRUCache(temp_array[1],temp_array[2], mymapLRU,&headLRU,&endLRU,cache_size);
		code=200;
		//cout<<"request full filled successfully\n";
		//cout<<"going to write the persistent storage\n";
		//insert_into_metadata(temp_array[1],temp_array[2]);
		MessageEncoding(code,temp_array[3],buffer,2); //server encoding the PUT KVmessage response
		//cout<<"message encoded is "<<buffer<<endl;
	}
	else if(buf[0]=='3')
	{	//here temp_array[1] contains the key
		//thats DEL command
		
		if(mymapLRU.find(temp_array[1])==mymapLRU.end())
		{
			if(metadata.find(temp_array[1])==metadata.end())//key not found inside the metadata also
			{
				code=240;	//so error code
			}
			else
			{
				//cout<<"going to erase the entry from metadata too\n";
				metadata.erase(temp_array[1]);
				code=200;
			}	
			//cout<<"ERROR no such key in cache\n";
			
		}
		else
		{
			mymapLRU.erase(temp_array[1]);
			code=200;
			//cout<<"going to erase the entry from metadata too\n";
			metadata.erase(temp_array[1]);
		}
		MessageEncoding(code,temp_array[1],buffer,3); //server encoding the DEL KVmessage response	
    }
    //cout<<"here goes the server replies";
    //s= write(fd,buffer,strlen(buffer));
    //pthread_mutex_lock(&lock1);
    //cout<<"going to send the message to the server\n";
    s=send(clientfd,buffer,strlen(buffer),0);
    //cout<<"message sent to client successfully\n";
    //pthread_mutex_unlock(&lock1);
    if(s==-1)
    {
		perror("write");
		return -1;
    }
    return 0;
    
    /*ssize_t sent;
    if ((sent = send(clientfd, readbuff, n, 0)) < 0) {
        return -1;
    }

    readbuff[sent] = '\0';

    printf("*** [%p] server -> [%s:%" PRIu16 "]: %s", (void *) pthread_self(),
           ip_buff, ntohs(addr.sin_port), readbuff);

    return 0;*/
}

void *worker_thr(void *args) {
    struct epoll_event *events = (epoll_event*)malloc(sizeof(*events)*EVENTS_BUFF_SZ);
    if (events == NULL) {
        perror("malloc(3) failed when attempting to allocate events buffer");
        pthread_exit(NULL);
    }

    int events_cnt;
    while ((events_cnt = epoll_wait(epoll_fd, events, EVENTS_BUFF_SZ, -1)) > 0) {
        int i;
        for (i = 0; i < events_cnt; i++) {
            assert(events[i].events & EPOLLIN);

            if (events[i].data.fd == serversock) {
                if (accept_new_client() == -1) {
                    fprintf(stderr, "Error accepting new client: %s\n",
                        strerror(errno));
                }
            } else {
            cout<<"got some data on "<<events[i].data.fd<<endl;
                if (handle_request(events[i].data.fd) == -1) {
                    fprintf(stderr, "Error handling request: %s\n",
                        strerror(errno));
                }
            }
        }
    }

    if (events_cnt == 0) {
        fprintf(stderr, "epoll_wait(2) returned 0, but timeout was not specified...?");
    } else {
        perror("epoll_wait(2) error");
    }

    free(events);

    return NULL;
}

int main(void) {
    read_config_file();
    if ((serversock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
        perror("socket(2) failed");
        exit(EXIT_FAILURE);
    }
    pthread_cond_init(&canreadcache,NULL);
    pthread_cond_init(&canwritecache,NULL);
    pthread_mutex_init(&mutexcache,NULL);
    struct sockaddr_in serveraddr;
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(atoi(listening_port));
    serveraddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(serversock, (const struct sockaddr *) &serveraddr, sizeof(serveraddr)) < 0) 
    {
        perror("bind(2) failed");
        exit(EXIT_FAILURE);
    }

    if (listen(serversock, SERVERBACKLOG) < 0) 
    {
        perror("listen(2) failed");
        exit(EXIT_FAILURE);
    }

    if ((epoll_fd = epoll_create(1)) < 0) 
    {
        perror("epoll_create(2) failed");
        exit(EXIT_FAILURE);
    }

    struct epoll_event epevent;
    epevent.events = EPOLLIN | EPOLLET;
    epevent.data.fd = serversock;

    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, serversock, &epevent) < 0) 
    {
        perror("epoll_ctl(2) failed on main server socket");
        exit(EXIT_FAILURE);
    }

    int i;
    for (i = 0; i <thread_pool_size_initial ; i++) 
    {
        if (pthread_create(&threads[i], NULL, worker_thr, NULL) < 0) 
        {
            perror("pthread_create(3) failed");
            exit(EXIT_FAILURE);
        }
    }

    /* main thread also contributes as worker thread */
    worker_thr(NULL);

    return 0;
}
