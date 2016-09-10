/************************************************************************

  MxMath.c  

    standard vector and matrix operations

  Copyright (C) 1998 Michael Garland.  See "COPYING.txt" for details.
  
  $Id: //DWF/Working_Area/Willie.Zhu/w3dtk/hoops/mxmath.c#1 $

 ************************************************************************/

/*
#include "hoops.h"
*/
#include "lod.h"

#ifndef DISABLE_LOD_GENERATION

const double FOUR_ROOT3 = 6.928203230275509f;


/*
 *  Some operations specific to 3x3 matrices
 */

#if 0
static double det( const Mat3 *m1 )
{
    Vec3 temp;
    mxv_cross( temp.elt, m1->row[1].elt, m1->row[2].elt, 3 );
    return mxv_dot( m1->row[0].elt, temp.elt, 3 );
}

static void transpose( Mat3 *m_out, const Mat3 *m1 )
{
    col33( &(m_out->row[0]), m1, 0 );
    col33( &(m_out->row[1]), m1, 1 );
    col33( &(m_out->row[2]), m1, 2 );
}
#endif

void vecmul33( Vec3 *v_out, const Mat3 *m1, const Vec3 *v1)
{
    v_out->elt[0] = mxv_dot( m1->row[0].elt, v1->elt, 3 ); 
    v_out->elt[1] = mxv_dot( m1->row[1].elt, v1->elt, 3 ); 
    v_out->elt[2] = mxv_dot( m1->row[2].elt, v1->elt, 3 );
}

void col33( Vec3 *v_out, const Mat3 *m1, int i )
{ 
    v_out->elt[0] = m1->row[0].elt[i];
    v_out->elt[1] = m1->row[1].elt[i];
    v_out->elt[2] = m1->row[2].elt[i]; 

}

/*
 * invert a 3x3 matrix. returns the determinant.
 */
double invert33( Mat3 *m_out, const Mat3 *m1 )
{
    Mat3 temp, A;
    double d;
        
    /* compute the adjoint: adjoint( &A, m1 ); */
    mxv_cross( A.row[0].elt, m1->row[1].elt, m1->row[2].elt, 3 );
    mxv_cross( A.row[1].elt, m1->row[2].elt, m1->row[0].elt, 3 );
    mxv_cross( A.row[2].elt, m1->row[0].elt, m1->row[1].elt, 3 );

    d = mxv_dot( A.row[0].elt, m1->row[0].elt, 3 );

    if( d==0.0 )
        return 0.0;
    /* compute the transpose: transpose( &temp, &A ); */
    col33( &(temp.row[0]), &A, 0 );
    col33( &(temp.row[1]), &A, 1 );
    col33( &(temp.row[2]), &A, 2 );

    mxv_div( m_out->row[0].elt, temp.row[0].elt, d, 3 );
    mxv_div( m_out->row[1].elt, temp.row[1].elt, d, 3 );
    mxv_div( m_out->row[2].elt, temp.row[2].elt, d, 3 );
    return d;
}


/*
 *  Some operations specific to 4x4 matrices
 */


static void col44( Vec4 *result, const Mat4 *m, int i)
{ 
    result->elt[0] = m->row[0].elt[i];
    result->elt[1] = m->row[1].elt[i];
    result->elt[2] = m->row[2].elt[i];
    result->elt[3] = m->row[3].elt[i];
}


static void copy44(Mat4 *m1, const Mat4 *m2)
{
    mxv_setv( m1->row[0].elt, m2->row[0].elt, 4 ); 
    mxv_setv( m1->row[1].elt, m2->row[1].elt, 4 );
    mxv_setv( m1->row[2].elt, m2->row[2].elt, 4 ); 
    mxv_setv( m1->row[3].elt, m2->row[3].elt, 4 );

}


void vecmul44( Vec4 *out, const Mat4 *m1, const Vec4 *v) 
{
    out->elt[0] = mxv_dot( m1->row[0].elt, v->elt, 4 ); 
    out->elt[1] = mxv_dot( m1->row[1].elt, v->elt, 4 ); 
    out->elt[2] = mxv_dot( m1->row[2].elt, v->elt, 4 ); 
    out->elt[3] = mxv_dot( m1->row[3].elt, v->elt, 4 );

}




