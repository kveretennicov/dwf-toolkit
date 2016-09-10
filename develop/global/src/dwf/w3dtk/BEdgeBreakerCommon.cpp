/*
 * Copyright (c) by Tech Soft 3D, LLC.
 * The information contained herein is confidential and proprietary to
 * Tech Soft 3D, LLC., and considered a trade secret as defined under
 * civil and criminal statutes.  Tech Soft 3D shall pursue its civil
 * and criminal remedies in the event of unauthorized use or misappropriation
 * of its trade secrets.  Use of this information by anyone other than
 * authorized employees of Tech Soft 3D, LLC. is granted only under a
 * written non-disclosure agreement, expressly prescribing the scope and
 * manner of such use.
 *
 * $Header: //DWF/Working_Area/Willie.Zhu/w3dtk/BEdgeBreakerCommon.cpp#1 $
 */

#include <stdlib.h>
#include <memory.h>
#include "dwf/w3dtk/BInternal.h"
#include "dwf/w3dtk/BEdgeBreaker.h"
#include "dwf/w3dtk/utility/vhash.h"

#ifndef BSTREAM_DISABLE_EDGEBREAKER



ET_Action_Table *actions = NULL;
int g_hints = 0;

#ifdef __APPLE_CC__
	#ifdef __i386__
		#undef STREAM_BIGENDIAN
	#else
		#define STREAM_BIGENDIAN
	#endif
#endif

#if defined (DEBUG) && !defined (_WIN32_WCE)
  #include <assert.h>
  #define ASSERT(x) do { if( !(x) ) { assert(0); status = STATUS_ERROR; goto done; } } while(0)
  #define LABEL(x) x
#else
  #define ASSERT(x)
  #define LABEL(x)
#endif

#define COMPLAIN(x) ASSERT(0);
#define alter

#ifdef STREAM_BIGENDIAN
    static int swap32( int in ){
        int out;
        
        out =  (in >> 24) & 0x000000ff; 
        out |= (in & 0x00ff0000) >> 8;
        out |= (in & 0x0000ff00) << 8;
        out |= in << 24;
        return out;
    }
    
    #define SWAP32(i) (\
            (((i) >> 24) & 0x000000ff) |\
            (((i) & 0x00ff0000) >> 8) |\
            (((i) & 0x0000ff00) << 8) |\
            ((i) << 24) \
            )

    static INT16 swap16( INT16 in ){
        INT16 out;
        out =  (in >> 8) & 0x00ff;
        out |= in << 8;
        return out;
    }
#else
    #define swap32(i) (i)
    #define SWAP32(i) (i)
    #define swap16(i) (i)
#endif



void *default_malloc( size_t size, void *user_data )
{
    UNREFERENCED( user_data );
    return malloc( size );
}

void default_free( void *ptr, void *user_data )
{
    UNREFERENCED( user_data );
    free( ptr );
}

void default_new_vertex( int a, int b, int c, void *user_data )
{
    UNREFERENCED( user_data );
    UNREFERENCED( a );
    UNREFERENCED( b );
    UNREFERENCED( c );
}


int int_stack_init( int_stack alter *s ) 
{
    int status = STATUS_NORMAL;
    s->allocated = 6;
    s->data = (int *) EA_MALLOC (s->allocated * sizeof( int ));
    if( s->data == NULL ) {
        COMPLAIN( "int stack init allocation failed" );
    }
    s->used = 0;
    LABEL(done:);
    return status;
}


int int_stack_expand( int_stack alter *s ) 
{
    int status = STATUS_NORMAL;
    int *temp;
    int i;

    ASSERT( s->allocated != 0 );
    ASSERT( s->data != NULL );
    if( s->allocated < 2 )
        s->allocated = 2;
    else
        s->allocated *= 2;
    temp = (int *) EA_MALLOC (s->allocated * sizeof( int ));
    if( temp == NULL ) {
        COMPLAIN( "int stack expand allocation failed" );
    }
    if( s->data ) {
        for( i = 0 ; i < s->used ; i++ ) {
            temp[i] = s->data[i];
        }
        EA_FREE( s->data );     
    }
    s->data = temp;
    LABEL(done:);
    return status;
}

void int_stack_free( int_stack alter *s )
{
    if( s->data != NULL )
        EA_FREE( s->data );
    s->data = NULL;
    s->allocated = 0;
}



