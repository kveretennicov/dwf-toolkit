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
* $Id: //DWF/Development/Components/Internal/DWF Toolkit/v7.7/develop/global/src/dwf/w3dtk/utility/vhash.cpp#1 $
*/

#include "vhash.h"


#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _MSC_VER
#pragma warning(disable: 4127 4996)
#endif



#ifdef _WIN64
#define  V_POINTER_SIZED_INT    unsigned __int64
#else
#define  V_POINTER_SIZED_INT    unsigned long
#endif

#define NODE_STATE_NEW(_node_) ((_node_).count == 0)
#define NODE_STATE_OLD(_node_) ((_node_).count < 0)
#define NODE_STATE_IN_USE(_node_) ((_node_).count > 0)

#define MARK_OLD_NODE(_node_) ((_node_).count = -1)

typedef	struct hash_node_s {
    V_POINTER_SIZED_INT		key;
    void *					item;
    int						count;
} vhash_node_t;


typedef struct hash_string_node_s {
    char * 					string;
    void * 					item;
} vhash_string_node_t;

static unsigned long get_power_of_2_size(unsigned long in_size)
{
    register unsigned long out_size;
    for (out_size=1;  out_size < in_size;  out_size = out_size << 1) ;
    return out_size;
}


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

_W3DTK_API vhash_t * VHASH_CDECL new_vhash (
	unsigned long table_size,
	Memory_Pool * memory_pool)
#else
#define VMALLOC(v, p, t) do{(p)=(t*)v->vmalloc(sizeof(t));}while(0)
#define VFREE(v, p, t) do{v->vfree((void*)(p));}while(0)
#define VMALLOC_ARRAY(v, p, c, t) do{(p)=(t*)v->vmalloc(sizeof(t)*(c));}while(0)
#define VFREE_ARRAY(v, p, c, t) do{v->vfree((void*)(p));}while(0)

_W3DTK_API vhash_t * VHASH_CDECL new_vhash (
    unsigned long table_size,
	vmalloc_t vmalloc,
	vfree_t vfree)
#endif
{ 
    vhash_t *v = 0;


#ifdef HOOPS_DEFINED
	if(memory_pool)
		POOL_ALLOC(v, vhash_t, memory_pool);
	else
		v = (vhash_t *) HI_System_Alloc(sizeof(vhash_t));

	v->memory_pool = memory_pool;
#else
	v = (vhash_t *) vmalloc(sizeof(vhash_t));
	v->vmalloc = vmalloc;
	v->vfree = vfree;
#endif
    
    v->table_size = get_power_of_2_size(table_size);

	VMALLOC_ARRAY(v, v->table, v->table_size, vhash_node_t);

    memset(v->table, 0, sizeof(vhash_node_t)*v->table_size);

    v->unique_count = 0;
	v->key_count = 0;

    v->key_string = 0;


    return v;
}

static void vhash_clear(vhash_t * v)
{
    unsigned long		i;
	int					j;

    for (i=0; i<v->table_size; i++) {
		if (NODE_STATE_IN_USE(v->table[i])) {
			if (v->key_string) {
				if (v->table[i].count == 1) {
					VFREE_ARRAY(v, ((vhash_string_node_t*)v->table[i].item)->string, 
						strlen(((vhash_string_node_t*)v->table[i].item)->string)+1, char);
					VFREE(v, v->table[i].item, vhash_string_node_t);
				}
				else {
					void ** items = (void **)v->table[i].item;
					for (j=0; j<v->table[i].count; j++) {
						VFREE_ARRAY(v, ((vhash_string_node_t*)items[j])->string, 
							strlen(((vhash_string_node_t*)items[j])->string)+1, char);
						VFREE(v, items[j], vhash_string_node_t);
					}
				}
			}
			
			if (v->table[i].count > 1) 
				VFREE_ARRAY(v, v->table[i].item, v->table[i].count, void*);
		}
    }

}

_W3DTK_API void VHASH_CDECL vhash_flush(vhash_t * v)
{ 
	vhash_clear(v);
    memset(v->table, 0, sizeof(vhash_node_t)*v->table_size);
    v->unique_count = 0;
	v->key_count = 0;
    v->key_string = 0;
}

