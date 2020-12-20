#include<bits/stdc++.h>
#include "KVStore.h"

using namespace std;
//int key_size=3,value_size=1;
int key_size=SIZE,value_size=SIZE;
struct node{
    char key[SIZE];
    int flag;
    unsigned long offset;
};
typedef struct node node_t;

map<string,node_t*> metadata;

unsigned long file_offset;

void insert_into_metadata(char key[], char value[])
{
    FILE *fp; 
    fp = fopen("seeking", "a+");
    
    if(metadata.find(key)==metadata.end())
    {	//new entry needs to be inserted inside the metadata
    	//cout<<"element needs to be added to metadata and needs to be written to persistent storage\n";
    	fwrite(key,1,strlen(key),fp); //key is written inside the 
    	
    	file_offset=ftell(fp);
    	node_t *ptr;   
	ptr = (node_t *)malloc(sizeof(struct node));
	strcpy(ptr->key,key);
	
	ptr->offset=file_offset;
	cout<<"offset is "<<ptr->offset<<endl;
	ptr->flag=1;
	metadata[key]=ptr;
	
    	fwrite(value,1,strlen(value),fp);
	fflush(fp); 
    	fclose(fp);
    }
    else
    {	//key is already present inside the metadata
    	fwrite(key,1,strlen(key),fp);
    	file_offset=ftell(fp);
    	//cout<<"file offset older = "<<file_offset<<endl;
    	metadata[key]->offset=file_offset;
    	//cout<<"offset new = "<<metadata[key]->offset<<endl;
    	fwrite(value,1,strlen(value),fp);
	fflush(fp); 
    	fclose(fp);
    }
    
    
}
int check_from_file(char key[],char value[SIZE])
{
	//char buffer[SIZE];
	//int size_of_entry=;
	if(metadata.find(key)!=metadata.end())	//entry is lying inside the persistent storage
	{
		FILE *fp; 
		fp = fopen("seeking", "r");
		if(fp==NULL)
		{
			cout<<"FILE OPENING ERROR\n";
			return 0;
		}
		fseek(fp,metadata[key]->offset,SEEK_SET); //seek_set means from starting and offset is provided from metadata
		printf("pos is at %ld\n",ftell(fp));
		fread(value,value_size,1,fp);
		value[value_size]='\0';
        	fflush(fp);
        	fclose(fp); 
	}
	else	//entry is missing from persistent storage
	{
		cout<<"value not present inside the metadata"<<endl;
		return 0;
	}
	return 1;
}

/*void display()  
{
    map<string,node_t*>::iterator it;
    for(it=metadata.begin();it!=metadata.end();it++)
    {
    	cout<<"key = "<<it->first<<" key = "<<it->second->key<<" offset= "<<it->second->offset<<endl;
    }
}
int main() 
{ 
	int i,j,k,n;
	char key[SIZE],value[SIZE];
	cout<<"enter the number of key value pair is going to be added\n";
	cin>>n;
	for(i=0;i<n;i++)
	{
		cout<<"enter the key value pair"<<endl;
		cin>>key>>value;
		insert_into_metadata(key,value);
	}
    	display();
    	char ch;
    	do
    	{
    	fflush(stdin);
    	fflush(stdout);
    	cout<<"enter the entry you want to read\n";
    	cin>>key;
    	char value1[SIZE];
    	check_from_file(key,value1);
    	cout<<"value = "<<value1<<endl;
    	cout<<"want to make another query press y"<<endl;
    	cin>>ch;
    	}while(ch=='y');
    	return 0; 
}*/
