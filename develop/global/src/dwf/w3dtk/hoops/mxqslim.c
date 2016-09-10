/************************************************************************

  MxQslim.c

  Surface simplification using quadric error metrics

  Copyright (C) 1998 Michael Garland.  See "COPYING.txt" for details.
  
  $Id: //DWF/Working_Area/Willie.Zhu/w3dtk/hoops/mxqslim.c#1 $

 ************************************************************************/

/*
#include "hoops.h"
*/
#include "lod.h"

#ifndef DISABLE_LOD_GENERATION

/*
#include <stdio.h>
FILE *fp;
static int firsttime = 1;
static int debug_count = 0;
*/

void mxqslim_init( MxQSlim *q, MxModel *_m, MxConfig *cfg )
{
    int i;

    /* Initialize internal variables*/
    mxheap_init( &(q->heap) );
    block_init( &(q->quadrics), sizeof( MxQuadric3 * ) );
    block_init( &(q->edge_links), sizeof( MxEdgeList * ) );
    block_init( &(q->edge_array), sizeof( MxQSlimEdge * ) );

    /* these resize calls are not required, but should help performance */
    resizeb( &(q->edge_links), model_vertex_count( _m ) );
    resizeb( &(q->quadrics), model_vertex_count( _m ) );
    for( i = 0 ; i < model_vertex_count( _m ) ; i++ )
    {
        MxEdgeList *el   = (MxEdgeList *) MX_ALLOC ( sizeof ( MxEdgeList ) );
        MxQuadric3 *quad = (MxQuadric3 *) MX_ALLOC ( sizeof ( MxQuadric3 ) );

        addpb( &(q->quadrics), quad );
        block_init( el, sizeof( MxQSlimEdge * ) );
        addpb( &(q->edge_links), el );
    }
    q->m = _m;

    /* Externally visible variables*/
    q->placement_policy  = cfg->placement_policy;
    q->weighting_policy  = cfg->weighting_policy;
    q->boundary_weight   = cfg->boundary_weight;
    q->compactness_ratio = cfg->compactness_ratio;
    q->meshing_penalty   = cfg->meshing_penalty;
    q->will_join_only    = cfg->will_join_only;
    q->max_degree        = cfg->max_degree;

    q->object_transform = NULL;
    q->local_validity_threshold = 0.0;

    q->valid_faces = 0;
    q->valid_verts = 0;

    for( i = 0 ; i < model_face_count(q->m); i++) 
        if(face_is_valid( q->m, i))  
            q->valid_faces++;
    for( i = 0 ; i < model_vertex_count(q->m); i++) 
        if(vertex_is_valid( q->m, i))  
            q->valid_verts++;

    q->contraction_callback = NULL;

    collect_quadrics( q );
    if( q->boundary_weight > 0.0 )
        constrain_boundaries( q );
    if( q->object_transform )
        transform_quadrics( q, q->object_transform );

    collect_edges( q );

} /* end function mxqslim_init */


void mxqslim_cleanup( MxQSlim *q )
{
    int i;

    for( i = 0 ; i < lengthb( &(q->quadrics) ) ; i++ )
        MX_FREE( getpb( &(q->quadrics), i ), sizeof( MxQuadric3 ) );
    for( i = 0 ; i < lengthb( &(q->edge_links) ) ; i++ )
    {
        MxEdgeList *el = qs_get_edge_links( q, i ) ;
        block_cleanup( el );
        MX_FREE( el, sizeof( MxEdgeList ) );
    }
    for( i = 0 ; i < lengthb( &(q->edge_array) ) ; i++ )
    {
        MxQSlimEdge *e = getpb( &(q->edge_array), i ) ;
        MX_FREE( e, sizeof( MxQSlimEdge ) );
    }

    mxheap_cleanup( &(q->heap) );
    block_cleanup ( &(q->quadrics) );
    block_cleanup ( &(q->edge_links) );
    block_cleanup ( &(q->edge_array) );

}


