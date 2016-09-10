/*
 * Copyright (c) 1998 by Tech Soft America, LLC.
 * The information contained herein is confidential and proprietary to
 * Tech Soft America, LLC., and considered a trade secret as defined under
 * civil and criminal statutes.  Tech Soft America shall pursue its civil
 * and criminal remedies in the event of unauthorized use or misappropriation
 * of its trade secrets.  Use of this information by anyone other than
 * authorized employees of Tech Soft America, LLC. is granted only under a
 * written non-disclosure agreement, expressly prescribing the scope and
 * manner of such use.
 *
 * $Id: //DWF/Working_Area/Willie.Zhu/w3dtk/hoops/shellopt.cpp#1 $
 */

#include "lod.h"

/*
 * This defines an index that is reserved to mean "unreferenced"
 * It must be -1 because it must be less than any possible
 * referenced vertex.
 */
#define UNTOUCHED (-1)

struct Vertex {
    struct Vertex *     neighbor;
    float               proximity_value;
    int                 original_index;
    int                 final_index;
    int                 utility;
};

#define HASH(a, b, c)   ((a) + (b) + (c))

#define MEDIANOF3(a,b,c) (((a)>(b))?(((b)>(c))?(b):(((a)>(c))?(c):(a))):(((a)>(c))?(a):((b)>(c))?(c):(b)))
#define MEDIANOF3INDEX(a,b,c) (((a)>(b))?(((b)>(c))?(1):(((a)>(c))?(2):(0))):(((a)>(c))?(0):((b)>(c))?(2):(1)))

#define SWAPVALS(a,b,temp) { temp=a; a=b; b=temp; }

#ifdef AIX_SYSTEM
#define false 0
#define true 1
#endif

local void swap_vertices  (Vertex * v1, Vertex * v2) {
    int     temp_int;
    float   temp_float;

    SWAPVALS (v1->original_index,  v2->original_index,  temp_int);
    SWAPVALS (v1->utility,         v2->utility,         temp_int);
    SWAPVALS (v1->proximity_value, v2->proximity_value, temp_float);
}


/*
 * This sort doesn't need to swap the neighbor nor the
 * final_index fields since they should be the same for
 * all entries when this is called right after initializing
 * the vertex array
 */
local void sort_vertices (
    Vertex  *ileft,
    Vertex  *iright) {

    if (iright - ileft == 1) {
        if (ileft->proximity_value > iright->proximity_value)
            swap_vertices (ileft, iright);
    }
    else if (ileft < iright) {
        Vertex          *left, *right;
        Vertex          *candidates[3];
        float           pivot;
        int             m3i;

        candidates[0] = left = ileft;                   // left
        candidates[1] = ileft + (iright - ileft) / 2;   // middle
        candidates[2] = right = iright;                 // right

        // find the best pivot and move it to the far right
        m3i = MEDIANOF3INDEX(
            candidates[0]->proximity_value, 
            candidates[1]->proximity_value,
            candidates[2]->proximity_value 
        );
        pivot = candidates[m3i]->proximity_value;
        swap_vertices (candidates[m3i], right);

        for (;;) {
            left--;
            while ((++left)->proximity_value  <= pivot) 
                if (left >= right) 
                   goto partition_done;
            swap_vertices (left, right);

            right++;
            while ((--right)->proximity_value >= pivot)
                if (left >= right) 
                   goto partition_done;
            swap_vertices (left, right);
        }
        partition_done:

        sort_vertices (ileft, left - 1);
        sort_vertices (left + 1,  iright);
    }

}



typedef int (* HT_Collapse_Check_Function) (HT_Point const * points,
                                            HT_Vector const * normals,
                                            int parameter_width, float const * parameters,
                                            int a, int b,
                                            double tolerance_squared,
                                            double normal_distance_squared,
                                            double parameter_distance_squared);

