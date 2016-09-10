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
// $Header: //DWF/Working_Area/Willie.Zhu/w3dtk/BCompress.cpp#1 $
//
// This file contains the required functions and lookup tables to implement the
// some of the compression algorithms.  The algorithm is so named because it
// is based on quantization to a set of vectors that were the output of a point
// repulsion simulation.
//

#include "dwf/w3dtk/BStream.h"
#include "dwf/w3dtk/BOpcodeShell.h"
#include "dwf/w3dtk/BCompress.h"
#include "dwf/w3dtk/BByteStream.h"
#include "dwf/w3dtk/BInternal.h"
#include <math.h>

#ifndef PI
  #define PI (3.14159265358979323846f)
#endif
#ifndef TWOPI
  #define TWOPI (PI * 2.0f)
#endif
#ifndef EPS
  #define EPS (1e-4)
#endif
#ifndef BIG_FLOAT
  #define BIG_FLOAT (1e20f)
#endif


// in TKSH_BOUNDING_ONLY mode, we always the same face list or tristrips
const int bounding_faces[30] = {
    4,0,4,5,1,  
    4,0,2,6,4,  
    4,0,1,3,2,  
    4,7,6,2,3,  
    4,7,3,1,5,  
    4,7,5,4,6  
};
const int bounding_tristrips[19] = {
    18,6,2,4,0,0,1,4,5,6,7,2,3,0,1,1,3,5,7
};
const float color_cube[6]  = {  0, 0, 0, 1,1,1 };
const float normal_cube[6] = { -1,-1,-1, 1,1,1 };
const float polar_bounds[4] = { -PI, 0, PI, PI };


/* quantize a set of floats to a given number of bits.  
   Works via a linear subdivision of the bounding volume */
