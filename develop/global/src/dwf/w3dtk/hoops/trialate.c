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
 * $Id: //DWF/Working_Area/Willie.Zhu/w3dtk/hoops/trialate.c#1 $
 */

/*
#include "hoops.h"
#include "hi_proto.h"
*/
#include "lod.h"

#define HT_Integer32 int
#define HT_Boolean int
#define auto
#define HC_CDECL
#define ALLOC(p,t) p = (t *) MX_ALLOC (sizeof(t))
#define FREE(p,t) MX_FREE (p, sizeof (t))
#define FREE_ARRAY(p,c,t) MX_FREE(p, c * sizeof (t))
#ifndef __cplusplus
#define true 1
#define false 0
#endif
#define P(x) x
#define F(x) (float)(x)
#define CONST const
#define ABS(x) ((x) < 0 ? -(x) : (x))
#define forever while (1)

typedef struct proj_point {
    float       u;  /* abscissa */
    float       v;  /* ordinate */
    HT_Integer32        index;  /* index into original point list */
}   Proj_Point;

typedef struct vertex_stack {
    struct vertex_stack *next;      /* auto link */
    Proj_Point      *point;     /* a point */
}   Vertex_Stack;

typedef struct ht_edge_pair {
    struct ht_edge_pair *next, *prev;   /* doubly linked list links */
    Vertex_Stack    *left, *right;  /* a auto of vertices growing out from lowest point */
    Proj_Point      *lend, *rend;   /* points just beyond the top of the l & r stacks */
}   Edge_Pair;

/*
 * a span is essentially a wide vertex, covering three cases:
 * 1. duplicate points together form a single span
 *    in this case, left and right may have different values, but
 *    the points to which they refer must be equivalent
 * 2. all points connected on a horizontal form a single span
 *    here, left points to the leftmost point on the horizontal,
 *    right points to the rightmost, and mid contains all the points
 *    on the horizontal, from left to right, inclusive
 * 3. of course, a lone vertex forms a span
 *    in this case, left and right refer to the same point and this
 *    point is the sole member of the mid stack
 */
typedef struct ht_span {
    struct ht_span      *next;          /* auto link */
    Proj_Point      *left, *right;      /* extrema of the span (often same point) */
    Proj_Point      *next_left, *next_right;/* points just beyond the ends of this span */
    Vertex_Stack    *mid;           /* all points in span, l to r, right at head */
    int         type;           /* classification of this span,
                           basically peak, valley, or edge */
    HT_Boolean      original_order; /* is it backwards? */
} HT_Span;

typedef HT_Span *Span_Ptr;


/* swap a and b, using temporary variable t */
#define SWAP(a, b, t) (t = a, a = b, b = t)


/*
 * Quicksort to order spans on ordinate (v)
 * This algorithm might be improved by chosing a better pivot.
 * Currently the ordinate of the first span is chosen.
 * A better method might choose some average of several, say the
 * first, middle and last.
 */
local void sort_spans (
    auto    Span_Ptr        *first,
    auto    Span_Ptr        *last) {

    if (first < last) {
    register    float       pivot = (*last)->left->v;
    register    Span_Ptr    *i = first - 1;
    register    Span_Ptr    *j = last;
    register    int     ptype = (*last)->type;
    register    HT_Span     *temp;

    for (;;) {
        do ++i; while (((*i)->left->v < pivot ||
                (*i)->left->v == pivot && (*i)->type < ptype) &&
               i < j) ;
        do --j; while (((*j)->left->v > pivot ||
                (*j)->left->v == pivot && (*j)->type > ptype) &&
               i < j) ;
        if (i >= j) break;
        SWAP (*i, *j, temp);
    }

    SWAP (*i, *last, temp);

    sort_spans (first, i-1);
    sort_spans (i+1, last);
    }
}


