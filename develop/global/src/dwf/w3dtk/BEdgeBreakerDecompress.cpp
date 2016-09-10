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
 * $Header: //DWF/Working_Area/Willie.Zhu/w3dtk/BEdgeBreakerDecompress.cpp#1 $
 */

#include <stdlib.h>
#include <memory.h>

#include "dwf/w3dtk/BStream.h"
#include "dwf/w3dtk/BInternal.h"
#include "dwf/w3dtk/BEdgeBreaker.h"
#include "dwf/w3dtk/utility/varray.h"
#include "dwf/w3dtk/utility/vhash.h"

#ifndef BSTREAM_DISABLE_EDGEBREAKER


#if defined (DEBUG) && !defined (_WIN32_WCE)
  #include <assert.h>
  #define ASSERT(x) do { if( !(x) ) { assert(0); } } while(0)
  #define LABEL(x) x
  #include <stdio.h>
  static FILE *fp = NULL;
  static int debug = 0;
  #ifdef _MSC_VER
    #define COMPLAIN(x) OutputDebugString(x)
  #else
    #define COMPLAIN(x) printf("%s",x)
  #endif
#else
  #define ASSERT(x)
  #define LABEL(x)
  #define COMPLAIN(x)
#endif



#ifdef STREAM_BIGENDIAN
static int swap32( int in ){
    int out;
    
    out =  (in >> 24 ) & 0x000000ff; 
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
    out =  (in >> 8 ) & 0x00ff;
    out |= in << 8;
    return out;
}
#else
#define swap32(i) (i)
#define SWAP32(i) (i)
#define swap16(i) (i)
#endif



int unpack_mtable( void const *src, mtable_info alter *m, vhash_t alter **proxy_hash_out )
{
    int const *in = (int const *) src;
    int i, prev_patch, prev_dummy;
    vhash_t *proxy_hash = NULL;

    m->flags = swap32(*in++);
    if( m->flags & MTABLE_HAS_LENGTHS ) {
        m->mlengths_used = swap32(*in++);
        m->mlengths_allocated = m->mlengths_used;
        m->mlengths = (int *) EA_MALLOC (m->mlengths_allocated * sizeof( int ) );
    }
    else {
        m->mlengths_used = m->mlengths_allocated = 0;
        m->mlengths = NULL;
    }
    if( m->flags & MTABLE_HAS_M2STACKOFFSETS ) {
        ASSERT( m->flags & MTABLE_HAS_M2GATEOFFSETS );
        m->m2stackoffsets_used = swap32(*in++);
        m->m2stackoffsets_allocated = m->m2stackoffsets_used;
        m->m2stackoffsets = (int *) EA_MALLOC (m->m2stackoffsets_allocated * sizeof( int ) );
        m->m2gateoffsets_used = m->m2stackoffsets_used;
        m->m2gateoffsets_allocated = m->m2gateoffsets_used;
        m->m2gateoffsets = (int *) EA_MALLOC (m->m2gateoffsets_allocated * sizeof( int ) );
    }
    else {
        m->m2stackoffsets_used = m->m2stackoffsets_allocated = 0;
        m->m2gateoffsets_used = m->m2gateoffsets_allocated = 0;
        m->m2stackoffsets = NULL;
        m->m2gateoffsets = NULL;
    }
    if( m->flags & MTABLE_HAS_DUMMIES ) {
        m->dummies_used = swap32(*in++);
        m->dummies_allocated = m->dummies_used;
        m->dummies = (int *) EA_MALLOC (m->dummies_allocated * sizeof( int ) );
    }
    else {
        m->dummies_used = m->dummies_allocated = 0;
        m->dummies = NULL;
    }
    if( m->flags & MTABLE_HAS_PATCHES ) {
        m->patches_used = swap32(*in++);
        m->patches_allocated = m->patches_used;
        m->patches = (int *) EA_MALLOC (m->patches_allocated * sizeof( int ) );
        proxy_hash = new_vhash( m->patches_used + 2, malloc, free );
    }
    else {
        m->patches_used = m->patches_allocated = 0;
        m->patches = NULL;
    }   
 
    for( i = 0 ; i < m->mlengths_used ; i++ )
        m->mlengths[i] = swap32(*in++);
    for( i = 0 ; i < m->m2stackoffsets_used ; i++ )
        m->m2stackoffsets[i] = swap32(*in++); 
    for( i = 0 ; i < m->m2gateoffsets_used ; i++ )
        m->m2gateoffsets[i] = swap32(*in++); 

    prev_dummy = 0;
    for( i = 0 ; i < m->dummies_used ; i++ ) {
        m->dummies[i] = swap32(*in++) + prev_dummy; 
        prev_dummy = m->dummies[i];
    }
    ASSERT( (m->patches_used % 2) == 0 );
    prev_patch = 0;
    for( i = 0 ; i < m->patches_used ; i += 2 ) {
        m->patches[i] = swap32(*in++) + prev_patch; 
        prev_patch = m->patches[i];
        m->patches[i+1] = swap32(*in++); 
        vhash_insert_item( proxy_hash, I2V(m->patches[i]), I2V(m->patches[i+1]) );
    }
    
    if( m->flags & MTABLE_HAS_BOUNDING ) {
        int temp[6];
        m->bounding = (ET_Bounding *) EA_MALLOC (sizeof( ET_Bounding ) );
        temp[0] = swap32(*in++);
        temp[1] = swap32(*in++);
        temp[2] = swap32(*in++);
        temp[3] = swap32(*in++);
        temp[4] = swap32(*in++);
        temp[5] = swap32(*in++);
        m->bounding->x1 = *((float *) &temp[0] );
        m->bounding->y1 = *((float *) &temp[1] );
        m->bounding->z1 = *((float *) &temp[2] );
        m->bounding->x2 = *((float *) &temp[3] );
        m->bounding->y2 = *((float *) &temp[4] );
        m->bounding->z2 = *((float *) &temp[5] );
    }
    else
        m->bounding = NULL;


    if( m->flags & MTABLE_HAS_QUANTIZATION ) {
        m->x_quantization = swap32(*in++);
        m->y_quantization = swap32(*in++);
        m->z_quantization = swap32(*in++);
    }
    else
        m->x_quantization = m->y_quantization = m->z_quantization = DEFAULT_QUANTIZATION;
        
    if( m->flags & MTABLE_HAS_QUANTIZATION_NORMALS ) {
        m->x_quantization_normals = swap32(*in++);
        m->y_quantization_normals = swap32(*in++);
        m->z_quantization_normals = swap32(*in++);
    }
    else
        m->x_quantization_normals = m->y_quantization_normals = m->z_quantization_normals = DEFAULT_QUANTIZATION;

    *proxy_hash_out = proxy_hash;
    return 1;

} 