static void transpose44(Mat4 *out, const Mat4 *in)
{
    Mat4 temp;
    col44( &(temp.row[0]), in, 0 ); 
    col44( &(temp.row[1]), in, 1 ); 
    col44( &(temp.row[2]), in, 2 ); 
    col44( &(temp.row[3]), in, 3 ); 

    /* necessary to separate from above, in case out==in */
    mxv_setv( out->row[0].elt, temp.row[0].elt, 4 );
    mxv_setv( out->row[1].elt, temp.row[1].elt, 4 );
    mxv_setv( out->row[2].elt, temp.row[2].elt, 4 );
    mxv_setv( out->row[3].elt, temp.row[3].elt, 4 );

}

void matmul44(Mat4 *out, const Mat4 *m1, const Mat4 *m2)
{
    Mat4 temp, temp2;
    int i,j;

    transpose44( &temp, m1 );
    copy44( &temp2, m2 );

    for(i = 0 ; i < 4 ; i++ )
        for(j=0;j<4;j++)
            out->row[i].elt[j] = mxv_dot(temp2.row[i].elt, temp.row[j].elt, 4);

}

/* Code adapted from VecLib4d.c in Graphics Gems V*/
static void cross4(Vec4 *result, const Vec4 *a, const Vec4 *b, const Vec4 *c)
{
    double d1, d2, d3, d4, d5, d6;
    d1 = (b->elt[Z] * c->elt[W]) - (b->elt[W] * c->elt[Z]);
    d2 = (b->elt[Y] * c->elt[W]) - (b->elt[W] * c->elt[Y]);
    d3 = (b->elt[Y] * c->elt[Z]) - (b->elt[Z] * c->elt[Y]);
    d4 = (b->elt[X] * c->elt[W]) - (b->elt[W] * c->elt[X]);
    d5 = (b->elt[X] * c->elt[Z]) - (b->elt[Z] * c->elt[X]);
    d6 = (b->elt[X] * c->elt[Y]) - (b->elt[Y] * c->elt[X]);

    result->elt[X] = - a->elt[Y] * d1 + a->elt[Z] * d2 - a->elt[W] * d3;
    result->elt[Y] =   a->elt[X] * d1 - a->elt[Z] * d4 + a->elt[W] * d5;
    result->elt[Z] = - a->elt[X] * d2 + a->elt[Y] * d4 - a->elt[W] * d6;
    result->elt[W] =   a->elt[X] * d3 - a->elt[Y] * d5 + a->elt[Z] * d6;

}

void adjoint44(Mat4 *out, const Mat4 *in)
{
    Mat4 A;
    Vec4 tempv;

    copy44( &A, in );
    mxv_neg( tempv.elt, A.row[0].elt, 4 );

    cross4( &(out->row[0]), &(A.row[1]), &(A.row[2]), &(A.row[3]));
    cross4( &(out->row[1]), &(tempv),    &(A.row[2]), &(A.row[3]));
    cross4( &(out->row[2]), &(A.row[0]), &(A.row[1]), &(A.row[3]));
    cross4( &(out->row[3]), &(tempv),    &(A.row[1]), &(A.row[2]));
}


#if 0
/*
 * Matrix inversion code for 4x4 matrices.
 * Originally ripped off and degeneralized from Paul's matrix library
 * for the view synthesis (Chen) software.
 *
 * Returns determinant of a, and b=a inverse.
 * If matrix is singular, returns 0 and leaves trash in b.
 *
 * Uses Gaussian elimination with partial pivoting.
 */

#define SWAP(a, b, t)   {   \
    t = a;                  \
    a = b;                  \
    b = t;                  \
}                           