int int_stack_pop( int_stack alter *s )
{
    return s->data[ --(s->used) ];
}



int int_stack_pop_internal( int_stack alter *s, int offset, int *out )
{
    int status = STATUS_NORMAL;
    int i, place, return_val;

    ASSERT( s->used > offset );
    place = s->used - offset - 1;
    return_val = s->data[ place ];
    for( i = place ; i < s->used - 1 ; i++ )
        s->data[i] = s->data[i+1];
    s->used--;
    if( out != NULL )
        *out = return_val;
    LABEL(done:);
    return status;
}


void int_stack_push( int_stack alter *s, int n )
{
    if( s->used == s->allocated )
        int_stack_expand( s );
    s->data[ s->used++ ] = n;
}

void int_stack_push_if_unique( int_stack alter *s, int n )
{
    int i;

    for( i = 0 ; i < s->used ; i++ )
        if( s->data[i] == n )
            return;
    int_stack_push( s, n );
}





void mtable_info_init( mtable_info alter *m )
{
    m->flags = 0;
    m->mlengths_used = 0;
    m->mlengths_allocated = 0;
    m->mlengths =  NULL;
    m->m2stackoffsets_used = 0;
    m->m2stackoffsets_allocated = 0;
    m->m2stackoffsets =  NULL;
    m->m2gateoffsets_used = 0;
    m->m2gateoffsets_allocated = 0;
    m->m2gateoffsets = NULL;
    m->dummies_used = 0;
    m->dummies_allocated = 0;
    m->dummies = NULL;
    m->patches_used = 0;
    m->patches_allocated = 0;
    m->patches = NULL;
    m->bounding = NULL;
    m->x_quantization = DEFAULT_QUANTIZATION;
    m->y_quantization = DEFAULT_QUANTIZATION;
    m->z_quantization = DEFAULT_QUANTIZATION;
    m->x_quantization_normals = DEFAULT_QUANTIZATION;
    m->y_quantization_normals = DEFAULT_QUANTIZATION;
    m->z_quantization_normals = DEFAULT_QUANTIZATION;
}


void mtable_info_free( mtable_info alter *m )
{
    if( m->mlengths != NULL ) {
        EA_FREE( m->mlengths );
        m->mlengths = NULL;
    }
    if( m->m2stackoffsets != NULL ) {
        EA_FREE( m->m2stackoffsets );
        m->m2stackoffsets = NULL;
    }
    if( m->m2gateoffsets != NULL ) {
        EA_FREE( m->m2gateoffsets );
        m->m2gateoffsets = NULL;
    }
    if( m->dummies != NULL ) {
        EA_FREE( m->dummies );
        m->dummies = NULL;
    }
    if( m->patches != NULL ) {
        EA_FREE( m->patches );
        m->patches = NULL;
    }
    if( m->bounding != NULL ) {
        EA_FREE( m->bounding );
        m->bounding = NULL;
    }
}


#ifdef DEBUG


int validate_edge ( half_edge_array const *ea, half_edge const *a, loop_table const *ltable )
{
    int status = STATUS_NORMAL;
    int ai, bi;

    ai = (int)(a - ea->edges);
    if( !EDGE_INVALID( a->twin ) ) {
        bi = a->twin;
        if( ea->edges[bi].twin != ai ) {
            ASSERT( ea->edges[bi].twin == ai );
            return 0;
        }
        if( ea->edges[bi].start != ea->edges[HNEXT(ai)].start ) {
            ASSERT( ea->edges[bi].start == ea->edges[HNEXT(ai)].start );
            return 0;
        }
        if( ea->edges[HNEXT(bi)].start != ea->edges[ai].start ) {
            ASSERT( ea->edges[HNEXT(bi)].start == ea->edges[ai].start );
            return 0;
        }
    }

    if( ltable != NULL ) {
        int *P = ltable->P;
        int *N = ltable->N;
        if( !EDGE_INVALID( N[ai] ) ) {
            int length = 0;
            bi = ai;
            do {
                if( EDGE_INVALID( N[bi] ) ){
                    ASSERT( !EDGE_INVALID( N[bi] ) );
                    return 0;
                }
                if( P[N[bi]] != bi ){
                    ASSERT( P[N[bi]] == bi );
                    return 0;
                }
                if( length > 1e5 ){
                    ASSERT( length <= 1e5 );
                    return 0;
                }
                bi = N[bi];
                length++;
            } while( bi != ai );
        }
    }

    LABEL(done:);
    return status;

} 



