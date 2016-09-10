/*
 * Copyright (c) 2001 by Tech Soft America, LLC.
 * The information contained herein is confidential and proprietary to
 * Tech Soft America, LLC., and considered a trade secret as defined under
 * civil and criminal statutes.  Tech Soft America shall pursue its civil
 * and criminal remedies in the event of unauthorized use or misappropriation
 * of its trade secrets.  Use of this information by anyone other than
 * authorized employees of Tech Soft America, LLC. is granted only under a
 * written non-disclosure agreement, expressly prescribing the scope and
 * manner of such use.
 *
 * $Id: //DWF/Working_Area/Willie.Zhu/w3dtk/hoops/fastlod.c#1 $
 */


#include <math.h>
/*
#include "hoops.h"
*/
#include "lod.h"


typedef struct {
    int *data;
    int used;
    int allocated;
} hash_entry;

#define MxFEQ(a,b,eps) ((fabs((a)-(b))<(eps)))


local int add_data(hash_entry *he, int i, int j, int m)
{
    int c;
    for (c=he->used-1; c>=0; c--)
    if (he->data[c] == ((i<<(32-m))^(j<<(32-2*m)))) return 0;
    if (he->used >= he->allocated)
    {
    int *temp;
    he->allocated *= 2;
    temp = (int *) MX_ALLOC ((long)(he->allocated * sizeof( int )));
    MX_COPY( he->data, he->used * sizeof( int ), temp );
    MX_FREE( he->data, (long)(he->used * sizeof( int )) );
    he->data = temp;
    }
    he->data[he->used] = (i<<(32-m))^(j<<(32-2*m));
    he->used++;
    return 1;
}


local void get_bounding( int pcount, const float *points, float *min, float *max )
{
    int i;

    /* start by calculating a min/max over the points */
    max[0] = -10000;
    max[1] = -10000;
    max[2] = -10000;
    min[0] = 10000;
    min[1] = 10000;
    min[2] = 10000;
    for (i=0; i<pcount; i++)
    {
    if (points[3*i] > max[0]) 
        max[0] = points[3*i];
    if (points[3*i+1] > max[1]) 
        max[1] = points[3*i+1];
    if (points[3*i+2] > max[2]) 
        max[2] = points[3*i+2];
    if (points[3*i] < min[0]) 
        min[0] = points[3*i];
    if (points[3*i+1] < min[1]) 
        min[1] = points[3*i+1];
    if (points[3*i+2] < min[2]) 
        min[2] = points[3*i+2];
    }
}

