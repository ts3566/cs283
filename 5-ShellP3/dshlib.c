#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <signal.h>
#include "dshlib.h"

/*
 * Implement your exec_local_cmd_loop function by building a loop that prompts the 
 * user for input.  Use the SH_PROMPT constant from dshlib.h and then
 * use fgets to accept user input.
 * 
 *      while(1){
 *        printf("%s", SH_PROMPT);
 *        if (fgets(cmd_buff, ARG_MAX, stdin) == NULL){
 *           printf("\n");
 *           break;
 *        }
 *        //remove the trailing \n from cmd_buff
 *        cmd_buff[strcspn(cmd_buff,"\n")] = '\0';
 * 
 *        //IMPLEMENT THE REST OF THE REQUIREMENTS
 *      }
 * 
 *   Also, use the constants in the dshlib.h in this code.  
 *      SH_CMD_MAX              maximum buffer size for user input
 *      EXIT_CMD                constant that terminates the dsh program
 *      SH_PROMPT               the shell prompt
 *      OK                      the command was parsed properly
 *      WARN_NO_CMDS            the user command was empty
 *      ERR_TOO_MANY_COMMANDS   too many pipes used
 *      ERR_MEMORY              dynamic memory management failure
 * 
 *   errors returned
 *      OK                     No error
 *      ERR_MEMORY             Dynamic memory management failure
 *      WARN_NO_CMDS           No commands parsed
 *      ERR_TOO_MANY_COMMANDS  too many pipes used
 *   
 *   console messages
 *      CMD_WARN_NO_CMD        print on WARN_NO_CMDS
 *      CMD_ERR_PIPE_LIMIT     print on ERR_TOO_MANY_COMMANDS
 *      CMD_ERR_EXECUTE        print on execution failure of external command
 * 
 *  Standard Library Functions You Might Want To Consider Using (assignment 1+)
 *      malloc(), free(), strlen(), fgets(), strcspn(), printf()
 * 
 *  Standard Library Functions You Might Want To Consider Using (assignment 2+)
 *      fork(), execvp(), exit(), chdir()
 */

int alloc_cmd_buff(cmd_buff_t *cmd_buff) {
    cmd_buff->_cmd_buffer = (char *)malloc(SH_CMD_MAX);
    if (cmd_buff->_cmd_buffer == NULL) {
        return ERR_MEMORY;
    }
    
    clear_cmd_buff(cmd_buff);
    return OK;
}

int free_cmd_buff(cmd_buff_t *cmd_buff) {
    if (cmd_buff->_cmd_buffer) {
        free(cmd_buff->_cmd_buffer);
        cmd_buff->_cmd_buffer = NULL;
    }
    return OK;
}

int clear_cmd_buff(cmd_buff_t *cmd_buff) {
    if (cmd_buff->_cmd_buffer) {
        memset(cmd_buff->_cmd_buffer, 0, SH_CMD_MAX);
    }
    
    cmd_buff->argc = 0;
    for (int i = 0; i < CMD_ARGV_MAX; i++) {
        cmd_buff->argv[i] = NULL;
    }
    
    return OK;
}

int build_cmd_buff(char *cmd_line, cmd_buff_t *cmd_buff) {
    clear_cmd_buff(cmd_buff);
    
    if (strlen(cmd_line) == 0) {
        return WARN_NO_CMDS;
    }
    
    strcpy(cmd_buff->_cmd_buffer, cmd_line);
    
    char *p = cmd_buff->_cmd_buffer;
    bool in_quotes = false;
    int arg_count = 0;
    
    // Skip leading whitespace
    while (*p && isspace(*p)) {
        p++;
    }
    
    while (*p) {
        // Skip consecutive whitespace
        while (*p && isspace(*p)) {
            p++;
        }
        
        if (*p == '\0') {
            break;
        }
        
        // Handle quoted arguments
        if (*p == '"') {
            in_quotes = true;
            p++; // Skip the opening quote
            cmd_buff->argv[arg_count++] = p; // Start of argument
            
            // Find the closing quote
            while (*p && *p != '"') {
                p++;
            }
            
            if (*p == '"') {
                *p = '\0'; // Replace closing quote with null terminator
                p++; // Move past the closing quote
            }
            
            in_quotes = false;
        } else {
            // Handle non-quoted arguments
            cmd_buff->argv[arg_count++] = p; // Start of argument
            
            // Find the end of this argument (next whitespace)
            while (*p && !isspace(*p)) {
                p++;
            }
            
            if (*p != '\0') {
                *p = '\0'; // Replace whitespace with null terminator
                p++; // Move past the null terminator
            }
        }
        
        if (arg_count >= CMD_ARGV_MAX - 1) {
            return ERR_TOO_MANY_COMMANDS;
        }
    }
    
    cmd_buff->argv[arg_count] = NULL; // NULL-terminate the argument list
    cmd_buff->argc = arg_count;
    
    return OK;
}

