#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <time.h>
#include "queue.h"

#define MSG_SIZE 1024
#define LISTENQ 1024
#define NUM_THREADS 4

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
// Сокет сервера
int listensock;
// Пул потоков
pthread_t threads[NUM_THREADS];

// Обработчик сигнала ctrl-c
void catch_sigp(int sig_numb)
{
  signal(sig_numb, catch_sigp);
	for (int i = 0; i < NUM_THREADS; i++)
	{
		pthread_detach(threads[i]);
	}
	// Закрываем сокет
  close(listensock);
	exit(errno);
}

char *read_file(char *filename)
{
	char *fcontent = NULL;
  int fsize = 0;
  FILE *fp;

	if (strcmp(filename, "/") == 0)
	{
		fp = fopen("main.html", "r");
	}
	else
	{
		if (filename[0] == '/')
		{
			int sz = strlen(filename);
			memmove(filename, filename + 1, sz - 1);
			filename[sz - 1] = '\0';
		}
		fp = fopen(filename, "r");
	}

	if (fp)
	{
		fseek(fp, 0, SEEK_END);
		fsize = ftell(fp);
		rewind(fp);

		fcontent = calloc(fsize, sizeof(char));
		fread(fcontent, 1, fsize, fp);

		fclose(fp);
	}

	return fcontent;
}

char *response_message(char *filename)
{
  char *sendline = calloc(MSG_SIZE, sizeof(char));
	char *file_content = read_file(filename);

	if (file_content)
	{
		sprintf(sendline,
	       "HTTP/1.1 200 OK\r\n"
	       "Connection: closed\r\n"
				 "Content-Type: text/html\r\n"
				 "Content-Length: %lu\r\n\r\n"
				 "%s",
				 strlen(file_content),
	       file_content);
		free(file_content);
	}
	else
	{
		char *line_error = "<h1>404 PAGE NOT FOUND</h1>";
		sprintf(sendline,
	       "HTTP/1.1 404 Not found\r\n"
	       "Connection: closed\r\n"
				 "Content-Type: text/html\r\n"
			 	 "Content-Length: %lu\r\n\r\n"
				 "%s",
			 	 strlen(line_error),
			   line_error);
	}

  return sendline;
}

char *parse_client_message(char *message)
{
	char *method = strtok(message, " ");
  char *url = strtok(NULL, " ");

	return url;
}

char *replace_word(const char *s, const char *before_word)
{
  char *result = NULL;
  int i = 0, cnt = 0, new_num = 0;
	char num[20];

  for (i = 0; s[i] != '\0'; i++) {
    if (strstr(&s[i], before_word) == &s[i])
		{
      i += strlen(before_word);
			break;
    }
  }

  result = (char*)malloc(strlen(s) + 1);

	i += 2;
	cnt = i;
	for (int j = 0; j < i; j++)
		result[j] = s[j];

	int k = 0;
	while (s[cnt] != '<')
	{
		num[k] = s[cnt];
		cnt++;
		k++;
	}
	new_num = atoi(num) + 1;

	memset(num, 0, 20);
	sprintf(num, "%d", new_num);
	int new_len = strlen(num);

	for (int j = 0; j < new_len; j++)
		result[j + i] = num[j];

	i += new_len;

	while (s[cnt] != '\0')
	{
		result[i] = s[cnt];
		i++;
		cnt++;
	}

  result[i] = '\0';
  return result;
}

