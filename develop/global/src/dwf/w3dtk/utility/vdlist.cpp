
#include "vdlist.h"

#include <stdio.h>
#include <stdlib.h>

#ifdef HOOPS_DEFINED
#define VMALLOC(v, p, t) do{ \
	if (v->memory_pool){ \
	Memory_Pool * mp=v->memory_pool; POOL_ALLOC(p, t, mp); \
	} else { \
	(p)=(t*)HI_System_Alloc(sizeof(t)); \
	} \
} while(0)
#define VFREE(v, p, t) do{ \
	if (v->memory_pool){ \
	Memory_Pool * mp=v->memory_pool;POOL_FREE(p, t, mp); \
	} else { \
	HI_System_Free((void*)(p)); \
	} \
}while(0)
#define VMALLOC_ARRAY(v, p, c, t) do{ \
	if (v->memory_pool) { \
	Memory_Pool * mp=v->memory_pool;POOL_ALLOC_ARRAY(p, c, t, mp); \
	} else { \
	(p)=(t*)HI_System_Alloc(sizeof(t)*(c)); \
	} \
}while(0)
#define VFREE_ARRAY(v, p, c, t) do{ \
	if (v->memory_pool){ \
	Memory_Pool * mp=v->memory_pool;POOL_FREE_ARRAY(p, c, t, mp); \
	} else { \
	HI_System_Free((void*)(p)); \
	} \
}while(0)

VDLIST_API vdlist_t* VDLIST_CDECL new_vdlist(
	Memory_Pool * memory_pool)
#else
#define VMALLOC(v, p, t) do{(p)=(t*)v->vmalloc(sizeof(t));}while(0)
#define VFREE(v, p, t) do{v->vfree((void*)(p));}while(0)
#define VMALLOC_ARRAY(v, p, c, t) do{(p)=(t*)v->vmalloc(sizeof(t)*(c));}while(0)
#define VFREE_ARRAY(v, p, c, t) do{v->vfree((void*)(p));}while(0)

VDLIST_API vdlist_t* VDLIST_CDECL new_vdlist(
	void *(VDLIST_CDECL *vdlist_malloc) (size_t),
	void (VDLIST_CDECL *vdlist_free) (void *))
#endif
{
	vdlist_t *vdlist = 0;

#ifdef HOOPS_DEFINED
	POOL_ALLOC(vdlist, vdlist_t, memory_pool);
	vdlist->memory_pool = memory_pool;
#else
	vdlist = (vdlist_t *) vdlist_malloc(sizeof(vdlist_t));
	vdlist->vmalloc = vdlist_malloc;
	vdlist->vfree = vdlist_free;
#endif

	vdlist->head = 0;
	vdlist->tail = 0;
	vdlist->cursor = 0;
	vdlist->cursor_index = 0;
	vdlist->count = 0;

	return vdlist;
}


VDLIST_API void VDLIST_CDECL delete_vdlist(
	vdlist_t* vdlist)
{
	
	vdlist_node_t * head = vdlist->head;
	vdlist_node_t * tmp = 0;

	while (head) {
		tmp=head;
		head = head->next;
		VFREE(vdlist, tmp, vdlist_node_t);
	}

	VFREE(vdlist, vdlist, vdlist_t);
	
}


VDLIST_API void VDLIST_CDECL vdlist_add_first(
	vdlist_t* vdlist,
	void* item) 
{
	vdlist_node_t* node;

	VMALLOC(vdlist, node, vdlist_node_t);
	node->item = item;
	node->prev = 0;
	
			
/* no item on the list */
	if (!vdlist->head) 
	{
		
		vdlist->tail=node;
		node->next = 0;
	}

	else
	{
		node->next = vdlist->head;
		vdlist->head->prev = node;

	}
	vdlist->head = node;

	vdlist->count++;

	VDLIST_RESET_CURSOR(vdlist)
}


VDLIST_API void VDLIST_CDECL vdlist_add_last(
	vdlist_t* vdlist,
	void* item) 
{
	vdlist_node_t* node;

	if (!vdlist->head) {
		vdlist_add_first(vdlist,item);
		return;
	}

	VMALLOC(vdlist, node, vdlist_node_t);
	node->item = item;
	node->next = 0;

		/* added  for doubly linked list */
		node->prev = vdlist->tail;

	vdlist->tail->next = node;
	vdlist->tail = node;

	vdlist->count++;
}


