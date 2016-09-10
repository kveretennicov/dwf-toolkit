//
// Copyright (c) 2000 by Tech Soft 3D, LLC.
// The information contained herein is confidential and proprietary to
// Tech Soft 3D, LLC., and considered a trade secret as defined under
// civil and criminal statutes.  Tech Soft 3D shall pursue its civil
// and criminal remedies in the event of unauthorized use or misappropriation
// of its trade secrets.  Use of this information by anyone other than
// authorized employees of Tech Soft 3D, LLC. is granted only under a
// written non-disclosure agreement, expressly prescribing the scope and
// manner of such use.
//
// $Header: //DWF/Working_Area/Willie.Zhu/w3dtk/BPolyPoly.cpp#1 $
//

#include "dwf/w3dtk/BStream.h"
#include "dwf/w3dtk/BInternal.h"
#include "dwf/w3dtk/BCompress.h"
#include "dwf/w3dtk/BByteStream.h"
#include "dwf/w3dtk/BPolyPoly.h"
#define ASSERT(x)

#define X_2D(subop) ((subop)&TKPP_X_2D_MASK)
#define ZERO_X(subop) (((subop)&TKPP_X_2D_MASK) == TK_PolyPolypoint::TKPP_ZERO_X)
#define SAME_X(subop) (((subop)&TKPP_X_2D_MASK) == TK_PolyPolypoint::TKPP_SAME_X)
#define PER_PRIMITIVE_X(subop) (((subop)&TKPP_X_2D_MASK) == TK_PolyPolypoint::TKPP_PER_PRIMITIVE_X)
#define Y_2D(subop) ((subop)&TKPP_Y_2D_MASK)
#define ZERO_Y(subop) (((subop)&TKPP_Y_2D_MASK) == TK_PolyPolypoint::TKPP_ZERO_Y)
#define SAME_Y(subop) (((subop)&TKPP_Y_2D_MASK) == TK_PolyPolypoint::TKPP_SAME_Y)
#define PER_PRIMITIVE_Y(subop) (((subop)&TKPP_Y_2D_MASK) == TK_PolyPolypoint::TKPP_PER_PRIMITIVE_Y)
#define Z_2D(subop) ((subop)&TKPP_Z_2D_MASK)
#define ZERO_Z(subop) (((subop)&TKPP_Z_2D_MASK) == TK_PolyPolypoint::TKPP_ZERO_Z)
#define SAME_Z(subop) (((subop)&TKPP_Z_2D_MASK) == TK_PolyPolypoint::TKPP_SAME_Z)
#define PER_PRIMITIVE_Z(subop) (((subop)&TKPP_Z_2D_MASK) == TK_PolyPolypoint::TKPP_PER_PRIMITIVE_Z)


TK_PolyPolypoint::TK_PolyPolypoint(unsigned char opcode) : BBaseOpcodeHandler (opcode) { 
    m_points = null;
    m_lengths = null;
    m_workspace = null;
    m_points_allocated = 0;
    m_lengths_allocated = 0;
    m_workspace_allocated = 0;
    Reset();
}

TK_PolyPolypoint::~TK_PolyPolypoint() {
    delete [] m_points;
    m_points = null;
    m_points_allocated = 0;
    delete [] m_lengths;
    m_lengths = null;
    delete [] m_workspace;
    m_workspace = null;
}


TK_Status TK_PolyPolypoint::Execute (BStreamFileToolkit & tk) alter {
    int i;

    for (i = 0; i < m_primitive_count; i++)
        tk.AppendLastKey(-1);
    return TK_Normal;
}


TK_Status TK_PolyPolypoint::validate_primitive_count (BStreamFileToolkit & tk) alter {
    if (m_primitive_count <= 0) {
        int i = 0;

        m_primitive_count = 0;
        while (i < m_point_count) {
            if (m_lengths[ m_primitive_count ] < 0)
                return tk.Error("invalid data in TK_PolyPolypoint: negative length");
            i += m_lengths[ m_primitive_count ];
            m_primitive_count++;
        }
        if (i != m_point_count)
            return tk.Error("invalid data in TK_PolyPolypoint: sum of lengths should be equal to the point count");
    }
    
    return TK_Normal;
}


TK_Status TK_PolyPolypoint::figure_num_floats (BStreamFileToolkit & tk) alter 
{
    if (m_primitive_count <= 0) 
        return tk.Error("internal error: m_primitive count must be initialized prior to calling TK_PolyPolypoint::figure_m_points_num_floats");

    m_points_num_floats = 0;

    
    switch (m_subop & TKPP_X_2D_MASK) {
        case TKPP_ZERO_X:
            break;
        case TKPP_SAME_X:
            m_points_num_floats += 1;
            break;
        case TKPP_PER_PRIMITIVE_X:
            m_points_num_floats += m_primitive_count;
            break;
        case 0:
            m_points_num_floats += m_point_count;
            break;
    }

    switch (m_subop & TKPP_Y_2D_MASK) {
        case TKPP_ZERO_Y:
            break;
        case TKPP_SAME_Y:
            m_points_num_floats += 1;
            break;
        case TKPP_PER_PRIMITIVE_Y:
            m_points_num_floats += m_primitive_count;
            break;
        case 0:
            m_points_num_floats += m_point_count;
            break;
    }

    switch (m_subop & TKPP_Z_2D_MASK) {
        case TKPP_ZERO_Z:
            break;
        case TKPP_SAME_Z:
            m_points_num_floats += 1;
            break;
        case TKPP_PER_PRIMITIVE_Z:
            m_points_num_floats += m_primitive_count;
            break;
        case 0:
            m_points_num_floats += m_point_count;
            break;
    }

    return TK_Normal;
}


static TK_Status figure_num_dimensions (
    BStreamFileToolkit & tk, 
    int hints, 
    int *dimensions_out) 
{
    int num_dimensions = 0;

    UNREFERENCED(tk);

    if (!X_2D(hints))
        num_dimensions++;
    if (!Y_2D(hints))
        num_dimensions++;
    if (!Z_2D(hints))
        num_dimensions++;
    if (dimensions_out != null)
        *dimensions_out = num_dimensions;

    return TK_Normal;
}