int validate_associations( 
        int const *associations, 
        int pointcount )
{
    int status = STATUS_NORMAL;

    if( !VERTEX_SPECIAL( associations[0] ) ) {
        if( associations[0] < 0 ) {
            ASSERT( associations[0] >= 0 );
            return 0;
        } 
        if( associations[0] >= pointcount ) {
            ASSERT( associations[0] < pointcount );
            return 0;
        }
    }
    if( !VERTEX_SPECIAL( associations[1] ) ) {
        if( associations[1] < 0 ) {
            ASSERT( associations[1] >= 0 );
            return 0;
        } 
        if( associations[1] >= pointcount ) {
            ASSERT( associations[1] < pointcount );
            return 0;
        }
    }
    if( !VERTEX_SPECIAL( associations[2] ) ){
        if( associations[2] < 0 ) {
            ASSERT( associations[2] >= 0 );
            return 0;
        } 
        if( associations[2] >= pointcount ) {
            ASSERT( associations[2] < pointcount );
            return 0;
        }
    }
    LABEL(done:);
    return status;

} 


#else 

int validate_edge (half_edge_array const *ea, half_edge const *a, int *loops ) {
    UNREFERENCED( ea );
    UNREFERENCED( a );
    UNREFERENCED( loops );
    return 1;
}

int validate_associations( int const *association, int pointcount ) {
    UNREFERENCED( association );
    UNREFERENCED( pointcount );
    return 1;
}

int no_duplications( half_edge_array const *ea, half_edge const *a ) {
    UNREFERENCED( ea );
    UNREFERENCED( a );
    return 1;
}

#endif



void half_edge_copy( half_edge alter *dest, half_edge const *src ) {
    dest->start = src->start;
    dest->twin = src->twin;
}



int half_edge_array_init( half_edge_array alter *ea, int initial_size ) {
    int status = STATUS_NORMAL;
    int i;

    ea->allocated = initial_size;
    ea->edges = (half_edge *) EA_MALLOC (ea->allocated * sizeof( half_edge ));
    ea->used = 0;
    if( ea->edges == NULL ) {
        COMPLAIN( "half edge array init allocation failed" );
        return STATUS_ERROR;
    }
    for( i = 0 ; i < ea->allocated ; i++ )
        HALF_EDGE_INIT( ea->edges + i );
    ea->visitations_used = 0;
    ea->visitations = NULL;
    LABEL(done:);
    return status;
}


void half_edge_array_free( half_edge_array alter *ea ) {
    if( ea->allocated != 0 ) {
        EA_FREE(ea->edges);
        ea->edges = NULL;
    }
    if( ea->visitations != NULL ) {        
        EA_FREE(ea->visitations);
        ea->visitations = NULL;
    }
}


int half_edge_array_expand( half_edge_array alter *ea ) {
    int status = STATUS_NORMAL;
    half_edge *temp;
    int i;

    if( ea->allocated > 5 )
        ea->allocated += ea->allocated / 5;
    else
        ea->allocated += 5;
    temp = (half_edge *) EA_MALLOC (ea->allocated * sizeof( half_edge ));
    if( temp == NULL ) {
        COMPLAIN( "half edge array expand allocation failed" );
        return STATUS_ERROR;
    }
    memcpy( temp, ea->edges, ea->used * sizeof( half_edge ) );
    for( i = ea->used ; i < ea->allocated ; i++ )
        HALF_EDGE_INIT( temp + i );
    EA_FREE( ea->edges );
    ea->edges = temp;
    if( ea->visitations != NULL ) {
        int *itemp;
        itemp = (int *) EA_MALLOC (ea->allocated * sizeof( int ) );
        memcpy( itemp, ea->visitations, ea->visitations_used * sizeof( int ) );
        EA_FREE( ea->visitations );
        ea->visitations = itemp;
    }    
    LABEL(done:);
    return status;
}


int half_edge_array_append( half_edge_array alter *ea, half_edge **out )
{
    int status = STATUS_NORMAL;
    half_edge *h;

    if( ea->used == ea->allocated ) {
        if(( status = half_edge_array_expand( ea )) <= 0 )
            return status;
    }
    h = ea->edges + ea->used++;
    HALF_EDGE_INIT( h );
    *out = h; 
    return status;
}