VDLIST_API void VDLIST_CDECL vdlist_add_before_cursor(
	vdlist_t* vdlist,
	void* item) 
{
	vdlist_node_t* node;

/* If there is no node on the list */

	if (!vdlist->cursor) {
		vdlist_add_first(vdlist,item);
		return;
	}

	VMALLOC(vdlist, node, vdlist_node_t);
	node->item = item;
	node->next = vdlist->cursor;

			/* added  for doubly linked list */
			
			if(vdlist->cursor == vdlist->head)
			{
				vdlist->cursor->prev = node;
				node->prev = 0;
				vdlist->head = node;
				
			}
			else
			{
				node->prev = vdlist->cursor->prev;
				vdlist->cursor->prev = node;
				node->prev->next = node;
			}

	vdlist->count++;
}


VDLIST_API void VDLIST_CDECL vdlist_add_after_cursor(
	vdlist_t* vdlist,
	void* item) 
{
	vdlist_node_t* node;

	if (!vdlist->cursor) {
		vdlist_add_first(vdlist,item);
		return;
	}

	VMALLOC(vdlist, node, vdlist_node_t);
	node->item = item;
	node->next = vdlist->cursor->next;

		
	vdlist->cursor->next = node;
	if (vdlist->cursor == vdlist->tail) {
		vdlist->tail = node;
		/* added  for doubly linked list */
		node->prev = vdlist->cursor;
	

	}

	/* added  for doubly linked list */
	else
	{
			node->next->prev = node;
			node->prev = vdlist->cursor;
	}

	vdlist->count++;
}


VDLIST_API int VDLIST_CDECL vdlist_remove(
	vdlist_t* vdlist, 
	void* item)
{
	vdlist_node_t* node = vdlist->head;

	while (node) {


		if(node ->item == item){

			if(vdlist->head == node)
			{
				vdlist->head = node->next;
				vdlist->head->prev = 0;
			}

				else if (vdlist->tail==node)
				{
					vdlist->tail = node->prev;
					vdlist->tail->next = 0;

				}

					else
					{
						node->prev->next = node->next;
						node->next->prev = node->prev;


					}


			VFREE(vdlist, node, vdlist_node_t);
			vdlist->count--; 
			VDLIST_RESET_CURSOR(vdlist)
			return 1;



		}

		node=node->next;  /* traverse the list */
	}
	return 0;
}


VDLIST_API void * VDLIST_CDECL vdlist_remove_first(
	vdlist_t* vdlist)
{
	vdlist_node_t* node = vdlist->head;
	void * item;
	
	if (!vdlist->head) return 0;

	vdlist->head = node->next;

						/* added  for doubly linked list */
						vdlist->head->prev = 0;

	item = node->item;

	VFREE(vdlist, node, vdlist_node_t);
	vdlist->count--; 

	if (!vdlist->head) vdlist->tail = 0;

	VDLIST_RESET_CURSOR(vdlist)
	return item;
}


VDLIST_API void * VDLIST_CDECL vdlist_remove_cursor_next(
	vdlist_t* vdlist)
{
    vdlist_node_t* cursor = vdlist->cursor;
	vdlist_node_t* node;
	void * item;
	
	if (!cursor) return 0;
	if (!cursor->next) return 0;
    node = cursor->next;
	item = node->item;
	if (vdlist->tail == node) 
        vdlist->tail = cursor;
    cursor->next = node->next;

							
		/* added  for doubly linked list */
		   cursor->next->prev = cursor;


	VFREE(vdlist, node, vdlist_node_t);
	vdlist->count--; 
	return item;
}

VDLIST_API void * VDLIST_CDECL vdlist_remove_at_cursor(
	vdlist_t* vdlist)
{
	vdlist_node_t* cursor = vdlist->cursor;
	vdlist_node_t* node;
	void * item;
	
	if (!cursor) return 0;

	if (vdlist->cursor->prev)
		vdlist->cursor->prev->next = cursor->next;

	item = cursor->item;
	if (vdlist->tail == cursor) 
        vdlist->tail = vdlist->cursor->prev;
	if (vdlist->head == cursor)
		vdlist->head = cursor->next;
	node = cursor;
    vdlist->cursor = cursor->next;
	VFREE(vdlist, node, vdlist_node_t);
	vdlist->count--; 
	return item;
}