TK_Status quantize_and_pack_floats (
        BStreamFileToolkit & tk,
        int count_in,
        int size_in,
        float const *floats_in, 
        float const *bounding_in,
        unsigned int *exists_in,
        unsigned int mask_in,
        unsigned int bits_per_sample_in,
        float *bounding_out,
        int *workspace_len_in_out,
        int *workspace_used_out,
        unsigned char **workspace_in_out)
{
#ifndef BSTREAM_READONLY
#ifndef BSTREAM_DISABLE_QUANTIZATION
    const float *bounding, *min, *max;
    float *tempmin, *tempmax;
    float range[128], bounding_buffer[256];  
    float const *fptr = null;
    int i, j, count, len;
    BPack pack;
    unsigned int val;
    unsigned char *out;

    if (count_in == 0)
        return TK_Normal;

    /* ignore bounding in if it contains all 0's and bounding out is non-null */
    if (bounding_out != null &&
        bounding_in != null) {
        for (i = 0; i < size_in; i++) {
            if (bounding_in[size_in + i] != 0)
                break;
            if (bounding_in[i] != 0)
                break;
        }
        if (i == size_in)
            bounding_in = null;
    }

    /* if conditions are right, use the old function, which was more specialized and faster */
    if (bits_per_sample_in == 8 && size_in == 3) {
        return trivial_compress_points(
                tk, 
                count_in, 
                floats_in, 
                bounding_in, 
                exists_in, 
                mask_in, 
                workspace_len_in_out, 
                workspace_used_out, 
                workspace_in_out, 
                bounding_out);
    }

    // if we weren't given the bounding box, calculate it
    if (bounding_in == null) {
        bounding = bounding_buffer;
        min = tempmin = &bounding_buffer[0];
        max = tempmax = &bounding_buffer[size_in];
        for (j = 0; j < size_in; j++) {
            tempmin[j] = BIG_FLOAT; 
            tempmax[j] = -BIG_FLOAT;
        }
        fptr = floats_in;
        for (i = 0; i < count_in; i++) {
            if (exists_in == null || exists_in[i] & mask_in) {
                for (j = 0; j < size_in; j++) {
                    if (fptr[j] < tempmin[j]) tempmin[j] = fptr[j];
                    if (fptr[j] > tempmax[j]) tempmax[j] = fptr[j];
                }
            }
            fptr += size_in;
        }
    }
    else {
        bounding = bounding_in; 
        min = &bounding[0];
        max = &bounding[size_in];
    }

    if (bounding_out != null) {
        memcpy(bounding_out, bounding, 2 * size_in * sizeof(float));
    }

    for (i = 0; i < size_in; i++) {
        if (max[i] == min[i]) 
            range[i] = 0;
        else
            range[i] = (float)((1L<<bits_per_sample_in)-1) / (max[i] - min[i]);
    }

    /* len_out is specified in bytes, but padded to the next multiple of 4 */
    len = ((size_in * count_in * bits_per_sample_in + 31) / 32) * 4;
    if (len > *workspace_len_in_out) {
        *workspace_len_in_out = len;
        delete [] *workspace_in_out;
        *workspace_in_out = new unsigned char [ len ];
        if (!(*workspace_in_out)) {
            return tk.Error(
                "Out of memory.  Buffer allocation failed from function quantize_and_pack_floats." 
           );
        }
    }
    out = *workspace_in_out;

    pack.InitWrite(len, out);
    fptr = floats_in;
    count = 0;
    for (i = 0; i < count_in; i++) {
        if (exists_in == null || exists_in[i] & mask_in) {
            for (j = 0; j < size_in; j++) {
                val = (unsigned int)((fptr[j] - min[j]) * range[j] + 0.5f);
                pack.Put(bits_per_sample_in, val);
            }
            fptr += size_in;
            count++;
        }
        else {
            fptr += size_in;
        }
    }
    pack.SwapBytes();
    if (workspace_used_out != null)
        *workspace_used_out = pack.NumBytes();

    return TK_Normal;
#else
        UNREFERENCED (count_in);
        UNREFERENCED (size_in);
        UNREFERENCED (floats_in);
        UNREFERENCED (bounding_in);
        UNREFERENCED (exists_in);
        UNREFERENCED (mask_in);
        UNREFERENCED (bits_per_sample_in);
        UNREFERENCED (bounding_out);
        UNREFERENCED (workspace_len_in_out);
        UNREFERENCED (workspace_used_out);
        UNREFERENCED (workspace_in_out);
    return tk.Error (stream_disable_quantization);
#endif
#else
        UNREFERENCED (count_in);
        UNREFERENCED (size_in);
        UNREFERENCED (floats_in);
        UNREFERENCED (bounding_in);
        UNREFERENCED (exists_in);
        UNREFERENCED (mask_in);
        UNREFERENCED (bits_per_sample_in);
        UNREFERENCED (bounding_out);
        UNREFERENCED (workspace_len_in_out);
        UNREFERENCED (workspace_used_out);
        UNREFERENCED (workspace_in_out);
    return tk.Error (stream_readonly);
#endif
} // end overloaded helper function quantize_and_pack_floats (with exists and mask)


TK_Status quantize_and_pack_floats (
        BStreamFileToolkit & tk,
        int count_in,
        int size_in,
        float const *floats_in, 
        float const *bounding_in,
        unsigned int bits_per_sample_in,
        float *bounding_out,
        int *workspace_len_in_out,
        int *workspace_used_out,
        unsigned char **workspace_in_out)
{
    //just pass through to the main variant, providing default values to the missing exists_in and mask_in values
    return quantize_and_pack_floats(
        tk, 
        count_in, 
        size_in, 
        floats_in, 
        bounding_in,
        null,
        0,
        bits_per_sample_in,
        bounding_out,
        workspace_len_in_out,
        workspace_used_out,
        workspace_in_out);
} //end overloaded helper function quantize_and_pack_floats (without exists and mask)


TK_Status unquantize_and_unpack_floats (
        BStreamFileToolkit & tk,
        int count_in,
        int size_in,
        unsigned int bits_per_sample_in,
        float const *bounding_in,
        unsigned char *buf_in,
        float **floats_out)
{
#ifndef BSTREAM_DISABLE_QUANTIZATION
    float range[128], *fptr = null;
    unsigned int max, val;
    int i, length;
    float inverse_max;
    BPack pack;

    /* if conditions are right, use the old function, which was more specialized and faster */
    if (bits_per_sample_in == 8 && size_in == 3) {
        return trivial_decompress_points(tk, count_in, buf_in, floats_out, bounding_in);
    }

    if (*floats_out == null) {
        fptr = new float[ count_in * size_in ];
        *floats_out = fptr;
    }
    else
        fptr = *floats_out;

    max = (unsigned int)(1L << bits_per_sample_in) - 1;
    inverse_max = 1.0f / max;

    for (i = 0; i < size_in; i++) {
        range[i] = (bounding_in[size_in + i] - bounding_in[i]) * inverse_max;
    }

    length = ((size_in * count_in * bits_per_sample_in + 31) / 32) * 4;
    pack.InitRead(length, buf_in);
    pack.SwapBytes();
    while (count_in--) {
        for (i = 0; i < size_in; i++) {
            val = pack.Get(bits_per_sample_in);
			if (val != max) {
                fptr[i] = val * range[i] + bounding_in[i];
			}
            else
                fptr[i] = bounding_in[i + size_in]; // directly set to max -- avoid fp rounding error
        }
        fptr += size_in;
    }

    return TK_Normal;
#else
    return tk.Error (stream_disable_quantization);
#endif
} // end local helper function unquantize_and_unpack_floats 


