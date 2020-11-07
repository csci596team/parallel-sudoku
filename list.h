#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>


typedef struct Node 
{
	// if num == -1, the Node is dummy
	int num;  
	int pos;
	struct Node* next;
	struct Node* prev;
}Node;


typedef struct List
{
	struct Node* head;
	struct Node* tail;
}List;


Node* new_node(int n, int p);

List* init_list();
void clear_list(List* list);

void insert_head(List* list, Node* node);
void insert_tail(List* list, Node* node);

Node* pop_head(List* list);
Node* pop_tail(List* list);

int get_tail_pos(List* list);
bool is_empty(List* list);