static int decompress_preprocess( int opslen, char const *ops, mtable_info const *mtable, 
            int alter *ecount_out, VArray<int> alter &offsets )
{
    int status = STATUS_NORMAL;
    int i, j, s, length, stackoffset;
    int ecount = 0, scount = 0, mcount = 0, m2count = 0;
    
    int_stack estack;
    
    int_stack sstack;

    int_stack_init( &estack );
    int_stack_init( &sstack );
    
    for( i = 0 ; (i < opslen) && (sstack.used >= 0) ; i++ ) {
        switch( ops[i] ) {
            case CASE_C:
                ecount -= 1;
                break;
            case CASE_L:
                ecount += 1;
                break;
            case CASE_E:
                ecount += 3;
                if( estack.used > 0 ) {
                    s = int_stack_pop( &sstack );
                    offsets[s] = ecount - 2 - int_stack_pop( &estack );
                }
                else
                    
                    goto done;
                break;
            case CASE_R:
                ecount += 1;
                break;
            case CASE_S:
                ecount -= 1;
                int_stack_push( &sstack, scount );
                int_stack_push( &estack, ecount );
                scount += 1;
                break;
            case CASE_M: 
                length = mtable->mlengths[ mcount ];
                mcount += 1;
                ecount -= length + 1;
                break;
            case CASE_M2: 
                length = mtable->mlengths[ m2count ];
                stackoffset = mtable->m2stackoffsets[ m2count ];
                status = int_stack_pop_internal( &sstack, stackoffset, &s );
                ASSERT( status == STATUS_NORMAL );
                status = int_stack_pop_internal( &estack, stackoffset, &j );
                ASSERT( status == STATUS_NORMAL );
                offsets[s] = length + ecount - 2 - j;
                m2count += 1;
                ecount -= 1;
                break;
            default:
                ASSERT(0);
        }
    }
    done:
    int_stack_free( &estack );
    int_stack_free( &sstack );
    *ecount_out = ecount;
    return status;

} 


#ifdef DEBUG
bool has_special_vertices( half_edge_array const *ea )
{
    int i;
    for( i = 0 ; i < ea->used ; i++ ) {
        if( VERTEX_SPECIAL( ea->edges[i].start ) )
            return true;
    }
    return false;
}
#endif


#define DUMMY (GARBAGE_VERTEX-1)
#define ALIAS (GARBAGE_VERTEX-2)

int patch_faces( int opcode_pointcount, 
                mtable_info const *mtable,
                vhash_t const *proxy_hash,
                int alter *flen_in_out, 
                int alter *faces, 
                int alter *associations,
                half_edge_array alter *ea ) 
{
    int i, shift, flen, flen_temp, vhash_result, pcount;
    int *pointmap, *ptr;
    void *item;

    pointmap = (int *) EA_MALLOC ( opcode_pointcount * sizeof( int ) );
    memset( pointmap, 0, opcode_pointcount * sizeof( int ) );
    if( pointmap == NULL ) {
        COMPLAIN( "pointmap array allocation failed in function patch_faces" );
        return 0;
    }
    for( i = 0 ; i < mtable->dummies_used ; i++ ) {
        
        ASSERT( mtable->dummies[i] >= 0 );
        ASSERT( mtable->dummies[i] < opcode_pointcount );
        
        ASSERT( pointmap[ mtable->dummies[i] ] != DUMMY );
        pointmap[ mtable->dummies[i] ] = DUMMY;
    }
    for( i = 0 ; i < mtable->patches_used ; i += 2 ) {
        
        ASSERT( mtable->patches[i] >= 0 );
        ASSERT( mtable->patches[i] < opcode_pointcount );
        
        ASSERT( pointmap[ mtable->patches[i] ] != DUMMY );
        
        ASSERT( pointmap[ mtable->patches[i] ] != ALIAS );
        pointmap[ mtable->patches[i] ] = ALIAS;
    }

    shift = 0;
    for( i = 0 ; i < opcode_pointcount ; i++ ) {
        if( pointmap[i] < 0 )
            shift++;
        else
            pointmap[i] = shift;
    }
    pcount = opcode_pointcount - shift;

    
    flen_temp = flen = *flen_in_out; 
    shift = 0;
    for( i = 0 ; i < flen_temp ; i += 4 ) {
        ASSERT( faces[i] == 3 );  
        ASSERT( faces[i+1] < opcode_pointcount );
        ASSERT( faces[i+2] < opcode_pointcount );
        ASSERT( faces[i+3] < opcode_pointcount );

        if( pointmap[ faces[ i+1 ] ] == DUMMY ||
            pointmap[ faces[ i+2 ] ] == DUMMY ||
            pointmap[ faces[ i+3 ] ] == DUMMY ) {
            
            shift += 4;
            flen -= 4;
        }
        else {  
            faces[ i+1 - shift ] = faces[ i+1 ];
            faces[ i+2 - shift ] = faces[ i+2 ];
            faces[ i+3 - shift ] = faces[ i+3 ];
        }
    }

    
    for( i = 0 ; i < flen ; i++ ) {
        
        if( (i & 0x3) == 0 ) { 
            ASSERT( faces[ i ] == 3 );
            continue;
        }
        
        ASSERT( pointmap[ faces[ i ] ] != DUMMY );
        if( pointmap[ faces[ i ] ] == ALIAS) {
            vhash_result = vhash_lookup_item( proxy_hash, I2V(faces[i]), &item );
            ASSERT( vhash_result == VHASH_STATUS_SUCCESS );
            faces[ i ] = V2I(item);
            ASSERT( faces[ i ] >= 0 );
        }
        else {
            faces[ i ]  -= pointmap[ faces[ i ]  ];
        }
    }

    
    if( associations != NULL ) {
        
        shift = 0;
        for( i = 0 ; i < opcode_pointcount ; i++ ) {
            if( pointmap[ i ] == DUMMY ) {
                shift += 3;
            }
            else {
                associations[ i*3 + 0 - shift ] = associations[ i*3 + 0 ];
                associations[ i*3 + 1 - shift ] = associations[ i*3 + 1 ];
                associations[ i*3 + 2 - shift ] = associations[ i*3 + 2 ];
            }
        }

        ptr = associations;
        for( i = 0 ; i < opcode_pointcount * 3 ; i++ ) {
            
            ASSERT( i%3 || validate_associations( ptr, opcode_pointcount ) );
            ASSERT( ptr[0] < opcode_pointcount );
            if( ptr[0] != GARBAGE_VERTEX ) {
                if( pointmap[ ptr[0] ] == DUMMY )  {
                    ptr[0] = GARBAGE_VERTEX;
                }
                else if( pointmap[ ptr[0] ] == ALIAS ) {
                    vhash_result = vhash_lookup_item( proxy_hash, I2V(ptr[0]), &item );
                    ASSERT( vhash_result == VHASH_STATUS_SUCCESS );
                    ptr[0] = V2I(item);
                    ASSERT( ptr[0] >= 0 );
                }
                else {
                    ptr[0] -= pointmap[ ptr[0] ];
                }
                ASSERT( ptr[0] < i );
            }    
            ptr++;
        }
    }
    if( ea != NULL ) {
        
        ASSERT( !has_special_vertices( ea ) );
        for( i = 0 ; i < ea->used ; i++ ) {
            if( pointmap[ ea->edges[i].start ] == ALIAS ) {
                vhash_result = vhash_lookup_item( proxy_hash, I2V(ea->edges[i].start), &item );
                ASSERT( vhash_result == VHASH_STATUS_SUCCESS );
                ea->edges[i].start = V2I(item);        
            }
            else if( pointmap[ ea->edges[i].start ] == DUMMY ) {
                ea->edges[i].start = DUMMY_VERTEX;

                
                int next, prev;
                next = HNEXT(i);
                prev = HPREV(i);
                if( ea->edges[next].twin != GARBAGE_EDGE )
                    ea->edges[ ea->edges[next].twin ].twin = GARBAGE_EDGE;
                ea->edges[next].start = DUMMY_VERTEX;
                ea->edges[prev].start = DUMMY_VERTEX;
                ea->edges[i].twin = GARBAGE_EDGE;
                ea->edges[next].twin = GARBAGE_EDGE;
                ea->edges[prev].twin = GARBAGE_EDGE;
                
                while( next == i+1 ) {
                    i++; 
                    next = HNEXT(i);
                }
            }
            else {
                ea->edges[i].start -= pointmap[ ea->edges[i].start ];
            }
            ASSERT( ea->edges[i].start < pcount );
        }
    }
    
    *flen_in_out = flen;
    EA_FREE( pointmap );
    return 1;

} 


