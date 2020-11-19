#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>

#define MSG_SIZE 1024

char *request_message(char *page, char *host)
{
  char *sendline = calloc(MSG_SIZE, sizeof(char));
  page[strlen(page) - 1] = '\0';
  sprintf(sendline,
       "GET %s HTTP/1.1\r\n"
       "Host: %s\r\n\r\n",
       page, host);
  return sendline;
}

int main(int argc, char **argv)
{
  struct sockaddr_in server;
  struct hostent *host;
  char msg_client[MSG_SIZE], msg_server[MSG_SIZE];
  char id[10];
  sprintf(id, "%d", getpid());
  id[strlen(id)] = 0;

  if (argc < 3)
  {
     fprintf(stderr, "Использование: %s <hostname> <port_number>\n", argv[0]);
     return 1;
  }

  // Создание сетевого сокета (домен AF_INET)
  // Тип сокета -- SOCK_STREAM, сокет должен быть потоковым
  // Протокол -- IPPROTO_TCP
  int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (sock < 0)
  {
    printf("%s", strerror(errno));
    return errno;
  }

  // Преобразование доменного имени сервера в его сетевой адрес
  host = gethostbyname(argv[1]);
  if (host == NULL)
  {
    printf("%s", strerror(errno));
    return errno;
  }

  // Укажем семейство адресов, которыми мы будем пользоваться
  server.sin_family = AF_INET;
  // Укажем адрес (наша программа-сервер зарегистрируется на всех адресах
  // машины, на которой она выполняется)
  memcpy(&server.sin_addr, host->h_addr_list[0], host->h_length);
  // Укажем значение порта. Функция htons() переписывает двухбайтовое значение
  // порта так, чтобы порядок байтов соответствовал принятому в Интернете
  server.sin_port = htons(atoi(argv[2]));

  // Установка соединения
  if (connect(sock, (struct sockaddr *) &server, sizeof(server)) < 0)
  {
    printf("%s", strerror(errno));
    return errno;
  }

  memset(msg_client, 0, MSG_SIZE);
  printf("Введите сообщение:\n");
  fgets(msg_client, MSG_SIZE, stdin);

  char adress[50];
  sprintf(adress, "%s:%s", argv[1], argv[2]);
  char *send_message = request_message(msg_client, adress);
  // Для записи данных - write
  write(sock, send_message, strlen(send_message));
  free(send_message);

  // Заполнение массива нулями
  memset(msg_server, 0, MSG_SIZE);
  // Для чтения данных из сокета - read
  read(sock, msg_server, MSG_SIZE);
  printf("%s\n", msg_server);

  // Закрываем сокет
  close(sock);

  return 0;
}
