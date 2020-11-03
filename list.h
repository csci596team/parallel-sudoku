#include <stdio.h>
#include <stdlib.h>


struct Node 
{
	// if num == -1, the Node is dummy
	int num;  
	int cellPosition, cellRow, cellCol;
	Node* next;
	Node* prev;
};


struct List
{
	Node* head;
	Node* tail;
};


Node* new_node(int n, int p, int r, int c);

List* init_list();
void clear_list(List* list);

void insert_head(List* list, Node* node);
void insert_tail(List* list, Node* node);

Node* pop_head(List* list);
Node* pop_tail(List* list);






