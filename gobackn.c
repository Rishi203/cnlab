SERVER

#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>
void main() {
  int server_sock, acpt_sock, ret_val, i = -1;
  char msg[50] = "Received frame", read_buff[50], write_buff[50];
  fd_set set;
  struct sockaddr_in serv_addr, other_addr;
  struct timeval timeout;
  socklen_t len;
  server_sock = socket(AF_INET, SOCK_STREAM, 0);
  memset(&serv_addr, 0, sizeof(serv_addr));
  memset(&other_addr, 0, sizeof(other_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(6901);
  serv_addr.sin_addr.s_addr = INADDR_ANY;
  if (bind(server_sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) ==-1) {
    printf("Binding failed");
    exit(0);
  }
  printf("\nSender of Go Back N where N = 8\n");
  listen(server_sock, 5);
  len = sizeof(other_addr);
  acpt_sock = accept(server_sock, (struct sockaddr *)&other_addr, &len);
zero:
  i = i + 1;
  memset(&write_buff, 0, sizeof(write_buff));
  strcpy(write_buff, msg);
  write_buff[strlen(msg)] = i + '0';
  printf("To Receiver -> Frame %d\n", i);
  write(acpt_sock, write_buff, sizeof(write_buff));
  i = i + 1;
  sleep(1);
one:
  memset(&write_buff, 0, sizeof(write_buff));
  strcpy(write_buff, msg);
  write_buff[strlen(msg)] = i + '0';
  printf("To Receiver -> Frame %d\n", i);
  write(acpt_sock, write_buff, sizeof(write_buff));
  FD_ZERO(&set);
  FD_SET(acpt_sock, &set);
  timeout.tv_sec = 2;
  ret_val = select(acpt_sock + 1, &set, NULL, NULL, &timeout);
  if (ret_val == -1)
    printf("Error in select()");
  else if (ret_val == 0) {
    printf("\n--- Timer Timeout - Packet %d Sent Lost ---\n", i - 1);
    printf("--- Restart Timer - Resending Packets in Go Back N Method ---\n\n");
    i = i - 2;
    goto zero;
  } else {
    read(acpt_sock, read_buff, sizeof(read_buff));
    if ((i == 6) || (i == 4) || (i == 1))
      printf("--- From Receiver <- %s -- Cumulative Acknowledgement ---\n",
             read_buff);
    else
      printf("--- From Receiver <- %s -- Individual Acknowledgement ---\n",
             read_buff);
    printf("-----------------------------------------------------\n");
    i++;
    if (i > 7)
      exit(0);
    if ((i == 5) || (i == 3)) {
      i--;
      goto zero;
    }
    if ((i < 8) || (i != 5))
      goto one;
  }
  close(acpt_sock);
  close(server_sock);
}

CLIENT

#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
void main() {
  int cli_sock, i, j, flag = 1, note = 1;
  char msg1[50] = "ACK", write_buff[50], read_buff[100];
  struct sockaddr_in client;
  cli_sock = socket(AF_INET, SOCK_STREAM, 0);
  memset(&client, 0, sizeof(client));
  client.sin_family = AF_INET;
  client.sin_port = htons(6901);
  client.sin_addr.s_addr = inet_addr("127.0.0.1");
  if (connect(cli_sock, (struct sockaddr *)&client, sizeof(client)) == -1) {
    printf("Connection failed");
    exit(0);
  }
  printf("\nReceiver of Go Back N where N = 8\n\n");
  for (i = 0; i < 8; i++) {
    memset(&read_buff, 0, sizeof(read_buff));
    memset(&write_buff, 0, sizeof(write_buff));
    if (i == 5 && flag == 1) {
      flag = 0;
      read(cli_sock, read_buff, sizeof(read_buff));
    }
    read(cli_sock, read_buff, sizeof(read_buff));
    if (read_buff[strlen(read_buff) - 1] != i + '0') {
      printf("\n --- Packet from Sender %d Lost - %d Received Out of Order - "
             "Discarding ---\n",
             i, i + 1);
      i--;
    } else {
      if (i == 0) {
        printf("-> %s\n", read_buff);
        read(cli_sock, read_buff, sizeof(read_buff));
        i++;
      }
      printf("-> %s\n", read_buff);
      if ((i == 5) || (i == 3)) {
        read(cli_sock, read_buff, sizeof(read_buff));
        printf("-> %s\n", read_buff);
        i++;
      }
      if (i == 7)
        j = 0;
      else
        j = i + 1;
      if ((i == 6) || (i == 4) || (i == 1))
        printf("<- ACK %d - Cumulative Acknowledgement\n", j);
      else
        printf("<- ACK %d - Individual Acknowledgement\n", j);
      printf("-----------------------------------------------------\n");
      strcpy(write_buff, msg1);
      write_buff[strlen(msg1)] = j + '0';
      write(cli_sock, write_buff, sizeof(write_buff));
    }
  }
  close(cli_sock);
}