void collect_quadrics( MxQSlim *qs )
{
    int i, j;
    MxFace *f;
    Vec3 v1, v2, v3;
    Vec4 p;
    MxQuadric3 quad;

    for( j = 0 ; j < lengthb( &(qs->quadrics) ) ; j++ )
        quad_clear( qs_get_quadrics( qs, j ) );
    for( i = 0 ; i < model_face_count(qs->m) ; i++)
    {
        f = model_face( qs->m, i);

        mxv_setv( v1.elt, model_vertex( qs->m, f->v[0] )->elt, 3);
        mxv_setv( v2.elt, model_vertex( qs->m, f->v[1] )->elt, 3);
        mxv_setv( v3.elt, model_vertex( qs->m, f->v[2] )->elt, 3);

        if (qs->weighting_policy==MX_WEIGHT_RAWNORMALS)
            triangle_raw_plane( &p, &v1, &v2, &v3 );
        else
            if( !triangle_plane( &p, &v1, &v2, &v3 ) )
            {
                /* the triangle was degenerate.  No contribution */
                /* MX_ASSERT(0); */
                continue;
            }

        mxquadric3_init( &quad, 
                        p.elt[X], p.elt[Y], p.elt[Z], p.elt[W], 
                        compute_face_area(qs->m, i));

        switch( qs->weighting_policy )
        {
        case MX_WEIGHT_ANGLE:
            for(j=0; j<3; j++)
            {
                MxQuadric3 *qptr;
                MxQuadric3 Q_j;
                quad_copy( &Q_j, &quad );
                quad_scale( &Q_j, compute_corner_angle( qs->m, i, j ) );
                qptr = qs_get_quadrics(qs, f->v[j]);
                quad_add( qptr, qptr, &Q_j );
            }
            break;
        case MX_WEIGHT_AREA:
            quad_scale( &quad, quad_area( &quad ) );
            /* no break: fallthrough*/
        default:
            {
                MxQuadric3 *qptr1, *qptr2, *qptr3;
                qptr1 = qs_get_quadrics(qs, f->v[0]);
                qptr2 = qs_get_quadrics(qs, f->v[1]);
                qptr3 = qs_get_quadrics(qs, f->v[2]); 
                quad_add( qptr1, qptr1, &quad );
                quad_add( qptr2, qptr2, &quad );
                quad_add( qptr3, qptr3, &quad );
            }
            break;
        }
    }
}

void transform_quadrics( MxQSlim *qs, const Mat4 *mat)
{
    int j;
    MxQuadric3 *quad;
    for( j = 0 ; j < lengthb( &(qs->quadrics) ) ; j++ )
    {
        quad = qs_get_quadrics(qs, j);
        quad_transform( quad, quad, mat );
    }
}


/* tjh: open edges should be mostly kept constant, not simplified.
 * This function applies a penalty to simplifying away from such 
 * an edge.
 */
void discontinuity_constraint( MxQSlim *q, MxVertexID i, MxVertexID j,
                       MxFaceList *faces)
{
    MxFaceID fid;
    double ff;
    Vec3 org, dest;
    Vec3 e;         /* edge between vertex i and j */
    Vec3 n;         /* face normal of model_faces[ faces[ fid ] ] */
    Vec3 n2;        /* unit vector perpendicular to e and n */
    MxQuadric3 quad;
    MxQuadric3 *qptr1, *qptr2;

    for( fid = 0 ; fid < lengthb( faces ) ; fid++ )
    {
        mxv_setv( org.elt, model_vertex( q->m, i )->elt, 3 ); 
        mxv_setv( dest.elt, model_vertex( q->m, j )->elt, 3 );
        mxv_sub( e.elt, dest.elt, org.elt, 3 );

        if( !compute_face_normal( q->m, fl_get_face( faces, fid ), n.elt, 1 ) )
            continue;

        mxv_cross( n2.elt, e.elt, n.elt, 3 );
        ff = mxv_len( n2.elt, 3 );
        if( MxFEQ( ff, 0.0, 1e-12f ) )
            continue;

        mxv_div( n2.elt, n2.elt, ff, 3 );

        mxquadric3_init( 
            &quad, 
            n2.elt[0], n2.elt[1], n2.elt[2], 
            -mxv_dot( n2.elt, org.elt, 3 ),
            1.0f );
        quad_scale( &quad, q->boundary_weight );

        if( q->weighting_policy == MX_WEIGHT_AREA )
        {
            quad_set_area( &quad, compute_face_area(q->m, fl_get_face( faces, fid ) ));
            quad_scale( &quad, quad_area( &quad ) );
        }
    
        qptr1 = qs_get_quadrics( q, i );
        qptr2 = qs_get_quadrics( q, j );
        quad_add( qptr1, qptr1, &quad );
        quad_add( qptr2, qptr2, &quad );
    }
} /* end function discontinuity_constraint */


