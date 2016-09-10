/************************************************************************

  MxMain.c

  This file provides the external interface for the LOD module.
  All communication to and from the outside world should pass through
  this file, through the function LOD_main.

  This file should be the only C file that knows about anything from HOOPS

  Copyright (C) 1998 Michael Garland.  See "COPYING.txt" for details.
  
  $Id: //DWF/Working_Area/Willie.Zhu/w3dtk/hoops/mxmain.c#1 $

 ************************************************************************/

/*
#include "hoops.h"
*/
#include "lod.h"


/*
 *  This function handles the interface to the function that extracts the 
 *  shell from the hoops metafile.
 *  fmap_intermediate describes the mapping from the original shell faces 
 *  to the first triangle in the triangle map
 */
static int shell_to_model( MxModel *m, 
                int plist_len, const float *plist, 
                int flist_len, const int *flist,
                int *fmap_intermediate )
{
    const float *ptrf;
    const int *ptri;
    int i, j, count;
    int num_triangles = 0;
    int num_shell_faces = 0;

    for( i = 0, ptrf = plist ; i < plist_len ; i++, ptrf += 3 ) {
    add_vertex( m, ptrf[0], ptrf[1], ptrf[2] );
    }

    for( ptri = flist ; ptri < flist + flist_len ; ptri += count + 1 ) {
    count = *ptri;
    MX_ASSERT( count >= 0 );
    fmap_intermediate[ num_shell_faces++ ] = num_triangles;
    for( j = 3 ; j <= count ; j++ ) {
        add_face( m, ptri[ 1 ], ptri[ j-1 ], ptri[ j ] );
        num_triangles++;
    }
    }

    return num_triangles;

} /* end function shell_to_model */


/* model_to_shell
 * note: There are two levels of indirection in vertex maps--
 *          1) the first maps from input vertices to model vertex IDs
 *          2) the second maps from model vertex IDs to their compacted array 
 *              representation that has invalid vertices removed.
 *       vmap_intermediate expresses #2, whereas vmap_final expresses both combined
 */
static void model_to_shell( MxModel *m,
        int *pcount, float *plist, 
        int *flist_len, int   *flist,
        int *vmap_final, int vmap_final_len )
{
    double *fptr_in;
    float *fptr_out;
    int *iptr_in, *iptr_out;
    int i, maxi;
    int validvertices = 0;
    int *vmap_intermediate, vmap_intermediate_len;

    /* in this block we output vertices and create vmap_intermediate */
    fptr_out = plist;
    maxi = vmap_intermediate_len = model_vertex_count( m );
    vmap_intermediate = MX_ALLOC( (long)(vmap_intermediate_len * sizeof( int )) );
    if( vmap_intermediate == NULL )
        MX_ERROR( ERR_INSUFFICIENT_MEMORY, "memory allocation failed" );

    for( i = 0 ; i < maxi ; i++ ) {
        vmap_intermediate[i] = -1;
        if( vertex_is_valid(m, i) ) {
        vmap_intermediate[i] = validvertices++;
        fptr_in = model_vertex( m, i )->elt;
        fptr_out[0] = (float) (fptr_in[0]);
        fptr_out[1] = (float) (fptr_in[1]);
        fptr_out[2] = (float) (fptr_in[2]);
        fptr_out += 3;
    }
    }
    *pcount = validvertices;

    /* in this block we output faces
     * assumes that model has triangles only -- will break otherwise 
     */
    iptr_out = flist;
    maxi = model_face_count( m );
    for( i = 0 ; i < maxi ; i++  )
    {
    if( face_is_valid( m, i ) )
    {
        iptr_in = model_face( m, i )->v;
        iptr_out[0] = 3;
        iptr_out[1] = vmap_intermediate[ iptr_in[0] ];
        iptr_out[2] = vmap_intermediate[ iptr_in[1] ];
        iptr_out[3] = vmap_intermediate[ iptr_in[2] ];
        MX_ASSERT( ( iptr_out[1] >= 0 ) && ( iptr_out[1] < *pcount) );
        MX_ASSERT( ( iptr_out[2] >= 0 ) && ( iptr_out[2] < *pcount) );
        MX_ASSERT( ( iptr_out[3] >= 0 ) && ( iptr_out[3] < *pcount) );
        iptr_out += 4;
    }
    }
    *flist_len = (int)(iptr_out - flist);

    /* in this block we create vmap_final, if appropriate */
    if( vmap_final != NULL )
    {
        for( i = 0 ; i < vmap_final_len ; i++ )
        {
            vmap_final[i] = vmap_intermediate[ model_vertex_map_entry( m, i ) ];
            MX_ASSERT( vmap_final[i] < validvertices );
        }
    }

    MX_FREE( vmap_intermediate, (long)(vmap_intermediate_len * sizeof( int )) );

} /* end function model_to_shell */


/*  model_to_shell_flist_only
 *  When MX_PLACE_ENDPOINTS is used, all representations index back
 *  into the same points array.  This corresponds to the HOOPS "conserve memory"
 *  setting.
 */
