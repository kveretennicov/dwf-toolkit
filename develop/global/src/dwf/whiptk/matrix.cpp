//  Copyright (c) 1996-2006 by Autodesk, Inc.
//
//  By using this code, you are agreeing to the terms and conditions of
//  the License Agreement included in the documentation for this code.
//
//  AUTODESK MAKES NO WARRANTIES, EXPRESS OR IMPLIED, AS TO THE CORRECTNESS
//  OF THIS CODE OR ANY DERIVATIVE WORKS WHICH INCORPORATE IT. AUTODESK
//  PROVIDES THE CODE ON AN "AS-IS" BASIS AND EXPLICITLY DISCLAIMS ANY
//  LIABILITY, INCLUDING CONSEQUENTIAL AND INCIDENTAL DAMAGES FOR ERRORS,
//  OMISSIONS, AND OTHER PROBLEMS IN THE CODE.
//
//  Use, duplication, or disclosure by the U.S. Government is subject to
//  restrictions set forth in FAR 52.227-19 (Commercial Computer Software
//  Restricted Rights) and DFAR 252.227-7013(c)(1)(ii) (Rights in Technical
//  Data and Computer Software), as applicable.
//
// $Header: /NewRoot/DWF Toolkit/v6/develop/global/src/dwf/whiptk/matrix.cpp 1     9/12/04 8:54p Evansg $

#include "whiptk/pch.h"

///////////////////////////////////////////////////////////////////////////
void WT_Matrix::set_identity()
{
    m_elements[0][0] =
    m_elements[1][1] =
    m_elements[2][2] =
    m_elements[3][3] =  1.0;

    m_elements[0][1] = m_elements[0][2] = m_elements[0][3] =
    m_elements[1][0] = m_elements[1][2] = m_elements[1][3] =
    m_elements[2][0] = m_elements[2][1] = m_elements[2][3] =
    m_elements[3][0] = m_elements[3][1] = m_elements[3][2] = 0.0;
}

///////////////////////////////////////////////////////////////////////////
void WT_Matrix::set(WT_Matrix const & xform)
{
    WD_COPY_MEMORY(&xform.m_elements[0][0], 16 * sizeof(double), &m_elements[0][0]);
}

///////////////////////////////////////////////////////////////////////////
void WT_Matrix::set(double const * xform)
{
    WD_COPY_MEMORY(xform, 16 * sizeof(double), &m_elements[0][0]);
}

///////////////////////////////////////////////////////////////////////////
WT_Boolean WT_Matrix::is_identity() const
{
    if (m_elements[0][0] == 1.0 && m_elements[1][1] == 1.0 && m_elements[2][2] == 1.0 && m_elements[3][3] == 1.0 &&
        m_elements[0][1] == 0.0 && m_elements[0][2] == 0.0 && m_elements[0][3] == 0.0 &&
        m_elements[1][0] == 0.0 && m_elements[1][2] == 0.0 && m_elements[1][3] == 0.0 &&
        m_elements[2][0] == 0.0 && m_elements[2][1] == 0.0 && m_elements[2][3] == 0.0 &&
        m_elements[3][0] == 0.0 && m_elements[3][1] == 0.0 && m_elements[3][2] == 0.0    )
        return WD_True;
    else
        return WD_False;
}

///////////////////////////////////////////////////////////////////////////
void WT_Matrix::transform (
    WT_Point3D const &        pt,
    WT_Point3D       &        result,
    double *                  out_w,
    double                    cutoff) const
{
    double                   w;

    result.m_x = pt.m_x * m_elements[0][0] +
                 pt.m_y * m_elements[1][0] +
                 pt.m_z * m_elements[2][0] +
                          m_elements[3][0];
    result.m_y = pt.m_x * m_elements[0][1] +
                 pt.m_y * m_elements[1][1] +
                 pt.m_z * m_elements[2][1] +
                         m_elements[3][1];
    result.m_z = pt.m_x * m_elements[0][2] +
                 pt.m_y * m_elements[1][2] +
                 pt.m_z * m_elements[2][2] +
                          m_elements[3][2];
    w        =   pt.m_x * m_elements[0][3] +
                 pt.m_y * m_elements[1][3] +
                 pt.m_z * m_elements[2][3] +
                          m_elements[3][3];

    if (w > cutoff && w != 1.0)
    {
        result.m_x /= w;
        result.m_y /= w;
        result.m_z /= w;
    }

    if (out_w != WD_Null) *out_w = w;
}