int old_predict( 
    int const *associations, 
    int const *points, 
    ET_Bounding const *bounding, 
    int x_quantization,
    int y_quantization,
    int z_quantization,
    int alter *out )
{
    int status = STATUS_NORMAL;
    int const *a, *b, *c;

    UNREFERENCED(x_quantization);
    UNREFERENCED(y_quantization);
    UNREFERENCED(z_quantization);

    a = points + (associations[0] * 3);
    b = points + (associations[1] * 3);
    c = points + (associations[2] * 3);
    
    ASSERT( VERTEX_SPECIAL( associations[0] ) || 
              ( a[0] != GARBAGE &&
                a[1] != GARBAGE &&
                a[2] != GARBAGE ) );
    ASSERT( VERTEX_SPECIAL( associations[1] ) || 
              ( b[0] != GARBAGE &&
                b[1] != GARBAGE &&
                b[2] != GARBAGE ) );
    ASSERT( VERTEX_SPECIAL( associations[2] ) || 
              ( c[0] != GARBAGE &&
                c[1] != GARBAGE &&
                c[2] != GARBAGE ) );

    
    if( associations[0] >= 0 && 
        associations[1] >= 0 &&
        associations[2] >= 0 ) {
        
        out[0] = a[0] + b[0] - c[0];
        out[1] = a[1] + b[1] - c[1];
        out[2] = a[2] + b[2] - c[2];
    }
    else if( associations[0] >= 0 ) {
        out[0] = a[0];  
        out[1] = a[1];  
        out[2] = a[2];  
    }
    else if( associations[1] >= 0 ) {
        out[0] = b[0];  
        out[1] = b[1];  
        out[2] = b[2];  
    }
    else if( associations[2] >= 0 ) {
        out[0] = c[0];  
        out[1] = c[1];  
        out[2] = c[2];  
    }
    else {
        out[0] = 0;
        out[1] = 0;
        out[2] = 0;
    }

    (void) bounding;
    LABEL(done:);
    return status;

} 



int lookup_vertex(const vhash_t *proxy_hash, int v) 
{
    void *item;
    int status = 0;
    int proxy = 0;

    if( v == DUMMY_VERTEX )
        return DUMMY_VERTEX;
    ASSERT( proxy_hash != NULL );
    
    #ifdef DEBUG
        if( vhash_lookup_item( proxy_hash, I2V(v), &item ) != VHASH_STATUS_SUCCESS ) {
            COMPLAIN( "internal error: attempt to lookup invalid vertex index\n" );
        }
    #else
        vhash_lookup_item( proxy_hash, I2V(v), &item );
    #endif
    proxy = V2I(item);
  LABEL(done:);
    UNREFERENCED(status);
    return proxy;
}


void predict( 
        half_edge_array const *ea, 
        int ei,                     
        int third_vertex_unknown,   
        char const *touched,
        vhash_t const *proxy_hash,
        int const *quantized_points,
        int alter *prediction_out )
{
    const int *a, *b, *c;
    int ai, bi, ci, twin;
    int status = 0;

    ai = ea->edges[ei].start;
    a = quantized_points + 3 * PROXY_VERTEX_INDEX( proxy_hash, ai );
    bi = ea->edges[HNEXT(ei)].start;
    b = quantized_points + 3 * PROXY_VERTEX_INDEX( proxy_hash, bi );
    ASSERT( ea->edges[ei].twin != GARBAGE_EDGE );
    twin = REAL_EDGE_INDEX(ea->edges[ei].twin);
    ci = ea->edges[HPREV(twin)].start;
    c = quantized_points + 3 * PROXY_VERTEX_INDEX( proxy_hash, ci );
    ASSERT( touched[ PROXY_VERTEX_INDEX( proxy_hash, ai ) ] );
    ASSERT( touched[ PROXY_VERTEX_INDEX( proxy_hash, bi ) ] );
    ASSERT( touched[ PROXY_VERTEX_INDEX( proxy_hash, ci ) ] );
    ASSERT( !touched[ PROXY_VERTEX_INDEX( proxy_hash, third_vertex_unknown ) ] );
    prediction_out[0] = a[0] + b[0] - c[0];
    prediction_out[1] = a[1] + b[1] - c[1];
    prediction_out[2] = a[2] + b[2] - c[2];
    
  LABEL(done:);
    UNREFERENCED(status);
    UNREFERENCED(touched);
    UNREFERENCED(third_vertex_unknown);
}




