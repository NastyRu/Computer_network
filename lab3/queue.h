#ifndef QUEUE_H_
#define QUEUE_H_

struct node
{
  int *client_socket;
  struct node *next;
};
typedef struct node node_t;

void push(int *client_socket);
int *pop();

#endif
