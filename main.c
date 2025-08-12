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
  
  while(!end){
    bRead = read(sockfd,chunk,BUFFLEN);
    if(bRead == 0){
      perror("Nothing was recieved\n");
      out+=1;
      goto _GracefullEnd;
    }
    if(strcmp(chunk,"STOP\n")==0){
      end+=1;
    }
    memcpy(cmd,chunk,bRead);
    while(checkEnd(chunk,bRead)){
      printf("wee need to fill the buffer\n");
      break;
    }
    printf("%s <- cmd\n",cmd);
    memset(cmd,'\0',BUFFLEN);
  }
  printf("Client has been Gracefully ShutDown\n");

_GracefullEnd:
  free(chunk);
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