void constrain_boundaries( MxQSlim *q )
{
    MxVertexList star;
    MxFaceList faces;
    MxVertexID vid;
    int i, j;

    block_init( &faces, sizeof( MxFaceID ) );
    block_init( &star, sizeof( MxVertexID ) );

    for( i = 0 ; i < model_vertex_count(q->m) ; i++ )
    {
        resetb( &star );
        collect_vertex_star(q->m, i, &star);

        for( j = 0 ; j < lengthb( &star ) ; j++ )
        {
            vid = vl_get_vertex( &star, j );
            if( i < vid )
            {
                resetb( &faces );
                collect_edge_neighbors(q->m, i, vid, &faces);
                if( lengthb( &faces ) == 1 )
                    discontinuity_constraint( q, i, vid, &faces);
            }
        }
    }

    block_cleanup( &faces );
    block_cleanup( &star );

}

#if 0
/* HELPER 
 * Determine if the edge (v1,v2) is non-manifold.
 * [This includes boundary edges.]
 */
static
MxBool IsG1(MxModel *m, MxVertexID v1, MxVertexID v2)
{
    int return_val;
    MxFaceList faces;
    block_init( &faces, sizeof( MxFaceID ) );

    collect_edge_neighbors(m, v1, v2, &faces);
    return_val = lengthb( &faces ) != 2;
    
    block_cleanup( &faces );
    return return_val;
}

/* HELPER
 * Determine if the vertex v is non-manifold.
 * [This includes boundary vertices.]
 */
static
MxBool IsG0(MxModel *m, MxVertexID v)
{
    MxVertexList star;
    int i, num_disks;

    block_init( &star, sizeof( MxVertexID ) );
    collect_vertex_star(m, v, &star);

    for( i = 0 ; i < lengthb( &star ) ; i++ )
        if( IsG1(m, v, vl_get_vertex( &star, i )) )
        {
            block_cleanup( &star );
            return 1;
        }

    num_disks = lengthb( &star ) - lengthb( neighbors(m, v) );
    MX_ASSERT(num_disks >= 0);

    /* Locally non-manifold vertex: its neighborhood is not isomorphic*/
    /* to a disk. */
    block_cleanup( &star );
    return (num_disks > 0);
}
#endif

/*
 * IMPORTANT NOTE:  These check_xxx functions assume that the local
 *                  neighborhoods have been marked so that each face
 *                  is marked with the number of involved vertices it has.
 */

double check_local_compactness( MxQSlim *q, int v1, int v2,
                        const double *vnew)
{
    MxFaceList *N1 = neighbors(q->m, v1);
    double c_min = 1.0;
    Vec3 f_after[3];
    double c;
    int i, j;
    MxFace *f;

    v2;

    for( i = 0 ; i < lengthb( N1 ) ; i++ )
        if( get_face_mark(q->m, fl_get_face( N1, i )) == 1 )
        {
            f = model_face( q->m, fl_get_face( N1, i ));
            for( j = 0 ; j < 3 ; j++ )
                if (f->v[j]==v1)
                    mxv_setv( f_after[j].elt, vnew, 3);
                else
                    mxv_setv( f_after[j].elt, model_vertex( q->m, f->v[j])->elt, 3);
    
                c = triangle_compactness( f_after, f_after + 1, f_after + 2 );
        
                if( c < c_min ) 
                    c_min = c;
        }

    return c_min;
}


/* Takes the triangle normal, computes the dot product with all of its
 * neighbor triangles, and returns the min.  This minimum will be negative
 * iff there is an inversion.  
 */
