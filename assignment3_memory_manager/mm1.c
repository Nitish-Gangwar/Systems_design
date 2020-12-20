/*
 * mm-naive.c - The fastest, least memory-efficient malloc package.
 * 
 * In this naive approach, a block is allocated by simply incrementing
 * the brk pointer.  A block is pure payload. There are no headers or
 * footers.  Blocks are never coalesced or reused. Realloc is
 * implemented directly using mm_malloc and mm_free.
 *
 * NOTE TO STUDENTS: Replace this header comment with your own header
 * comment that gives a high level description of your solution.
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>

#include "mm.h"
#include "memlib.h"

/*********************************************************
 * NOTE TO STUDENTS: Before you do anything else, please
 * provide your team information in the following struct.
 ********************************************************/
team_t team = {
    /* Team name */
    "203050069+203050102",
    /* First member's full name */
    "Nitish Gangwar",
    /* First member's email address */
    "nitishgangwar@cse.iitb.ac.in",
    /* Second member's full name (leave blank if none) */
    "Suhanshu Patel",
    /* Second member's email address (leave blank if none) */
    "suhanshu@cse.iitb.ac.in"
};

/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~0x7)


#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))

#define MAX 300000000

struct header
{
	size_t size;
	unsigned is_free;
	struct header *next,*prev;
};
typedef struct header header_t;

header_t *head,*tail;

/* 
 * mm_init - initialize the malloc package.
 */

void *init_mem_sbrk_break = NULL;

int mm_init(void)
{
	head=NULL;
	tail=NULL;
	
    return 0;		//Returns 0 on successfull initialization.
}