///////////////////////////////////////////////////////////////////////////
void WT_Matrix::adjoin (WT_Matrix & result) const
{
    double                      a10_a21_a11_a20 = m_elements[1][0] * m_elements[2][1] -
                                                  m_elements[1][1] * m_elements[2][0],
                                a10_a22_a12_a20 = m_elements[1][0] * m_elements[2][2] -
                                                  m_elements[1][2] * m_elements[2][0],
                                a10_a23_a13_a20 = m_elements[1][0] * m_elements[2][3] -
                                                  m_elements[1][3] * m_elements[2][0],
                                a10_a31_a11_a30 = m_elements[1][0] * m_elements[3][1] -
                                                  m_elements[1][1] * m_elements[3][0],
                                a10_a32_a12_a30 = m_elements[1][0] * m_elements[3][2] -
                                                  m_elements[1][2] * m_elements[3][0],
                                a10_a33_a13_a30 = m_elements[1][0] * m_elements[3][3] -
                                                  m_elements[1][3] * m_elements[3][0],
                                a11_a22_a12_a21 = m_elements[1][1] * m_elements[2][2] -
                                                  m_elements[1][2] * m_elements[2][1],
                                a11_a23_a13_a21 = m_elements[1][1] * m_elements[2][3] -
                                                  m_elements[1][3] * m_elements[2][1],
                                a11_a32_a12_a31 = m_elements[1][1] * m_elements[3][2] -
                                                  m_elements[1][2] * m_elements[3][1],
                                a11_a33_a13_a31 = m_elements[1][1] * m_elements[3][3] -
                                                  m_elements[1][3] * m_elements[3][1],
                                a12_a23_a13_a22 = m_elements[1][2] * m_elements[2][3] -
                                                  m_elements[1][3] * m_elements[2][2],
                                a12_a33_a13_a32 = m_elements[1][2] * m_elements[3][3] -
                                                  m_elements[1][3] * m_elements[3][2],
                                a20_a31_a21_a30 = m_elements[2][0] * m_elements[3][1] -
                                                  m_elements[2][1] * m_elements[3][0],
                                a20_a32_a22_a30 = m_elements[2][0] * m_elements[3][2] -
                                                  m_elements[2][2] * m_elements[3][0],
                                a20_a33_a23_a30 = m_elements[2][0] * m_elements[3][3] -
                                                  m_elements[2][3] * m_elements[3][0],
                                a21_a32_a22_a31 = m_elements[2][1] * m_elements[3][2] -
                                                  m_elements[2][2] * m_elements[3][1],
                                a21_a33_a23_a31 = m_elements[2][1] * m_elements[3][3] -
                                                  m_elements[2][3] * m_elements[3][1],
                                a22_a33_a23_a32 = m_elements[2][2] * m_elements[3][3] -
                                                  m_elements[2][3] * m_elements[3][2];


    result.m_elements[0][0] = m_elements[1][1] * a22_a33_a23_a32 -
                              m_elements[1][2] * a21_a33_a23_a31 +
                              m_elements[1][3] * a21_a32_a22_a31;
    result.m_elements[0][1] = m_elements[0][2] * a21_a33_a23_a31 -
                              m_elements[0][3] * a21_a32_a22_a31 -
                              m_elements[0][1] * a22_a33_a23_a32;
    result.m_elements[0][2] = m_elements[0][1] * a12_a33_a13_a32 -
                              m_elements[0][2] * a11_a33_a13_a31 +
                              m_elements[0][3] * a11_a32_a12_a31;
    result.m_elements[0][3] = m_elements[0][2] * a11_a23_a13_a21 -
                              m_elements[0][3] * a11_a22_a12_a21 -
                              m_elements[0][1] * a12_a23_a13_a22;
    result.m_elements[1][0] = m_elements[1][2] * a20_a33_a23_a30 -
                              m_elements[1][3] * a20_a32_a22_a30 -
                              m_elements[1][0] * a22_a33_a23_a32;
    result.m_elements[1][1] = m_elements[0][0] * a22_a33_a23_a32 -
                              m_elements[0][2] * a20_a33_a23_a30 +
                              m_elements[0][3] * a20_a32_a22_a30;
    result.m_elements[1][2] = m_elements[0][2] * a10_a33_a13_a30 -
                              m_elements[0][3] * a10_a32_a12_a30 -
                              m_elements[0][0] * a12_a33_a13_a32;
    result.m_elements[1][3] = m_elements[0][0] * a12_a23_a13_a22 -
                              m_elements[0][2] * a10_a23_a13_a20 +
                              m_elements[0][3] * a10_a22_a12_a20;
    result.m_elements[2][0] = m_elements[1][0] * a21_a33_a23_a31 -
                              m_elements[1][1] * a20_a33_a23_a30 +
                              m_elements[1][3] * a20_a31_a21_a30;
    result.m_elements[2][1] = m_elements[0][1] * a20_a33_a23_a30 -
                              m_elements[0][3] * a20_a31_a21_a30 -
                              m_elements[0][0] * a21_a33_a23_a31;
    result.m_elements[2][2] = m_elements[0][0] * a11_a33_a13_a31 -
                              m_elements[0][1] * a10_a33_a13_a30 +
                              m_elements[0][3] * a10_a31_a11_a30;
    result.m_elements[2][3] = m_elements[0][1] * a10_a23_a13_a20 -
                              m_elements[0][3] * a10_a21_a11_a20 -
                              m_elements[0][0] * a11_a23_a13_a21;
    result.m_elements[3][0] = m_elements[1][1] * a20_a32_a22_a30 -
                              m_elements[1][2] * a20_a31_a21_a30 -
                              m_elements[1][0] * a21_a32_a22_a31;
    result.m_elements[3][1] = m_elements[0][0] * a21_a32_a22_a31 -
                              m_elements[0][1] * a20_a32_a22_a30 +
                              m_elements[0][2] * a20_a31_a21_a30;
    result.m_elements[3][2] = m_elements[0][1] * a10_a32_a12_a30 -
                              m_elements[0][2] * a10_a31_a11_a30 -
                              m_elements[0][0] * a11_a32_a12_a31;
    result.m_elements[3][3] = m_elements[0][0] * a11_a22_a12_a21 -
                              m_elements[0][1] * a10_a22_a12_a20 +
                              m_elements[0][2] * a10_a21_a11_a20;
}