local int points_are_collapsible (
    HT_Point const *    pts,
    HT_Vector const *   norms,
    int                 parameter_width,
    float const *       parameters,
    int                 a,
    int                 b,
    double              tolerance_squared,
    double              normal_distance_squared,
    double              parameter_distance_squared) {
    register    float               delta;
    register    float               dist;    // accumulated squared distance

    delta = pts[a].x - pts[b].x;    dist  = delta * delta;
    delta = pts[a].y - pts[b].y;    dist += delta * delta;
    delta = pts[a].z - pts[b].z;    dist += delta * delta;

    if (dist > tolerance_squared) 
        return false;

    if (norms != NULL) {
        delta = norms[a].x - norms[b].x;    dist  = delta * delta;
        delta = norms[a].y - norms[b].y;    dist += delta * delta;
        delta = norms[a].z - norms[b].z;    dist += delta * delta;

        if (dist > normal_distance_squared) 
            return false;
    }

    if (parameters != NULL) {
        int             pa = a * parameter_width;
        int             pb = b * parameter_width;
        register    int             i;

        for (i=0; i<parameter_width; i++) {
            delta = parameters[pa+i] - parameters[pb+1];
            dist = delta * delta;

            if (dist > parameter_distance_squared) 
                return false;
        }
    }

    return true;
}