_W3DTK_API void VHASH_CDECL delete_vhash(vhash_t * v)
{
	vhash_clear(v);
	VFREE_ARRAY(v, v->table, v->table_size, vhash_node_t);
	VFREE(v, v, vhash_t);
}

#ifdef HOOPS_DEFINED
_W3DTK_API Memory_Pool * VHASH_CDECL vhash_memory_pool(vhash_t * v)
{
	return v->memory_pool;
}
#else
_W3DTK_API vmalloc_t VHASH_CDECL vhash_malloc(vhash_t * v)
{
	return v->vmalloc;
}

_W3DTK_API vfree_t VHASH_CDECL vhash_free(vhash_t * v)
{
	return v->vfree;
}
#endif


_W3DTK_API vhash_status_t VHASH_CDECL vhash_rebuild_table(
	vhash_t *			v,
	unsigned long		table_size)
{
    unsigned long		i;
	int					j;
    unsigned long		old_size = v->table_size;
    vhash_node_t *		old_table = v->table;

    if (table_size < v->unique_count) 
	    table_size = v->unique_count;

    v->table_size = get_power_of_2_size(table_size);
	VMALLOC_ARRAY(v, v->table, v->table_size, vhash_node_t);

    memset(v->table, 0, sizeof(vhash_node_t)*v->table_size);

    v->key_count = 0;
	v->unique_count = 0;

	for (i=0; i<old_size; i++) {
		if (NODE_STATE_IN_USE(old_table[i])) {
			if (old_table[i].count == 1) {
				vhash_insert_item(v, (void *) old_table[i].key, old_table[i].item);
			}
			else {
				void ** items = (void **)old_table[i].item;
				for (j=0; j<old_table[i].count; j++) {
					vhash_insert_item(v, (void *) old_table[i].key, items[j]);
				}
				VFREE_ARRAY(v, items, old_table[i].count, void*);
			}
		}
	}

	VFREE_ARRAY(v, old_table, old_size, vhash_node_t);

    return VHASH_STATUS_SUCCESS;
}

_W3DTK_API unsigned long VHASH_CDECL vhash_count(vhash_t* vhash)
{
    return vhash->key_count;
}

_W3DTK_API unsigned long VHASH_CDECL vhash_unique_keys(vhash_t* vhash)
{
    return vhash->unique_count;
}

_W3DTK_API unsigned long VHASH_CDECL vhash_table_size(vhash_t* vhash)
{
	return vhash->table_size;
}


_W3DTK_API vhash_status_t VHASH_CDECL vhash_replace_item(
	vhash_t *			v,
	void *				in_key, 
	void *				new_item,
	void **				replaced_item)
{
    void *				item = 0;
    vhash_status_t		status = VHASH_STATUS_INSERTED;

    if (vhash_remove_item(v, in_key, &item)) {
		if (replaced_item)
			*replaced_item=item;
    	status = VHASH_STATUS_SUCCESS;
    }
	
    vhash_insert_item(v, in_key, new_item);

    return status;
}

#define HASH_FROM_KEY(key,hash,size) do { \
	hash = key; \
	hash = (hash ^ 61) ^ (hash >> 16); \
    hash = hash + (hash << 3); \
    hash = hash ^ (hash >> 4); \
    hash = hash * 0x27d4eb2d; \
    hash = hash ^ (hash >> 15); \
    hash = (unsigned long) (hash&(size-1)); \
} while(0)


static V_POINTER_SIZED_INT vhash_char_key(
	const vhash_t *		v,
	const char *		string)
{
    V_POINTER_SIZED_INT key = 0;

	/* sdbm */
	int c;
	
	((vhash_t *)v)->key_string = (char*)1;

	while ((c = *string++) != 0)
		key = c + (key << 6) + (key << 16) - key;

    return key;
}

static int string_node_match(void * node, const char *string_key)
{
    vhash_string_node_t *		sn = (vhash_string_node_t *) node;

    if (strcmp(sn->string, string_key) == 0) 
		return 1;
	return 0;
}


