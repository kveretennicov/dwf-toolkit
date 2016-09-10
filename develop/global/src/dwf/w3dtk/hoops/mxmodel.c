/************************************************************************

  MxModel

  Copyright (C) 1998 Michael Garland.  See "COPYING.txt" for details.
  
  $Id: //DWF/Working_Area/Willie.Zhu/w3dtk/hoops/mxmodel.c#1 $

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
#include <string.h>

const vertex_data vblank = { 0, 0, 0, 0 };
const face_data   fblank = { 0, 0, 0, 0 };



void mxmodel_init( MxModel *m, int nvert, int nface )
{
    block_init( &(m->vertices),   sizeof( MxVertex ) );
    block_init( &(m->faces),      sizeof( MxFace ) );
    block_init( &(m->face_links), sizeof( MxFaceList * ) );
    block_init( &(m->v_data),     sizeof( vertex_data ) );
    block_init( &(m->f_data),     sizeof( face_data ) );
    block_init( &(m->face_map1),  sizeof( int ) );
    block_init( &(m->face_map2),  sizeof( int ) );
    block_init( &(m->vertex_map), sizeof( int ) );
    m->colors = NULL;  m->normals = NULL;  m->tcoords = NULL;
    m->cbinding = m->nbinding = m->tbinding = MX_UNBOUND;
    m->binding_mask = MX_ALL_MASK;

    /* Prevent VC from complaining about error C4100 */
    nvert;
    nface;
}

void mxmodel_cleanup( MxModel *m )
{
    int i;

    /* clean up the face links block */
    for( i = 0 ; i < lengthb( &(m->face_links) ) ; i++ )  
    {
        block_cleanup( getpb( &(m->face_links), i ) );
        MX_FREE( getpb( &(m->face_links), i ), sizeof( Block ) );
    }
    block_cleanup( &(m->face_links) );

    /* clean up everything else */
    block_cleanup( &(m->vertices) );
    block_cleanup( &(m->faces) );
    block_cleanup( &(m->face_links) );
    block_cleanup( &(m->v_data) );
    block_cleanup( &(m->f_data) );
    block_cleanup( &(m->face_map1) );
    block_cleanup( &(m->face_map2) );
    block_cleanup( &(m->vertex_map) );

    if( m->normals ) MX_FREE( m->normals, sizeof( Block ) );
    if( m->colors )  MX_FREE( m->colors,  sizeof( Block ) );
    if( m->tcoords ) MX_FREE( m->tcoords, sizeof( Block ) );
}




void free_vertex(MxModel *m, MxVertexID v)
{
    removeb( &(m->face_links), v);
}

MxFaceID alloc_face(MxModel *m, MxVertexID v1, MxVertexID v2, MxVertexID v3)
{
    MxFace temp;
    MxFaceID id;

    mxface_init( &temp, v1, v2, v3 );
    id = addb( &(m->faces), &temp);
    addb( &(m->f_data), &fblank );
    face_mark_valid( m, id );

    return id;
}

void init_face(MxModel *m, MxFaceID id)
{
    MxFaceList *N1, *N2, *N3;
    N1 = neighbors( m, model_face( m, id )->v[0] );
    N2 = neighbors( m, model_face( m, id )->v[1] );
    N3 = neighbors( m, model_face( m, id )->v[2] );
    addb( N1, &id );
    addb( N2, &id );
    addb( N3, &id );

}

void mark_neighborhood( MxModel *m, MxVertexID vid, unsigned short mark )
{
    int i, maxi;
    MxFaceID fid;
    MX_ASSERT( vid < model_vertex_count( m ) ); 
    maxi = lengthb( neighbors(m, vid) );

    for( i = 0 ; i < maxi ; i++ )
    {
        fid = fl_get_face( neighbors(m, vid), i );
        fmark( m, fid, (unsigned char) mark );
    }
}

void collect_unmarked_neighbors(MxModel *m, MxVertexID vid,MxFaceList *faces)
{
    int i, fid, maxi;
    MX_ASSERT( vid < model_vertex_count(m) ); 

    maxi = lengthb( neighbors(m, vid) );
    for( i = 0 ; i < maxi ; i++ )
    {
        fid = fl_get_face( neighbors(m, vid), i);
        if( !get_fmark(m, fid) )
        {
            addb( faces, &fid );
            fmark( m, fid, 1 );
        }
    }
}

