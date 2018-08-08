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
#define QUEUE_LEN 20
#define SIZE 4096

int main (int argc, char** argv) {
  int listener = socket(AF_INET, SOCK_STREAM, 0);
  if (listener < 0) {
    perror("socket");
    return 1;
  }
  struct sockaddr_in s = {0};
  s.sin_family = AF_INET;
  s.sin_port = htons(PORT);
  s.sin_addr.s_addr = htonl(IP_ADDR);
  int status = bind(listener,(struct sockaddr*)&s, sizeof(s));
  if(status < 0) {
    perror("bind error");
    return 1;
  }
  if(listen(listener,QUEUE_LEN) < 0) {
    perror("listen error");
    return 1;
  }
  pid_t pId;
  while(1) {
    struct sockaddr_in clientIn;
    int clientInSize = sizeof clientIn;
    int newfd = accept(listener, (struct sockaddr*)&clientIn, (socklen_t*)&clientInSize);
    if (newfd < 0) {
      perror("accept");
      return 1;
    }
    pId = fork();
    if(pId < 0) {
      perror("fork error");
      return 1;
    }
    if(pId == 0) {
      int nameLen, opLen ,fail = -1, success = 1;
      if(recv(newfd, &opLen, sizeof(int), 0) < 0) {
	perror("recive");
	return 1;
      }
      char op_buf [opLen + 1];
      op_buf[opLen] = 0;
      if(recv(newfd, op_buf, opLen, 0) < 0) {
	perror("recive");
	return 1;
      }
      if(recv(newfd, &nameLen, sizeof(int), 0) < 0) {
	perror("recive");
	return 1;
      }
      char buf [nameLen + 1];
      if(recv(newfd, buf, nameLen, 0) < 0) {
	perror("recive");
	return 1;
      }
      buf[nameLen] = 0;
      int newFile = open(buf, O_RDONLY);
      if(newFile < 0) {
        perror("No such file");
        send(newfd, &fail, sizeof(int), 0);
        return 1;
      }
      else
        send(newfd, &success, sizeof(int), 0);
      struct stat st;
      if(fstat(newFile, &st) < 0 ) {
	perror("fstat");
	return 1;
      }
      int fileLen = st.st_size;
      if(strcmp(op_buf, "download-file") == 0) {
        char buf1 [SIZE];
        while(fileLen > 0) {
          int readed = read(newFile, buf1, SIZE);
          send(newfd, buf1, readed, 0);
          fileLen -= readed;
        }
      }
      printf("%s\n", op_buf);
      if(strcmp(op_buf, "get-file-info") == 0) {
        int owner = st.st_uid;
        send(newfd, &fileLen, sizeof(int), 0);
        send(newfd, &owner, sizeof(int), 0);
      }
      close(newFile);
      close(newfd);
    }
  }
  close(listener);
  return 0;
}