TK_Status trivial_compress_points (
        BStreamFileToolkit & tk,
        int point_count,
        float const *points, 
        float const *bounding_in, 
        unsigned int *exists,
        unsigned int mask,
        int *workspace_len_in_out,
        int *workspace_used_out,
        unsigned char **workspace_in_out,
        float *bounding_out)
{
#ifndef BSTREAM_READONLY
#ifndef BSTREAM_DISABLE_QUANTIZATION
    float min[3];
    float max[3];
    float range[3];
    float const *fptr = null;
    unsigned char *out = null;
    int i, j, valid_count, len;

    if (point_count == 0)
        return TK_Normal;

    /* ignore bounding in if it contains all 0's and bounding out is non-null */
    if (bounding_in == null ||
        (bounding_in[3] == 0 && bounding_in[4] == 0 && bounding_in[5] == 0 &&
          bounding_in[0] == 0 && bounding_in[1] == 0 && bounding_in[2] == 0)
     )
        bounding_in = null;

    // if we weren't given the bounding box, calculate it
    if (bounding_in == null) {
        fptr = points;
    //find the first point
	if (exists) {
	    for (i = 0; i < point_count; i++, fptr += 3) {
		if (exists[i] & mask) {
		    for (j = 0; j < 3; j++) {
			min[j] = max[j] = fptr[j];
		    }
		}
	    }
	}
	else {
	    for (j = 0; j < 3; j++) {
		min[j] = max[j] = fptr[j];
	    }
	    i = 1;
	    fptr += 3;
	}
    //accumulate the rest of the points
        for( /*no init*/ ; i < point_count ; i++, fptr += 3 ) {
            if (exists == null || exists[i] & mask) {
                if (fptr[0] < min[0]) min[0] = fptr[0];
                if (fptr[1] < min[1]) min[1] = fptr[1];
                if (fptr[2] < min[2]) min[2] = fptr[2];
                if (fptr[0] > max[0]) max[0] = fptr[0];
                if (fptr[1] > max[1]) max[1] = fptr[1];
                if (fptr[2] > max[2]) max[2] = fptr[2];
            }
        }
    }
    else {
        for (i = 0; i < 3; i++) {
            min[i] = bounding_in[i];
            max[i] = bounding_in[3+i];
        }
    }

    if (bounding_out != null) {
        memcpy(bounding_out, min, 3 * sizeof(float));
        bounding_out += 3;
        memcpy(bounding_out, max, 3 * sizeof(float));
    }
    if (max[0] == min[0]) 
        range[0] = 0;
    else
        range[0] = 255.0f / (max[0] - min[0]);
    if (max[1] == min[1]) 
        range[1] = 0;
    else
        range[1] = 255.0f / (max[1] - min[1]);
    if (max[2] == min[2]) 
        range[2] = 0;
    else
        range[2] = 255.0f / (max[2] - min[2]);

    len = 3 * point_count;
    if (len > *workspace_len_in_out) {
        *workspace_len_in_out = len;
        delete [] *workspace_in_out;
        *workspace_in_out = new unsigned char [ len ];
        if (!(*workspace_in_out)) {
            return tk.Error(
                "Out of memory.  Buffer allocation failed from function quantize_and_pack_floats." 
           );
        }
    }

    out = *workspace_in_out;
    fptr = points;
    valid_count = 0;    
    for (i = 0; i < point_count; i++) {
        if (exists == null || exists[i] & mask) {
            *out++ = (unsigned char)((*fptr++ - min[0]) * range[0] + 0.5f); 
            *out++ = (unsigned char)((*fptr++ - min[1]) * range[1] + 0.5f); 
            *out++ = (unsigned char)((*fptr++ - min[2]) * range[2] + 0.5f); 
            valid_count++;
        }
        else {
            fptr += 3;
            //don't advance out -- we want it packed, not spread over the entire array
        }
    }
    if (workspace_used_out != null)
        *workspace_used_out = valid_count * 3;

    return TK_Normal;
#else
        UNREFERENCED (tk);
        UNREFERENCED (point_count);
        UNREFERENCED (points);
        UNREFERENCED (bounding_in);
        UNREFERENCED (exists);
        UNREFERENCED (mask);
        UNREFERENCED (workspace_len_in_out);
        UNREFERENCED (workspace_used_out);
        UNREFERENCED (workspace_in_out);
        UNREFERENCED (bounding_out);
    return tk.Error (stream_disable_quantization);
#endif
#else
        UNREFERENCED (tk);
        UNREFERENCED (point_count);
        UNREFERENCED (points);
        UNREFERENCED (bounding_in);
        UNREFERENCED (exists);
        UNREFERENCED (mask);
        UNREFERENCED (workspace_len_in_out);
        UNREFERENCED (workspace_used_out);
        UNREFERENCED (workspace_in_out);
        UNREFERENCED (bounding_out);
    return tk.Error (stream_readonly);
#endif
} // end local helper function trivial_compress_points 