local int calc_subdivision(int pcount, const float *points, const float *min, const float *max, float target_percentage)
{
    hash_entry *hash, *hash_low;
    int d, i, j, k, subdiv, subdiv_min, subdiv_result, target_drops, sum, sum_low, n, m, n_low, m_low;
    int hash_table_size, hash_table_mask, hashkey1;
    int hash_table_size_low, hash_table_mask_low, hashkey2;
    float step[3], wanted, measured;

    target_drops = (int)(pcount * target_percentage);
    m = (int)(1.75 * log(target_drops) - 6);
    subdiv = (int)pow(2, m);
    m_low = m-1;
    n = m;
    if( n < 10 )
    n = 10;
    n_low = n-1;
    hash_table_size = (int)pow(2, n);
    hash_table_mask = hash_table_size - 1;
    hash_table_size_low = hash_table_size / 2;
    hash_table_mask_low = hash_table_size_low - 1;
    hash = (hash_entry *)MX_ALLOC ((long)(hash_table_size * sizeof(hash_entry)));
    hash_low = (hash_entry *)MX_ALLOC ((long)(hash_table_size_low * sizeof(hash_entry)));
    for (i=0; i<hash_table_size; i++)
    {
    hash[i].allocated = 2;
    hash[i].data = MX_ALLOC ((long)(2 * sizeof(int)));
    hash[i].used = 0;
    }
    for (i=0; i<hash_table_size_low; i++)
    {
    hash_low[i].allocated = 2;
    hash_low[i].data = MX_ALLOC ((long)(2 * sizeof(int)));
    hash_low[i].used = 0;
    }

    if( max[0] - min[0] != 0 )
    step[0] = (float)(subdiv-1) / (max[0] - min[0]);
    else
    step[0] = 0;
    if( max[1] - min[1] != 0 )
    step[1] = (float)(subdiv-1) / (max[1] - min[1]);
    else
    step[1] = 0;
    if( max[2] - min[2] != 0 )
    step[2] = (float)(subdiv-1) / (max[2] - min[2]);
    else
    step[2] = 0;
    sum = 0;
    sum_low = 0;
    for (d=0; d<pcount; d++)
    {
    i = (int)((points[d*3 + 0] - min[0]) * step[0]);
    j = (int)((points[d*3 + 1] - min[1]) * step[1]);
    k = (int)((points[d*3 + 2] - min[2]) * step[2]);
    hashkey1 = ((i<<(n-m))^(j<<(n-m))^k) & hash_table_mask;
    sum += add_data( hash + hashkey1, i, j, m );
    hashkey2 = (((i>>1)<<(n_low-m_low))^((j>>1)<<(n_low-m_low))^(k>>1)) & hash_table_mask_low;
    sum_low += add_data( hash_low + hashkey2, (i>>1), (j>>1), m_low );
    }
    
    for (i=0; i<hash_table_size; i++) {
    MX_FREE( hash[i].data, (long)(hash[i].allocated * sizeof( int )) );
    hash[i].data = null;
    }
    MX_FREE( hash, (long)(hash_table_size * sizeof(hash_entry)) );
    hash = null;
    for (i=0; i<hash_table_size_low; i++) {
    MX_FREE (hash_low[i].data, (long)(hash_low[i].allocated * sizeof( int )) );
    hash_low[i].data = null;
    }
    MX_FREE( hash_low, (long)(hash_table_size_low * sizeof(hash_entry)) );
    hash_low = null;

    wanted = (float)log((float)target_drops / sum_low);
    measured = (float)log((float)sum / sum_low);
    subdiv_result = (int)((subdiv / 2) * pow(2, wanted / measured )) + 1;
    subdiv_min = (int)pow(target_drops, 1.0 / 3);
    if (subdiv_result < subdiv_min) 
    subdiv_result = subdiv_min;
    return subdiv_result;

} /* end function calc_subdivision */


local void quadric_tensor( const MxQuadric3 *q, Mat3 *m )
{
    m->row[0].elt[0] = q->a2;
    m->row[0].elt[1] = q->ab;
    m->row[0].elt[2] = q->ac;
    m->row[1].elt[0] = q->ab;
    m->row[1].elt[1] = q->b2;
    m->row[1].elt[2] = q->bc;
    m->row[2].elt[0] = q->ac;
    m->row[2].elt[1] = q->bc;
    m->row[2].elt[2] = q->c2;
}


local void cross(double *r, const double *u, const double *v, int dim)
{
    if( dim == 3 )
    {
    r[0] =  (u[1] * v[2]) - (v[1] * u[2]); 
    r[1] = -(u[0] * v[2]) + (v[0] * u[2]);
    r[2] =  (u[0] * v[1]) - (v[0] * u[1]);  
    }
}

local double dot(const double *u, const double *v, int dim)
{
    int i;
    double val=0.0;  

    for(i = 0 ; i < dim ; i++) 
    val += u[i]*v[i];  
    return val;
}

local void divv( double *r, const double *u, const double d, int dim )
{
    int i;
    for(i = 0 ; i < dim ; i++) 
    r[i] = u[i] / d;
}

local void col3x3( Vec3 *v_out, const Mat3 *m1, int i )
{ 
    v_out->elt[0] = m1->row[0].elt[i];
    v_out->elt[1] = m1->row[1].elt[i];
    v_out->elt[2] = m1->row[2].elt[i]; 
}

