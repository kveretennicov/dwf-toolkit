/*
* Copyright (c) 1998 by Tech Soft 3D, LLC.
* The information contained herein is confidential and proprietary to
* Tech Soft 3D, LLC., and considered a trade secret as defined under
* civil and criminal statutes.	 Tech Soft 3D shall pursue its civil
* and criminal remedies in the event of unauthorized use or misappropriation
* of its trade secrets.  Use of this information by anyone other than
* authorized employees of Tech Soft 3D, LLC. is granted only under a
* written non-disclosure agreement, expressly prescribing the scope and
* manner of such use.
*
* $Id: //DWF/Development/Components/Internal/DWF Toolkit/v7.7/develop/global/src/dwf/w3dtk/utility/vlist.cpp#1 $
*/

#include "vlist.h"

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

_W3DTK_API vlist_t* VLIST_CDECL new_vlist(
	Memory_Pool * memory_pool)
#else
#define VMALLOC(v, p, t) do{(p)=(t*)v->vmalloc(sizeof(t));}while(0)
#define VFREE(v, p, t) do{v->vfree((void*)(p));}while(0)
#define VMALLOC_ARRAY(v, p, c, t) do{(p)=(t*)v->vmalloc(sizeof(t)*(c));}while(0)
#define VFREE_ARRAY(v, p, c, t) do{v->vfree((void*)(p));}while(0)

_W3DTK_API vlist_t* VLIST_CDECL new_vlist(
	void *(VLIST_CDECL *vlist_malloc) (size_t),
	void (VLIST_CDECL *vlist_free) (void *))
#endif
{
	vlist_t *vlist = 0;

#ifdef HOOPS_DEFINED
	if(memory_pool)
		POOL_ALLOC(vlist, vlist_t, memory_pool);
	else
		vlist = (vlist_t *)HI_System_Alloc(sizeof(vlist_t));

	vlist->memory_pool = memory_pool;

#else
	vlist = (vlist_t *) vlist_malloc(sizeof(vlist_t));
	vlist->vmalloc = vlist_malloc;
	vlist->vfree = vlist_free;
#endif

	vlist->head = 0;
	vlist->tail = 0;
	vlist->cursor = 0;
	vlist->cursor_backlink = 0;
	vlist->cursor_index = 0;
	vlist->count = 0;

	return vlist;
}


_W3DTK_API void VLIST_CDECL delete_vlist(
	vlist_t* vlist)
{
	vlist_node_t * head = vlist->head;
	vlist_node_t * tmp = 0;

	while (head) {
		tmp=head;
		head = head->next;
		VFREE(vlist, tmp, vlist_node_t);
	}

	VFREE(vlist, vlist, vlist_t);
}


_W3DTK_API void VLIST_CDECL vlist_add_first(
	vlist_t* vlist,
	void* item) 
{
	vlist_node_t* node;

	VMALLOC(vlist, node, vlist_node_t);
	node->item = item;
	node->next = vlist->head;

	if (!vlist->head) vlist->tail=node;

	vlist->head = node;
	vlist->count++;

	VLIST_RESET_CURSOR(vlist)
}


_W3DTK_API void VLIST_CDECL vlist_add_last(
	vlist_t* vlist,
	void* item) 
{
	vlist_node_t* node;

	if (!vlist->head) {
		vlist_add_first(vlist,item);
		return;
	}

	VMALLOC(vlist, node, vlist_node_t);
	node->item = item;
	node->next = 0;

	vlist->tail->next = node;
	vlist->tail = node;

	vlist->count++;
}


_W3DTK_API void VLIST_CDECL vlist_add_before_cursor(
	vlist_t* vlist,
	void* item) 
{
	vlist_node_t* node;

	if (vlist->cursor == vlist->head) {
		vlist_add_first(vlist,item);
		return;
	}

	VMALLOC(vlist, node, vlist_node_t);
	node->item = item;
	node->next = vlist->cursor;

	if(vlist->cursor_backlink)
		vlist->cursor_backlink->next = node;

	vlist->cursor_backlink=node;
	
	vlist->count++;
}


_W3DTK_API void VLIST_CDECL vlist_add_after_cursor(
	vlist_t* vlist,
	void* item) 
{
	vlist_node_t* node;

	if (!vlist->cursor) {
		vlist_add_first(vlist,item);
		return;
	}

	VMALLOC(vlist, node, vlist_node_t);
	node->item = item;
	node->next = vlist->cursor->next;

	vlist->cursor->next = node;
	if (vlist->cursor == vlist->tail) {
		vlist->tail = node;
	}

	vlist->count++;
}