double check_local_inversion( MxQSlim *q, int v1, const double *vnew)
{
    int i, j;
    double delta, Nmin = 1.0;
    MxFaceList *N1 = neighbors(q->m, v1);
    MxFace *f;
    MxFaceID fid;
    Vec3 n_before, ntemp;
    Vec3 f_after[3];

    for( i = 0 ; i < lengthb( N1 ) ; i++ )
    {
        fid = fl_get_face( N1, i );
        if( get_face_mark( q->m, fid ) == 1 )
        {
            f = model_face( q->m, fid );
            if( !compute_face_normal( q->m, fid, n_before.elt, 1 ) )
                continue;

            for( j = 0 ; j < 3 ; j++ )
                if( f->v[j] == v1 )
                    mxv_setv( f_after[j].elt, vnew, 3 );
                else
                    mxv_setv( f_after[j].elt, model_vertex( q->m, f->v[j])->elt, 3 );
        
            triangle_normal( &ntemp, f_after, f_after + 1, f_after + 2 ); 
            delta = mxv_dot( n_before.elt, ntemp.elt, 3 );
            if( delta < Nmin ) 
                Nmin = delta;
        }
    }
    return Nmin;

} /* end function check_local_inversion */


int check_local_validity( MxQSlim *q, MxVertexID v1, MxVertexID v2, const double *vnew)
{
    MxFaceList *N1 = neighbors(q->m, v1);
    MxFaceID fid;
    MxFace *f;
    int nfailed = 0;
    int i;
    int k, x, y;
    double d_yx[3], d_vx[3], d_vnew[3], f_n[3], n[3];

    v2;

    for( i = 0 ; i < lengthb( N1 ) ; i++ )
    {
        fid = fl_get_face( N1, i );
        if( get_face_mark( q->m, fid ) == 1 )
        {
            f = model_face( q->m, fid );
            k = face_find_vertex( f, v1 );
            x = f->v[ ( k + 1 ) % 3 ];
            y = f->v[ ( k + 2 ) % 3 ];

            mxv_sub(d_yx, model_vertex( q->m, y)->elt,  model_vertex( q->m, x)->elt, 3);   /* d_yx = y-x*/
            mxv_sub(d_vx, model_vertex( q->m, v1)->elt, model_vertex( q->m, x)->elt, 3);   /* d_vx = v-x*/
            mxv_sub(d_vnew, vnew, model_vertex( q->m, x)->elt, 3);          /* d_vnew = vnew-x*/

            mxv_cross(f_n, d_yx, d_vx, 3);
            mxv_cross(n, f_n, d_yx, 3);     /* n = ((y-x)^(v-x))^(y-x)*/
            mxv_unitize(n, 3);

            if( mxv_dot( d_vnew, n, 3 )   < 
                        q->local_validity_threshold * mxv_dot( d_vx, n, 3 ) )
                nfailed++;
        }
    }

    return nfailed;
}


/* tjh: prevent black hole vertices -- if there are too many edges coming 
 * out of a particular vertex, set a penalty so high that it will 
 * effectively prevent it from ever being involved in another edge compression 
 */
static void enforce_degree_limit( MxQSlim *q, MxQSlimEdge *info, int limit )
{
    if(
    ( lengthb( neighbors( q->m, info->e.v1 )) > limit ) ||
    ( lengthb( neighbors( q->m, info->e.v2 )) > limit ) 
      )
    {
        set_heap_key( 
        &(info->h), 
        get_heap_key( &(info->h) ) - 1e12 
    );
    }

} /* end function enforce_degree_limit */


#ifndef MIN
#  define MIN(a,b) (((a)>(b))?(b):(a))
#endif

