#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
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
    
    while (*p) {
        while (*p && isspace(*p)) {
            p++;
        }
        if (*p == '\0') {
            break;
        }
        
        if (*p == '"') {
            in_quotes = true;
            p++;
        }
        
        cmd_buff->argv[arg_count++] = p;
        
        while (*p) {
            if (*p == '"') {
                in_quotes = false;
                *p = '\0';
                p++;
                break;
            } else if (!in_quotes && isspace(*p)) {
                *p = '\0';
                p++;
                break;
            }
            p++;
        }
        
        if (arg_count >= CMD_ARGV_MAX - 1) {
            return ERR_TOO_MANY_COMMANDS;
        }
    }
    
    cmd_buff->argv[arg_count] = NULL;
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

int exec_local_cmd_loop()
{
    char *cmd_buff;
    int rc = 0;
    cmd_buff_t cmd;

    cmd_buff = (char *)malloc(SH_CMD_MAX);
    if (cmd_buff == NULL) {
        return ERR_MEMORY;
    }

    if (alloc_cmd_buff(&cmd) != OK) {
        free(cmd_buff);
        return ERR_MEMORY;
    }

    while(1) {
        printf("%s", SH_PROMPT);
        if (fgets(cmd_buff, ARG_MAX, stdin) == NULL) {
            printf("\n");
            break;
        }
        
        cmd_buff[strcspn(cmd_buff, "\n")] = '\0';
        
        rc = build_cmd_buff(cmd_buff, &cmd);
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
        
        if (cmd.argc > 0) {
            Built_In_Cmds bi_cmd = match_command(cmd.argv[0]);
            
            //exit
            if (bi_cmd == BI_CMD_EXIT) {
                break;
            }
            
            //cd
            if (bi_cmd == BI_CMD_CD) {
                if (cmd.argc > 1) {
                    if (chdir(cmd.argv[1]) != 0) {
                        perror("cd");
                    }
                }
                continue;
            }
            
            pid_t pid = fork();
            if (pid < 0) {
                perror("fork");
                continue;
            } else if (pid == 0) {
                execvp(cmd.argv[0], cmd.argv);
                perror(cmd.argv[0]);
                exit(ERR_EXEC_CMD);
            } else {
                int status;
                wait(&status);
            }
        }
        
        clear_cmd_buff(&cmd);
    }

    free_cmd_buff(&cmd);
    free(cmd_buff);
    
    return OK;
}
