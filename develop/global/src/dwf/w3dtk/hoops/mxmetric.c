/************************************************************************

  mxmetric.c

  3D Quadric Error Metrics

  Copyright (C) 1998 Michael Garland.  See "COPYING.txt" for details.
  
  $Id: //DWF/Working_Area/Willie.Zhu/w3dtk/hoops/mxmetric.c#1 $

 ************************************************************************/

/*
#include "hoops.h"
*/
#include "lod.h"

#ifndef DISABLE_LOD_GENERATION

void mxquadric3_init( MxQuadric3 *q, double a, double b, double c, double d, double area)
{
    q->a2 = a*a;  q->ab = a*b;  q->ac = a*c;  q->ad = a*d;
                  q->b2 = b*b;  q->bc = b*c;  q->bd = b*d;
                                q->c2 = c*c;  q->cd = c*d;
                                              q->d2 = d*d;

    q->r = area;

    MX_ASSERT(!( quad_evaluate( q, 0, 0, 0 ) < 0 ));
    MX_ASSERT(!( quad_evaluate( q, 0, 0, 0 ) < 0 ));
/*
    MX_ASSERT( ((*((int *)(&q->a2)) >> 23) & 0xFF) != 0xFF );
    MX_ASSERT( ((*((int *)(&q->a2)) >> 23) & 0xFF) != 0xFF );
    MX_ASSERT( !isNaN( q->a2 ) );
*/

} /* end function mxquadric3_init */


void mxquadric3_init_by_matrix( MxQuadric3 *q, const Mat4 *M, double area)
{
    q->a2 = M->row[0].elt[0];  
    q->ab = M->row[0].elt[1];  
    q->ac = M->row[0].elt[2];  
    q->ad = M->row[0].elt[3];

    q->b2 = M->row[1].elt[1];  
    q->bc = M->row[1].elt[2];  
    q->bd = M->row[1].elt[3];
                        
    q->c2 = M->row[2].elt[2];  
    q->cd = M->row[2].elt[3];
                              
    q->d2 = M->row[3].elt[3];

    q->r = area;

    /* MX_ASSERT( quad_evaluate( q, 0, 0, 0 ) >= 0 ); */
}



static void quad_vector( Vec3 *v_out, const MxQuadric3 *q ) { 
    v_out->elt[0] = q->ad;
    v_out->elt[1] = q->bd;
    v_out->elt[2] = q->cd;
}

double quad_offset( const MxQuadric3 *q ) { 
    return q->d2; 
}
double quad_area( const MxQuadric3 *q ) { 
    return q->r; 
}
void quad_clear( MxQuadric3 *q ) { 
    q->a2=q->ab=q->ac=q->ad=q->b2=q->bc=q->bd=q->c2=q->cd=q->d2=q->r=0.0; 
}



static void quad_tensor( const MxQuadric3 *q, Mat3 *m )
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
    /* garland: BUG: is it ok to leave the bottom part of the matrix untouched? */

}

static void quad_homogeneous( Mat4 *m, const MxQuadric3 *q )
{
    m->row[0].elt[0] = q->a2;
    m->row[0].elt[1] = q->ab;
    m->row[0].elt[2] = q->ac;
    m->row[0].elt[3] = q->ab;
    m->row[1].elt[0] = q->ab;
    m->row[1].elt[1] = q->b2;
    m->row[1].elt[2] = q->bc;
    m->row[1].elt[3] = q->bd;
    m->row[2].elt[0] = q->ac;
    m->row[2].elt[1] = q->bc;
    m->row[2].elt[2] = q->c2;
    m->row[2].elt[3] = q->cd;
    m->row[3].elt[0] = q->ad;
    m->row[3].elt[1] = q->bd;
    m->row[3].elt[2] = q->cd;
    m->row[3].elt[3] = q->d2;

}

#if 0
static void set_coefficients( MxQuadric3 *q, const double *v)
{
    q->a2 = v[0];  q->ab = v[1];  q->ac = v[2];  q->ad = v[3];
                   q->b2 = v[4];  q->bc = v[5];  q->bd = v[6];
                                  q->c2 = v[7];  q->cd = v[8];
                                                 q->d2 = v[9];

    /* MX_ASSERT( quad_evaluate( q, 0, 0, 0 ) >= 0 ); */
}
#endif