_W3DTK_API vhash_status_t VHASH_CDECL vhash_insert_item(
    vhash_t *		v,
    void *			in_key, 
    void *			item)
{ 
    V_POINTER_SIZED_INT key = (V_POINTER_SIZED_INT) in_key;

    V_POINTER_SIZED_INT hash;
    
    if (v->unique_count*2 > v->table_size) {
	    vhash_rebuild_table(v, v->table_size*2);
    }

	HASH_FROM_KEY(key, hash, v->table_size);

    while (NODE_STATE_IN_USE(v->table[hash])) {

		if (v->table[hash].key == key) {
			void ** items = 0;

			if (v->table[hash].count == 1) {
				VMALLOC_ARRAY(v, items, 2, void*);
				items[0] = v->table[hash].item;
				items[1] = item;
			}
			else {
				VMALLOC_ARRAY(v, items, v->table[hash].count+1, void*);
				memcpy(items, v->table[hash].item, sizeof(void *)*v->table[hash].count);
				items[v->table[hash].count] = item;
				VFREE_ARRAY(v, v->table[hash].item, v->table[hash].count, void*);
			}

			v->table[hash].count++;
			v->table[hash].item = (void *)items;
			goto DONE;
		}

	    if (++hash == v->table_size) 
		    hash = 0;
    }

    v->table[hash].key = key;
    v->table[hash].item = item;
    v->table[hash].count = 1;
	v->unique_count++;

DONE:
    v->key_count++;

    return VHASH_STATUS_SUCCESS;
}


_W3DTK_API void VHASH_CDECL vhash_insert(
	vhash_t *		v,
	void *			in_key, 
	void *			item) 
{
    vhash_insert_item(v, in_key, item);
}

static vhash_status_t vhash_remove_nth_item_common(
	vhash_t *				v,
	void *					in_key,
	int						n,
	void **					out_item,
	const char *			string_key)
{
    V_POINTER_SIZED_INT		key = (V_POINTER_SIZED_INT) in_key;
    V_POINTER_SIZED_INT 	hash;
    V_POINTER_SIZED_INT 	first;

	if (string_key)
		key = vhash_char_key(v, string_key);
    
	HASH_FROM_KEY(key, hash, v->table_size);

    first = hash;

    while (1) {
	    if (NODE_STATE_NEW(v->table[hash]))
		    return VHASH_STATUS_FAILED;    	
	
	    if (NODE_STATE_IN_USE(v->table[hash]) && 
			v->table[hash].key == key) {

			if (v->table[hash].count == 1) {
				if (n == 0) {
					if (out_item) {
						if (!string_key) 
							*out_item = v->table[hash].item;
						else {
							vhash_string_node_t *sn = (vhash_string_node_t *)v->table[hash].item;
							*out_item = sn->item;
							/*throw out node*/
							VFREE_ARRAY(v, sn->string, strlen(sn->string)+1, char);
							VFREE(v, sn, vhash_string_node_t);
						}
					}						
					MARK_OLD_NODE(v->table[hash]);
					v->unique_count--; 
					goto DONE;
				}
				else
					if (!string_key ||
						string_node_match(v->table[hash].item, string_key))
						n--;
			}
			else {
				if (!string_key) {
					if (n > v->table[hash].count-1) {
						n -= v->table[hash].count;
					}
					else {				
						void **			old_items = (void **)v->table[hash].item;
						void **			new_items;
						unsigned int	i;
						int				j;
						unsigned int	new_count = v->table[hash].count-1;

						if (out_item)
    						*out_item = old_items[n];

						if (new_count > 1) {
							VMALLOC_ARRAY(v, new_items, new_count, void*);

							i = 0;

							/* fix me sometime */
							for (j=0; j<v->table[hash].count; j++) 
								if (n != j)
									new_items[i++] = old_items[j];

							v->table[hash].item = new_items;
						}
						else {
							if (n == 0) {
								v->table[hash].item = old_items[1];
							}
							else {
								v->table[hash].item = old_items[0];
							}
						}

						VFREE_ARRAY(v, old_items, v->table[hash].count, void*);
						v->table[hash].count = new_count;
						
						goto DONE;
					}
				}
				else {
					void ** old_items = (void **)v->table[hash].item;
					void ** new_items;

					unsigned int	i;
					int				j,k;

					for (j=0; j<v->table[hash].count; j++) {
						if (string_node_match(old_items[j], string_key)) {
							if (n == 0) {
								unsigned int	new_count = v->table[hash].count-1;

								if (out_item) {
									vhash_string_node_t *sn = (vhash_string_node_t *) old_items[j];
									*out_item = sn->item;
									/*throw out node*/
									VFREE_ARRAY(v, sn->string, strlen(sn->string)+1, char);
									VFREE(v, sn, vhash_string_node_t);
								}

								if (new_count > 1) {
									VMALLOC_ARRAY(v, new_items, new_count, void*);

									i = 0;

									/* fix me sometime */
									for (k=0; k<v->table[hash].count; k++) 
										if (j != k)
											new_items[i++] = old_items[k];

									v->table[hash].item = new_items;
								}
								else {
									if (j == 0) {
										v->table[hash].item = old_items[1];
									}
									else {
										v->table[hash].item = old_items[0];
									}
								}

								VFREE_ARRAY(v, old_items, v->table[hash].count, void*);
								v->table[hash].count--;
								
								goto DONE;
							}
							else
								n--;
						}
					}
				}
			}
		}

	    if (++hash == v->table_size) 
			hash = 0;

	    /*full circle*/
	    if (hash == first) 
			return VHASH_STATUS_FAILED;
    }
  
DONE:
	v->key_count--;  

    return VHASH_STATUS_SUCCESS;
}

