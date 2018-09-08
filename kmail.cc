#include <iostream>
#include <netinet/in.h>
#include <pthread.h>
#include <string>
#include <sys/socket.h>

void *listen(void *param) {
  int client_socket, len;
  char buffer[4096];
  memset(buffer, 0, sizeof buffer);

  std::string code = "220 Ready\r\n";
  send(client_socket, code.c_str(), code.size(), 0);

  while (1) {
    len = recv(client_socket, buffer, sizeof buffer, 0);
    if (len > 0) {
    } else {
      break;
    }
  }

  return NULL;
}

int main() {
  signal(SIGPIPE, SIG_IGN);
  int server_sockfd, client_sockfd;
  socklen_t sin_size;
  struct sockaddr_in server_addr, client_addr;

  memset(&server_addr, 0, sizeof server_addr);
  server_sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (server_sockfd == -1) {
    std::cerr << "Error creating socket" << std::endl;
    return 1;
  }

  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(25);
  server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  bzero(&(server_addr.sin_zero), 8);

  int link = bind(server_sockfd, (struct sockaddr *)&server_addr,
                  sizeof(struct sockaddr));
  if (link == -1) {
    std::cerr << "Error binding socket. Try running with root" << std::endl;
    return 1;
  }

  int client_socket;

  pthread_t id;
  pthread_create(&id, NULL, listen, &client_socket);
  pthread_join(id, NULL);

  return 0;
}
