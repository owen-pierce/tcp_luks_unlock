#ifndef MYQUEUE_H_
#define MYQUEUE_H_

struct node
{
    struct node* next; /* data */
    int *client_socket;
};
typedef struct node node_t;

void enqueue(int *client_socket);
int* dequeue();

#endif