_W3DTK_API vhash_status_t VHASH_CDECL vhash_remove_nth_string_key_item(
    vhash_t *			v,
    const char *		string_key,
    int					n,
	void **				removed_item)
{
	return vhash_remove_nth_item_common(v,0,n,removed_item,string_key);
}

_W3DTK_API vhash_status_t VHASH_CDECL vhash_remove_nth_item(
	vhash_t *			v,
	void *				in_key,
	int					n,
	void **				removed_item)
{
	return vhash_remove_nth_item_common(v,in_key,n,removed_item,0);
}


_W3DTK_API vhash_status_t VHASH_CDECL vhash_remove_item(
	vhash_t *			v,
	void *				in_key,
	void **				removed_item)
{
	return vhash_remove_nth_item(v, in_key, 0, removed_item); 
}

_W3DTK_API void * VHASH_CDECL vhash_remove(
	vhash_t *			v,
	void *				in_key) 
{ 
    void * item = 0;
    vhash_remove_item(v, in_key, &item);
    return item;
}

static vhash_status_t vhash_lookup_nth_item_common(
	const vhash_t *		v,
	void *				in_key,
	int					n,
	void **				out_item, 
    const char *		string_key)
{
    V_POINTER_SIZED_INT key = (V_POINTER_SIZED_INT) in_key;
    V_POINTER_SIZED_INT	hash;
    V_POINTER_SIZED_INT first;
    
	if (string_key)
    	key = vhash_char_key(v, string_key);

	HASH_FROM_KEY(key, hash, v->table_size);

    first = hash;

    while (1) {
	    if (NODE_STATE_NEW(v->table[hash])) 
			return VHASH_STATUS_FAILED;

	    if (NODE_STATE_IN_USE(v->table[hash]) && 
			v->table[hash].key == key) {

			if (v->table[hash].count == 1) {
				if (n == 0) {
					if (out_item) {
						if (!string_key) 
							*out_item = v->table[hash].item;
						else {
							vhash_string_node_t *sn = (vhash_string_node_t *)v->table[hash].item;
							*out_item = sn->item;
						}
					}
					goto DONE;
				}
				else
					if (!string_key ||
						string_node_match(v->table[hash].item, string_key))
						n--;
			}
			else {
				if (!string_key) {
					if (n > v->table[hash].count-1) {
						n -= v->table[hash].count;
					}
					else {	
						void ** items = (void **)v->table[hash].item;
						
						if (out_item)
							*out_item = items[n];

						goto DONE;
					}

				}
				else {
					void **			items = (void **)v->table[hash].item;
					int				j;

					for (j=0; j<v->table[hash].count; j++) {
						if (string_node_match(items[j], string_key)) {
							if (n == 0) {
								if (out_item) {
									vhash_string_node_t *	sn = (vhash_string_node_t *) items[j];
									*out_item = sn->item;
								}

								goto DONE;
							}
							else
								n--;
						}
					}
				}
			}
	    }
		    
	    if (++hash == v->table_size) hash=0;

	    /*full circle*/
	    if (hash == first) 
			return VHASH_STATUS_FAILED;
    }

DONE:
    return VHASH_STATUS_SUCCESS;
}

