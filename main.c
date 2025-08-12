#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <errno.h>


#define LPORT 4190
#define LHOST "212.227.28.186"


int main(){

  int sockfd, connfd = 0;
  struct sockaddr_in addr = {0};
  socklen_t addrlen = sizeof(addr);
  char* command = malloc(1024);

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

  close(sockfd);
  free(command);
  return 0;

}
