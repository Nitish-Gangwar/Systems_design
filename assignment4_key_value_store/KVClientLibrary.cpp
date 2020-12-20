#include<bits/stdc++.h>
#define MAXIV 514
#include "KVClientLibrary.h"
using namespace std;
void generate_tokens(char str[MAXIV],char temp_array[][MAXIV])//function tokkenizing the given input
{
	int i,j;
	i=0;
	j=0;
	int position=0;
	while(str[i]!='\0')
	{
		while(str[i]!=' ' && str[i]!='\0')
		{
			temp_array[position][j]=str[i];
			i++;
			j++;
		}
		if(j!=0)
		{
			temp_array[position][j]='\0';
			j=0;
			position++;
		}
		if(str[i]!='\0')
		i++;	
		
	}
}


void MessageEncoding(char message[MAXIV],char key[][MAXIV])
{
	int i,count=1;
	
	if(strcmp(key[0],"GET")==0)
	{
		//GET will send 0 and 256 bytes key so total 257bytes of data
		//cout<<"wait message is being prepared";
		message[0]='1';
		i=0;
		while(key[1][i]!='\0' && key[1][i]!='\n')
		{
			message[count]=key[1][i];
			count++;
			i++;
		}
		
		for(i=count;i<=256;i++)
		{
			message[i]='%';
		}
		message[257]='\0';
		//write the logic for encoding the GET command
	}
	else if(strcmp(key[0],"PUT")==0)
	{
		//cout<<"Getting the PUT request\n";
		message[0]='2';
		i=0;
		count=1;
		//cout<<"len of key is "<<strlen(key[1])<<endl;
		while(key[1][i]!='\0' && key[1][i]!='\n' && key[1][i]!=' ')
		{
			message[count]=key[1][i];
			count++;
			i++;
			
		}
		//cout<<"\n";
		for(i=count;i<=256;i++)
		{
			message[i]='%';
		}
		count=i;
		i=0;
		//cout<<"length of value = "<<strlen(key[2])<<endl;
		//cout<<"value= "<<key[2]<<endl;
		while(key[2][i]!='\0' && key[2][i]!='\n' && key[2][i]!=' ')
		{
			message[count]=key[2][i];
			//cout<<message[count]<<" "<<count<<" ";
			count++;
			i++;
		}
		if(count==257)
		{
			cout<<"ERROR in requesting way"<<endl;
			message[0]='E';
			message[1]='R';
			message[2]='R';
			message[3]='O';
			message[4]='R';
			message[5]='\0';
		}
		for(i=count;i<=512;i++)
		{
			message[i]='%';
		}
		message[513]='\0';
		//cout<<"encoded message at client kvlibrary is "<<message<<endl;
		//cout<<"message generated for PUT format is "<<message<<endl;
		//write the logic for encoding the PUT request which will have key and value both
	}
	else if(strcmp(key[0],"DEL")==0)
	{
		//cout<<"getting the DEL request\n";
		//cout<<"********************wait message is being prepared************************\n"; 
		message[0]='3';
		i=0;
		while(key[1][i]!='\0' && key[1][i]!='\n')
		{
			message[count]=key[1][i];
			count++;
			i++;
		}
		
		for(i=count;i<=256;i++)
		{
			message[i]='%';
		}
		message[257]='\0';
	}
}

