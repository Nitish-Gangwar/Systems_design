#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<string.h>
#include<time.h>
#include <signal.h>
#include <setjmp.h>
#include <fcntl.h>
#define MAX 1024
int arg_count=0;

static jmp_buf env;

static volatile sig_atomic_t jump_state = 0;

void executeCommands(int start,char temp_array[][MAX],int write_file);
void write_to_file(FILE *fd_read,int start,char temp_array[][MAX]);
int checkioredirection(int start,int end,char temp_array[][MAX]);
void cat_call(int start,char temp_array[][MAX]);
int pipe_present(int start,int end,char temp_array[][MAX]);
void cat_with_grep(int start,char temp_array[][MAX],char output_filename[MAX],int write_to_file);
void sortFile_with_grep(int start,char temp_array[][MAX],char output_filename[MAX],int write_to_file);
void grep_input_output_file(int start,char temp_array[][MAX]);
int check_grep_input_output(int start,char temp_array[][MAX]);
void checkresidentmemory_with_grep(int start,char temp_array[][MAX],char output_filename[MAX],int write_to_file);
void checkcpupercentage_with_grep(int start,char temp_array[][MAX],char output_filename[MAX],int write_to_file);
void executeCommands_with_output_to_file(int start,char temp_array1[][MAX],char output_filename[MAX]);

void sigint_handler(int signo) //function executing the signal handler
{
    if (!jump_state) {
        return;
    }
    siglongjmp(env, 8);
}

long double read_cpu_jiffies(char cpu_total_jiffies[][MAX])//function for reading the /proc/stat file to read cpu jiffies
{
	char buffer[1024];
	int co=0;
	FILE *f2=fopen("/proc/stat","r");
	if(f2==NULL)
	{
		printf("Illegal command or arguments\n");
		return 0;
	}
	while(fscanf(f2, "%s", buffer)!=EOF)
   {
		if(strcmp("cpu0",buffer)==0)
		break;
		else if(strcmp("cpu",buffer)==0)
		continue;
		strcpy(cpu_total_jiffies[co],buffer);
		co++;
   }
   long double total_time_before=0;
   long double temp=0;
   int i,j,k;
   for(i=0;i<co;i++)
	{
		int index=0;
		long double number=0;
		while(cpu_total_jiffies[i][index]!='\0')
		{
			if(cpu_total_jiffies[i][index]>='0' && cpu_total_jiffies[i][index]<='9')
				number=number*10+(int)(cpu_total_jiffies[i][index]-'0');
			index++;
		}
		total_time_before+=number;
	}
	return total_time_before;
}

void listFiles()//function for executing listFiles function
{
	if(fork())//parent process
	{
	  	wait(NULL);
	  	return;
	}
	else//child process
	{
		 char *args[] = { "ls", NULL };
		 int output_fds = open("files.txt", O_WRONLY |  O_TRUNC | O_CREAT,00700  | O_CLOEXEC);
		 if(output_fds<0)
		 {
		 	printf("Illegal command or arguments\n");
		 	exit(1);
		 }
		 dup2(output_fds, 1);//dup2 writing the content to the file instead of stdout
		 execvp(args[0], &args[0]);
		 printf("Illegal command or arguments\n");
		 exit(1);
	}
}

//checkcpupercentage pid 
//checkcpupercentage pid > output.txt
//checkcpupercentage pid >> output.txt
void checkcpupercentage(int start,char temp_array[][MAX],int write_file,char output_filename[MAX],int write_to_file)//function for executing the 
{	
	int i=0;
	i=0;
	char pathname[1024]="/proc/";
	int count=6;
	int fl=0;
	while(temp_array[start+1][i]>='0' && temp_array[start+1][i]<='9')
	{
		pathname[count]=temp_array[start+1][i];
		i++;
		count++;
		fl=1;
	}
	if(count==6)
	{
		printf("Illegal command or arguments\n");
		return;
	}
	pathname[count++]='/';
	pathname[count++]='s';
	pathname[count++]='t';
	pathname[count++]='a';
	pathname[count++]='t';
	pathname[count]='\0';
	char filename[MAX];
	sprintf(filename,pathname,temp_array[start+1]);
	FILE *f=fopen(filename,"r");
	if(f==NULL)
	{
		printf("Illegal command or arguments\n");
		return;
	}
	if(fork())//parent process
	{
		wait(NULL);
		return;
	}
	else//child process for checkcpupercentage
	{
		char cpu_total_jiffies_before[MAX][MAX];
		long double total_time_before=read_cpu_jiffies(cpu_total_jiffies_before);
		char ch;
		long long int buff;
		long double bu,utime_before,utime_after,stime_before,stime_after,cutime_before,cutime_after,cstime_before,cstime_after;
		char temp[MAX];
		fscanf(f,"%lld %s %c %lld %lld %lld %lld %lld %lld %lld %lld %lld %lld %Lf %Lf %Lf %Lf",&buff,temp,&ch,&buff,&buff,&buff,&buff,&buff,&buff,&buff,&buff,&buff,&buff,&utime_before,&stime_before,&cutime_before,&cstime_before);
		  fclose(f);
		  
		  sleep(2);
				  
				  
		 char cpu_total_jiffies_after[MAX][MAX];
		 long double total_time_after=read_cpu_jiffies(cpu_total_jiffies_after);
		 FILE *f3=fopen(filename,"r");
		 if(f3==NULL)
		 {
		 	printf("Illegal command or arguments\n");
  			exit(1);
		 }
		 fscanf(f3,"%lld %s %c %lld %lld %lld %lld %lld %lld %lld %lld %lld %lld %Lf %Lf %Lf %Lf",&buff,temp,&ch,&buff,&buff,&buff,&buff,&buff,&buff,&buff,&buff,&buff,&buff,&utime_after,&stime_after,&cutime_after,&cstime_after);
		 fclose(f3);
				
		  double user_percentage=100*(utime_after - utime_before) / (total_time_after - total_time_before);
		  double system_percentage=100*(stime_after - stime_before) / (total_time_after - total_time_before);
		  if(write_file==0 && write_to_file==0)
		  {
		 		printf("user mode cpu percentage: %0.2lf%%\nsystem mode cpu percentage: %0.2lf%%\n",user_percentage,system_percentage);
		  }
		  else if(write_to_file==1)
		  {
		  		int fd_write;
		  		fd_write = open(output_filename,O_WRONLY | O_APPEND | O_CREAT,00700  | O_CLOEXEC);
		  		if(fd_write<0)
		  		{
		  			printf("Illegal command or arguments\n");
		  			exit(1);
		  		}
		  		dup2(fd_write,1);
		  		close(fd_write);
		  		printf("user mode cpu percentage: %0.2lf%%\nsystem mode cpu percentage: %0.2lf%%\n",user_percentage,system_percentage);

		  }
		  else
		  {
		  		int fd_write;
		  		if(strcmp(temp_array[start+2],">")==0)
		  			fd_write = open(temp_array[start+3],O_WRONLY | O_TRUNC | O_CREAT,00700  | O_CLOEXEC); 
		  		else if(strcmp(temp_array[start+2],">>")==0)
		  		   fd_write = open(temp_array[start+3],O_WRONLY | O_APPEND | O_CREAT,00700  | O_CLOEXEC);
		  		else 
		  		{
		  			printf("Illegal command or arguments\n");
		  			exit(1);
		  		}
		  		if(fd_write<0)
		  		{
		  			printf("Illegal command or arguments\n");
		  			exit(1);
		  		}
		  		dup2(fd_write,1);
		  		close(fd_write);
		  		printf("user mode cpu percentage: %0.2lf%%\nsystem mode cpu percentage: %0.2lf%%\n",user_percentage,system_percentage);
		  }
		  exit(1);						  
	}
}