void HU_OptimizeShell (
    int                     in_point_count,
    HT_Point const *        in_points,
    HT_Vector const *       in_normals,         // null if not interested
    int                     in_parameter_width,
    float const *           in_parameters,      // null if not interested
    int                     in_face_list_length,
    int const *             in_face_list,

    HT_Collapse_Check_Function  collapsible,
    double                  tolerance,
    double                  normal_tolerance,
    double                  parameter_tolerance,
    int                     orphan_elimination,

    int *                   out_point_count,
    HT_Point *              out_points,
    int *                   out_face_list_length,
    int *                   out_face_list,
    int *                   vertex_mapping,     // null if not interested
    int *                   face_mapping) {     // null if not interested
    Vertex                  *vertices;
    int *                   tmp_vertex_mapping = null;
    float                   hood;
    double                  tolerance_squared;  // double because blindly passed
    double                  normal_distance_squared;
    double                  parameter_distance_squared;

    if (collapsible == null)
        collapsible = points_are_collapsible;

    if (in_parameter_width <= 0)        // if none (or silly value)
        in_parameters = null;           // we can skip the extra work to process 'none'

    // Get a vertex mapping array if the user didn't give us one and clear it out
    if (vertex_mapping == null)
        vertex_mapping = tmp_vertex_mapping = new int [in_point_count];
    {
        register        int     *ip = vertex_mapping;
        register        int     *ie = ip + in_point_count;
        do *ip = UNTOUCHED;
        until (++ip == ie);
    }


    // Allocate and initialize a structure for each referenced vertex.
    // This will be used in the algorithm below.
    vertices = new Vertex [in_point_count];
    {
        register        Vertex          *vp = vertices;
        register        HT_Point const  *pp = in_points;
        register        HT_Point const  *pe = pp + in_point_count;
        register        int             index = 0;

        do {
            vp->original_index  = index++;
            vp->final_index     = UNTOUCHED;
            vp->neighbor        = null;
            vp->proximity_value = HASH (pp->x, pp->y, pp->z);
            vp->utility         = 0;
            ++vp;
        } until (++pp == pe);
    }

    // Figure the utilities of the points if we're interested
    if (orphan_elimination && in_face_list_length > 0) {
        register        int const       *listp = in_face_list;
        register        int const       *liste = listp + in_face_list_length;
        register        int const       *local_end;

        do {
            if (*listp < 0)
                local_end = listp - *listp + 1;
            else
                local_end = listp + *listp + 1;

            until (++listp == local_end)
                vertices[*listp].utility++;
        } until (listp == liste);
    }


    // Sort the points by order of decreasing proximity value.
    // That is, vertices[n].proximity_value <= vertices[n+1].proximity_value.
    sort_vertices (vertices, vertices + in_point_count - 1);

    // Determine the neighboorhood limit, "hood" for points
    // to be considered collapsible, and the spatial tolerance for
    // such a collapse to occur.
    if (tolerance < 0.0f) {
        register        int const       *listp = in_face_list;
        register        int const       *liste = listp + in_face_list_length;
        register        int const       *local_end;
        register        int             prev_index;
        register        int             index;
        register        float           dx, dy, dz, len;
        float           min_len = 1.0e30f;

        tolerance = -0.01f * tolerance; // change to percentage

        // Find the distance between the closest two vertices on any one face in the shell.
        // This will define the min_len for use in calculating an fru and tolerances.
        if (in_face_list_length > 0) do {
            if (*listp < 0)
                local_end = listp - *listp + 1;
            else
                local_end = listp + *listp + 1;

            prev_index = local_end[-1];

            until (++listp == local_end) {
                index = *listp;

                dx = in_points[index].x - in_points[prev_index].x;
                dy = in_points[index].y - in_points[prev_index].y;
                dz = in_points[index].z - in_points[prev_index].z;
                len = dx*dx + dy*dy + dz*dz;
                if (len < min_len && len != 0.0f) min_len = len;

                prev_index = index;
            }
        } until (listp == liste);

        tolerance_squared = min_len * tolerance;
        {
            register    float   tol = (float)sqrt (tolerance_squared);
            hood = HASH (tol, tol, tol);
        }
    }
    else {
        hood = (float)HASH (tolerance, tolerance, tolerance);
        tolerance_squared = tolerance * tolerance;
    }

    // Use the law of cosines (C^2 = A^2 + B^2 - 2ABcos(theta)) 
    // to calculate the normal_distance_squared.
    // Here we are making the assumption that the normals are normalized, so A = B = 1.0
    // Since we are interested in comparison, not absolutes, euclidian distance between
    // vectors is sufficient.
    normal_distance_squared = 2.0 - 2.0 * cos (normal_tolerance);

    parameter_distance_squared = parameter_tolerance * parameter_tolerance;

    // Find unique points
    {
        register        Vertex          *candidate;
        register        Vertex          *reference;
        register        Vertex          *vp = vertices;
        Vertex          *ve = vp + in_point_count;
        int             unique_point_count = 0;

        do {
            if (orphan_elimination && vp->utility == 0) {
                vp->final_index = UNTOUCHED;
            }
            else if (vp->final_index != UNTOUCHED) {
                // we've already seen this point
            }
            else {
                // This point is unique, so far
                vp->final_index =
                    vertex_mapping[vp->original_index] =
                        unique_point_count++;

                reference = vp;

                // Find neighbors further down the list, within the 'hood.
                candidate = vp + 1;
                while (candidate != ve &&
                       candidate->proximity_value -
                         reference->proximity_value <= hood) {
                    if ((*collapsible) (in_points, in_normals, in_parameter_width, in_parameters,
                                        reference->original_index, candidate->original_index,
                                        tolerance_squared, normal_distance_squared, parameter_distance_squared)) {
                        register        Vertex  *neighbor;

                        // Be sure we're a neighbor of all neighbors
                        if ((neighbor = reference->neighbor) != null) do {
                            if (!(*collapsible) (in_points, in_normals, in_parameter_width, in_parameters,
                                                 candidate->original_index, neighbor->original_index,
                                                 tolerance_squared, normal_distance_squared, parameter_distance_squared))
                                break;
                        } until ((neighbor = neighbor->neighbor) == null);

                        if (neighbor == null) {
                            candidate->final_index =
                                vertex_mapping[candidate->original_index] =
                                    reference->final_index;

                            candidate->neighbor = reference;
                            reference = candidate;
                        }
                    }

                    ++candidate;
                }
            }
        } until (++vp == ve);
    }

    // Repack the points
    // We know that the final indices of the sorted vertices will be monotically, but not uniformly, non-decreasing.
    // By this I mean that:   vertex[ii + 1].final_index - vertex[ii].final_index = [0, 1]
    // We mean to record the jumps, which indicate a new, unique, non-collapsed vertex.
    // This is done by noticing an increase in magnitude of the final_index when linearly traversing the vertices array.
    if (out_points != null) {
        register        int             index;
        register        HT_Point        *pp = out_points;
        register        Vertex          *vp;
        Vertex          *ve;

        index = UNTOUCHED;
        vp = vertices;
        ve = vp + in_point_count;

        do if (index < vp->final_index) {
            index = vp->final_index;
            pp->x = in_points[vp->original_index].x;
            pp->y = in_points[vp->original_index].y;
            pp->z = in_points[vp->original_index].z;
            ++pp;
        } until (++vp == ve);

        *out_point_count = (int)(pp - out_points);
    }

    // Repack the face list, excluding duplicate vertices and blowing away degenerate faces
    if (out_face_list_length != null) {
        if (in_face_list_length > 0) {
            register        int const       *listp;
            register        int const       *liste;
            register        int             *out_listp;
            int             unique_faces = 0;

            listp = in_face_list;
            liste = listp + in_face_list_length;
            out_listp = out_face_list;

            do {
                register    int const       *local_end;
                register    int             prev_index;
                int             *face_length_ptr;
                int             face_length;
                int             is_hole;

                if (*listp < 0) {
                    is_hole = 1;
                    local_end = listp - *listp + 1;
                }
                else {
                    is_hole = 0;
                    local_end = listp + *listp + 1;
                }

                prev_index = vertex_mapping[local_end[-1]];
                face_length_ptr = out_listp++;

                until (++listp == local_end) {
                    register        int     index = vertex_mapping[*listp];
                    if (index != prev_index)
                        *out_listp++ = prev_index = index;
                }

                face_length = (int)(out_listp - face_length_ptr - 1);
                if (face_length < 3) {
                    if (face_mapping != null) *face_mapping++ = UNTOUCHED;
                    out_listp -= face_length + 1;
                }
                else {
                    if (is_hole)
                        *face_length_ptr = -face_length;
                    else
                        *face_length_ptr = face_length;
                    if (face_mapping != null) *face_mapping++ = unique_faces++;
                }
            } until (listp == liste);

            if (out_face_list_length != null)
                *out_face_list_length = (int)(out_listp - out_face_list);
        }
        else
            *out_face_list_length = 0;
    }

    // Clean up
    delete [] vertices;
    delete [] tmp_vertex_mapping;
}


