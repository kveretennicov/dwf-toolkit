/************************************************************************

  Heap data structure

  Copyright (C) 1998 Michael Garland.  See "COPYING.txt" for details.
  
  $Id: //DWF/Working_Area/Willie.Zhu/w3dtk/hoops/mxtype.c#1 $

 ************************************************************************/

/*
#include "hoops.h"
*/
#include "lod.h"

#ifndef DISABLE_LOD_GENERATION

#ifndef VMS_SYSTEM
# include <memory.h>
#else
# include <stdlib.h>
#endif

static int parent(int i) { return (i-1)/2; }
static int left(int i) { return 2*i+1; }
static int right(int i) { return 2*i+2; }


/*
 * Internal helper functions for manipulating the heap
 */
static void swaph( MxHeap *h, int i, int j )
{
    MxHeapable *hi, *hj;

    swapb( &(h->data), i, j );
    hi = (MxHeapable *) getpb ( &(h->data), i );
    hj = (MxHeapable *) getpb ( &(h->data), j );
    set_heap_pos( hi, i );
    set_heap_pos( hj, j );

}


static void upheaph( MxHeap *h, int i )
{
    MxHeapable *moving = (MxHeapable *) getpb( &(h->data), i );
    MxHeapable *tmp;
    int p = parent(i);
    MX_ASSERT( i >= 0 && i < lengthb( &(h->data) ) );

    while( i > 0 )
    {
        tmp = (MxHeapable *) getpb ( &(h->data), p );
        if( get_heap_key( moving ) > get_heap_key ( tmp ) )
        {
            swaph( h, i, p );
            i = p;
            p = parent( p );
        }
        else
            break;
    }
}

static void downheaph( MxHeap *h, int i )
{
    int l = left(i);
    int r = right(i);
    MxHeapable *moving, *hl, *hr, *largest;
    int oldpos, newpos;
    MX_ASSERT( i >= 0 && i < lengthb( &(h->data) ) );

    /* we are careful here to handle the case where l is in range, but r is not */
    while( l < lengthb( &(h->data) ) )
    {
        moving = (MxHeapable *) getpb ( &(h->data), i );
        hl     = (MxHeapable *) getpb ( &(h->data), l );
        hr     = (MxHeapable *) getpb ( &(h->data), r );
        if( 
            ( r < lengthb( &(h->data) ) ) && 
            ( get_heap_key( hl ) < get_heap_key( hr ) )
        )
            largest = hr;
        else 
            largest = hl;

        if( get_heap_key( moving ) < get_heap_key( largest ) )
        {
            oldpos = get_heap_pos( moving );
            newpos = get_heap_pos( largest );
            swaph( h, oldpos, newpos );
            i = newpos;
            l = left( i );
            r = right( i );
        }
        else
            break;
    }
}

/*
 * Exported interface to the heap
 */

void mxheap_init( MxHeap *h )  
{ 
    /* initialize the block to store MxHeapable *'s */
    block_init( &(h->data), sizeof( MxHeapable * ) ); 
}

void mxheap_cleanup( MxHeap *h )  
{ 
    block_cleanup( &(h->data) ); 
}


void inserth( MxHeap *h, MxHeapable *t )
{
    int i;
#if 0
    double v = get_heap_key( t );
#endif

    i = addpb( &(h->data), t );
    set_heap_pos( t, i );
    upheaph( h, i );
}

void updateh( MxHeap *h, MxHeapable *t )
{
    int i;
    double v = get_heap_key( t );
    MxHeapable *tmp;
    
    i = get_heap_pos( t );
    MX_ASSERT( i >= 0 && i < lengthb( &(h->data) ) );
    tmp = (MxHeapable *) getpb ( &(h->data), parent( i ) );
    if( i > 0 && v > get_heap_key( tmp ) )
        upheaph( h, i );
    else
        downheaph( h, i );
}