//sortFile input.txt > output.txt
//sortFile input.txt
//sortFile input.txt >> output.txt
void sortFile(int start,char temp_array[][MAX],int write_file,char output_filename[MAX],int write_to_file)//function for sorting the file and printing content over the shell
{
	if(temp_array[start+1]!=NULL)
	{
		char *filename=temp_array[start+1];
		if(fork())//parent process
		{
			wait(NULL);
			return;
		}
		else//child process
		{
			char *args[]={"sort",filename,NULL};
			int input_fds=open(filename,O_RDONLY);
			if(input_fds<0)
	  		{
	  			printf("Illegal command or arguments\n");
	  			exit(1);
	  		}
			dup2(input_fds,STDIN_FILENO);
			close(input_fds);
			if(write_file==1 && write_to_file==0)
			{
				int fd_write;
				if(strcmp(temp_array[start+2],">")==0)
				fd_write = open(temp_array[start+3],O_WRONLY | O_TRUNC | O_CREAT,00700  | O_CLOEXEC); 
				else if(strcmp(temp_array[start+2],">>")==0)
				fd_write = open(temp_array[start+3],O_WRONLY | O_APPEND | O_CREAT,00700  | O_CLOEXEC);
				else
				{
					printf("Illegal command or arguments\n");
		  			exit(1);
				}
				if(fd_write<0)
		  		{
		  			printf("Illegal command or arguments\n");
		  			exit(1);
		  		}
		  		dup2(fd_write,1);
		  		close(fd_write);
			}
			else if(write_to_file==1)
			{
				int fd_write;
				fd_write = open(output_filename,O_WRONLY | O_APPEND | O_CREAT,00700  | O_CLOEXEC);
				if(fd_write<0)
		  		{
		  			printf("Illegal command or arguments\n");
		  			exit(1);
		  		}
		  		dup2(fd_write,1);
		  		close(fd_write);
			}
			execvp(args[0],args);
			printf("Illegal command or arguments\n");
			exit(1);
		}
	}
	else
	{
		printf("Illegal command or arguments\n");
		return;
	}
}


void checkresidentmemory(int start,char temp_array[][MAX],int write_file,char output_filename[MAX],int write_to_file)//function for checking the resident memory of a given pid process
{
	if(temp_array[start+1]!=NULL)
	{
		if(fork())//parent process
		{
			wait(NULL);//let the child execute
			return;
		}
		else//child process
		{
			char filename[MAX]="/proc/";
			int index=6;
			int tt=0;
			while(temp_array[start+1][tt]!='\0')
			{
				filename[index]=temp_array[start+1][tt];
				tt++;
				index++;
			}
			if(index==6)
			{
				printf("Illegal command or arguments\n");
				exit(1);
			}
			filename[index++]='/';
			filename[index++]='s';
			filename[index++]='t';
			filename[index++]='a';
			filename[index++]='t';
			filename[index++]='\0';
			FILE *f=fopen(filename,"r");
			if(f==NULL)
			{
				printf("Illegal command or arguments\n");
				exit(1);
			}
			fclose(f);
			if(write_file==1 && write_to_file==0)
			{
				int fd_write;
				if(strcmp(temp_array[start+2],">")==0)
				fd_write = open(temp_array[start+3],O_WRONLY | O_TRUNC | O_CREAT,00700  | O_CLOEXEC);
		  		else if(strcmp(temp_array[start+2],">>")==0)
		  		fd_write = open(temp_array[start+3],O_WRONLY | O_APPEND | O_CREAT,00700  | O_CLOEXEC);
		  		else 
		  		{
		  			printf("Illegal command or arguments\n");
		  			exit(1);
		  		}
		  		if(fd_write<0)
		  		{
		  			printf("Illegal command or arguments\n");
		  			exit(1);
		  		}
		  		dup2(fd_write,1);
		  		close(fd_write);
			}
			else if(write_to_file==1)
			{
				int fd_write;
		  		fd_write = open(output_filename,O_WRONLY | O_APPEND | O_CREAT,00700  | O_CLOEXEC);
		  		if(fd_write<0)
		  		{
		  			printf("Illegal command or arguments\n");
		  			exit(1);
		  		}
		  		dup2(fd_write,1);
		  		close(fd_write);
			}
			char *argv[]={"ps","-q",temp_array[start+1],"eo","rss","--no-heading",NULL};
			execve("/usr/bin/ps",argv,NULL);
			printf("Illegal command or arguments\n");
			exit(1);
		}
	}
	else
	{
		printf("Illegal command or arguments​\n");
		return;
	}
}

void tokkens(char *str,char temp_array[][MAX])//function tokkenizing the given input
{
	int i,j,k;
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
		i++;	
	}
	arg_count=position;
	return;
}