static double invert(Mat4 *result, const Mat4 *input) 
{
    Mat4 temp;
    int i, j, k;
    double max, t, det, pivot;

    copy44( &temp, input );

    /*---------- forward elimination ----------*/

    for (i=0; i<4; i++)                 /* put identity matrix into output */
        for (j=0; j<4; j++)
            result->row[i].elt[j] = (double)(i==j);

    det = 1.0;
    for (i=0; i<4; i++) {               /* eliminate in column i, below diag */
        max = -1.;
        for (k=i; k<4; k++)             /* find pivot for column i */
            if( fabs( temp.row[k].elt[i] ) > max) {
                max = (double) fabs( temp.row[k].elt[i] );
                j = k;
            }
        if (max<=0.) return 0.;         /* if no nonzero pivot, PUNT */
        if (j!=i) {                     /* swap rows i and j */
            for (k=i; k<4; k++)
                SWAP(temp.row[i].elt[k], temp.row[j].elt[k], t);
            for (k=0; k<4; k++)
                SWAP( result->row[i].elt[k], result->row[j].elt[k], t );
            det = -det;
        }
        pivot = temp.row[i].elt[i];
        det *= pivot;
        for (k=i+1; k<4; k++)           /* only do elems to right of pivot */
            temp.row[i].elt[k] /= pivot;
        for (k=0; k<4; k++)
            result->row[i].elt[k] /= pivot;
        /* we know that A(i, i) will be set to 1, so don't bother to do it */

        for (j=i+1; j<4; j++) {         /* eliminate in rows below i */
            t = temp.row[j].elt[i];       /* we're gonna zero this guy */
            for (k=i+1; k<4; k++)       /* subtract scaled row i from row j */
                temp.row[j].elt[k] -= temp.row[i].elt[k] * t;   /* (ignore k<=i, we know they're 0) */
            for (k=0; k<4; k++)
                result->row[j].elt[k] -= result->row[i].elt[k] * t;
        }
    }

    /*---------- backward elimination ----------*/

    for (i=4-1; i>0; i--) {             /* eliminate in column i, above diag */
        for (j=0; j<i; j++) {           /* eliminate in rows above i */
            t = temp.row[j].elt[i];       /* we're gonna zero this guy */
            for (k=0; k<4; k++)         /* subtract scaled row i from row j */
                result->row[j].elt[k] -= result->row[i].elt[k] * t;
        }
    }

    return det;

} /* end function invert */
#endif


/*
 * some basic triangle operations
 */

void triangle_raw_normal( Vec3 *out, const Vec3 *v1, const Vec3 *v2, const Vec3 *v3)
{
    Vec3 a, b;
    mxv_sub( a.elt, v2->elt, v1->elt, 3 );
    mxv_sub( b.elt, v3->elt, v1->elt, 3 );
    mxv_cross( out->elt, a.elt, b.elt, 3 );
}

/* uses property that triangle area is cross product magnitude / 2 */
double triangle_area( const Vec3 *v1, const Vec3 *v2, const Vec3 *v3 )
{
    Vec3 temp;
    triangle_raw_normal( &temp, v1, v2, v3 );
    return 0.5f * mxv_len( temp.elt, 3 );
}


int triangle_normal( Vec3 *out, const Vec3 *v1, const Vec3 *v2, const Vec3 *v3 )
{
    double f;
    
    triangle_raw_normal( out, v1, v2, v3);
    /* now unitize */
    f = mxv_len( out->elt, 3 );

    if( MxFEQ( f, 0.0, 1e-12f ) )
        return 0;

    mxv_div( out->elt, out->elt, f, 3 );
    return 1;

} /* end function triangle_normal */


int triangle_plane( Vec4 *out, const Vec3 *v1, const Vec3 *v2, const Vec3 *v3 )
{
    Vec3 n;
    
    if( !triangle_normal( &n, v1, v2, v3 ) )
        return 0;

    out->elt[0] = n.elt[0];
    out->elt[1] = n.elt[1];
    out->elt[2] = n.elt[2];
    out->elt[3] = -( mxv_dot(n.elt, v1->elt, 3) );
    return 1;

} /* end function triangle_plane */


void triangle_raw_plane( Vec4 *out, const Vec3 *v1, const Vec3 *v2, const Vec3 *v3 )
{
    Vec3 n;

    triangle_raw_normal( &n, v1, v2, v3);
    out->elt[0] = n.elt[0];
    out->elt[1] = n.elt[1];
    out->elt[2] = n.elt[2];
    out->elt[3] = -( mxv_dot( n.elt, v1->elt, 3 ) );

} /* end function triangle_raw_plane */

#if 0
/* HELPER
 * calculates the maximum from 3 doubles
 */
static double max3f( double f1, double f2, double f3 )
{
    if( f1 >= f2 )
        if( f1 >= f3 )
            return f1;
        else {
            MX_ASSERT( f3 >= f2 );
            return f3;
        }
    else
        if( f2 >= f3 )
            return f2;
        else {
            MX_ASSERT( f3 >= f1 );
            return f3;
        }  
       
} /* end function max3f */
#endif