_W3DTK_API int VLIST_CDECL vlist_remove(
	vlist_t* vlist, 
	void* item)
{
	vlist_node_t* node = vlist->head;
	vlist_node_t** node_last = &vlist->head;
	vlist_node_t* saved_tail = 0;

	while (node) {
		if (node->item == item) {
			*node_last = node->next;	
			if (vlist->tail==node) vlist->tail=saved_tail;

			VFREE(vlist, node, vlist_node_t);
			vlist->count--; 
			VLIST_RESET_CURSOR(vlist)
			return 1;
		}

		node_last=&node->next;
		saved_tail=node;
		node=node->next; 
	}
	return 0;
}


_W3DTK_API void * VLIST_CDECL vlist_remove_first(
	vlist_t* vlist)
{
	vlist_node_t* node = vlist->head;
	void * item;
	
	if (!vlist->head) return 0;

	vlist->head = node->next;

	item = node->item;

	VFREE(vlist, node, vlist_node_t);
	vlist->count--; 

	if (!vlist->head) vlist->tail = 0;

	VLIST_RESET_CURSOR(vlist)
	return item;
}


_W3DTK_API void * VLIST_CDECL vlist_remove_cursor_next(
	vlist_t* vlist)
{
    vlist_node_t* cursor = vlist->cursor;
	vlist_node_t* node;
	void * item;
	
	if (!cursor) return 0;
	if (!cursor->next) return 0;
    node = cursor->next;
	item = node->item;
	if (vlist->tail == node) 
        vlist->tail = cursor;
    cursor->next = node->next;
	VFREE(vlist, node, vlist_node_t);
	vlist->count--; 
	return item;
}

_W3DTK_API void * VLIST_CDECL vlist_remove_at_cursor(
	vlist_t* vlist)
{
	vlist_node_t* cursor = vlist->cursor;
	vlist_node_t* node;
	void * item;
	
	if (!cursor) return 0;

	if (vlist->cursor_backlink)
		vlist->cursor_backlink->next = cursor->next;

	item = cursor->item;
	if (vlist->tail == cursor) 
        vlist->tail = vlist->cursor_backlink;
	if (vlist->head == cursor)
		vlist->head = cursor->next;
	node = cursor;
    vlist->cursor = cursor->next;
	VFREE(vlist, node, vlist_node_t);
	vlist->count--; 
	return item;
}

_W3DTK_API void VLIST_CDECL vlist_reset_cursor(
	vlist_t* vlist)
{
    VLIST_RESET_CURSOR(vlist)
}


_W3DTK_API void * VLIST_CDECL vlist_peek_cursor(
	vlist_t* vlist)
{
    return VLIST_PEEK_CURSOR(vlist);
}


_W3DTK_API void * VLIST_CDECL vlist_peek_cursor_next(
	vlist_t* vlist)
{
	if (!vlist->cursor) return 0;
	if (!vlist->cursor->next) return 0;
	return vlist->cursor->next->item;
}


_W3DTK_API void * VLIST_CDECL vlist_peek_cursor_next_next(
	vlist_t* vlist)
{
	if (!vlist->cursor) return 0;
	if (!vlist->cursor->next) return 0;
	if (!vlist->cursor->next->next) return 0;
	return vlist->cursor->next->next->item;
}


_W3DTK_API void VLIST_CDECL vlist_advance_cursor(
	vlist_t* vlist)
{
    VLIST_ADVANCE_CURSOR(vlist);
}


_W3DTK_API void * VLIST_CDECL vlist_peek_first(
	vlist_t* vlist)
{
	if (!vlist->head) return 0;
	return vlist->head->item;
}


_W3DTK_API void * VLIST_CDECL vlist_peek_last(
	vlist_t* vlist)
{
	if (!vlist->tail) return 0;
	return vlist->tail->item;
}


_W3DTK_API unsigned long VLIST_CDECL vlist_count(
	vlist_t* vlist)
{
	return vlist->count;
}


_W3DTK_API int VLIST_CDECL vlist_item_exists(
	vlist_t* vlist, 
	void* item)
{
	vlist_node_t* node = vlist->head;

	while (node) {
		if (node->item == item) return 1;

		node = node->next;
	}

	return 0;
}