local Edge_Pair *reduce_from_right (
    auto    Edge_Pair   *edge,
    auto    Proj_Point  *point,
    auto    void        (HC_CDECL * triangle_action)(
                    void    *P(info),
                    HT_Integer32    P(convex_triangulation),
                    HT_Integer32    P(v1),
                    HT_Integer32    P(v2),
                    HT_Integer32    P(v3)),
    auto    void        *action_info,
    auto    HT_Boolean  reorient) {
    register    Vertex_Stack    *vs,
                *nvs;

    vs = edge->right;
    if ((nvs = vs->next) != null) {
    do {
        /*
         * If the top two points on the side (vs and nvs) are not
         * on the same horizontal, then check the inverse slopes between this
         * reduction point and the top point and the top point and the next
         * point.  If the upper section does not overhang the lower, then we
         * cannot reduce any further.
         *           @ point
         *          /
         *         * vs
         *          \
         *        ...* nvs
         */
        if (vs->point->v != nvs->point->v) {
        if ((point->u - vs->point->u) * (vs->point->v - nvs->point->v) >=
            (vs->point->u - nvs->point->u) * (point->v - vs->point->v)) break;
        }

        /* If the auto's top two are on the same horizontal, then if the top one is left
         * of the next, then break because we have an inverted reflex and cannot reduce
         * any further
         *           @ point
         *          /
         *      vs *---* nvs
         *         .../
         */
        else if (nvs->point->u > vs->point->u) break;

        /* If the auto's top two are coincident, then advance, but do not draw the
         * triangle.
         *           @ point
         *          /
         *      vs * nvs
         *     .../
         */
        else if (nvs->point->u == vs->point->u) {
        FREE (vs, Vertex_Stack);
        vs = nvs;
        continue;
        }

        /* Otherwise, we can draw the triangle and advance
         *           @ point
         *           |
         *           |
         *           * vs
         *          /
         *      ...* nvs
         */
        if (reorient)
        (*triangle_action) (action_info, false, point->index,
                    nvs->point->index, vs->point->index);
        else
        (*triangle_action) (action_info, false, point->index,
                    vs->point->index, nvs->point->index);
        FREE (vs, Vertex_Stack);
        vs = nvs;
    } until ((nvs = nvs->next) == null);

    /* the right edge starts where we stopped reducing */
    edge->right = vs;
    }

    vs = edge->left;
    if ((nvs = vs->next) != null) {
    auto    Vertex_Stack    *first = vs;

    do {
        if (vs->point->u != nvs->point->u ||
        vs->point->v != nvs->point->v) {
        if (reorient)
            (*triangle_action) (action_info, false, point->index,
                    vs->point->index, nvs->point->index);
        else
            (*triangle_action) (action_info, false, point->index,
                    nvs->point->index, vs->point->index);
        }
        if (vs != first) FREE (vs, Vertex_Stack);
        vs = nvs;
    }
    until ((nvs = nvs->next) == null);
    FREE (vs, Vertex_Stack);
    edge->right->point = first->point;
    edge->left->next = null;
    }

    /*
     *  there's               @ point
     *  no edge lend->       /
     *                  *   /
     *                 / \ /
     *             ...*   *  <- edge
     */
    if (edge->lend == null) {
    auto    Edge_Pair   *victim = edge;

    /* remove edge pair "edge", hop left (if we can) */
    if (edge->prev != null) edge->prev->next = edge->next;
    if (edge->next != null) edge->next->prev = edge->prev;

    if (edge->prev != null)
        edge = edge->prev;
    else
        edge = edge->next;

    FREE (victim->left, Vertex_Stack);
    FREE (victim->right, Vertex_Stack);
    FREE (victim, Edge_Pair);
    }

    return  edge;
}


local Edge_Pair *reduce_from_left (
    auto    Edge_Pair   *edge,
    auto    Proj_Point  *point,
    auto    void        (HC_CDECL * triangle_action)(
                    void    *P(info),
                    HT_Integer32    P(convex_triangulation),
                    HT_Integer32    P(v1),
                    HT_Integer32    P(v2),
                    HT_Integer32    P(v3)),
    auto    void        *action_info,
    auto    HT_Boolean  reorient) {
    register    Vertex_Stack    *vs,
                *nvs;

    vs = edge->left;
    if ((nvs = vs->next) != null) {
    do {
        /*
         * If the top two points on the side (vs and nvs) are not on the
         * same horizontal, then check the inverse slopes between this reduction point
         * and the top point and the top point and the next point.  If the upper
         * section does not overhang the lower, then we cannot reduce any further.
         *           @ point
         *            \
         *          vs *
         *            /
         *       nvs *...
         */
        if (vs->point->v != nvs->point->v) {
        if ((point->u - vs->point->u) * (vs->point->v - nvs->point->v) <=
            (vs->point->u - nvs->point->u) * (point->v - vs->point->v)) break;
        }

        /* If the auto's top two are on the same horizontal, then if the top one is right
         * of the next, then break because we have an inverted reflex and cannot reduce
         * any further
         *           @ point
         *            \
         *     nvs *---* vs
         *          \...
         */
        else if (nvs->point->u < vs->point->u) break;

        /* If the auto's top two are coincident, then advance, but do not draw the
         * triangle.
         *           @ point
         *            \
         *          vs * nvs
         *              \...
         */
        else if (nvs->point->u == vs->point->u) {
        FREE (vs, Vertex_Stack);
        vs = nvs;
        continue;
        }

        /* Otherwise, we can draw the triangle and advance
         *           @ point
         *           |
         *           |
         *        vs *
         *            \
         *         nvs *...
         */
        if (reorient)
        (*triangle_action) (action_info, false, point->index,
                    vs->point->index, nvs->point->index);
        else
        (*triangle_action) (action_info, false, point->index,
                    nvs->point->index, vs->point->index);
        FREE (vs, Vertex_Stack);
        vs = nvs;
    } until ((nvs = nvs->next) == null);

    /* the left edge starts where we stopped reducing */
    edge->left = vs;
    }

    vs = edge->right;
    if ((nvs = vs->next) != null) {
    auto    Vertex_Stack    *first = vs;

    do {
        if (vs->point->u != nvs->point->u ||
        vs->point->v != nvs->point->v) {
        if (reorient)
            (*triangle_action) (action_info, false, point->index,
                    nvs->point->index, vs->point->index);
        else
            (*triangle_action) (action_info, false, point->index,
                    vs->point->index, nvs->point->index);
        }
        if (vs != first) FREE (vs, Vertex_Stack);
        vs = nvs;
    }
    until ((nvs = nvs->next) == null);
    FREE (vs, Vertex_Stack);
    edge->left->point = first->point;
    edge->right->next = null;
    }

    /*
     *      point @         there's
     *             \     <- no edge rend
     *              \   *
     *               \ / \
     *         edge-> *   *...
     */
    if (edge->rend == null) {
    auto    Edge_Pair   *victim = edge;

    /* remove edge pair "edge", hop right (if we can) */
    if (edge->prev != null) edge->prev->next = edge->next;
    if (edge->next != null) edge->next->prev = edge->prev;

    edge = edge->next;

    FREE (victim->left, Vertex_Stack);
    FREE (victim->right, Vertex_Stack);
    FREE (victim, Edge_Pair);
    }

    return  edge;
}