TK_Status TK_PolyPolypoint::expand_lengths(BStreamFileToolkit &tk) alter 
{
    int *temp;

    UNREFERENCED(tk);

    if (m_lengths_allocated > 100)
        m_lengths_allocated *= 2;
    else
        m_lengths_allocated += 16;
    temp = new int[m_lengths_allocated];
    memcpy(temp, m_lengths, m_primitive_count * sizeof(int));
    delete [] m_lengths;
    m_lengths = temp;

    return TK_Normal;
}


#define XBIT 0x1
#define YBIT 0x2
#define ZBIT 0x4
static TK_Status analyze_floats (
    BStreamFileToolkit & tk,
    float const *points,
    int primitive_count,
    int *lengths,
    int *hintmask_out,
    int *num_dimensions_out) 
{
#ifndef BSTREAM_READONLY
    int i, all_primitives_uniform, num_dimensions, hintmask;
    float const *ptr, *ptr2, *end;

    UNREFERENCED(tk);

    ptr = points;
    all_primitives_uniform = XBIT|YBIT|ZBIT;
    num_dimensions = 3;
    hintmask = 0;
    for (i = 0; i < primitive_count; i++) {
        end = ptr + 3*lengths[i];
        
        if (all_primitives_uniform & XBIT) {
            for (ptr2 = ptr+3; ptr2 < end; ptr2 += 3) {
                if (*ptr != *ptr2) {
                    all_primitives_uniform &= ~XBIT;
                    break;
                }
            }
        }
        
        ptr++;
        if (all_primitives_uniform & YBIT) {
            for (ptr2 = ptr+3; ptr2 < end; ptr2 += 3) {
                if (*ptr != *ptr2) {
                    all_primitives_uniform &= ~YBIT;
                    break;
                }
            }
        }
        
        ptr++;
        if (all_primitives_uniform & ZBIT) {
            for (ptr2 = ptr+3; ptr2 < end; ptr2 += 3) {
                if (*ptr != *ptr2) {
                    all_primitives_uniform &= ~ZBIT;
                    break;
                }
            }
        }
        if (!all_primitives_uniform)
            goto done;
        ptr = end; 
    }

    if (all_primitives_uniform & XBIT) {
        num_dimensions--;
        ptr = points;
        for (i = 1; i < primitive_count; i++) {
            ptr += 3*lengths[i];
            if (ptr[0] != points[0])
                break;
        }
        if (i == primitive_count) {
            if (points[0] == 0)
                hintmask |= TK_PolyPolypoint::TKPP_ZERO_X;
            else
                hintmask |= TK_PolyPolypoint::TKPP_SAME_X;
        }
        else
            hintmask |= TK_PolyPolypoint::TKPP_PER_PRIMITIVE_X;
    }
    if (all_primitives_uniform & YBIT) {
        num_dimensions--;
        ptr = points;
        for (i = 1; i < primitive_count; i++) {
            ptr += 3*lengths[i];
            if (ptr[1] != points[1])
                break;
        }
        if (i == primitive_count) {
            if (points[1] == 0)
                hintmask |= TK_PolyPolypoint::TKPP_ZERO_Y;
            else
                hintmask |= TK_PolyPolypoint::TKPP_SAME_Y;
        }
        else
            hintmask |= TK_PolyPolypoint::TKPP_PER_PRIMITIVE_Y;
    }
    if (all_primitives_uniform & ZBIT) {
        num_dimensions--;
        ptr = points;
        for (i = 1; i < primitive_count; i++) {
            ptr += 3*lengths[i];
            if (ptr[2] != points[2])
                break;
        }
        if (i == primitive_count) {
            if (points[2] == 0)
                hintmask |= TK_PolyPolypoint::TKPP_ZERO_Z;
            else
                hintmask |= TK_PolyPolypoint::TKPP_SAME_Z;
        }
        else
            hintmask |= TK_PolyPolypoint::TKPP_PER_PRIMITIVE_Z;
    }
done:
    if (hintmask_out != null)
        *hintmask_out = hintmask;
    if (num_dimensions_out != null)
        *num_dimensions_out = num_dimensions;
    return TK_Normal;
#else
    UNREFERENCED (points);
    UNREFERENCED (primitive_count);
    UNREFERENCED (lengths);
    UNREFERENCED (hintmask_out);
    UNREFERENCED (num_dimensions_out);
    return tk.Error (stream_readonly);
#endif
} 


TK_Status TK_PolyPolypoint::analyze_dimensionality(BStreamFileToolkit & tk) alter {
    TK_Status       status = TK_Normal;
    int             hintmask = 0;

    
    status = analyze_floats(tk, 
            m_points, m_primitive_count, m_lengths,
            &hintmask, &m_points_num_dimensions);
    m_subop |= hintmask;
    return status;
}


#ifndef BSTREAM_READONLY

static TK_Status mangle_floats (
    BStreamFileToolkit & tk,  
    int point_count,
    float const *points,
    int *lengths,
    int hintmask,
    float *points_out) 
{
    float *out_ptr;
    float const *in_ptr, *end;
    int i;

    UNREFERENCED(tk);

    out_ptr = points_out;
    for (i = 0; i < point_count; i++) {
        if (!X_2D(hintmask))
            *out_ptr++ = points[ i*3 + 0 ];
        if (!Y_2D(hintmask))
            *out_ptr++ = points[ i*3 + 1 ];
        if (!Z_2D(hintmask))
            *out_ptr++ = points[ i*3 + 2 ];
    }

    end = points + (3 * point_count);
    if (X_2D(hintmask)) {
        in_ptr = points + 0;
        if (PER_PRIMITIVE_X(hintmask)) {
            for (i = 0; in_ptr < end; i++) {
                *out_ptr++ = *in_ptr;
                in_ptr += 3*lengths[i];
            }
        }
        else if (SAME_X(hintmask)) {
            *out_ptr++ = *in_ptr;
        }
    }
    if (Y_2D(hintmask)) {
        in_ptr = points + 1;
        if (PER_PRIMITIVE_Y(hintmask)) {
            for (i = 0; in_ptr < end; i++) {
                *out_ptr++ = *in_ptr;
                in_ptr += 3*lengths[i];
            }
        }
        else if (SAME_Y(hintmask)) {
            *out_ptr++ = *in_ptr;
        }
    }
    if (Z_2D(hintmask)) {
        in_ptr = points + 2;
        if (PER_PRIMITIVE_Z(hintmask)) {
            for (i = 0; in_ptr < end; i++) {
                *out_ptr++ = *in_ptr;
                in_ptr += 3*lengths[i];
            }
        }
        else if (SAME_Z(hintmask)) {
            *out_ptr++ = *in_ptr;
        }
    }
    return TK_Normal;
    /***
#else
    UNREFERENCED(points_out);
    UNREFERENCED(hintmask);
    UNREFERENCED(lengths);
    UNREFERENCED(points);
    UNREFERENCED(point_count);
    UNREFERENCED(mangle_floats);
    return tk.Error (stream_readonly);
#endif
    ***/
}
#endif