/* return and remove the top element of the heap */
void *extracth( MxHeap *h )
{
    MxHeapable *dead;
    int len = lengthb( &(h->data) );
    if( len < 1 ) return NULL;

    swaph( h, 0, len - 1 );
    dead = (MxHeapable *) getpb ( &(h->data), len-1 );
    chopb( &(h->data) );
    not_in_heap( dead );

    downheaph( h, 0 );
    return dead->payload;
}

/* remove a particular element of the heap */
void removeh( MxHeap *h, MxHeapable *t )
{
    MxHeapable *temp;
    int i, end;

    if( !is_in_heap( t ) ) return;
    i = get_heap_pos( t );
    end = lengthb( &(h->data) ) - 1;
    MX_ASSERT( i >= 0 && i <= end );
    not_in_heap( t );
    if( i!= end )
    {
        swaph( h, i, end );
        chopb( &(h->data) );

        temp = (MxHeapable *) getpb ( &(h->data), i );
        if( get_heap_key( temp ) < get_heap_key( t ) )
            downheaph( h, i );
        else
            upheaph( h, i );
    }
    else
        chopb( &(h->data) ); /* remove last element */
        

}

/* access in place without modifying heap.  
 * unused in the LOD module */
void *itemh( MxHeap *h, int i )
{
    MxHeapable *temp;

    MX_ASSERT( i >= 0 && i < lengthb( &(h->data) ) );
    temp = (MxHeapable *) getpb( &(h->data), i ); 
    return temp->payload; 
}


/*
 *  functions for handling blocks
 */

void block_init( Block *b, int size_each_in )
{
    b->allocated = 3;    /* initial size is arbitrary, but must be >= 1 */
    b->used = 0;
    b->size_each = size_each_in;
    b->data   = (void *) MX_ALLOC ( b->allocated * size_each_in );
    if( b->data == NULL )
        MX_ERROR( ERR_INSUFFICIENT_MEMORY, "memory allocation failed" );
}

void block_cleanup( Block *b )
{
    if( b->data  ) { 
        MX_FREE( b->data, b->allocated * b->size_each ); 
        b->data = NULL; 
    }
}

void resetb( Block *b )
{
    b->used = 0;
}


void resizeb( Block *b, int size_in )
{
    void *tmp;
    int oldsize = b->allocated * b->size_each;
    int newsize = size_in * b->size_each;

    tmp = b->data;
    b->data = MX_ALLOC( newsize );
    MX_COPY( tmp, oldsize, b->data );
    MX_FREE( tmp, oldsize );

    /* note: this can move b->data */
    /* MX_REALLOC ( b->data, oldsize, newsize ); */ 

    MX_ASSERT( size_in > b->used );

    if( b->data == NULL )
        MX_ERROR( ERR_INSUFFICIENT_MEMORY, "memory allocation failed" );


    b->allocated = size_in;

} /* end function resizeb */


/* copies the thing pointed to by elem into the right place */
int addb( Block *b, const void *elem )
{
    void *place;
    if( b->used >= b->allocated )
        resizeb( b, b->allocated * 2 );
    place = (char *) b->data + (b->used * b->size_each);
    MX_COPY (elem, b->size_each , place);

    return b->used++;
}

/* as in addb, but specialized for blocks that store pointers, not copies */
int addpb( Block *b, const void *elem )
{
    MX_ASSERT( b->size_each == sizeof( void * ) );
    return addb( b, &elem );
}


/* copies the thing pointed to by elem into the right place */
void clobberb( Block *b, const void *elem, int i )
{
    void *place;
    place = (char *) b->data + (i * b->size_each);
    MX_COPY (elem, b->size_each , place);
}

static void swapb_big( Block *b, int i, int j, void *place1, void *place2 )
{
    void *temp = MX_ALLOC ( b->size_each );

    MX_ASSERT( i >= 0 && i < b->used );
    MX_ASSERT( j >= 0 && j < b->used );

    MX_COPY (place1, b->size_each, temp);
    MX_COPY (place2, b->size_each, place1);
    MX_COPY (temp,   b->size_each, place2);

    MX_FREE( temp, b->size_each );
}