static void model_to_shell_flist_only(
                MxModel *m,
                int *pcount, 
                int *flist_len, int *flist )
{
    int *iptr_in, *iptr_out;
    int i, maxi;
    int numvertices, validvertices = 0;

    maxi = numvertices = model_vertex_count( m );
    for( i = 0 ; i < maxi ; i++ )
        if( vertex_is_valid(m, i) )
            validvertices++;
    *pcount = validvertices; /* pcount may or may not be needed later */

    iptr_out = flist;
    maxi = model_face_count( m );
    for( i = 0 ; i < maxi ; i++  )
    {
        if( face_is_valid( m, i ) )
        {
            iptr_in = model_face( m, i )->v;
            /* assumes triangles only */
            iptr_out[0] = 3;
            /* note: unlike model_to_shell, no vertex mapping */
            iptr_out[1] = iptr_in[0];
            iptr_out[2] = iptr_in[1];
            iptr_out[3] = iptr_in[2];
            MX_ASSERT( ( iptr_out[1] >= 0 ) && ( iptr_out[1] < numvertices ) );
            MX_ASSERT( ( iptr_out[2] >= 0 ) && ( iptr_out[2] < numvertices ) );
            MX_ASSERT( ( iptr_out[3] >= 0 ) && ( iptr_out[3] < numvertices ) );
            iptr_out += 4;
        }
    }
    *flist_len = (int)(iptr_out - flist);

} /* end function model_to_shell_flist_only */


#if 0
static void create_mapping_list( MxModel *m, int *vertex_map )
{
    int i, *iptr, *iptr2;

    if( vertex_map != NULL )
    {
        for( i = 0 ; i < lengthb( &(m->vertex_map) ) ; i++ )
        {
            iptr = (int *) getb ( &(m->vertex_map), i ) ;
            while( !vertex_is_valid( m, *iptr ) )
            {
                iptr2 = (int *) getb ( &(m->vertex_map), *iptr );
                if( *iptr == *iptr2 ) {
                    MX_ASSERT( 0 ); /* we shouldn't be here.  Die if debug mode */
                    break; /* avoid an infinite loop */
                }
                else
                    iptr = iptr2;
            }
            MX_ASSERT( vertex_is_valid( m, *iptr ) );
            vertex_map[ i ] = *iptr;
        }
    }
}
#endif

static void assemble_config( const MxConfig *cfg_in, MxConfig *cfg )
{
    /* put in default values */
    cfg->placement_policy  = MX_PLACE_OPTIMAL_BB;
    cfg->weighting_policy  = MX_WEIGHT_AREA; 
    cfg->boundary_weight   = 1000.0;
    cfg->compactness_ratio = 0;
    cfg->meshing_penalty   = 0;
    cfg->will_join_only    = 0;
    cfg->ratio             = 0.5;
    cfg->max_degree        = 15;
    if( cfg_in != NULL )
    {
        if( cfg_in->placement_policy != MX_USE_DEFAULT )
            cfg->placement_policy = cfg_in->placement_policy;
        if( cfg_in->weighting_policy != MX_USE_DEFAULT )
            cfg->weighting_policy = cfg_in->weighting_policy;
        if( cfg_in->boundary_weight != MX_USE_DEFAULT )
            cfg->boundary_weight = cfg_in->boundary_weight;
        if( cfg_in->compactness_ratio != MX_USE_DEFAULT )
            cfg->compactness_ratio = cfg_in->compactness_ratio;
        if( cfg_in->meshing_penalty != MX_USE_DEFAULT )
            cfg->meshing_penalty = cfg_in->meshing_penalty;
        if( cfg_in->will_join_only != MX_USE_DEFAULT )
            cfg->meshing_penalty = cfg_in->meshing_penalty;
        if( cfg_in->ratio != MX_USE_DEFAULT )
            cfg->ratio = cfg_in->ratio;
        if( cfg_in->max_degree != MX_USE_DEFAULT )
            cfg->max_degree = cfg_in->max_degree;
    }

} /* end function assemble_config */


void HU_LOD_Execute(    
        int pcount, const float *plist, 
        int flist_length, const int *flist, 
        MxConfig *cfg_in, 
        int *new_pcount, float *new_points, 
        int *new_flist_length, int *new_flist, 
        int *point_mapping_list, int *face_mapping_list )
{
    MxModel m;
    MxQSlim slim;
    MxConfig cfg;
    int num_faces;
    int num_faces_target;
    int *face_map_intermediate;


    face_map_intermediate = (int *) MX_ALLOC ((long)(flist_length * sizeof( int )) );
    if( face_map_intermediate == NULL )
    MX_ERROR( ERR_INSUFFICIENT_MEMORY, "memory allocation failed" );

    mxmodel_init( &m, pcount, 100 );
    num_faces = shell_to_model( &m, pcount, plist, 
        flist_length, flist, face_map_intermediate );
    assemble_config( cfg_in, &cfg );
    mxqslim_init( &slim, &m, &cfg );

    num_faces = model_face_count( &m ); /* assumes that all faces are valid */
    num_faces_target = (int) (cfg.ratio * num_faces);
    qs_decimate( &slim, num_faces_target );
    model_to_shell( &m, new_pcount, new_points, new_flist_length, 
        new_flist, point_mapping_list, pcount );

    /* clean up our memory */
    mxqslim_cleanup( &slim );
    mxmodel_cleanup( &m );

    MX_FREE( face_map_intermediate, (long)(flist_length * sizeof( int )) );

    /* Prevent VC from complaining about error C4100 */
    face_mapping_list;

} /* end function HU_LOD_Execute */