static TK_Status unmangle_floats (
    BStreamFileToolkit & tk,  
    int point_count,
    float const *points,
    int primitive_count,
    int *lengths,
    int hintmask,
    float *points_out) 
{
    float *out_ptr;
    float const *in_ptr;
    int i, j;

    in_ptr = points;
    for (i = 0; i < point_count; i++) {
        if (!X_2D(hintmask))
            points_out[ i*3 + 0 ] = *in_ptr++;
        if (!Y_2D(hintmask))
            points_out[ i*3 + 1 ] = *in_ptr++;
        if (!Z_2D(hintmask))
            points_out[ i*3 + 2 ] = *in_ptr++;
    }

    if (X_2D(hintmask)) {
        out_ptr = points_out + 0;
        if (PER_PRIMITIVE_X(hintmask)) {
            for (i = 0; i < primitive_count; i++) {
                for (j = 0; j < lengths[i]; j++) {
                    *out_ptr = *in_ptr;
                    out_ptr += 3;
                }
                in_ptr++;
            }
        }
        else if (SAME_X(hintmask)) {
            for (i = 0; i < point_count; i++) {
                *out_ptr = *in_ptr;
                out_ptr += 3;
            }
            in_ptr++;
        }
        else if (ZERO_X(hintmask)) {
            for (i = 0; i < point_count; i++) {
                *out_ptr = 0;
                out_ptr += 3;
            }
        }
        else return tk.Error("internal error in unmangle_floats");
    }
    if (Y_2D(hintmask)) {
        out_ptr = points_out + 1;
        if (PER_PRIMITIVE_Y(hintmask)) {
            for (i = 0; i < primitive_count; i++) {
                for (j = 0; j < lengths[i]; j++) {
                    *out_ptr = *in_ptr;
                    out_ptr += 3;
                }
                in_ptr++;
            }
        }
        else if (SAME_Y(hintmask)) {
            for (i = 0; i < point_count; i++) {
                *out_ptr = *in_ptr;
                out_ptr += 3;
            }
            in_ptr++;
        }
        else if (ZERO_Y(hintmask)) {
            for (i = 0; i < point_count; i++) {
                *out_ptr = 0;
                out_ptr += 3;
            }
        }
        else return tk.Error("internal error in unmangle_floats");
    }
    if (Z_2D(hintmask)) {
        out_ptr = points_out + 2;
        if (PER_PRIMITIVE_Z(hintmask)) {
            for (i = 0; i < primitive_count; i++) {
                for (j = 0; j < lengths[i]; j++) {
                    *out_ptr = *in_ptr;
                    out_ptr += 3;
                }
                in_ptr++;
            }
        }
        else if (SAME_Z(hintmask)) {
            for (i = 0; i < point_count; i++) {
                *out_ptr = *in_ptr;
                out_ptr += 3;
            }
            in_ptr++;
        }
        else if (ZERO_Z(hintmask)) {
            for (i = 0; i < point_count; i++) {
                *out_ptr = 0;
                out_ptr += 3;
            }
        }
        else return tk.Error("internal error in unmangle_floats");
    }
    return TK_Normal;

} 


static void predict(int i, int num_dimensions, int *minusone, int *minustwo, int *prediction) 
{
    int j;

    
    if (i < 2) {
        if (i < 1)
            for (j = 0; j < num_dimensions; j++)
                prediction[j] = 0;
        else
            for (j = 0; j < num_dimensions; j++)
                prediction[j] = minusone[j];
    }
    else {
        
        for (j = 0; j < num_dimensions; j++)
            prediction[j] = minusone[j] + (minusone[j] - minustwo[j]);
    }
}


#define CLAMP(value,min,max) (value = (value<min)?(min):((value>max)?max:value))

TK_Status TK_PolyPolypoint::compute_line_extend_points (BStreamFileToolkit & tk) alter 
{
#ifndef BSTREAM_READONLY
    float range[3], *min, *max;
    int *minusone, *minustwo, *temp, *val, scratchspace[6];
    int prediction[3], diff[3];
    int i, j;
    BPack pack;
    float *fptr;
    int len;
    int highest;

    min = m_bbox;
    max = m_bbox + m_points_num_dimensions;
    if (m_bbox[3] == 0 &&
        m_bbox[4] == 0 &&
        m_bbox[5] == 0 &&
        m_bbox[0] == 0 &&
        m_bbox[1] == 0 &&
        m_bbox[2] == 0) {
        for (i = 0; i < m_points_num_dimensions; i++)
            min[i] = max[i] = m_points[i];
        fptr = m_points;
        for (i = 0; i < m_point_count; i++) {
            for (j = 0; j < m_points_num_dimensions; j++) {
                if (fptr[j] < min[j]) min[j] = fptr[j];
                if (fptr[j] > max[j]) max[j] = fptr[j];
            }
            fptr += m_points_num_dimensions;
        }
    }
    for (i = 0; i < m_points_num_dimensions; i++) {
        if (max[i] == min[i]) 
            range[i] = 0;
        else
            range[i] = (float)((1L<<m_bits_per_sample)-1) / (max[i] - min[i]);
    }

    len = ((m_points_num_dimensions * m_point_count * (m_bits_per_sample+1) + 31) / 32) * 4;
    if (len > m_workspace_allocated) {
        m_workspace_allocated = len;
        delete [] m_workspace;
        m_workspace = new unsigned char [ len ];
        if (!m_workspace) {
            return tk.Error(
                "Out of memory.  Buffer allocation failed from function TK_PolyPolypoint::process_line_extend." 
           );
        }
    }

    highest = (1UL << m_bits_per_sample) - 1;
    pack.InitWrite(len, m_workspace);
    fptr = m_points;
    minusone = scratchspace;
    minustwo = scratchspace + 3;
    for (i = 0; i < m_point_count; i++) {
        
        predict(i, m_points_num_dimensions, minusone, minustwo, prediction);
        
        
        val = minustwo;  
        for (j = 0; j < m_points_num_dimensions; j++) {
            val[j] = (unsigned int)((fptr[j] - min[j]) * range[j] + 0.5f);
            CLAMP(prediction[j], 0, highest);
            diff[j] = prediction[j] - val[j];
            
            pack.Put(m_bits_per_sample + 1, diff[j] + highest);
        }
        fptr += m_points_num_dimensions;
        
        temp = minusone;
        minusone = minustwo;
        minustwo = temp;
    }
    if (pack.GetStatus() == STATUS_ERROR)
        return tk.Error("internal error in bit packing for compute_line_extend_points");
    pack.SwapBytes(); 
    m_workspace_used = pack.NumBytes();

    return TK_Normal;
#else
    return tk.Error (stream_readonly);
#endif
} 