local double invert3x3( Mat3 *m_out, const Mat3 *m1 )
{
    Mat3 temp, A;
    double d;

    /* compute the adjoint: adjoint( &A, m1 ); */
    cross( A.row[0].elt, m1->row[1].elt, m1->row[2].elt, 3 );
    cross( A.row[1].elt, m1->row[2].elt, m1->row[0].elt, 3 );
    cross( A.row[2].elt, m1->row[0].elt, m1->row[1].elt, 3 );

    d = dot( A.row[0].elt, m1->row[0].elt, 3 );

    if( d==0.0 )
        return 0.0;
    /* compute the transpose: transpose( &temp, &A ); */
    col3x3( &(temp.row[0]), &A, 0 );
    col3x3( &(temp.row[1]), &A, 1 );
    col3x3( &(temp.row[2]), &A, 2 );

    divv( m_out->row[0].elt, temp.row[0].elt, d, 3 );
    divv( m_out->row[1].elt, temp.row[1].elt, d, 3 );
    divv( m_out->row[2].elt, temp.row[2].elt, d, 3 );
    return d;
}

local void quadric_vector( Vec3 *v_out, const MxQuadric3 *q ) { 
    v_out->elt[0] = q->ad;
    v_out->elt[1] = q->bd;
    v_out->elt[2] = q->cd;
}

local void vecmul3x3( Vec3 *v_out, const Mat3 *m1, const Vec3 *v1)
{
    v_out->elt[0] = dot( m1->row[0].elt, v1->elt, 3 ); 
    v_out->elt[1] = dot( m1->row[1].elt, v1->elt, 3 ); 
    v_out->elt[2] = dot( m1->row[2].elt, v1->elt, 3 );
}

local void neg ( double *r, const double *u, int dim) 
{ 
    int i;
    for(i = 0 ; i < dim ; i++) 
        r[i] = -u[i];
}

local int optimize( const MxQuadric3 *q, Vec3 *v)
{
    Mat3 Ainv, tensor;
    double det;
    Vec3 vtemp;

    quadric_tensor( q, &tensor );
    det = invert3x3( &Ainv, &tensor );
    if( MxFEQ(det, 0.0, 1e-12f) )
    return 0;

    quadric_vector( &vtemp, q );
    vecmul3x3( v, &Ainv, &vtemp );
    neg( v->elt, v->elt, 3 );

    return 1;
}

local void quadric_init( MxQuadric3 *q, double a, double b, double c, double d)
{
    q->a2 = a*a;  q->ab = a*b;  q->ac = a*c;  q->ad = a*d;
    q->b2 = b*b;  q->bc = b*c;  q->bd = b*d;
    q->c2 = c*c;  q->cd = c*d;
    q->d2 = d*d;
    q->r = 1;
}

local void quadric_add( MxQuadric3 *q_out, const MxQuadric3 *q1, const MxQuadric3 *q2 )
{
    /* Accumulate area*/
    q_out->r  = q1->r + q2->r;

    /* Accumulate coefficients*/
    q_out->a2 = q1->a2 + q2->a2;
    q_out->ab = q1->ab + q2->ab;
    q_out->ac = q1->ac + q2->ac;  
    q_out->ad = q1->ad + q2->ad;

    q_out->b2 = q1->b2 + q2->b2;  
    q_out->bc = q1->bc + q2->bc;  
    q_out->bd = q1->bd + q2->bd;

    q_out->c2 = q1->c2 + q2->c2;  
    q_out->cd = q1->cd + q2->cd;

    q_out->d2 = q1->d2 + q2->d2;
}

local int quadric_optimize( const MxQuadric3 *q, float *out)
{
    Vec3 v;
    if( optimize( q, &v ) )
    {
    out[0] = (float) v.elt[0];
    out[1] = (float) v.elt[1];
    out[2] = (float) v.elt[2];
    return 1;
    }
    return 0;
}