void quad_copy( MxQuadric3 *dest, const MxQuadric3 *src )
{
    dest->r = src->r;

    dest->a2 = src->a2;  dest->ab = src->ab;  dest->ac = src->ac;  dest->ad = src->ad;
                         dest->b2 = src->b2;  dest->bc = src->bc;  dest->bd = src->bd;
                                              dest->c2 = src->c2;  dest->cd = src->cd;
                                                                   dest->d2 = src->d2;

}

void quad_add( MxQuadric3 *q_out, const MxQuadric3 *q1, const MxQuadric3 *q2 )
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

    /* MX_ASSERT( quad_evaluate( q_out, 0, 0, 0 ) >= 0 ); */
}

void quad_sub( MxQuadric3 *q_out, const MxQuadric3 *q1, const MxQuadric3 *q2 )
{
    /* Accumulate area*/
    q_out->r  = q1->r - q2->r;

    /* Accumulate coefficients*/
    q_out->a2 = q1->a2 - q2->a2;
    q_out->ab = q1->ab - q2->ab;
    q_out->ac = q1->ac - q2->ac;  
    q_out->ad = q1->ad - q2->ad;

    q_out->b2 = q1->b2 - q2->b2;  
    q_out->bc = q1->bc - q2->bc;  
    q_out->bd = q1->bd - q2->bd;

    q_out->c2 = q1->c2 - q2->c2;  
    q_out->cd = q1->cd - q2->cd;

    q_out->d2 = q1->d2 - q2->d2;

    /* MX_ASSERT( quad_evaluate( q_out, 0, 0, 0 ) >= 0 ); */

}


void quad_scale( MxQuadric3 *q, double s)
{
    q->a2 *= s;  q->ab *= s;  q->ac *= s;  q->ad *= s;
                 q->b2 *= s;  q->bc *= s;  q->bd *= s;
                              q->c2 *= s;  q->cd *= s;
                                           q->d2 *= s;
    /* MX_ASSERT( quad_evaluate( q, 0, 0, 0 ) >= 0 ); */

}

void quad_transform( MxQuadric3 *q_out, const MxQuadric3 *q_in, const Mat4 *mat)
{
    Mat4 M, Pa, temp;

    quad_homogeneous( &M, q_in );
    adjoint44( &Pa, mat );

    /* Compute:  trans(Pa) * Q * Pa*/
    /* NOTE: Pa is symmetric since Q is symmetric*/
    matmul44( &temp, &Pa, &M );
    matmul44( &M, &temp, &Pa );

    /* garland: ??BUG: Should we be transforming the area??*/
    mxquadric3_init_by_matrix( q_out, &M, q_in->r );

    /* MX_ASSERT( quad_evaluate( q_out, 0, 0, 0 ) >= 0 ); */

}


double quad_evaluate( const MxQuadric3 *q, double x, double y, double z)
{
    /* Evaluate vAv + 2bv + c*/

    double return_val = 
           x*x*(q->a2) + 2*x*y*(q->ab) + 2*x*z*(q->ac) + 2*x*(q->ad)
                       + y*y*(q->b2)   + 2*y*z*(q->bc) + 2*y*(q->bd)
                                       + z*z*(q->c2)   + 2*z*(q->cd)
                                                       + (q->d2);
    /* MX_ASSERT( return_val >= 0 ); */
    return return_val;
}

double quad_evaluatev( const MxQuadric3 *q, const double *v )
{
    return quad_evaluate( q, v[X], v[Y], v[Z]);
}

MxBool optimizev( const MxQuadric3 *q, Vec3 *v)
{
    Mat3 Ainv, tensor;
    double det;
    Vec3 vtemp;

    quad_tensor( q, &tensor );
    det = invert33( &Ainv, &tensor );
    if( MxFEQ(det, 0.0, 1e-12f) )
        return 0;

    quad_vector( &vtemp, q );
    vecmul33( v, &Ainv, &vtemp );
    mxv_neg( v->elt, v->elt, 3 );


    /* MX_ASSERT( quad_evaluatev( q, v->elt ) >= 0 ); */
    return 1;
}

MxBool optimize3f( const MxQuadric3 *q, double *x, double *y, double *z)
{
    Vec3 v;
    MxBool success = optimizev( q, &v );

    if( success )
    {
        *x = (double)v.elt[X];
        *y = (double)v.elt[Y];
        *z = (double)v.elt[Z];
    }

    /* MX_ASSERT( quad_evaluatev( q, v.elt ) >= 0 ); */
    return success;
}

