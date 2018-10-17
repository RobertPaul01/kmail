#include <arpa/inet.h>
#include <fcntl.h>
#include <iostream>
#include <netinet/in.h>
#include <pthread.h>
#include <string>
#include <sys/socket.h>
#include <unistd.h>

#define MAX_CLIENTS 10

/* SMTP Protocol */
#define HELO 0
#define MAIL_FROM 1
#define RCPT_TO 2
#define DATA 3
#define RSET 4
#define NOOP 5
#define QUIT 6
#define INVALID 7

void send_data(int socket, const char *data) {
  if (data != NULL) {
    send(socket, data, strlen(data), 0);
    std::cout << "Reply stream: " << data;
  }
}

void respond(int client_sockfd, char *request) {
  int index = 0;
  std::string smtp[] = {"HELO", "MAIL FROM", "RCPT TO", "DATA",
                        "RSET", "NOOP",      "QUIT"};

  for (std::string str : smtp) {
    if (strncmp(request, str.c_str(), str.size()) == 0) {
      break;
    }
    index++;
  }

  switch (index) {
  case HELO:
    send_data(client_sockfd, "Goodbye\n");
    break;
  case MAIL_FROM:
    send_data(client_sockfd, "Mail from who\n");
    break;
  case INVALID:
    send_data(client_sockfd, "Invalid request\n");
    break;
  }
}

void *listen(void *param) {
  int client_sockfd, len;
  char buffer[4096];

  memset(buffer, 0, sizeof buffer);
  client_sockfd = *(int *)param;

  send_data(client_sockfd, "220 Ready\r\n");

  while (1) {
    memset(buffer, 0, sizeof buffer);
    len = recv(client_sockfd, buffer, sizeof buffer, 0);
    if (len > 0) {
      std::cout << "Request: " << buffer;
      respond(client_sockfd, buffer);
    } else {
      std::cout << "No data from server" << std::endl;
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

  fcntl(server_sockfd, F_SETFL, fcntl(server_sockfd, F_GETFL, 0) | O_NONBLOCK);

  if (listen(server_sockfd, MAX_CLIENTS) == -1) {
    std::cerr << "Listen error!" << std::endl;
    return 1;
  }

  std::cout << "Starting server!" << std::endl;
  sin_size = sizeof client_addr;
  while (1) {
    if ((client_sockfd = accept(server_sockfd, (struct sockaddr *)&client_addr,
                                &sin_size)) == -1) {
      sleep(1);
      continue;
    }

    std::cout << "Connection opened from " << inet_ntoa(client_addr.sin_addr)
              << " at " << time(NULL) << std::endl;

    pthread_t id;
    pthread_create(&id, NULL, listen, &client_sockfd);
    pthread_join(id, NULL);
  }

  close(client_sockfd);
  return 0;
}
