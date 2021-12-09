#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <termios.h>

// I am going to use Linked list as the main data structure for this task.
struct Node { 
    char data[1024];
    int pid; 
    struct Node* next; 
};
struct Node* head = NULL; 
struct Node* current = NULL; 



// Shell pid, pgid, terminal modes
static pid_t GBSH_PID;
static pid_t GBSH_PGID;
static int GBSH_IS_INTERACTIVE;
static struct termios GBSH_TMODES;

static char* currentDirectory;
extern char** environ;

struct sigaction act_child;
struct sigaction act_int;

int no_reprint_prmpt;

#define TRUE 1
#define FALSE !TRUE

pid_t pid;

#define LIMIT 128 
#define MAXLINE 512



//Displays the prompt for the shell
void shellPrompt()
{
char host[100];
	strcpy(host,"Kaushal's_pc");
	char cwd[1024];
	char hostn[1204] = "";
	gethostname(hostn, sizeof(hostn));
	getcwd(cwd, sizeof(cwd));
	if (strncmp(cwd,hostn,strlen(hostn))==0){
		char *b = cwd +strlen(hostn);
		printf("<%s:~%s>",host,b);
	}
	else{
		printf("<%s:%s> ",host,cwd);
	}	
}

//Method to change directory (the shell command chdir or cd)
int changeDirectory(char* args[]){
	// If no path written (only 'cd'), then go to the home directory
	if (args[1] == NULL) {
		chdir(getenv("HOME")); 
		return 1;
	}
	// Else change the directory to the one specified by the argument, if possible
	else{ 
		if (chdir(args[1]) == -1) {
			printf(" %s: no such directory\n", args[1]);
            return -1;
		}
	}
	return 0;
}

//------------------------------------Welcome Screen----------------------------------------
void welcomeScreen(){
       
        printf("\t Opening the Interactive Shell\n");
        printf("\n\n");
}

/*
 LAUNCHING THE PROGRAM
*/ 
void launchProg(struct Node** head,struct Node** current,char **args, int background){	 
	 int err = -1;
	 int a=0;
	char line[1024]="";
	
	if(background==1){
	char token[256];
	strncpy(args[0],args[0]+1,strlen(args[0]));
	}
	
	while (args[a]){
		strcat(line,args[a]);
		strcat(line," ");
		a++;
	}
	 if((pid=fork())==-1){
		 printf("Child process could not be created\n");
		 return;
	 }
	if(pid==0){
		// set the child to ignore SIGINT signals
		signal(SIGINT, SIG_IGN);
		
		setenv("parent",getcwd(currentDirectory, 1024),1);	
		
		if (execvp(args[0],args)==err){
			printf("Command not found!!\n");
			//kill(getpid(),SIGTERM);
		}
	 }
	 else{
	 
	 if (background == 0){
		 waitpid(pid,NULL,0);
		 append(head,line,pid);
	 }else{
		 append(current,line,pid);
		 append(head,line,pid);
		 //printf("Process created with PID: %d\n",pid);
	 }}	 
}



//-----------------------------------------PARSE----------------------------
void parse(char *line, char **args){
    if (strcmp(line, "exit\n") == 0)
            exit(EXIT_SUCCESS);
        char **next = args;
        char *temp = strtok(line, " \n");
        while (temp != NULL)
        {
            *next++ = temp;
            temp = strtok(NULL, " \n");
        }
        *next = NULL;
        for (next = args; *next != 0; next++)
            puts(*next);
}
//----------------------------------------APPEND-------------------------------
void append(struct Node** head, char *new_data,int child_pid) 
{
	struct Node* new_node = (struct Node*) malloc(sizeof(struct Node)); 
	struct Node *last = *head;
  strcpy(new_node-> data , new_data);  
  new_node->pid=child_pid;
	new_node->next = NULL; 
	if (*head == NULL) 
	{ 
	*head = new_node; 
	return; 
	} 
	while (last->next != NULL) 
		last = last->next; 
	last->next = new_node; 
	return;	 
}
//---------------------------------------checking numbers----------------------------------
int digits_only(char *s)
{
    for (int i = 0; i < strlen(s); i++) {
        if ((s[i]<'0' || s[i]>'9')) return 0;
    }

    return 1;
}

//-----------------------------------------Number of nodes------------------------------------
int getCount(struct Node* head) 
{ 
    // Base case 
    if (head == NULL) 
        return 0; 
  
    // count is 1 + count of remaining list 
    return 1 + getCount(head->next); 
} 