int old_decompress_loop( half_edge_array alter *ea, int ecount, int vbase, int *N, int *P )
{
    int ebase;
    int i, hi;
    half_edge *h;

    ebase = ea->used;
    
    for( i = 0 ; i < ecount ; i++ ) {
        half_edge_array_append( ea, &h ); 
        h->start = i + vbase;
    }
    
    for( i = 0 ; i < ecount - 1 ; i++ ) {
        P[ ebase + i + 1 ] = ebase + i;
        N[ ebase + i ] = ebase + i+1;
    }
    
    hi = ecount - 1;
    h = ea->edges + hi;
    h->start = hi + vbase;
    P[hi] = ebase + ecount - 2;
    N[hi] = ebase;
    P[ebase] = ebase + ecount - 1;
    return ebase;
    
} 


static void expand_hash( 
        int oldsize, 
        int newsize, 
        int **P, 
        int **N, 
        int **start, 
        int **twin)
{
    int *temp;

    if( P != NULL ) {
        temp = (int *) EA_MALLOC ( newsize * sizeof( int ) );
        memcpy( temp, *P, oldsize * sizeof( int ) );
        EA_FREE( *P );
        *P = temp;
    }

    if( N != NULL ) {
        temp = (int *) EA_MALLOC ( newsize * sizeof( int ) );
        memcpy( temp, *N, oldsize * sizeof( int ) );
        EA_FREE( *N );
        *N = temp;
    }

    if( start != NULL ) {
        temp = (int *) EA_MALLOC ( newsize * sizeof( int ) );
        memcpy( temp, *start, oldsize * sizeof( int ) );
        EA_FREE( *start );
        *start = temp;
    }

    if( twin != NULL ) {
        temp = (int *) EA_MALLOC ( newsize * sizeof( int ) );
        memcpy( temp, *twin, oldsize * sizeof( int ) );
        EA_FREE( *twin );
        *twin = temp;
    }
}



int old_process_opcodes( 
        int opslen, 
        char const *ops, 
        mtable_info const *mtable, 
        int alter *associations,
        int alter *faces, 
        int alter *opcode_pointcount) 
{
    int i, j, k;
    half_edge_array ea; 
    int v = 0;          
    half_edge *b, *h, *gate;
    int_stack gate_stack;
    int ecount, scount = 0;
    int status = STATUS_NORMAL; 
    int vbase = 0;
    int mcount = 0, m2count = 0;
    int loop_start, loop_end, loop_length;
    int hi, bi;
    int gi, gi_start, gi_end;
    int third;          
    int_stack third_stack;
    VArray<int> offsets;
    int hash_allocated = 3 * opslen;
    int *N = NULL, *P = NULL;

    N = (int *) EA_MALLOC ( hash_allocated * sizeof( int ) );
    P = (int *) EA_MALLOC ( hash_allocated * sizeof( int ) );
    half_edge_array_init( &ea, 200 );

    i = 0;
    while( i < opslen ) {
        ea.used = 0;
        decompress_preprocess( opslen - i, ops + i, mtable, &ecount, offsets );
        if( ecount <= 0 ) {
            COMPLAIN( "error in file: invalid edgebreaker data" );
            status = STATUS_ERROR;
            goto done;
        }
        ASSERT( ecount > 0 );
        if( ecount >= hash_allocated ) {
            expand_hash( hash_allocated, ecount*2, &P, &N, NULL, NULL );
            hash_allocated = ecount*2;
        }
        
        old_decompress_loop( &ea, ecount, vbase, N, P );
        gate = ea.edges;
        third = GARBAGE_VERTEX;
        v = ecount + vbase;
        gi = (int)(gate - ea.edges);
        bi = gi;
        do {
            if( bi != gi )
                associations[0] = ea.edges[ P[bi] ].start;
            else
                associations[0] = GARBAGE_VERTEX;
            associations[1] = GARBAGE_VERTEX;
            associations[2] = GARBAGE_VERTEX;   
            ASSERT( associations[0] < v );
            associations += 3;
            bi = N[bi];
        } while ( bi != gi );
    
        int_stack_init( &gate_stack );
        int_stack_init( &third_stack );
        
        while( gate_stack.used >= 0 ) {
            
            gi = (int)(gate - ea.edges);
            gi_start = ea.edges[ gi ].start;
            gi_end = ea.edges[ N[gi] ].start;
            faces[0] = 3;
            faces[1] = gi_start;
            faces[2] = gi_end;
            switch( ops[i++] ) {
                case CASE_C:
                    faces[3] = v; 
                    associations[0] = gi_start;
                    associations[1] = gi_end;
                    associations[2] = third;
                    ASSERT( associations[0] < v &&
                            associations[1] < v &&
                            associations[2] < v );
                    associations += 3;
                    half_edge_array_append( &ea, &h ); 
                    gate = ea.edges + gi;
                    h->start = gate->start;
                    third = gate->start;
                    gate->start = v;
                    
                    hi = (int)(h - ea.edges);
                    P[hi] = P[gi];
                    N[hi] = gi;
                    N[P[gi]] = hi;
                    P[gi] = hi; 
                    v++;
                    break;
                case CASE_L:
                    faces[3] = ea.edges[ P[gi] ].start;
                    third = gate->start;
                    gate->start = ea.edges[ P[gi] ].start;
                    
                    P[gi] = P[P[gi]];
                    N[P[gi]] = gi;
                    break;
                case CASE_E:
                    faces[3] = ea.edges[ P[gi] ].start;
                    if( gate_stack.used > 0 ) {
                        gi = int_stack_pop( &gate_stack );
                        third = int_stack_pop( &third_stack );
                        gate = ea.edges + gi;
                    }
                    else
                        gate_stack.used = -1; 
                    break;
                case CASE_R:
                    faces[3] = ea.edges[ N[N[gi]] ].start;
                    third = ea.edges[ N[gi] ].start;
                    
                    N[gi] = N[N[gi]];
                    P[N[gi]] = gi;
                    break;
                case CASE_S:
                    half_edge_array_append( &ea, &h ); 
                    gate = ea.edges + gi;
                    bi = gi;
                    for( k = 0 ; k <= offsets[ scount ] ; k++ )
                        bi = N[bi];
                    b = ea.edges + bi;
                    faces[3] = ea.edges[N[bi]].start;
                    h->start = gate->start;
                    
                    hi = (int)(h - ea.edges);
                    int_stack_push( &gate_stack, hi );
                    int_stack_push( &third_stack, ea.edges[N[gi]].start );
                    third = gate->start;
                    gate->start = ea.edges[N[bi]].start;
                    
                    N[hi] = N[bi];
                    P[hi] = P[gi];
                    P[N[bi]] = hi;
                    N[P[gi]] = hi;
                    P[gi] = bi;
                    N[bi] = gi;
                    scount++;
                    break;
                case CASE_M: 
                    faces[3] = v;
                    loop_length = mtable->mlengths[ mcount++ ];
                    
                    if( ea.used+ecount+1 >= hash_allocated ) {
                        expand_hash( hash_allocated, ea.used+ecount+100, &P, &N, NULL, NULL );
                        hash_allocated = ea.used+ecount+100;
                    }
                    loop_start = old_decompress_loop( &ea, loop_length, v, N, P );
                    loop_end = P[loop_start];
                    
                    half_edge_array_append( &ea, &h ); 
                    gate = ea.edges + gi;
                    h->start = gate->start;
                    gate->start = v;
                    hi = (int)(h - ea.edges);
                    N[hi] = loop_start;
                    P[hi] = P[gi];
                    N[P[hi]] = hi;
                    P[loop_start] = hi;
                    P[gi] = loop_end;
                    N[loop_end]  = gi;
                    for( k = 0 ; k < loop_length ; k++ ) {
                        associations[0] = GARBAGE_VERTEX;
                        associations[1] = GARBAGE_VERTEX;
                        associations[2] = GARBAGE_VERTEX;
                        associations += 3;
                    }       
                    v += loop_length;
                    break;
                case CASE_M2: 
                    status = int_stack_pop_internal( &gate_stack, mtable->m2stackoffsets[ m2count ], &j );
                    ASSERT( status == STATUS_NORMAL );
                    status = int_stack_pop_internal( &third_stack, mtable->m2stackoffsets[ m2count ], &third );
                    ASSERT( status == STATUS_NORMAL );
                    bi = j;
                    for( k = 0 ; k < mtable->m2gateoffsets[ m2count ] ; k++ )
                        bi = N[bi];
                    loop_end = bi;
                    loop_start = N[bi];
                    faces[3] = ea.edges[N[bi]].start;          
                    
                    half_edge_array_append( &ea, &h ); 
                    gate = ea.edges + gi;
                    
                    hi = (int)(h - ea.edges);
                    h->start = gate->start;
                    gate->start = ea.edges[N[bi]].start;   
                    P[hi] = P[gi];
                    N[P[gi]] = hi;
                    
                    N[loop_end] = gi;
                    P[loop_start] = hi;
                    N[hi] = loop_start;
                    P[gi] = loop_end;
                    ASSERT( validate_edge( &ea, h, NULL ) );
                    mcount++;
                    m2count++;
                    break;
                default:
                    ASSERT(0);
            } 
            ASSERT( faces[1] != faces[2] &&
                    faces[2] != faces[3] &&
                    faces[3] != faces[1] );
            faces += 4;
            ASSERT( validate_edge( &ea, gate, NULL ) );
        } 
        int_stack_free( &gate_stack );
        int_stack_free( &third_stack );
        ea.used = 0;
        vbase = v;
        scount = mcount = m2count = 0;
    } 
    ASSERT( i == opslen );

  done:
    half_edge_array_free( &ea );
    if( N != NULL )
	EA_FREE( N );
    if( P != NULL )
	EA_FREE( P );
    *opcode_pointcount = v;
    return status;

} 