void partition_marked_neighbors(MxModel *m, MxVertexID v, unsigned short pivot,
                        MxFaceList *lo, MxFaceList *hi)
{
    int i, f, maxi;

    MX_ASSERT( v < model_vertex_count(m) );
    maxi = lengthb( neighbors(m, v) );
    for( i = 0 ; i < maxi ; i++ )
    {
        f = fl_get_face( neighbors(m, v), i);
        if( get_fmark(m, f) )
        {
            if( get_fmark(m, f) < pivot )  
                addb( lo, &f );
            else  
                addb( hi, &f );
            fmark(m, f, 0);
        }
    }
}

void mark_corners(MxModel *m, MxFaceList *faces, unsigned short mark)
{
    int i, j;
    MxFaceID fid;
    MxVertexID vid;

    for( i = 0 ; i < lengthb( faces ) ; i++ )
        for( j = 0 ; j < 3; j++ ) 
        {
            fid = fl_get_face( faces, i );
            vid = model_face( m, fid )->v[j];
            vmarkc(m, vid, (unsigned char) mark);
        }
}

void collect_unmarked_corners(MxModel *m, MxFaceList *faces,
                      MxVertexList *verts)
{
    int i, j; 
    for( i = 0 ; i < lengthb( faces ) ; i++ )
        for( j = 0 ; j < 3 ; j++ )
        {
            MxVertexID v = model_face( m, fl_get_face( faces, i ))->v[j];
            if( !get_vmark(m, v) )
            {
                addb( verts, &v );
                vmarkc( m, v, 1 );
            }
        }
}


void collect_edge_neighbors( MxModel *m, MxVertexID v1, MxVertexID v2,
                    MxFaceList *faces )
{
    mark_neighborhood( m, v1, 1 );
    mark_neighborhood( m, v2, 0 );
    collect_unmarked_neighbors( m, v1, faces );
}


void collect_vertex_star( MxModel *m, MxVertexID v, MxVertexList *verts )
{
    MxFaceList *N = neighbors(m, v);

    mark_corners(m, N, 0);
    vmarkc(m, v, 1); /* Don't want to include v in the star*/
    collect_unmarked_corners(m, N, verts);
}

/* see macro in MxModel.h
MxFaceList *neighbors( MxModel *m, MxVertexID v) 
{ 
    return (MxFaceList *) getpb( &(m->face_links), v ); 
}
*/

void compute_vertex_normal(MxModel *m, MxVertexID v, double *n)
{
    int i;
    double fn[3];
    MxFaceList *star = neighbors(m, v);

    mxv_set(n, 0.0f, 3);

    for( i = 0 ; i < lengthb( star ) ; i++ )
    {

        /* Weight normals uniformly */
        compute_face_normal(m, fl_get_face( star, i ), fn, 0 );
        /* Weight normals by angle around vertex
         *      uint c = face(star[i]).find_vertex(v);
         *      compute_face_normal(star[i], fn);
         *      mxv_scale(fn, compute_corner_angle(star[i], c), 3);
         */
        mxv_add(n, n, fn, 3);
    }
    if( i>0 )
        mxv_unitize(n, 3);
}


void model_remap_vertex( MxModel *m, MxVertexID from, MxVertexID to)
{
    int i;
    int *ptri;
    MxFace *f;
#if 0
    int temp1 = vertex_is_valid( m, from );
    int temp2 = vertex_is_valid( m, to );
#endif
    MX_ASSERT( from < model_vertex_count( m ) ); 
    MX_ASSERT( to < model_vertex_count( m ) ); 
    MX_ASSERT( vertex_is_valid( m, from ) );
    MX_ASSERT( vertex_is_valid( m, to ) );
    
    for( i = 0 ; i < lengthb( neighbors( m, from ) ) ; i++ )
    {
        f = model_face( m, fl_get_face( neighbors( m, from ), i ));
        face_remap_vertex( f, from, to );
    }

    mark_neighborhood( m, from, 0 );
    mark_neighborhood( m, to, 1 );
    collect_unmarked_neighbors( m, from, neighbors( m, to ) );

    vertex_mark_invalid( m, from );
    resetb( neighbors( m, from ) );   /* remove links in old vertex*/

    /* keep track of where vertices are going to */
    ptri = (int *) getb ( &(m->vertex_map), from );
    *ptri = to;
}