//--------------------------------------PRINT-----------------------------------
void printdata(struct Node** head_ref,char* value) {
  struct Node* ptr = *head_ref;
  ///// pid all
  if (strcmp(value,"FULL")==0){
  printf("List of processes with PIDs spawned from this shell:(If no following output then NO Background Process!)\n");
  while(ptr) {
      printf("command name: %s  process id: %d\n",ptr->data,ptr->pid);
      ptr = ptr->next;
  }
  }
  else if (digits_only(value)==1){
    int num=atoi(value);
    int last=0; 
    int count=1;
    int com=getCount(ptr);
    while(ptr) {
      last=getCount(ptr);
      if(last<=num){
         while(ptr) {
      printf("%d %s\n",count,ptr->data);
      count=count+1;
      ptr = ptr->next;
                  } 
          break;
      }
      ptr = ptr->next;

  }
    if (last!=num){
        printf("Only %d commands were executed previously.\n",com);
    }

    }
}

//-------------------------------------FREELIST------------------------------------------------

void freeList(struct Node** head_ref)
{
   struct Node* current = *head_ref;
   struct Node* next;
 
   while (current != NULL) 
   {
       next = current->next;
       free(current);
       current = next;
   }
   *head_ref = NULL;
}

//----------------------------------STANDARD INPUT HANDLER-------------------------------------------
 
int commandHandler(char * args[]){
	int i=0,j = 0;
	int fileDescriptor,standardOut, aux;
	int background = 0;
	char line[1024]="";
	int a=0;
	int child_pid;
	while (args[a]){
		strcat(line,args[a]);
		strcat(line," ");
		a++;
	}
	char *args_aux[256];
	
	while ( args[j] != NULL){
		if ( (strcmp(args[j],">") == 0) || (strcmp(args[j],"<") == 0) || (strcmp(args[j],"&") == 0)){
			break;
		}
		args_aux[j] = args[j];
		j++;
	}
	// 'exit' command quits the shell
	if(strcmp(args[0],"stop") == 0) {
		//append(&head, "exit");
		freeList(&head);
		printf("Freed memory successfully!!\n");
		printf("Exiting Successfully!!\n");
		exit(0);
		}
	//-----------------------------------PID ALL and PID CURRENT----------------------------------
	else if (strcmp("pid",args[0])==0 && args[1]){
		if (strcmp("all",args[1])==0){
		printdata(&head,"FULL");
        }
		else if(strcmp("current",args[1])==0){
		printdata(&current,"FULL");
        }
		append(&head,line,getpid()); 
	  }
	//------------------------------------------------PID--------------------------------------------- 
	else if (strcmp("pid",args[0])==0 ){
		int child_pid;
		printf("command name: ./  process id: %d\n",getpid());
        child_pid=getpid();
		append(&head,line,getpid());
        return child_pid;   
	  }
	//--------------------------- 'pwd' command prints the current directory--------------------------
 	else if (strcmp(args[0],"pwd") == 0){
		 append(&head, "pwd",getpid());
		if (args[j] != NULL){
			if ( (strcmp(args[j],">") == 0) && (args[j+1] != NULL) ){
				fileDescriptor = open(args[j+1], O_CREAT | O_TRUNC | O_WRONLY, 0600); 
				standardOut = dup(STDOUT_FILENO); 	
				dup2(fileDescriptor, STDOUT_FILENO); 
				close(fileDescriptor);
				printf("%s\n", getcwd(currentDirectory, 1024));
				dup2(standardOut, STDOUT_FILENO);
			}
		}else{
			printf("%s\n", getcwd(currentDirectory, 1024));
		}
	} 
	//-------------------------!HISTN----------------------------------
	else if (strncmp(args[0],"!HIST",5)==0){
		strncpy(args[0],args[0]+5,strlen(args[0]));
		int num=atoi(args[0]);
		struct Node *temp=head;
		int count=1;
		while(temp){
			if (count==num){
			char new_args[MAXLINE]; // buffer for the user input
			char * tokens[LIMIT];
			memset ( new_args, '\0', MAXLINE );
			strcpy(new_args,temp->data);
			if((tokens[0] = strtok(new_args," \n\t")) == NULL) continue;
			int numTokens = 1;
			while((tokens[numTokens] = strtok(NULL, " \n\t")) != NULL) numTokens++;
				commandHandler(tokens);
				break;
			}
			count++;
		temp=temp->next;

		}

	}
	    //----------------------------------HISTN------------------------------------------
    else if (strncmp("HIST",args[0],4)==0){
          char num_check[1024];
          strncpy(num_check,args[0]+4,strlen(args[0]));
          if (digits_only(num_check)==1 && strcmp(num_check,"")!=0){
              printdata(&head,num_check);   
          }
          else{
              printf("Did you mean HISTN?\nHIST is to be used with number n, where the n commands exected previously will be shown\n");
          }
          child_pid=getpid();
            append(&head,line,child_pid);
            return child_pid;

      }
 	// --------------------------------'clear' command clears the screen-------------------------
	else if (strcmp(args[0],"clear") == 0) {
		//append(&head, "clear");
		system("clear");
	}
	// -----------------------'cd' command to change directory-----------------------------------
	else if (strcmp(args[0],"cd") == 0) {
	   append(&head, line ,getpid());
	changeDirectory(args);
	}
	//----------------------------for rest command using EXECVP---------------------------------
	else{
		while (args[i] != NULL && background == 0){
			if (strncmp(args[i],"&",1) == 0){
				background = 1;
			}
			// else if (strcmp(args[i],"|") == 0){
			// 	pipeHandler(args);
			// 	return 1;
			// }
			i++;
		}
		args_aux[i] = NULL;
		launchProg(&head,&current,args_aux,background);
	}
return 1;
}
//------------------------------delete node(for pid current in background)-----------------------------
void deleteNode(struct Node** head_ref, int key)
{
    struct Node *temp = *head_ref, *prev;
    if (temp != NULL && temp->pid == key) {
        *head_ref = temp->next; // Changed head
        free(temp); // free old head
        return;
    }
    while (temp != NULL && temp->pid != key) {
        prev = temp;
        temp = temp->next;
    }
    if (temp == NULL)
        return;

    prev->next = temp->next;
 
    free(temp); // Free memory
}
 