VDLIST_API void VDLIST_CDECL vdlist_reset_cursor(
	vdlist_t* vdlist)
{
    VDLIST_RESET_CURSOR(vdlist)
}

VDLIST_API void VDLIST_CDECL vdlist_reset_cursor_back(
	vdlist_t* vdlist)
{
    VDLIST_RESET_CURSOR_BACK(vdlist)
}

VDLIST_API void * VDLIST_CDECL vdlist_peek_cursor(
	vdlist_t* vdlist)
{
    return VDLIST_PEEK_CURSOR(vdlist);
}


VDLIST_API void * VDLIST_CDECL vdlist_peek_cursor_next(
	vdlist_t* vdlist)
{
	if (!vdlist->cursor) return 0;
	if (!vdlist->cursor->next) return 0;
	return vdlist->cursor->next->item;
}


VDLIST_API void * VDLIST_CDECL vdlist_peek_cursor_next_next(
	vdlist_t* vdlist)
{
	if (!vdlist->cursor) return 0;
	if (!vdlist->cursor->next) return 0;
	if (!vdlist->cursor->next->next) return 0;
	return vdlist->cursor->next->next->item;
}

VDLIST_API void * VDLIST_CDECL vdlist_peek_cursor_prev(
	vdlist_t* vdlist)
{
	if (!vdlist->cursor) return 0;
	if (!vdlist->cursor->prev) return 0;
	return vdlist->cursor->prev->item;
}


VDLIST_API void * VDLIST_CDECL vdlist_peek_cursor_prev_prev(
	vdlist_t* vdlist)
{
	if (!vdlist->cursor) return 0;
	if (!vdlist->cursor->prev) return 0;
	if (!vdlist->cursor->prev->prev) return 0;
	return vdlist->cursor->prev->prev->item;
}

VDLIST_API void VDLIST_CDECL vdlist_advance_cursor(
	vdlist_t* vdlist)
{
    VDLIST_ADVANCE_CURSOR(vdlist);
}

VDLIST_API void VDLIST_CDECL vdlist_rewind_cursor(
	vdlist_t* vdlist)
{
    VDLIST_REWIND_CURSOR(vdlist);
}

VDLIST_API void * VDLIST_CDECL vdlist_peek_first(
	vdlist_t* vdlist)
{
	if (!vdlist->head) return 0;
	return vdlist->head->item;
}


VDLIST_API void * VDLIST_CDECL vdlist_peek_last(
	vdlist_t* vdlist)
{
	if (!vdlist->tail) return 0;
	return vdlist->tail->item;
}


VDLIST_API unsigned long VDLIST_CDECL vdlist_count(
	vdlist_t* vdlist)
{
	return vdlist->count;
}


VDLIST_API int VDLIST_CDECL vdlist_item_exists(
	vdlist_t* vdlist, 
	void* item)
{
	vdlist_node_t* node = vdlist->head;

	while (node) {
		if (node->item == item) return 1;

		node = node->next;
	}

	return 0;
}

VDLIST_API void VDLIST_CDECL vdlist_items_to_array(
	vdlist_t* vdlist, 
	void ** items)
{
	vdlist_node_t* node = vdlist->head;
	unsigned long n = 0;

	while (node) {
		items[n] = node->item;	
		n++;
		node = node->next;
    }
}

VDLIST_API void VDLIST_CDECL vdlist_map_function(
	vdlist_t* vdlist, 
	void(*function)(void*, void*),
	void * user_data)
{
	vdlist_node_t* node = vdlist->head;

	while (node) {
		(*function)(node->item, user_data);
		node = node->next;
    }
}

VDLIST_API void* VDLIST_CDECL vdlist_nth_item(
	vdlist_t* vdlist,
	unsigned long index)
{
	if ((index+1) > vdlist->count) 
		return 0;
	else {
		if (!vdlist->cursor || (vdlist->cursor_index > index)) {
			VDLIST_RESET_CURSOR(vdlist)
		}
		index -= vdlist->cursor_index;
		while (index > 0)
		{
			VDLIST_ADVANCE_CURSOR(vdlist)
			index--;
		}

		return vdlist->cursor->item;
	}
}