MxVertexID split_edge_simple( MxModel *m, MxVertexID a, MxVertexID b )
{
    double *v1 = model_vertex( m, a )->elt;
    double *v2 = model_vertex( m, b )->elt;

    return split_edge( m, a, b,
                      (v1[X] + v2[X])/2.0f,
                      (v1[Y] + v2[Y])/2.0f,
                      (v1[Z] + v2[Z])/2.0f);
}

static
void remove_face(MxFaceList *faces, unsigned int f)
{
    int j;
    if( fl_find_face( faces, f, &j ) )
        removeb( faces, j);
}

MxVertexID split_edge( MxModel *m, MxVertexID v1, MxVertexID v2,
                double x, double y, double z)
{
    int vnew, i;
    MxFaceList faces;
    MxFaceID fid;
    MxVertexID v3;
    MxFace *f;
    
    MX_ASSERT( v1 < model_vertex_count(m) );   
    MX_ASSERT( v2 < model_vertex_count(m) );
    MX_ASSERT( vertex_is_valid(m, v1) ); 
    MX_ASSERT( vertex_is_valid(m, v2) );
    MX_ASSERT( v1 != v2 );

    block_init( &faces, sizeof( MxFaceID ) );

    collect_edge_neighbors(m, v1, v2, &faces);
    MX_ASSERT( lengthb( &faces ) > 0 );

    vnew = add_vertex( m, x, y, z );

    for( i = 0 ; i < lengthb( &faces ) ; i++ )
    {
        fid = fl_get_face( &faces, i );
        v3 = face_opposite_vertex( model_face( m, fid ), v1, v2 );
        MX_ASSERT( v3!=v1 && v3!=v2 );
        MX_ASSERT( vertex_is_valid(m, v3) );

        /* in fid, remap v2-->vnew*/
        f = model_face( m, fid);
        face_remap_vertex( f, v2, vnew );
        addb( neighbors( m, vnew ), &fid );

        /* remove fid from neighbors(v2)*/
        remove_face( neighbors( m, v2 ), fid );

        /* assure orientation is consistent*/
        if( face_is_inorder( model_face( m, fid ), vnew, v3 ) )
            add_face( m, vnew, v2, v3 );
        else
            add_face( m, vnew, v3, v2 );
    }

    block_cleanup( &faces );
    return vnew;
}

void flip_edge( MxModel *m, MxVertexID v1, MxVertexID v2)
{
    MxFaceList faces;  
    MxFaceID f1, f2;
    MxVertexID v3, v4;
    MxFace *f;

    block_init( &faces, sizeof( MxFaceID ) );

    collect_edge_neighbors( m, v1, v2, &faces );
    if( lengthb( &faces ) != 2 ) {
        block_cleanup( &faces );
        return;
    }

    f1 = fl_get_face( &faces, 0 );
    f2 = fl_get_face( &faces, 1 );
    v3 = face_opposite_vertex( model_face( m, f1 ), v1, v2 );
    v4 = face_opposite_vertex( model_face( m, f2 ), v1, v2 );

    /* ?? Should we check for convexity or assume thats been taken care of?*/
    remove_face(neighbors(m, v1), f2);
    remove_face(neighbors(m, v2), f1);
    addb( neighbors(m, v3), &f2 );
    addb( neighbors(m, v4), &f1 );

    f = model_face( m, f1);
    face_remap_vertex( f, v2, v4 );
    f = model_face( m, f2);
    face_remap_vertex( f, v1, v3 );

    block_cleanup( &faces );

}


void split_face4( MxModel *m, MxFaceID f, MxVertexID *newverts)
{
    int v0 = model_face( m, f )->v[0];
    int v1 = model_face( m, f )->v[1];
    int v2 = model_face( m, f )->v[2];

    int pivot = split_edge_simple( m, v0, v1 );
    int new1  = split_edge_simple( m, v1, v2 );
    int new2  = split_edge_simple( m, v0, v2 );

    if( newverts )
    {
        newverts[0] = pivot;
        newverts[1] = new1;
        newverts[2] = new2;
    }

    flip_edge( m, pivot, v2 );
}