void update_statistic()
{
	time_t t = time(NULL);
	struct tm *now = localtime(&t);
	int week_day = now->tm_wday;

	FILE *fp;
	char *fcontent = NULL;
	fp = fopen("statistic.html", "r");
	if (fp)
	{
		fseek(fp, 0, SEEK_END);
		int fsize = ftell(fp);
		rewind(fp);

		fcontent = calloc(fsize, sizeof(char));
		fread(fcontent, 1, fsize, fp);

		fclose(fp);
	}
	else
	{
		return;
	}

	char *new_fcontent = NULL;

	switch (week_day)
	{
    case 1:

			new_fcontent = replace_word(fcontent, "Monday");
			break;
    case 2:
			new_fcontent = replace_word(fcontent, "Tuesday");
			break;
    case 3:
			new_fcontent = replace_word(fcontent, "Wednesday");
      printf("1\n");
			break;
		case 4:
			new_fcontent = replace_word(fcontent, "Thursday");
			break;
		case 5:
			new_fcontent = replace_word(fcontent, "Friday");
			break;
		case 6:
			new_fcontent = replace_word(fcontent, "Saturday");
			break;
		case 7:
			new_fcontent = replace_word(fcontent, "Sunday");
			break;
	}
	free(fcontent);

	fp = fopen("statistic.html", "w");
	if (fp)
	{
		fprintf(fp, "%s", new_fcontent);
		fclose(fp);
	}

	free(new_fcontent);
}

void *client_handler(void *my_socket)
{
	printf("Connected ...\n");
  update_statistic();
  // Получение дескриптора сокета
  int read_size;
  int socket = *(int*)my_socket;
  char *message = NULL, client_message[2000];

  // Получаем запросы от клиента
  read_size = recv(socket, client_message, 2000, 0);
  client_message[read_size] = '\0';
  printf("%s\n", client_message);

	char *url = parse_client_message(client_message);

	// Отправляем ответ
  message = response_message(url);
  write(socket, message, strlen(message));
	free(message);
	free(my_socket);
  return 0;
}

void *thread_function(void *arg)
{
  while (1)
  {
    pthread_mutex_lock(&mutex);
    int *pclient = pop();
    pthread_mutex_unlock(&mutex);
    if (pclient != NULL)
		{
      client_handler(pclient);
    }
  }
}

int main(int argc, char ** argv)
{
  // Структура предназначен для хранения адресов в формате Интернета
  struct sockaddr_in server_addr, client_addr;
  int client_len = sizeof(client_addr);

  for (int i = 0; i < NUM_THREADS; i++)
  {
    pthread_create(&threads[i], NULL, thread_function, NULL);
  }

  if (argc < 2)
  {
    fprintf(stderr, "Использование: %s <port_number>\n", argv[0]);
    return 1;
  }

  // Создание сетевого сокета (домен AF_INET)
  // Тип сокета -- SOCK_STREAM, сокет должен быть потоковым
  // Протокол -- IPPROTO_TCP
  listensock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (listensock < 0)
  {
		for (int i = 0; i < NUM_THREADS; i++)
	  {
	    pthread_detach(threads[i]);
	  }
    printf("%s", strerror(errno));
    return errno;
  }

  // Укажем семейство адресов, которыми мы будем пользоваться
  server_addr.sin_family = AF_INET;
  // Укажем адрес (наша программа-сервер зарегистрируется на всех адресах
  // машины, на которой она выполняется)
  server_addr.sin_addr.s_addr = INADDR_ANY;
  // Укажем значение порта. Функция htons() переписывает двухбайтовое значение
  // порта так, чтобы порядок байтов соответствовал принятому в Интернете
  server_addr.sin_port = htons(atoi(argv[1]));

  // Связывание сокета с заданным адресом
  // bind(дескриптор сокета, указатель на структуру, длина структуры)
  if (bind(listensock, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0)
  {
    printf("%s", strerror(errno));
    return errno;
  }

  // Переводим сервер в режим ожидания запроса на соединение
  // Второй параметр - максимальное число обрабатываемых одновременно соединений
  listen(listensock, LISTENQ);
	printf("Waiting for a new connections ...\n");

	// Обработчик сигнала
	signal(SIGINT, catch_sigp);
  while (1)
  {
    int new_socket = accept(listensock, (struct sockaddr *)&client_addr, (socklen_t *)&client_len);

    int *pclient = malloc(sizeof(int));
    *pclient = new_socket;
    pthread_mutex_lock(&mutex);
    push(pclient);
    pthread_mutex_unlock(&mutex);
  }

	for (int i = 0; i < NUM_THREADS; i++)
  {
    pthread_detach(threads[i]);
  }

  // Закрываем сокет
  close(listensock);

  return errno;
}