_W3DTK_API void VLIST_CDECL vlist_items_to_array(
	vlist_t* vlist, 
	void ** items)
{
	vlist_node_t* node = vlist->head;
	unsigned long n = 0;

	while (node) {
		items[n] = node->item;	
		n++;
		node = node->next;
    }
}

_W3DTK_API void VLIST_CDECL vlist_map_function(
	vlist_t* vlist, 
	void(*function)(void*, void*),
	void * user_data)
{
	vlist_node_t* node = vlist->head;

	while (node) {
		(*function)(node->item, user_data);
		node = node->next;
    }
}

_W3DTK_API void* VLIST_CDECL vlist_nth_item(
	vlist_t* vlist,
	unsigned long index)
{
	if ((index+1) > vlist->count) 
		return 0;
	else {
		if (!vlist->cursor || (vlist->cursor_index > index)) {
			VLIST_RESET_CURSOR(vlist)
		}
		index -= vlist->cursor_index;
		while (index > 0)
		{
			VLIST_ADVANCE_CURSOR(vlist)
			index--;
		}

		return vlist->cursor->item;
	}
}


_W3DTK_API void VLIST_CDECL vlist_reverse(
	vlist_t* vlist)
{
	vlist_node_t* tail_node = vlist->head;
	vlist_node_t* new_nodes = 0;
	vlist_node_t** linker = &new_nodes;
	vlist_node_t* node = 0;

	while ((node = vlist->head) != 0) {
		vlist->head = vlist->head->next;
		node->next = *linker;
		*linker = node;
	}

	vlist->head = new_nodes;
	vlist->tail = tail_node;

	VLIST_RESET_CURSOR(vlist)
}


_W3DTK_API void VLIST_CDECL vlist_flush(
	vlist_t* vlist)
{
	while (vlist->head) {
		vlist_node_t* node = vlist->head;
		vlist->head = node->next;
		VFREE(vlist, node, vlist_node_t);
	}

	vlist->head = 0;
	vlist->tail = 0;
	vlist->cursor = 0;
	vlist->cursor_backlink = 0;
	vlist->cursor_index = 0;
	vlist->count = 0;
}


struct qs_container {
	vlist_node_t** nodes;
	vcompare_t compare;
	void * temp_item;
	void * data;
};

#ifndef HOOPS_DEFINED
#define QS_USE_SYSTEM_RAND
#endif
#define QS_TYPE				struct qs_container *					
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


_W3DTK_API void VLIST_CDECL vlist_sort(
	vlist_t* vlist,
	vcompare_t compare,
	void * data)
{
	struct qs_container qc;
	vlist_node_t* node = vlist->head;
	unsigned long n = 0;

	qc.compare = compare;
	VMALLOC_ARRAY(vlist, qc.nodes, vlist->count,vlist_node_t*);
	qc.data = data;
	
	while (node) {
		qc.nodes[n++] = node;	
		node = node->next;
    }

	quick_sort(&qc, 0, vlist->count - 1);

	VFREE_ARRAY(vlist, qc.nodes, vlist->count,vlist_node_t*);
}



_W3DTK_API void VLIST_CDECL vlist_add_sorted(
	vlist_t* vlist,
	void* item,
	vcompare_t compare,
	void * data)
{

	vlist_node_t* list_node = vlist->head;
	vlist_node_t* node;

	VMALLOC(vlist, node, vlist_node_t);
	node->item = item;
	node->next = 0;

	/* empty list */
	if (!vlist->head) {
		vlist->tail=node;
		vlist->head = node;
		goto DONE;
	}

	if (compare(node->item, vlist->head->item, data) < 0) {
		node->next = vlist->head;
		vlist->head = node;
		goto DONE;
	}

	if (compare(node->item, vlist->tail->item, data) >= 0) {
		vlist->tail->next = node;
		vlist->tail = node;
		goto DONE;
	}

	/* find the spot */
	list_node = vlist->head;
	while (list_node->next) {
		if (compare(node->item, list_node->next->item, data) < 0) {
			node->next = list_node->next;
			list_node->next = node;
			goto DONE;
		}
		list_node = list_node->next;
	}

	/* WHAT? bug I guess ... it goes last */
	vlist->tail=node;

DONE:
	vlist->count++;
	VLIST_RESET_CURSOR(vlist)
}