TK_Status TK_PolyPolypoint::compute_trivial_points (BStreamFileToolkit & tk) alter 
{
#ifndef BSTREAM_READONLY
    TK_Status status = TK_Normal;

    ASSERT(m_compression_scheme == CS_TRIVIAL);

    if (m_subop & TKPP_GLOBAL_QUANTIZATION) {
        float const *global_bbox;

        global_bbox = tk.GetWorldBounding()
        
        ASSERT(tk.GetTargetVersion() >= 806);
        ASSERT(global_bbox);
        memcpy(m_bbox, global_bbox, 6 * sizeof(float));
    }

    if ((m_bits_per_sample != 8) || (m_subop & TKPP_ANY_2D_MASK)) {
        
        status = quantize_and_pack_floats(
                    tk, 
                    m_point_count, 
                    m_points_num_dimensions, 
                    m_points, 
                    m_bbox, 
                    m_bits_per_sample, 
                    m_bbox, 
                    &m_workspace_allocated, 
                    &m_workspace_used, 
                    &m_workspace);
    }
    else {
        m_bits_per_sample = 8;
        status = trivial_compress_points(
                    tk, 
                    m_point_count, 
                    m_points, 
                    m_bbox, 
                    null, 
                    0, 
                    &m_workspace_allocated, 
                    &m_workspace_used, 
                    &m_workspace, 
                    m_bbox);
        m_workspace_used = 3 * m_point_count;
    }
    tk.ReportQuantizationError(m_bits_per_sample, m_bbox);
    return status;

#else
    return tk.Error (stream_readonly);
#endif
} 




TK_Status TK_PolyPolypoint::write_compressed_points (BStreamFileToolkit & tk) alter 
{
#ifndef BSTREAM_READONLY
    TK_Status       status = TK_Normal;

    switch (m_substage) {

        

        
        case 0: {
            if (!(m_subop & TKPP_GLOBAL_QUANTIZATION)) {
                if ((status = PutData(tk, m_bbox, 2*m_points_num_dimensions)) != TK_Normal)
                    return status;
            }
            m_substage++;
        } nobreak;

        
        case 1: {
            if ((status = PutData(tk, m_bits_per_sample)) != TK_Normal)
                return status;
            m_substage++;
        } nobreak;


        
        case 2: {
            if ((status = PutData(tk, m_workspace_used)) != TK_Normal)
                return status;
            m_substage++;
        } nobreak;

        
        case 3: {
            if ((status = PutData(tk, m_workspace, m_workspace_used)) != TK_Normal)
                return status;

            if (!(m_subop & TKPP_ANY_2D_MASK)) {
                
                m_substage = 0;
                break;
            }
            m_substage = 0;
        } break;


        default:
            return tk.Error("internal error from TK_PolyPolypoint::write_compressed_points");
    }
    return TK_Normal;
#else
    return tk.Error (stream_readonly);
#endif
} 



TK_Status TK_PolyPolypoint::write_trivial_leftovers (BStreamFileToolkit & tk) alter 
{
#ifndef BSTREAM_READONLY
    TK_Status       status = TK_Normal;
    float           const *ptr;
    char            temp_compression_scheme;

    switch (m_substage) {
        
        case 0: {
            m_templen = m_points_num_floats - (m_points_num_dimensions * m_point_count);
            if (m_templen == 1) {
                
                ptr = m_points + (m_points_num_dimensions * m_point_count);  
                if ((status = PutData(tk, ptr, 1)) != TK_Normal)
                    return status;
            }
            if (m_templen <= 1) {
                
                m_substage = 0;
                break;
            }
            m_workspace_used = 0;
            m_substage++;
        } nobreak;

        case 1: {
            
            ptr = m_points + (m_points_num_dimensions * m_point_count);  
            
            status = quantize_and_pack_floats(
                            tk, 
                            m_templen, 
                            1, 
                            ptr, 
                            null, 
                            m_bits_per_sample,                                  
                            m_temprange, 
                            &m_workspace_allocated, 
                            &m_workspace_used, 
                            &m_workspace);
            if (status != TK_Normal)
                return status;
            tk.ReportQuantizationError(m_bits_per_sample, m_temprange, 1);
            m_substage++;
        } nobreak;

        
        case 2: {
            temp_compression_scheme = CS_TRIVIAL;
            if ((status = PutData(tk, temp_compression_scheme)) != TK_Normal)
                return status;
            m_substage++;
        } nobreak;

        
        case 3: {
            if (!(m_subop & TKPP_GLOBAL_QUANTIZATION)) {
                if ((status = PutData(tk, m_temprange, 2)) != TK_Normal)
                    return status;
            }
            m_substage++;
        } nobreak;

        
        case 4: {
            if ((status = PutData(tk, m_workspace_used)) != TK_Normal)
                return status;
            m_substage++;
        } nobreak;

        

        
        case 5: {
            if ((status = PutData(tk, m_workspace, m_workspace_used)) != TK_Normal)
                return status;
            m_substage = 0;
        } break;

        default:
            return tk.Error("internal error from TK_PolyPolypoint::write_trivial_leftovers");
    }
    return status;
#else
    return tk.Error (stream_readonly);
#endif
} 