void two_way_command(int start,int j,char temp_array[][MAX],char output_filename[MAX],int write_to_file)//function executing the two way query
{
	int child=fork();
	if(child)//parent process
	{
		wait(NULL);
		start=j+1;
		int tt=0;
		tt=pipe_present(start,arg_count,temp_array);
		if(tt)// executing the query of the type cat input.txt | grep wordsearch 
		{
			if(strcmp(temp_array[start],"cat")==0 && strcmp(temp_array[tt+1],"grep")==0)
			{
					cat_with_grep(start,temp_array,output_filename,write_to_file);
			}
			else if(strcmp(temp_array[start],"sortFile")==0 && strcmp(temp_array[tt+1],"grep")==0)
			{
				sortFile_with_grep(start,temp_array,output_filename,write_to_file);
			}
			else if(strcmp(temp_array[start],"checkcpupercentage")==0 && strcmp(temp_array[tt+1],"grep")==0)
			{
				checkcpupercentage_with_grep(start,temp_array,output_filename,write_to_file);
			}
			else if(strcmp(temp_array[start],"checkresidentmemory")==0 && strcmp(temp_array[tt+1],"grep")==0)
			{
				checkresidentmemory_with_grep(start,temp_array,output_filename,write_to_file);
			}
			else
			{
				printf("Illegal command or arguments\n");
				return;
			}
		}
		else if(checkioredirection(start,arg_count,temp_array))//checking for the command types with ioredirection
		{
			if(strcmp(temp_array[start],"cat")==0)// checking for the call type cat filename.txt > output_filename.txt
			{
				cat_call(start,temp_array);
			}
			else if(strcmp(temp_array[start],"checkcpupercentage")==0)// writing the output of checkcpupercentage pid > output.txt
			{
				checkcpupercentage(start,temp_array,1,NULL,0);//1 for writing to the file
			}
			else if(strcmp(temp_array[start],"sortFile")==0)
			{
				sortFile(start,temp_array,1,NULL,0);
			}
			else if(strcmp(temp_array[start],"checkresidentmemory")==0)
			{
				checkresidentmemory(start,temp_array,1,NULL,0);
			}
			else if(strcmp(temp_array[start],"grep")==0)
			{
				if(check_grep_input_output(start,temp_array))//means write the output to the file
				{
					grep_input_output_file(start,temp_array);
				}
				else
				{
					printf("Illegal command or arguments\n");
					return;
				}
			}
			else if(strcmp(temp_array[start],"executeCommands")==0)
			{	
				if(strcmp(temp_array[start+2],">")==0)	// ; executeCommands command.txt > output.txt
				{
					fclose(fopen(output_filename,"w+"));
					executeCommands_with_output_to_file(start,temp_array,output_filename);
				}
				else
				{
					executeCommands_with_output_to_file(start,temp_array,output_filename);
				}
			}
			else
			{
				printf("Illegal command or arguments\n");
				return;
			}
		}
		else if(strcmp(temp_array[start],"checkcpupercentage")==0)
		{
			checkcpupercentage(start,temp_array,0,output_filename,write_to_file);
		}
		else if(strcmp(temp_array[start],"listFiles")==0)
		{
			listFiles();
		}
		else if(strcmp("executeCommands",temp_array[start])==0)
		{
			executeCommands(start,temp_array,0);
		}
		else if(strcmp(temp_array[start],"sortFile")==0)
		{
			sortFile(start,temp_array,0,output_filename,write_to_file);
		}
		else if(strcmp(temp_array[start],"checkresidentmemory")==0)
		{
			checkresidentmemory(start,temp_array,0,output_filename,write_to_file);
		}
		else
		{
			printf("Illegal command or arguments\n");
		}
		return;
	}
	else//child process executing the first query of two way query
	{
			int tt=0;
			tt=pipe_present(start,j,temp_array);
			if(tt)// executing the query of the type cat input.txt | grep wordsearch 
			{
				if(strcmp(temp_array[start],"cat")==0 && strcmp(temp_array[tt+1],"grep")==0)
				{
						cat_with_grep(start,temp_array,output_filename,write_to_file);
				}
				else if(strcmp(temp_array[start],"sortFile")==0 && strcmp(temp_array[tt+1],"grep")==0)
				{
					sortFile_with_grep(start,temp_array,output_filename,write_to_file);
				}
				else if(strcmp(temp_array[start],"checkcpupercentage")==0 && strcmp(temp_array[tt+1],"grep")==0)
				{
					checkcpupercentage_with_grep(start,temp_array,output_filename,write_to_file);
				}
				else if(strcmp(temp_array[start],"checkresidentmemory")==0 && strcmp(temp_array[tt+1],"grep")==0)
				{
					checkresidentmemory_with_grep(start,temp_array,output_filename,write_to_file);
				}
				else
				{
					printf("Illegal command or arguments\n");
					exit(1);
				}
			}
			else if(checkioredirection(start,j,temp_array))
			{
				if(strcmp(temp_array[start],"cat")==0)
				{
					cat_call(start,temp_array);  //cat input_file > output_file
				}
				else if(strcmp(temp_array[start],"checkcpupercentage")==0)
				{
					checkcpupercentage(start,temp_array,1,NULL,0);//1 for writing to the file
				}
				else if(strcmp(temp_array[start],"sortFile")==0)
				{
					sortFile(start,temp_array,1,NULL,0);
				}
				else if(strcmp(temp_array[start],"checkresidentmemory")==0)
				{
					checkresidentmemory(start,temp_array,1,NULL,0);
				}
				else if(strcmp(temp_array[start],"grep")==0)
				{
					if(check_grep_input_output(start,temp_array))//means write the output to the file
					{
						grep_input_output_file(start,temp_array);
					}
					else
					{
						printf("Illegal command or arguments\n");
						exit(1);
					}
				}
				else if(strcmp(temp_array[start],"executeCommands")==0)
				{																		// ; executeCommands command.txt > output.txt
					if(strcmp(temp_array[start+2],">")==0)
					{
						fclose(fopen(output_filename,"w+"));
						executeCommands_with_output_to_file(start,temp_array,output_filename);
					}
					else
					{
						executeCommands_with_output_to_file(start,temp_array,output_filename);
					}
				}
				
				else
				{
					printf("Illegal command or arguments\n");
					exit(1);
				}
			}
			else if(strcmp(temp_array[start],"checkcpupercentage")==0)
			{
				checkcpupercentage(start,temp_array,0,output_filename,write_to_file);
			}
			else if(strcmp(temp_array[start],"listFiles")==0)
			{
				listFiles();
			}
			else if(strcmp(temp_array[start],"sortFile")==0)
			{
				sortFile(start,temp_array,0,output_filename,write_to_file);
			}
			else if(strcmp("executeCommands",temp_array[start])==0)
			{
				executeCommands(start,temp_array,0);
			}
			else if(strcmp(temp_array[start],"checkresidentmemory")==0)
			{
				checkresidentmemory(start,temp_array,0,output_filename,write_to_file);
			}
			else
			{
				printf("Illegal command or arguments\n");
			}
			exit(1);
	}
}

int check_two_way_query(int start,char temp_array[][MAX])//function to check two way query
{
	int i,j;
	for(j=0;j<arg_count;j++)//checking for two way command in this part
	{
		if(strcmp(temp_array[j],";")==0)
		{
			return j;
		}
	}
	return -1;
}