/* the ijknode stores all of the information associated with a representative vertex
 * for everything that resolves to bucket i,j,k */
typedef struct {
    int i, j, k, id;
    /* sum of constraints from which we will attempt to compute an optimal vertex location */
    MxQuadric3 quadric; 
    /* fallback xyz in case the quadric can't be optimized */
    float x, y, z;  
} vertex;

typedef struct {
    vertex *data;
    int allocated;
    int used;
} ijknode;

typedef struct {
    ijknode *ijkhash;
    int ijkhash_size;
    int ijkhash_mask;

    const float *min, *max;
    int subdivision;
    float step[3];

    int *faces;
    int faces_allocated;
    int faces_used;

    int pcount;
    float *points;

} model;


local void model_init( model *m, const float *min, const float *max, 
    int subdiv, int hash_size )
{
    int ii, temp;
    ijknode *node;

    m->faces_used = 0;
    m->faces_allocated = 100;
    m->faces = (int *) MX_ALLOC ( (long)(m->faces_allocated * sizeof( int )));
    m->pcount = 0;
    m->points = null;
    m->min = min;
    m->max = max;
    m->subdivision = subdiv;

    /* m->ijkhash_size must be a power of 2 so that we can use a mask */
    m->ijkhash_size = 1;
    while( m->ijkhash_size < hash_size )
    m->ijkhash_size = m->ijkhash_size << 1;
    m->ijkhash = MX_ALLOC ((long)(m->ijkhash_size * sizeof(ijknode)));
    for ( ii = 0 ; ii < m->ijkhash_size ; ii++ )
    {
    node = &m->ijkhash[ii];
    node->allocated = 2;
    node->data = (vertex *) MX_ALLOC ((long)(node->allocated * sizeof(vertex)));
    node->used = 0;
    }
    /* construct the ijkhash_mask.  Assumes that ijkhash_size is a power of 2 */
    temp = m->ijkhash_size;
    m->ijkhash_mask = 0;
    while( temp ) {
    temp = temp >> 1;
    m->ijkhash_mask |= temp;
    }

    if( max[0] - min[0] != 0 )
    m->step[0] = (float)(subdiv-1) / (max[0] - min[0]);
    else
    m->step[0] = 0;
    if( max[1] - min[1] != 0 )
    m->step[1] = (float)(subdiv-1) / (max[1] - min[1]);
    else
    m->step[1] = 0;
    if( max[2] - min[2] != 0 )
    m->step[2] = (float)(subdiv-1) / (max[2] - min[2]);
    else
    m->step[2] = 0;
}

local void model_free( model *m )
{
    int i;

    MX_FREE( m->points, (long)(3 * m->pcount * sizeof( float )) );
    m->points = null;
    MX_FREE( m->faces, (long)(m->faces_allocated * sizeof( int )) );
    m->faces = null;
    for( i = 0 ; i < m->ijkhash_size ; i++ ) {
    MX_FREE( m->ijkhash[i].data, (long)(m->ijkhash[i].allocated * sizeof( vertex )) );
    m->ijkhash[i].data = null;
    }
    MX_FREE( m->ijkhash, (long)(m->ijkhash_size * sizeof( ijknode )) );
    m->ijkhash = null;

} /* end function model_free */


#define IJKHASHFUNC(i,j,k,mask) (((i << 8) ^ (j << 4) ^ k) & mask)


local int ijk_hash_get( model *m, float x, float y, float z, vertex **out )
{
    int i, j, k, ii, hashkey;
    ijknode *node;
    vertex *v;

    i = (int)((x - m->min[0]) * m->step[0]);
    j = (int)((y - m->min[1]) * m->step[1]);
    k = (int)((z - m->min[2]) * m->step[2]);
    hashkey = IJKHASHFUNC(i,j,k,m->ijkhash_mask);
    node = &m->ijkhash[ hashkey ];
    v = node->data;
    for( ii = 0 ; ii < node->used ; ii++ ) {
    if( (i == v->i) &&
        (j == v->j) &&
        (k == v->k) ) {
        *out = v;
        return 1;
    }
    v++;
    }
    *out = null;
    return 0;
}