TK_Status TK_PolyPolypoint::read_compressed_points (BStreamFileToolkit & tk) alter {
    TK_Status       status = TK_Normal;

    switch (m_substage) {
        

        
        case 0: {
            if (m_subop & TKPP_GLOBAL_QUANTIZATION) {
                float const *global_bbox = tk.GetWorldBounding();
                if (global_bbox == null)
                    return tk.Error("error in TK_PolyPolypoint::read_compressed_points: "
                                        "global quantization requested without bounding");
                memcpy(m_bbox, global_bbox, 6 * sizeof(float));
            }
            else {
                if ((status = GetData(tk, m_bbox, 2*m_points_num_dimensions)) != TK_Normal)
                    return status;
            }
            m_substage++;
        } nobreak;

        
        case 1: {
            if ((status = GetData(tk, m_bits_per_sample)) != TK_Normal)
                return status;
            m_substage++;
        } nobreak;

        
        case 2: {
            if ((status = GetData(tk, m_workspace_used)) != TK_Normal)
                return status;
            if (m_workspace_allocated < m_workspace_used) {
                delete [] m_workspace;
                m_workspace = new unsigned char[ m_workspace_used ];
                m_workspace_allocated = m_workspace_used;
                if (m_workspace == null)
                    return tk.Error("allocation failed in function TK_PolyPolypoint::read_trivial_points");
            }
            m_substage++;
        } nobreak;

        
        case 3: {
            if ((status = GetData(tk, m_workspace, m_workspace_used)) != TK_Normal)
                return status;
            m_substage = 0;
        } break;
                
        default:
            return tk.Error("internal error from TK_PolyPolypoint::read_trivial_points");
    }
    return status;

} 



TK_Status TK_PolyPolypoint::process_trivial_points (BStreamFileToolkit & tk) alter {
    TK_Status       status = TK_Normal;
    float           *temp = null;

    if ((m_bits_per_sample != 8) || (m_subop & TKPP_ANY_2D_MASK)) {
        status = unquantize_and_unpack_floats(
                        tk, 
                        m_point_count,
                        m_points_num_dimensions,
                        m_bits_per_sample,
                        m_bbox,
                        m_workspace,
                        &temp);
    }
    else {
        status = trivial_decompress_points(
                        tk,
                        m_point_count,
                        m_workspace,
                        &temp,
                        m_bbox);
    }
    if (status != TK_Normal)
        return status;
    
    
    memcpy(m_points, temp, m_point_count * m_points_num_dimensions * sizeof(float));
    delete [] temp;
    return TK_Normal;

} 


TK_Status TK_PolyPolypoint::process_line_extend_points (BStreamFileToolkit & tk) alter {
    float range[3];
    int *minusone, *minustwo, *temp, *val, scratchspace[6];
    int prediction[3], diff[3];
    int i, j;
    BPack pack;
    float *fptr;
    int highest;
    float inverse_highest;

    highest = (unsigned int)(1L << m_bits_per_sample) - 1;
    inverse_highest = 1.0f / highest;
    for (i = 0; i < m_points_num_dimensions; i++) {
        range[i] = (m_bbox[m_points_num_dimensions + i] - m_bbox[i]) * inverse_highest;
    }

    highest = (1UL << m_bits_per_sample) - 1;
    pack.InitRead(m_workspace_used, m_workspace);
    pack.SwapBytes();
    fptr = m_points;
    minusone = scratchspace;
    minustwo = scratchspace + 3;
    for (i = 0; i < m_point_count; i++) {
        
        predict(i, m_points_num_dimensions, minusone, minustwo, prediction);
        
        
        val = minustwo;  
        for (j = 0; j < m_points_num_dimensions; j++) {
            CLAMP(prediction[j], 0, highest);
            
            
            diff[j] = pack.Get(m_bits_per_sample + 1) - highest;
            val[j] = prediction[j] - diff[j];
            if (val[j] != highest)
                fptr[j] = val[j] * range[j] + m_bbox[j];
            else
                fptr[j] = m_bbox[j + m_points_num_dimensions]; 

        }
        fptr += m_points_num_dimensions;
        
        temp = minusone;
        minusone = minustwo;
        minustwo = temp;
    }
    if (pack.GetStatus() == STATUS_ERROR)
        return tk.Error("error in bit unpacking for process_line_extend_points");

    return TK_Normal;

} 


TK_Status TK_PolyPolypoint::read_trivial_leftovers (BStreamFileToolkit & tk) alter {
    TK_Status       status = TK_Normal;
    float           *ptr, *temp = null;
    char            temp_compression_scheme;

    switch (m_substage) {

        
        case 0: {
            if (m_subop & TKPP_ANY_2D_MASK)
                m_templen = m_points_num_floats - (m_points_num_dimensions * m_point_count);
            else
                m_templen = 0;
            if (m_templen <= 1) {
                if (m_templen == 1) {
                    ptr = m_points + (m_points_num_dimensions * m_point_count);  
                    if ((status = GetData(tk, ptr, 1)) != TK_Normal)
                        return status;
                }
                m_substage = 0;
                break;
            }
            m_substage++;
        } nobreak;

        
        case 1: {
            if ((status = GetData(tk, temp_compression_scheme)) != TK_Normal)
                return status;
            ASSERT(temp_compression_scheme == CS_TRIVIAL);
            m_substage++;
        } nobreak;


        
        
        case 2: {
            if (!(m_subop & TKPP_GLOBAL_QUANTIZATION)) {
                if ((status = GetData(tk, m_temprange, 2)) != TK_Normal)
                    return status;
            }
            m_substage++;
        } nobreak;
                
        
        case 3: {
            if ((status = GetData(tk, m_workspace_used)) != TK_Normal)
                return status;
            if (m_workspace_allocated < m_workspace_used) {
                delete [] m_workspace;
                m_workspace = new unsigned char[ m_workspace_used ];
                m_workspace_allocated = m_workspace_used;
                if (m_workspace == null)
                    return tk.Error("allocation failed in function TK_PolyPolypoint::read_trivial_points");
            }
            m_substage++;
        } nobreak;

        
        case 4: {
            if ((status = GetData(tk, m_workspace, m_workspace_used)) != TK_Normal)
                return status;
            m_substage++;
        } nobreak;

        
        case 5: {
            ptr = m_points + (m_points_num_dimensions * m_point_count); 
            status = unquantize_and_unpack_floats(
                            tk, 
                            m_templen, 
                            1, 
                            m_bits_per_sample,
                            m_temprange,
                            m_workspace,
                            &temp);
            if (status != TK_Normal)
                return status;
            
            
            memcpy(ptr, temp, m_templen * sizeof(float));
            delete [] temp;
            temp = null;
            m_substage = 0;
        } break;

        default:
            return tk.Error("internal error from TK_PolyPolypoint::read_trivial_points");
    }
    return status;

} 