#ifdef DEBUG
int validate_loop( int gi, int const *N, int const *P )
{
    int bi = gi;
    do {
        
        if( P[N[bi]] != bi )
            return 0;
        bi = N[bi];
    } while( bi != gi );
    return 1;
}
int loop_contains_no_freed_edges( int gi, int const *N, int_stack const *available_edges )
{
    int i, bi;
    bi = gi;
    do {
        for( i = 0 ; i < available_edges->used ; i++ ) {
            ASSERT( available_edges->data[i] != bi );
        }
        bi = N[bi];
    } while( bi != gi );
    return 1;
}
#else
int validate_loop( int, int const *, int const * ){ return 1; }
int loop_contains_no_freed_edges( int, int const *, int_stack const * );
#endif


int process_opcodes( 
        int opslen, 
        char const *ops, 
        mtable_info const *mtable, 
        half_edge_array *ea,
        int alter *faces, 
        int alter *opcode_pointcount) 
{
    int i, j, k;
    int ecount;         
    int v = 0;          
    int status = STATUS_NORMAL; 
    VArray<int> offsets;
    int *N = NULL, *P = NULL, *start = NULL, *twin = NULL; 
    int_stack gate_stack;
    int_stack available_edges;
    int hash_allocated = 100, hash_used;
    int ei = 0;     
    int gi, bi, hi; 
    int scount = 0;

    half_edge_array_init( ea, 3*opslen );
    N = (int *) EA_MALLOC ( hash_allocated * sizeof( int ) );
    P = (int *) EA_MALLOC ( hash_allocated * sizeof( int ) );
    start = (int *) EA_MALLOC ( hash_allocated * sizeof( int ) );
    twin = (int *) EA_MALLOC ( hash_allocated * sizeof( int ) );

    
    i = 0;
    int_stack_init( &gate_stack );
    int_stack_init( &available_edges );
    while( i < opslen ) {
        gate_stack.used = 0;
        available_edges.used = 0;
        decompress_preprocess( opslen - i, ops + i, mtable, &ecount, offsets );
        if( ecount <= 0 ) {
            COMPLAIN( "error in file: invalid edgebreaker data" );
            status = STATUS_ERROR;
            goto done;
        }
        ASSERT( ecount > 0 );
        if( ecount >= hash_allocated ) {
            expand_hash( hash_allocated, ecount*2, &P, &N, &start, &twin );
            hash_allocated = ecount*2;
        }
        hash_used = ecount;
        
        for( j = 0 ; j < ecount ; j++ ) {
            P[j] = j-1;
            N[j] = j+1;
            start[j] = v++;
            twin[j] = GARBAGE_EDGE;
        }
        P[0] = ecount - 1;
        N[ecount-1] = 0;
        gi = 0;
        
        while( gate_stack.used >= 0 ) {
            
            faces[0] = 3;
            faces[1] = start[gi];
            faces[2] = start[ N[gi] ];
            
            if( available_edges.used == 0 ) {
                if( hash_used >= hash_allocated ) {
                    expand_hash( hash_allocated, hash_allocated*2, &P, &N, &start, &twin );
                    hash_allocated *= 2;
                }
                int_stack_push( &available_edges, hash_used++ );
            }
            
            if( twin[gi] != GARBAGE_EDGE )
                ea->edges[twin[gi]].twin = ei;
            ea->edges[ ei ].twin = twin[gi];
            ea->edges[ ei ].start = start[gi];
            ea->edges[ ei+1 ].start = start[ N[gi] ];
            switch( ops[i++] ) {
                case CASE_C:
                    faces[3] = v; 
                    ea->edges[ ei+1 ].twin = GARBAGE_EDGE;
                    ea->edges[ ei+2 ].twin = GARBAGE_EDGE;
                    ea->edges[ ei+2 ].start = v;
                    
                    hi = int_stack_pop( &available_edges );
                    N[ P[gi] ] = hi;
                    P[hi] = P[gi];
                    N[hi] = gi;
                    P[gi] = hi;
                    start[hi] = start[gi];
                    start[gi] = v;
                    twin[gi] = ei+1;
                    twin[hi] = ei+2;
                    v++;
                    break;
                case CASE_L:
                    faces[3] = start[ P[gi] ];
                    ea->edges[ ei+1 ].twin = GARBAGE_EDGE;
                    ea->edges[ ei+2 ].twin = twin[ P[gi] ];
                    if( twin[ P[gi] ] != GARBAGE_EDGE )
                        ea->edges[ twin[P[gi]] ].twin = ei+2;
                    ea->edges[ ei+2 ].start = start[ P[gi]];
                    
                    start[ gi ] = start[ P[gi] ];
                    int_stack_push( &available_edges, P[gi] );
                    P[gi] = P[ P[gi] ];
                    N[ P[gi] ] = gi;
                    twin[gi] = ei+1;
                    break;
                case CASE_E:
                    faces[3] = start[ P[gi] ];
                    ea->edges[ ei+1 ].twin = twin[ N[gi] ];
                    if( twin[ N[gi] ] != GARBAGE_EDGE )
                        ea->edges[ twin[N[gi]] ].twin = ei+1;
                    ea->edges[ ei+2 ].twin = twin[ P[gi] ];
                    if( twin[ P[gi] ] != GARBAGE_EDGE )
                        ea->edges[ twin[P[gi]] ].twin = ei+2;
                    ea->edges[ ei+2 ].start = start[ P[gi]];
                    
                    ASSERT( N[N[N[gi]]] == gi );
                    int_stack_push( &available_edges, gi );
                    int_stack_push( &available_edges, P[gi] );
                    int_stack_push( &available_edges, N[gi] );
                    if( gate_stack.used > 0 ) {
                        gi = int_stack_pop( &gate_stack );
                    }
                    else
                        gate_stack.used = -1; 
                    break;
                case CASE_R:
                    faces[3] = start[ N[N[gi]] ];
                    ea->edges[ ei+1 ].twin = twin[ N[gi] ];
                    if( twin[ N[gi] ] != GARBAGE_EDGE )
                        ea->edges[ twin[N[gi]] ].twin = ei+1;
                    ea->edges[ ei+2 ].twin = GARBAGE_EDGE;
                    ea->edges[ ei+2 ].start = start[ N[N[gi]] ];
                    
                    
                    int_stack_push( &available_edges, N[gi] );
                    N[gi] = N[N[gi]];
                    P[N[gi]] = gi;
                    twin[gi] = ei+2;
                    break;
                case CASE_S:
                    bi = gi;
                    for( k = 0 ; k <= offsets[ scount ] ; k++ )
                        bi = N[bi];
                    faces[3] = start[N[bi]];
                    ea->edges[ ei+1 ].twin = GARBAGE_EDGE;
                    ea->edges[ ei+2 ].twin = GARBAGE_EDGE;
                    ea->edges[ ei+2 ].start = start[N[bi]];
                    scount++;
                    
                    hi = int_stack_pop( &available_edges );
                    int_stack_push( &gate_stack, hi );
                    N[P[gi]] = hi;
                    P[hi] = P[gi];
                    N[hi] = N[bi];
                    P[N[bi]] = hi;
                    twin[hi] = ei+2;
                    start[hi] = start[gi];
                    start[gi] = start[N[bi]];
                    twin[gi] = ei+1;
                    P[gi] = bi;
                    N[bi] = gi;
                    break;
                case CASE_M: 
                case CASE_M2: 
                default:
                    ASSERT(0);
            } 
			if ( faces[1] == faces[2] ||
				 faces[2] == faces[3] ||
				 faces[3] == faces[1] ) {
	             COMPLAIN( "error in file: invalid face detected in edgebreaker data" );
				 status = STATUS_ERROR;
				 goto done;
			}
            ASSERT( faces[1] != faces[2] &&
                    faces[2] != faces[3] &&
                    faces[3] != faces[1] );
            faces += 4;
            ASSERT( validate_edge( ea, ea->edges + ei, NULL ) );
            ASSERT( validate_edge( ea, ea->edges + ei + 1, NULL ) );
            ASSERT( validate_edge( ea, ea->edges + ei + 2, NULL ) );
            
            if( gate_stack.used >= 0 ) {
                ASSERT( validate_loop( gi, N, P ) );
                
                ASSERT( i % 20 ||
                        loop_contains_no_freed_edges( gi, N, &available_edges ) );
            }
            ea->used += 3;
            ei += 3;
        } 
        scount = 0;
    }

  done:
    if( N != NULL )
        EA_FREE( N );
    if( P != NULL )
        EA_FREE( P );
    if( start != NULL )
        EA_FREE( start );
    if( twin != NULL )
        EA_FREE( twin );
    int_stack_free( &gate_stack );
    int_stack_free( &available_edges );
    ASSERT( status == STATUS_ERROR || i == opslen );
    *opcode_pointcount = v;
    return status;

} 