TK_Status trivial_decompress_points (
        BStreamFileToolkit & tk,
        int pointcount,
        void const *buf_in,
        float **points_out,
        float const * bounding)
{
#ifndef BSTREAM_DISABLE_QUANTIZATION
    float min[3];
    float range[3];
    float *fptr = null;
    unsigned char *cptr = null;
    unsigned char bits_per_sample = 8;
    float inverse_max;

    ENSURE_ALLOCATED(fptr, float, pointcount * 3);
    *points_out = fptr;

    //the following assumes bits_per_sample is never 0
    inverse_max = 1.0f / ((1 << bits_per_sample) - 1);

    min[0] = bounding[0];
    min[1] = bounding[1];
    min[2] = bounding[2];
    range[0] = (bounding[3] - min[0]) * inverse_max;
    range[1] = (bounding[4] - min[1]) * inverse_max;
    range[2] = (bounding[5] - min[2]) * inverse_max;

    cptr = (unsigned char *) buf_in;

    while (pointcount--) {
        if (cptr[0] == 255)
            fptr[0] = bounding[3]; // avoid fp rounding error
        else
            fptr[0] = cptr[0] * range[0] + min[0];
        if (cptr[1] == 255)
            fptr[1] = bounding[4]; // avoid fp rounding error
        else
            fptr[1] = cptr[1] * range[1] + min[1];
        if (cptr[2] == 255)
            fptr[2] = bounding[5]; // avoid fp rounding error
        else
            fptr[2] = cptr[2] * range[2] + min[2];
        fptr += 3;
        cptr += 3;
    }
    return TK_Normal;
#else
    return tk.Error (stream_disable_quantization);
#endif

} // end local helper function trivial_decompress_points 