local void ijk_hash_insert( model *m, float x, float y, float z, int id )
{
    int i, j, k, hashkey;
    ijknode *node;
    vertex *v, *temp;

    i = (int)((x - m->min[0]) * m->step[0]);
    j = (int)((y - m->min[1]) * m->step[1]);
    k = (int)((z - m->min[2]) * m->step[2]);
    hashkey = IJKHASHFUNC(i,j,k,m->ijkhash_mask);
    node = &m->ijkhash[ hashkey ];
    if( node->used == node->allocated ) {
    node->allocated *= 2;
    temp = (vertex *) MX_ALLOC ( (long)(node->allocated * sizeof( vertex )));
    MX_COPY( node->data, node->used * sizeof( vertex ), temp );
    MX_FREE( node->data, (long)(node->used * sizeof( vertex )) );
    node->data = temp;
    }
    v = &node->data[node->used++];
    v->i = i;
    v->j = j;
    v->k = k;
    v->id = id;
    v->x = x;
    v->y = y;
    v->z = z;
    quadric_init (&v->quadric, 0.0, 0.0, 0.0, 0.0);
    m->pcount++;

} /* end function ijk_hash_insert */


local void faces_append( model *m, int val )
{
    if( m->faces_used == m->faces_allocated ) {
    int *temp;

    m->faces_allocated *= 2;
    temp = (int *) MX_ALLOC ( (long)(m->faces_allocated * sizeof( int )));
    MX_COPY( m->faces, m->faces_used * sizeof( int ), temp );
    MX_FREE( m->faces, (long)(m->faces_used * sizeof( int )) );
    m->faces = temp;
    }
    m->faces[ m->faces_used++ ] = val;
} /* end function faces_append */


/* Given three floating point coords, get an abcd plane equation.
 * Start by converting to double so that the intermediate calculations have high precision */
local void get_plane( const float *v1f, const float *v2f, const float *v3f,     
        double *a, double *b, double *c, double *d )
{
    double v1[3], v2[3], v3[3];
    v1[0] = v1f[0];   v1[1] = v1f[1];   v1[2] = v1f[2];
    v2[0] = v2f[0];   v2[1] = v2f[1];   v2[2] = v2f[2];
    v3[0] = v3f[0];   v3[1] = v3f[1];   v3[2] = v3f[2];
    *a = v1[1] * (v2[2] - v3[2]) + v2[1] * (v3[2] - v1[2]) + v3[1] * (v1[2] - v2[2]);
    *b = v1[2] * (v2[0] - v3[0]) + v2[2] * (v3[0] - v1[0]) + v3[2] * (v1[0] - v2[0]);
    *c = v1[0] * (v2[1] - v3[1]) + v2[0] * (v3[1] - v1[1]) + v3[0] * (v1[1] - v2[1]);
    *d = - (*a * v1[0] + *b * v1[1] + *c * v1[2]);
} /* end function get_plane */