void executeCommands(int start,char temp_array1[][MAX],int write_file) 
{
	FILE *f9=fopen(temp_array1[start+1],"r");//opening the file mentioned on shell
	if(f9==NULL)
	{
		printf("Illegal command or arguments\n");
		return;
	}
	FILE *fPtr = fopen(temp_array1[start+1], "a");
	char tem[MAX]="\n";
	fputs(tem, fPtr);
	fclose(fPtr);
	char st[MAX];
	size_t BUFSIZE=MAX;
	char temp_array[MAX][MAX];//array to get the query one by one from the given text file
	while(fgets(st,MAX,f9)!=NULL)
	{
		if(strlen(st)==1 || strlen(st)==2 || strlen(st)==0)
		{
			break;
		}
		int to=strlen(st)-1;
		if(st[to]!='\0')
		st[to]='\0';
		tokkens(st,temp_array);
		start=0;
		int j=check_two_way_query(start,temp_array);//checking the file entry is two way query or not
		if(j>=0)
		{
			two_way_command(start,j,temp_array,NULL,0);// command1 ; command2
		}
		else
		{
			int tt=0;
			tt=pipe_present(start,arg_count,temp_array);
			if(tt)
			{			// cat input.txt | grep pattern > output.txt 
						// cat input.txt | grep pattern 
				if(strcmp(temp_array[start],"cat")==0 && strcmp(temp_array[tt+1],"grep")==0)
				{
						cat_with_grep(start,temp_array,NULL,0);
				}		// sortFile input.txt | grep pattern > output.txt
						// sortFile input.txt | grep pattern
				else if(strcmp(temp_array[start],"sortFile")==0 && strcmp(temp_array[tt+1],"grep")==0)
				{
					sortFile_with_grep(start,temp_array,NULL,0);
				}
				else if(strcmp(temp_array[start],"checkcpupercentage")==0 && strcmp(temp_array[tt+1],"grep")==0)
				{
					checkcpupercentage_with_grep(start,temp_array,NULL,0);
				}
				else if(strcmp(temp_array[start],"checkresidentmemory")==0 && strcmp(temp_array[tt+1],"grep")==0)
				{
					checkresidentmemory_with_grep(start,temp_array,NULL,0);
				}
				else
				{
					printf("Illegal command or arguments\n");
				}
			}
			else if(checkioredirection(start,arg_count,temp_array))
			{
				if(strcmp(temp_array[start],"cat")==0)
				{
					cat_call(start,temp_array);
				}
				else if(strcmp(temp_array[start],"checkcpupercentage")==0)
				{
					checkcpupercentage(start,temp_array,1,NULL,0);//1 for writing to the file
				}
				else if(strcmp(temp_array[start],"sortFile")==0)//matching the query with sortFile query
				{
					sortFile(start,temp_array,1,NULL,0);
				}
				else if(strcmp(temp_array[start],"checkresidentmemory")==0)//matching the query with checkresidentmemory
				{
					checkresidentmemory(start,temp_array,1,NULL,0);
				}
				else if(strcmp(temp_array[start],"grep")==0)
				{
					if(check_grep_input_output(start,temp_array))//means write the output to the file
					{
						grep_input_output_file(start,temp_array);
					}
					else
					{
						printf("Illegal command or arguments\n");
					}
				}
				else
				{
					printf("Illegal command or arguments\n");
				}
			}
			else if(strcmp(temp_array[start],"checkcpupercentage")==0)//matching the query with checkcpupercentage query
			{
				checkcpupercentage(start,temp_array,0,NULL,0);
			}
			else if(strcmp(temp_array[start],"listFiles")==0)//matching the query with listFiles query
			{
				listFiles();
			}
			else if(strcmp(temp_array[start],"sortFile")==0)//matching the query with sortFile query
			{
				sortFile(start,temp_array,0,NULL,0);
			}
			else if(strcmp(temp_array[start],"checkresidentmemory")==0)//matching the query with checkresidentmemory
			{
				checkresidentmemory(start,temp_array,0,NULL,0);
			}
			else if(strcmp(temp_array[start],"exit")==0)//exit command
			{
				exit(1);
			}
			else//if nothing matches uptill now then error can be because of query not typed properly
			{
				printf("Illegal command or arguments\n");
			}
	   }
	}
	fclose(f9);
	return;
}


volatile sig_atomic_t done = 0;
 
void term(int signum)
{
    done = 1;
    printf("Got SIGTERM-Leaving\n");
    exit(1);
}

int checkioredirection(int start,int end,char temp_array[][MAX])
{
	int i,j,k;
	int semicolon=0;
	int redirection=0;
	for(i=start;i<end;i++)
	{
		if(strcmp(temp_array[i],">")==0 || strcmp(temp_array[i],">>")==0)
		{
			return i;
		}
	}
	return 0;
}

// cat inputfile > outputfile
// cat inputfile >> outputfile
void cat_call(int start,char temp_array[][MAX])  
{
	int child=fork();
	if(child)   //parent process
	{
		wait(NULL);
		return;
	}
	else        //child process
	{
		FILE *fd_read=fopen(temp_array[start+1],"r");
		int length;
		if(fd_read==NULL)
		{
			printf("Illegal command or arguments\n");
			exit(1);
		}
		char buffer[MAX];
		size_t BUFSIZE=MAX;
		FILE *fd_write;
		if(strcmp(temp_array[start+2],">")==0)
		{
			fd_write=fopen(temp_array[start+3],"w+");
			if(fd_write==NULL)
			{
				printf("Illegal command or arguments\n");
				exit(1);
			}
		}
		else if(strcmp(temp_array[start+2],">>")==0)
		{
		  fd_write=fopen(temp_array[start+3],"a+");
		  if(fd_write==NULL)
		  {
				printf("Illegal command or arguments\n");
				exit(1);
		  }
		}
		else
		{
			printf("Illegal command or arguments\n");
			exit(1);
		}
	  while (!feof(fd_read)) 
	  {
		  fgets(buffer, sizeof(buffer), fd_read);
		  fprintf(fd_write, "%s", buffer);
	  }
	  fclose(fd_read);
	  fclose(fd_write);
	  exit(1);
	}
}

int pipe_present(int start,int end,char temp_array[][MAX])
{
	int i;
	for(i=start;i<end;i++)
	{
		if(strcmp(temp_array[i],"|")==0)
		{
			return i;
		}
	}
	return 0;
}