void compact_vertices( MxModel *m )
{
    int i, maxi;
    MxVertexID oldID;
    MxVertexID newID = 0;

    for( oldID = 0 ; oldID < model_vertex_count(m) ; oldID++ )
    {
        if( vertex_is_valid(m, oldID) )
        {
            if( newID != oldID )
            {
                swapb( &(m->vertices), oldID, newID );

                if( normal_binding(m) == MX_PERVERTEX )
                {
                    MX_ASSERT( m->normals != NULL );
                    swapb( m->normals, oldID, newID );
                }
                if( color_binding(m) == MX_PERVERTEX )
                {
                    MX_ASSERT( m->colors != NULL );
                    swapb( m->colors, oldID, newID );
                }
                if( texcoord_binding(m) == MX_PERVERTEX )
                {
                    MX_ASSERT( m->tcoords != NULL );
                    swapb( m->tcoords, oldID, newID );
                }

                /* garland: Because we'll be freeing the link lists for the
                 * old vertices, we actually have to swap values instead
                 * of the simple copying in the block above.
                 */
                swapb( &(m->face_links), newID, oldID );

                /* garland:
                 *   vertex_mark_valid( m, newID );
                 *   model_remap_vertex( m, oldID, newID ); 
                 * I had thought the above snippet would work.
                 * The following block of code seems to match
                 * model_remap_vertex closely, but it fails for some reason
                 */
                vertex_mark_valid( m, newID );
                maxi = lengthb( neighbors( m,newID) );
                for( i = 0 ; i < maxi ; i++ )
                {
                    MxFace *f;
                    f = model_face( m, fl_get_face( neighbors( m, newID), i));
                    face_remap_vertex( f, oldID, newID );
                }
                /* keep track of where vertices are going to */
                *((int *) getb ( &(m->vertex_map), oldID )) = newID;

            }
            newID++;
        }
    }

    for( i = lengthb( &(m->vertices) ) ; i > newID ; i-- )
    {
        chopb( &(m->vertices) );
    }
}

void unlink_face( MxModel *m, MxFaceID fid)
{
    MxFace *f = model_face( m, fid);
    int j, found=0;

    face_mark_invalid( m, fid);

    if( fl_find_face( neighbors( m, f->v[0] ), fid, &j ) ) { 
        found++; 
        removeb( neighbors( m, f->v[0] ), j );
    }
    if( fl_find_face( neighbors( m, f->v[1] ), fid, &j ) ) { 
        found++; 
        removeb( neighbors( m, f->v[1] ), j );
    }
    if( fl_find_face( neighbors( m, f->v[2] ), fid, &j ) ) { 
        found++; 
        removeb( neighbors( m, f->v[2] ), j ); 
    }
    MX_ASSERT( found > 0 );
    MX_ASSERT( !fl_find_face( neighbors( m, f->v[0] ), fid, &j ) );
    MX_ASSERT( !fl_find_face( neighbors( m, f->v[1] ), fid, &j ) );
    MX_ASSERT( !fl_find_face( neighbors( m, f->v[2] ), fid, &j ) );
}

#if 0
void remove_degeneracy( MxModel *m, MxFaceList *faces_in )
{
    int i;
    MxFace *f;

    for( i = 0 ; i < lengthb( faces_in ) ; i++ )
    {
        MX_ASSERT( face_is_valid( m, fl_get_face( faces_in, i )) );
        f = model_face( m, fl_get_face( faces_in, i ));

        if( f->v[0]==f->v[1] || f->v[1]==f->v[2] || f->v[0]==f->v[2] )
            unlink_face( m, fl_get_face( faces_in, i ));
    }
}
#endif

void compute_pair_contraction( MxModel *m, MxVertexID v1, MxVertexID v2,
                     MxPairContraction *conx)
{
    int i, maxi;
    MxFace *f;
    int v1_matched;
    MxFaceList *N1;
    MxFaceID fid;

    conx->v1 = v1;
    conx->v2 = v2;
    conx->dv1[X] = conx->dv1[Y] = conx->dv1[Z] = 0.0;
    conx->dv2[X] = conx->dv2[Y] = conx->dv2[Z] = 0.0;

    resetb( &(conx->delta_faces) );
    resetb( &(conx->dead_faces) );

    /* Assign neighbors of v2 to dead_faces and delta_faces*/
    maxi = lengthb( neighbors( m, v2) );
    for( i = 0 ; i < maxi ; i++ )
    {
        f = model_face( m, fl_get_face( neighbors( m, v2), i));
        v1_matched = ( f->v[0]==v1 || f->v[1]==v1 || f->v[2]==v1 );
        N1 = neighbors( m, v2 );
        fid = fl_get_face( N1, i );
        if( v1_matched ) 
            addb( &(conx->dead_faces), &fid );
        else             
            addb( &(conx->delta_faces), &fid );
    }
}