local void face_add( model *m, const int *face, const float *points )
{
    int ii, count;
    double a, b, c, d;
    const float *v, *v1, *v2, *v3;
    MxQuadric3 quadtemp;
    vertex **verts, *vertbuffer[7];

    if( face[0] < 3 )
    return;
    /* cutoff at one less than the space in vertbuffer to allow for a null at the start */
    if( face[0] > 6 )  
    verts = (vertex **) MX_ALLOC ( (long)((2+face[0]) * sizeof(vertex *)));
    else
    verts = vertbuffer;

    /* calc the quadric that summarizes the constraint added by this particular face */
    v1 = &points[3*face[1]];
    v2 = &points[3*face[2]];
    v3 = &points[3*face[3]];
    get_plane( v1, v2, v3, &a, &b, &c, &d );    
    quadric_init (&quadtemp, a, b, c, d);

    verts[0] = null;
    count = 0;
    for( ii = 1 ; ii <= face[0] ; ii++ ) {
    v = &points[3*face[ii]];
    /* get vertex struct */
    ijk_hash_get( m, v[0], v[1], v[2], &verts[ii] );
    if( verts[ii] != verts[ii-1] )
        count++;
    /* add quadric (dest,src1,src2) */
    quadric_add (&verts[ii]->quadric, &verts[ii]->quadric, &quadtemp);
    }
    /* If the last vertex == the first one, we have one less unique value. */
    if( verts[ face[0] ] == verts[1] )
    count--;
    verts[0] = verts[ face[0] ]; /* copy the last to the start */
    /* Append a face to the output face list if the number of unique vertices touched was at least 3.
     * Note: the calculation comes up with 0 for the case where count should be exactly 1, 
     * but that shouldn't matter. */
    if( count >= 3 ) {
    faces_append( m, count );
    for( ii = 1 ; ii <= face[0] ; ii++ ) {
        if( verts[ii] != verts[ii-1] )
        faces_append( m, verts[ii]->id );
    }
    }
    /* if we allocated something (which should probably be pretty rare), we have to free it */
    if( verts != vertbuffer )
    MX_FREE( verts, (long)(face[0] * sizeof( vertex )) );

} /* end function face_add */


local void compute_optimal_vertices( model *m )
{
    int ii, jj, i, j, k;
    ijknode *node;
    float *newpoint;
    vertex *vert;
    int success;
    float *cell_borders[3], cell_size;
    
    /* x */
    cell_borders[0] = (float *) MX_ALLOC ((long)((m->subdivision + 1) * sizeof( float )));
    if( m->step[0] != 0 )
    cell_size = 1.0f / m->step[0];
    else
    cell_size = 0;
    for( ii = 0 ; ii < m->subdivision+1 ; ii++ )
    cell_borders[0][ii] = cell_size * ii;
    /* y */
    cell_borders[1] = (float *) MX_ALLOC ((long)((m->subdivision + 1) * sizeof( float )));
    if( m->step[1] != 0 )
    cell_size = 1.0f / m->step[1];
    else
    cell_size = 0;
    for( ii = 0 ; ii < m->subdivision+1 ; ii++ )
    cell_borders[1][ii] = cell_size * ii;
    /* z */
    cell_borders[2] = (float *) MX_ALLOC ((long)((m->subdivision + 1) * sizeof( float )));
    if( m->step[2] != 0 )
    cell_size = 1.0f / m->step[2];
    else
    cell_size = 0;
    for( ii = 0 ; ii < m->subdivision+1 ; ii++ )
    cell_borders[2][ii] = cell_size * ii;

    m->points = (float *) MX_ALLOC ((long)(3 * m->pcount * sizeof( float )));
    for( ii = 0 ; ii < m->ijkhash_size ; ii++ ) {
    node = &m->ijkhash[ii];
    for( jj = 0 ; jj < node->used ; jj++ ) {
        vert = &node->data[jj];
        i = (int)((vert->x - m->min[0]) * m->step[0]);
        j = (int)((vert->y - m->min[1]) * m->step[1]);
        k = (int)((vert->z - m->min[2]) * m->step[2]);
        newpoint = m->points + (3 * (vert->id));
        success = quadric_optimize (&vert->quadric, newpoint);
        if( !success ||
        newpoint[0] < -cell_borders[0][i] ||
        newpoint[1] < -cell_borders[1][j] ||
        newpoint[2] < -cell_borders[2][k] ||
        newpoint[0] > cell_borders[0][i+1] ||
        newpoint[1] > cell_borders[1][j+1] ||
        newpoint[2] > cell_borders[2][k+1] ) 
        {
        newpoint[0] = vert->x;
        newpoint[1] = vert->y;
        newpoint[2] = vert->z;
        }
    }
    }
    MX_FREE( cell_borders[0], (long)((m->subdivision + 1) * sizeof( float )) );
    MX_FREE( cell_borders[1], (long)((m->subdivision + 1) * sizeof( float )) );
    MX_FREE( cell_borders[2], (long)((m->subdivision + 1) * sizeof( float )) );

} /* end function compute_optimal_vertices */