int init_actions_table( eb_decompress_configs const *ci )
{
    if( ci == NULL ) {
        actions = (ET_Action_Table *) default_malloc( sizeof( ET_Action_Table ), NULL );
        actions->malloc_action = default_malloc;
        actions->free_action = default_free;
        actions->new_vertex_action = default_new_vertex;
        actions->user_data = NULL;
    }
    else {
        if( ci->malloc_action == NULL ) {
            actions = (ET_Action_Table *) default_malloc( sizeof( ET_Action_Table ), NULL );
            actions->malloc_action = default_malloc;
        }
        else {
            actions = (ET_Action_Table *) ci->malloc_action( sizeof( ET_Action_Table ), ci->user_data );
            actions->malloc_action = ci->malloc_action;
            ASSERT( ci->free_action != NULL );
        }       
        if( ci->free_action == NULL )
            actions->free_action = default_free;
        else
            actions->free_action = ci->free_action;
        if( ci->new_vertex_action == NULL )
            actions->new_vertex_action = default_new_vertex;
        else
            actions->new_vertex_action = ci->new_vertex_action;
        actions->user_data = ci->user_data;
    }
    return STATUS_NORMAL;

} 

#ifdef DEBUG
static int all_points_touched( int pcount, char *touched ) {
    int i;
    for( i = 0 ; i < pcount ; i++ ) {
        ASSERT( touched != 0 );
        if( touched == 0 )
            return 0;
    }
    return 1;
}
#endif


int decode_tristrip( 
        int start_ei,
        int strip_len, 
        half_edge_array const *ea, 
        vhash_t const *proxy_hash, 
        int alter *tristrip ) 
{   
    int status = STATUS_NORMAL;
    int i, ei, v, proxyv;

    ASSERT( strip_len > 0 ); 

    tristrip[0] = strip_len;
    ei = start_ei;
    v = ea->edges[ei].start;
    proxyv = PROXY_VERTEX_INDEX(proxy_hash,v);
    if( proxyv < 0 )
        return STATUS_ERROR;
    tristrip[1] = proxyv;

    v = ea->edges[HNEXT(ei)].start;
    proxyv = PROXY_VERTEX_INDEX(proxy_hash,v);
    if( proxyv < 0 )
        return STATUS_ERROR;
    tristrip[2] = proxyv;

    ei = HNEXT(ei);
    i = 3;
    for(;;) {
        v = ea->edges[HNEXT(ei)].start;
        proxyv = PROXY_VERTEX_INDEX(proxy_hash,v);
        if( proxyv < 0 )
            return STATUS_ERROR;
        tristrip[i] = proxyv;
        if( i == strip_len ) 
            break;
        i++;

        if( ea->edges[ei].twin == GARBAGE_EDGE )
            return STATUS_ERROR;
        ei = ea->edges[ei].twin;
        ei = HPREV(ei);

        v = ea->edges[ei].start;
        proxyv = PROXY_VERTEX_INDEX(proxy_hash,v);
        if( proxyv < 0 )
            return STATUS_ERROR;
        tristrip[i] = proxyv;
        if( i == strip_len ) 
            break;
        i++;  

        if( ea->edges[ei].twin == GARBAGE_EDGE )
            return STATUS_ERROR;
        ei = ea->edges[ei].twin;
        ei = HNEXT(ei);
    }
    return status;
}