//--------------------------------------SIGNAL HANDLER signal handler for SIGCHLD------------------------
// void signalHandler_child(int p){
// 	int child;
// 	int status;
// 	struct Node *check = current;
// 	child=waitpid(-1, &status, WNOHANG);
// 	//printf("%d",child);
// 	while(check){
// 		if (check->pid==child){
// 		printf("The command %s with process id %d is terinated\n",check->data,check->pid);
// 		deleteNode(&current,child);
// 		break;	
// 		}
// 		check=check->next;
// 	}	
// }

//-------------------------------------Signal handler for SIGINT----------------------------------------------
void signalHandler_int(int p){
	// Sending a SIGTERM signal to the child process
	if (kill(pid,SIGTERM) == 0){
		printf("\nProcess %d received a SIGINT signal\n",pid);
		no_reprint_prmpt = 1;			
	}else{
		printf("\n");
	}
}
//-------------------------------------INTIALIZING function-------------------------------
void init(){
        GBSH_PID = getpid();  
        GBSH_IS_INTERACTIVE = isatty(STDIN_FILENO);  

		if (GBSH_IS_INTERACTIVE) {
			while (tcgetpgrp(STDIN_FILENO) != (GBSH_PGID = getpgrp()))
					kill(GBSH_PID, SIGTTIN);             
			act_child.sa_handler = signalHandler_child;
			act_int.sa_handler = signalHandler_int;			
			sigaction(SIGCHLD, &act_child, 0);
			sigaction(SIGINT, &act_int, 0);
			setpgid(GBSH_PID, GBSH_PID); 
			GBSH_PGID = getpgrp();
			if (GBSH_PID != GBSH_PGID) {
					printf("Error, the shell is not process group leader");
					exit(EXIT_FAILURE);
			}
			tcsetpgrp(STDIN_FILENO, GBSH_PGID);  
			

			tcgetattr(STDIN_FILENO, &GBSH_TMODES);
			currentDirectory = (char*) calloc(1024, sizeof(char));
        } else {
                printf("Could not make the shell interactive.\n");
                exit(EXIT_FAILURE);
        }
}
//--------------------------------------Main method-------------------------------------------- 

int main(int argc, char *argv[], char ** envp) {
	//struct Node* head = NULL; 
	char line[MAXLINE]; // buffer for the user input
	char * tokens[LIMIT]; // array for the different tokens in the command
	int numTokens;
		
	no_reprint_prmpt = 0; 	// to prevent the printing of the shell
							// after certain methods
	pid = -10;
	

	init();
	welcomeScreen();
	environ = envp;

	setenv("shell",getcwd(currentDirectory, 1024),1);

	while(TRUE){

		if (no_reprint_prmpt == 0) shellPrompt();
		no_reprint_prmpt = 0;
		
		memset ( line, '\0', MAXLINE );

		fgets(line, MAXLINE, stdin);
		//if nothing is put into the shell
		if((tokens[0] = strtok(line," \n\t")) == NULL) continue;
		//read and tokenize the command
		numTokens = 1;
		while((tokens[numTokens] = strtok(NULL, " \n\t")) != NULL) numTokens++;
		//execute the command
		commandHandler(tokens);
		
	}          

	exit(0);
}