_W3DTK_API vhash_status_t VHASH_CDECL vhash_lookup_nth_item(
	const vhash_t *		v,
	void *				in_key,
	int					n,
	void **				out_item) 
{
	return vhash_lookup_nth_item_common(v, in_key, n, out_item, 0);
}

_W3DTK_API vhash_status_t VHASH_CDECL vhash_lookup_item(
	const vhash_t *		v,
	void *				in_key,
	void **				out_item) 
{
    return vhash_lookup_nth_item_common(v, in_key, 0, out_item, 0);
}

_W3DTK_API void * VHASH_CDECL vhash_lookup(
	vhash_t *			v,
	void *				in_key) 
{ 
    void * item = 0;
    vhash_lookup_nth_item_common(v, in_key, 0, &item, 0);
    return item;
}

_W3DTK_API vhash_status_t VHASH_CDECL vhash_lookup_nth_string_key_item(
    vhash_t *			v,
    const char *		string_key,
    int					n,
    void **				out_item)
{ 
	return vhash_lookup_nth_item_common(v, 0, n, out_item, string_key);
}

_W3DTK_API vhash_status_t VHASH_CDECL vhash_lookup_string_key_item(
    vhash_t *			v,
    const char *		string_key,
    void **				out_item)
{ 
    return vhash_lookup_nth_item_common(v, 0, 0, out_item, string_key);
}

_W3DTK_API void * VHASH_CDECL vhash_lookup_string_key(
	vhash_t *			v,
	const char *		string_key)
{
    void * item = 0;
    vhash_lookup_nth_item_common(v, 0, 0, &item, string_key); 
    return item; 
}



_W3DTK_API vhash_status_t VHASH_CDECL vhash_insert_string_key_item(
	vhash_t *			v,
	const char *		string, 
	void *				item) 
{ 
    V_POINTER_SIZED_INT key = vhash_char_key(v, string);

    /*build up string node*/
    vhash_string_node_t * sn;
	VMALLOC(v, sn , vhash_string_node_t);
    sn->item = item;
	VMALLOC_ARRAY(v, sn->string, strlen(string)+1, char);
    strcpy(sn->string, string);
    
    return vhash_insert_item(v, (void *) key, (void *) sn);
}

_W3DTK_API void VHASH_CDECL vhash_insert_string_key(
	vhash_t *			v,
	const char *		string, 
	void *				item) 
{ 
    vhash_insert_string_key_item(v,string,item);
}

_W3DTK_API vhash_status_t VHASH_CDECL vhash_remove_string_key_item(
    vhash_t *			v,
    const char *		string_key,
    void **				removed_item)
{ 
	return vhash_remove_nth_string_key_item(v, string_key, 0, removed_item);
}

_W3DTK_API void * VHASH_CDECL vhash_remove_string_key(
    vhash_t *			v,
    const char *		string)
{
    void * item = 0;
    vhash_remove_string_key_item(v, string, &item); 
    return item;
}



_W3DTK_API vhash_status_t VHASH_CDECL vhash_replace_string_key_item(
	vhash_t *			v,
	const char *		string_key, 
	void *				new_item,
	void **				replaced_item)
{
    void *				item = 0;
    vhash_status_t		status = VHASH_STATUS_INSERTED;

    if (vhash_remove_string_key_item(v, string_key, &item)) {
		if (replaced_item)
			*replaced_item = item;
		status = VHASH_STATUS_SUCCESS;
    }
	
    vhash_insert_string_key_item(v, string_key, new_item);

    return status;
}




