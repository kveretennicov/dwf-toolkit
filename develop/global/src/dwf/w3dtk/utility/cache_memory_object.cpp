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
* $Id: //DWF/Development/Components/Internal/DWF Toolkit/v7.7/develop/global/src/dwf/w3dtk/utility/cache_memory_object.cpp#1 $
*/

#include "cache_memory_object.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// static member variable initialization
void * CacheMemoryObject::free_list_small[8][FREE_LIST_SIZE] = {};
int CacheMemoryObject::cursor_small[8] = { -1, -1, -1, -1, -1, -1, -1, -1 };

CMObject CacheMemoryObject::free_list_large[FREE_LIST_SIZE] = {};
int CacheMemoryObject::cursor_large = -1;

int CacheMemoryObject::ref_count = 0;

CacheMemoryObject::CacheMemoryObject () {
	ref_count++;
}

CacheMemoryObject::~CacheMemoryObject() {
	ref_count--;

	if (ref_count == 0) {
		int i, j;
		for ( i = 0 ; i < 8 ; i++ ) {
			for ( j = 0 ; j <= cursor_small[i] ; j++ )
				free(free_list_small[i][j]);
			
			cursor_small[i] = -1;
		}

		for ( i = 0 ; i <= cursor_large ; i++ )
			free(free_list_large[i].p);

		cursor_large = -1;
	}
}

void * CacheMemoryObject::operator new ( size_t size ) {
	void *p = 0;
	int i;

	if (size <= 4096) {
		size_t base = 32;
		for ( i = 0 ; i < 8 ; i++, base *= 2 ) {
			if (size <= base) {
				if (cursor_small[i] >= 0) {
					p = free_list_small[i][cursor_small[i]];
					if (p) {
						cursor_small[i]--;
					}
				}
				else
					size = base;
				break;
			}
		}
	}
	else {
		for ( i = 0 ; i <= cursor_large ; i++ ) {
			if (size <= free_list_large[i].size) {
				p = free_list_large[i].p;
				int j;
				for ( j = i ; j < cursor_large ; j++ )
					free_list_large[j] = free_list_large[j+1];
				cursor_large--;
				break;
			}
		}
	}

	if (p == 0)
		p = malloc(size);

	return p;
}

void CacheMemoryObject::operator delete ( void * p, size_t size ) throw () {
	bool need_to_free = true;
	int i;

	if (size <= 4096) {
		size_t base = 32;
		for ( i = 0 ; i < 8 ; i++, base *= 2 ) {
			if (size <= base) {
				if (cursor_small[i] < FREE_LIST_SIZE - 1) {
					free_list_small[i][++cursor_small[i]] = p;
					need_to_free = false;
				}
				break;
			}
		}
	}
	else {
		for ( i = 0 ; i <= cursor_large ; i++ ) {
			if (size > free_list_large[i].size) {
				void *old_p = free_list_large[i].p;
				free_list_large[i].p = p;
				free_list_large[i].size = size;
				p = old_p; // get's freed below if needed
				break;
			}
		}

		if (cursor_large < FREE_LIST_SIZE - 1) {
			free_list_large[++cursor_large] = CMObject(p, size);
			need_to_free = false;
		}
	}

	if (need_to_free)
		free(p);
}
