#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdlib.h>

#define MSG_SIZE 256

char intToChar(int num)
{
  if (num > 9)
  {
    return num + 'a' - 10;
  }
  else
  {
    return num + '0';
  }
}

char *insertFisrt(char *array, char elem)
{
  char *res = (char *) malloc(sizeof(array) + sizeof(char));
  res[0] = elem;

  int i = 0;
  while (array[i] != '\0')
  {
    res[i + 1] = array[i];
    i++;
  }
  res[i + 1] = '\0';

  free(array);
  return res;
}

char *decToAny(char *num, int ns)
{
  char *res = (char *) malloc(sizeof(char));
  int number = atoi(num);
  int i = 0;

  res[0] = '\0';

  while (number >= ns)
  {
    res = insertFisrt(res, intToChar(number % ns));
    number /= ns;
    i++;
  }

  res = insertFisrt(res, intToChar(number));

  return res;
}

int main(int argc, char ** argv)
{
  // Структура предназначен для хранения адресов в формате Интернета
  struct sockaddr_in server;
  char msg[MSG_SIZE];
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

  // Связывание сокета с заданным адресом
  // bind(дескриптор сокета, указатель на структуру, длина структуры)
  if (bind(sock, (struct sockaddr *) &server, sizeof(server)) < 0)
  {
    printf("%s", strerror(errno));
    return errno;
  }

  // Пока клиент не отправит сообщение "break"
  while (strcmp(msg, "break"))
  {
    // Для чтения данных из датаграммного сокета - recvfrom,
    // которая блокирует программу до тех пор, пока на входе не появятся новые данные
    // Так как нас не интересуют данные об адресе клиента
    // передаем значения NULL в предпоследнем и последнем параметрах
    bytes = recvfrom(sock, msg, MSG_SIZE, 0, NULL, NULL);
    if (bytes < 0)
    {
      printf("%s", strerror(errno));
      return errno;
    }
    // Символ окончания строки
    msg[bytes] = 0;
    printf("Полученное от клиента число в десятичной системе счисления: %s\n", msg);
    char *bin = decToAny(msg, 2);
    printf("Полученное от клиента число в двоичной системе счисления: %s\n", bin);
    free(bin);
    printf("Полученное от клиента число в восьмеричной системе счисления: %o\n", atoi(msg));
    printf("Полученное от клиента число в шестнадцатиричной системе счисления: %x\n", atoi(msg));
    char *nineteenth = decToAny(msg, 19);
    printf("Полученное от клиента число в девятнадцатиричной системе счисления: %s\n\n", nineteenth);
    free(nineteenth);
  }

  // Закрываем сокет
  close(sock);

  return errno;
}