///////////////////////////////////////////////////////////////////////////
void WT_Matrix::rotate (WT_Matrix & result, long rotation) const
{
    // This method takes the transform and applies a rotation of the first
    // quadrant of DWF coordinate space about that quadrant's center
    // (in other words it rotates the first quadrant around (MAX_Int/2, Max_int/2)


    // Step 1: Copy the current matrix into the destination
    result.set(*this);

    switch (rotation)
    {
        case 0:
            // Nothing to do in this case.
            break;
        case 90:
            {
                // Rotate into the second quadrant and then shift back into the 1st.
                result.m_elements[0][0] = - m_elements[0][1];
                result.m_elements[1][0] = - m_elements[1][1];
                result.m_elements[2][0] = - m_elements[2][1];
                result.m_elements[3][0] = - m_elements[3][1];

                result.m_elements[0][1] = m_elements[0][0];
                result.m_elements[1][1] = m_elements[1][0];
                result.m_elements[2][1] = m_elements[2][0];
                result.m_elements[3][1] = m_elements[3][0];

                result.m_elements[3][0] += 0x7FFFFFFF;
            }
            break;
        case 180:
                // Negate X and Y and then shift back up into the first quadrant
                result.m_elements[0][0] *= -1;
                result.m_elements[1][0] *= -1;
                result.m_elements[2][0] *= -1;
                result.m_elements[3][0] *= -1;

                result.m_elements[0][1] *= -1;
                result.m_elements[1][1] *= -1;
                result.m_elements[2][1] *= -1;
                result.m_elements[3][1] *= -1;

                result.m_elements[3][0] += 0x7FFFFFFF;
                result.m_elements[3][1] += 0x7FFFFFFF;
            break;
        case 270:
            {
                // Rotate into the fourth quadrant and then shift back into the 1st.
                result.m_elements[0][0] = m_elements[0][1];
                result.m_elements[1][0] = m_elements[1][1];
                result.m_elements[2][0] = m_elements[2][1];
                result.m_elements[3][0] = m_elements[3][1];

                result.m_elements[0][1] = - m_elements[0][0];
                result.m_elements[1][1] = - m_elements[1][0];
                result.m_elements[2][1] = - m_elements[2][0];
                result.m_elements[3][1] = - m_elements[3][0];

                result.m_elements[3][1] += 0x7FFFFFFF;
            }
            break;
        default:
            throw WT_Result::Internal_Error;
    }
}