local MxShellChain *merge_shell( int pcount, const float *points, int faces_length, const int *faces, 
        const float *min, const float *max, int subdiv_result ) 
{
    int ii, id, hash_size;
    float x, y, z;
    int const *fptr, *end;
    model *m;
    vertex *vert;
    MxShellChain *out;
    int *pointmap;

    m = (model *) MX_ALLOC (sizeof ( model ));  
    hash_size = pcount / 2;
    model_init( m, min, max, subdiv_result, hash_size );

    /* figure out exactly how many unique buckets we have, the mapping
     * between old and new points, and initialize the fallback xyz values */
    ALLOC_ARRAY( pointmap, pcount, int );
    id = 0;
    for ( ii = 0 ; ii < pcount ; ii++ )
    {
    x = points[ii*3 + 0];
    y = points[ii*3 + 1];
    z = points[ii*3 + 2];
    if( !ijk_hash_get( m, x, y, z, &vert ) ) {
        ijk_hash_insert( m, x, y, z, id );
        pointmap[ii] = id;
        id++;
    }
    else {
        pointmap[ii] = vert->id;
    }
    }

    fptr = faces;
    end = fptr + faces_length;
    while(fptr < end)
    {
    if( fptr[0] > 0 ) {
        /* a regular face -- add it to the model */
        face_add( m, fptr, points );
        fptr += fptr[0] + 1;
    }
    else {
        /* a hole in the previous face -- ignore it */
        fptr += -fptr[0] + 1;
    }
    }

    compute_optimal_vertices( m );

    out = (MxShellChain *) MX_ALLOC (sizeof (MxShellChain));
    out->sh.pcount = m->pcount;
    if( m->pcount > 0 ) {
    ALLOC_ARRAY( out->sh.points, 3 * m->pcount, float );
    MX_COPY( m->points, 3 * m->pcount * sizeof( float ), out->sh.points );
    }
    else {
    out->sh.points = null;
    }
    out->sh.flen = m->faces_used;
    if( m->faces_used > 0 ) {
    ALLOC_ARRAY( out->sh.faces, m->faces_used, int );
    MX_COPY( m->faces, m->faces_used * sizeof( int ), out->sh.faces );
    }
    else {
    out->sh.faces = null;
    }
    model_free( m );
    out->pointmap_count = pcount;
    out->pointmap = pointmap;
    MX_FREE( m, sizeof( model ) );
    return out;

} /* end function merge_shell */



MxShellChain *HU_Compute_LOD_Fast(
        int pcount,
        const float *points,
        int flen, 
        const int *faces,
        float ratio,
        int depth )
{
    MxShellChain *root, *node;
    int i, subdivision;
    float min[3], max[3];

    get_bounding( pcount, points, min, max );
    subdivision = calc_subdivision( pcount, points, min, max, ratio );
    root = merge_shell( pcount, points, flen, faces, min, max, subdivision );

    node = root;
    for( i = 0 ; i < depth - 1 ; i++ ) {
    subdivision = calc_subdivision( 
        node->sh.pcount, node->sh.points, 
        min, max, ratio );
    node->next = merge_shell( 
        node->sh.pcount, node->sh.points, 
        node->sh.flen, node->sh.faces, 
        min, max, subdivision );
    if( node->next == null )
        break;
    node = node->next;
    }
    node->next = null;
    return root;

} /* HU_Compute_LOD_Fast */
