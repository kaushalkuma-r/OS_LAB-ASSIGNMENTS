#include <stdio.h>
#include<stdlib.h>
#include<sys/wait.h>
#include<unistd.h>
#include<string.h>
#include<signal.h>

// Here I am using the concept of linked list to design the structure that will store our arguments and their addresses.
struct LL{
    char** arguments; 
    int argSize; 
    pid_t pid; 
    int running;
    struct LL* next;
    struct LL* prev;
};
// As we are going to need our linked list for the whole program so it is better to declare the linked list globally instead of declaring it inside a function.
struct LL* topCurrent= NULL;
struct LL* top = NULL;
struct LL* tail = NULL;

// This function will store every command that is being executed.
void push(char** arguments, int argSize, pid_t pid){
    struct LL* current = top;
    struct LL* newLL = malloc(sizeof(struct LL)); 
    newLL->argSize = argSize;
    newLL->pid = pid;
    newLL->arguments = (char **) malloc(sizeof(char *)*(argSize+1));
    for(int i=0;i<argSize;i++){
        newLL->arguments[i] = (char *)malloc(sizeof(char)*sizeof(strlen(arguments[i])+1));
        strcpy(newLL->arguments[i], arguments[i]);
    }
    newLL->next = NULL;
    if(current == NULL){
        top = newLL;
        tail = newLL;
    }
    else{
        tail->next = newLL;
        newLL->prev = tail;
        tail = newLL;
    }
}
// This is the helper function to find the command on the basis of position  of the command in the linked list.
struct LL* countCommand(int x)
{
    struct LL* currLL = tail;
    int count = 1;
    while(currLL != NULL && count!=x)
    {
        currLL = currLL->prev;
        count++;
    }
    return currLL;
}

// Function that will free the space acquired by a LL in case if it is deleted.
void deleteLL(pid_t pid)
{
    struct LL *currLL = topCurrent;
    struct LL* prev = NULL;
    // if head is deleted, then the next LL has to become the head.
    if(currLL!=NULL && currLL->pid == pid)
    {
        topCurrent = topCurrent->next;
        free(currLL);
        return;
    }
    // IF a LL in the middle is deleted, then this part will be needed.
    else{
        while(currLL != NULL && currLL->pid != pid)
        {
            prev = currLL;
            currLL = currLL->next;
        }
        if(currLL == NULL) return;
        prev->next = currLL->next;
        free(currLL);
    }
}
// This function will store the commands that are currently running along with their process id.
void pushCurrent(char** arguments, int argSize, pid_t pid)
{
    struct LL* current = topCurrent;
    struct LL* newLL = malloc(sizeof(struct LL));
    newLL->argSize = argSize;
    newLL->pid = pid;
    newLL->running = 1;
    newLL->arguments = (char **) malloc(sizeof(char *)*(argSize+1));
    for(int i=0;i<argSize;i++){
        newLL->arguments[i] = (char *)malloc(sizeof(char)*sizeof(strlen(arguments[i])+1));
        strcpy(newLL->arguments[i], arguments[i]);
    }
    newLL->next = NULL;
    if(current == NULL){
        topCurrent = newLL;
    }
    else{
    while (current->next != NULL) {
        current = current->next;
    }
    current->next = newLL;
    }
}

//This is helper function that will help us to search for a command on the basis of the arguments.
struct LL* searchCommand(char* command)
{
    struct LL* currLL = top;
    while(currLL != NULL && (strcmp((currLL->arguments)[0], command) != 0))
    {
        currLL = currLL->next;
    }
    return currLL;
}



// Function that will print all the commands executed up till now.
void displayFull(int numberOfCommandsINT){
    if(tail == NULL){
        printf("NO history commands!!\n");
    }else{
        struct LL* currLL = tail;
        int commandNumber = 1;
        while(currLL != NULL && commandNumber<=numberOfCommandsINT){
            printf("%d. ",commandNumber);
            for(int i=0;i<currLL->argSize;i++){
                printf("%s ", (currLL->arguments)[i]);
            }
            printf("\n");
            currLL = currLL->prev;
            commandNumber++;
        }
    }
}

//This is the function for the History BRIEF command so it will only prevent .
void displayBrief(){
    if(top == NULL){
        printf("NO processes spawned till now!!\n");
    }else{
        struct LL* currLL = top;
        int commandNumber = 1;
        while(currLL != NULL){
            printf("%d.command name:%s    ",commandNumber, (currLL->arguments)[0]);
            printf("process id:%d\n",currLL->pid);
            currLL = currLL->next;
            commandNumber++;
        }
    }
}

