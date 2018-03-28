/*Contains implementation of list.h*/
#include "list.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <unistd.h>       
#include <stdio.h>
#include "common.h"


/* Allocate memory for a node of type struct nodeStruct and initialize
 * it with the value item. Return a pointer to the new node. */
struct nodeStruct* List_createNode(int item, char *command)
{
	struct nodeStruct * newNode;
	newNode = (struct nodeStruct *)malloc(sizeof(struct nodeStruct));
	newNode->item = item;
	strcpy(newNode->command, command);
	newNode->next=NULL;

	return newNode;
}


/* Insert node at the head of the list. */
void List_insertHead (struct nodeStruct **headRef, struct nodeStruct *node)
{
	node->next = *headRef;
	*headRef=node;
}


/* Insert node after the tail of the list. */
void List_insertTail (struct nodeStruct **headRef, struct nodeStruct *node)
{
	struct nodeStruct* cur =*headRef;
	while(cur->next != NULL)
	{
		cur=cur->next;
	}
	cur->next=node;

}


/*Count number of nodes in the list.
 * Return 0 if the list is empty, i.e., head == NULL */
int List_countNodes (struct nodeStruct *head)
{
	struct nodeStruct* cur = head;
	int count =0;
	while(cur != NULL)
	{
		count++;
		cur=cur->next;
	}
	return count;
}

/* Return the first node holding the value item, return NULL if none found */
struct nodeStruct* List_findNode(struct nodeStruct *head, int item)
{
	struct nodeStruct* cur = head;
	while(cur != NULL)
	{
		if(cur->item == item)
		{
			return cur;
		}
		cur = cur->next;
	}
	return NULL;
}

/* Delete node from the list and free memory allocated to it.
 * This function assumes that node has been properly set to a valid node 
 * in the list. For example, the client code may have found it by calling 
 * List_findNode(). If the list contains only one node, the head of the list 
 * should be set to NULL. */
void List_deleteNode (struct nodeStruct **headRef, struct nodeStruct *node)
{
	struct nodeStruct *cur = *headRef;

	//List contains one node
	if(cur->next == NULL)
	{
		*headRef = NULL;
		return;
	}

	struct nodeStruct *prev=NULL;
	while(cur!= NULL)
	{
		if(cur->item == node->item)
		{
			//Head is being removed
			if(prev == NULL)
			{
				//printf("Head being removed\n");
				struct nodeStruct *tmp;		
				tmp=cur->next;
				free(cur);
				*headRef = tmp;
				return;
			}
			prev->next=cur->next;
			free(cur);
			return;
		}
		prev = cur;
		cur = cur->next;
	}
}

/* Sort the list in ascending order based on the item field.
 * Any sorting algorithm is fine. */
void List_sort (struct nodeStruct **headRef) 
{
	struct nodeStruct *i,*j;
	struct nodeStruct *cur = *headRef;
	for(i=cur;i!=NULL && i->next!=NULL; i=i->next)
	{
		struct nodeStruct *min;
		min=i;
		for(j=i->next;j!=NULL;j=j->next)
		{
			if(j->item < i->item)
			{
				min=j;
			}
		}
		if(min!=i)
		{
			int tmp;
			tmp=min->item;
			min->item=i->item;
			i->item=tmp;
		}
	}


}

/* Prints node with write */
void print_Node(struct nodeStruct *node)
{
	char int_buff[25];
	sprintf(int_buff,"%d",node->item);
	write(STDOUT_FILENO, int_buff, strlen(int_buff));
	write(STDOUT_FILENO, "\t", strlen("\t"));
	write(STDOUT_FILENO, node->command, strlen(node->command));
	write(STDOUT_FILENO, "\n", strlen("\n"));
}

/* Prints list */
void List_print(struct nodeStruct **headRef)
{
	if(*headRef==NULL)
	{
		return;
	}
	struct nodeStruct *cur = *headRef; 
	while(cur->next != NULL)
	{
		print_Node(cur);
		cur = cur->next;
	}
	print_Node(cur);

}

void List_append(struct nodeStruct **headRef, struct nodeStruct *node)
{
	//printf("Appending %s\n", node->command);
	//check if head is null
	if(*headRef==NULL)
	{
		List_insertHead(headRef, node);
		return;
	}
	//check length, if longer than 10, delete head
	if(List_countNodes(*headRef)>=10)
	{
		//printf("List count >10\n");
		List_deleteNode(headRef, *headRef);
	}
	List_insertTail(headRef, node);
}

void List_free(struct nodeStruct *headRef)
{
	struct nodeStruct *tmp;

	while(headRef !=NULL)
	{
		tmp=headRef;
		headRef = headRef->next;
		free(tmp);
	}
}