TK_Status TK_PolyPolypoint::mangle (BStreamFileToolkit & tk) alter 
{
#ifndef BSTREAM_READONLY
    TK_Status status = TK_Normal;
    int hints = m_subop & TKPP_ANY_2D_MASK;
    float *points_copy;

    if (!hints) {
        m_points_num_floats = 3 * m_point_count;
        return TK_Normal;
    }

    
    points_copy = new float[ m_points_allocated * 3 ];
    if (points_copy == null) 
        return tk.Error("memory allocation failed in TK_PolyPolypoint::Write");
    status = mangle_floats(
        tk,       
        m_point_count, 
        m_points,
        m_lengths,
        hints, 
        points_copy);
    delete [] m_points;
    if (status != TK_Normal) {
        m_points = null;
        m_points_allocated = 0;
        return status;
    }
    m_points = points_copy;

    return TK_Normal;
#else
    return tk.Error (stream_readonly);
#endif
}


TK_Status TK_PolyPolypoint::unmangle (BStreamFileToolkit & tk) alter 
{
    TK_Status status = TK_Normal;
    float *points_copy;

    if (!(m_subop & TKPP_ANY_2D_MASK))
        return TK_Normal;

    if (m_points_num_floats <= 0)
        return tk.Error("internal error: m_points_num_floats must be initialized prior to TK_PolyPolypoint::unmangle");

    points_copy = new float[ m_points_allocated * 3 ];
    if (points_copy == null) 
        return tk.Error("memory allocation failed in TK_PolyPolypoint::Write");
    status = unmangle_floats(
        tk,       
        m_point_count, 
        m_points,
        m_primitive_count,
        m_lengths,
        m_subop, 
        points_copy);
    delete [] m_points;
    if (status != TK_Normal) {
        m_points = null;
        m_points_allocated = 0;
        return status;
    }
    m_points = points_copy;

    return TK_Normal;
}