void HU_GenerateShellLOD (int pcount, const HT_Point * points,     int flistlen, const int * flist,
                          double ratio, int algorithm,
                          int * pcount_out, HT_Point * points_out, int * flistlen_out, int * flist_out) {
    MxShellChain *node;

    if (algorithm == LOD_Algorithm_Fast) {
        node = HU_Compute_LOD_Fast (pcount, (float *)points, flistlen, flist, (float)ratio, 1);
    } 
    else {
        HT_Point * new_points;
        int * new_face_list;
        int new_point_count, new_face_list_length;

        new_points = new HT_Point [pcount];
        new_face_list = new int [flistlen];

        // assumes that desired tolerance (locations and normals) for collapse is 0 */
        HU_OptimizeShell (pcount, points, null, 0, null,
                          flistlen, flist,
                          null,  0.0, 0.0, 0.0,  false,
                          &new_point_count,      new_points,
                          &new_face_list_length, new_face_list,
                          null, null);


        node = HU_LOD_Chain_Execute (new_point_count, (float *)new_points,new_face_list_length, new_face_list,
                                     (float)ratio, 1);
    }

    *pcount_out = node->sh.pcount;
    *flistlen_out = node->sh.flen;
    memcpy (points_out, node->sh.points, node->sh.pcount * sizeof (HT_Point));
    memcpy (flist_out,  node->sh.faces,  node->sh.flen * sizeof (int));

    delete [] node->sh.points;
    delete [] node->sh.faces;
    delete node;
}