double triangle_compactness(const Vec3 *v1, const Vec3 *v2, const Vec3 *v3)
{

    Vec3 temp1, temp2, temp3;
    double l1, l2, l3;

    mxv_sub( temp1.elt, v2->elt, v1->elt, 3 );
    mxv_sub( temp2.elt, v3->elt, v2->elt, 3 );
    mxv_sub( temp3.elt, v1->elt, v3->elt, 3 );
    l1 = mxv_len2( temp1.elt, 3 );
    l2 = mxv_len2( temp2.elt, 3 );
    l3 = mxv_len2( temp3.elt, 3 );

    return FOUR_ROOT3 * triangle_area(v1, v2, v3) / (l1+l2+l3);

} /* end function triangle_compactness */


/* enumerate all of the corners of the box around two points */
void mx3d_box_corners( Vec3 v[], const Vec3 *min, const Vec3 *max )
{
    v[0].elt[0] = min->elt[0]; v[0].elt[1] = min->elt[1]; v[0].elt[2] = min->elt[2];
    v[1].elt[0] = min->elt[0]; v[1].elt[1] = max->elt[1]; v[1].elt[2] = min->elt[2];
    v[2].elt[0] = max->elt[0]; v[2].elt[1] = min->elt[1]; v[2].elt[2] = min->elt[2];
    v[3].elt[0] = max->elt[0]; v[3].elt[1] = max->elt[1]; v[3].elt[2] = min->elt[2];
    v[4].elt[0] = min->elt[0]; v[4].elt[1] = min->elt[1]; v[4].elt[2] = max->elt[2];
    v[5].elt[0] = min->elt[0]; v[5].elt[1] = max->elt[1]; v[5].elt[2] = max->elt[2];
    v[6].elt[0] = max->elt[0]; v[6].elt[1] = min->elt[1]; v[6].elt[2] = max->elt[2];
    v[7].elt[0] = max->elt[0]; v[7].elt[1] = max->elt[1]; v[7].elt[2] = max->elt[2];

} /* end function mx3d_box_corners */




/*
 *  Begin Vector Math equations
 */


void mxv_add( double *r, const double *u, const double *v, int dim )
{
    int i;
    for(i = 0 ; i < dim ; i++) 
        r[i] = u[i] + v[i];
} 


void mxv_sub( double *r, const double *u, const double *v, int dim )
{
    int i;
    for(i = 0 ; i < dim ; i++) 
        r[i] = u[i] - v[i];
} 


void mxv_mul( double *r, const double *u, const double d, int dim )
{
    int i;
    for(i = 0 ; i < dim ; i++) 
        r[i] = u[i] * d;
} 

void mxv_div( double *r, const double *u, const double d, int dim )
{
    int i;
    for(i = 0 ; i < dim ; i++) 
        r[i] = u[i] / d;
} 

void mxv_neg ( double *r, const double *u, int dim) 
{ 
    int i;
    for(i = 0 ; i < dim ; i++) 
        r[i] = -u[i];
}

void mxv_set ( double *r, const double d, int dim) 
{ 
    int i;
    for(i = 0 ; i < dim ; i++) 
        r[i] = d;
}

void mxv_setv ( double *r, const double *u, int dim)
{
    int i;
    for(i = 0 ; i < dim ; i++) 
        r[i] = u[i];
}

double mxv_dot(const double *u, const double *v, int dim)
{
    int i;
    double dot=0.0;  
    for(i = 0 ; i < dim ; i++) 
        dot += u[i]*v[i];  
    return dot;
}

void mxv_cross(double *r, const double *u, const double *v, int dim)
{
    if( dim == 3 )
    {
        r[0] =  (u[1] * v[2]) - (v[1] * u[2]); 
        r[1] = -(u[0] * v[2]) + (v[0] * u[2]);
        r[2] =  (u[0] * v[1]) - (v[0] * u[1]);  
    }
    else MX_ASSERT(0);
}


double mxv_len(const double *v, int dim) 
{ 
    return (double) sqrt(mxv_dot(v,v,dim)); 
}


double mxv_len2(const double *v, int dim) 
{ 
    return mxv_dot(v,v,dim); 
}


/* If l is non-zero, division succeeds and return 1, else return 0 
 * In other words, return 0 on failure */
int mxv_unitize(double *v, int dim)
{
    double len = mxv_len2(v, dim);
    if( MxFEQ( len, 0.0, 1e-12f ) )
        return 0;

    len = (double) sqrt (len);  
    mxv_div(v, v, len, dim); 

    return 1;

} /* end function mxv_unitize */

#endif