void apply_penalties( MxQSlim *q, MxQSlimEdge *info )
{
    /* a quick escape if this function would have no effect */
    if( ( q->compactness_ratio < 0 ) && ( q->meshing_penalty <= 0 ) )
    {
        return;
    }
    else 
    {
        int i;
        MxFaceList *N1, *N2;
        double base_error, bias;

        N1 = neighbors(q->m, info->e.v1);
        N2 = neighbors(q->m, info->e.v2);

        /* Set up the face marks as the check_xxx() functions expect. */
        for( i = 0 ; i < lengthb( N2 ) ; i++ ) 
            face_mark(q->m, fl_get_face( N2, i ), 0);
        for( i = 0 ; i < lengthb( N1 ) ; i++ ) 
            face_mark(q->m, fl_get_face( N1, i ), 1);
        for( i = 0 ; i < lengthb( N2 ) ; i++ ) 
        {
            unsigned char c = get_face_mark( q->m, fl_get_face( N2, i ) );
            face_mark( q->m, fl_get_face( N2, i ), c + 1 );
        }
    
        base_error = get_heap_key( &(info->h) );
        bias = 0.0;

/*
        nfailed = check_local_validity( q, info->e.v1, info->e.v2, info->vnew );
        nfailed += check_local_validity( q, info->e.v2, info->e.v1, info->vnew );
        if( nfailed )
            bias += nfailed * q->meshing_penalty;
*/

        if( q->compactness_ratio >= 0.0 )
        {
            double c1_min=check_local_compactness( q, info->e.v1, info->e.v2, info->vnew );
            double c2_min=check_local_compactness( q, info->e.v2, info->e.v1, info->vnew );
            double c_min = MIN( c1_min, c2_min );
    
            if( c_min <= q->compactness_ratio )
                bias += (q->compactness_ratio - c_min) * 100.0;
        }

        set_heap_key( &(info->h), base_error - bias );
    }

} /* end function apply_penalties */


/* bb_test 
 * returns true if vnew is within the bounding box around info's vertices
 * and their neighbors.
 */
#define BIG_FLOAT (1e6)
static MxBool bb_test( MxQSlim *q, MxQSlimEdge *info )
{
    int i;
    MxVertexID vid;
    MxVertexID v1 = info->e.v1;
    MxVertexID v2 = info->e.v2;
    MxEdgeList *el;
    MxQSlimEdge *qse;
    MxVertex *v;
    double min[3];
    double max[3];

    min[0] = min[1] = min[2] =  BIG_FLOAT;
    max[0] = max[1] = max[2] = -BIG_FLOAT;

    el = qs_get_edge_links( q, v1 );
    for( i = 0 ; i < lengthb( el ) ; i++ )
    {
        qse = el_get_edge( el, i );
        vid = opposite_vertex( &(qse->e), v1 );
        v = model_vertex( q->m, vid );
        if( v->elt[0] < min[0] ) min[0] = v->elt[0];
        if( v->elt[1] < min[1] ) min[1] = v->elt[1];
        if( v->elt[2] < min[2] ) min[2] = v->elt[2];
        if( v->elt[0] > max[0] ) max[0] = v->elt[0];
        if( v->elt[1] > max[1] ) max[1] = v->elt[1];
        if( v->elt[2] > max[2] ) max[2] = v->elt[2];
    }
    
    el = qs_get_edge_links( q, v2 );
    for( i = 0 ; i < lengthb( el ) ; i++ )
    {
        qse = el_get_edge( el, i );
        vid = opposite_vertex( &(qse->e), v2 );
        v = model_vertex( q->m, vid );
        if( v->elt[0] < min[0] ) min[0] = v->elt[0];
        if( v->elt[1] < min[1] ) min[1] = v->elt[1];
        if( v->elt[2] < min[2] ) min[2] = v->elt[2];
        if( v->elt[0] > max[0] ) max[0] = v->elt[0];
        if( v->elt[1] > max[1] ) max[1] = v->elt[1];
        if( v->elt[2] > max[2] ) max[2] = v->elt[2];
    }
/*
    MX_ASSERT( !isNaN( info->vnew[0] ) );
    MX_ASSERT( !isNaN( info->vnew[1] ) );
    MX_ASSERT( !isNaN( info->vnew[2] ) );
*/

    if( info->vnew[0] < min[0] ) return 0;
    if( info->vnew[1] < min[1] ) return 0;
    if( info->vnew[2] < min[2] ) return 0;
    if( info->vnew[0] > max[0] ) return 0;
    if( info->vnew[1] > max[1] ) return 0;
    if( info->vnew[2] > max[2] ) return 0;

    return 1;

} /* end function bb_test */