//cat files.txt | grep pattern
void cat_with_grep(int start,char temp_array[][MAX],char output_filename[MAX],int write_to_file)
{
  pid_t pid1,pid2 ;
  int fd_read = open(temp_array[start+1], O_RDONLY); 
  if(fd_read<0)
  {
	 	printf("Illegal command or arguments\n");
	 	return;
  }
  pid1 = fork(); 
  if(pid1 > 0)//parent process
  {
    wait(NULL);
    return;
  }
  else//child process of pid1
  {
  		int fd[2];
      pipe(fd);
	  	pid2=fork();
	  	if(pid2 > 0)//parent process
		{
			 wait(NULL);
			 close(0);
			 dup(fd[0]);
			 close (fd[1]);
			 close(fd[0]);
			 int i;
			 char *exp[MAX];
			 int count=0;
			 int flag=0;
			 int index=start;
			 for(i=start+3;i<arg_count;i++)
			 {
			 	if(strcmp(temp_array[i],">")==0 || strcmp(temp_array[i],">>")==0)
			 	{
			 		flag=1;
				 	index=i;
				 	break;
				}
				else  if(strcmp(temp_array[i],";")==0)
				{
					flag=0;
					index=i;
					break;
				}
			 	exp[count]=(char *)&temp_array[i];  
			 	count++;
			 }
			 exp[count++]=NULL;
			 int file_d;
			 if(flag==1)   // command will be like cat files_txt | grep word > output_file_txt
			 {
				if(strcmp(temp_array[index],">")==0)
				{
			 		file_d=open(temp_array[index+1],O_WRONLY | O_TRUNC | O_CREAT,00700  | O_CLOEXEC);
			 	}
			 	else if(strcmp(temp_array[index],">>")==0)
			 	{
			 		file_d=open(temp_array[index+1],O_WRONLY | O_APPEND | O_CREAT,00700  | O_CLOEXEC);
			 	}
			 	else
			 	{
			 		printf("Illegal command or arguments\n");
			 		exit(1);
			 	}
			 	if(file_d<0)
			 	{
			 		printf("Illegal command or arguments\n");
			 		exit(1);
			 	}
			 	dup2(file_d,1);
			 	close(file_d);
			 }
			 else if(write_to_file==1)
			 {
			 	file_d=open(output_filename,O_WRONLY | O_APPEND | O_CREAT,00700  | O_CLOEXEC);
			 	if(file_d<0)
			 	{
			 		printf("Illegal command or arguments\n");
			 		exit(1);
			 	}
			 	dup2(file_d,1);
			 	close(file_d);
			 }
			 if((execvp("grep", exp)) < 0) 
			 {
				printf("Illegal command or arguments\n");
			 }
			 exit(EXIT_FAILURE);
		}
		else
		{
			 close(1);
			 dup(fd[1]);
			 close(fd[0]);
			 close(fd[1]);
			 char *exp[] = {temp_array[start], temp_array[start+1], NULL};
			 if( (execvp("cat", exp)) < 0)
			  {
					printf("Illegal command or arguments\n");
			  }
			  exit(EXIT_FAILURE);
		}
  }
}

void sortFile_with_grep(int start,char temp_array[][MAX],char output_filename[MAX],int write_to_file)
{
  pid_t pid1,pid2 ;
  int fd_read = open(temp_array[start+1], O_RDONLY); 
  if(fd_read<0)
  {
	 	printf("Illegal command or arguments\n");
	 	return;
  }
  pid1 = fork(); 
  if(pid1 > 0)//parent process
  {
    wait(NULL);
    return;
  }
  else//child process of pid1
  {
  		int fd[2];
      pipe(fd);
	  	pid2=fork();
	  	if(pid2 > 0)//parent process
		{
			 wait(NULL);
			 int file_d;
			 close(0);
			 dup(fd[0]);//duplicating the stdin to fd0 of pipe
			 close (fd[1]);
			 close(fd[0]);
			 int i;
			 char *exp[MAX];
			 int count=0;
			 int index=start+3;
			 int flag=0;
			 for(i=start+3;i<arg_count;i++)
			 {
			 	if(strcmp(temp_array[i],">")==0 || strcmp(temp_array[i],">>")==0)
			 	{
			 		flag=1;
				 	index=i;
				 	break;
				}
				else if(strcmp(temp_array[i],";")==0)
				{
					flag=0;
					index=i;
					break;
				}
			 	exp[count]=(char *)&temp_array[i];
			 	count++;
			 }
			 exp[count]=NULL;
			 if(flag==1)
			 {
			 	if(strcmp(temp_array[index],">")==0)
			 	{
			 		file_d=open(temp_array[index+1],O_WRONLY | O_TRUNC | O_CREAT,00700  | O_CLOEXEC);
			 	}
			 	else if(strcmp(temp_array[index],">>")==0)
			 	{
			 		file_d=open(temp_array[index+1],O_WRONLY | O_APPEND | O_CREAT,00700  | O_CLOEXEC);
			 	}
			 	else
			 	{
			 		printf("Illegal command or arguments\n");
			 		exit(1);
			 	}
			 	if(file_d<0)
			 	{
			 		printf("Illegal command or arguments\n");
			 		exit(1);
			 	}
			 	dup2(file_d,1);
			 	close(file_d);
			 }
			 else if(write_to_file==1)
			 {
			 	file_d=open(output_filename,O_WRONLY | O_APPEND | O_CREAT,00700  | O_CLOEXEC);
			 	if(file_d<0)
			 	{
			 		printf("Illegal command or arguments\n");
			 		exit(1);
			 	}
			 	dup2(file_d,1);
			 	close(file_d);
			 }
			 if((execvp("grep", exp)) < 0) 
			 {
				printf("Illegal command or arguments\n");
			 }
			 exit(EXIT_FAILURE);
		}
		else
		{
			 close(1);
			 dup(fd[1]);
			 close(fd[0]);
			 close(fd[1]);
			 char *args[]={"sort",temp_array[start+1],NULL};
			 if(execvp(args[0],args)<0)
			 printf("Illegal command or arguments\n");
			 exit(EXIT_FAILURE);
			 
		}
  }
}
void int_to_string(char str[],int pid)//program converting the integer pid to string
{	
	int index=0;
	while(pid)
	{
		str[index]=pid%10+ '0';
		pid=pid/10;
		index++;
	}
	str[index]='\0';
	int start=0,end=index-1;
	while(start<end)
	{
		char temp=str[start];
		str[start]=str[end];
		str[end]=temp;
		start++;
		end--;
	}
	return;
}

int check_grep_input_output(int start,char temp_array[][MAX])
{
	int i,j,k;
	int opening=0,closing=0;
	for(i=start;i<arg_count;i++)
	{	
		if(strcmp(temp_array[i],"<")==0 && opening==0)
		{
			opening=i;
		}
		else if(((strcmp(temp_array[i],">")==0) || (strcmp(temp_array[i],">>")==0)) && closing==0)
		{
			closing=i;
		}
	}
	if(opening && closing)
	{
		return 1;
	}
	else
	return 0;
}


/* grep word < input.txt > output.txt*/
void grep_input_output_file(int start,char temp_array[][MAX])//grep function requires work its not working
{
	int pid=fork();
	if(pid)//parent executing 
	{
		wait(NULL);
		return;
	}
	else//child executing
	{
		int file_d;
		int i;
		int count=0;
		char *exp[] = {temp_array[start], temp_array[start+1], temp_array[start+3],NULL};
		if(strcmp(temp_array[start+4],">")==0)
		{
		 	file_d=open(temp_array[start+5],O_WRONLY | O_TRUNC  | O_CREAT,00700 | O_CLOEXEC);
	 	}
	 	else if(strcmp(temp_array[start+4],">>")==0)
	 	{
	 		file_d=open(temp_array[start+5],O_WRONLY | O_APPEND  | O_CREAT,00700 | O_CLOEXEC);
	 	}
	 	else
	 	{
	 		printf("Illegal command or arguments\n");
	 		exit(1);
	 	}
	 	if(file_d<0)
	 	{
	 		printf("Illegal command or arguments\n");
	 		exit(1);
	 	}
	 	dup2(file_d,1);
	 	close(file_d);

		if((execvp("grep", exp)) < 0) 
		{
			printf("Illegal command or arguments\n");
		}
		exit(EXIT_FAILURE);
	}
}