void apply_pair_contraction( MxModel *m, MxPairContraction *conx )
{
    int i;
    MxVertexID v1=conx->v1, v2=conx->v2;
    MxFaceID fid;
    MxFace *f;
    MxVertex *v;

    /* Move v1 to new position*/
    v = model_vertex( m, v1);
    mxv_add( v->elt, v->elt, conx->dv1, 3);

    /* Remove dead faces*/
    for( i = 0 ; i < lengthb( &(conx->dead_faces) ) ; i++ )
    {
        fid = get_dead_faces( conx, i );
        unlink_face( m, fid);
    }

    /* Update changed faces*/
    for( i = 0 ; i < lengthb( &(conx->delta_faces) ) ; i++ )
    {
        fid = get_delta_faces( conx, i );
        f = model_face( m, fid );
        face_remap_vertex( f, v2, v1 );
        addb( neighbors( m, v1), &fid );
    }

    /* !!HACK: This is really only a temporary solution to the problem*/
    /*
    if( normal_binding( m ) == MX_PERFACE )
    {
        for( i = 0 ; i < lengthb( &(conx->delta_faces) ) ; i++ )
        {
            mxnormal_init( &temp, n[0], n[1], n[2] );
            compute_face_normal( m, get_delta_faces( conx, i ), n, 1 );
            out = model_normal( m, get_delta_faces( conx, i ) );
            memcpy( out, &temp, sizeof( MxNormal ) );
        }
    }
    */

    /* Kill v2*/
    /* vertex_mark_invalid( m, v2); */
    model_remap_vertex( m, v2, v1 );
    resetb( neighbors( m, v2 ) );

} /* end function apply_pair_contraction */


void apply_pair_expansion( MxModel *m, MxPairExpansion *conx )
{
    int i, j;
    int found;
    MxVertexID v1=conx->v1, v2=conx->v2;
    MxFaceID fid;
    MxFace *f;

    mxv_sub( model_vertex( m, v2 )->elt, model_vertex( m, v1 )->elt, conx->dv2, 3);
    mxv_sub( model_vertex( m, v1 )->elt, model_vertex( m, v1 )->elt, conx->dv1, 3);

    for( i = 0 ; i < lengthb( &(conx->dead_faces) ) ; i++ )
    {
        fid = get_dead_faces( conx, i );
        face_mark_valid( m, fid);
        addb( neighbors( m, model_face( m, fid)->v[0]), &fid );
        addb( neighbors( m, model_face( m, fid)->v[1]), &fid );
        addb( neighbors( m, model_face( m, fid)->v[2]), &fid );
    }

    for( i = 0 ; i < lengthb( &(conx->delta_faces) ) ; i++ )
    {
        fid = get_delta_faces( conx, i );
        f = model_face( m, fid);
        face_remap_vertex( f, v1, v2 );
        addb( neighbors( m, v2 ), &fid );
        found = fl_find_face( neighbors( m, v1), fid, &j);
        MX_ASSERT( found );
        removeb( neighbors( m, v1 ), j );
    }

    /* !!HACK: This is really only a temporary solution to the problem*/
    if( normal_binding( m ) == MX_PERFACE )
    {
        /* flat shading is not supported */
        MX_ASSERT( 0 );
    }
    vertex_mark_valid( m, v2);
}


void pair_contract( MxModel *m, MxVertexID v1, MxVertexID v2,
              const double *vnew, MxPairContraction *conx)
{
    compute_pair_contraction( m, v1, v2, conx );
    mxv_sub(conx->dv1, vnew, model_vertex( m, v1 )->elt, 3);
    mxv_sub(conx->dv2, vnew, model_vertex( m, v2 )->elt, 3);
    apply_pair_contraction( m, conx );
}