void vsinit_write( varstream *vs, int size, void *pointer )
{
    int i;

    vs->allocated = size/4;
    vs->data = (unsigned int *) pointer;
    vs->data[0] = 0;
    vs->rdata = NULL;
    vs->used = 0;
    vs->bit = 0;
    vs->rused = 0;
    vs->rbit = 0;
    vs->can_reallocate = 0;
    vs->status = STATUS_NORMAL;
    vs->mask[0] = 0;
    vs->range[0] = 0;
    for( i = 1 ; i <= 32 ; i++ ) {
        vs->mask[i] = ((unsigned int)(-1)) >> (32-i);
        vs->range[i] = (0x1 << (i-1)) - 1;
    }
}



void vsinit_read( varstream *vs, int size, void *pointer )
{
    int status = STATUS_NORMAL;
    int i;

    ASSERT( size % 4 == 0 );
    vs->allocated = size/4;
    vs->data = NULL;
    vs->rdata = (unsigned int *) pointer;
    vs->used = 0;
    vs->bit = 0;
    vs->rused = 0;
    vs->rbit = 0;
    vs->can_reallocate = 0;
    vs->status = status;
    vs->mask[0] = 0;
    vs->range[0] = 0;
    for( i = 1 ; i <= 32 ; i++ ) {
        vs->mask[i] = ((unsigned int)(-1)) >> (32-i);
        vs->range[i] = (0x1 << (i-1)) - 1;
    }
  LABEL(done:);
}


void vsfree( varstream *vs )
{
    if( vs->data != NULL ) {
        if( vs->can_reallocate )
            EA_FREE( vs->data );
        vs->data = NULL;
    }
    
}

void vsswap( varstream *vs ) 
{
#ifdef STREAM_BIGENDIAN
    int i;
    unsigned int *ptr;

    if( vs->data != null )
        ptr = vs->data;
    else
        ptr = vs->rdata;
    for( i = 0 ; i < vs->allocated ; i++ )
        ptr[i] = SWAP32( ptr[i] );
#else
    (void)vs;
#endif
}


static void vsreallocate( varstream *vs )
{

    if( vs->can_reallocate ) {
        unsigned int *temp;
        if( vs->allocated < 2 )
            vs->allocated = 2;
        else
            vs->allocated *= 2;
        temp = (unsigned int *)EA_MALLOC( vs->allocated * sizeof( unsigned int ) );
        if( temp == NULL ) {
            
            vs->status = STATUS_ERROR;
            vs->used = 0; 
        }
        else {
            if( vs->data ) {
                memcpy( temp, vs->data, (vs->used+1) * sizeof( unsigned int ) );
                EA_FREE( vs->data );
            }
            vs->data = temp;
        }
    }
    else {
        
        vs->status = STATUS_ERROR;
        vs->used = 0;
    }
}


static void vsput2( varstream *vs, int numbits, int val ) 
{
    if( vs->bit + numbits <= 32 ) {
        vs->data[vs->used] |= val << (32 - vs->bit - numbits);
        vs->bit += numbits;
    }
    else {
        int shift = (numbits + vs->bit - 32);

        if( vs->used + 1 >= vs->allocated )
            vsreallocate( vs );
        vs->data[vs->used++] |= val >> shift;
        vs->data[vs->used] = val << (32-shift);
        vs->bit += numbits - 32;
    }
}

void vsput( varstream *vs, const int *numbits_array, int val )
{
    int range = 0, i;

    i = 0;
    for(;;){
        range = vs->range[ numbits_array[i] ];
        if( val < -range || val > range ) {
            
            vsput2( vs, numbits_array[i], vs->mask[ numbits_array[i] ] );
            i++;
        }
        else {
            
            vsput2( vs, numbits_array[i], val+range );
            break;
        }
    }
}


static void vsget2( varstream *vs, int numbits, int *val ) 
{
    if( vs->rbit + numbits <= 32 ) {
        *val = (vs->rdata[vs->rused] >> (32 - vs->rbit - numbits)) & vs->mask[numbits];
        vs->rbit += numbits;
    }
    else {
        int shift = (numbits + vs->rbit - 32);
        *val = (vs->rdata[vs->rused++] << shift) & vs->mask[numbits];
        *val |= vs->rdata[vs->rused] >> (32-shift);
        vs->rbit += numbits - 32;
    }
}