void displayCurrent()
{
    // display all current processes by traversing each node of current process
   struct LL* currLL = topCurrent;
   int commandNumber = 1;
    while(currLL != NULL){
        // print the process if it is currently running
        if(currLL->running == 1){
        printf("%d.command name: ",commandNumber);
        printf("%s    ", (currLL->arguments)[0]);
        printf("process id: %d\n", currLL->pid);
        commandNumber++;
        }
        currLL = currLL->next;   
    }
    if(commandNumber == 1)
    {
        // if no current processes are running 
        printf("No current processess running\n");
    }
    return;
}

// Function to print the command in specific format
void printCommand(char** arguments){
    printf("Executing command \"");
    int i = 0;
    while((arguments)[i+1] != NULL)
    {
        printf("%s ", (arguments)[i]);
        i++;
    }
    printf("%s\"\n", (arguments)[i]);
}

//This function will execute the command using exec function.
void executeCommand(char** args, int background, int argSize)
{
    pid_t  pid;
    int    status;
    pid = fork();
    if (pid == 0) {
        // If Child Process
        if(background == 1){
            // set a new process group id to run process in Background
            int pgidResult = setpgid(0,0);
            if(pgidResult == -1){
                printf("** Background process spawned failed!! **\n");
                exit(1);
            }
        }
        int status = execvp(args[0], args);
        if(status < 0)
        {
            // Exec command failed
            printf("** Invalid command **\n");
            exit(1);
        }
    }
    else if(pid == -1){
        // fork failed
       printf("** Fork failed **\n");
       exit(1);
    }
    else{
        // parent process
        if(background == 1){
            args[argSize]="&";
            args[argSize+1]=NULL;
            argSize++;
        }
        push(args,argSize,pid);
        if(background!=1) waitpid(pid,&status,0);
        else{
            pushCurrent(args,argSize,pid);
            printf("Process running with pid %d running in background\n", pid);
        }
    }
}

// helper function to process command and split into arguments
void processCommand(char* command, char** arguments, int* i)
{
    /* Args -> command string, a pointer to an array of arguments 
       Stores each args in the argument array terminated by NULL character */
    int j = 0;
    char *arg = strtok(command, " ");
    while (arg != NULL)
    {
        arguments[j] = arg;
        arg = strtok(NULL, " ");
        j++;
    }
    arguments[j] = NULL;
    *i = j;
}

// helper function to free history LinkList
void freeHistory()
{
    // traverse each LL node 
    printf("Clearing History Memory....\n");
    struct LL* currLL = top;
    struct LL* nextLL = top;
    while(currLL != NULL)
    {   
        nextLL = currLL->next;
        for(int i=0;i<currLL->argSize;i++){
            // free memory for each argument
            free((currLL->arguments)[i]);
        }
        // free the argument pointer
        free(currLL->arguments);
        // free the LL
        free(currLL);
        currLL = nextLL;
    }
    free(nextLL);
    printf("Memory Cleared Successfully!!\n");
}

// for handling Ctrl-C events
void handle_sigint(int sig) 
{
    printf("Process Interupted!!\n"); 
}

void removeKilledProcess()
{
    /* Function routine to remove all killed processes from Current List
       for freeing up memory */
    struct LL* currLL = topCurrent;
    while(currLL != NULL)
    {
        if(currLL->running==0)
        {
            deleteLL(currLL->pid);
        }
        currLL = currLL->next;
    }
    return;
}

void handle_childkills(int sig){
    /* Function to handle SIGCHLD signals, 
       Checks status of for each process and sets the running status to 0*/
    //printf("Caught SigChld\n");
    pid_t pid1;
    int status;
    struct LL* currLL = topCurrent;
    while (currLL!=NULL)
    {
        pid1 = waitpid(currLL->pid,&status,WNOHANG);
        if(pid1 == -1)
        {
            printf("\nChild process was terminated with pid %d \n", currLL->pid);
            currLL->running = 0;
        }else if(pid1!=0 && (WIFEXITED(status) || WIFSIGNALED(status) || WCOREDUMP(status))){
            printf("\nChild process was terminated with pid %d \n", currLL->pid);
            currLL->running = 0;
            delete(currLL->running);
        }
        currLL = currLL->next;
    }
    return;
}
size_t size = 1024;