_W3DTK_API void VHASH_CDECL vhash_map_function(
    vhash_t *				v,
    void					(*function)(void *, void *, void *),
    void *					user_data)
{
    unsigned long			i;
    
    for (i=0; i<v->table_size; i++) {
	    if (NODE_STATE_IN_USE(v->table[i])) {
			if (v->table[i].count == 1) {
				(*function)(v->table[i].item, (void *) v->table[i].key, user_data);
			}
			else {
				void **			items = (void **)v->table[i].item;
				int				j;

				for (j=0; j<v->table[i].count; j++) {
					(*function)(items[j], (void *) v->table[i].key, user_data);
				}
			}
	    }
    }
}

_W3DTK_API void VHASH_CDECL vhash_map_function_with_return(
    vhash_t *				v,
    vhash_map_return_t		(*function)(void *, void *, void *),
    void *					user_data)
{
    unsigned long			i;
    
    for (i=0; i<v->table_size; i++) {
	    if (NODE_STATE_IN_USE(v->table[i])) {
			if (v->table[i].count == 1) {
				vhash_map_return_t	ret = (*function)(v->table[i].item, (void *) v->table[i].key, user_data);

				if (ret & VHASH_MAP_RETURN_DELETE)
				{
					MARK_OLD_NODE(v->table[i]);
					v->unique_count--;
					v->key_count--;
				}

				if (ret & VHASH_MAP_RETURN_STOP)
					return;
			}
			else {
				void **		items = (void **)v->table[i].item;
				int			j;

				/* XXX */
				for (j=0; j<v->table[i].count; j++) {
					(*function)(items[j], (void *) v->table[i].key, user_data);
				}
			}
	    }
    }
}
    

_W3DTK_API void VHASH_CDECL vhash_string_key_map_function(
    vhash_t *				v,
    void					(*function)(void *, const char *, void *),
    void *					user_data)
{
    unsigned long			i;
    vhash_string_node_t *	sn;
    
    for (i=0; i<v->table_size; i++) {
	    if (NODE_STATE_IN_USE(v->table[i])) {

			if (v->table[i].count == 1) {
				sn = (vhash_string_node_t *) v->table[i].item;
				(*function)(sn->item, sn->string, user_data);
			}
			else {
				void **		items = (void **)v->table[i].item;
				int			j;

				for (j=0; j<v->table[i].count; j++) {
					sn = (vhash_string_node_t *) items[j];
					(*function)(sn->item, sn->string, user_data);
				}
			}
	    }
    }
}

_W3DTK_API void VHASH_CDECL vhash_string_key_map_function_with_return(
    vhash_t *				v,
    vhash_map_return_t		(*function)(void *, const char *, void *),
    void *					user_data)
{
    unsigned long			i;
    vhash_string_node_t *	sn;
    
    for (i=0; i<v->table_size; i++) {
	    if (NODE_STATE_IN_USE(v->table[i])) {
		    vhash_map_return_t	ret;
			
			if (v->table[i].count == 1) {
				sn = (vhash_string_node_t *) v->table[i].item;
				ret = (*function)(sn->item, sn->string, user_data);
				
				if (ret & VHASH_MAP_RETURN_DELETE) {
					MARK_OLD_NODE(v->table[i]);
					v->key_count--;
					v->unique_count--;
				}

				if (ret & VHASH_MAP_RETURN_STOP)
					return;
			}
			else {
				void **		items = (void **)v->table[i].item;
				int			j;

				/* XXX */
				for (j=0; j<v->table[i].count; j++) {
					sn = (vhash_string_node_t *) items[j];
					ret = (*function)(sn->item, sn->string, user_data);
				}
			}
		}
    }
}

struct vhash_to_vlist_data {
#ifdef HOOPS_DEFINED
	Memory_Pool * memory_pool;
#else
	vmalloc_t	vmalloc;
	vfree_t		vfree;
#endif
    vlist_t *	vlist;
};

static void vhash_to_vlist_function(
	void * 		item,
	void * 		key,
	void * 		user_data)
{
    struct vhash_to_vlist_data *	ud = (struct vhash_to_vlist_data*) user_data;	 
    vhash_pair_t * pair;

#ifdef HOOPS_DEFINED
	POOL_ALLOC(pair, vhash_pair_t, ud->memory_pool);
	pair->memory_pool = ud->memory_pool;
#else
    pair = (vhash_pair_t *) ud->vmalloc(sizeof(vhash_pair_t)); 
	pair->free = ud->vfree;
#endif 

    pair->item = item;
    pair->key = key;
	
    vlist_add_first(ud->vlist, (void *) pair);
}

