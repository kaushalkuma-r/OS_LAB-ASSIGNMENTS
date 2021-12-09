#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>

#define MAX_LEN 1000

int *commands_executed;
int *flag_double;
int *flag_single;

struct trex
{
    int filled;
    char **command;
};
struct trex *records;

void sigint_handler(int signo)
{
    printf("\nCaught SIGINT -- Interupting current running command.\n");
}

void sigint_handler_main(int signo)
{
    printf("\nCaught SIGINT -- Exiting forefully.\n");
    _exit(1);
}

/*For calculating length of interger*/
int int_len(int num)
{
    int len = 0;
    while (num)
    {
        num = num / 10;
        len++;
    }

    return len;
}

/*Function that use execvp to execute commands*/
void exec_cmd(int argCounter, char **command)
{
    pid_t pid;
    if ((pid = fork()) < (pid_t)0)
    {
        printf("Fork failed.\n");
        _exit(1);
    }
    else if (pid == (pid_t)0)
    {

        if (*flag_single || *flag_double)
        {
            printf("[~][~] Message [~][~]\n[~][~] Always use <\"> for enclosing a string and <'> inside the string.\n[~][~] <\"> are not allowed inside a string and <\'> are not allowed for enclosing a string.\n");
        }

        printf("Executing :");
        for (int i = 0; i < argCounter; i++)
        {
            printf(" %s", command[i]);
        }
        printf("\n");

        execvp(command[0], command);
        printf("[!][!][!] Please provide a valid command.\n");
        printf("\"");
        for (int i = 0; i < argCounter - 1; i++)
        {
            printf("%s ", command[i]);
        }
        printf("%s\" ", command[argCounter - 1]);
        printf("is not a valid command.\n");

        _exit(1);
    }
    else
    {
        signal(SIGINT, sigint_handler);

        wait(NULL);

        /*Saving command to history*/

        (records + *(commands_executed))->filled = argCounter;
        (records + *(commands_executed))->command = command;
        *(commands_executed) += 1;

        printf("Done with :");
        for (int i = 0; i < argCounter; i++)
        {
            printf(" %s", command[i]);
        }
        printf("\n");
    }
}

/*To count the no. of spaces or tabs*/
int spaces(char *str)
{
    int no_of_spaces = 0;
    for (int i = 0; i < strlen(str); i++)
    {
        if (str[i] == ' ' || str[i] == '\t')
        {
            no_of_spaces += 1;
        }
    }

    return no_of_spaces;
}

/*To make arguments array that will be used by execvp*/
int tokenizer(char *private_line, char **command)
{
    int counter = 0;
    char *iter;
    char *start;

    iter = private_line;
    start = private_line;

    int flag_double_private = 0;

    while (*iter != '\n' && *iter != '\0')
    {
        if (*iter == '"' && flag_double_private == 0)
        {
            flag_double_private = !flag_double_private;
            *(flag_double) = 1;
            // start += 1;
        }
        else if (*iter == '\'' && *(flag_single) == 0)
        {
            *(flag_single) = 1;
            // start += 1;
        }
        else if (*iter == '"' && flag_double_private == 1)
        {
            flag_double_private = !flag_double_private;
            start += 1;
            *iter = '\0';
            // start = private_line;
            command[counter] = start;
            counter += 1;
            start = iter + 1;
        }
        // else if (*iter == '\'' && *(flag_single) == 1)
        // {
        //     *(flag_single) = !(*flag_single);
        //     *iter = '\0';
        //     // start = private_line;
        //     command[counter] = start;
        //     counter += 1;
        //     start = iter + 1;
        // }
        else if ((*iter == ' ' || *iter == '\t') && flag_double_private != 1)
        {
            if ((int)(iter - start) == 0)
            {
                /*simply move start also by 1*/
                start += 1;
            }
            else
            {
                *iter = '\0';
                // start = private_line;
                command[counter] = start;
                counter += 1;
                start = iter + 1;
            }
        }
        else
        {
            // printf("this is not space: %c\n", *iter);
        }
        iter += 1;
    }
    if ((int)(iter - start) != 0)
    {
        // printf("herer...");
        *iter = '\0';
        // start = private_line;
        command[counter] = start;
        counter += 1;
    }
    command[counter] = NULL;

    return counter;
}