MxBool optimize2v( const MxQuadric3 *q, Vec3 *v, const Vec3 *v1, const Vec3 *v2)
{
    Vec3 Av2, Ad, d;
    Mat3 A;
    double denom;
    double a, f1, f2, f3, f4;
    Vec3 vtemp;
    Vec3 temp;

    mxv_sub( d.elt, v1->elt, v2->elt, 3 );
    quad_tensor( q, &A );
    vecmul33( &Av2, &A, v2 );
    vecmul33( &Ad, &A, &d );

    denom = 2*( mxv_dot( d.elt, Ad.elt, 3 ) );
    if( MxFEQ(denom, 0.0, 1e-12f) )
        return 0;

    quad_vector( &vtemp, q );
    f1 = mxv_dot( vtemp.elt, d.elt, 3 );
    f2 = mxv_dot( Av2.elt, d.elt, 3 );
    f3 = mxv_dot( v2->elt, Ad.elt, 3 );
    f4 = mxv_dot( Ad.elt, d.elt, 3 );
    a =  ( -2*f1 - f2 - f3 ) / ( 2*f4 );

    if( a<0.0 ) a=0.0; else if( a>1.0 ) a=1.0;

    mxv_mul( temp.elt, d.elt, a, 3 );
    mxv_add( v->elt, temp.elt, v2->elt, 3 );


    /* MX_ASSERT( quad_evaluatev( q, v->elt ) >= 0 ); */
    return 1;
}


MxBool optimize3v( const MxQuadric3 *q, Vec3 *v, const Vec3 *v1,
              const Vec3 *v2, const Vec3 *v3)
{
    Mat3 A;
    Vec3 d13, d23, B;
    Vec3 temp1, temp2, temp3;
    double d13_d23, v3_d13, v3_d23, d23Ad23, d13Ad13, denom;
    double a, b, f1, f2, f3, f4;
    Vec3 Ad13, Ad23, Av3;

    mxv_sub( d13.elt, v1->elt, v3->elt, 3 );
    mxv_sub( d23.elt, v2->elt, v3->elt, 3 );
    quad_tensor( q, &A );
    quad_vector( &B, q );

    vecmul33( &Ad13, &A, &d13 );
    vecmul33( &Ad23, &A, &d23 );
    vecmul33( &Av3, &A, v3 );

    d13_d23 = 
        (mxv_dot( d13.elt, Ad23.elt, 3 )) + 
        (mxv_dot( d23.elt, Ad13.elt, 3 ));
    v3_d13 = ( mxv_dot( d13.elt, Av3.elt, 3 )) + ( mxv_dot( v3->elt, Ad13.elt, 3 ));
    v3_d23 = ( mxv_dot( d23.elt, Av3.elt, 3 )) + ( mxv_dot( v3->elt, Ad23.elt, 3 ));

    d23Ad23 = mxv_dot( d23.elt, Ad23.elt, 3 );
    d13Ad13 = mxv_dot( d13.elt, Ad13.elt, 3 );

    denom = d13Ad13 * d23Ad23 - 2 * d13_d23;
    if( MxFEQ(denom, 0.0, 1e-12f) )
        return 0;

    f1 = mxv_dot( B.elt, d13.elt, 3 );
    f2 = mxv_dot( B.elt, d23.elt, 3 );
    f3 = mxv_dot( B.elt, d23.elt, 3 );
    f4 = mxv_dot( B.elt, d13.elt, 3 );
    a = ( d23Ad23*(2*f1 + v3_d13) -
          d13_d23*(2*f2 + v3_d23) ) / -denom;

    b = ( d13Ad13*(2*f3 + v3_d23) -
          d13_d23*(2*f4 + v3_d13) ) / -denom;

    if( a<0.0 ) a=0.0; else if( a>1.0 ) a=1.0;
    if( b<0.0 ) b=0.0; else if( b>1.0 ) b=1.0;

    /*his original code for the next 4 lines...*/
    /*v = a*d13 + b*d23 + v3;*/
    mxv_mul( temp1.elt, d13.elt, a, 3 );
    mxv_mul( temp2.elt, d23.elt, b, 3 );
    mxv_add( temp3.elt, temp1.elt, temp2.elt, 3 );
    mxv_add( v->elt, temp3.elt, v3->elt, 3 );


    /* MX_ASSERT( quad_evaluatev( q, v->elt ) >= 0 ); */
    return 1;
}

#endif