//checkresidentmemory pid | grep pattern
//checkresidentmemory pid | grep pattern > file.txt
//checkresidentmemory pid | grep pattern >> file.txt
void checkresidentmemory_with_grep(int start,char temp_array[][MAX],char output_filename[MAX],int write_to_file)
{
	if(temp_array[start+1]!=NULL)
	{
		pid_t pid1;
		pid1=fork();
		if(pid1)//parent process
		{
			wait(NULL);//let the child execute
			return;
		}
		else//child process
		{
			char filename[MAX]="/proc/";
			int index=6;
			int tt=0;
			while(temp_array[start+1][tt]!='\0')
			{
				filename[index]=temp_array[start+1][tt];
				tt++;
				index++;
			}
			if(index==6)
			{
				printf("Illegal command or arguments\n");
				exit(1);
			}
			filename[index++]='/';
			filename[index++]='s';
			filename[index++]='t';
			filename[index++]='a';
			filename[index++]='t';
			filename[index++]='\0';
			FILE *f=fopen(filename,"r");
			if(f==NULL)
			{
				printf("Illegal command or arguments\n");
				exit(1);
			}
			fclose(f);
			pid_t pid2;
			int fd[2];
			pipe(fd);
			pid2=fork();
			if(pid2) //parent process
			{
				wait(NULL);
				close(0);
				int file_d;
				dup(fd[0]);
				close(fd[0]);  //very important
				close(fd[1]);  //very important
				int i;
				char *exp[MAX];
				int count=0;
				int index=start+3;
				int flag=0;
				for(i=start+3;i<arg_count;i++)
				{
				 	if(strcmp(temp_array[i],">")==0 || strcmp(temp_array[i],">>")==0)
				 	{
				 		flag=1;
					 	index=i;
					 	break;
					}
					else if(strcmp(temp_array[i],";")==0)
					{
						flag=0;
						index=i;
						break;
					}
				 	exp[count]=(char *)&temp_array[i];
				 	count++;
				}
				exp[count]=NULL;
				if(flag==1)
				{
					if(strcmp(temp_array[index],">")==0)
					{
						file_d=open(temp_array[index+1],O_WRONLY | O_TRUNC | O_CREAT,00700 | O_CLOEXEC);
				   }
				   else if(strcmp(temp_array[index],">>")==0)
				   {
				   	file_d=open(temp_array[index+1],O_WRONLY | O_APPEND | O_CREAT,00700 | O_CLOEXEC);
				   }
				   else
				   {
				   	printf("Illegal command or arguments\n");
				 		exit(1);
				   }
				   if(file_d<0)
				   {
				 		printf("Illegal command or arguments\n");
				 		exit(1);
				   }
				   dup2(file_d,1);
				   close(file_d);
				}
				else if(write_to_file==1)
				{
					file_d=open(output_filename,O_WRONLY | O_APPEND | O_CREAT,00700 | O_CLOEXEC);
				   if(file_d<0)
				   {
				 		printf("Illegal command or arguments\n");
				 		exit(1);
				   }
				   dup2(file_d,1);
				   close(file_d);
				}
				execvp("grep",exp);
				printf("Illegal command or arguments\n");
				exit(1);
			}
			else //child process
			{
				close(1);
				dup(fd[1]);
				close(fd[0]);
				close(fd[1]);
				char *argv[]={"ps","-q",temp_array[start+1],"eo","rss","--no-heading",NULL};
				execve("/usr/bin/ps",argv,NULL);
				printf("Illegal command or arguments\n");
				exit(1);
			}
		}
	}
	else
	{
		printf("Illegal command or arguments\n");
		return;
	}
	return;
}

//checkcpupercentage pid | grep pattern
//checkcpupercentage pid | grep pattern
void checkcpupercentage_with_grep(int start,char temp_array[][MAX],char output_filename[MAX],int write_to_file)
{
	int i=0;
	i=0;
	char pathname[1024]="/proc/";
	int count=6;
	int fl=0;
	while(temp_array[start+1][i]>='0' && temp_array[start+1][i]<='9')
	{
		pathname[count]=temp_array[start+1][i];
		i++;
		count++;
		fl=1;
	}
	if(count==6)
	{
		printf("Illegal command or arguments\n");
		return;
	}
	pathname[count++]='/';
	pathname[count++]='s';
	pathname[count++]='t';
	pathname[count++]='a';
	pathname[count++]='t';
	pathname[count]='\0';
	char filename[MAX];
	sprintf(filename,pathname,temp_array[start+1]);
	FILE *f=fopen(filename,"r");
	if(f==NULL)
	{
		printf("Illegal command or arguments\n");
		return;
	}
	pid_t pid1=fork();
	if(pid1)//parent process
	{
		wait(NULL);
		return;
	}
	else//child process for checkcpupercentage
	{
		int fd[2];
		pipe(fd);
		pid_t pid2;
		pid2=fork();
		if(pid2)//parent process
		{
			wait(NULL);
			int file_d;
			close(0);
			dup(fd[0]);//read end duplicated
			close(fd[0]);
			close(fd[1]);
			int i;
			char *exp[MAX];
			int count=0;
			int index=start+3;
			int flag=0;
			for(i=start+3;i<arg_count;i++)
			{
			 	if(strcmp(temp_array[i],">")==0 || strcmp(temp_array[i],">>")==0)
			 	{
			 		flag=1;
				 	index=i;
				 	break;
				}
				else if(strcmp(temp_array[i],";")==0)
				{
					flag=0;
					index=i;
					break;
				}
			 	exp[count]=(char *)&temp_array[i];
			 	count++;
			}
			exp[count]=NULL;
			if(flag==1)
			{
				if(strcmp(temp_array[index],">")==0)
				{
			  		file_d=open(temp_array[index+1],O_WRONLY | O_TRUNC| O_CREAT,00700 | O_CLOEXEC);
			  	}
			  	else if(strcmp(temp_array[index],">>")==0)
			  	{
			  		file_d=open(temp_array[index+1],O_WRONLY | O_APPEND | O_CREAT,00700 | O_CLOEXEC);
			  	}
			  	else
			  	{
			  		printf("Illegal command or arguments\n");
			 		exit(1);
			  	}
			   if(file_d<0)
			   {
			 		printf("Illegal command or arguments\n");
			 		exit(1);
			   }
			   dup2(file_d,1);
			   close(file_d);
		   }
		   else if(write_to_file==1)
		   {
			  	file_d=open(output_filename,O_WRONLY | O_APPEND | O_CREAT,00700 | O_CLOEXEC);
			   if(file_d<0)
			   {
			 		printf("Illegal command or arguments\n");
			 		exit(1);
			   }
			   dup2(file_d,1);
			   close(file_d);
		   }
		   execvp("grep",exp);
		   printf("Illegal command or arguments\n");
			exit(1);
		}
	   else //child processes
	   {
	 		char cpu_total_jiffies_before[MAX][MAX];
			long double total_time_before=read_cpu_jiffies(cpu_total_jiffies_before);
			char ch;
			long long int buff;
			long double bu,utime_before,utime_after,stime_before,stime_after,cutime_before,cutime_after,cstime_before,cstime_after;
			char temp[MAX];
			fscanf(f,"%lld %s %c %lld %lld %lld %lld %lld %lld %lld %lld %lld %lld %Lf %Lf %Lf %Lf",&buff,temp,&ch,&buff,&buff,&buff,&buff,&buff,&buff,&buff,&buff,&buff,&buff,&utime_before,&stime_before,&cutime_before,&cstime_before);
			fclose(f);
			sleep(2);
			char cpu_total_jiffies_after[MAX][MAX];
			long double total_time_after=read_cpu_jiffies(cpu_total_jiffies_after);
			FILE *f3=fopen(filename,"r");
			if(f3==NULL)
			{
			 	printf("Illegal command or arguments\n");
	  		   exit(1);
			}
			fscanf(f3,"%lld %s %c %lld %lld %lld %lld %lld %lld %lld %lld %lld %lld %Lf %Lf %Lf %Lf",&buff,temp,&ch,&buff,&buff,&buff,&buff,&buff,&buff,&buff,&buff,&buff,&buff,&utime_after,&stime_after,&cutime_after,&cstime_after);
			fclose(f3);
			double user_percentage=100*(utime_after - utime_before) / (total_time_after - total_time_before);
			double system_percentage=100*(stime_after - stime_before) / (total_time_after - total_time_before);
		   close(1);
		   dup(fd[1]);//here it will write content to the write end of the file
		   close(fd[0]);
		   close(fd[1]);
		   printf("user mode cpu percentage: %0.2lf%%\nsystem mode cpu percentage: %0.2lf%%\n",user_percentage,system_percentage);
		   exit(1);	
	   }					  
	}
}