#ifndef I2V
    #define I2V(x) ((void *)(POINTER_SIZED_INT)(x))
#endif
#ifndef V2I  
    #define V2I(x) ((POINTER_SIZED_INT)(x))
#endif

#define EKEY(a,b) (I2V(((a)<<8)^(b)))


int decode_all_tristrips( 
        half_edge_array alter *ea, 
        int start_tristrip_id, 
        int *tristrips, 
        int num_tristrips,
        int *tristrip_starts, 
        int *tristrip_lengths, 
        vhash_t const *proxy_hash )
{
    int status = STATUS_NORMAL;
    int i, ei;
    int *tsptr;

    tsptr = tristrips;
    for( i = 0 ; i < start_tristrip_id ; i++ )
        tsptr += tristrip_lengths[i] + 1;
    for( i = start_tristrip_id ; i < num_tristrips ; i++ ) {
        ei = tristrip_starts[i];
        if( decode_tristrip( ei, tristrip_lengths[i], ea, proxy_hash, tsptr ) <= 0 ) {
            
            
            status = STATUS_ERROR;
            COMPLAIN( "decode_all_tristrips failed.  Falling back to triangles\n" );
            goto done;
        }
        ASSERT( tristrip_lengths[i] > 0 ); 
        tsptr += tristrip_lengths[i] + 1;
    }
  done:
    return status;
}