void update(char* string,char* sub,char* new_str)
{
    // replace substring for changing absolute path
    int stringLen,subLen,newLen;
    int i=0,j,k;
    int flag=0,start,end;
    stringLen=strlen(string);
    subLen=strlen(sub);
    newLen=strlen(new_str);

    for(i=0;i<stringLen;i++)
    {
        flag=0;
        start=i;
        for(j=0;string[i]==sub[j];j++,i++)
            if(j==subLen-1) flag=1;
            end=i;
            if(flag==0) i-=j;
            else
            {
                for(j=start;j<end;j++)
                {
                    for(k=start;k<stringLen;k++) string[k]=string[k+1];
                    stringLen--;
                    i--;
                }

                for(j=start;j<start+newLen;j++)
                {
                    for(k=stringLen;k>=j;k--) string[k+1]=string[k];
                    string[j]=new_str[j-start];
                    stringLen++;
                    i++;
                }
            }
    }
}

// updatePath the path on calling cd
void updatePath(char* pwd, char* homeDir){
    pwd = getcwd(pwd, size);
    update(pwd,homeDir,"~");
}

// to remove tabs from input buffer
void replaceTabs(char* buf)
{
    int n = strlen(buf);
    for(int i=0;i<n;i++)
    {
        if(buf[i]=='\t' || buf[i] == '\r') buf[i]=' ';
    }
}
void rstrip(char* buf)
{
    int n = strlen(buf);
    int i;
    for(i=(n-1); i>=0; i--)
    {
        if(buf[i]==' ' || buf[i]=='\n') continue;
        else break;
    }
    buf[i+2] = '\0';
}
//This function will execute the command that will be entered by the user and will handle the commands such as HISTORY FULL,HISTORY BRIEF. 