int vsget( varstream *vs, const int *numbits_array ) 
{
    int i = 0;
    int val;

    for(;;) {
        vsget2( vs, numbits_array[i], &val );
        if( val == (int)vs->mask[ numbits_array[i] ] )
            i++;
        else
            break;
    }
    val -= vs->range[ numbits_array[i] ];
    return val;
}



#ifndef BSTREAM_READONLY

int old_pack_points( 
        mtable_info *mtable, int alter *associations, edgebreaker_header *hptr,
        int original_pointcount, int const *pointmap, float const *points, 
        int buffsize, void alter *buffer_out, 
        eb_compress_configs const *configs )
{
    int status = STATUS_NORMAL;
    int i;
    int p[3];
    int *quantized_points = NULL;
    ET_Bounding *bounding = NULL;
    float xrange, yrange, zrange;
    int xmaxval, ymaxval, zmaxval;

    ASSERT( configs != NULL );
    xmaxval = MAXVAL(configs->x_quantization);
    ymaxval = MAXVAL(configs->y_quantization);
    zmaxval = MAXVAL(configs->z_quantization);
    if( configs->bounding != NULL ) {
        bounding = configs->bounding;
    }
    else {
        
        ASSERT( mtable->bounding != NULL );
        bounding = mtable->bounding;
    }
    if( bounding->x1 == bounding->x2 )
        xrange = 0;
    else
        xrange = ((float) xmaxval ) / (bounding->x2 - bounding->x1);
    if( bounding->y1 == bounding->y2 )
        yrange = 0;
    else
        yrange = ((float) ymaxval ) / (bounding->y2 - bounding->y1);
    if( bounding->z1 == bounding->z2 )
        zrange = 0;
    else
        zrange = ((float) zmaxval ) / (bounding->z2 - bounding->z1);

    quantized_points = (int *) EA_MALLOC ( original_pointcount * 3 * sizeof( int ) );
    if( quantized_points == NULL ) {
        COMPLAIN( "quantized_points array allocation failed in function old_pack_points()" );
        status = STATUS_ERROR;
        goto done;
    }

    for( i = 0 ; i < original_pointcount ; i ++ ) {
        quantized_points[ i*3 + 0 ] = (int) ((points[ i*3 + 0 ] - bounding->x1) * xrange + 0.5f);
        quantized_points[ i*3 + 1 ] = (int) ((points[ i*3 + 1 ] - bounding->y1) * yrange + 0.5f);
        quantized_points[ i*3 + 2 ] = (int) ((points[ i*3 + 2 ] - bounding->z1) * zrange + 0.5f);
    }

    if( hptr->points_scheme == 0 ) {
        INT16 *diffs = (INT16 *)buffer_out;
        for( i = 0 ; i < hptr->pcount ; i++ ) {
            
            ASSERT( validate_associations( associations + i*3, original_pointcount ) );
            ASSERT( pointmap[i] != GARBAGE_VERTEX );
            ASSERT( quantized_points[ pointmap[i]*3 + 0 ] <= xmaxval );
            ASSERT( quantized_points[ pointmap[i]*3 + 1 ] <= ymaxval );
            ASSERT( quantized_points[ pointmap[i]*3 + 2 ] <= zmaxval );
            
            old_predict( associations + i*3, quantized_points, bounding, 
                    configs->x_quantization, configs->y_quantization, configs->z_quantization, p );
            
            diffs[ i*3 + 0 ] = (INT16)swap16(quantized_points[ pointmap[i]*3 + 0 ] - p[0]);
            diffs[ i*3 + 1 ] = (INT16)swap16(quantized_points[ pointmap[i]*3 + 1 ] - p[1]);
            diffs[ i*3 + 2 ] = (INT16)swap16(quantized_points[ pointmap[i]*3 + 2 ] - p[2]);
        }
        
        hptr->pointslen = hptr->pcount * 6; 
    }
    else {
        varstream vs;
        int bits[8] = { 2, 6, 10, 14, 18, 22, 26, 31 };

        vsinit_write( &vs, buffsize, buffer_out );
        for( i = 0 ; i < hptr->pcount ; i++ ) {
            
            ASSERT( validate_associations( associations + i*3, original_pointcount ) );
            ASSERT( pointmap[i] != GARBAGE_VERTEX );
            ASSERT( quantized_points[ pointmap[i]*3 + 0 ] <= xmaxval );
            ASSERT( quantized_points[ pointmap[i]*3 + 1 ] <= ymaxval );
            ASSERT( quantized_points[ pointmap[i]*3 + 2 ] <= zmaxval );
            
            old_predict( associations + i*3, quantized_points, bounding, 
                    configs->x_quantization, configs->y_quantization, configs->z_quantization, p );
            
            vsput( &vs, bits, quantized_points[ pointmap[i]*3 + 0 ] - p[0] );
            vsput( &vs, bits, quantized_points[ pointmap[i]*3 + 1 ] - p[1] );
            vsput( &vs, bits, quantized_points[ pointmap[i]*3 + 2 ] - p[2] );
        }
        hptr->pointslen = (vs.used + 1) * sizeof( int );
        
        if( vs.status == STATUS_ERROR )
            status = STATUS_TRY_AGAIN;
        vsswap( &vs ); 
    }

    done:
    
    if( quantized_points != NULL )
        EA_FREE( quantized_points );
    return status;

} 