void swapb( Block *b, int i, int j)
{
    char tempspace[12];
    void *temp = tempspace;
    void *place1 = (char *) b->data + (i * b->size_each);
    void *place2 = (char *) b->data + (j * b->size_each);

    if( b->size_each > 12 ) swapb_big( b, i, j, place1, place2 );

    MX_ASSERT( i >= 0 && i < b->used );
    MX_ASSERT( j >= 0 && j < b->used );

    MX_COPY (place1, b->size_each, temp);
    MX_COPY (place2, b->size_each, place1);
    MX_COPY (temp,   b->size_each, place2);

}


void chopb( Block *b )
{
    b->used--;
}

void removeb( Block *b, int which )
{
    swapb( b, which, b->used - 1 );
    chopb( b );
}


/*
 * The rest of the file is functions that were originally defined
 * as inline's in MxType.h
 */

void mxface_init( MxFace *f, MxVertexID v0, MxVertexID v1, MxVertexID v2 )
{
    MX_ASSERT( v0 != v1 );
    MX_ASSERT( v1 != v2 );
    MX_ASSERT( v2 != v0 );
    f->v[0] = v0;
    f->v[1] = v1;
    f->v[2] = v2;
}

int face_find_vertex( const MxFace *f, MxVertexID i )
{
    if( f->v[0]==i ) return 0;
    else if( f->v[1]==i ) return 1;
    else { 
        MX_ASSERT(f->v[2]==i); 
        return 2; 
    }
}

MxVertexID face_opposite_vertex( const MxFace *f, MxVertexID v0, MxVertexID v1 )
{
    if( f->v[0]!=v0 && f->v[0]!=v1 ) return f->v[0];
    else if( f->v[1]!=v0 && f->v[1]!=v1 ) return f->v[1];
    else { 
        MX_ASSERT( f->v[2]!=v0 && f->v[2]!=v1 ); 
        return f->v[2]; 
    }
}

MxBool face_is_inorder( const MxFace *f, MxVertexID v0, MxVertexID v1 )
{
    if( f->v[0]==v0 ) return f->v[1]==v1;
    else if( f->v[1]==v0 ) return f->v[2]==v1;
    else { 
        MX_ASSERT(f->v[2]==v0); 
        return f->v[0]==v1; 
    }
}

int face_remap_vertex( MxFace *f, MxVertexID from, MxVertexID to )
{
    int i;
    int nmapped = 0;

    for( i = 0 ; i < 3 ; i++ )
        if( f->v[i] == from )
        {
            f->v[i] = to;
            nmapped++;
        }
    return nmapped;
}


void mxcolor_init( MxColor *c, double r, double g, double b )
{
    c->r = ftop(r);
    c->g = ftop(g);
    c->b = ftop(b);
    
}

void mxtexcoord_init( MxTexCoord *t, double u, double v )
{
    t->u[0] = u;
    t->u[1] = v;
}

void mxnormal_init( MxNormal *n, double x, double y, double z )
{
    n->dir[0] = x;
    n->dir[1] = y;
    n->dir[2] = z;
}


MxBool fl_find_face( MxFaceList *fl, MxFaceID fid, int *index )
{
    int i;
    for( i = 0 ; i < lengthb( fl ) ; i++ )
        if( fl_get_face( fl, i ) == fid )
        {
            if( index )
                *index = i;
            return 1;
        }
    return 0;
}

MxBool vl_find_vertex( MxVertexList *vl, MxFaceID vid, int *index )
{
    int i;
    for( i = 0 ; i < lengthb( vl ) ; i++ )
        if( vl_get_vertex( vl, i ) == vid )
        {
            if( index )
                *index = i;
            return 1;
        }
    return 0;
}

#endif