TK_Status TK_Polyhedron::repulse_compress_normals (BStreamFileToolkit & tk, int bits)
{
#ifndef BSTREAM_READONLY
#ifndef BSTREAM_DISABLE_QUANTIZATION
#ifndef BSTREAM_DISABLE_REPULSE_COMPRESSION
    int i, u, v, end; 
    float theta, phi;
    float length;
    float const *normal;

    /* normalize the normals */
    for (i = 0; i < mp_pointcount; i++) {
        length = (float) sqrt(
            mp_normals[i*3+0] * mp_normals[i*3+0] +
            mp_normals[i*3+1] * mp_normals[i*3+1] +
            mp_normals[i*3+2] * mp_normals[i*3+2]
       );
        if (length != 0) {
            mp_normals[i*3+0] /= length;
            mp_normals[i*3+1] /= length;
            mp_normals[i*3+2] /= length;
        }
    }

    // quantize and look up the precalculated closest vector 
    switch(bits){
        case 8: {
            end = mp_pointcount;
            for (i = 0; i < end; i++) {
                if (mp_exists[i] & Vertex_Normal) {
                    normal = mp_normals + i*3;
                    phi = (float) acos(normal[2]); 
                    if (fabs(normal[1]) < EPS && 
                        fabs(normal[0]) < EPS)
                        theta = 0;
                    else
                        theta = (float) atan2(normal[1], normal[0] ); 

                    if (theta < 0) 
                        theta += TWOPI;
                    theta = (theta * MAXU8 / TWOPI);
                    u = (int)theta;
                    if (u == MAXU8)
                        u = 0;    
                    phi = (phi * MAXV8 / PI);
                    v = (int)phi;
                    if (v >= MAXV8)
                        v = MAXV8 - 1;    
                    mp_workspace[i] = closest_points8[u][v];
                }
                else {
                    mp_workspace[i] = 255;
                }
            }
        } break;

        case 10: {
            //last entry always reserved as "doesn't exist" flag
            unsigned short last_entry = (unsigned short)((1<<bits)-1); 

            memset(mp_workspace, 0, mp_workspace_allocated);
            BByteStream bs(mp_workspace_allocated, mp_workspace, 10);
            end = mp_pointcount;
            for (i = 0; i < end; i++) {
                if (mp_exists[i] & Vertex_Normal) {
                    normal = mp_normals + i*3;
                    phi = (float) acos(normal[2]); 
                    if (fabs(normal[1]) < EPS && 
                        fabs(normal[0]) < EPS)
                        theta = 0;
                    else
                        theta = (float) atan2(normal[1], normal[0] ); 

                    if (theta < 0) 
                        theta += TWOPI;
                    theta = (theta * MAXU10 / TWOPI);
                    u = (int)theta;
                    if (u == MAXU10)
                        u = 0;    
                    phi = (phi * MAXV10 / PI);
                    v = (int)phi;
                    if (v >= MAXV10)
                        v = MAXV10 - 1;    
                    bs.put(closest_points10[u][v]);
                }
                else {
                    bs.put(last_entry);
                }
            }
        } break;

        default:
            return tk.Error("internal: unsupported bit depth in repulse compress");

    }
    return TK_Normal;
#else
		UNREFERENCED (bits);
    return tk.Error (stream_disable_repulse_compression);
#endif
#else
		UNREFERENCED (bits);
    return tk.Error (stream_disable_quantization);
#endif
#else
		UNREFERENCED (bits);
    return tk.Error (stream_readonly);
#endif
} /* end function TK_Polyhedron::repulse_compress_normals */



TK_Status TK_Polyhedron::repulse_decompress_normals (BStreamFileToolkit & tk, int bits)
{
UNREFERENCED(bits);

#ifndef BSTREAM_DISABLE_QUANTIZATION
#ifndef BSTREAM_DISABLE_REPULSE_COMPRESSION
    int i; 
    unsigned short index;
    int last_entry = ((1<<bits)-1); //last entry always reserved as "doesn't exist" flag 

    switch(bits) {
        case 8: {
            for (i = 0; i < mp_pointcount; i++) {
                mp_normals[i*3+0] = normal_vectors8[mp_workspace[i]][0];
                mp_normals[i*3+1] = normal_vectors8[mp_workspace[i]][1];
                mp_normals[i*3+2] = normal_vectors8[mp_workspace[i]][2];
                if (mp_workspace[i] != last_entry) {
                    mp_exists[i] |= Vertex_Normal;
                    mp_normalcount++;
                }
            }
        } break;

        case 10: {
            BByteStream bs(mp_workspace_allocated, mp_workspace, 10);
            for (i = 0; i < mp_pointcount; i++) {
                bs.get(index);
                mp_normals[i*3+0] = normal_vectors10[index][0];
                mp_normals[i*3+1] = normal_vectors10[index][1];
                mp_normals[i*3+2] = normal_vectors10[index][2];
                if (mp_workspace[i] != last_entry) {
                    mp_exists[i] |= Vertex_Normal;
                    mp_normalcount++;
                }
            }
        } break;

        default:
            return tk.Error();
    }
    return TK_Normal;
#else
    return tk.Error (stream_disable_repulse_compression);
#endif
#else
    return tk.Error (stream_disable_quantization);
#endif
} /* end function TK_Polyhedron::repulse_decompress_normals */