///////////////////////////////////////////////////////////////////////////
#if DESIRED_CODE(WHIP_OUTPUT)
WT_Result WT_Matrix_IO::serialize(WT_File & file) const
{
    if (!file.heuristics().allow_binary_data())
    {
        WD_CHECK(file.write_tab_level());
        WD_CHECK(file.write("\t\t"));
    }

    WD_CHECK(file.write("(("));
    WD_CHECK(file.write_ascii(m_elements[0][0]));
    WD_CHECK(file.write((WT_Byte) ' '));
    WD_CHECK(file.write_ascii(m_elements[0][1]));
    WD_CHECK(file.write((WT_Byte) ' '));
    WD_CHECK(file.write_ascii(m_elements[0][2]));
    WD_CHECK(file.write((WT_Byte) ' '));
    WD_CHECK(file.write_ascii(m_elements[0][3]));
    if (!file.heuristics().allow_binary_data())
    {
        WD_CHECK(file.write((WT_Byte)')'));
        WD_CHECK(file.write_tab_level());
        WD_CHECK(file.write("\t\t("));
    }
    else
        WD_CHECK(file.write(")("));
    WD_CHECK(file.write_ascii(m_elements[1][0]));
    WD_CHECK(file.write((WT_Byte) ' '));
    WD_CHECK(file.write_ascii(m_elements[1][1]));
    WD_CHECK(file.write((WT_Byte) ' '));
    WD_CHECK(file.write_ascii(m_elements[1][2]));
    WD_CHECK(file.write((WT_Byte) ' '));
    WD_CHECK(file.write_ascii(m_elements[1][3]));
    if (!file.heuristics().allow_binary_data())
    {
        WD_CHECK(file.write((WT_Byte)')'));
        WD_CHECK(file.write_tab_level());
        WD_CHECK(file.write("\t\t("));
    }
    else
        WD_CHECK(file.write(")("));
    WD_CHECK(file.write_ascii(m_elements[2][0]));
    WD_CHECK(file.write((WT_Byte) ' '));
    WD_CHECK(file.write_ascii(m_elements[2][1]));
    WD_CHECK(file.write((WT_Byte) ' '));
    WD_CHECK(file.write_ascii(m_elements[2][2]));
    WD_CHECK(file.write((WT_Byte) ' '));
    WD_CHECK(file.write_ascii(m_elements[2][3]));
    if (!file.heuristics().allow_binary_data())
    {
        WD_CHECK(file.write((WT_Byte)')'));
        WD_CHECK(file.write_tab_level());
        WD_CHECK(file.write("\t\t("));
    }
    else
        WD_CHECK(file.write(")("));
    WD_CHECK(file.write_ascii(m_elements[3][0]));
    WD_CHECK(file.write((WT_Byte) ' '));
    WD_CHECK(file.write_ascii(m_elements[3][1]));
    WD_CHECK(file.write((WT_Byte) ' '));
    WD_CHECK(file.write_ascii(m_elements[3][2]));
    WD_CHECK(file.write((WT_Byte) ' '));
    WD_CHECK(file.write_ascii(m_elements[3][3]));
    return file.write("))");
}
#else
WT_Result WT_Matrix_IO::serialize(WT_File &) const
{
    return WT_Result::Success;
}
#endif  // DESIRED_CODE()