#ifdef HOOPS_DEFINED
_W3DTK_API vhash_status_t VHASH_CDECL vhash_to_vlist(
	vhash_t * 		vhash,
	vlist_t * 		vlist,
	Memory_Pool *	memory_pool)
#else
_W3DTK_API vhash_status_t VHASH_CDECL vhash_to_vlist(
    vhash_t * 		vhash,
    vlist_t * 		vlist,
    void *			(VHASH_CDECL *vhash_pair_malloc) (size_t))
#endif
{
    struct vhash_to_vlist_data ud;

#ifdef HOOPS_DEFINED
	if (memory_pool) {
		ud.memory_pool = memory_pool;
	} else {
		ud.memory_pool = vlist->memory_pool;
	}
#else
	if (vhash_pair_malloc) {
		ud.vmalloc = vhash_pair_malloc;
		ud.vfree = 0;
	} else {
		ud.vmalloc = vlist->vmalloc;
		ud.vfree = vlist->vfree;
	}
#endif

    ud.vlist = vlist;

    vhash_map_function(vhash, vhash_to_vlist_function, (void *)&ud);
    
    return VHASH_STATUS_SUCCESS;
}

static void vhash_string_keys_to_vlist_function(
	void * item, const char* key, void * user_data)
{
    struct vhash_to_vlist_data* ud = (struct vhash_to_vlist_data*) user_data;	 
    vhash_string_key_pair_t * pair;

#ifdef HOOPS_DEFINED
	if (ud->memory_pool)
		POOL_ALLOC(pair, vhash_string_key_pair_t, ud->memory_pool);
	else
		pair = (vhash_string_key_pair_t*) HI_System_Alloc(sizeof(vhash_string_key_pair_t));
	pair->memory_pool = ud->memory_pool;
#else
	pair = (vhash_string_key_pair_t *) ud->vmalloc(sizeof(vhash_string_key_pair_t)); 
	pair->free = ud->vfree;
#endif 

    pair->item = item;
    pair->key = key;

    vlist_add_first(ud->vlist, (void *) pair);

}


#ifdef HOOPS_DEFINED
_W3DTK_API vhash_status_t VHASH_CDECL vhash_string_keys_to_vlist(
	vhash_t * 		vhash,
	vlist_t * 		vlist,
	Memory_Pool *	memory_pool)
#else
_W3DTK_API vhash_status_t VHASH_CDECL vhash_string_keys_to_vlist(
    vhash_t * 			vhash,
    vlist_t * 			vlist,
    void *				(VHASH_CDECL *vhash_pair_malloc) (size_t))
#endif
{
    struct vhash_to_vlist_data ud;

#ifdef HOOPS_DEFINED
	if (memory_pool) {
		ud.memory_pool = memory_pool;
	} else {
		ud.memory_pool = vlist->memory_pool;
	}
#else
	if (vhash_pair_malloc) {
		ud.vmalloc = vhash_pair_malloc;
		ud.vfree = 0;
	} else {
		ud.vmalloc = vlist->vmalloc;
		ud.vfree = vlist->vfree;
	}
#endif

    ud.vlist = vlist;

    vhash_string_key_map_function(vhash, vhash_string_keys_to_vlist_function, (void *)&ud);
    
    return VHASH_STATUS_SUCCESS;
}

_W3DTK_API int VHASH_CDECL vhash_item_set_to_vlist(
	vhash_t *			v,
	vlist_t *			vlist,
	void *				in_key)
{

	V_POINTER_SIZED_INT key = (V_POINTER_SIZED_INT) in_key;

	V_POINTER_SIZED_INT hash;
	V_POINTER_SIZED_INT	first;
	unsigned long n = 0;

	HASH_FROM_KEY(key, hash, v->table_size);

	first = hash;

	while (1) {
		if (NODE_STATE_NEW(v->table[hash]))
			break;

		if (NODE_STATE_IN_USE(v->table[hash]) && 
			v->table[hash].key == key) {

			if (v->table[hash].count == 1) {
				vlist_add_first(vlist, v->table[hash].item);
				n++;
			}
			else {
				void **		items = (void **)v->table[hash].item;
				int			j;

				for (j=0; j<v->table[hash].count; j++) {
					vlist_add_first(vlist, items[j]);
					n++;
				}
			}
		}

		if (++hash == v->table_size) hash=0;

		/*full circle*/
		if (hash == first) 
			break;
	}

	return n;
}