TK_Status TK_PolyPolypoint::Write (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_READONLY
    TK_Status       status = TK_Normal;
#ifndef BSTREAM_DISABLE_ASCII
	if (tk.GetAsciiMode()) 
		return WriteAscii (tk);
#endif
    switch (m_stage) {
        case 0: {
            if (tk.GetTargetVersion() < 702)    
                return TK_Normal;
            if (m_subop & TKPP_GLOBAL_QUANTIZATION) {
                if (tk.GetTargetVersion() < 806 ||
                    !tk.GetWorldBounding()) {
                    m_subop &= ~TKPP_GLOBAL_QUANTIZATION;
                }
            }
            if ((status = analyze_dimensionality(tk)) != TK_Normal)
                return status;
            figure_num_floats(tk);
            if (m_subop & TKPP_COMPRESSED)
                m_compression_scheme = CS_TRIVIAL;
            else
                m_compression_scheme = CS_NONE;
            m_stage++;
        }   nobreak;

        
        case 1: {
            if ((status = PutOpcode (tk, m_primitive_count)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        
        case 2: {
            if ((status = PutData (tk, m_subop)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        
        case 3: {
            if ((status = PutData (tk, m_point_count)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        
        case 4: {
            if (m_subop & TKPP_HAS_EXPLICIT_PRIMITIVE_COUNT) {
                if ((status = PutData (tk, m_primitive_count)) != TK_Normal)
                    return status;
            }
            m_stage++;
        }   nobreak;

        
        case 5: {
            if (!(m_subop & TKPP_ONE_PRIMITIVE_ONLY)) {
                if ((status = PutData (tk, m_lengths, m_primitive_count)) != TK_Normal)
                    return status;
            }
            m_stage++;
        }   nobreak;

        
        case 6: {
            if (m_subop & TKPP_COMPRESSED) {
                if ((status = PutData(tk, m_compression_scheme)) != TK_Normal)
                    return status;
            }
            m_stage++;
        } nobreak;

        
        case 7: {
            if ((status = mangle(tk)) != TK_Normal)
                return status;
            if (m_compression_scheme != CS_NONE) {
                
                m_bits_per_sample = tk.GetNumVertexBits() / 3;
                switch (m_compression_scheme) {
                    case CS_TRIVIAL: {
                        status = compute_trivial_points (tk);
                    } break;
                    case CS_LINE_EXTEND: {
                        status = compute_line_extend_points(tk);
                    } break;
                    default:
                        return tk.Error("unhandled compression scheme in TK_PolyPolypoint::Write");
                }
                if (status != TK_Normal)
                    return status;
            }
            m_stage++;
        }   nobreak;

        
        case 8: {
            if (m_compression_scheme != CS_NONE)
                status = write_compressed_points (tk);
            else
                status = PutData (tk, m_points, m_points_num_floats);
            if (status != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        
        case 9: {
            if (m_compression_scheme != CS_NONE) {
                status = write_trivial_leftovers (tk);
                if (status != TK_Normal)
                    return status;
            }
            m_stage++;
        }   nobreak;

        case 10: {
            if (Tagging (tk))
                status = Tag (tk);
            m_stage = -1;
        }   break;

        default:
            return tk.Error("internal error in TK_Status TK_PolyPolypoint::Write");
    }

    return status;
#else
    return tk.Error (stream_readonly);
#endif
}  


TK_Status TK_PolyPolypoint::Read (BStreamFileToolkit & tk) alter {
    TK_Status       status = TK_Normal;
#ifndef BSTREAM_DISABLE_ASCII
	if (tk.GetAsciiMode()) 
		return ReadAscii (tk);
#endif
    switch (m_stage) {
        
        case 0: {
            if ((status = GetData (tk, m_subop)) != TK_Normal)
                return status;
            figure_num_dimensions(tk, m_subop, &m_points_num_dimensions);
            m_stage++;
        }   nobreak;

        
        case 1: {
            if ((status = GetData (tk, m_point_count)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        
        case 2: {
            if (m_subop & TKPP_EXPLICIT_PRIMITIVE_MASK) {
                if (m_subop & TKPP_ONE_PRIMITIVE_ONLY) {
                    m_primitive_count = 1;
                }
                else if (m_subop & TKPP_HAS_EXPLICIT_PRIMITIVE_COUNT) {
                    if ((status = GetData (tk, m_primitive_count)) != TK_Normal)
                        return status;
                    m_lengths_allocated = m_primitive_count;
                    m_lengths = new int[ m_lengths_allocated ];
                    if (m_lengths == null)
                        return tk.Error("memory allocation failed in TK_PolyPolypoint::Read");
                }
            }
            m_stage++;
            m_progress = 0;
        }   nobreak;

        
        case 3: {
            if (m_subop & TKPP_EXPLICIT_PRIMITIVE_MASK) {
                if ((status = GetData (tk, m_lengths, m_primitive_count)) != TK_Normal)
                    return status;
            }
            else {
                while (m_progress < m_point_count) {
                    if (m_primitive_count >= m_lengths_allocated)
                        expand_lengths(tk);
                    if ((status = GetData (tk, m_lengths[m_primitive_count])) != TK_Normal)
                        return status;
                    m_progress += m_lengths[ m_primitive_count ];
                    m_primitive_count++;                    
                }
                m_progress = 0;
            }
            figure_num_floats(tk);
            m_stage++;
        }   nobreak;

        
        case 4: {
            if (m_subop & TKPP_COMPRESSED) {
                if ((status = GetData(tk, m_compression_scheme)) != TK_Normal)
                    return status;
            }
            else
                m_compression_scheme = CS_NONE;
            SetPoints(m_point_count);
            m_stage++;
        }   nobreak;

        case 5: {
            if (m_compression_scheme != CS_NONE)
                status = read_compressed_points (tk);
            else
                status = GetData (tk, m_points, m_points_num_floats);
            if (status != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 6: {
            if (m_compression_scheme != CS_NONE) {
                switch (m_compression_scheme) {
                    case CS_TRIVIAL: {
                        status = process_trivial_points (tk);
                    } break;
                    case CS_LINE_EXTEND: {
                        status = process_line_extend_points(tk);
                    } break;
                    default:
                        return tk.Error("unhandled compression scheme in TK_PolyPolypoint::Read");
                }
                if (status != TK_Normal)
                    return status;
            }
            m_stage++;
        }   nobreak;

        case 7: {
            if (m_compression_scheme != CS_NONE) {
                status = read_trivial_leftovers (tk);
                if (status != TK_Normal)
                    return status;
            }
            status = unmangle(tk);
            if (status != TK_Normal)
                return status;
            m_stage = -1;
        }   break;

        default:
            return tk.Error("internal error in TK_PolyPolypoint::Read");
    }
    return status;

} 


TK_Status TK_PolyPolypoint::SetPoints (int point_count, float const * points) alter {
    m_point_count = point_count;
    if (m_point_count > m_points_allocated) 
    {
        delete [] m_points;
        m_points = null;
        m_points_allocated = m_point_count;
        m_points = new float [3*m_points_allocated];
        if (m_points == null)
            return TK_Error;
    }
    if (points != null)
        memcpy (m_points, points, m_point_count*3*sizeof(float));
    return TK_Normal;
}

TK_Status TK_PolyPolypoint::SetLengths (int primitive_count, int const *lengths) alter {
    m_primitive_count = primitive_count;
    if (m_primitive_count > m_lengths_allocated) {
        delete [] m_lengths;
        m_lengths = null;
        m_lengths_allocated = m_primitive_count + 16;  
        m_lengths = new int [m_lengths_allocated];
        if (m_lengths == null)
            return TK_Error;
    }
    if (lengths != null)
        memcpy (m_lengths, lengths, primitive_count*sizeof(int));
    return TK_Normal;
}

TK_Status TK_PolyPolypoint::WriteAscii (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_READONLY
    TK_Status       status = TK_Normal;
	PutTab t0(&tk);
    switch (m_stage) {
        case 0: {
            if (tk.GetTargetVersion() < 702)    
                return TK_Normal;
            if (m_subop & TKPP_GLOBAL_QUANTIZATION) {
                if (tk.GetTargetVersion() < 806 ||
                    !tk.GetWorldBounding()) {
                    m_subop &= ~TKPP_GLOBAL_QUANTIZATION;
                }
            }
            if ((status = analyze_dimensionality(tk)) != TK_Normal)
                return status;
            figure_num_floats(tk);
            if (m_subop & TKPP_COMPRESSED)
             
                m_compression_scheme = CS_NONE;
            m_stage++;
        }   nobreak;

        
        case 1: {
            if ((status = PutAsciiOpcode (tk, m_primitive_count)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        
        case 2: {
			PutTab t(&tk);
            if ((status = PutAsciiData (tk, "Suboptions", m_subop)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        
        case 3: {
			PutTab t(&tk);
            if ((status = PutAsciiData (tk, "Point_Count", m_point_count)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        
        case 4: {
			PutTab t(&tk);
            if (m_subop & TKPP_HAS_EXPLICIT_PRIMITIVE_COUNT) {
                if ((status = PutAsciiData (tk, "Primitive_Count", m_primitive_count)) != TK_Normal)
                    return status;
            }
            m_stage++;
        }   nobreak;

        
        case 5: {
			PutTab t(&tk);
            if (!(m_subop & TKPP_ONE_PRIMITIVE_ONLY)) {
                if ((status = PutAsciiData (tk,"Length", m_lengths, m_primitive_count)) != TK_Normal)
                    return status;
            }
            m_stage++;
        }   nobreak;

        
        case 6: {
			PutTab t(&tk);
            if (m_subop & TKPP_COMPRESSED) {
                if ((status = PutAsciiData(tk, "Compression_Scheme", (int)m_compression_scheme)) != TK_Normal)
                    return status;
            }
            m_stage++;
        } nobreak;

        
        case 7: {
			PutTab t(&tk);
            if ((status = mangle(tk)) != TK_Normal)
                return status;
            if (m_compression_scheme != CS_NONE) {
                
                m_bits_per_sample = tk.GetNumVertexBits() / 3;
                switch (m_compression_scheme) {
                    case CS_TRIVIAL: {
                        status = compute_trivial_points (tk);
                    } break;
                    case CS_LINE_EXTEND: {
                        status = compute_line_extend_points(tk);
                    } break;
                    default:
                        return tk.Error("unhandled compression scheme in TK_PolyPolypoint::Write");
                }
                if (status != TK_Normal)
                    return status;
            }
            m_stage++;
        }   nobreak;

        
        case 8: {
			PutTab t(&tk);
            if (m_compression_scheme != CS_NONE)
                status = write_compressed_points (tk);
            else
                status = PutAsciiData (tk, "Points", m_points, m_points_num_floats);
            if (status != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        
        case 9: {
			PutTab t(&tk);
            if (m_compression_scheme != CS_NONE) {
                status = write_trivial_leftovers (tk);
                if (status != TK_Normal)
                    return status;
            }
            m_stage++;
        }   nobreak;

		case 10: {
			if ((status = PutAsciiOpcode (tk,1,true)) != TK_Normal)
				return status;
			m_stage++;
		}   nobreak;

        case 11: {
			PutTab t(&tk);
            if (Tagging (tk))
                status = Tag (tk);
            m_stage = -1;
        }   break;

        default:
            return tk.Error("internal error in TK_Status TK_PolyPolypoint::Write");
    }

    return status;
#else
    return tk.Error (stream_readonly);
#endif
}  


TK_Status TK_PolyPolypoint::ReadAscii (BStreamFileToolkit & tk) alter {
    TK_Status       status = TK_Normal;
#ifndef BSTREAM_DISABLE_ASCII

    switch (m_stage) {
        
        case 0: {
            if ((status = GetAsciiData (tk, "Suboptions", m_subop)) != TK_Normal)
                return status;
            figure_num_dimensions(tk, m_subop, &m_points_num_dimensions);
            m_stage++;
        }   nobreak;

        
        case 1: {
            if ((status = GetAsciiData (tk, "Point_Count", m_point_count)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        
        case 2: {
            if (m_subop & TKPP_EXPLICIT_PRIMITIVE_MASK) {
                if (m_subop & TKPP_ONE_PRIMITIVE_ONLY) {
                    m_primitive_count = 1;
                }
                else if (m_subop & TKPP_HAS_EXPLICIT_PRIMITIVE_COUNT) {
                    if ((status = GetAsciiData (tk, "Primitive_Count", m_primitive_count)) != TK_Normal)
                        return status;
                    m_lengths_allocated = m_primitive_count;
                    m_lengths = new int[ m_lengths_allocated ];
                    if (m_lengths == null)
                        return tk.Error("memory allocation failed in TK_PolyPolypoint::Read");
                }
            }
            m_stage++;
            m_progress = 0;
        }   nobreak;

        
        case 3: {
            if (m_subop & TKPP_EXPLICIT_PRIMITIVE_MASK) {
                if ((status = GetAsciiData (tk, "Length", m_lengths, m_primitive_count)) != TK_Normal)
                    return status;
            }
            else {

			 switch (m_ascii_stage) {
				case 0:{

					if ((status =  ReadAsciiWord(tk, 0)) != TK_Normal)
						return status;
						  m_ascii_stage++;   
					 }	 nobreak;
				
				case 1:{
				  while (m_progress < m_point_count) {
                    if (m_primitive_count >= m_lengths_allocated)
                        expand_lengths(tk);

					if ((status =  ReadAsciiWord(tk, 0)) != TK_Normal)
						return status;

					if (!(RemoveQuotes(m_ascii_buffer)))
						status = TK_Error;

					if (sscanf(m_ascii_buffer," %d", &m_lengths[m_primitive_count]) != 1)
						return TK_Error;
				

                   
                    m_progress += m_lengths[ m_primitive_count ];
                    m_primitive_count++;                    
				 }
					 m_ascii_stage++;   
				 }	 nobreak;
		
				 case 2:{
					if ((status = ReadAsciiLine(tk, 0)) != TK_Normal)
							return status;
						m_ascii_stage = 0;  
					} break;
				default:
					return tk.Error("internal error in TK_PolyPolypoint::Read");
			 }  
                m_progress = 0;
				m_ascii_stage = 0;
            }
            figure_num_floats(tk);
            m_stage++;
        }   nobreak;

        
        case 4: {
            if (m_subop & TKPP_COMPRESSED) 
			{
				int temp_int = 0 ;
                if ((status = GetAsciiData(tk, "Compression_Scheme", temp_int)) != TK_Normal)
                    return status;
				m_compression_scheme = (unsigned char)temp_int;
            
                m_compression_scheme = CS_NONE;
            SetPoints(m_point_count);
			}
            m_stage++;
        }   nobreak;

        case 5: {
            if (m_compression_scheme != CS_NONE)
                status = read_compressed_points (tk);
            else
                status = GetAsciiData (tk, "Points", m_points, m_points_num_floats);
            if (status != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 6: {
            if (m_compression_scheme != CS_NONE) {
                switch (m_compression_scheme) {
                    case CS_TRIVIAL: {
                        status = process_trivial_points (tk);
                    } break;
                    case CS_LINE_EXTEND: {
                        status = process_line_extend_points(tk);
                    } break;
                    default:
                        return tk.Error("unhandled compression scheme in TK_PolyPolypoint::Read");
                }
                if (status != TK_Normal)
                    return status;
            }
            m_stage++;
        }   nobreak;

        case 7: {
            if (m_compression_scheme != CS_NONE) {
                status = read_trivial_leftovers (tk);
                if (status != TK_Normal)
                    return status;
            }
            status = unmangle(tk);
            if (status != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;
		
		case 8: {
			if ((status = ReadEndOpcode(tk))!= TK_Normal)
					return status;
			m_stage = -1;
		}   break;
        default:
            return tk.Error("internal error in TK_PolyPolypoint::Read");
    }
    return status;
#else
	return tk.Error(stream_disable_ascii);
#endif 
} 

void TK_PolyPolypoint::Reset (void) alter {
    m_point_count = 0;
    m_primitive_count = 0;
    m_subop = 0;
    m_points_num_dimensions = 3;
    m_points_num_floats = 0;
    m_bits_per_sample = 0;
    m_substage = 0;
    m_progress = 0;
    m_templen = 0;
    m_workspace_used = 0;
    m_bbox[0] = m_bbox[1] = m_bbox[2] = m_bbox[3] = m_bbox[4] = m_bbox[5] = 0;
    m_compression_scheme = CS_NONE;

    BBaseOpcodeHandler::Reset();
}

IMPLEMENT_CLONE_OPCODE (TK_PolyPolypoint)