local Vertex_Stack *reduce_stack (
    auto    Vertex_Stack    *ivs,
    auto    Proj_Point  *point,
    auto    void        (HC_CDECL * triangle_action)(
                    void    *P(info),
                    HT_Integer32    P(convex_triangulation),
                    HT_Integer32    P(v1),
                    HT_Integer32    P(v2),
                    HT_Integer32    P(v3)),
    auto    void        *action_info,
    auto    HT_Boolean  reorient) {
    register    Vertex_Stack    *vs = ivs;
    register    HT_Integer32    pivot_index = point->index;

    while (vs->next != null) {
    register    Vertex_Stack *victim = vs;
    vs = victim->next;
    if (vs->point->u != victim->point->u ||
        vs->point->v != victim->point->v) {
        if (reorient)
        (*triangle_action) (action_info, false, pivot_index,
                    victim->point->index, vs->point->index);
        else
        (*triangle_action) (action_info, false, pivot_index,
                    vs->point->index, victim->point->index);
    }
    FREE (victim, Vertex_Stack);
    }

    return vs;
}


/*
 * Push a vertex "point" onto top of stack "side."
 * NOTE that side is passed by reference, because it gets modified
 */
local void push_vertex (
    auto    Proj_Point      *point,
    auto    Vertex_Stack    **side) {
    register    Vertex_Stack    *vs;

    ALLOC (vs, Vertex_Stack);
    vs->point = point;
    vs->next = *side;
    *side = vs;
}


local Proj_Point *record_point (
    auto    HT_Point    *points,
    register    int     index,
    auto    int     perp_axis,
    auto    Vertex_Stack    **record) {
    register    Proj_Point  *pt;

    ALLOC (pt, Proj_Point);
    pt->index = index;

#   define X_AXIS 0
#   define Y_AXIS 1
#   define Z_AXIS 2
    switch (perp_axis) {
      case X_AXIS: {
    pt->u = points[index].y;
    pt->v = points[index].z;
      }   break;
      case Y_AXIS: {
    pt->u = points[index].z;
    pt->v = points[index].x;
      }   break;
      case Z_AXIS: {
    pt->u = points[index].x;
    pt->v = points[index].y;
      }   break;
    };

    push_vertex (pt, record);
    return pt;
}



local int determine_perp_axis (
    auto    HT_Vector   *normal,
    auto    HT_Point    *points,
    auto    int     *face_list) {

    /*
     * determine the axis perpendicular to the projection plane.
     * it's the axis most closely aligned to the polygon normal.
     * note that we're only interested in the normal reflected
     * into the first quadrant - it's easier to test there.
     */
    if (normal != null &&
    (normal->x != F(0) || normal->y != F(0) || normal->z != F(0))) {
    register    float   ax = ABS (normal->x);
    register    float   ay = ABS (normal->y);
    register    float   az = ABS (normal->z);
    if (ax > ay)  {
        if (az > ax) return Z_AXIS;
        else return X_AXIS;
    }
    else {
        if (az > ay) return Z_AXIS;
        else return Y_AXIS;
    }
    }
    else {
    /* No normal given - use the bounding box as a good
     * first-approximation indicator.
     */
    register    HT_Point    *point;
    register    int     *item = face_list + 1;
    register    int     *end = item + *face_list;
    auto        float       minx, miny, minz,
                    maxx, maxy, maxz;

    /* Find a bounding box of the main face */
    point = points + *item++;
    minx = maxx = point->x;
    miny = maxy = point->y;
    minz = maxz = point->z;
    until (item == end) {
        point = points + *item++;
        if (minx > point->x) minx = point->x;
        else if (maxx < point->x) maxx = point->x;
        if (miny > point->y) miny = point->y;
        else if (maxy < point->y) maxy = point->y;
        if (minz > point->z) minz = point->z;
        else if (maxz < point->z) maxz = point->z;
    }

    /* convert to ranges */
    maxx -= minx;
    maxy -= miny;
    maxz -= minz;

    /* test for the minimum range */
    if (maxx < maxy)  {
        if (maxz < maxx) return Z_AXIS;
        else return X_AXIS;
    }
    else {
        if (maxz < maxy) return Z_AXIS;
        else return Y_AXIS;
    }
    }
}