//executeCommands command.txt > output_filename.txt
void executeCommands_with_output_to_file(int start,char temp_array1[][MAX],char output_filename[MAX])
{
	FILE *f9=fopen(temp_array1[start+1],"r");//opening the file mentioned on shell
	if(f9==NULL || output_filename==NULL )
	{
		printf("Illegal command or arguments\n");
		return;
	}
	FILE *fPtr = fopen(temp_array1[start+1], "a");
	char tem[MAX]="\n";
	fputs(tem, fPtr);
	fclose(fPtr);
	char st[MAX];
	size_t BUFSIZE=MAX;
	char temp_array[MAX][MAX];//array to get the query one by one from the given text file
	while(fgets(st,MAX,f9)!=NULL)
	{
		if(strlen(st)==1 || strlen(st)==2 || strlen(st)==0)
		{
			break;
		}
		int to=strlen(st)-1;
		if(st[to]!='\0')
		st[to]='\0';
		tokkens(st,temp_array);
		start=0;
		int j=check_two_way_query(start,temp_array);//checking the file entry is two way query or not
		if(j>=0)
		{
			two_way_command(start,j,temp_array,output_filename,1);// command1 ; command2
		}
		else
		{
			int tt=0;
			tt=pipe_present(start,arg_count,temp_array);
			if(tt)
			{			// cat input.txt | grep pattern > output.txt 
						// cat input.txt | grep pattern 
				if(strcmp(temp_array[start],"cat")==0 && strcmp(temp_array[tt+1],"grep")==0)
				{
						cat_with_grep(start,temp_array,output_filename,1);
				}		// sortFile input.txt | grep pattern > output.txt
						// sortFile input.txt | grep pattern
				else if(strcmp(temp_array[start],"sortFile")==0 && strcmp(temp_array[tt+1],"grep")==0)
				{
					sortFile_with_grep(start,temp_array,output_filename,1);
				}
				else if(strcmp(temp_array[start],"checkcpupercentage")==0 && strcmp(temp_array[tt+1],"grep")==0)
				{
					checkcpupercentage_with_grep(start,temp_array,output_filename,1);
				}
				else if(strcmp(temp_array[start],"checkresidentmemory")==0 && strcmp(temp_array[tt+1],"grep")==0)
				{
					checkresidentmemory_with_grep(start,temp_array,output_filename,1);
				}
				else
				{
					printf("Illegal command or arguments\n");
				}
			}
			else if(checkioredirection(start,arg_count,temp_array))
			{
				if(strcmp(temp_array[start],"cat")==0)
				{
					cat_call(start,temp_array);
				}
				else if(strcmp(temp_array[start],"checkcpupercentage")==0)
				{
					checkcpupercentage(start,temp_array,1,NULL,0);//1 for writing to the file
				}
				else if(strcmp(temp_array[start],"sortFile")==0)//matching the query with sortFile query
				{
					sortFile(start,temp_array,1,NULL,0);
				}
				else if(strcmp(temp_array[start],"checkresidentmemory")==0)//matching the query with checkresidentmemory
				{
					checkresidentmemory(start,temp_array,1,NULL,0);
				}
				else if(strcmp(temp_array[start],"grep")==0)
				{
					if(check_grep_input_output(start,temp_array))//means write the output to the file
					{
						grep_input_output_file(start,temp_array);
					}
					else
					{
						printf("Illegal command or arguments\n");
					}
				}
				else
				{
					printf("Illegal command or arguments\n");
				}
			}
			else if(strcmp(temp_array[start],"checkcpupercentage")==0)//matching the query with checkcpupercentage query
			{
				checkcpupercentage(start,temp_array,0,output_filename,1);
			}
			else if(strcmp(temp_array[start],"listFiles")==0)//matching the query with listFiles query
			{
				listFiles();
			}
			else if(strcmp(temp_array[start],"sortFile")==0)//matching the query with sortFile query
			{
				sortFile(start,temp_array,0,output_filename,1);
			}
			else if(strcmp(temp_array[start],"checkresidentmemory")==0)//matching the query with checkresidentmemory
			{
				checkresidentmemory(start,temp_array,0,output_filename,1);
			}
			else if(strcmp(temp_array[start],"exit")==0)//exit command
			{
				exit(1);
			}
			else//if nothing matches uptill now then error can be because of query not typed properly
			{
				printf("Illegal command or arguments\n");
			}
	   }
	}
	fclose(f9);
	return;
}