VDLIST_API void VDLIST_CDECL vdlist_reverse(
	vdlist_t* vdlist)
{
	vdlist_node_t* tail_node = vdlist->head;
	vdlist_node_t* new_nodes = 0;
	vdlist_node_t** linker = &new_nodes;
	vdlist_node_t* node = 0;


	while ((node = vdlist->head) != 0) {

		/* changes the head to the next node */
		vdlist->head = vdlist->head->next;

			/* added for doubly linked lists */
			node->prev = node->next;

		node->next = *linker;
		
		*linker = node;
		
	
		
	}

	vdlist->head = new_nodes;
	vdlist->tail = tail_node;

			/* added for doubly linked lists */

			vdlist->head->prev =0;
			vdlist->tail->next =0;

	VDLIST_RESET_CURSOR(vdlist)
}


VDLIST_API void VDLIST_CDECL vdlist_flush(
	vdlist_t* vdlist)
{
	while (vdlist->head) {
		vdlist_node_t* node = vdlist->head;
		vdlist->head = node->next;
		VFREE(vdlist, node, vdlist_node_t);
	}

	vdlist->head = 0;
	vdlist->tail = 0;
	vdlist->cursor = 0;
	vdlist->cursor_index = 0;
	vdlist->count = 0;
}

struct vdlist_qs_container {
	vdlist_node_t** nodes;
	vcompare_t compare;
	void * temp_item;
	void * data;
};

#ifndef HOOPS_DEFINED
#define QS_USE_SYSTEM_RAND
#endif
#define QS_TYPE				struct vdlist_qs_container *					
#define QS_VAL(thing,a)		((thing)->nodes[(a)]->item)	
#define QS_COMPLEX_COMPARE(thing, i1,i2) ((thing)->compare((thing)->nodes[(i1)]->item, (thing)->nodes[(i2)]->item, (thing)->data) >= 0)
#define QS_SWAP(thing,i1,i2)	( \
	(thing)->temp_item = (thing)->nodes[(i1)]->item, \
	(thing)->nodes[(i1)]->item = (thing)->nodes[(i2)]->item, \
	(thing)->nodes[(i2)]->item = (thing)->temp_item )

#include "hqsort.h"

#undef QS_TYPE
#undef QS_VAL
#undef QS_COMPLEX_COMPARE
#undef QS_SWAP

VDLIST_API void VDLIST_CDECL vdlist_sort(
	vdlist_t* vlist,
	vcompare_t compare,
	void * data)
{
	struct vdlist_qs_container qc;
	vdlist_node_t* node = vlist->head;
	unsigned long n = 0;

	qc.compare = compare;
	VMALLOC_ARRAY(vlist, qc.nodes, vlist->count,vdlist_node_t*);
	qc.data = data;
	
	while (node) {
		qc.nodes[n++] = node;	
		node = node->next;
    }

	quick_sort(&qc, 0, vlist->count - 1);

	VFREE_ARRAY(vlist, qc.nodes, vlist->count,vlist_node_t*);
}


VDLIST_API void VDLIST_CDECL vdlist_add_sorted(
	vdlist_t* vlist,
	void* item,
	vcompare_t compare,
	void * data)
{

	vdlist_node_t* list_node = vlist->head;
	vdlist_node_t* node;

	VMALLOC(vlist, node, vdlist_node_t);
	node->item = item;
	node->next = 0;
	node->prev = 0;

	/* empty list */
	if (!vlist->head) {
		vlist->tail=node;
		vlist->head = node;
		goto DONE;
	}

	if (compare(node->item, vlist->head->item, data) < 0) {
		vlist->head->prev = node;
		node->next = vlist->head;
		vlist->head = node;
		goto DONE;
	}

	if (compare(node->item, vlist->tail->item, data) >= 0) {
		node->prev = vlist->tail;
		vlist->tail->next = node;
		vlist->tail = node;
		goto DONE;
	}

	/* find the spot */
	list_node = vlist->head;
	while (list_node->next) {
		if (compare(node->item, list_node->next->item, data) < 0) {

			node->prev = list_node;
			node->next = list_node->next;
			list_node->next->prev = node;
			list_node->next = node;
			
			goto DONE;
		}
		list_node = list_node->next;
	}

	/* WHAT? bug I guess ... it goes last */
	vlist->tail=node;

DONE:
	vlist->count++;
	VDLIST_RESET_CURSOR(vlist)
}