//---------------------------------------------------------------------------------------------------------------
/* 
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size)
{	
	size_t total_size;
	void *block;
	header_t *header=NULL;
	if(size <= 0){		// Invalid request size
		return NULL;
	}
	size=ALIGN(size);
	total_size = ALIGN(sizeof(header_t) + size);
	header_t *cur=head;
	long int difference=MAX;
	header=cur;
	while(cur)
	{
		if(!(cur->is_free) && (cur->size >= total_size))
		{
			int diff = cur->size - total_size;
			//printf("contestant is %d for %d\n",cur->size,size);
			if(diff < difference)
			{
				header=cur;
				difference=diff;
			}
		
		}
		//back=cur;
		cur=cur->next;
	}
	
	if(difference < MAX)
	{
		
		if(header->size-size>16)
		{
			size_t new_block_size=ALIGN(header->size-size-sizeof(header_t));
			header_t *new=(void*)((char*)header + sizeof(header_t) + size);
			new->size=new_block_size;
			new->is_free=0;
			new->next=header->next;
			new->prev=header;
			header->next=new;
			header->size=size;
			header->is_free=1;
			if(new->next)
			{
				new->next->prev=new;
			}
			else
			{
				tail=new;
			}
			
			return (void*)(header + 1);	
		}
		header->is_free = 1;
		header->size=total_size;
		return (void*)(header + 1);
	}
	block = mem_sbrk(total_size);
	
	if(block == (void*) -1)
	{
		return NULL;
	}
	header = block;
	header->size = size;
	header->is_free=1;
	header->next=NULL;
	if(!head)
	{
		head = header;
		head->prev=NULL;
	}
	if(tail)
	{
		tail->next = header;
		header->prev=tail;
		//printf("inserted at tail\n");
	}
	tail = header;
	return (void*)(header + 1);		//mem_sbrk() is wrapper function for the sbrk() system call. 
								//Please use mem_sbrk() instead of sbrk() otherwise the evaluation results 
								//may give wrong results
}


void mm_free(void *ptr)
{
	 header_t *header;
	 
	 if(!ptr)
	 	return;
	 
	 header = (header_t*)ptr -1;
	 
	 header->is_free=0;
	 
	 if(header->prev==NULL)		//header->prev is NULL
	 {
	 	//printf("header prev==NULL first element of the list\n");
	 	if(header && header->next && header->is_free==0 && header->next->is_free==0)
	 	{
	 		//printf("inside the first case\n");
	 		header->size+=header->next->size;
	 		//printf("header->size = %d\n",header->size);
	 		//printf("added size \n");
	 		header->size=ALIGN(header->size);
	 		if(header->next->next!=NULL)
	 		{
	 			//printf("inside the header->next->next\n");
	 			header->next=header->next->next;
	 			header->next->prev=header;
	 		}
	 		//printf("size after colescing of curr and next with no prev = %d\n",header->size);
	 	}
	 }
	 if(header->next==NULL)		//header->next=NULL
	 {
	 	//printf("header->next==NULL\n");
	 	if(header && header->prev && header->is_free==0 && header->prev->is_free==0)
	 	{
	 		
	 		header->prev->size+=header->size;
	 		header->prev->size=ALIGN(header->prev->size);
	 		header->prev->next=header->next;
	 		if(header->next)
	 		header->next->prev=header->prev;
	 		//printf("size after colescing prev and cur = %d\n",header->prev->size);
	 	}
	 }
	 else if(header && header->next && header->prev)	//blocks lies inbetween blocks
	 {
	 	//printf("in between the list %d %d %d\n",header->prev->is_free,header->is_free,header->next->is_free);
	 	if(header->prev->is_free==0 && header->is_free==0 && header->next->is_free==0)
	 	{
	 		
	 		
	 		//printf("prev=%d cur=%d next=%d\n",header->prev->size,header->size,header->next->size);
	 		header->prev->size+=(header->size + header->next->size);
	 		header->prev->size=ALIGN(header->prev->size);
	 		header->prev->next=header->next->next;
	 		if(header->next->next)
	 		{
	 			header->next->next->prev=header->prev;
	 		}
	 		//printf("size after colescing prev cur and next is %d\n",header->prev->size);
	 	}
	 	else if(header->prev->is_free==1 && header->is_free==0 && header->next->is_free==0)
	 	{
	 		
	 		header->size+=header->next->size;
	 		header->size=ALIGN(header->size);
	 		header->next=header->next->next;
	 		//printf("working fine\n");
	 		if(header->next!=NULL)
	 		{
	 			//printf("entered inside the if condition\n");
		 		header->next->prev=header;
	 		}
	 		//printf("size after colescing cur and next with prev is %d\n",header->size);
	 	}
	 	else if(header->prev->is_free==0 && header->is_free==0 && header->next->is_free==1)
	 	{
	 		//printf("header->prev->size=%d header->size=%d\n",header->prev->size,header->size);
	 		header->prev->size+=header->size;
	 		header->prev->size=ALIGN(header->prev->size);
	 		header->prev->next=header->next;
	 		header->next->prev=header->prev;
	 		//printf("size after colescing prev and cur is %d\n",header->prev->size);
	 	}
	 	/*else
	 	{
	 		//printf("no one empty in left and right\n");
	 	}*/
	 }
	 //printf("freed size is %d\n",header->size);
}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *ptr, size_t size)
{	
	//printf("+++++++++++++++++going to realloc+++++++++++++++++++++++\n");
	size = ((size+7)/8)*8; //8-byte alignement
	//size=ALIGN(sizeof(header_t) + size);	
	header_t *header;
	
	if(ptr == NULL){			//memory was not previously allocated
		return mm_malloc(size);
	}
	
	if(size == 0){				//new size is zero
		mm_free(ptr);
		return NULL;
	}
	header = (header_t*)ptr-1;
	void *newptr;
	if(header->size >= size)
	{
		return ptr;
	}
	else
	{
		newptr=mm_malloc(size);
			memcpy(newptr,ptr,header->size);
			mm_free(ptr);
			return newptr;
	}
	/*
	 * This function should also copy the content of the previous memory block into the new block.
	 * You can use 'memcpy()' for this purpose.
	 * 
	 * The data structures corresponding to free memory blocks and allocated memory 
	 * blocks should also be updated.
	*/

	
	
}