int main()//main directing everyone to their respective work
{
	signal(SIGINT, sigint_handler);
	struct sigaction action;
   memset(&action, 0, sizeof(struct sigaction));
   action.sa_handler = term;
   sigaction(SIGTERM, &action, NULL);
	while(1)//infinite shell running 
	{
		fflush(stdin);
		fflush(stdout);
		int write_file=0;
		int start=0;
		//int flag1=0;
		int next_line=0;
		if (sigsetjmp(env, 1) == 8) 
		{
			char ch;
         printf("the program is interrupted, do you want to exit [Y/N]​\n");
       	scanf("%c",&ch);
       	if(ch=='Y' || ch=='y')
       	{
       		exit(1);
       	}
       	else
       	{
       		continue;
       	}
      }
      jump_state = 1; 
		int flag=0;
		size_t buffersize=MAX;
		int i,j,k;
		int usercmdcount=5;
		char *str;
		int pid=getpid();
		char pid_process[MAX];
		int_to_string(pid_process,pid);
		start=0;
		char temp_array[MAX][MAX];//this char array will contain tokkens of query
		str=(char*)malloc(sizeof(char)*buffersize);//dynamically allocating memory
		int buf;
		printf("myShell>");
		buf=getline(&str,&buffersize,stdin);//taking input from the shell
		str[buf-1]='\0';//doing this because last char is nextline operator
		char **string_array;
		tokkens(str,temp_array);//tokkenizing the string
		int check=0;
		int io=0;
		if(strcmp(temp_array[0],"exit")==0)//exit
		{
			exit(1);
		}
		else if(buf==0 || buf==1)//something might be pressed type output or just nextline operator
		{
			printf("\n");
			continue;
		}
		//kill -15 pid
		else if(strcmp(temp_array[start],"kill")==0 && strcmp(temp_array[start+1],"-15")==0 && strcmp(temp_array[start+2],pid_process)==0)
		{
			printf("Got SIGTERM-Leaving\n");
			exit(1);
		}
		else
		{
			int j=0;
			j=check_two_way_query(start,temp_array);// command1 ; command2
			if(j>=0)
			{	//executeCommands command.txt > output.txt ; executeCommands command.txt > output.txt
				if(strcmp(temp_array[start],"executeCommands")==0 && checkioredirection(start,j,temp_array))
				{
					two_way_command(start,j,temp_array,temp_array[start+3],0);// command1 ; command2
				}
				else if(strcmp(temp_array[j+1],"executeCommands")==0 && checkioredirection(j+1,arg_count,temp_array))
				{
					two_way_command(start,j,temp_array,temp_array[j+4],0);
				}
				else
				{
					two_way_command(start,j,temp_array,NULL,0);
				}
				continue;
			}
			else
			{
				int tt=0;
				tt=pipe_present(start,arg_count,temp_array); // checking that in the query is pipe present or not
					if(tt)
					{				// cat input.txt | grep pattern >> output.txt
						if(strcmp(temp_array[start],"cat")==0 && strcmp(temp_array[tt+1],"grep")==0)
						{													
								cat_with_grep(start,temp_array,NULL,0); // cat input.txt | grep pattern
						}													// cat input.txt | grep pattern > output.txt
						else if(strcmp(temp_array[start],"sortFile")==0 && strcmp(temp_array[tt+1],"grep")==0)
						{														// sortFile input.txt | grep pattern >> output.txt
							sortFile_with_grep(start,temp_array,NULL,0); // sortFile input.txt | grep pattern
						}													  // sortFile input.txt | grep pattern > output.txt
						else if(strcmp(temp_array[start],"checkcpupercentage")==0 && strcmp(temp_array[tt+1],"grep")==0)
						{												//checkcpupercentage pid | grep pattern > output.txt
							checkcpupercentage_with_grep(start,temp_array,NULL,0);//checkcpupercentage pid | grep pattern >> output.txt
						}												//checkcpupercentage pid | grep pattern > output.txt
						else if(strcmp(temp_array[start],"checkresidentmemory")==0 && strcmp(temp_array[tt+1],"grep")==0)
						{												//checkresidentmemory pid | grep pattern > output.txt
							checkresidentmemory_with_grep(start,temp_array,NULL,0);//checkresidentmemory pid | grep pattern >> output.txt
						}												// checkresidentmemory pid | grep pattern
						else
						{
							printf("Illegal command or arguments\n");
							continue;
						}
					}
					else if(checkioredirection(start,arg_count,temp_array))
					{
						if(strcmp(temp_array[start],"cat")==0)  //cat inputfile.txt > outputfile.txt
						{													 //cat inputfile.txt >> outputfile.txt
							cat_call(start,temp_array);
						}
						else if(strcmp(temp_array[start],"checkcpupercentage")==0)  //checkcpupercentage pid > outputtt.txt
						{
							checkcpupercentage(start,temp_array,1,NULL,0);//1 for writing to the file
						}
						else if(strcmp("checkresidentmemory",temp_array[start])==0)//checkresidentmemory pid > output.txt
						{
							checkresidentmemory(start,temp_array,1,NULL,0);
						}
						else if(strcmp(temp_array[start],"sortFile")==0)//sortFile input.txt > output.txt
						{
							sortFile(start,temp_array,1,NULL,0);
						}
						else if(strcmp(temp_array[start],"grep")==0)
						{
							if(check_grep_input_output(start,temp_array))
							{
								grep_input_output_file(start,temp_array);	//grep pattern < input.txt > output.txt
							}
							else
							{
								printf("Illegal command or arguments\n");
								continue;
							}
						}
						else if(strcmp(temp_array[start],"executeCommands")==0) //executeCommands command.txt > output.txt
						{
							if(strcmp(temp_array[start+2],">")==0)
							{
								fclose(fopen(temp_array[start+3],"w+"));
								executeCommands_with_output_to_file(start,temp_array,temp_array[start+3]);
							}
							else
							{
								executeCommands_with_output_to_file(start,temp_array,temp_array[start+3]);
							}
						}
						else
						{
							printf("Illegal command or arguments\n");
							continue;
						}
					}
					else if(strcmp(temp_array[start],"checkcpupercentage")==0)//checkcpupercentage pid
					{
						checkcpupercentage(start,temp_array,0,NULL,0);//function call to checkcpupercentage
					}
					else if(strcmp(temp_array[start],"listFiles")==0)//listFiles 
					{
						listFiles();//function call  to listFiles
					}
					else if(strcmp(temp_array[start],"sortFile")==0)//sortFile input.txt
					{
						sortFile(start,temp_array,0,NULL,0);
					}
					else if(strcmp("checkresidentmemory",temp_array[start])==0)//checkresidentmemory pid
					{
						checkresidentmemory(start,temp_array,0,NULL,0);
					}
					else if(strcmp("executeCommands",temp_array[start])==0)//executeCommands command.txt
					{
						executeCommands(start,temp_array,0);
					}
					else
					{
						printf("Illegal command or arguments\n");
						continue;
					}
			}
		}
	}
	return 0;
}