local HT_Boolean triangulate_hardily (
    auto    HT_Point    *points,
    auto    HT_Vector   *normal,
    auto    int     *face_list,
    auto    int     *face_list_end,
    auto    void        (HC_CDECL * triangle_action)(
                    void    *P(info),
                    HT_Integer32    P(convex_triangulation),
                    HT_Integer32    P(v1),
                    HT_Integer32    P(v2),
                    HT_Integer32    P(v3)),
    auto    void        *action_info)  {

    auto    Vertex_Stack    *projections = null;

    auto    HT_Span     *spans = null;
    register    HT_Span     *current_span;

    auto    int     span_count = 0;

    auto    Span_Ptr    *span_ptrs = null;
    auto    Span_Ptr    *current_span_ptr;
    auto    Span_Ptr    *end_span_ptrs;

    auto    Edge_Pair   *edges = null;
    register    Edge_Pair   *current_edge;
    auto    HT_Boolean  reorient;
    auto    HT_Boolean  broken = false;

    {
    register    int *index_ptr = face_list;
    auto        int perp_axis;

    perp_axis = determine_perp_axis (normal, points, face_list);

    until (index_ptr == face_list_end) {
        auto    HT_Span     *first_face_span = null;
        auto    HT_Span     *end_face_span;

        /*
         * identify spans on a face
         */
        {
        auto        int     *last_index = index_ptr + ABS (*index_ptr);
        register    Proj_Point  *ptptr;

        /* point to the first vertex on the current face */
        ++index_ptr;
        ptptr = record_point (points, *index_ptr, perp_axis, &projections);

        forever {
            /* begin a new span */
            ALLOC (current_span, HT_Span);
            current_span->next = spans;
            spans = current_span;
            ++span_count;

            if (first_face_span == null) first_face_span = current_span;

            /*
             * set left end of the span to current vertex
             */
            current_span->left = ptptr;

            /*
             * add points to the mid while they are degenerate (on the same
             * horizontal as the left side)
             */
            current_span->mid = null;
            forever {
            push_vertex (ptptr, &current_span->mid);
            if (++index_ptr > last_index) break;
            ptptr = record_point (points, *index_ptr, perp_axis, &projections);
            if (ptptr->v != current_span->left->v) break;
            }
            current_span->right = current_span->mid->point;

            if (index_ptr > last_index) break;
        }
        }

        /* watch for the case where all the points are on a single span */
        if (current_span == first_face_span) {
        /* we can just ignore this span, it adds nothing to the shape */
        spans = spans->next;
        until (current_span->mid == null) {
            register    Vertex_Stack *  victim = current_span->mid;
            current_span->mid = victim->next;
            FREE (victim, Vertex_Stack);
        }
        FREE (current_span, HT_Span);
        --span_count;
        continue;
        }

        /*
         * join first and last spans on this face if
         * they are on the same horizontal, so that
         *
         *     current_span   first_face_span
         *    *------------**---------------*
         *    l            rL               R
         *    mmmmmmmmmmmmmmMMMMMMMMMMMMMMMMM
         *
         *  becomes
         *
         *           first_face_span
         *    *--------------------------*
         *    L                          R
         *    MMMMMMMMMMMMMMMMMMMMMMMMMMMM
         */
        if (current_span->right->v == first_face_span->left->v) {
        /*
         * leftmost mid (current_span->mid) absorbs 2 points rightward to make
         *
         *     current_span   first_face_span
         *    *-------------**--------------*
         *    l             rL              R
         *    mmmmmmmmmmmmmmmmMMMMMMMMMMMMMMM
         */
        push_vertex (current_span->right, &current_span->mid);
        push_vertex (first_face_span->left, &current_span->mid);

        /*
         * rightmost mid (first_face_span->mid) appends
         * leftmost mid (current_span->mid) to make
         *
         *     current_span   first_face_span
         *    *-------------**---------------*
         *    l             rL               R
         *    MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
         */
        {
            register        Vertex_Stack    *vs;

            vs = first_face_span->mid;
            until (vs->next == null) vs = vs->next;
            vs->next = current_span->mid;
        }

        /*
         * now get rid of the current_span, by extending the
         * first_face_span to the left and freeing the current_span
         *
         *           first_face_span
         *    *-------------**---------------*
         *    L                              R
         *    MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
         */
        first_face_span->left = current_span->left;
        spans = spans->next;
        FREE (current_span, HT_Span);
        current_span = spans;
        --span_count;
        }

        end_face_span = first_face_span->next;

        /* point the next_left and next_right pointers correctly */
        current_span = spans;
        do {
        register    HT_Span *next_span;

        if (current_span == null) {
            /* Self intersecting polygons again - bail out */
            /*
            HI_Warning (HEC_POLYGON, HES_SELF_INTERSECTING_FACE,
            "Encountered a badly self-intersecting or disjoint face");
            */
            broken = true;
            goto cleanup;
        }
        /* the next span in the list may wrap around */
        if ((next_span = current_span->next) == end_face_span)
            next_span = spans;

        /*
         * we hook things up in the following order because
         * the mid grows from left to right; therefore as we
         * move forward through the linked list (which is impossible
         * because we have no prev pointers, only next pointers)
         * the right side of the current would be closest to the
         * left side of the next; however, we are moving back down the
         * auto, so the right side of the next is the closest to the
         * left of this
         *
         * for example, something similar to
         *          a     b     c
         *          *-----*-----*
         *          scan --->
         * would become
         *      .     then    ab .   then, finally    bc ab .
         *      ^             ^                       ^
         *
         * one can see that the left of the top links with the right of
         * of the next (the b's).  Thus the current->next_left is next->right
         * and next->next_right is current->left.
         */

        if (next_span == null) {
            /* We have real trouble - self interesecting - bail out */
            /*
                HI_Warning (HEC_POLYGON, HES_SELF_INTERSECTING_FACE,
            "Encountered a badly self-intersecting or disjoint face");
            */
            broken = true;
            goto cleanup;
        }

        /* call next span's right the "next left" of this span */
        current_span->next_left = next_span->right;

        /* call this span's left the "next right" of the next span */
        next_span->next_right = current_span->left;
        } until ((current_span = current_span->next) == end_face_span);

        /* classify spans */
        current_span = spans;
        do {
#       define LEFT_IS_RISING   0x02
#       define RIGHT_IS_RISING  0x01
#       define VALLEY       0x03    /* both are rising */
#       define PEAK     0x00    /* neither is rising */
        current_span->type = 0;
        if (current_span->next_left->v  > current_span->left->v)
            current_span->type |= LEFT_IS_RISING;
        if (current_span->next_right->v > current_span->right->v)
            current_span->type |= RIGHT_IS_RISING;
        } until ((current_span = current_span->next) == end_face_span);

        /*
         * flip the spans around so that the left is really left
         * and the right is really right, if it isn't that way already.
         */
        current_span = spans;
        do {
        current_span->original_order = true;    /* assume */

        if (current_span->left->u == current_span->right->u) {
            switch (current_span->type) {
              case VALLEY:
            if ((current_span->next_left->u - current_span->left->u) *
                (current_span->next_right->v - current_span->right->v) >
                (current_span->next_right->u - current_span->right->u) *
                (current_span->next_left->v - current_span->left->v) )
                current_span->original_order = false;
            break;
              case PEAK:
            if ((current_span->next_left->u - current_span->left->u) *
                (current_span->next_right->v - current_span->right->v) <
                (current_span->next_right->u - current_span->right->u) *
                (current_span->next_left->v - current_span->left->v) )
                current_span->original_order = false;
            break;
              case LEFT_IS_RISING:
              case RIGHT_IS_RISING:
            if (current_span->next_left->v > current_span->next_right->v)
                current_span->original_order = false;
            break;
            }
        }
        else if (current_span->left->u > current_span->right->u)
            current_span->original_order = false;

        if (!current_span->original_order) {
            /* swap lefts and rights */
            {
            register    Proj_Point  *pt;

            SWAP (current_span->right, current_span->left, pt);
            SWAP (current_span->next_right, current_span->next_left, pt);
            }
            /* reverse the mid auto */
            {
            register    Vertex_Stack    *new_top = null;
            register    Vertex_Stack    *victim;

            while ((victim = current_span->mid) != null) {
                current_span->mid = victim->next;
                victim->next = new_top;
                new_top = victim;
            }
            current_span->mid = new_top;
            }
        }
        } until ((current_span = current_span->next) == end_face_span);
    }
    }

    /*
     * set up an array of pointer to spans, so we can sort those, rather
     * than the structures
     */
    ALLOC_ARRAY (span_ptrs, span_count, Span_Ptr);
    current_span_ptr = span_ptrs;
    current_span = spans;
    do *current_span_ptr++ = current_span;
    until ((current_span = current_span->next) == null);

    /* use recursive quicksort to order spans by increasing v */
    sort_spans (span_ptrs, span_ptrs + span_count - 1);


#ifdef JEFFS_DEBUG
    /* for debugging, label the spans with numbers */
    {
    register    Span_Ptr    *spptr = span_ptrs;
    register    int     ii = span_count;
    until (ii-- == 0) {
        auto    char    buf[80];
        sprintf (buf, "%d", span_count - ii - 1);
        HC_Insert_Text (((*spptr)->left->u + (*spptr)->right->u)/2.0,
                (*spptr)->left->v,
                0.0,
                buf);
        sprintf (buf, "0x%04x", (HT_Integer32)(*spptr)->left & 0xFFFF);
        HC_Insert_Text ((*spptr)->left->u,
                (*spptr)->left->v - 0.05,
                0.0,
                buf);
        sprintf (buf, "0x%04x", (HT_Integer32)(*spptr)->right & 0xFFFF);
        HC_Insert_Text ((*spptr)->right->u,
                (*spptr)->right->v - 0.05,
                0.0,
                buf);
        spptr++;
    }
    }
#endif

    reorient = (*span_ptrs)->original_order;

    /* the main loop */
    end_span_ptrs = span_ptrs + span_count;
    current_span_ptr = span_ptrs - 1;
    until (++current_span_ptr == end_span_ptrs) {

    current_span = *current_span_ptr;

    if (!broken) switch (current_span->type) {

        case VALLEY: {
        register    Edge_Pair   *new_edge;

#ifdef JEFFS_DEBUG
        printf ("%d is a VALLEY\n", current_span_ptr - span_ptrs);
        HC_Pause ();
#endif
        /*
         * create a new edge pair
         */
        ALLOC (new_edge, Edge_Pair);
        /* the left auto gets a single point */
        new_edge->left = new_edge->right = null;
        push_vertex (current_span->left, &new_edge->left);
        /* the mid points become the right auto */
        {
            register    Vertex_Stack    *bottom = current_span->mid;
            while (bottom->next != null) bottom = bottom->next;
            bottom->next = new_edge->right;
            new_edge->right = current_span->mid;
            current_span->mid = null;
        }
        /* the ends point appropriately */
        new_edge->lend = current_span->next_left;
        new_edge->rend = current_span->next_right;

        /*
         * insert this new pair into the list of pairs
         */
        if (edges == null)  {
            /* the list is null -- this pair becomes the list */
            edges = new_edge;
            new_edge->next = new_edge->prev = null;
        }

        else for (current_edge = edges; current_edge != null;
              current_edge = current_edge->next)  {
            /* need to insert the pair somewhere */
            auto    Proj_Point  *leftA = current_edge->left->point,
                        *leftB = current_edge->lend,
                        *rightA = current_edge->right->point,
                        *rightB = current_edge->rend;

            if  (leftB != null &&
             ((current_span->right->v != leftA->v  &&
               (current_span->right->u - leftA->u) * (leftB->v - leftA->v)  <
               (leftB->u - leftA->u) * (current_span->right->v - leftA->v) ) ||
              (current_span->right->v == leftA->v  &&
               (current_span->right->u < leftA->u ||
                current_span->right->u == leftA->u &&
                (current_span->next_right->u - current_span->right->u) *
                 (leftB->v - leftA->v) <
                (leftB->u - leftA->u) *
                 (current_span->next_right->v - current_span->right->v))))) {

            /* insert to left of this edge pair */
            if  (edges == current_edge)  edges = new_edge;
            new_edge->next = current_edge;
            new_edge->prev = current_edge->prev;
            if (current_edge->prev != null) current_edge->prev->next = new_edge;
            current_edge->prev = new_edge;

            break;  /* exit search */
            }

            else if (rightB != null &&
                 ((current_span->left->v != rightA->v &&
                   (current_span->left->u - rightA->u) * (rightB->v - rightA->v) <
                   (rightB->u - rightA->u) * (current_span->left->v - rightA->v))||
                  (current_span->left->v == rightA->v &&
                   current_span->left->u < rightA->u ||
                   current_span->left->u == rightA->u &&
                   (current_span->next_right->u - current_span->right->u) *
                (rightB->v - rightA->v) <
                   (rightB->u - rightA->u) *
                (current_span->next_right->v - current_span->right->v))))  {

            /*
             *      ...         ...
             *         \       /
             *          @-----@
             *
             *             *
             *            / \  <---current_edge
             *     ...#--#   *--*...
             */
            if (current_edge->lend == null) {
                /* splitting over a peak, merge */

                /*
                 * first, reduce the valley by the peak
                 * we're going to destroy the new_edge->right in the process,
                 * but that's ok, since we have to get rid of new_edge
                 * in a moment anyway.  We've got a pointer to the top of
                 * new_edge->right saved in current_span->right so that we
                 * can use it later
                 */
                new_edge->right =
                reduce_stack (new_edge->right,
                          current_edge->left->point,
                          triangle_action, action_info,
                          reorient);

                /* now reduce the current edge by the valley's right end */
                reduce_from_left (current_edge,
                          current_span->right,
                          triangle_action, action_info,
                          reorient);
                push_vertex (current_span->right,
                     &current_edge->left);
                current_edge->lend = new_edge->rend;

                /* hop left */
                current_edge = current_edge->prev;

                /* now reduce the prev edge by the valley's left end */
                reduce_from_right (current_edge,
                           current_span->left,
                           triangle_action, action_info,
                           reorient);
                push_vertex (current_span->left,
                     &current_edge->right);
                current_edge->rend = new_edge->lend;

                /* don't really need a new pair */
                FREE (new_edge->left, Vertex_Stack);
                FREE (new_edge->right, Vertex_Stack);
                FREE (new_edge, Edge_Pair);
            }

            /*
             *      ...         ...
             *         \       /
             *          @-----@
             *
             *     ...*         *...
             *         \       /
             *          *-----*
             */
            else {
                /* split this edge pair */

                if (current_edge->right->next != null) {
                reduce_from_right (current_edge,
                           current_span->right,
                           triangle_action, action_info,
                           reorient);

                if (current_span->right->u != current_span->left->u) {
                    /* reduce the new flat valley */
                    new_edge->right =
                    reduce_stack (new_edge->right,
                              current_edge->right->point,
                              triangle_action,
                              action_info, reorient);

                    /* and maybe now further reduce the right edge
                     * by the left side if the valley */
                    reduce_from_right (current_edge,
                               current_span->left,
                               triangle_action,
                               action_info, reorient);

                }

                new_edge->left->point =
                    new_edge->right->point = rightA;
                }
                else if (current_edge->left->next != null) {
                reduce_from_left (current_edge,
                          current_span->left,
                          triangle_action, action_info,
                          reorient);

                if (current_span->right->u != current_span->left->u) {
                    /* reduce the new flat valley */
                    new_edge->right =
                    reduce_stack (new_edge->right,
                              current_edge->left->point,
                              triangle_action,
                              action_info, reorient);

                    /* and maybe now further reduce the left edge
                     * by the left side if the valley */
                    reduce_from_left (current_edge,
                              current_span->right,
                              triangle_action,
                              action_info, reorient);
                }

                {
                    register    Vertex_Stack    *vs;

                    vs = new_edge->right;
                    new_edge->right = current_edge->right;
                    current_edge->right = vs;
                    current_edge->right->point = leftA;

                    vs = new_edge->left;
                    new_edge->left = current_edge->left;
                    current_edge->left = vs;
                    current_edge->left->point = leftA;
                }
                }
                else {
                if (current_span->right->u != current_span->left->u)
                    /* reduce the new flat valley */
                    new_edge->right = reduce_stack (new_edge->right,
                                    leftA, triangle_action,
                                    action_info, reorient);

                new_edge->left->point = rightA;
                new_edge->right->point = rightA;
                current_edge->right->point = leftA;
                }

                push_vertex (current_span->right, &new_edge->left);
                push_vertex (current_span->left, &current_edge->right);

                {
                register    Proj_Point *pt = new_edge->lend;
                new_edge->lend = new_edge->rend;
                new_edge->rend = current_edge->rend;
                current_edge->rend = pt;
                }

                /* place in edges list after current_edge */
                new_edge->next = current_edge->next;
                new_edge->prev = current_edge;
                if (current_edge->next != null)
                current_edge->next->prev = new_edge;
                current_edge->next = new_edge;
            }
            break;  /* exit search */
            }

            else if  (current_edge->next == null)  {
            /* insert at right of all other edge pairs */
            new_edge->next = null;
            new_edge->prev = current_edge;
            current_edge->next = new_edge;

            break;  /* exit search */
            }
        }
        }   break;

        case PEAK: {
#ifdef JEFFS_DEBUG
        printf ("%d is a PEAK\n", current_span_ptr - span_ptrs);
        HC_Pause ();
#endif
        for (current_edge = edges; current_edge != null;
             current_edge = current_edge->next) {
            /*
             *   @---...
             *    \
             *     *        external (closing) peak
             *      \
             *       *...
             */
            if (current_span->left  == current_edge->lend) {
            auto    Edge_Pair   *old_tmp = current_edge;
            auto    Proj_Point  *reducer;

            until (current_span->right == current_edge->rend) {
                register    Edge_Pair   *old_edge;
                old_edge = current_edge;
                current_edge = reduce_from_left (current_edge,
                                 current_span->left,
                                 triangle_action, action_info, reorient);
                if (old_edge == current_edge) {
                /*
                 * making no progress - uh oh
                 * you know what that means
                 */
#ifdef DEBUG
                HI_Warning (HEC_POLYGON, HES_SELF_INTERSECTING_FACE,
                    "Encountered a badly self-intersecting or disjoint face");
#endif
                broken = true;
                break;
                }
                else if (current_edge == null) {
                /*
                 * rats.
                 */
#ifdef DEBUG
                HI_Warning (HEC_POLYGON, HES_SELF_INTERSECTING_FACE,
                    "Encountered a badly self-intersecting or disjoint face");
#endif
                broken = true;
                break;
                }
            }

            /* in case the reduce threw away the first edge pair */
            if (edges == old_tmp) edges = current_edge;

            /*
             * since we can't break both the until above and the
             * containing switch, and we need the fixup check
             * immediately above anyway.
             */
            if (broken) break;

            reducer = current_span->right;
            if (current_span->right->u != current_span->left->u) {
                if (current_edge->right->next != null) {
                current_span->mid = reduce_stack (current_span->mid,
                                  current_edge->right->point,
                                  triangle_action,
                                  action_info, reorient);
                reducer = current_span->left;
                }
                else
                current_span->mid = reduce_stack (current_span->mid,
                                  current_edge->left->point,
                                  triangle_action,
                                  action_info, reorient);
            }

            old_tmp = current_edge;
            current_edge = reduce_from_right (current_edge, reducer,
                              triangle_action, action_info, reorient);

            if (old_tmp == current_edge) {
                reduce_from_left (current_edge, current_span->left,
                          triangle_action, action_info, reorient);

                /*
                 * remove this edge from the list of edges
                 * because it's been completely triangulated
                 */
                FREE (current_edge->left, Vertex_Stack);
                FREE (current_edge->right, Vertex_Stack);
                if (current_edge->next != null)
                current_edge->next->prev = current_edge->prev;
                if (current_edge->prev != null)
                current_edge->prev->next = current_edge->next;
                if (edges == current_edge) edges = current_edge->next;
                FREE (current_edge, Edge_Pair);
            }

            /* in case the reduce threw away the first edge pair */
            else if (edges == old_tmp) edges = current_edge;

            break;  /* exit search */
            }

            /*
             *         @---...
             *        /
             *       *        internal peak
             *      /
             *  ...*
             */
            else if (current_span->left  == current_edge->rend) {

            current_edge = reduce_from_right (current_edge,
                              current_span->left,
                              triangle_action, action_info, reorient);
            push_vertex (current_span->left, &current_edge->right);
            current_edge->rend = null; /* indicate that the right side is a peak */

            current_edge = current_edge->next;

            /* sanity check - shouldn't be null for behaved figures */
            if (current_edge == null) {
#ifdef DEBUG
                HI_Warning (HEC_POLYGON, HES_SELF_INTERSECTING_FACE,
                    "Encountered a badly self-intersecting or disjoint face");
#endif
                broken = true;
                break;
            }

            current_edge = reduce_from_left (current_edge,
                             current_span->right,
                             triangle_action, action_info, reorient);
            while (current_span->mid != null) {
                register    Vertex_Stack    *victim = current_span->mid;
                current_span->mid = victim->next;
                victim->next = current_edge->left;
                current_edge->left = victim;
            }
            current_edge->lend = null; /* indicate that the left side is a peak */

            break;  /* exit search */
            }
        }
        }   break;

        case LEFT_IS_RISING:
        case RIGHT_IS_RISING: {
#ifdef JEFFS_DEBUG
        printf ("%d is an EDGE\n", current_span_ptr - span_ptrs);
        HC_Pause ();
#endif
        for (current_edge = edges; current_edge != null;
             current_edge = current_edge->next)  {

            /*
             *       @------...
             *      /
             *     *
             *      \
             *       *...
             */
            if (current_span->left == current_edge->lend) {
            auto    Edge_Pair   *old_tmp = current_edge;

            current_edge = reduce_from_left (current_edge,
                             current_span->left,
                             triangle_action, action_info, reorient);

            /* if flat span, reduce against current_edge->left */
            if (current_span->right->u != current_span->left->u) {
                current_span->mid = reduce_stack (current_span->mid,
                                  current_edge->left->point,
                                  triangle_action, action_info, reorient);
                current_edge = reduce_from_left (current_edge,
                                 current_span->right,
                                 triangle_action, action_info, reorient);
            }

            /* in case the reduce threw away the first edge pair */
            if (edges == old_tmp) edges = current_edge;

            push_vertex (current_span->right, &current_edge->left);
            current_edge->lend = current_span->next_right;

            break;  /* exit search */
            }

            /*
             *     ...------@
             *             /
             *            *
             *             \
             *              *...
             */
            else if (current_span->right == current_edge->lend) {
            auto    Edge_Pair   *old_tmp = current_edge;

            current_edge = reduce_from_left (current_edge, current_span->right,
                             triangle_action, action_info, reorient);

            /* in case the reduce threw away the first edge pair */
            if (edges == old_tmp) edges = current_edge;

            while (current_span->mid != null) {
                register    Vertex_Stack *victim = current_span->mid;
                current_span->mid = victim->next;
                victim->next = current_edge->left;
                current_edge->left = victim;
            }

            current_edge->lend = current_span->next_left;

            break;  /* exit search */
            }

            /*
             *       @------...
             *        \
             *         *
             *        /
             *    ...*
             */
            else if (current_span->left == current_edge->rend) {
            current_edge = reduce_from_right (current_edge, current_span->left,
                              triangle_action, action_info, reorient);

            {
                register    Vertex_Stack    *bottom = current_span->mid;
                while (bottom->next != null) bottom = bottom->next;
                bottom->next = current_edge->right;
                current_edge->right = current_span->mid;
                current_span->mid = null;
            }

            current_edge->rend = current_span->next_right;

            break;  /* exit search */
            }

            /*
             *     ...------@
             *               \
             *                *
             *               /
             *           ...*
             */
            else if (current_span->right == current_edge->rend) {
            current_edge = reduce_from_right (current_edge, current_span->right,
                              triangle_action, action_info, reorient);

            /* if flat span, reduce against current_edge->right */
            if (current_span->right->u != current_span->left->u) {
                current_span->mid = reduce_stack (current_span->mid,
                                  current_edge->right->point,
                                  triangle_action, action_info, reorient);
                current_edge = reduce_from_right (current_edge,
                                  current_span->left,
                                  triangle_action, action_info, reorient);
            }

            push_vertex (current_span->left, &current_edge->right);
            current_edge->rend = current_span->next_left;

            break;  /* exit search */
            }
        }
        }   break;
    }

    /* free the current span's structures, if any haven't been absorbed */
    until (current_span->mid == null) {
        register    Vertex_Stack *victim = current_span->mid;
        current_span->mid = victim->next;
        FREE (victim, Vertex_Stack);
    }
    }

    /* in case we left due to brain-damage, clean up */
cleanup:
    until (edges == null) {
    register    Edge_Pair   *victim = edges;
    register    Vertex_Stack    *tmpv;

    edges = victim->next;

    until ((tmpv = victim->left) == null) {
        victim->left = tmpv->next;
        FREE (tmpv, Vertex_Stack);
    }
    until ((tmpv = victim->right) == null) {
        victim->right = tmpv->next;
        FREE (tmpv, Vertex_Stack);
    }
    FREE (victim, Edge_Pair);
    }

    /* free the array of span pointers and auto of spans */
    FREE_ARRAY (span_ptrs, span_count, Span_Ptr);
    until (spans == null) {
    current_span = spans;
    spans = spans->next;
    FREE (current_span, HT_Span);
    }

    /* free the projection points */
    until (projections == null) {
    register    Vertex_Stack *victim = projections;
        projections = victim->next;
    FREE (victim->point, Proj_Point);
    FREE (victim, Vertex_Stack);
    }

    return !broken;
}