static int unpack_points_and_normals( 
        edgebreaker_header alter *hptr,
        half_edge_array alter *ea, 
        int pcount,
        vhash_t const *proxy_hash, 
        int len, 
        void *data,
        int alter *quantized_points_out,
        int alter *quantized_normals_out,
        int alter *tristrips_len_out,
        int alter **tristrips_out )
{
    int status = STATUS_NORMAL;
    int_stack regular_edges, discontinuous_edges;
    int i, ei, k, v, v2, v3, c;
    char *touched = NULL;
    int *coords = NULL;
    int *diffs = NULL;
    int *normals = NULL;
    int *ndiffs = NULL;
    int *tristrip_starts = NULL;
    int *tristrip_lengths = NULL;
    int *tristrips = NULL;
    int *tsptr = NULL; 
    int p[3];
    int diffs_count = 0, coords_count = 0, next_diff = 0;
    varstream vs;
    static const int bits[9] = { 2, 6, 10, 14, 18, 22, 26, 31, 32 };
    static const int bits2[3] = { 16, 31, 32 };
    
    int eav, proxyv;
    bool by_tristrips = false;
    int next_tristrip_ei = -1, next_tristrip_id = -1, num_tristrips = -1;
    int tristrip_total_length = 0;
    int code;

    touched = (char *) EA_MALLOC( pcount * sizeof( char ) );
    memset( touched, 0, pcount * sizeof( char ) );
    int_stack_init( &regular_edges );
    int_stack_init( &discontinuous_edges );

    vsinit_read( &vs, len, data );
    vsswap( &vs ); 
    code = vsget( &vs, bits );
    by_tristrips = (code != 0);
    coords_count = vsget( &vs, bits2 );
    diffs_count = vsget( &vs, bits2 );
    if( hptr->points_scheme > 0 ) {
        ASSERT( coords_count > 0 && coords_count <= pcount );
        ASSERT( coords_count + diffs_count == pcount );
        ASSERT( quantized_points_out != NULL );
        coords = (int *) EA_MALLOC( 3 * coords_count * sizeof( int ) );
        for( i = 0 ; i < coords_count ; i++ ) {
            coords[i*3+0] = vsget( &vs, bits2 );
            coords[i*3+1] = vsget( &vs, bits2 );
            coords[i*3+2] = vsget( &vs, bits2 );
        }
        diffs = (int *) EA_MALLOC( 3 * diffs_count * sizeof( int ) );
        for( i = 0 ; i < diffs_count ; i++ ) {
            diffs[i*3+0] = vsget( &vs, bits );
            diffs[i*3+1] = vsget( &vs, bits );
            diffs[i*3+2] = vsget( &vs, bits );
        }
    }
    if( hptr->normals_scheme > 0 ) {
        ASSERT( quantized_normals_out != NULL );
        normals = (int *) EA_MALLOC( 3 * coords_count * sizeof( int ) );
        for( i = 0 ; i < coords_count ; i++ ) {
            normals[i*3+0] = vsget( &vs, bits2 );
            normals[i*3+1] = vsget( &vs, bits2 );
            normals[i*3+2] = vsget( &vs, bits2 );
        }
        ndiffs = (int *) EA_MALLOC( 3 * diffs_count * sizeof( int ) );
        for( i = 0 ; i < diffs_count ; i++ ) {
            ndiffs[i*3+0] = vsget( &vs, bits );
            ndiffs[i*3+1] = vsget( &vs, bits );
            ndiffs[i*3+2] = vsget( &vs, bits );
        }
    }
    if( by_tristrips ) {
        if( tristrips_out != NULL ) {
            num_tristrips = vsget( &vs, bits2 );
            ASSERT( num_tristrips > 0 );
            tristrip_starts = (int *) EA_MALLOC( num_tristrips * sizeof( int ) );
            tristrip_lengths = (int *) EA_MALLOC( num_tristrips * sizeof( int ) );
            for( i = 0 ; i < num_tristrips ; i++ ) {
                tristrip_starts[i] = vsget( &vs, bits2 );
                if( i > 0 )
                    tristrip_starts[i] += tristrip_starts[i-1];
            }
            tristrip_total_length = 0;
            for( i = 0 ; i < num_tristrips ; i++ ) {
                code = vsget( &vs, bits );
                ASSERT( code >= 0 );
                tristrip_lengths[i] = code + 3;
                tristrip_total_length += tristrip_lengths[i] + 1;
            }
            tristrips = (int *) EA_MALLOC( tristrip_total_length * sizeof( int ) );
            tsptr = tristrips;
            next_tristrip_ei = tristrip_starts[0];
            next_tristrip_id = 0;
        }
        else
            by_tristrips = false;
    }

    if( quantized_points_out != NULL || quantized_normals_out != NULL ) {
        c = 0;
        for( i = 0 ; i < ea->used ; i++ ) {
            
            ei = i;
            if( ei == next_tristrip_ei ) {
                
                ASSERT( by_tristrips );
                if( decode_tristrip( ei, tristrip_lengths[next_tristrip_id], ea, proxy_hash, tsptr ) <= 0 ){
                    by_tristrips = false;
                    next_tristrip_ei = -1; 
                    
                    COMPLAIN( "tristrip decoding failed.  Falling back to triangles\n" );
                } 
                else {
                    ASSERT( tristrip_lengths[next_tristrip_id] > 0 ); 
                    tsptr += tristrip_lengths[next_tristrip_id] + 1;
                    next_tristrip_id++;
                    if( next_tristrip_id == num_tristrips )
                        next_tristrip_ei = -1;
                    else
                        next_tristrip_ei = tristrip_starts[next_tristrip_id];
                }
            }
            v = PROXY_VERTEX_INDEX(proxy_hash,ea->edges[ei].start);
            if( v == DUMMY_VERTEX )
                continue;
            if( touched[v] )
                continue;
            
            v2 = PROXY_VERTEX_INDEX(proxy_hash,ea->edges[HNEXT(ei)].start);
            v3 = PROXY_VERTEX_INDEX(proxy_hash,ea->edges[HPREV(ei)].start);
            if( v2 == DUMMY_VERTEX || v3 == DUMMY_VERTEX )
                continue;

            
            for( k = 0 ; k < 3 ; k++ ) { 
                v = PROXY_VERTEX_INDEX(proxy_hash,ea->edges[ei].start);
                if( !touched[v] ) {
                    touched[v] = 1; 
                    
                    if( quantized_points_out ) {
                        quantized_points_out[v*3] = coords[c*3];
                        quantized_points_out[v*3+1] = coords[c*3+1];
                        quantized_points_out[v*3+2] = coords[c*3+2];
                    }
                    if( quantized_normals_out ) {
                        quantized_normals_out[v*3] = normals[c*3];
                        quantized_normals_out[v*3+1] = normals[c*3+1];
                        quantized_normals_out[v*3+2] = normals[c*3+2];
                    }
                    c++;
                }
                
                if( EDGE_INVALID( ea->edges[ei].twin ) ) {
                    
                    
                    
                }
                else
                    int_stack_push( &regular_edges, ei );
                ei = HNEXT(ei);
            } 
            
            while( regular_edges.used || discontinuous_edges.used ) {
                if( regular_edges.used ) {
                    ei = int_stack_pop( &regular_edges );
                }
                else {
                    ei = int_stack_pop( &discontinuous_edges );
                }
                ei = ea->edges[ei].twin;
                v = ea->edges[HPREV(ei)].start;

                if( v == DUMMY_VERTEX )
                    continue;
                ASSERT( v != GARBAGE );
                if( VERTEX_SPECIAL(v) ) {
                    
                    eav = EA_VERTEX_INDEX(v);
                    
                    proxyv = PROXY_VERTEX_INDEX(proxy_hash,v);
                }
                else
                    eav = proxyv = v;
                ASSERT( eav >= 0 && eav < pcount );
                if( proxyv == DUMMY_VERTEX )
                    continue;
                ASSERT( proxyv >= 0 && proxyv < pcount );

                
                if( !touched[proxyv] ) {
                    if( quantized_points_out ) {
                        predict( ea, ei, proxyv, touched, proxy_hash, quantized_points_out, p );
                        quantized_points_out[proxyv*3+0] = p[0] - diffs[next_diff*3 + 0];
                        quantized_points_out[proxyv*3+1] = p[1] - diffs[next_diff*3 + 1];
                        quantized_points_out[proxyv*3+2] = p[2] - diffs[next_diff*3 + 2];
                    }
                    if( quantized_normals_out ) {
                        predict( ea, ei, proxyv, touched, proxy_hash, quantized_normals_out, p );
                        quantized_normals_out[proxyv*3+0] = p[0] - ndiffs[next_diff*3 + 0];
                        quantized_normals_out[proxyv*3+1] = p[1] - ndiffs[next_diff*3 + 1];
                        quantized_normals_out[proxyv*3+2] = p[2] - ndiffs[next_diff*3 + 2];
                    }
                    next_diff++;
                    ei = HNEXT(ei);
                    if( EDGE_INVALID( ea->edges[ei].twin ) ) {
                        
                        
                    }
                    else {
                        int_stack_push( &regular_edges, ei );
                    }
                    ei = HNEXT(ei);
                    if( EDGE_INVALID( ea->edges[ei].twin ) ) {
                        
                        
                    }
                    else {
                        int_stack_push( &regular_edges, ei );
                    }
                    touched[proxyv] = 1;
                }
            }
        }
        ASSERT( next_diff == diffs_count );
        ASSERT( all_points_touched( pcount, touched ) );
    }
    else if( by_tristrips ) {
        
        if( decode_all_tristrips( 
                    ea, 0, tristrips, 
                    num_tristrips, tristrip_starts, tristrip_lengths, 
                    proxy_hash ) <= 0)
            by_tristrips = false;
    }

    
    if( vs.status == STATUS_ERROR )
        status = STATUS_ERROR;

    if( diffs )
        EA_FREE( diffs );
    if( coords )
        EA_FREE( coords );
    if( ndiffs )
	EA_FREE( ndiffs );
    if( normals )
	EA_FREE( normals );
    int_stack_free( &regular_edges );
    int_stack_free( &discontinuous_edges );
    if( touched )
        EA_FREE( touched );
    if( tristrip_starts )
        EA_FREE( tristrip_starts );
    if( tristrip_lengths )
        EA_FREE( tristrip_lengths );
    if( by_tristrips ) {
        *tristrips_len_out = tristrip_total_length;
        *tristrips_out = tristrips;
    }
    else {
        if( tristrips != NULL )
            EA_FREE( tristrips );
    }
    return status;
} 


#define unquantize_normals(a,b,c,d,e,f,g)  unquantize_points(a,b,c,d,e,f,g)