Built_In_Cmds match_command(const char *input) {
    if (strcmp(input, EXIT_CMD) == 0) {
        return BI_CMD_EXIT;
    } else if (strcmp(input, "cd") == 0) {
        return BI_CMD_CD;
    }
    return BI_NOT_BI;
}

Built_In_Cmds exec_built_in_cmd(cmd_buff_t *cmd) {
    if (cmd->argc < 1) {
        return BI_NOT_BI;
    }
    
    Built_In_Cmds bi_cmd = match_command(cmd->argv[0]);
    
    switch (bi_cmd) {
        case BI_CMD_EXIT:
            return BI_CMD_EXIT;
        
        case BI_CMD_CD:
            if (cmd->argc > 1) {
                if (chdir(cmd->argv[1]) != 0) {
                    perror("cd");
                }
            }
            return BI_EXECUTED;
        default:
            return BI_NOT_BI;
    }
}

int build_cmd_list(char *cmd_line, command_list_t *clist) {
    clist->num = 0;
    bool empty_command_found = false;
    
    if (strlen(cmd_line) == 0) {
        return WARN_NO_CMDS;
    }
    
    // Check for trailing pipe at end of command
    size_t cmd_len = strlen(cmd_line);
    bool has_trailing_pipe = false;
    
    // Find the last non-whitespace character
    int last = cmd_len - 1;
    while (last >= 0 && isspace(cmd_line[last])) {
        last--;
    }
    
    // Check if it's a pipe
    if (last >= 0 && cmd_line[last] == PIPE_CHAR) {
        has_trailing_pipe = true;
        empty_command_found = true;
    }
    
    // Make a copy of the command line to work with
    char *cmd_copy = strdup(cmd_line);
    if (cmd_copy == NULL) {
        return ERR_MEMORY;
    }
    
    // Custom tokenization to respect quotes
    char *start = cmd_copy;
    char *current = cmd_copy;
    bool in_quotes = false;
    
    while (clist->num < CMD_MAX) {
        // Allocate memory for the command buffer
        if (alloc_cmd_buff(&clist->commands[clist->num]) != OK) {
            free(cmd_copy);
            return ERR_MEMORY;
        }
        
        // Find the next pipe character that's not inside quotes
        char *pipe_pos = NULL;
        current = start;
        in_quotes = false;
        
        while (*current) {
            if (*current == '"') {
                in_quotes = !in_quotes;
            } else if (*current == PIPE_CHAR && !in_quotes) {
                pipe_pos = current;
                break;
            }
            current++;
        }
        
        // If we found a pipe, replace it with null terminator temporarily
        if (pipe_pos != NULL) {
            *pipe_pos = '\0';
        }
        
        // Parse the individual command
        int rc = build_cmd_buff(start, &clist->commands[clist->num]);
        
        // Restore the pipe character if needed
        if (pipe_pos != NULL) {
            *pipe_pos = PIPE_CHAR;
        }
        
        if (rc == WARN_NO_CMDS) {
            // Found an empty command in the pipeline
            empty_command_found = true;
            free_cmd_buff(&clist->commands[clist->num]);
        } else if (rc != OK) {
            free(cmd_copy);
            return rc;
        } else {
            // Only increment command count if the command is not empty
            clist->num++;
        }
        
        // If we didn't find a pipe or reached the end, we're done
        if (pipe_pos == NULL) {
            break;
        }
        
        // Check for empty commands (consecutive pipes)
        if (*(pipe_pos + 1) == PIPE_CHAR || 
            (*(pipe_pos + 1) == '\0') || 
            (strlen(pipe_pos + 1) == strspn(pipe_pos + 1, " \t\n"))) {
            empty_command_found = true;
        }
        
        // Move start to the character after the pipe
        start = pipe_pos + 1;
    }
    
    free(cmd_copy);
    
    // Check if we hit the pipe limit
    if (*start != '\0' && clist->num >= CMD_MAX) {
        return ERR_TOO_MANY_COMMANDS;
    }
    
    // Check if we have any commands
    if (clist->num == 0) {
        return WARN_NO_CMDS;
    }
    
    // If we found empty commands in the pipeline or a trailing pipe, print a warning
    if (empty_command_found || has_trailing_pipe) {
        printf("%s", CMD_WARN_NO_CMD);
    }
    
    return OK;
}

