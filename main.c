#include "netinet/in.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "sys/socket.h"
#include "unistd.h"
#include <asm-generic/socket.h>
#include <setjmp.h>
#include <sys/socket.h>
#include <sys/types.h>

#define PORT 8000

jmp_buf exception_buffer;
int main() {
  int server_fd, new_socket;
  ssize_t valread;
  struct sockaddr_in address;
  int opt = 1;
  socklen_t addrlen = sizeof(address);
  char buffer[1024] = {0};
  char *msg = "HTTP/1.1 200 OK\r\nContent-Type: "
              "text/html\r\n\r\n<html><body><h1>Hi</h1></body></html>";

  printf("creating socket...\n");
  if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    perror("socket not created\n");
    exit(1);
  }

  printf("setting options\n");
  if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt,
                 sizeof(opt))) {
    perror("socket options not setted\n");
    exit(1);
  }
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY;
  address.sin_port = htons(PORT);
  printf("binding the server to PORT 8000\n");
  if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
    perror("bind failed");
    exit(EXIT_FAILURE);
  }
  printf("listening\n");
  if (listen(server_fd, 3) < 0) {
    perror("listen");
    exit(EXIT_FAILURE);
  }
  while (1) {
    printf("accepting\n");
    if ((new_socket =
             accept(server_fd, (struct sockaddr *)&address, &addrlen)) < 0) {
      perror("accept");
      exit(EXIT_FAILURE);
    }
    valread = read(new_socket, buffer, 1024 - 1);
    char *buf = malloc(sizeof(buffer));
    strcpy(buf, buffer);
    char *tok = strtok(buf, "\n");
    char *method = (char *)malloc(sizeof(char) * strlen(tok));
    char *url = (char *)malloc(sizeof(char) * strlen(tok));
    char *http_version = (char *)malloc(sizeof(char) * strlen(tok));
    sscanf(tok, "%s %s %s", method, url, http_version);
    char **args = (char **)malloc(sizeof(char *) * 100);
    printf("%s|%s|%s\n", method, url, http_version);
    int i = 0;
    while (1) {
      tok = strtok(NULL, "\n");
      if (tok == NULL) {
        break;
      }
      if (strcmp(tok, "\r") == 0) {
        break;
      }
      args[i] = (char *)malloc(sizeof(char) * strlen(tok));
      strcpy(args[i], tok);
      printf("%s\n", args[i]);
      i++;
    }

    for (int i = 0; i < 100; i++) {
      // TODO: parse the request like Content: shit -> Content = shit
      // args[i];
    }

    // printf("%s\n", buffer);

    send(new_socket, msg, strlen(msg), 0);
    printf("Hello send to the client\n");

    for (int i = 0; i < 100; i++) {
      free(args[i]);
    }
    free(args);
    free(buf);
    free(method);
    free(url);
    free(http_version);
    close(new_socket);
  }
  close(server_fd);
}
