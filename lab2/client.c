#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#define MSG_SIZE 256

int main(int argc, char ** argv)
{
  char msg[MSG_SIZE];
  struct sockaddr_in server;
  int bytes;

  // Создание сетевого сокета (домен AF_INET)
  // Тип сокета -- SOCK_DGRAM означает датаграммный сокет
  // Протокол -- 0, протокол выбирается по умолчанию
  int sock = socket(AF_INET, SOCK_DGRAM, 0);
  if (sock < 0)
  {
    printf("%s", strerror(errno));
    return errno;
  }

  // Укажем семейство адресов, которыми мы будем пользоваться
  server.sin_family = AF_INET;
  // Укажем адрес (наша программа-сервер зарегистрируется на всех адресах
  // машины, на которой она выполняется)
  server.sin_addr.s_addr = INADDR_ANY;
  // Укажем значение порта. Функция htons() переписывает двухбайтовое значение
  // порта так, чтобы порядок байтов соответствовал принятому в Интернете
  server.sin_port = htons(atoi(argv[1]));

  // Приглашение и ввод сообщения для сервера
  printf("Введите число:\n");
  scanf("%s", msg);

  // Передаем сообщение серверу
  // sendto(дескриптор сокета, адрес буфера для передачи данных, его длина,
  // дополнительные флаги, адрес сервера, его длина)
  sendto(sock, msg, strlen(msg), 0, (struct sockaddr *) &server, sizeof(server));

  return errno;
}
