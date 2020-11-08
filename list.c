#include "list.h"

Node* new_node(int n, int p)
{
	Node* node = (Node*)malloc(sizeof(Node));

	node -> num = n;
	node -> pos = p;

	node -> next = NULL;
	node -> prev = NULL;

	return node;
}

List* init_list()
{
	List* list = (List*)malloc(sizeof(List));

	list -> head = new_node(-1, -1);
	list -> tail = new_node(-1, -1);

	list -> head -> next = list -> tail;
	list -> tail -> prev = list -> head;

	return list;
}

void clear_list(List* list)
{
	Node* cur = list -> head -> next;
	while(cur)
	{
		list -> head -> next = cur -> next;
		free(cur);
		cur = list -> head -> next;
	}

	list -> tail -> prev = list -> head;

}

void insert_head(List* list, Node* node)
{
	node -> next = list -> head -> next;
	node -> prev = list -> head;

	list -> head -> next -> prev = node;
	list -> head -> next = node;
}

void insert_tail(List* list, Node* node)
{
	node -> next = list -> tail;
	node -> prev = list -> tail -> prev;

	list -> tail -> prev -> next = node;
	list -> tail -> prev = node;
}

Node* pop_head(List* list)
{
	if(list -> head -> next == list -> tail)
		return NULL;

	Node* cur = list -> head -> next;
	cur -> next -> prev = list -> head;
	list -> head -> next = cur -> next;

	return cur;
}

Node* pop_tail(List* list)
{
	if(list -> tail -> prev == list -> head)
		return NULL;

	Node* cur = list -> tail -> prev;
	cur -> prev -> next = list -> tail;
	list -> tail -> prev = cur -> prev;

	return cur;
}

int get_tail_pos(List* list)
{
	return list -> tail -> prev -> pos;
}

bool is_empty(List* list)
{
	if(list -> head -> next == list -> tail)
		return true;
	return false;
}

int get_list_size(List* list)
{
	int size = 0;
	Node* cur = list -> head;
	while(cur -> next != list -> tail)
	{
		size++;
		cur = cur -> next;
	}
	return size;
}