void compute_target_placement( MxQSlim *q, MxQSlimEdge *info )
{
    MxVertexID i=info->e.v1, j=info->e.v2;
    const MxQuadric3 *Qi=qs_get_quadrics(q, i), *Qj=qs_get_quadrics(q, j);
    double e_min = 0;
    MxQuadric3 quad;
    Vec3 *vi, *vj;
    Vec3 best;
    Vec3 temp, mid;
    double e_mid;
    MxBool bb_test_failed = 0;
    MxBool optimize_failed = 0;

    quad_add( &quad, Qi, Qj );

    if( (q->placement_policy == MX_PLACE_OPTIMAL) ||
        (q->placement_policy == MX_PLACE_OPTIMAL_BB) )
    {
        if( optimize3f( &quad, &info->vnew[X], &info->vnew[Y], &info->vnew[Z] ) )
        {
            e_min = quad_evaluatev( &quad, info->vnew );
/* MX_ASSERT( e_min > -1000 ); */
            if( q->placement_policy == MX_PLACE_OPTIMAL_BB )
                bb_test_failed = !bb_test( q, info );
        }
        else
            optimize_failed = 1;
    }

    if( q->placement_policy == MX_PLACE_LINE || optimize_failed )
    {
        vi = model_vertex( q->m, i ); 
        vj = model_vertex( q->m, j );   
        optimize_failed = 0;

        if( optimize2v( &quad, &best, vi, vj) ) {
            e_min = quad_evaluatev( &quad, best.elt );
/* MX_ASSERT( e_min > -1000 ); */
            mxv_setv( info->vnew, best.elt, 3 );
        }
        else
            optimize_failed = 1;
    }

    /* for MX_PLACE_OPTIMAL_BB, if the bounding box test fails, 
     * default to same behavior as MX_PLACE_ENDORMID */
    if ( q->placement_policy == MX_PLACE_ENDPOINTS || 
         q->placement_policy == MX_PLACE_ENDORMID  || 
         optimize_failed ||
         bb_test_failed )
    {
        double ei, ej;

        vi = model_vertex( q->m, i ); 
        vj = model_vertex( q->m, j );   
        ei = quad_evaluatev( &quad, vi->elt );
        ej = quad_evaluatev( &quad, vj->elt );
    
        if( ei < ej ) { e_min = ei; mxv_setv( best.elt, vi->elt, 3 ); }
        else          { e_min = ej; mxv_setv( best.elt, vj->elt, 3 ); }
/* MX_ASSERT( e_min > -1000 ); */
    
        if( q->placement_policy >= MX_PLACE_ENDORMID || bb_test_failed )
        {
            /* his original code...*/
            /* Vec3 mid = (vi+vj)/2;*/
            mxv_add( temp.elt, vi->elt, vj->elt, 3 );
            mxv_div( mid.elt, temp.elt, 2.0f, 3 );
            e_mid = quad_evaluatev( &quad, mid.elt );
            if( e_mid < e_min ) { e_min = e_mid; best = mid; }
        }
        mxv_setv( info->vnew, best.elt, 3 );
    }

/*
    if( q->weighting_policy == MX_WEIGHT_AREA )
        e_min /= quad_area( &quad );
*/

    set_heap_key( &(info->h), -e_min );

} /* end function compute_target_placement */



void compute_edge_info( MxQSlim *q, MxQSlimEdge *info )
{
    compute_target_placement( q, info );

    /* apply_penalties( q, info ); */

    enforce_degree_limit( q, info, q->max_degree );

    if( is_in_heap( &(info->h) ) )
        updateh( &(q->heap), &(info->h) );
    else
        inserth( &(q->heap), &(info->h) );

} /* end function compute_edge_info */



void create_edge( MxQSlim *q, MxVertexID i, MxVertexID j )
{
    MxQSlimEdge *info;

    info = ( MxQSlimEdge * ) MX_ALLOC ( sizeof ( MxQSlimEdge ) );
    mxqslimedge_init( info );
    el_add_edge( qs_get_edge_links(q, i), info );
    el_add_edge( qs_get_edge_links(q, j), info );

    info->e.v1 = i;
    info->e.v2 = j;

    compute_edge_info( q, info );
    addpb( &(q->edge_array), info );

} /* end function create_edge */