#endif 



int old_unpack_points( 
        int const *associations, 
        edgebreaker_header const *hptr,
        void const *diffs_in, 
        float alter *points_out, 
        ET_Bounding const *bounding, 
        int x_quantization, int y_quantization, int z_quantization )
{
    int status = STATUS_NORMAL;
    int i;
    int *quantized_points;
    int p[3];
    int xmaxval, ymaxval, zmaxval;
    float fx_range_inverse, fy_range_inverse, fz_range_inverse;
    int scheme = hptr->points_scheme;
    int pointcount = hptr->pcount;
    int len = hptr->pointslen;

    quantized_points = (int *) EA_MALLOC ( pointcount * 3 * sizeof( int ) );
    #ifdef DEBUG
    
    for( i = 0 ; i < pointcount * 3 ; i++ )
        points_out[i] = GARBAGE;
    #endif

    xmaxval = MAXVAL(x_quantization);
    ymaxval = MAXVAL(y_quantization);
    zmaxval = MAXVAL(z_quantization);
    if( scheme == 0 ) {
        
        INT16 *diffs = (INT16 *) diffs_in;

        for( i = 0 ; i < pointcount*3 ; i += 3 )        {
            ASSERT( associations[ i + 0 ] < i );
            ASSERT( associations[ i + 1 ] < i );
            ASSERT( associations[ i + 2 ] < i );
            old_predict( associations + i, quantized_points, bounding, 
                    x_quantization, y_quantization, z_quantization, p );
            quantized_points[ i + 0 ] = p[0] + swap16( diffs[ i + 0 ] );
            quantized_points[ i + 1 ] = p[1] + swap16( diffs[ i + 1 ] );
            quantized_points[ i + 2 ] = p[2] + swap16( diffs[ i + 2 ] );
            ASSERT( quantized_points[ i + 0 ] <= xmaxval );
            ASSERT( quantized_points[ i + 1 ] <= ymaxval );
            ASSERT( quantized_points[ i + 2 ] <= zmaxval );
        }
    }
    else {
        varstream vs;
        int bits[8] = { 2, 6, 10, 14, 18, 22, 26, 31 };

        
        vsinit_read( &vs, len, (void *)diffs_in );
        vsswap( &vs );
        for( i = 0 ; i < pointcount*3 ; i += 3 )        {
            ASSERT( associations[ i + 0 ] < i );
            ASSERT( associations[ i + 1 ] < i );
            ASSERT( associations[ i + 2 ] < i );
            old_predict( associations + i, quantized_points, bounding, 
                    x_quantization, y_quantization, z_quantization, p );
            quantized_points[ i + 0 ] = p[0] + vsget( &vs, bits );
            quantized_points[ i + 1 ] = p[1] + vsget( &vs, bits );
            quantized_points[ i + 2 ] = p[2] + vsget( &vs, bits );
            ASSERT( quantized_points[ i + 0 ] <= xmaxval );
            ASSERT( quantized_points[ i + 1 ] <= ymaxval );
            ASSERT( quantized_points[ i + 2 ] <= zmaxval );
        }
    }
    if( bounding->x2 == bounding->x1 )
        fx_range_inverse = 0;
    else
        fx_range_inverse = (bounding->x2 - bounding->x1) / xmaxval;
    if( bounding->y2 == bounding->y1 )
        fy_range_inverse = 0;
    else
        fy_range_inverse = (bounding->y2 - bounding->y1) / ymaxval;
    if( bounding->z2 == bounding->z1 )
        fz_range_inverse = 0;
    else
        fz_range_inverse = (bounding->z2 - bounding->z1) / zmaxval;

    for( i = 0 ; i < pointcount*3 ; i += 3 )    {
        
        if( quantized_points[ i + 0 ] == xmaxval )
            points_out[ i + 0 ] = bounding->x2;
        else
            points_out[ i + 0 ] = bounding->x1 + (fx_range_inverse * quantized_points[ i + 0 ]);
        
        if( quantized_points[ i + 1 ] == ymaxval )
            points_out[ i + 1 ] = bounding->y2;
        else
            points_out[ i + 1 ] = bounding->y1 + (fy_range_inverse * quantized_points[ i + 1 ]);
        
        if( quantized_points[ i + 2 ] == zmaxval )
            points_out[ i + 2 ] = bounding->z2;
        else
            points_out[ i + 2 ] = bounding->z1 + (fz_range_inverse * quantized_points[ i + 2 ]);
    }

    LABEL(done:);
    if( quantized_points != NULL )
        EA_FREE( quantized_points );
    return status;

} 