local void triangulate_easily (
    register    int     *face_list,
    auto    void        (HC_CDECL * itriangle_action)(
                    void    *P(info),
                    HT_Integer32    P(convex_triangulation),
                    HT_Integer32    P(v1),
                    HT_Integer32    P(v2),
                    HT_Integer32    P(v3)),
    auto    void        *action_info)  {
    register    HT_Integer32    pt1, pt2, pt3;
    register    int     offset_to_far_end,
                count = *face_list++;
    register    void        (HC_CDECL * triangle_action)(
                    void    *P(info),
                    HT_Integer32    P(convex_triangulation),
                    HT_Integer32    P(v1),
                    HT_Integer32    P(v2),
                    HT_Integer32    P(v3)) = itriangle_action;

    pt1 = *face_list++;
    pt2 = *face_list;
    offset_to_far_end = count - 2;

    until (offset_to_far_end < 2) {
    /* choose from closing half-gon */
    pt3 = face_list[offset_to_far_end];
    (*triangle_action) (action_info, true, pt1, pt2, pt3);

    /* shift */
    pt1 = pt2;
    pt2 = pt3;

    /* advance */
    ++face_list;
    offset_to_far_end -= 2;

    /* choose from opening half-gon */
    pt3 = *face_list;
    (*triangle_action) (action_info, true, pt1, pt3, pt2);

    /* shift */
    pt1 = pt2;
    pt2 = pt3;
    }

    if (offset_to_far_end != 0) {
    pt3 = *++face_list;
    (*triangle_action) (action_info, true, pt1, pt2, pt3);
    }
}



 HT_Boolean HC_CDECL HU_Triangulate_Face (
    auto    float const *points,
    auto    float const *normal,
    auto    int const   *face_list,
    auto    int const   *face_list_end,
    auto    void        (HC_CDECL * triangle_action)
                     (void *P(info),
                           HT_Integer32 P(convex_triangulation),
                           HT_Integer32 P(v1),
                           HT_Integer32 P(v2),
                           HT_Integer32 P(v3)),
    auto    void        *action_info) {

    if (*face_list < 3) return false; /* say what? */

    if (face_list + *face_list + 1 == face_list_end) {
    /* There are no holes in this face. Look for an easy way. */
    if (*face_list == 3)
        (*triangle_action) (action_info, false,
                face_list[1], face_list[2], face_list[3]);
    else
        return triangulate_hardily ((HT_Point  *)points, (HT_Vector  *)normal,
                 (int *)face_list, (int *)face_list_end,
                 triangle_action, action_info);
    }
    else {
    /* there *are* one or more holes - must do it the hard way */
    triangulate_hardily ((HT_Point  *)points, (HT_Vector  *)normal,
                 (int *)face_list, (int *)face_list_end,
                 triangle_action, action_info);
    }
    return true;
}