struct vhash_merge_data {
    vhash_t * vhash_to;
	int flags;
};

static vhash_map_return_t vhash_merge_vhash_function(
	void * item, void* key, void * user_data)
{
    struct vhash_merge_data * ud = (struct vhash_merge_data *)user_data;
    vhash_t * vhash_to = (vhash_t *) ud->vhash_to;	 
	int flags = ud->flags;
	int insert_it = 1;

	if (flags & (VHASH_MERGE_REPLACE_DUPLICATES|VHASH_MERGE_PRESERVE_DUPLICATES)) {
		
		if (vhash_lookup_item(vhash_to, key, 0) == VHASH_STATUS_SUCCESS) {
			
			if (flags & VHASH_MERGE_REPLACE_DUPLICATES) {
				vhash_remove_item(vhash_to, key, 0);
			}
			else
				insert_it = 0; /* keep existing */

		}
	}

	if (insert_it) {

		vhash_insert_item(vhash_to, key, item);

		if (flags & VHASH_MERGE_DESTRUCTIVE)
			return VHASH_MAP_RETURN_DELETE;
	}

	return VHASH_MAP_RETURN_CONTINUE;
}

_W3DTK_API vhash_status_t VHASH_CDECL vhash_merge_vhash(
    vhash_t * vhash_to,
	vhash_t * vhash_from,
	int flags)
{
	struct vhash_merge_data ud;

	if (flags & VHASH_MERGE_REPLACE_DUPLICATES &&
		flags & VHASH_MERGE_PRESERVE_DUPLICATES)
		return VHASH_STATUS_FAILED;

	ud.flags = flags;
	ud.vhash_to = vhash_to;

	vhash_map_function_with_return(vhash_from, vhash_merge_vhash_function, &ud);

	return VHASH_STATUS_SUCCESS;
}


static vhash_map_return_t vhash_string_key_merge_vhash_function(
	void * item, const char *key, void * user_data)
{
    struct vhash_merge_data * ud = (struct vhash_merge_data *)user_data;
    vhash_t * vhash_to = (vhash_t *) ud->vhash_to;	 
	int flags = ud->flags;
	int insert_it = 1;

	if (flags & (VHASH_MERGE_REPLACE_DUPLICATES|VHASH_MERGE_PRESERVE_DUPLICATES)) {
		
		if (vhash_lookup_string_key_item(vhash_to, key, 0) == VHASH_STATUS_SUCCESS) {
			
			if (flags & VHASH_MERGE_REPLACE_DUPLICATES) {
				vhash_remove_string_key_item(vhash_to, key, 0);
			}
			else
				insert_it = 0; /* keep existing */

		}
	}

	if (insert_it) {

		vhash_insert_string_key_item(vhash_to, key, item);

		if (flags & VHASH_MERGE_DESTRUCTIVE)
			return VHASH_MAP_RETURN_DELETE;
	}

	return VHASH_MAP_RETURN_CONTINUE;
}

_W3DTK_API vhash_status_t VHASH_CDECL vhash_string_key_merge_vhash(
    vhash_t * vhash_to,
	vhash_t * vhash_from,
	int flags)
{
	struct vhash_merge_data ud;

	if (flags & VHASH_MERGE_REPLACE_DUPLICATES &&
		flags & VHASH_MERGE_PRESERVE_DUPLICATES)
		return VHASH_STATUS_FAILED;

	ud.flags = flags;
	ud.vhash_to = vhash_to;

	vhash_string_key_map_function_with_return(vhash_from, vhash_string_key_merge_vhash_function, &ud);

	return VHASH_STATUS_SUCCESS;
}