int old_unpack_normals( 
        int const *associations, 
        edgebreaker_header const *hptr,
        void const *diffs_in, 
        float alter *normals_out, 
        int x_quantization_normals, int y_quantization_normals, int z_quantization_normals )
{
    int status = STATUS_NORMAL;
    int i;
    int *quantized_normals;
    int p[3];
    int xmaxval, ymaxval, zmaxval;
    float fx_range_inverse, fy_range_inverse, fz_range_inverse;
    int pointcount = hptr->pcount;
    int len = hptr->normalslen;
    ET_Bounding bounding;
    varstream vs;
    int bits[8] = { 2, 6, 10, 14, 18, 22, 26, 31 };

    quantized_normals = (int *) EA_MALLOC ( pointcount * 3 * sizeof( int ) );
    xmaxval = MAXVAL(x_quantization_normals);
    ymaxval = MAXVAL(y_quantization_normals);
    zmaxval = MAXVAL(z_quantization_normals);
    bounding.x1 = bounding.y1 = bounding.z1 = -1;
    bounding.x2 = bounding.y2 = bounding.z2 = 1;
    fx_range_inverse = 2.0f / xmaxval;
    fy_range_inverse = 2.0f / ymaxval;
    fz_range_inverse = 2.0f / zmaxval;

    ASSERT( hptr->normals_scheme == 0 );

    
    vsinit_read( &vs, len, (void *)diffs_in );
    vsswap( &vs );
    for( i = 0 ; i < pointcount*3 ; i += 3 )    {
        ASSERT( associations[ i + 0 ] < i );
        ASSERT( associations[ i + 1 ] < i );
        ASSERT( associations[ i + 2 ] < i );
        old_predict( associations + i, quantized_normals, &bounding, 
                x_quantization_normals, y_quantization_normals, z_quantization_normals, p );

        quantized_normals[ i + 0 ] = p[0] + vsget( &vs, bits );
        quantized_normals[ i + 1 ] = p[1] + vsget( &vs, bits );
        quantized_normals[ i + 2 ] = p[2] + vsget( &vs, bits );

        ASSERT( quantized_normals[ i + 0 ] <= xmaxval );
        ASSERT( quantized_normals[ i + 1 ] <= ymaxval );
        ASSERT( quantized_normals[ i + 2 ] <= zmaxval );
    }

    for( i = 0 ; i < pointcount*3 ; i += 3 )    {
        
        if( quantized_normals[ i + 0 ] == xmaxval )
            normals_out[ i + 0 ] = bounding.x2;
        else
            normals_out[ i + 0 ] = bounding.x1 + (fx_range_inverse * quantized_normals[ i + 0 ]);
        
        if( quantized_normals[ i + 1 ] == ymaxval )
            normals_out[ i + 1 ] = bounding.y2;
        else
            normals_out[ i + 1 ] = bounding.y1 + (fy_range_inverse * quantized_normals[ i + 1 ]);
        
        if( quantized_normals[ i + 2 ] == zmaxval )
            normals_out[ i + 2 ] = bounding.z2;
        else
            normals_out[ i + 2 ] = bounding.z1 + (fz_range_inverse * quantized_normals[ i + 2 ]);
    }

    LABEL(done:);
    EA_FREE( quantized_normals );
    return status;

} 

#endif 