///////////////////////////////////////////////////////////////////////////
#if DESIRED_CODE(WHIP_OUTPUT)
WT_Result WT_Matrix_IO::serialize_padded(WT_File & file) const
{
    if (!file.heuristics().allow_binary_data())
    {
        WD_CHECK(file.write_tab_level());
        WD_CHECK(file.write("\t\t"));
    }

    WD_CHECK(file.write("(("));
    WD_CHECK(file.write_padded_ascii(m_elements[0][0]));
    WD_CHECK(file.write((WT_Byte) ' '));
    WD_CHECK(file.write_padded_ascii(m_elements[0][1]));
    WD_CHECK(file.write((WT_Byte) ' '));
    WD_CHECK(file.write_padded_ascii(m_elements[0][2]));
    WD_CHECK(file.write((WT_Byte) ' '));
    WD_CHECK(file.write_padded_ascii(m_elements[0][3]));
    if (!file.heuristics().allow_binary_data())
    {
        WD_CHECK(file.write((WT_Byte)')'));
        WD_CHECK(file.write_tab_level());
        WD_CHECK(file.write("\t\t("));
    }
    else
        WD_CHECK(file.write(")("));
    WD_CHECK(file.write_padded_ascii(m_elements[1][0]));
    WD_CHECK(file.write((WT_Byte) ' '));
    WD_CHECK(file.write_padded_ascii(m_elements[1][1]));
    WD_CHECK(file.write((WT_Byte) ' '));
    WD_CHECK(file.write_padded_ascii(m_elements[1][2]));
    WD_CHECK(file.write((WT_Byte) ' '));
    WD_CHECK(file.write_padded_ascii(m_elements[1][3]));
    if (!file.heuristics().allow_binary_data())
    {
        WD_CHECK(file.write((WT_Byte)')'));
        WD_CHECK(file.write_tab_level());
        WD_CHECK(file.write("\t\t("));
    }
    else
        WD_CHECK(file.write(")("));
    WD_CHECK(file.write_padded_ascii(m_elements[2][0]));
    WD_CHECK(file.write((WT_Byte) ' '));
    WD_CHECK(file.write_padded_ascii(m_elements[2][1]));
    WD_CHECK(file.write((WT_Byte) ' '));
    WD_CHECK(file.write_padded_ascii(m_elements[2][2]));
    WD_CHECK(file.write((WT_Byte) ' '));
    WD_CHECK(file.write_padded_ascii(m_elements[2][3]));
    if (!file.heuristics().allow_binary_data())
    {
        WD_CHECK(file.write((WT_Byte)')'));
        WD_CHECK(file.write_tab_level());
        WD_CHECK(file.write("\t\t("));
    }
    else
        WD_CHECK(file.write(")("));
    WD_CHECK(file.write_padded_ascii(m_elements[3][0]));
    WD_CHECK(file.write((WT_Byte) ' '));
    WD_CHECK(file.write_padded_ascii(m_elements[3][1]));
    WD_CHECK(file.write((WT_Byte) ' '));
    WD_CHECK(file.write_padded_ascii(m_elements[3][2]));
    WD_CHECK(file.write((WT_Byte) ' '));
    WD_CHECK(file.write_padded_ascii(m_elements[3][3]));
    return file.write("))");
}
#else
WT_Result WT_Matrix_IO::serialize_padded(WT_File &) const
{
    return WT_Result::Success;
}
#endif  // DESIRED_CODE()

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Matrix_IO::materialize(WT_File & file)
{
    switch (m_stage)
    {
    case Eating_Initial_Whitespace:
        WD_CHECK (file.eat_whitespace());
        m_stage = Eating_Outermost_Open_Paren;

        // no break
    case Eating_Outermost_Open_Paren:
        WT_Byte a_byte;

        WD_CHECK (file.read(a_byte));
        if (a_byte != '(')
            return WT_Result::Corrupt_File_Error;

        m_paren_count = file.increment_paren_count();

        m_stage = Getting_First_Row;

        // no break
    case Getting_First_Row:

        WD_CHECK (materialize_row(file, 0));

        m_stage = Getting_Second_Row;

        // no break
    case Getting_Second_Row:

        WD_CHECK (materialize_row(file, 1));

        m_stage = Getting_Third_Row;

        // no break
    case Getting_Third_Row:

        WD_CHECK (materialize_row(file, 2));

        m_stage = Getting_Fourth_Row;

        // no break
    case Getting_Fourth_Row:

        WD_CHECK (materialize_row(file, 3));

        m_stage = Skipping_Past_Close_Paren;

        // no break
    case Skipping_Past_Close_Paren:

        WD_CHECK (file.skip_past_matching_paren(m_paren_count));
        // Done!!
        m_stage = Eating_Initial_Whitespace;
        break;
    default:
        return WT_Result::Internal_Error;
    }

    return WT_Result::Success;
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Matrix_IO::materialize_row(WT_File & file, int row)
{

    switch(m_row_stage)
    {
    case Eating_Initial_Row_Whitespace:
        WD_CHECK(file.eat_whitespace());

        m_row_stage = Eating_Open_Paren;

        // no break
    case Eating_Open_Paren:
        WT_Byte a_byte;

        WD_CHECK (file.read(a_byte));
        if (a_byte != '(')
            return WT_Result::Corrupt_File_Error;

        m_row_paren_count = file.increment_paren_count();
        m_row_stage = Getting_First_Column;

        // no break
    case Getting_First_Column:
        WD_CHECK(file.read_ascii(m_elements[row][0]));

        m_row_stage = Getting_Second_Column;

        // no break
    case Getting_Second_Column:
        WD_CHECK(file.read_ascii(m_elements[row][1]));

        m_row_stage = Getting_Third_Column;

        // no break
    case Getting_Third_Column:
        WD_CHECK(file.read_ascii(m_elements[row][2]));

        m_row_stage = Getting_Fourth_Column;

        // no break
    case Getting_Fourth_Column:
        WD_CHECK(file.read_ascii(m_elements[row][3]));

        m_row_stage = Skipping_Past_Row_Close_Paren;

        // no break
    case Skipping_Past_Row_Close_Paren:

        WD_CHECK(file.skip_past_matching_paren(m_row_paren_count));

        // Done, now get ready to start over!
        m_row_stage = Eating_Initial_Row_Whitespace;

        break;
    default:
        return WT_Result::Internal_Error;
    }

    return WT_Result::Success;
}

///////////////////////////////////////////////////////////////////////////
WT_Boolean WT_Matrix::operator==( WT_Matrix const & r ) const
{
    for (int i=0; i<4; i++)
        for (int j=0; j<4; j++)
            if( m_elements[i][j] != r.m_elements[i][j] )
                return WD_False;
    return WD_True;
}

///////////////////////////////////////////////////////////////////////////
WT_Matrix &    WT_Matrix::operator *= (WT_Transform const & xform)
{
    m_elements[3][0] += xform.m_translate.m_x;
    m_elements[3][1] += xform.m_translate.m_y;
    m_elements[0][0] *= xform.m_x_scale;
    m_elements[1][1] *= xform.m_y_scale;

    return *this;
}

const WT_Matrix2D WT_Matrix2D::kIdentity;

///////////////////////////////////////////////////////////////////////////
WT_Matrix2D & WT_Matrix2D::operator *= (const WT_Matrix2D & rMatrix)
{
    WT_Matrix2D oResult;

    for (int i=0; i<3; i++)
    {
        for (int j=0; j<3; j++)
        {
            oResult(i,j) = 0.0;
            for (int k=0;k<3;k++)
            {
                oResult(i,j) += m_elements[i][k] * rMatrix(k,j);
            }
        }
    }

    set( oResult );
    return *this;
}

///////////////////////////////////////////////////////////////////////////
void WT_Matrix2D::rotate (WT_Matrix2D & result, double fRads) const
{
    double fSin = sin( fRads );
    double fCos = cos( fRads );

    WT_Matrix2D oRotationMatrix;
    oRotationMatrix(0,0) = fCos;
    oRotationMatrix(0,1) = -fSin;
    oRotationMatrix(1,0) = fSin;
    oRotationMatrix(1,1) = fCos;

    //Note, we had neglected to take in to account the fact that
    //matrix arithmetic is non-Abelian

    oRotationMatrix *= *this;
    result.set( oRotationMatrix );
}

///////////////////////////////////////////////////////////////////////////
void WT_Matrix2D::rotate (WT_Matrix2D & result, long degrees) const
{
    double fRads = degrees * (3.14159265358979323846 / 180.0);
    rotate( result, fRads );
} 

///////////////////////////////////////////////////////////////////////////
WT_Matrix2D& WT_Matrix2D::set_to_identity()
{
    m_elements[0][0] =
    m_elements[1][1] =
    m_elements[2][2] = 1.0;
    m_elements[0][1] =
    m_elements[0][2] =
    m_elements[1][0] =
    m_elements[1][2] =
    m_elements[2][0] =
    m_elements[2][1] = 0.0;
    return *this;
}

///////////////////////////////////////////////////////////////////////////
void WT_Matrix2D::set( const WT_Matrix2D& r )
{
    WD_COPY_MEMORY(
        &(r.m_elements[0][0]),
        9 * sizeof(double),
        &(m_elements[0][0]));
}

///////////////////////////////////////////////////////////////////////////
double WT_Matrix2D::transform(
    const WT_Point2D& p,
    WT_Point2D& result,
    double cutoff ) const
{
    WT_Point2D pt = p;
    result.m_x =
        pt.m_x * m_elements[0][0]
        + pt.m_y * m_elements[1][0]
        + m_elements[2][0];
    result.m_y =
        pt.m_x * m_elements[0][1]
        + pt.m_y * m_elements[1][1]
        + m_elements[2][1];
    double w =
        pt.m_x * m_elements[0][2]
        + pt.m_y * m_elements[1][2]
        + m_elements[2][2];
    if( w>cutoff && w!=1.0 )
    {
        result.m_x /= w;
        result.m_y /= w;
    }
    return w;
}

///////////////////////////////////////////////////////////////////////////
void WT_Matrix2D::get_adjoint( WT_Matrix2D& r ) const
{
    r.m_elements[0][0] = m_elements[1][1]*m_elements[2][2] - m_elements[1][2]*m_elements[2][1];
    r.m_elements[0][1] = m_elements[2][1]*m_elements[0][2] - m_elements[0][1]*m_elements[2][2];
    r.m_elements[0][2] = m_elements[0][1]*m_elements[1][2] - m_elements[2][1]*m_elements[0][2];
    r.m_elements[1][0] = m_elements[2][0]*m_elements[1][2] - m_elements[1][0]*m_elements[2][2];
    r.m_elements[1][1] = m_elements[0][0]*m_elements[2][2] - m_elements[2][0]*m_elements[0][2];
    r.m_elements[1][2] = m_elements[1][0]*m_elements[0][2] - m_elements[0][0]*m_elements[1][2];
    r.m_elements[2][0] = m_elements[1][0]*m_elements[2][1] - m_elements[1][1]*m_elements[2][0];
    r.m_elements[2][1] = m_elements[2][0]*m_elements[0][1] - m_elements[0][0]*m_elements[2][1];
    r.m_elements[2][2] = m_elements[0][0]*m_elements[1][1] - m_elements[1][0]*m_elements[0][1];
}

///////////////////////////////////////////////////////////////////////////
WT_Matrix2D& WT_Matrix2D::adjoin()
{
    WT_Matrix2D r;
    get_adjoint(r);
    set(r);
    return *this;
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Matrix2D_IO::serialize( WT_File& file ) const
{
    if( !file.heuristics().allow_binary_data() )
    {
        WD_CHECK(file.write_tab_level());
        WD_CHECK(file.write("\t\t"));
    }
    WD_CHECK(file.write("(("));
    WD_CHECK(file.write_ascii(m_elements[0][0]));
    WD_CHECK(file.write((WT_Byte)' '));
    WD_CHECK(file.write_ascii(m_elements[0][1]));
    WD_CHECK(file.write((WT_Byte)' '));
    WD_CHECK(file.write_ascii(m_elements[0][2]));
    if( !file.heuristics().allow_binary_data() )
    {
        WD_CHECK(file.write((WT_Byte)')'));
        WD_CHECK(file.write_tab_level());
        WD_CHECK(file.write("\t\t("));
    }
    else
        WD_CHECK(file.write(")("));
    WD_CHECK(file.write_ascii(m_elements[1][0]));
    WD_CHECK(file.write((WT_Byte)' '));
    WD_CHECK(file.write_ascii(m_elements[1][1]));
    WD_CHECK(file.write((WT_Byte)' '));
    WD_CHECK(file.write_ascii(m_elements[1][2]));
    if( !file.heuristics().allow_binary_data() )
    {
        WD_CHECK(file.write((WT_Byte)')'));
        WD_CHECK(file.write_tab_level());
        WD_CHECK(file.write("\t\t("));
    }
    else
        WD_CHECK(file.write(")("));
    WD_CHECK(file.write_ascii(m_elements[2][0]));
    WD_CHECK(file.write((WT_Byte)' '));
    WD_CHECK(file.write_ascii(m_elements[2][1]));
    WD_CHECK(file.write((WT_Byte)' '));
    WD_CHECK(file.write_ascii(m_elements[2][2]));
    return file.write("))");
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Matrix2D_IO::materialize( WT_File& file )
{
    WT_Byte a_byte;
    switch( m_stage )
    {
    case Eating_Initial_Whitespace:
        WD_CHECK( file.eat_whitespace() );
        m_stage = Getting_Open_Paren_0;
        // No Break Here

    case Getting_Open_Paren_0:
        WD_CHECK( file.read(a_byte) );
        if( a_byte != '(' )
            return WT_Result::Corrupt_File_Error;
        m_stage = Getting_Open_Paren_1;
        // No Break Here

    case Getting_Open_Paren_1:
        WD_CHECK( file.read(a_byte) );
        if( a_byte != '(' )
            return WT_Result::Corrupt_File_Error;
        m_stage = Getting_Element_00;
        // No Break Here

    case Getting_Element_00:
        WD_CHECK( file.read_ascii(m_elements[0][0]) );
        m_stage = Getting_Element_01;
        // No Break Here

    case Getting_Element_01:
        WD_CHECK( file.read_ascii(m_elements[0][1]) );
        m_stage = Getting_Element_02;
        // No Break Here

    case Getting_Element_02:
        WD_CHECK( file.read_ascii(m_elements[0][2]) );
        m_stage = Getting_Close_Paren_1;
        // No Break Here

    case Getting_Close_Paren_1:
        WD_CHECK( file.read(a_byte) );
        if( a_byte != ')' )
            return WT_Result::Corrupt_File_Error;
        m_stage = Getting_Open_Paren_2;
        // No Break Here

    case Getting_Open_Paren_2:
        WD_CHECK( file.eat_whitespace() );
        WD_CHECK( file.read(a_byte) );
        if( a_byte != '(' )
            return WT_Result::Corrupt_File_Error;
        m_stage = Getting_Element_10;
        // No Break Here

    case Getting_Element_10:
        WD_CHECK( file.read_ascii(m_elements[1][0]) );
        m_stage = Getting_Element_11;
        // No Break Here

    case Getting_Element_11:
        WD_CHECK( file.read_ascii(m_elements[1][1]) );
        m_stage = Getting_Element_12;
        // No Break Here

    case Getting_Element_12:
        WD_CHECK( file.read_ascii(m_elements[1][2]) );
        m_stage = Getting_Close_Paren_2;
        // No Break Here

    case Getting_Close_Paren_2:
        WD_CHECK( file.read(a_byte) );
        if( a_byte != ')' )
            return WT_Result::Corrupt_File_Error;
        m_stage = Getting_Open_Paren_3;
        // No Break Here

    case Getting_Open_Paren_3:
        WD_CHECK( file.eat_whitespace() );
        WD_CHECK( file.read(a_byte) );
        if( a_byte != '(' )
            return WT_Result::Corrupt_File_Error;
        m_stage = Getting_Element_20;
        // No Break Here

    case Getting_Element_20:
        WD_CHECK( file.read_ascii(m_elements[2][0]) );
        m_stage = Getting_Element_21;
        // No Break Here

    case Getting_Element_21:
        WD_CHECK( file.read_ascii(m_elements[2][1]) );
        m_stage = Getting_Element_22;
        // No Break Here

    case Getting_Element_22:
        WD_CHECK( file.read_ascii(m_elements[2][2]) );
        m_stage = Getting_Close_Paren_3;
        // No Break Here

    case Getting_Close_Paren_3:
        WD_CHECK( file.read(a_byte) );
        if( a_byte != ')' )
            return WT_Result::Corrupt_File_Error;
        m_stage = Eating_End_Whitespace;
        // No Break Here

    case Eating_End_Whitespace:
        WD_CHECK( file.skip_past_matching_paren(1) );
        m_stage = Eating_Initial_Whitespace;
        break;

    default:
        return WT_Result::Internal_Error;
    }
    return WT_Result::Success;
}

///////////////////////////////////////////////////////////////////////////
WT_Boolean WT_Matrix2D::operator== ( const WT_Matrix2D& M ) const
{
    return m_elements[0][0]==M.m_elements[0][0]
        && m_elements[0][1]==M.m_elements[0][1]
        && m_elements[0][2]==M.m_elements[0][2]
        && m_elements[1][0]==M.m_elements[1][0]
        && m_elements[1][1]==M.m_elements[1][1]
        && m_elements[1][2]==M.m_elements[1][2]
        && m_elements[2][0]==M.m_elements[2][0]
        && m_elements[2][1]==M.m_elements[2][1]
        && m_elements[2][2]==M.m_elements[2][2]
        ? WD_True
        : WD_False;
}

///////////////////////////////////////////////////////////////////////////
WT_Matrix2D & WT_Matrix2D::operator *= (double d)
{
  m_elements[0][0] *= d; m_elements[0][1] *= d; m_elements[0][2] *= d;
  m_elements[1][0] *= d; m_elements[1][1] *= d; m_elements[1][2] *= d;
  m_elements[2][0] *= d; m_elements[2][1] *= d; m_elements[2][2] *= d;
  return *this;
}

///////////////////////////////////////////////////////////////////////////
double WT_Matrix2D::minor(unsigned int r0, unsigned int r1, unsigned int c0, unsigned int c1) const
{
    if( r0>2 || c0>2 || r1>2 || c1>2 )
        throw WT_Result::Toolkit_Usage_Error;
    return m_elements[r0][c0] * m_elements[r1][c1] - m_elements[r1][c0] * m_elements[r0][c1];
}

///////////////////////////////////////////////////////////////////////////
double WT_Matrix2D::determinant() const
{
  return m_elements[0][0] * minor(1, 2, 1, 2) -
         m_elements[0][1] * minor(1, 2, 0, 2) +
         m_elements[0][2] * minor(1, 2, 0, 1);
}

///////////////////////////////////////////////////////////////////////////
void WT_Matrix2D::get_inverse (WT_Matrix2D & result) const
{
    get_adjoint(result);
    result *= 1.0 / determinant();
}