void collect_edges( MxQSlim *q )
{
    int i, j;
    MxVertexList star;
    MxVertexID vid;

    block_init( &star, sizeof( MxVertexID ) );

    for( i = 0 ; i < model_vertex_count( q->m ) ; i++ )
    {
        resetb( &star );
        collect_vertex_star( q->m, i, &star );

        for( j = 0 ; j < lengthb( &star ) ; j++ )
        {
            vid = vl_get_vertex( &star, j );
            if( i < vid )                   /* Only add particular edge once */
                create_edge( q, i, vid );
        }
    }

    block_cleanup( &star );
}


void update_pre_contract( MxQSlim *q, const MxPairContraction *conx )
{
    MxVertexID v1=conx->v1, v2=conx->v2;
    int i, j;
    MxVertexList star;
    MxEdgeList *el;
    MxEdge *temp;
    MxQSlimEdge *info;
    MxVertexID id, u;

    block_init( &star, sizeof( MxVertexID ) );

    /*
     * Before, I was gathering the vertex "star" using:
     *      collect_vertex_star(m, v1, star);
     * This doesn't work when we initially begin with a subset of
     * the total edges.  Instead, we need to collect the "star"
     * from the edge links maintained at v1.
     */

    for( i = 0 ; i < lengthb( qs_get_edge_links(q, v1) ) ; i++ )
    {
        el = qs_get_edge_links(q, v1);
        temp = &( el_get_edge( el, i )->e );
        id = opposite_vertex( temp, v1 );
        addb( &star, &id );
    }

    for( i = 0 ; i < lengthb( qs_get_edge_links(q, v2)) ; i++ )
    {
        el = qs_get_edge_links(q, v2);
        info = el_get_edge( el, i );

        if( info->e.v1 == v2 ) u = info->e.v2;
        else                   u = info->e.v1;
        MX_ASSERT( info->e.v1==v2 || info->e.v2==v2 );
        MX_ASSERT( u != v2 );

        if( u==v1 || vl_find_vertex( &star, u, NULL ) )
        {
            /* This is a useless link --- kill it */
            MxBool found = find_edge( qs_get_edge_links(q, u), info, &j );
            MX_ASSERT( found );
            removeb( qs_get_edge_links(q, u), j );
            removeh( &(q->heap), &(info->h) );
        }
        else
        {
            /* Relink this to v1 */
            info->e.v1 = v1;
            info->e.v2 = u;
            el_add_edge( qs_get_edge_links(q, v1), info );
        }
    }

    resetb( qs_get_edge_links(q, v2) );
    block_cleanup( &star );

}

void qs_apply_contraction( MxQSlim *q, MxPairContraction *conx )
{
    int i;
    MxEdgeList *el;

    /* Pre-contraction update*/
    q->valid_verts--;
    q->valid_faces -= lengthb( &(conx->dead_faces) );
    quad_add( qs_get_quadrics( q, conx->v1 ), 
              qs_get_quadrics( q, conx->v1 ), 
              qs_get_quadrics( q, conx->v2 ) );

    update_pre_contract( q, conx );

    apply_pair_contraction( q->m, conx );

    /* garland: Must update edge info here so that the meshing penalties*/
    /* will be computed with respect to the new mesh rather than the old*/
    el = qs_get_edge_links( q, conx->v1 );
    for( i = 0 ; i < lengthb( el ) ; i++ )
        compute_edge_info( q, el_get_edge( el, i ) );
}