void read_cmd(char *file_name)
{
    FILE *stream;

    /* opening file for reading */
    stream = fopen(file_name, "r");
    if (stream == NULL)
    {
        printf("[!][!][!] Error : %s does not exist.\n", file_name);
        // exit(-1);
        return;
    }

    char *private_line;
    char **command;
    while (1)
    {
        private_line = (char *)malloc(sizeof(char) * MAX_LEN);

        if (fgets(private_line, MAX_LEN, stream) == NULL)
        {
            break;
        }

        int no_of_spaces = spaces(private_line);

        // /*no of arguments <= no_of_spaces+1 always*/
        command = (char **)malloc(sizeof(char *) * (no_of_spaces + 2));

        int counter = tokenizer(private_line, command);

        exec_cmd(counter, command);
    }
    fclose(stream);
}

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        printf("[!][!][!] Please provide atleast one file with the executable.\nUsage: ./shell <file1> <file2> ...\n");

        return 0;
    }

    printf("[~][~] Message [~][~]\n[~][~] Always use <\"> for enclosing a string and <'> inside the string.\n[~][~] <\"> are not allowed inside a string and <\'> are not allowed for enclosing a string.\n");

    records = (struct trex *)malloc(sizeof(struct trex) * MAX_LEN);

    commands_executed = (int *)malloc(sizeof(int));
    *(commands_executed) = 0;

    flag_double = (int *)malloc(sizeof(int));
    flag_single = (int *)malloc(sizeof(int));
    *(flag_double) = 0;
    *(flag_single) = 0;

    for (int i = 1; i < argc; i++)
    {
        printf("EXECUTING commands of <file%d> :\n", i);
        read_cmd(argv[i]);
        printf("-------------------Done with <file%d>-----------------------\n", i);
    }

    printf("-----------------------------------------------------------\n[!][!][!] Done with all the files.\n[!][!][!] Moving to interactive mode.\n");

    char *user_input;

    char **command;

    while (1)
    {
        printf("KAUSHAL@terminal : ");

        user_input = (char *)malloc(sizeof(char) * MAX_LEN);

        signal(SIGINT, sigint_handler_main);

        fgets(user_input, MAX_LEN, stdin);

        int no_of_spaces = spaces(user_input);

        command = (char **)malloc(sizeof(char *) * (no_of_spaces + 2));

        int counter = tokenizer(user_input, command);

        if (!strcmp("STOP\0", command[0]))
        {
            if (counter == 1)
            {
                break;
            }
            else
            {
                printf("The correct command to stop is : \"STOP\".\n");
            }
        }
        else if (!(strcmp("HISTORY\0", command[0])))
        {
            if (counter == 1)
            {
                printf("[!][!][!] Please choose a correct command.\nThe correct commands with HISTORY are :\n 1) HISTORY BRIEF -- For short description of command history.\n 2) HISTORY FULL  -- For full description of command history.\n");
                // printf("command executed -- %d", *(commands_executed));
                continue;
            }

            if (!(strcmp("BRIEF\0", command[1])))
            {

                if (*(commands_executed) == 0)
                {
                    printf("[!][!][!] No HISTORY to show till now.\n");
                    continue;
                }

                printf("Below are the commands executed in the current session.\n");
                for (int i = 0; i < *(commands_executed); i++)
                {
                    printf(" %d. %s\n", i + 1, ((records + i)->command[0]));
                }
            }
            else if (!strcmp("FULL\0", command[1]))
            {
                if (*(commands_executed) == 0)
                {
                    printf("[!][!][!] No HISTORY to show till now.\n");
                    continue;
                }
                printf("Below are the full-commands executed in the current session.\n");

                for (int i = 0; i < *(commands_executed); i++)
                {
                    printf(" %d.", i + 1);
                    for (int j = 0; j < records[i].filled; j++)
                    {
                        printf(" %s", (records + i)->command[j]);
                    }
                    printf("\n");
                }
            }
            else
            {
                printf("[!][!][!] Please choose a correct command.\nThe correct commands with HISTORY are :\n 1) HISTORY BRIEF -- For short description of command history.\n 2) HISTORY FULL  -- For full description of command history.\n");
                // printf("command executed -- %d", *(commands_executed));
            }
        }
        else if (!(strcmp("EXEC\0", command[0])))
        {
            if (counter == 1)
            {
                printf("[!][!][!] Please choose a correct command.\nThe correct commands with EXEC are :\n 1) EXEC <COMMAND_NAME>         -- For executing a terminal command.\n 2) EXEC <COMMAND_INDEX_NUMBER> -- For executing a command by its index in cmd_history.\n");
                // printf("command executed -- %d", *(commands_executed));
                continue;
            }

            int command_no = atoi(command[1]);

            if (command_no == 1)
            {

                if (strlen(command[1]) == int_len(command_no))
                {
                    command_no = 1;
                }
                else
                {
                    command_no = 0;
                }
            }

            if (command_no) //  if integer
            {
                if (command_no <= *(commands_executed) && command_no > 0)
                {
                    exec_cmd(records[command_no - 1].filled, records[command_no - 1].command);
                }
                else
                {
                    printf("[!][!][!] Please type a correct COMMAND_INDEX_NUMBER. The command with %d as index_no does not exist.\n", command_no);
                }
            }
            else // if string
            {

                // counter -1 because we do not need EXEC in execvp
                exec_cmd(counter - 1, command + 1);
            }
        }
        else
        {

            printf("[!][!][!] \"");
            for (int i = 0; i < counter - 1; i++)
            {
                printf("<%s> ", command[i]);
            }
            printf("<%s>", command[counter - 1]);

            printf("\" is not a valid command.\nBelow are the available valid commands :\n 1) HISTORY BRIEF               -- For short description of command history.\n 2) HISTORY FULL                -- For full description of command history.\n 3) EXEC <COMMAND_NAME>         -- For executing a terminal command.\n 4) EXEC <COMMAND_INDEX_NUMBER> -- For executing a command by its index in cmd_history.\n");
        }

        *(flag_double) = 0;
        *(flag_single) = 0;
    }

    printf("\"Exiting normally, bye.\"\n");

    // printf("%d", *(commands_executed));

    return 0;
}