int executeCommandLine(char* buf, char* pwd, char* homeDir, char* procName){
    
    replaceTabs(buf);
    rstrip(buf);
    if(strcmp(buf,"\n")==0)
    {
        // handle enters
        return 1;
    }
        int n = strlen(buf);
        // replace the last character by EOF
        buf[n-1]='\0';
        char* bufCopy = malloc((n+1)*sizeof(char));
        strcpy(bufCopy, buf);
        char* option = strtok(buf," ");
        char* subOption = strtok(NULL," ");
        if(option == NULL){
            return 1;
        }
        if(strcmp(option,"STOP") == 0){
            return -1;
        }
        else if(strcmp(option,"HISTORY") == 0){
            if(strcmp(subOption,"FULL") == 0)
            {
                //"Execute full option"
                displayFull(10);
            }
            else if(strcmp(subOption,"BRIEF") == 0)
            {
                //"Execute brief history"
                displayBrief();
            }else{
                printf("Wrong arguments provided!!\n");
                return 0;
            }
        }
        else if(strcmp("pid",option) == 0){
            char** arguments = (char **)malloc(sizeof(char *)*strlen(bufCopy));
            int i;
            processCommand(bufCopy, arguments, &i);
            push(arguments, i, getpid());

            free(arguments);
            if(subOption == NULL)
            {
                printf("command name: %s    process id: %d\n", procName, getpid());
                return 0;
            }
            else if(strcmp("current", subOption) == 0)
            {
                displayCurrent();
                return 0;
            }
            else if(strcmp("all", subOption) == 0)
            {
                displayBrief();
                return 0;
            }
            else{
                printf("Wrong arguments provided!!\n");
                return 0;
            }
        }
        else if(strcmp("cd",option) == 0){
            // handle cd command
            char** arguments = (char **)malloc(sizeof(char *)*strlen(bufCopy));
            int i;
            processCommand(bufCopy, arguments, &i);
            push(arguments, i, getpid());

            free(arguments);
            int dirResult;
            if(strcmp("~", subOption) == 0){
                dirResult = chdir(homeDir);
            }else dirResult = chdir(subOption);
            // if path not found
            if(dirResult<0){
                perror("The given path cannot be accessed.");
                return 0;
            }else{
                updatePath(pwd, homeDir);
                printf("The working directory was changed to %s\n", pwd);
            }
        }
        else if(strncmp("HIST",option,4) == 0)
        {
            // print last n commands
            char* numberOfCommands = (char *) malloc(sizeof(char)*(strlen(option)-3));
            strcpy(numberOfCommands,bufCopy+4);
            int numberOfCommandsINT = atoi(numberOfCommands);
            if(numberOfCommandsINT == 0 && (strcmp("0",numberOfCommands) == 0)) return 0;
            else if(numberOfCommandsINT == 0)
            {
                // error in providing number of commands
                printf("Please provide the command in the format as HIST3\n");
                return 0;
            }else{
                // non zero number of commands
                displayFull(numberOfCommandsINT);
                return 0;
            }
        }
        else if(strncmp("!HIST",option,5) == 0){
            // execute command with index number
            char* numberOfCommands = (char *) malloc(sizeof(char)*(strlen(option)-3));
            strcpy(numberOfCommands,bufCopy+5);
            int numberOfCommandsINT = atoi(numberOfCommands);
            if(numberOfCommandsINT == 0)
            {
                printf("Please specify correct number from 1-N\n");
                return 0;
            }else{
                struct LL* command = countCommand(numberOfCommandsINT);
                if(command == NULL)
                {
                    // command not found
                    printf("No command was found on the given index.\n");
                    return 0;
                }else{
                    // command found, first reconstruct the command and recall the function
                    int totSize = 0;
                    for(int i=0;i<command->argSize;i++)
                    {
                        totSize += strlen(command->arguments[i]);
                        totSize += strlen(" ");
                    }
                    char* fullCommand = (char *)malloc(sizeof(char)*(totSize+1));

                    for(int i=0;i<command->argSize;i++)
                    {
                        strcat(fullCommand,command->arguments[i]);
                        strcat(fullCommand," ");
                    }
                    return executeCommandLine(fullCommand, pwd, homeDir, procName);
                }
            }
        }
        else{
            // a shell command is executed
            int background;
            if(bufCopy[n-2]=='&')
            {
                // if background process
                background = 1;
                bufCopy[n-2]='\0';
            }else{
                background = 0;
            }
            char** arguments = (char **)malloc(sizeof(char *)*strlen(bufCopy));
            int i;
            processCommand(bufCopy, arguments, &i);
            executeCommand(arguments, background, i);

            free(arguments);
        }
        printf("\n");
        free(buf); // free the buffer
        free(bufCopy); // free the copy of buffer allocated
        return 0;
}
// This is the function to kill all the currenty running background processes.
void killAllCurrent()
{
    struct LL* currLL = topCurrent;
    if(currLL != NULL){
        printf("The background processes that were currently running are being killed now.\n");
    }
    while(currLL != NULL)
    {
        kill(currLL->pid, SIGKILL);
        currLL = currLL->next;
    }
}


int main(int argc, char* argv[])
{
    printf("\n");
    signal(SIGINT, handle_sigint);
    signal(SIGCHLD, handle_childkills);
    char* hostName = (char *)malloc(sizeof(char)*size);
    char* userName = (char *)malloc(sizeof(char)*size);
    char* defaultHostName = "DefaultHostName";
    char* defaultUserName = "DefaultUserName";
    char* homeDir = (char *)malloc(sizeof(char)*size);

    // store current homeDir 
    homeDir = getcwd(homeDir, size);
    char* pwd = (char *)malloc(sizeof(char)*size);
    int result;
    // get hostName
    result = gethostname(hostName,size);

    if(result<0)
    {   
        // if hostName cant be fetched
        strcpy(hostName, defaultHostName);
    }

    result = getlogin_r(userName, size);
    if(result<0)
    {
        // if userName cant be fetched
        strcpy(userName, defaultUserName);
    }
    updatePath(pwd, homeDir);
    while(1){
        removeKilledProcess();
        printf("Kaushal");
        printf("<%s@%s:%s>",userName,hostName,pwd);
        // again allocate a memory buffer
        char* buf;
        buf = (char *)malloc(sizeof(char)*1024);
        int endofFile = getline(&buf, &size, stdin); 
        if(endofFile == -1)
        {
            // handling ctrl D
            printf("\n\nTerminating the program.\n\n");
            killAllCurrent();
            exit(1);
        }
        // execute the command 
        int exitStatus = executeCommandLine(buf,pwd,homeDir,argv[0]);
        if(exitStatus == -1) break;
    }
    // free the history
    freeHistory();
    printf("Exiting normally,bye!\n");
    // kill all current processes
    killAllCurrent();
    return 0;
}