void update_post_expand( MxQSlim *q, const MxPairContraction *conx )
{
    MxVertexID v1=conx->v1, v2=conx->v2;
    MxVertexList star, star2;
    int i, v1_linked, v2_linked;

    block_init( &star, sizeof( MxVertexID ) );
    block_init( &star2, sizeof( MxVertexID ) );

    collect_vertex_star(q->m, conx->v1, &star);
    collect_vertex_star(q->m, conx->v2, &star2);

    i = 0;
    while( i < lengthb( qs_get_edge_links( q, v1) ) )
    {
        MxEdgeList *el = qs_get_edge_links( q, v1);
        MxQSlimEdge *info = el_get_edge( el, i );
        MxVertexID u;
        if( info->e.v1 == v1 )  u = info->e.v2;
        else                    u = info->e.v1;
        MX_ASSERT( info->e.v1==v1 || info->e.v2==v1 );
        MX_ASSERT( u!=v1 && u!=v2 );

        v1_linked = vl_find_vertex( &star, u, NULL );
        v2_linked = vl_find_vertex( &star2, u, NULL );

        if( v1_linked )
        {
            if( v2_linked )  create_edge( q, v2, u );
            i++;
        }
        else
        {
            /* garland: !!BUG: I expected this to be true, but it's not.
             *         Need to find out why, and whether it's my
             *         expectation or the code that's wrong.
             * SanityCheck(v2_linked);
             */
            info->e.v1 = v2;
            info->e.v2 = u;
            el_add_edge( qs_get_edge_links(q, v2), info );
            removeb( qs_get_edge_links(q, v1), i );
        }

        compute_edge_info( q, info );
    }

    /* garland: ??BUG: Is it legitimate for there not to be an edge here ?? */
    if( vl_find_vertex( &star, v2, NULL ) )
        create_edge( q, v1, v2 );

    block_cleanup( &star );
    block_cleanup( &star2 );
}


MxBool qs_decimate( MxQSlim *q, int target )
{
    MxQSlimEdge *info;
    MxVertexID v1, v2;
    MxPairContraction local_conx;

    mxpaircontraction_init( &local_conx );

    while( q->valid_faces > target )
    {
        info = (MxQSlimEdge *) extracth ( &(q->heap) );
        if( !info ) { 
            mxpaircontraction_cleanup( &local_conx );
            return 0; 
        }

/* at various times, this probe has proved to be extremely valuable */
/*
        if( firsttime )
        {
            firsttime = 0;
            fp = fopen( "coneout.txt", "w" );
            MX_ASSERT( fp );
        }
        fprintf( fp, "%f \n", (double)(info->h.import) );
        debug_count++;
        if( debug_count > 1000 )
        {
            fclose( fp );
            exit(0);
        }
*/

        v1=info->e.v1;
        v2=info->e.v2;

        if( vertex_is_valid( q->m, v1) && vertex_is_valid( q->m, v2) )
        {
            MxPairContraction *next_conx=NULL;
            /* if( contraction_log ) next_conx=contraction_log->next_record(); */
            MxPairContraction *conx = next_conx?next_conx:&local_conx;
    
            compute_pair_contraction( q->m, v1, v2, conx );
            /*
             * garland: !!BUG: Need to give back the record allocated by
             *        next_record() (if any).
             * tjh: ?
             */
            if( q->will_join_only && lengthb( &(conx->dead_faces) )>0 ) continue;
            conx->dv1[X] = info->vnew[X] - model_vertex( q->m, v1 )->elt[X];
            conx->dv1[Y] = info->vnew[Y] - model_vertex( q->m, v1 )->elt[Y];
            conx->dv1[Z] = info->vnew[Z] - model_vertex( q->m, v1 )->elt[Z];
            conx->dv2[X] = info->vnew[X] - model_vertex( q->m, v2 )->elt[X];
            conx->dv2[Y] = info->vnew[Y] - model_vertex( q->m, v2 )->elt[Y];
            conx->dv2[Z] = info->vnew[Z] - model_vertex( q->m, v2 )->elt[Z];

            /* if( contraction_log ) contraction_log->contract(conx); */
            if( q->contraction_callback )
                (*(q->contraction_callback))(conx, -get_heap_key( &(info->h) ));
        
            qs_apply_contraction( q, conx );
        }

    }

    mxpaircontraction_cleanup( &local_conx );
    return 1;
}


void mxqslimedge_init( MxQSlimEdge *qse )
{
    mxheapable_init( &(qse->h) );
    qse->h.payload = qse;
}


MxBool find_edge( MxEdgeList *el, const MxQSlimEdge *t, int *index )
{
    int i;
    for( i = 0 ; i < lengthb( el ) ; i++ )
        if( el_get_edge( el,  i ) == t )
        {
            if( index ) *index = i;
            return 1;
        }
    return 0;
}

#endif