static int unquantize_points( 
        int pcount,
        int const *quantized_points, 
        ET_Bounding const *bounding, 
        int x_quantization,
        int y_quantization,
        int z_quantization,
        float alter *points_out )
{
    int xmaxval, ymaxval, zmaxval;
    float fx_range_inverse, fy_range_inverse, fz_range_inverse;
    int i;

    xmaxval = MAXVAL(x_quantization);
    ymaxval = MAXVAL(y_quantization);
    zmaxval = MAXVAL(z_quantization);

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

    for( i = 0 ; i < pcount*3 ; i += 3 )    {
        
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
    return STATUS_NORMAL;
}



int show_edgebreaker_decompress_size( int stream_len, void const *stream, 
                            int alter *pcount_out, int alter *normal_count_out, int alter *flen_out )
{
    edgebreaker_header const *hptr = (edgebreaker_header *) stream;
    
    UNREFERENCED(stream_len);

    ASSERT( hptr->scheme <= 2 );
    ASSERT( hptr->mtable_scheme == 0 );
    ASSERT( hptr->points_scheme <= 3 );
    if( pcount_out != NULL )
        *pcount_out = swap32(hptr->pcount);
    if( flen_out != NULL )
        *flen_out = swap32(hptr->opslen) * 4;
    if( normal_count_out != NULL ) {
        if( hptr->scheme >= 1 &&
            hptr->normalslen != 0 ) {
            ASSERT( hptr->normals_scheme <= 3 );
            *normal_count_out = swap32(hptr->pcount);
        }
        else
            *normal_count_out = 0;
    }
    return 1;

} 


int edgebreaker_decompress( int stream_len, void const *stream, 
            int alter *pcount_in_out, float alter *points_out, float alter *normals_out,
            bool alter *by_tristrips_out, int alter *flen_in_out, int alter *faces_out, 
            eb_decompress_configs const *configs )
{
    int offset = 0, pcount = 0, flen = 0, status = 1;
    int opcode_pointcount; 
    void const *pdata, *mdata;
    char const *ops;
    mtable_info mtable;
    edgebreaker_header const *hptr_in = (edgebreaker_header const *) stream;
    edgebreaker_header hptr;
    int *associations = NULL;
    half_edge_array *ea = NULL;
    ET_Bounding const *bounding = NULL;
    vhash_t *proxy_hash = NULL;
    int *quantized_points = NULL;
    int *quantized_normals = NULL;
    bool by_tristrips = false;
    int tristrips_len = 0;
    int *tristrips = NULL;

    UNREFERENCED(stream_len);

    init_actions_table( configs );
    mtable_info_init( &mtable );

    
    hptr.scheme = hptr_in->scheme;
    hptr.mtable_scheme = hptr_in->mtable_scheme;
    hptr.points_scheme = hptr_in->points_scheme;
    hptr.normals_scheme = hptr_in->normals_scheme;
    hptr.opslen = swap32( hptr_in->opslen );
    hptr.mtablelen = swap32( hptr_in->mtablelen );
    hptr.pointslen = swap32( hptr_in->pointslen );
    hptr.pcount = swap32( hptr_in->pcount );
    if( hptr_in->scheme >= 1 )
        hptr.normalslen = swap32( hptr_in->normalslen );
    else
        hptr.normalslen = 0; 

    if( hptr.opslen == 0 )
        goto done;

    ASSERT( hptr_in->scheme <= 2 ); 
    
    ASSERT( (hptr.pointslen == 0) || (*pcount_in_out >= hptr.pcount) );
    ASSERT( *flen_in_out >= hptr.opslen * 4 );
    pcount = hptr.pcount;

    switch( hptr.scheme ) {
        case 0:
            offset = (int)sizeof(edgebreaker_header0);
        break;
        case 1:
        case 2:
            offset = (int)sizeof(edgebreaker_header1);
        break;
        default:
            COMPLAIN( "internal error: unrecognized edgebreaker data format" );
    }
    ops = (char *)stream + offset;
    
    offset += hptr.opslen;
    offset += TONEXT4(offset);          

    
    mdata = ((char *)stream + offset);
    unpack_mtable ( mdata, &mtable, &proxy_hash );
    if( mtable.bounding != NULL ) {
        bounding = mtable.bounding;
    }
    else {
        if( configs == NULL ||
            configs->bounding == NULL ) {
            if ((hptr.scheme >= 2 && hptr.points_scheme > 0 ) ||
                (hptr.scheme < 2 && hptr.pointslen > 0 )) {
                COMPLAIN( "if a bounding was passed to compress, it has to be passed to decompress, too!" );
                return 0;
            }
            bounding = NULL;
        }
        else {
            bounding = configs->bounding;
        }
    }           
    offset += hptr.mtablelen;
    offset += TONEXT4(offset);          

    if( hptr.scheme >= 2 ) {
        ea = (half_edge_array *) EA_MALLOC ( sizeof( half_edge_array ) );
        if((status = process_opcodes( 
                hptr.opslen, 
                ops, 
                &mtable, 
                ea, 
                faces_out, 
                &opcode_pointcount )) <= 0 )
            goto done;
    }
    else {
        associations = (int *) EA_MALLOC ( (hptr.opslen*3) * 3 * sizeof( int ) );
        if( associations == NULL ) {
            COMPLAIN( "associations array allocation failed during decompress" );
        }
        if((status = old_process_opcodes( 
                hptr.opslen, 
                ops, 
                &mtable, 
                associations, 
                faces_out, 
                &opcode_pointcount )) <= 0 )
            goto done;
    }

    flen = *flen_in_out;  
    if((status = patch_faces( opcode_pointcount, &mtable, proxy_hash, &flen, faces_out, associations, ea )) <= 0 )
        goto done;

    if( hptr.scheme >= 2 &&
        hptr.pointslen > 0 ) {
        pdata = ((char *)stream + offset);
        if( hptr.points_scheme > 0 ) {
            quantized_points = (int *) EA_MALLOC( 3 * pcount * sizeof( int ) );
        }
        if( hptr.normals_scheme > 0 ) {
            quantized_normals = (int *) EA_MALLOC( 3 * pcount * sizeof( int ) );
        }
        
        status = unpack_points_and_normals( 
                &hptr, 
                ea, pcount, 
                proxy_hash, 
                hptr.pointslen, 
                (void *)pdata, 
                quantized_points,
                quantized_normals,
                &tristrips_len,
                &tristrips );
        if( status <= 0 )
            goto done;
        if( tristrips != NULL ) {
            by_tristrips = true;
        }
        if( quantized_points ) {
            status = unquantize_points( 
                pcount, quantized_points, bounding, 
                mtable.x_quantization, mtable.y_quantization, mtable.z_quantization,
                points_out );
            if( status <= 0 )
                goto done;
        }
        if( quantized_normals ) {
            ET_Bounding normal_bbox = { -1,-1,-1, 1,1,1 };
            status = unquantize_normals( 
                pcount, quantized_normals, &normal_bbox, 
                mtable.x_quantization_normals, mtable.y_quantization_normals, mtable.z_quantization_normals,
                normals_out );
            if( status <= 0 )
                goto done;
        }
        offset += hptr.pointslen + TONEXT4(hptr.pointslen);
        offset += hptr.normalslen + TONEXT4(hptr.normalslen);
    }
    else {
        
        if( hptr.pointslen > 0 ) {
            pdata = ((char *)stream + offset);
            ASSERT( hptr.points_scheme <= 1 );
            status = old_unpack_points( 
                    associations, &hptr, pdata, points_out, bounding, 
                    mtable.x_quantization, mtable.y_quantization, mtable.z_quantization );
            if( status <= 0 )
                goto done;
            offset += hptr.pointslen + TONEXT4(hptr.pointslen);
        }
        if( hptr.scheme >= 1 &&
            hptr.normalslen > 0 ) {
            pdata = ((char *)stream + offset);
            ASSERT( hptr.normals_scheme == 0 );
            status = old_unpack_normals( 
                    associations, &hptr, pdata, normals_out,  
                    mtable.x_quantization_normals, mtable.y_quantization_normals, mtable.z_quantization_normals );
            if( status <= 0 )
                goto done;
            offset += hptr.normalslen + TONEXT4(hptr.normalslen);
        }
    }


  done:
    *by_tristrips_out = by_tristrips;
    if( by_tristrips ) {
        *flen_in_out = tristrips_len;
        memcpy( faces_out, tristrips, tristrips_len * sizeof( int ) );
    }
    else {
        *flen_in_out = flen;
    }
    if( tristrips != NULL )
        EA_FREE( tristrips );
    *pcount_in_out = pcount;
    if( associations != NULL )
        EA_FREE( associations );
    if( ea != NULL ) {
        half_edge_array_free( ea );
        EA_FREE( ea );
    }
    if( proxy_hash != NULL )
        delete_vhash( proxy_hash );
    if( quantized_points != NULL )
        EA_FREE( quantized_points );
    if( quantized_normals != NULL )
        EA_FREE( quantized_normals );
    mtable_info_free( &mtable );
    EA_FREE( actions );

    return status;

} 


#endif 