#if 0
static void contract( MxModel *m, MxVertexID v1, MxVertexID v2, MxVertexID v3,
              const double *vnew,
              MxFaceList *changed)
{
    mark_neighborhood( m, v1, 0 );
    mark_neighborhood( m, v2, 0 );
    mark_neighborhood( m, v3, 0 );
    resetb( changed );
    collect_unmarked_neighbors( m, v1, changed );
    collect_unmarked_neighbors( m, v2, changed );
    collect_unmarked_neighbors( m, v3, changed );

    /* Move v1 to vnew */
    mxv_setv( model_vertex( m, v1)->elt, vnew, 3 );

    /* Replace occurrences of v2 & v3 with v1 */
    model_remap_vertex( m, v2, v1 );
    model_remap_vertex( m, v3, v1 );

    remove_degeneracy( m, changed );

    /* !!HACK: Only a temporary solution */
    if( normal_binding( m ) == MX_PERFACE )
    {
        /* flat shading is not supported */
        MX_ASSERT( 0 );
    }
}

static void contractv( MxModel *m, MxVertexID v1, MxVertexList *rest,
              const double *vnew, MxFaceList *changed)
{
    int i;
    MxVertex *vptr;

    /* Collect all effected faces*/
    mark_neighborhood( m, v1, 0);
    for( i = 0 ; i < lengthb( rest ) ; i++ )
        mark_neighborhood( m, vl_get_vertex( rest, i ), 0);

    resetb( changed );

    collect_unmarked_neighbors( m, v1, changed);
    for( i = 0 ; i < lengthb( rest ) ; i++ )
        collect_unmarked_neighbors( m, vl_get_vertex( rest, i ), changed);

    /* Move v1 to vnew*/
    vptr = model_vertex( m, v1 );
    mxv_setv( vptr->elt, vnew, 3);

    /* Replace all occurrences of vi with v1*/
    for( i = 0 ; i < lengthb( rest ) ; i++ )
        model_remap_vertex( m, vl_get_vertex( rest, i ), v1);

    remove_degeneracy( m, changed );
}
#endif


/*
 *
 * Everything else comes from the original MxBlockModel
 *
 */


MxVertexID add_vertex(MxModel *m, double x, double y, double z)
{
    MxVertexID vid, vid2, vid3;
    MxFaceList *fl;


    MxVertex temp;
    temp.elt[0] = x;
    temp.elt[1] = y;
    temp.elt[2] = z;
    vid = addb( &(m->vertices), &temp );
    addb( &(m->v_data), &vblank );
    vertex_mark_valid( m, vid);

    vid3 = addb( &(m->vertex_map), &vid );      /* (will not modify vid) */


    /* these two calls have to be (and are) cleaned up in mxmodel_cleanup */
    fl = (MxFaceList *) MX_ALLOC ( sizeof( MxFaceList ) );
    block_init( fl, sizeof( MxFaceID ) ); 
    vid2 = addpb( &(m->face_links), fl );
    MX_ASSERT( vid2 == vid );
    MX_ASSERT( vid3 == vid );
    MX_ASSERT( lengthb( neighbors( m, vid ) ) == 0 );

    return vid;
}


void remove_vertex( MxModel *m, MxVertexID v )
{
    MX_ASSERT( v < lengthb( &(m->vertices) ) );

    free_vertex( m, v );
    removeb( &(m->vertices), v );
    if( normal_binding(m) == MX_PERVERTEX ) removeb( m->normals, v );
    if( color_binding(m) == MX_PERVERTEX ) removeb( m->colors, v);
    if( texcoord_binding(m) == MX_PERVERTEX ) removeb( m->tcoords, v);
}

MxFaceID add_face( MxModel *m, int v1, int v2, int v3 )
{
    MxFaceID id = alloc_face( m, v1, v2, v3 );
    init_face( m, id );
    return id;
}

int add_color( MxModel *m, double r, double g, double b )
{
    MxColor c;

    MX_ASSERT( m->colors );
    mxcolor_init( &c, r, g, b );
    return addb( m->colors, &c );
}

int add_normal( MxModel *m, double x, double y, double z )
{
    MxNormal n;

    MX_ASSERT( m->normals );
    mxnormal_init( &n, x, y, z );
    return addb( m->normals, &n );
}

int add_texcoord( MxModel *m,double s, double t)
{
    MxTexCoord temp;

    MX_ASSERT( m->tcoords );
    mxtexcoord_init( &temp, s, t );
    return addb( m->tcoords, &temp );
}


/*  
 * Property binding
 */

static const char *bindings[] = {
    "unbound",
    "face",
    "vertex",
    NULL
};


#ifndef MAX
#  define MAX(a,b) (((a)>(b))?(a):(b))
#endif