/* 
 * helper
 */
static void recursive_chain_execute ( 
            MxShellChain *root, 
            MxConfig *cfg,
            MxQSlim *slim,
            int num_faces_target,
            int depth )
{
    int validfaces;
    int validvertices;

    qs_decimate( slim, num_faces_target );
    validfaces = model_valid_face_count( slim->m );
    root->sh.faces = (int *) MX_ALLOC ( (long)(4 * validfaces * sizeof( int )) );
    if( root->sh.faces == NULL )
        MX_ERROR( ERR_INSUFFICIENT_MEMORY, "memory allocation failed" );


    if( cfg->placement_policy == MX_PLACE_ENDPOINTS )
    {
    root->sh.points = NULL;
    model_to_shell_flist_only( 
        slim->m,
        &(root->sh.pcount),
        &(root->sh.flen),
        root->sh.faces
    );
    root->pointmap = NULL; /* point map is identity */
    }
    else
    {
    validvertices = model_valid_vertex_count( slim->m );
    root->sh.points = (float *) 
        MX_ALLOC ( (long)(3 * validvertices * sizeof( float )) );
    if( root->sh.faces == NULL )
        MX_ERROR( ERR_INSUFFICIENT_MEMORY, "memory allocation failed" );
    model_to_shell(
        slim->m, 
        &(root->sh.pcount),
        root->sh.points,
        &(root->sh.flen),
        root->sh.faces,
        NULL, 0             /* no vertex maps needed */
    );
    root->pointmap = NULL; /* no support for pointmaps yet */
    }
    if( depth > 0 )
    {
    root->next = (MxShellChain *) MX_ALLOC ( sizeof (MxShellChain) );
    if( root->next == NULL )
        MX_ERROR( ERR_INSUFFICIENT_MEMORY, "memory allocation failed" );
    root->next->next = NULL;
    recursive_chain_execute( root->next, cfg, slim, 
        (int) (cfg->ratio * num_faces_target), 
        depth - 1 
    );
    }

} /* end function recursive_chain_execute */
            
local void store_a_triangle (
    void *      info,
    int         convex_triangulation,
    int         v1,
    int         v2,
    int         v3)
{
    MxModel *   m = (MxModel *)info;

    add_face (m, v1, v2, v3);

    /* Prevent VC from complaining about warning C4100 */
    convex_triangulation;
}

MxShellChain *HU_LOD_Chain_Execute(
        int point_count, const float *points, int flistlen, int *flist,
                float ratio, int depth )
{
    MxModel m;
    MxQSlim slim;
    MxConfig cfg;
    int num_faces;
    int num_faces_target;
    MxShellChain *root;
    int *item, *global_end;
    int i;

    mxmodel_init( &m, point_count, 100 );

    
    for (i = 0; i < point_count; ++i)
        add_vertex (&m, points[3*i+0], points[3*i+1], points[3*i+2]);

    item = flist;
    global_end = flist + flistlen;
    do {
        int * local_end = item + 1 + *item;

        /* skip past sub-faces */
        while (local_end != global_end && *local_end < 0)
            local_end += 1 - *local_end;

        if (*item == 3)
            store_a_triangle ((void *)&m, 1,
                              item[1], item[2], item[3]);
        else
            HU_Triangulate_Face (points, null,
                                 item, local_end,
                                 store_a_triangle, (void *)&m);

        item = local_end;
    } until (item == global_end);

        
    assemble_config( null, &cfg );
    cfg.ratio = ratio;
    mxqslim_init( &slim, &m, &cfg );

    root = (MxShellChain *) MX_ALLOC ( sizeof (MxShellChain) );
    if( root == NULL )
        MX_ERROR( ERR_INSUFFICIENT_MEMORY, "memory allocation failed" );
    root->next = NULL;

    num_faces = model_face_count( slim.m ); /* assumes all faces are valid */
    num_faces_target = (int) (cfg.ratio * num_faces);
    recursive_chain_execute( root, &cfg, &slim, num_faces_target, depth );  

    /* clean up our memory */
    mxqslim_cleanup( &slim );
    mxmodel_cleanup( &m );

    return root;

} /* end function HU_LOD_Chain_Execute */
