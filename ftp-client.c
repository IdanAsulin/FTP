//Made by Idan Asulin & Uri Elimelech

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/select.h>

#define PORT 0x0123a
#define IP_ADDR 0x7f000001
#define SIZE 4096

int main (int argc, char** argv) {
  if(strcmp(argv[1], "get-file-info") != 0 && strcmp(argv[1], "download-file") != 0) {
    printf("Wrong action");
    return 1;
  }
  int sock = socket(AF_INET, SOCK_STREAM, 0);
  struct sockaddr_in s = {0};
  s.sin_family = AF_INET;
  s.sin_port = htons(PORT);
  s.sin_addr.s_addr = htonl(IP_ADDR);
  int con = connect(sock, (struct sockaddr*)&s, (sizeof(s)));
  if(con < 0)
  {
    perror("connect error");
    return 1;
  }
  int stat_recieve;
  printf("Successfully connected \n");
  int operationLen = strlen(argv[1]);
  send(sock, &operationLen, sizeof(int), 0);
  send(sock, argv[1], operationLen, 0);
  int lenName = strlen(argv[2]);
  send(sock, &lenName, sizeof(int), 0);
  send(sock, argv[2], lenName, 0);
  if(recv(sock, &stat_recieve, sizeof(int), 0) < 0 ) {
	perror("recive");
	return 1;
      }
  if(stat_recieve == -1) {
    printf("No such file \n");
    return 0;
  }

  if(strcmp(argv[1], "download-file") == 0) {
    int fd = open(argv[2], O_WRONLY | O_CREAT | O_TRUNC, 0600);
    char buf [SIZE];
    int writed = lenName;
    while(writed > 0) {
      int recieved = recv(sock, buf, SIZE, 0);
      write(fd, buf, recieved);
      writed -= recieved;
    }
    close(fd);
  }
  if(strcmp(argv[1], "get-file-info") == 0) {
    int owner, fileSize;
    if(recv(sock, &fileSize, sizeof(int), 0) < 0) {
	perror("recive");
	return 1;
      }
    if(recv(sock, &owner, sizeof(int), 0) < 0) {
	perror("recive");
	return 1;
      }
    printf("%s - size: %d bytes, owner uid: %d\n",argv[2], fileSize, owner);
  }
  return 0;
}