int free_cmd_list(command_list_t *cmd_lst) {
    for (int i = 0; i < cmd_lst->num; i++) {
        free_cmd_buff(&cmd_lst->commands[i]);
    }
    cmd_lst->num = 0;
    return OK;
}

// Add this function to handle file redirection parsing
int handle_redirection(cmd_buff_t *cmd) {
    if (cmd->argc < 1) {
        return OK;
    }
    
    for (int i = 0; i < cmd->argc; i++) {
        if (strcmp(cmd->argv[i], ">") == 0) {
            // Found a redirection operator
            if (i + 1 >= cmd->argc) {
                // No filename provided after >
                fprintf(stderr, "Redirection error: No filename provided\n");
                return ERR_CMD_ARGS_BAD;
            }
            
            // Save the filename
            char *filename = cmd->argv[i + 1];
            
            // Null-terminate the command arguments at the redirection operator
            cmd->argv[i] = NULL;
            cmd->argc = i;
            
            // Return the filename for redirection
            return i + 1;  // Return the index of the filename
        }
    }
    
    return 0;  // No redirection found
}

// Update execute_pipeline function to handle redirections
int execute_pipeline(command_list_t *clist) {
    if (clist->num == 0) {
        return WARN_NO_CMDS;
    }
    
    // Check for redirections in the last command
    int redirection_index = handle_redirection(&clist->commands[clist->num - 1]);
    char *output_file = NULL;
    
    if (redirection_index > 0) {
        output_file = clist->commands[clist->num - 1].argv[redirection_index];
    }
    
    // If there's only one command, no need for pipes
    if (clist->num == 1) {
        Built_In_Cmds bi_cmd = exec_built_in_cmd(&clist->commands[0]);
        if (bi_cmd == BI_CMD_EXIT) {
            printf("exiting...\n");
            return OK_EXIT;
        } else if (bi_cmd == BI_EXECUTED) {
            return OK;
        }
        
        pid_t pid = fork();
        if (pid < 0) {
            perror("fork");
            return ERR_EXEC_CMD;
        } else if (pid == 0) {
            // Child process
            
            // Handle output redirection if needed
            if (output_file != NULL) {
                int fd = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                if (fd < 0) {
                    perror("open");
                    exit(ERR_EXEC_CMD);
                }
                
                if (dup2(fd, STDOUT_FILENO) < 0) {
                    perror("dup2");
                    close(fd);
                    exit(ERR_EXEC_CMD);
                }
                
                close(fd);
            }
            
            execvp(clist->commands[0].argv[0], clist->commands[0].argv);
            perror(clist->commands[0].argv[0]);
            exit(ERR_EXEC_CMD);
        } else {
            // Parent process
            int status;
            waitpid(pid, &status, 0);
            return OK;
        }
    }
    
    // For multiple commands, create one pipe per connection
    int pipes[CMD_MAX-1][2]; // Array of pipe file descriptors
    
    // Create all pipes
    for (int i = 0; i < clist->num - 1; i++) {
        if (pipe(pipes[i]) == -1) {
            perror("pipe");
            // Close any already created pipes
            for (int j = 0; j < i; j++) {
                close(pipes[j][0]);
                close(pipes[j][1]);
            }
            return ERR_EXEC_CMD;
        }
    }
    
    // Fork all child processes
    pid_t pids[CMD_MAX];
    
    for (int i = 0; i < clist->num; i++) {
        // Special handling for built-in commands (only for first command)
        if (i == 0) {
            Built_In_Cmds bi_cmd = exec_built_in_cmd(&clist->commands[i]);
            if (bi_cmd == BI_CMD_EXIT) {
                printf("exiting...\n");
                // Close all pipes
                for (int j = 0; j < clist->num - 1; j++) {
                    close(pipes[j][0]);
                    close(pipes[j][1]);
                }
                return OK_EXIT;
            } else if (bi_cmd == BI_EXECUTED) {
                // We don't support built-in commands in pipes yet, so we'll just skip this
                // and continue with the rest of the pipeline
                continue;
            }
        }
        
        pids[i] = fork();
        if (pids[i] < 0) {
            perror("fork");
            
            // Close all pipes
            for (int j = 0; j < clist->num - 1; j++) {
                close(pipes[j][0]);
                close(pipes[j][1]);
            }
            
            // Kill any child processes already created
            for (int j = 0; j < i; j++) {
                if (pids[j] > 0) {
                    kill(pids[j], SIGTERM);
                }
            }
            
            return ERR_EXEC_CMD;
        } else if (pids[i] == 0) {
            // Child process
            
            // Connect stdin to the previous pipe's read end (if not first command)
            if (i > 0) {
                if (dup2(pipes[i-1][0], STDIN_FILENO) == -1) {
                    perror("dup2 stdin");
                    exit(ERR_EXEC_CMD);
                }
            }
            
            // Connect stdout to the next pipe's write end (if not last command)
            if (i < clist->num - 1) {
                if (dup2(pipes[i][1], STDOUT_FILENO) == -1) {
                    perror("dup2 stdout");
                    exit(ERR_EXEC_CMD);
                }
            } 
            // Set up file redirection for the last command if needed
            else if (output_file != NULL) {
                int fd = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                if (fd < 0) {
                    perror("open");
                    exit(ERR_EXEC_CMD);
                }
                
                if (dup2(fd, STDOUT_FILENO) < 0) {
                    perror("dup2");
                    close(fd);
                    exit(ERR_EXEC_CMD);
                }
                
                close(fd);
            }
            
            // Close all pipe file descriptors in the child
            for (int j = 0; j < clist->num - 1; j++) {
                close(pipes[j][0]);
                close(pipes[j][1]);
            }
            
            // Execute the command
            execvp(clist->commands[i].argv[0], clist->commands[i].argv);
            perror(clist->commands[i].argv[0]);
            exit(ERR_EXEC_CMD);
        }
    }
    
    // In the parent, close all pipe file descriptors
    for (int i = 0; i < clist->num - 1; i++) {
        close(pipes[i][0]);
        close(pipes[i][1]);
    }
    
    // Wait for all child processes
    for (int i = 0; i < clist->num; i++) {
        if (pids[i] > 0) {
            waitpid(pids[i], NULL, 0);
        }
    }
    
    return OK;
}