#if 0
static
int binding_size( MxModel *m, unsigned char i )
{
    switch( i )
    {
        case MX_UNBOUND: return 0;
        case MX_PERVERTEX: return MAX( 1, model_vertex_count(m) );
        case MX_PERFACE: return MAX( 1, model_face_count(m) );
        default: return 0;
    }
}
#endif

/* HELPER */    
static MxBool streq(const char *a, const char *b) 
{ 
    return !strcmp(a,b); 
}

int parse_binding( MxModel *m, const char *name)
{
    int i;

    /* Prevent VC complaining about error C4100 */
    m;

    for( i = 0 ; i <= MX_MAX_BINDING ; i++ )
        if( streq(bindings[i], name) )  
            return i;

    /* ??BUG: Is this a reasonable default?*/
    return MX_UNBOUND;
}



/*  
 * Utility methods for computing characteristics of faces.
 */

int compute_face_normal( MxModel *m, MxFaceID f, double *n, MxBool will_unitize)
{
    double *v1 = model_vertex( m, model_face( m, f)->v[0])->elt;
    double *v2 = model_vertex( m, model_face( m, f)->v[1])->elt;
    double *v3 = model_vertex( m, model_face( m, f)->v[2])->elt;

    double a[3], b[3];

    mxv_sub(a, v2, v1, 3);
    mxv_sub(b, v3, v1, 3);
    mxv_cross( n, a, b, 3);
    if( will_unitize )
        return mxv_unitize(n, 3);

    return 1;

} /* end function compute_face_normal */


double compute_face_area( MxModel *m, MxFaceID f)
{
    double n[3];

    compute_face_normal( m, f, n, 0 );
    return 0.5f * mxv_len(n, 3);
}


double compute_face_perimeter( MxModel *m, MxFaceID fid, MxBool *flags )
{
    int i;
    double perim = 0.0;
    double *vi, *vj, e[3];
    const MxFace *f = model_face( m, fid );

    for( i = 0 ; i < 3 ; i++ )
    {
        if( !flags || flags[i] )
        {
            vi = model_vertex( m, f->v[i] )->elt;  
            vj = model_vertex( m, f->v[(i+1)%3] )->elt;
            mxv_sub(e, vi, vj, 3);
            perim += mxv_len(e, 3);
        }   
    }

    return perim;
}

double compute_corner_angle( MxModel *m, MxFaceID f, int i)
{
    int i_prev = (i==0)?2:i-1;
    int i_next = (i==2)?0:i+1;
    double e_prev[3], e_next[3];
    MxVertex *vprev, *vnext, *vi;

    vprev = model_corner( m, f, i_prev );
    vnext = model_corner( m, f, i_next );
    vi    = model_corner( m, f, i );
    mxv_sub(e_prev, vprev->elt, vi->elt, 3);
    mxv_sub(e_next, vnext->elt, vi->elt, 3);
    mxv_unitize(e_prev, 3);
    mxv_unitize(e_next, 3);

    return (double) acos( mxv_dot(e_prev, e_next, 3) );

}


/*
 * The rest of the file is functions that were originally defined in the header
 * file as inline functions
 */

void mxpaircontraction_init( MxPairContraction *mpc )
{ 
    block_init( &(mpc->delta_faces), sizeof( int ) );
    block_init( &(mpc->dead_faces), sizeof( int ) );
}

void mxpaircontraction_cleanup( MxPairContraction *mpc )
{ 
    block_cleanup( &(mpc->delta_faces) );
    block_cleanup( &(mpc->dead_faces) );
}


/* model_valid_face_count
 * This function is pretty inefficient, so use sparingly */
int model_valid_face_count( MxModel *m )
{
    int i, maxi, count = 0;

    maxi = model_face_count( m );
    for( i = 0 ; i < maxi ; i++  )
    {
        if( face_is_valid( m, i ) )
            count++;
    }
    return count;

} /* end function model_valid_face_count */



/* model_valid_face_count
 * This function is pretty inefficient, so use sparingly */
int model_valid_vertex_count( MxModel *m )
{
    int i, maxi, count = 0;

    maxi = model_vertex_count( m );
    for( i = 0 ; i < maxi ; i++  )
    {
        if( vertex_is_valid( m, i ) )
            count++;
    }
    return count;

} /* end function model_valid_vertex_count */

#endif
