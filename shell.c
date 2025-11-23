#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

#define MAX_INPUT_SIZE 1024

int main()
{
    char input[MAX_INPUT_SIZE];
    char cwd[MAX_INPUT_SIZE];

    while (1)
    {
        char *token;
        int k;
        char *commands[100];
        int num_commands = 0;
        char hostname[50];
        char *username = getlogin();

        if (username == NULL)
        {
            perror("Error getting username");
            exit(EXIT_FAILURE);
        }

        if (gethostname(hostname, 50) != 0)
        {
            perror("Error getting hostname");
            exit(EXIT_FAILURE);
        }

        printf("%s@%s", username, hostname);
        printf(":%s$ ", getcwd(cwd, sizeof(cwd)));
        fgets(input, MAX_INPUT_SIZE, stdin);
        input[strcspn(input, "\n")] = '\0';

        token = strtok(input, ";"); /*Split input by semicolon to manage multiple commands*/
        while (token != NULL)
        {
            commands[num_commands++] = token;
            token = strtok(NULL, ";");
        }

        for (k = 0; k < num_commands; ++k)
        {
            int pipes_location[30];
            int i = 0;
            int v = 0;
            int j = 0;
            int pipes_count = 0;
            char *args_for_exec[100][100]; /* rows are the number of the fd - 1 , columns are the arguments */

            char *args[100];
            args[0] = strtok(commands[k], " \n");
            while (args[i] != NULL)
            {
                ++i;
                args[i] = strtok(NULL, " \n");
            }

            if (strcmp(args[0], "quit") == 0)
            {
                exit(EXIT_SUCCESS);
            }

            i = 0;

            while (args[i] != NULL)
            {
                if (strcmp(args[i], "|") == 0)
                {
                    pipes_location[pipes_count] = i;
                    pipes_count++;
                }
                i++;
            }

            /* Pipe implementation */
            if (pipes_count > 0)
            {
                pid_t *child = malloc(pipes_count * sizeof(pid_t));
                int(*fd)[2] = malloc(pipes_count * sizeof(int[2]));

                pid_t pipe_pid = fork();
                if (pipe_pid == 0)
                {
                    for (i = 0; args[i] != NULL; i++)
                    {
                        if (*args[i] == '|')
                        {
                            pipes_location[v] = i;
                            v++;
                            j = 0;
                        }
                        else
                        {
                            args_for_exec[v][j] = args[i];
                            j++;
                        }
                    }

                    for (i = 0; i < pipes_count; i++)
                    {
                        if (pipe(fd[i]) == -1)
                        {
                            perror("fd failed");
                            exit(EXIT_FAILURE);
                        }
                    }

                    for (i = 0; i < pipes_count; i++)
                    {
                        if ((child[i] = fork()) == -1)
                        {
                            perror("Fork failed");
                            exit(EXIT_FAILURE);
                        }

                        if (child[i] == 0)
                        {
                            if (i == 0)
                            {
                                dup2(fd[i][1], STDOUT_FILENO);
                            }
                            else
                            {
                                dup2(fd[i - 1][0], STDIN_FILENO);
                                dup2(fd[i][1], STDOUT_FILENO);
                            }

                            for (j = 0; j < pipes_count; j++)
                            {
                                close(fd[j][0]);
                                close(fd[j][1]);
                            }
                            execvp(args_for_exec[i][0], args_for_exec[i]);
                            perror("Exec failed");
                            exit(EXIT_FAILURE);
                        }
                    }

                    for (i = 0; i < pipes_count; i++)
                    {
                        close(fd[i][1]);
                        if (i == pipes_count - 1)
                        {
                            dup2(fd[i][0], STDIN_FILENO);
                        }
                        close(fd[i][0]);
                    }

                    execvp(args_for_exec[pipes_count][0], args_for_exec[pipes_count]);
                    perror("Exec failed");
                    exit(EXIT_FAILURE);
                }
                else if (pipe_pid > 0)
                {
                    waitpid(pipe_pid, NULL, 0);
                }
                else
                {
                    perror("shell");
                }
            }
            else /* No pipes, handle redirections and execute command */
            {
                pid_t red_pid = fork();

                if (red_pid == 0)
                {
                    int input_redirect = -1;
                    int output_redirect = -1;
                    int append_output_redirect = -1;

                    /* Check for input and output redirection operators */
                    for (j = 0; args[j] != NULL; j++)
                    {
                        if (strcmp(args[j], "<") == 0)
                        {
                            input_redirect = j;
                            args[j] = NULL; /* Remove the redirection operator from the args list */
                        }
                        else if (strcmp(args[j], ">") == 0)
                        {
                            output_redirect = j;
                            args[j] = NULL;
                        }
                        else if (strcmp(args[j], ">>") == 0)
                        {
                            append_output_redirect = j;
                            args[j] = NULL;
                        }
                    }

                    if (input_redirect != -1)
                    {
                        int input_file = open(args[input_redirect + 1], O_RDONLY);
                        if (input_file == -1)
                        {
                            perror("Error opening input file");
                            exit(EXIT_FAILURE);
                        }
                        if (dup2(input_file, STDIN_FILENO) == -1)
                        {
                            perror("Error redirecting input");
                            exit(EXIT_FAILURE);
                        }
                        close(input_file);
                    }

                    if (output_redirect != -1)
                    {
                        int output_file =
                            open(args[output_redirect + 1],
                                 O_WRONLY | O_CREAT | O_TRUNC, 0644);
                        if (output_file == -1)
                        {
                            perror("Error opening output file");
                            exit(EXIT_FAILURE);
                        }
                        if (dup2(output_file, STDOUT_FILENO) == -1)
                        {
                            perror("Error duplicating file descriptor");
                            exit(EXIT_FAILURE);
                        }
                        close(output_file);
                    }
                    else if (append_output_redirect != -1)
                    {
                        int output_file =
                            open(args[append_output_redirect + 1],
                                 O_WRONLY | O_CREAT | O_APPEND, 0644);
                        if (output_file == -1)
                        {
                            perror("Error opening output file");
                            exit(EXIT_FAILURE);
                        }
                        if (dup2(output_file, STDOUT_FILENO) == -1)
                        {
                            perror("Error duplicating file descriptor");
                            exit(EXIT_FAILURE);
                        }
                        close(output_file);
                    }

                    if (execvp(args[0], args) == -1)
                    {
                        if (strcmp(args[0], "cd") != 0)
                            perror("shell");
                        exit(EXIT_FAILURE);
                    }
                }
                else if (red_pid > 0)
                {
                    if (strcmp(args[0], "cd") == 0)
                    {
                        chdir(args[1]);
                    }

                    waitpid(red_pid, NULL, 0);
                }
                else
                {
                    perror("shell");
                }
            }
        }
    }

    return 0;
}