int exec_local_cmd_loop()
{
    char *cmd_buff;
    int rc = 0;
    command_list_t cmd_list;

    cmd_buff = (char *)malloc(SH_CMD_MAX);
    if (cmd_buff == NULL) {
        return ERR_MEMORY;
    }

    while(1) {
        printf("%s", SH_PROMPT);
        if (fgets(cmd_buff, ARG_MAX, stdin) == NULL) {
            printf("\n");
            break;
        }
        
        cmd_buff[strcspn(cmd_buff, "\n")] = '\0';
        
        // Initialize command list with zero commands
        cmd_list.num = 0;
        
        // Parse the command line into a list of piped commands
        rc = build_cmd_list(cmd_buff, &cmd_list);
        if (rc != OK) {
            if (rc == WARN_NO_CMDS) {
                printf("%s", CMD_WARN_NO_CMD);
                continue;
            } else if (rc == ERR_TOO_MANY_COMMANDS) {
                printf(CMD_ERR_PIPE_LIMIT, CMD_MAX);
                continue;
            }
            continue;
        }
        
        // Execute the command pipeline
        rc = execute_pipeline(&cmd_list);
        if (rc == OK_EXIT) {
            free_cmd_list(&cmd_list);
            break;
        }
        
        // Free resources for this command
        free_cmd_list(&cmd_list);
    }

    free(cmd_buff);
    
    return OK;
}
