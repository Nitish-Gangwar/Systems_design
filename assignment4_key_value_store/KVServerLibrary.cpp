#include<bits/stdc++.h>
#define MAXI 514
using namespace std;
void MessageEncoding(uint8_t code,char temp_array[MAXI],char encoded_msg[MAXI],int request_code)
{
	int i,index;
	if(request_code == 1)//this is the response encoding for GET request
	{
		i=0;
		index=0;
		encoded_msg[index]=(int)code;
		index++;
		while(temp_array[i]!='\0' && temp_array[i]!='\n')
		{
			encoded_msg[index]=temp_array[i];
			index++;
			i++;
		}
		//cout<<"server message encoding length = "<<index<<endl;
		encoded_msg[index]='\0';
	}
	else if(request_code == 2)//this is the response encoding for PUT request
	{
		i=0;
		index=0;
		encoded_msg[index]=(int)code;
		index++;
		while(temp_array[i]!='\0')
		{
			encoded_msg[index]=temp_array[i];
			index++;
			i++;
		}
		encoded_msg[index]='\0';
		//cout<<"message encoding length for PUT cmd = "<<index<<endl;	
	}
	else if(request_code == 3)
	{
		i=0;
		index=0;
		encoded_msg[index]=(int)code;
		index++;
		encoded_msg[index]='\0';
		/*while(temp_array[i]!='\0')
		{
			encoded_msg[index]=temp_array[i];
			index++;
			i++;
		}*/
	}
}
void MessageDecoding(char buffer[],char temp_array[][MAXI])
{
	int i,count;
	if(buffer[0]=='1')
	{	//This is for GETting the value corresponding to the key
		i=1;
		count=0;
		temp_array[0][0]='1';
		temp_array[0][1]='\0';
		while(buffer[i]!='\0' && buffer[i]!='\n')
		{
			temp_array[1][count]=buffer[i];
			i++;
			count++;
		}
		temp_array[1][count]='\0';
		//for GET request temp_array index[1] will suffice
	}
	else if(buffer[0]=='2')
	{	//This is for PUTting the key and value inside the cache and persistent storage
		//cout<<"PUT message from client\n";
		temp_array[0][0]='2';
		temp_array[0][1]='\0';
		count=0;
		i=1;
		//cout<<"length of message is = "<<strlen(buffer)<<endl;
		for(i=1;i<=256;i++)
		{
			temp_array[1][count]=buffer[i];
			count++;
		}
		temp_array[1][count]='\0';
		count=0;
		while(buffer[i]!='\0' && buffer[i]!='\n')
		{
			temp_array[2][count]=buffer[i];
			//cout<<"count= "<<count<<"-"<<temp_array[2][count]<<" ";
			i++;
			count++;
			
		}
		temp_array[2][count]='\0';
		i=1;
		count=0;
		while(buffer[i]!='\0' && buffer[i]!='\n')
		{
			temp_array[3][count]=buffer[i];
			i++;
			count++;
		}
		temp_array[3][count]='\0';
		//cout<<"length of third index value = "<<strlen(temp_array[3])<<endl;
	}
	else if(buffer[0]=='3')
	{	//This is for deleting the key and value given the key corresponding to it
		//cout<<"DEL message from client\n";
		i=1;
		count=0;
		temp_array[0][0]='3';
		temp_array[0][1]='\0';
		while(buffer[i]!='\0' && buffer[i]!='\n')
		{
			temp_array[1][count]=buffer[i];
			i++;
			count++;
		}
		temp_array[1][count]='\0';
	}
	else
	{
		cout<<"ERROR WRONG FORMAT FOR REQUEST\n";
	}
}
