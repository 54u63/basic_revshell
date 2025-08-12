#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <errno.h>
#include <string.h>

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

int iterationLoop(int sockfd){
  char* cmd = (char*)malloc(BUFFLEN); //buffer storing the entire command line
  char* chunk = (char*)malloc(BUFFLEN); //buffer storing the chunk
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

    write(sockfd,cmd,cmdLen);

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
