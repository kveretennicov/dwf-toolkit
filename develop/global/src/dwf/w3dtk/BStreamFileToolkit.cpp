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
// $Header: //DWF/Working_Area/Willie.Zhu/w3dtk/BStreamFileToolkit.cpp#1 $
  
//
//  Copyright (c) 2004-2006 by Autodesk, Inc.
//
//  By using this code, you are agreeing to the terms and conditions of
//  the License Agreement included in the documentation for this code.
//
//  AUTODESK MAKES NO WARRANTIES, EXPRESSED OR IMPLIED,
//  AS TO THE CORRECTNESS OF THIS CODE OR ANY DERIVATIVE
//  WORKS WHICH INCORPORATE IT.
//
//  AUTODESK PROVIDES THE CODE ON AN "AS-IS" BASIS
//  AND EXPLICITLY DISCLAIMS ANY LIABILITY, INCLUDING
//  CONSEQUENTIAL AND INCIDENTAL DAMAGES FOR ERRORS,
//  OMISSIONS, AND OTHER PROBLEMS IN THE CODE.
//
//  Use, duplication, or disclosure by the U.S. Government is subject to
//  restrictions set forth in FAR 52.227-19 (Commercial Computer Software
//  Restricted Rights) and DFAR 252.227-7013(c)(1)(ii) (Rights in Technical
//  Data and Computer Software), as applicable.
//
//
//

#define CREATE_FILE_FORMAT_HISTORY_TABLE

#include "dwf/w3dtk/BStream.h"
#include "dwf/w3dtk/BOpcodeShell.h"
#include "dwf/w3dtk/BPolyPoly.h"
#include "dwf/w3dtk/BInternal.h"

#ifdef HAVE_CONFIG_H
#include "dwf/config.h"
#endif

#ifdef    DWFTK_BUILD_ZLIB
#include "dwfcore/zlib/zlib.h"
#else
#include <zlib.h>
#endif


#ifndef _WIN32_WCE
#include <sys/stat.h>
#endif

inline void generate_error (char const * source, char const * msg) {
    dprintf ("\n----> Error in %s", source);
    if (msg != null) {
        dprintf (": %s", msg);
    }
    dprintf ("\n\n");
}


TK_Status BStreamFileToolkit::Error(char const * msg) const {
    generate_error ("BStreamFileToolkit", msg);

    if (GetLogging()) {
        BStreamFileToolkit *tk_alter = (BStreamFileToolkit *)this;
		bool				save_logging = GetLogging();

        //avoid infinite recursion in case there's an error with the log file
        tk_alter->SetLogging(false); 
        tk_alter->LogEntry ("\nTK_Error.  ");
        if (msg != null)
            tk_alter->LogEntry (msg);
        tk_alter->SetLogging(save_logging); 
    }
    return TK_Error;
}

TK_Status Internal_Data_Accumulator::error (char const * msg) const {
    generate_error ("BStreamFileToolkit::Data_Accumulator", msg);

    return TK_Error;
}

////////////////////////////////////////////////////////////////////////////////

TK_Status BStreamFileToolkit::SetAsciiMode(bool whether)  
{ 
#ifndef BSTREAM_DISABLE_ASCII
	m_is_ascii = whether; 
	return TK_Normal;
#else
	UNREFERENCED (whether);
	return Error(stream_disable_ascii);
#endif
}

bool BStreamFileToolkit::GetAsciiMode() 	
{
#ifndef BSTREAM_DISABLE_ASCII
	if ( m_is_ascii )
		return true;
	else
		return false;
#else
	Error(stream_disable_ascii);
	return false;
#endif
}

void BStreamFileToolkit::SetTabs(int num_tabs)  
{ 
	m_num_tabs = num_tabs; 
}

int BStreamFileToolkit::GetTabs() 	
{
	return m_num_tabs;
}

////////////////////////////////////////////////////////////////////////////////


#define ASSERT(x)

#ifdef BSTREAM_READONLY
    char const stream_readonly[] = "Function not implemented in Read-Only build";
#endif
#ifdef BSTREAM_DISABLE_QUANTIZATION
    char const stream_disable_quantization[] = "Function not implemented build with quantization disabled";
#endif
#ifdef BSTREAM_DISABLE_EDGEBREAKER
    char const stream_disable_edgebreaker[] = "Function not implemented build with edgebreaker disabled";
#endif
#ifdef BSTREAM_DISABLE_REPULSE_COMPRESSION
    char const stream_disable_repulse_compression[] = "Function not implemented build with repulse compression disabled";
#endif
#ifdef BSTREAM_DISABLE_ASCII
	char const stream_disable_ascii[] = "Function not implemented build with ascii disabled";
#endif



#ifdef _MSC_VER
    #ifndef BSTREAM_STATIC_LIB
        #ifndef BSTREAM_SUPPRESS_DLLMAIN
			#define WIN32_LEAN_AND_MEAN
            #include <windows.h>

            BOOL APIENTRY DllMain (HANDLE hModule, 
                                   DWORD  ul_reason_for_call, 
                                   LPVOID lpReserved) {
                switch (ul_reason_for_call) {
                    case DLL_PROCESS_ATTACH:
                    case DLL_THREAD_ATTACH:
                    case DLL_THREAD_DETACH:
                    case DLL_PROCESS_DETACH:
                        break;
                }
                UNREFERENCED (lpReserved);
                UNREFERENCED (hModule);
                return TRUE;
            }
        #endif
    #endif
#endif



void * BControlledMemoryObject::operator new (size_t size) {
    return ::operator new (size);
}

void BControlledMemoryObject::operator delete (void * p) {
    ::operator delete (p);
}




Internal_Data_Accumulator::~Internal_Data_Accumulator () {
    restart();
    delete m_z_stream;
    delete [] m_pending_buffer;
}

void Internal_Data_Accumulator::restart (void) alter {
    if (m_compressed) {
        if (m_writing)
            stop_compression (false);
        else
            stop_decompression (true);
        m_compressed = false;
    }
    
    m_pending_size = 0;
    m_buffer_size = 0;
    m_generated = 0;
}


void Internal_Data_Accumulator::clean (void) alter {
    
    delete [] m_pending_buffer;
    m_pending_buffer = null;
    m_pending_size = 0;
    m_buffer_size = 0;
        m_pending_buffer_allocated = 0;
        m_pending_position = 0;
}


TK_Status Internal_Data_Accumulator::start_compression (void) alter {
#ifndef BSTREAM_READONLY
    if (m_compressed)
        return error ("start_compression called while already compressing");

    if (m_z_stream == null)
        m_z_stream = new z_stream;

    m_z_stream->zalloc = Z_NULL;
    m_z_stream->zfree = Z_NULL;
    m_z_stream->data_type = Z_BINARY;

    if (deflateInit (m_z_stream, Z_BEST_COMPRESSION) != Z_OK)
        return error ("compression initialization failed");

    m_compressed = true;
    m_writing = true;

    return TK_Normal;
#else
    return error (stream_readonly);
#endif
}

TK_Status Internal_Data_Accumulator::stop_compression (bool flush) alter {
#ifndef BSTREAM_READONLY
    if (!m_compressed)
        return error ("stop_compression called while not compressing");

    if (flush) {
        if (m_buffer_size == 0)     
            return TK_Pending;

        int     success;
        m_z_stream->next_out = (Bytef *)m_buffer_data;
        m_z_stream->avail_out = m_buffer_size;

        success = deflate (m_z_stream, Z_FINISH);
        if (success < 0)
            return error ("compression failure during flush");

        m_generated += m_buffer_size - m_z_stream->avail_out;

        m_buffer_data = (char *)m_z_stream->next_out;
        m_buffer_size = m_z_stream->avail_out;

        if (success == Z_OK)
            return TK_Pending;

        if (deflateEnd (m_z_stream) != Z_OK)
            return error ("compression termination failed");
    }
    else {
        int     success;

        success = deflateEnd (m_z_stream);
        if (success != Z_OK && success != Z_DATA_ERROR) {   
            m_compressed = false;   
            return error ("compression termination failed");
        }
    }

    m_compressed = false;

    return TK_Normal;
#else
	UNREFERENCED (flush);
    return error (stream_readonly);
#endif
}

TK_Status Internal_Data_Accumulator::write (char const * buffer, int size) alter {
#ifndef BSTREAM_READONLY
    if (size == 0)              
        return TK_Normal;

    if (m_buffer_size == 0)
        return TK_Pending;      

    
    if (m_compressed) {
        int     success;

        m_z_stream->next_in = (Bytef *)buffer;
        m_z_stream->avail_in = size;
        m_z_stream->next_out = (Bytef *)m_buffer_data;
        m_z_stream->avail_out = m_buffer_size;

        success = deflate (m_z_stream, Z_NO_FLUSH);
        if (success != Z_OK)
            return error ("compression failure");

        m_generated += m_buffer_size - m_z_stream->avail_out;

        m_buffer_data = (char *)m_z_stream->next_out;
        m_buffer_size = m_z_stream->avail_out;

        if (m_buffer_size == 0 && m_z_stream->avail_in > 0) {
            size = m_z_stream->avail_in;
            if (size > m_pending_buffer_allocated) {
                delete [] m_pending_buffer;
                m_pending_buffer = new char [size];
            }
            memcpy (m_pending_buffer, m_z_stream->next_in, size);
            m_pending_position = m_pending_buffer;
            m_pending_size = size;
        }
    }
    else {
        m_generated += size;

        if (size <= m_buffer_size) {    
            memcpy (m_buffer_data, buffer, size);
            m_buffer_data += size;
            m_buffer_size -= size;
        }
        else {
            memcpy (m_buffer_data, buffer, m_buffer_size);
            buffer += m_buffer_size;
            size -= m_buffer_size;
            m_buffer_data += m_buffer_size;
            m_buffer_size = 0;

            if (size > m_pending_buffer_allocated) {
                delete [] m_pending_buffer;
                m_pending_buffer = new char [size];
            }
            memcpy (m_pending_buffer, buffer, size);
            m_pending_position = m_pending_buffer;
            m_pending_size = size;
        }
    }

    return TK_Normal;
#else
	UNREFERENCED (buffer);
	UNREFERENCED (size);
    return error (stream_readonly);
#endif
}

TK_Status Internal_Data_Accumulator::consume (void) alter {
#ifndef BSTREAM_READONLY
    if (m_pending_size == 0)
        return TK_Normal;
    else if (m_compressed) {
        int     success;

        m_z_stream->next_in = (Bytef *)m_pending_position;
        m_z_stream->avail_in = m_pending_size;
        m_z_stream->next_out = (Bytef *)m_buffer_data;
        m_z_stream->avail_out = m_buffer_size;

        success = deflate (m_z_stream, Z_NO_FLUSH);
        if (success != Z_OK)
            return error ("compression failure");

        m_generated += m_buffer_size - m_z_stream->avail_out;

        m_buffer_data = (char *)m_z_stream->next_out;
        m_buffer_size = m_z_stream->avail_out;
        m_pending_position = (char *)m_z_stream->next_in;
        m_pending_size = m_z_stream->avail_in;

        if (m_buffer_size == 0)
            return TK_Pending;
    }
    else {
        if (m_pending_size > m_buffer_size) {
            memcpy (m_buffer_data, m_pending_position, m_buffer_size);
            m_pending_position += m_buffer_size;
            m_pending_size -= m_buffer_size;
            m_buffer_size = 0;
            return TK_Pending;
        }
        else {
            memcpy (m_buffer_data, m_pending_position, m_pending_size);
            m_buffer_data += m_pending_size;
            m_buffer_size -= m_pending_size;
            m_pending_size = 0;
        }
    }

    return TK_Normal;
#else
    return error (stream_readonly);
#endif
}



TK_Status Internal_Data_Accumulator::start_decompression (void) alter {
    if (m_compressed)
        return error ("start_decompression called while already decompressing");

    if (m_z_stream == null)
        m_z_stream = new z_stream;

    m_z_stream->zalloc = Z_NULL;
    m_z_stream->zfree = Z_NULL;
    m_z_stream->data_type = Z_BINARY;

    if (inflateInit (m_z_stream) != Z_OK)
        return error("decompression initialization failed");

    m_compressed = true;
    m_writing = false;

    return TK_Normal;
}

TK_Status Internal_Data_Accumulator::stop_decompression (bool force) alter {
    
    

    
    

    if (force) {
        if (inflateEnd (m_z_stream) != Z_OK) {
            
        }

        m_compressed = false;
    }

    return TK_Normal;
}

TK_Status Internal_Data_Accumulator::lookat (char alter & buffer) alter {
    if (m_pending_size > 0) {
        buffer = *m_pending_position;
        return TK_Normal;
    }
    else if (m_compressed) {
        int             success = Z_OK;

        if (m_pending_buffer_allocated == 0) {
            m_pending_buffer_allocated = 1024;
            m_pending_buffer = new char [m_pending_buffer_allocated];
        }
        m_pending_position = m_pending_buffer;
        m_pending_size = 0;

        m_z_stream->next_in = (Bytef *)m_buffer_data;
        m_z_stream->avail_in = m_buffer_size;

        m_z_stream->next_out = (Bytef *)m_pending_position;
        m_z_stream->avail_out = 1;      

        success = inflate (m_z_stream, Z_NO_FLUSH);
        if (success < 0)
            return error ("decompression failed");

        m_buffer_data = (char *)m_z_stream->next_in;
        m_buffer_size = m_z_stream->avail_in;

        if (success == Z_STREAM_END) {
            
            if (inflateEnd (m_z_stream) != Z_OK)
                return error ("decompression termination failed");

            m_compressed = false;
            
        }

        if (m_z_stream->avail_out == 0) {       
            buffer = *m_pending_position;
            m_pending_size = 1;
            return TK_Normal;
        }
        else if (m_z_stream->avail_in == 0) {   
            return TK_Pending;
        }
        else if (success != Z_STREAM_END)   
            return error ("decompression in unknown state");
    }

    
    if (m_buffer_size == 0) {
        
        m_failed_size = 1;
        return TK_Pending;
    }

    buffer = *m_buffer_data;

    return TK_Normal;
}

TK_Status Internal_Data_Accumulator::read (char alter * buffer, int size) alter {
    if (size == 0)              
        return TK_Normal;

    if (size <= m_pending_size) {
        memcpy (buffer, m_pending_position, size);
        m_pending_position += size;
        m_pending_size -= size;

        return TK_Normal;
    }
    else if (m_compressed) {
        int             success = Z_OK;

        if (m_buffer_size == 0)     
            return TK_Pending;

        m_z_stream->next_in = (Bytef *)m_buffer_data;
        m_z_stream->avail_in = m_buffer_size;

        if (m_pending_size != 0) {
            
            m_z_stream->next_out = (Bytef *)m_pending_position + m_pending_size;
            m_z_stream->avail_out = size - m_pending_size;
        }
        else {
            
            m_z_stream->next_out = (Bytef *)buffer;
            m_z_stream->avail_out = size;
        }

        success = inflate (m_z_stream, Z_NO_FLUSH);
        if (success < 0)
            return error ("decompression failed");

        m_buffer_data = (char *)m_z_stream->next_in;
        m_buffer_size = m_z_stream->avail_in;

        if (success == Z_STREAM_END) {
            
            if (inflateEnd (m_z_stream) != Z_OK)
                return error ("decompression termination failed");

            m_compressed = false;
            
        }

        if (m_z_stream->avail_out == 0) {       
            if (m_pending_size != 0) {
                
                memcpy (buffer, m_pending_buffer, size);
                m_pending_size = 0;
            }
            return TK_Normal;
        }
        else if (m_z_stream->avail_in == 0) {   
            if (m_pending_size != 0) {
                
                m_pending_size = (int)((char *)m_z_stream->next_out - m_pending_position);
            }
            else {
                
                if (size > m_pending_buffer_allocated) {
                    delete [] m_pending_buffer;
                    m_pending_buffer_allocated = size + 1024;  
                    m_pending_buffer = new char [m_pending_buffer_allocated];
                }
                if ((m_pending_size = (int)((char *)m_z_stream->next_out - buffer)) > 0)
                    memcpy (m_pending_buffer, buffer, m_pending_size);
                m_pending_position = m_pending_buffer;
            }
            return TK_Pending;
        }
        else if (success != Z_STREAM_END)   
            return error ("decompression in unknown state");
    }

    
    if (size > m_pending_size + m_buffer_size) {
        m_failed_size = size;
        return TK_Pending;
    }

    if (m_pending_size != 0) {
        memcpy (buffer, m_pending_position, m_pending_size);
        buffer += m_pending_size;
        size -= m_pending_size;
        m_pending_size = 0;
    }

    memcpy (buffer, m_buffer_data, size);
    m_buffer_data += size;
    m_buffer_size -= size;


    return TK_Normal;
}

void Internal_Data_Accumulator::save (void) alter {
    int             needed = m_failed_size;
    char alter *    old_buffer = null;

    if (needed == 0)    
        return;

    if (needed > m_pending_buffer_allocated) {
        old_buffer = m_pending_buffer;
        m_pending_buffer_allocated = needed + 1024;  
        m_pending_buffer = new char [m_pending_buffer_allocated];
    }

    if (m_pending_size && m_pending_position != m_pending_buffer)
        memcpy (m_pending_buffer, m_pending_position, m_pending_size);

    if (m_buffer_size) {
        memcpy (m_pending_buffer + m_pending_size, m_buffer_data, m_buffer_size);
        m_pending_size += m_buffer_size;
        m_buffer_size = 0;
    }
    m_pending_position = m_pending_buffer;

    delete [] old_buffer;
}




Internal_ExRef_List::Internal_ExRef_List (char const * ref, ID_Key context) : m_next (null), m_context (context) {
    m_ref = new char [strlen (ref) + 1];
    strcpy (m_ref, ref);
}

Internal_ExRef_List::~Internal_ExRef_List () {
    delete [] m_ref;
}




#ifndef BSTREAM_READONLY
static float determinant (float const * points, int const * indices) {
    float const *   p0 = &points[3*indices[0]];
    float const *   p1 = &points[3*indices[1]];
    float const *   p2 = &points[3*indices[2]];
    float const *   p3 = &points[3*indices[3]];
    float           v0[3], v1[3], v2[3];

    v0[0] = p1[0] - p0[0];
    v0[1] = p1[1] - p0[1];
    v0[2] = p1[2] - p0[2];
    v1[0] = p2[0] - p0[0];
    v1[1] = p2[1] - p0[1];
    v1[2] = p2[2] - p0[2];
    v2[0] = p3[0] - p0[0];
    v2[1] = p3[1] - p0[1];
    v2[2] = p3[2] - p0[2];

    return v0[0] * (v1[1]*v2[2] - v1[2]*v2[1]) -
           v0[1] * (v1[0]*v2[2] - v1[2]*v2[0]) +
           v0[2] * (v1[0]*v2[1] - v1[1]*v2[0]);

}

static float find_basis_points (int count, float const * points, int * basis) {
    int         limit, save, i;
    int         swap = 0, random = 53 % count;
    float       best, score = 0;

    if (count < 4) return 0;

    for (i = 0; i < 4; i++)
        basis[i] = i * count / 4;

    if (count > 8)
        limit = 8;
    else
        limit = count;

    
    best = (float)fabs (determinant (points, basis));

    for (i = 0; i < limit; i++) {
        save = basis[swap];
        basis[swap] = random;
        score = (float)fabs (determinant (points, basis));
        if (score < best)
            basis[swap] = save;  
        else
            best = score;
        swap = (swap+1) % 4;
        random = (random * 13 + 1023) % count;
    }

    return best;
}

static bool invert_matrix (
    register    float const             *a,
    register    float alter             *inv) {
    float                   tmp[16];

    float       a10_21_11_20 = a[ 4]*a[ 9] - a[ 5]*a[ 8],
                            a10_22_12_20 = a[ 4]*a[10] - a[ 6]*a[ 8],
                            a10_23_13_20 = a[ 4]*a[11] - a[ 7]*a[ 8],
                            a10_31_11_30 = a[ 4]*a[13] - a[ 5]*a[12],
                            a10_32_12_30 = a[ 4]*a[14] - a[ 6]*a[12],
                            a10_33_13_30 = a[ 4]*a[15] - a[ 7]*a[12],
                            a11_22_12_21 = a[ 5]*a[10] - a[ 6]*a[ 9],
                            a11_23_13_21 = a[ 5]*a[11] - a[ 7]*a[ 9],
                            a11_32_12_31 = a[ 5]*a[14] - a[ 6]*a[13],
                            a11_33_13_31 = a[ 5]*a[15] - a[ 7]*a[13],
                            a12_23_13_22 = a[ 6]*a[11] - a[ 7]*a[10],
                            a12_33_13_32 = a[ 6]*a[15] - a[ 7]*a[14],
                            a20_31_21_30 = a[ 8]*a[13] - a[ 9]*a[12],
                            a20_32_22_30 = a[ 8]*a[14] - a[10]*a[12],
                            a20_33_23_30 = a[ 8]*a[15] - a[11]*a[12],
                            a21_32_22_31 = a[ 9]*a[14] - a[10]*a[13],
                            a21_33_23_31 = a[ 9]*a[15] - a[11]*a[13],
                            a22_33_23_32 = a[10]*a[15] - a[11]*a[14];
    float       p0, p1, p2, p3;

    tmp[ 0] = a[5]*a22_33_23_32 - a[6]*a21_33_23_31 + a[7]*a21_32_22_31;
    tmp[ 1] = a[2]*a21_33_23_31 - a[3]*a21_32_22_31 - a[1]*a22_33_23_32;
    tmp[ 2] = a[1]*a12_33_13_32 - a[2]*a11_33_13_31 + a[3]*a11_32_12_31;
    tmp[ 3] = a[2]*a11_23_13_21 - a[3]*a11_22_12_21 - a[1]*a12_23_13_22;
    tmp[ 4] = a[6]*a20_33_23_30 - a[7]*a20_32_22_30 - a[4]*a22_33_23_32;
    tmp[ 5] = a[0]*a22_33_23_32 - a[2]*a20_33_23_30 + a[3]*a20_32_22_30;
    tmp[ 6] = a[2]*a10_33_13_30 - a[3]*a10_32_12_30 - a[0]*a12_33_13_32;
    tmp[ 7] = a[0]*a12_23_13_22 - a[2]*a10_23_13_20 + a[3]*a10_22_12_20;
    tmp[ 8] = a[4]*a21_33_23_31 - a[5]*a20_33_23_30 + a[7]*a20_31_21_30;
    tmp[ 9] = a[1]*a20_33_23_30 - a[3]*a20_31_21_30 - a[0]*a21_33_23_31;
    tmp[10] = a[0]*a11_33_13_31 - a[1]*a10_33_13_30 + a[3]*a10_31_11_30;
    tmp[11] = a[1]*a10_23_13_20 - a[3]*a10_21_11_20 - a[0]*a11_23_13_21;
    tmp[12] = a[5]*a20_32_22_30 - a[6]*a20_31_21_30 - a[4]*a21_32_22_31;
    tmp[13] = a[0]*a21_32_22_31 - a[1]*a20_32_22_30 + a[2]*a20_31_21_30;
    tmp[14] = a[1]*a10_32_12_30 - a[2]*a10_31_11_30 - a[0]*a11_32_12_31;
    tmp[15] = a[0]*a11_22_12_21 - a[1]*a10_22_12_20 + a[2]*a10_21_11_20;

    p0 =  tmp[ 0];
    p1 = -tmp[ 4];
    p2 =  tmp[ 8];
    p3 = -tmp[12];

    float       det = a[0]*p0 - a[1]*p1 + a[2]*p2 - a[3]*p3;

    if (det == 0.0f)
        return false;

    register    float       invdet = 1.0f / det;

    inv[ 0] = tmp[ 0] * invdet;
    inv[ 1] = tmp[ 1] * invdet;
    inv[ 2] = tmp[ 2] * invdet;
    inv[ 3] = tmp[ 3] * invdet;
    inv[ 4] = tmp[ 4] * invdet;
    inv[ 5] = tmp[ 5] * invdet;
    inv[ 6] = tmp[ 6] * invdet;
    inv[ 7] = tmp[ 7] * invdet;
    inv[ 8] = tmp[ 8] * invdet;
    inv[ 9] = tmp[ 9] * invdet;
    inv[10] = tmp[10] * invdet;
    inv[11] = tmp[11] * invdet;
    inv[12] = tmp[12] * invdet;
    inv[13] = tmp[13] * invdet;
    inv[14] = tmp[14] * invdet;
    inv[15] = tmp[15] * invdet;

    return  true;
}
#endif

bool Recorded_Instance::generate_basis (int count, float const * points) alter {
#ifndef BSTREAM_READONLY
    if (m_basis_valid)
        return true;

    if (count > 0) {
        float           min, max, epsilon;
        int             i;

        
        min = max = points[0];
        for (i=1; i<3*count; i++) {
            if (points[i] < min)
                min = points[i];
            else if (points[i] > max)
                max = points[i];
        }
        
        epsilon = 0.01f * (max - min);
        epsilon *= epsilon * epsilon;   

        
        
        if (find_basis_points (count, points, m_basis_indices) >= epsilon) {
            int             b0 = 3*m_basis_indices[0];
            int             b1 = 3*m_basis_indices[1];
            int             b2 = 3*m_basis_indices[2];
            int             b3 = 3*m_basis_indices[3];
            float           matrix[16];

            
            
            matrix[ 0] = points[b1+0] - points[b0+0];
            matrix[ 1] = points[b1+1] - points[b0+1];
            matrix[ 2] = points[b1+2] - points[b0+2];
            matrix[ 3] = 0.0f;
            matrix[ 4] = points[b2+0] - points[b0+0];
            matrix[ 5] = points[b2+1] - points[b0+1];
            matrix[ 6] = points[b2+2] - points[b0+2];
            matrix[ 7] = 0.0f;
            matrix[ 8] = points[b3+0] - points[b0+0];
            matrix[ 9] = points[b3+1] - points[b0+1];
            matrix[10] = points[b3+2] - points[b0+2];
            matrix[11] = 0.0f;
            matrix[12] = points[b0+0];
            matrix[13] = points[b0+1];
            matrix[14] = points[b0+2];
            matrix[15] = 1.0f;

            
            if (invert_matrix (matrix, m_local_basis)) {
                
                i = count - 1;
                do if (i != m_basis_indices[0] &&
                       i != m_basis_indices[1] &&
                       i != m_basis_indices[2] &&
                       i != m_basis_indices[3])
                     break;
                while (--i > 0);

                m_arbitrary_point[0] = points[3*i+0];
                m_arbitrary_point[1] = points[3*i+1];
                m_arbitrary_point[2] = points[3*i+2];
                m_arbitrary_index = i;

                m_basis_valid = true;
            }
        }
    }

    
    if (!m_basis_valid)
        m_opcode = 0;

    return m_basis_valid;
#else
	UNREFERENCED (count);
	UNREFERENCED (points);
    return false;
#endif
}




BStreamFileToolkit::BStreamFileToolkit() {
	int         i;

    _pInputStream = null;

	m_active_segments = null;
	m_write_flags = 0;
	m_read_flags = 0;
	m_save_write_flags = 0;
	m_num_normal_bits = 10;
	m_num_vertex_bits = 24;
	m_num_parameter_bits = 24;
	m_num_color_bits = 24;
	m_num_index_bits = 8;
	m_stage = 0;
	m_substage = 0;
	m_pass = 0;
	m_tag_count = 0;
	m_revisit = null;
	m_revisit_working = null;
	m_file_names = null;
	m_file_indices = null;
	m_file_count = 0;
	m_files_allocated = 0;
	m_current_filename = null;
	m_index_base = 0;
	m_filename = null;
	m_wfilename = null;
	m_quantization_error = 0;
	m_file = null;
	m_position = 0;
	m_offset = 0;
	m_unused = 0;
	m_file_version = 0;
	m_target_version = TK_File_Format_Version;
	m_header_comment_seen = false;
	m_log_file = null;
	m_log_fp = null;
	m_logging = false;
	m_logging_options = ~0U; 
	m_log_line_length = 0;
	m_opcode_sequence = 0;
	m_objects_written = 0;
	m_progress_callback = 0;
	m_buffer_limit = TK_DEFAULT_BUFFER_SIZE;
	m_nesting_level = 0;
	m_dictionary_format = 3;
	m_dictionary_options = TK_Dictionary_Bounding_Volumes;
	m_pause_table = null;
	m_pause_table_size = 0;
	m_num_pauses = 0;
	m_jpeg_quality = 75;
	m_world_bounding = null;
	m_external_references = null;
	m_external_ref_tail = null;
	m_geometry_open = false;

	m_prewalk = null;
	m_postwalk = null;
	m_prewalk_count = 0;
	m_postwalk_count = 0;

	m_default_object = new TK_Default;
	m_current_object = m_default_object;

    m_is_ascii = 0;
	m_num_tabs = 0;


	for (i = 0; i < 256; ++i) {
		m_objects[i] = new TK_Unavailable ((char)i);
		m_instance_hash[i] = null;
	}

	m_last_keys_used = 0;
	m_last_keys_allocated = 1;
	m_last_keys = new ID_Key [m_last_keys_allocated];

	SetOpcodeHandler (TKE_Font,					new TK_Font);
	SetOpcodeHandler (TKE_Texture,				new TK_Texture);
	SetOpcodeHandler (TKE_Glyph_Definition,		new TK_Glyph_Definition);
	SetOpcodeHandler (TKE_Line_Style,			new TK_Line_Style);

	SetOpcodeHandler (TKE_Comment,				new TK_Comment);
	SetOpcodeHandler (TKE_File_Info,			new TK_File_Info);
	SetOpcodeHandler (TKE_Termination,			new TK_Terminator (TKE_Termination));
	SetOpcodeHandler (TKE_Pause,				new TK_Terminator (TKE_Pause));
	SetOpcodeHandler (TKE_Start_Compression,	new TK_Compression (TKE_Start_Compression));
	SetOpcodeHandler (TKE_Stop_Compression,		new TK_Compression (TKE_Stop_Compression));

	SetOpcodeHandler (TKE_Geometry_Attributes,	new TK_Geometry_Attributes);
	SetOpcodeHandler (TKE_Renumber_Key_Global,	new TK_Renumber (TKE_Renumber_Key_Global));
	SetOpcodeHandler (TKE_Renumber_Key_Local,	new TK_Renumber (TKE_Renumber_Key_Local));
	SetOpcodeHandler (TKE_Priority,				new TK_Renumber (TKE_Priority));
	SetOpcodeHandler (TKE_Tag,					new TK_Tag);
	SetOpcodeHandler (TKE_Dictionary,			new TK_Dictionary);
	SetOpcodeHandler (TKE_Dictionary_Locater,	new TK_Dictionary_Locater);
	SetOpcodeHandler (TKE_Thumbnail,			new TK_Thumbnail);

	SetOpcodeHandler (TKE_Start_User_Data,		new TK_User_Data);
	
	SetOpcodeHandler (TKE_XML,					new TK_XML);
	SetOpcodeHandler (TKE_External_Reference,	new TK_External_Reference);
	SetOpcodeHandler (TKE_URL,					new TK_URL);

	SetOpcodeHandler (TKE_Delete_Object,		new TK_Delete_Object);

	SetOpcodeHandler (TKE_Open_Segment,			new TK_Open_Segment);
	SetOpcodeHandler (TKE_Close_Segment,		new TK_Close_Segment);
	SetOpcodeHandler (TKE_Reopen_Segment,		new TK_Reopen_Segment);
	SetOpcodeHandler (TKE_Include_Segment,		new TK_Referenced_Segment (TKE_Include_Segment));
	SetOpcodeHandler (TKE_Style_Segment,		new TK_Referenced_Segment (TKE_Style_Segment));
	SetOpcodeHandler (TKE_Repeat_Object,		new TK_Instance);

	SetOpcodeHandler (TKE_Bounding,				new TK_Bounding (TKE_Bounding));
	SetOpcodeHandler (TKE_Bounding_Info,		new TK_Bounding (TKE_Bounding_Info));
	SetOpcodeHandler (TKE_Clip_Rectangle,		new TK_Clip_Rectangle);
	SetOpcodeHandler (TKE_Clip_Region,			new TK_Clip_Region);

	SetOpcodeHandler (TKE_Callback,				new TK_Callback);
	SetOpcodeHandler (TKE_Camera,				new TK_Camera);
	SetOpcodeHandler (TKE_View,					new TK_Camera (TKE_View));
	SetOpcodeHandler (TKE_Window,				new TK_Window);
	SetOpcodeHandler (TKE_Visibility,			new TK_Visibility);
	SetOpcodeHandler (TKE_Selectability,		new TK_Selectability);
	SetOpcodeHandler (TKE_Color,				new TK_Color);
	SetOpcodeHandler (TKE_Color_RGB,			new TK_Color_RGB);
	SetOpcodeHandler (TKE_Color_By_Value,		new TK_Color_By_Value);
	SetOpcodeHandler (TKE_Color_By_Index,		new TK_Color_By_Index (TKE_Color_By_Index));
	SetOpcodeHandler (TKE_Color_By_Index_16,	new TK_Color_By_Index (TKE_Color_By_Index_16));
	SetOpcodeHandler (TKE_Color_By_FIndex,		new TK_Color_By_FIndex);
	SetOpcodeHandler (TKE_Color_Map,			new TK_Color_Map);
	SetOpcodeHandler (TKE_Conditions,			new TK_Conditions);
	SetOpcodeHandler (TKE_Modelling_Matrix,		new TK_Matrix (TKE_Modelling_Matrix));
	SetOpcodeHandler (TKE_Texture_Matrix,		new TK_Matrix (TKE_Texture_Matrix));
	SetOpcodeHandler (TKE_Face_Pattern,			new TK_Enumerated (TKE_Face_Pattern));
	SetOpcodeHandler (TKE_Window_Pattern,		new TK_Enumerated (TKE_Window_Pattern));
	SetOpcodeHandler (TKE_Marker_Symbol,		new TK_Named (TKE_Marker_Symbol));
	SetOpcodeHandler (TKE_Text_Alignment,		new TK_Enumerated (TKE_Text_Alignment));
	SetOpcodeHandler (TKE_Window_Frame,			new TK_Enumerated (TKE_Window_Frame));
	SetOpcodeHandler (TKE_Handedness,			new TK_Enumerated (TKE_Handedness));
	SetOpcodeHandler (TKE_Edge_Pattern,			new TK_Named (TKE_Edge_Pattern));
	SetOpcodeHandler (TKE_Line_Pattern,			new TK_Named (TKE_Line_Pattern));
	SetOpcodeHandler (TKE_Edge_Weight,			new TK_Size (TKE_Edge_Weight));
	SetOpcodeHandler (TKE_Line_Weight,			new TK_Size (TKE_Line_Weight));
	SetOpcodeHandler (TKE_Marker_Size,			new TK_Size (TKE_Marker_Size));
	SetOpcodeHandler (TKE_Rendering_Options,	new TK_Rendering_Options);
	SetOpcodeHandler (TKE_Heuristics,			new TK_Heuristics);
	SetOpcodeHandler (TKE_Text_Font,			new TK_Text_Font);
	SetOpcodeHandler (TKE_Text_Spacing,			new TK_Size (TKE_Text_Spacing));
	SetOpcodeHandler (TKE_Text_Path,			new TK_Point (TKE_Text_Path));
	SetOpcodeHandler (TKE_Unicode_Options,		new TK_Unicode_Options);
	SetOpcodeHandler (TKE_User_Options,			new TK_User_Options);
	SetOpcodeHandler (TKE_User_Index,			new TK_User_Index);
	SetOpcodeHandler (TKE_User_Value,			new TK_User_Value);
	SetOpcodeHandler (TKE_Streaming_Mode,		new TK_Streaming);

	SetOpcodeHandler (TKE_Circle,				new TK_Circle (TKE_Circle));
	SetOpcodeHandler (TKE_Circular_Arc,			new TK_Circle (TKE_Circular_Arc));
	SetOpcodeHandler (TKE_Circular_Chord,		new TK_Circle (TKE_Circular_Chord));
	SetOpcodeHandler (TKE_Circular_Wedge,		new TK_Circle (TKE_Circular_Wedge));
	SetOpcodeHandler (TKE_Sphere,				new TK_Sphere);
	SetOpcodeHandler (TKE_Cylinder,				new TK_Cylinder);
	SetOpcodeHandler (TKE_Ellipse,				new TK_Ellipse (TKE_Ellipse));
	SetOpcodeHandler (TKE_Elliptical_Arc,		new TK_Ellipse (TKE_Elliptical_Arc));
	SetOpcodeHandler (TKE_Grid,					new TK_Grid);
	SetOpcodeHandler (TKE_Image,				new TK_Image);
	SetOpcodeHandler (TKE_Line,					new TK_Line);
	SetOpcodeHandler (TKE_PolyCylinder,			new TK_PolyCylinder);
	SetOpcodeHandler (TKE_Polygon,				new TK_Polypoint (TKE_Polygon));
	SetOpcodeHandler (TKE_Polyline,				new TK_Polypoint (TKE_Polyline));
	SetOpcodeHandler (TKE_PolyPolyline,			new TK_PolyPolypoint (TKE_PolyPolyline));
	SetOpcodeHandler (TKE_Marker,				new TK_Point (TKE_Marker));
	SetOpcodeHandler (TKE_Distant_Light,		new TK_Point (TKE_Distant_Light));
	SetOpcodeHandler (TKE_Local_Light,			new TK_Point (TKE_Local_Light));
	SetOpcodeHandler (TKE_Spot_Light,			new TK_Spot_Light);
	SetOpcodeHandler (TKE_Area_Light,			new TK_Area_Light);
	SetOpcodeHandler (TKE_Shell,				new TK_Shell);
	SetOpcodeHandler (TKE_Mesh,					new TK_Mesh);
	SetOpcodeHandler (TKE_NURBS_Curve,			new TK_NURBS_Curve);
	SetOpcodeHandler (TKE_NURBS_Surface,		new TK_NURBS_Surface);
	SetOpcodeHandler (TKE_Cutting_Plane,		new TK_Cutting_Plane);
	SetOpcodeHandler (TKE_Text,					new TK_Text (TKE_Text));
	SetOpcodeHandler (TKE_Text_With_Encoding,	new TK_Text (TKE_Text_With_Encoding));
	SetOpcodeHandler (TKE_Reference,			new TK_Reference);
	SetOpcodeHandler (TKE_Infinite_Line,		new TK_Infinite_Line);
    //
    // W3D Extensions
    //
    //

    SetOpcodeHandler (TKE_HW3D_Image,            new W3D_Image);

	SetOpcodeHandler (TKE_Close_Geometry_Attributes,	new TK_Close_Geometry_Attributes);
}



BStreamFileToolkit::~BStreamFileToolkit() {
    int         i;

	delete [] m_filename;
	delete [] m_wfilename;

    for (i = 0; i < m_file_count; ++i)
        delete [] m_file_names[i];
    delete [] m_file_names;
    delete [] m_file_indices;

    delete [] m_last_keys;
    delete [] m_log_file;
    empty_lists();
    for (i = 0; i < 256; ++i)
        delete m_objects[i];
    delete m_default_object;
    for (i = 0; i < m_prewalk_count; i++)
        delete m_prewalk[i];
    for (i = 0; i < m_postwalk_count; i++)
        delete m_postwalk[i];
    delete [] m_prewalk;
    delete [] m_postwalk;
    delete [] m_pause_table;
    delete [] m_world_bounding;
}


void BStreamFileToolkit::RecordPause (int offset) alter {
    if (m_num_pauses >= m_pause_table_size) {
        int *       old_table = m_pause_table;

        m_pause_table = new int [m_pause_table_size += 8];
        if (m_num_pauses > 0)
            memcpy (m_pause_table, old_table, m_num_pauses * sizeof(int));
        delete [] old_table;
    }

    m_pause_table[m_num_pauses++] = offset;
}




TK_Status BStreamFileToolkit::LocateDictionary (void) alter {
    FILE *          fp = (FILE *)m_file;
    char            block[TK_DEFAULT_BUFFER_SIZE];
    int             amount;
    TK_Status       status = TK_Normal;

    if (!GetWriteFlags(TK_Generate_Dictionary))
        return Error ("file not generated with dictionary");

    if (fp == null)
        return Error ("no open file");

    if ((status = PositionFile (-5)) != TK_Normal)
        return status;
    if ((status = ReadBuffer (block, 5, amount)) != TK_Normal)
        return status;
    
    if (block[4] != TKE_Termination)
        return Error ("file does not end correctly");

    
    #ifdef STREAM_BIGENDIAN
        block[4] = block[0];
        block[0] = block[3];
        block[3] = block[4];
        block[4] = block[1];
        block[1] = block[2];
        block[2] = block[4];
    #endif
    memcpy (&amount, block, sizeof (int));

    return PositionFile (amount);
}



TK_Status BStreamFileToolkit::LocateEntity (ID_Key key, int variant) alter {
    int             offset, length;
    char const *    oldfile = GetCurrentFile();
    char const *    newfile;
    TK_Status       status = TK_Normal;

    if ((status = GetOffset (key, variant, offset, length, newfile)) != TK_Normal) {
        if (status == TK_Error) 
            return Error ("locate entity translation failure");
        else 
            return status;
    }

    if (offset == 0)    
        return TK_NotFound;

    if (newfile != null && newfile != oldfile) {    
        if ((status = SelectFile (newfile)) != TK_Normal)
            return Error ("locate entity cannot select file");

        CloseFile ();
        OpenFile (newfile, false);
    }

    return PositionFile (offset);
}



void BStreamFileToolkit::SetFilename (char const * name) alter {
	delete [] m_filename;
	m_filename = new char [strlen (name) + 1];
	strcpy (m_filename, name);

	delete [] m_wfilename;
	m_wfilename = null;
}

void BStreamFileToolkit::SetFilename (unsigned short const * name) alter {
    int					length = 0;

	while (name[length] != 0)
		++length;

	delete [] m_wfilename;
	m_wfilename = new unsigned short [length + 1];
	memcpy (m_wfilename, name, (length+1)*sizeof(unsigned short));

	delete [] m_filename;
	m_filename = null;
}



void BStreamFileToolkit::SetNewFile (char const * name) alter {
    if (m_file_count == m_files_allocated) {
        register    char **     old_names = m_file_names;
        register    int *       old_indices = m_file_indices;

        m_files_allocated += 16;
        m_file_names = new char * [m_files_allocated];
        memset (m_file_names, 0, m_files_allocated * sizeof (char *));
        memcpy (m_file_names, old_names, m_file_count * sizeof (char *));
        m_file_indices = new int [m_files_allocated];
        memset (m_file_indices, 0, m_files_allocated * sizeof (int));
        memcpy (m_file_indices, old_indices, m_file_count * sizeof (int));
    }

    m_file_names[m_file_count] = new char [strlen(name)+1];
    strcpy (m_file_names[m_file_count], name);
    m_file_indices[m_file_count] = m_translator.used();

    m_current_filename = m_file_names[m_file_count];
    m_index_base = m_file_indices[m_file_count];

    ++m_file_count;
}

TK_Status BStreamFileToolkit::SelectFile (char const * name) alter {
    if (m_file_names != null) {
        register    int             i;

        for (i = 0; i < m_file_count; ++i) {
            if (streq (name, m_file_names[i])) {    
                m_current_filename = m_file_names[i];
                m_index_base = m_file_indices[i];
                return TK_Normal;
            }
        }
    }

    return TK_NotFound;
}


TK_Status BStreamFileToolkit::OpenFile (char const * name, bool write) alter {
    if ((m_file = fopen (name, write ? "wb" : "rb")) == null)
        return Error ("file open failure");
	if (name != m_filename)
		SetFilename(name);
    return TK_Normal;
}

TK_Status BStreamFileToolkit::OpenFile (unsigned short const * name, bool write) alter {
	#ifdef _MSC_VER
            //
            // cast safe on Win32 since wchar_t will be mapped to unsigned short
            //
		if ((m_file = _wfopen ((const wchar_t*)name, write ? L"wb" : L"rb")) == null)
			return Error ("file open failure");
		if (name != m_wfilename)
			SetFilename(name);	
		return TK_Normal;
	#else
		return Error ("wide character filenames not supported in this build");
	#endif
}
TK_Status
BStreamFileToolkit::OpenStream( DWFInputStream& rReadStream ) alter
{
    _pInputStream = &rReadStream;
    return TK_Normal;
}

TK_Status
BStreamFileToolkit::CloseStream() alter
{
    _pInputStream = null;
    return TK_Normal;
}

TK_Status BStreamFileToolkit::CloseFile (void) alter {
    FILE *          fp = (FILE *)m_file;

    if (fp == null)
        return Error ("no file open to close");
    if (fclose (fp) != 0)
        return Error ("file close failure");

    m_file = null;
    return TK_Normal;
}




TK_Status BStreamFileToolkit::ReadBuffer (char alter * buffer, int size, int alter & amount_read) alter
{
    amount_read = 0;

        //
        // use the stream if available
        //
    if (_pInputStream)
    {
        try
        {
            if (_pInputStream->available() == 0)
            {
                return Error( "end of stream" );
            }

            amount_read = (int) _pInputStream->read( buffer, size );
        }
        catch (DWFException&)
        {
            return Error( "DWFException occured - i/o error" );
        }
    }
    else
    {
        FILE *          fp = (FILE *)m_file;

        if (fp == null)
            return Error ("no file open for read");
        if (feof (fp))
            return Error ("read past end of file");

        amount_read = (int) fread (buffer, sizeof (char), size, fp);
    }

    return TK_Normal;
}



TK_Status BStreamFileToolkit::WriteBuffer (char alter * buffer, int size) alter {
#ifndef BSTREAM_READONLY
    FILE *          fp = (FILE *)m_file;
    int             amount_written;

    if (fp == null)
        return Error ("no file open for write");

    amount_written = (int)fwrite (buffer, sizeof (char), size, fp);
    if (amount_written != size)
        return Error ("file write failure");

    return TK_Normal;
#else
	UNREFERENCED (buffer);
	UNREFERENCED (size);
    return Error (stream_readonly);
#endif
}



TK_Status BStreamFileToolkit::PositionFile (int offset) alter
{
    if (_pInputStream)
    {
        try
        {
            if (offset >=0)
            {
                _pInputStream->seek( SEEK_SET, offset );
            }
            else
            {
                _pInputStream->seek( SEEK_END, offset );
            }
        }
        catch (DWFException&)
        {
            return Error( "DWFException - i/o error" );
        }
    }
    else
    {
        FILE *          fp = (FILE *)m_file;
        int             ret;

        if (fp == null)
            return Error ("no file open to position");

        if (offset >= 0)
            ret = fseek (fp, offset, SEEK_SET);
        else
            ret = fseek (fp, offset, SEEK_END);

        if (ret != 0)
            return Error ("file position failure");
    }

    return TK_Normal;
}



TK_Status BStreamFileToolkit::GetFileSize (unsigned long & size) alter {
    bool			query = false;
    if (_pInputStream)
    {
        try
        {
            // Limits to 4GB -- bump it to size_t / WT_Integer64?
            size = (unsigned long) _pInputStream->available();
            return TK_Normal;
        }
        catch (DWFException&)
        {
            return Error( "DWFException - i/o error" );
        }
    }
    else
    {
        FILE *          fp = (FILE *)m_file;
        if (fp == null)
            return Error ("no file open to query");

#ifndef _WIN32_WCE
        struct stat     statbuf;
#ifdef  _DWFCORE_WIN32_SYSTEM
        if (fstat (_fileno (fp), &statbuf) != 0)
#else
        if (fstat (fileno (fp), &statbuf) != 0)
#endif
        {
#ifdef _MSC_VER
            if (m_filename && _stat(m_filename, (struct _stat *)&statbuf) == 0)
                query = true;

            if (m_wfilename && _wstat((const wchar_t *)m_wfilename, (struct _stat *)&statbuf) == 0)
                query  = true;
#endif
        }
        else
            query = true;

        if (!query)
            return Error ("unable to query file");
        size = statbuf.st_size;

        return TK_Normal;
#else
        return Error ("unable to query file");
#endif
    }
}

void BStreamFileToolkit::SetOpcodeHandler (int which, BBaseOpcodeHandler * handler) alter {
    if (which < 0 || which > 255)
            return;

    delete m_objects[which];
    m_objects[which] = handler;
}


void BStreamFileToolkit::SetPrewalkHandler (BBaseOpcodeHandler * handler) alter {
    BBaseOpcodeHandler **temp;

    temp = new BBaseOpcodeHandler *[m_prewalk_count + 1];
    memcpy (temp, m_prewalk, m_prewalk_count * sizeof (BBaseOpcodeHandler *));
    delete [] m_prewalk;
    m_prewalk = temp;
    m_prewalk[ m_prewalk_count ] = handler;
    m_prewalk_count++;
}


void BStreamFileToolkit::SetPostwalkHandler (BBaseOpcodeHandler * handler) alter {
    BBaseOpcodeHandler **temp;

    temp = new BBaseOpcodeHandler *[m_postwalk_count + 1];
    memcpy (temp, m_postwalk, m_postwalk_count * sizeof (BBaseOpcodeHandler *));
    delete [] m_postwalk;
    m_postwalk = temp;
    m_postwalk[ m_postwalk_count ] = handler;
    m_postwalk_count++;
}


void BStreamFileToolkit::empty_lists (void) alter {
    int         i;

    for (i=0; i<256; i++) {
        while (m_instance_hash[i] != null) {
            register    Recorded_Instance alter *       victim = m_instance_hash[i];
            m_instance_hash[i] = victim->m_next;
            delete victim;
        }
    }

    while (m_active_segments != null) {
        register    Internal_Segment_List alter *       victim = m_active_segments;
        m_active_segments = victim->m_next;
        delete victim;
    }

    while (m_revisit != null) {
        register    Internal_Revisit_Item alter *       victim = m_revisit;
        m_revisit = victim->m_next;
        delete victim;
    }
    while (m_revisit_working != null) {
        register    Internal_Revisit_Item alter *       victim = m_revisit_working;
        m_revisit_working = victim->m_next;
        delete victim;
    }

    while (m_external_references != null) {
        register    Internal_ExRef_List alter *         victim = m_external_references;
        m_external_references = victim->m_next;
        delete victim;
    }
    m_external_ref_tail = null;
}



void BStreamFileToolkit::Restart (void) alter {
    int         i;

    _pInputStream = null;

    m_accumulator.restart();
    m_translator.clean();
    m_visited_items.clean();
    m_current_object = m_default_object;
    ClearLastKey();
    m_stage = 0;
    m_substage = 0;
    m_pass = 0;
    m_tag_count = 0;
    m_file_version = 0;
    m_target_version = TK_File_Format_Version;
    m_header_comment_seen = false;
    m_position = 0;
    m_offset = 0;
    m_objects_written = 0;
    m_nesting_level = 0;
    m_num_pauses = 0;
    m_jpeg_quality = 75;
	m_geometry_open = false;

    if (m_file != null)
        CloseFile();

    if (m_log_fp != null)
        CloseLogFile();
    delete [] m_log_file;
    m_log_file = null;
    
    m_log_line_length = 0;
    m_opcode_sequence = 0;

    for (i = 0; i < m_file_count; ++i)
        delete [] m_file_names[i];
    delete [] m_file_names;
    delete [] m_file_indices;
    m_file_names = null;
    m_file_indices = null;
    m_file_count = 0;
    m_files_allocated = 0;
    m_current_filename = null;
    m_index_base = 0;

	delete [] m_filename;
    m_filename = null;
	delete [] m_wfilename;
    m_wfilename = null;

    empty_lists();

    for (i = 0; i < m_prewalk_count; i++)
        m_prewalk[i]->Reset();
    for (i = 0; i < m_postwalk_count; i++)
        m_postwalk[i]->Reset();
    for (i = 0; i < 256; i++)
        m_objects[i]->Reset();
}

void BStreamFileToolkit::read_completed (void) alter {
    
}

int BStreamFileToolkit::ParseVersion (char const * block) const 
{
	if (!strncmp (block, ";; HSF V", 8)) {
		char const *    cp = &block[8];
		int             version = 0;

		while (cp) {
			if ('0' <= *cp && *cp <= '9')
				version = 10 * version  +  (int)(*cp++ - '0');
			else if (*cp == '.')
				++cp;       
			else if (*cp == ' ')
				cp = 0;		
			else
				return Error ("error reading version number");   
		}
		return version;
	}
	else 
		return 0;
}


TK_Status BStreamFileToolkit::ParseBuffer (char const * b, int s, TK_Status mode) alter {
    TK_Status                   status = TK_Normal;
    TK_Status                   executed;
    Internal_Segment_List *     current_active = null;
    ID_Key                      current_key = -1;
    bool                        local_open = false;

    if (m_logging && m_log_fp == null) {
        if (OpenLogFile (m_log_file ? m_log_file : "hsf_import_log.txt",
                         m_opcode_sequence ? "a+" : "w") != TK_Normal)
            return TK_Error;

        local_open = true;
    }

    m_accumulator.set_data ((char *)b, s);

    if ((current_active = m_active_segments) != null)
        ActivateContext (current_key = current_active->key());

    forever {
        status = m_current_object->Read (*this);
        
        if (status == TK_Pause) {
            
            if ((mode == TK_Pause) && GetWriteFlags(TK_Generate_Dictionary))
                status = TK_Complete;
            else
                status = TK_Normal;
        }

        if (status != TK_Normal) {
            if (status == TK_Pending)
                m_accumulator.save ();
            else if (status == TK_Complete) {
				if (m_geometry_open) {
					CloseGeometry();
					m_current_object = m_default_object;
					continue;
				}

                m_current_object->Reset();
                m_current_object = m_default_object;
            }
            break;
        }

        executed = m_current_object->Execute (*this);

        if (current_active != m_active_segments) {      
            if (current_active != null)
                DeactivateContext (current_key);

            if ((current_active = m_active_segments) != null)
                ActivateContext (current_key = current_active->key());
        }

        m_current_object->Reset();
        m_current_object = m_default_object;

        if (executed == TK_Revisit)
            continue;                                   

        if (executed != TK_Normal)
            return executed;

        if (mode == TK_Single && m_nesting_level == 0 && !m_geometry_open) {
            status = TK_Complete;
            break;
        }
    }

    if (current_active != null)
        DeactivateContext (current_key);

    m_unused = m_accumulator.unused();

    if (status == TK_Complete) {
        if (m_accumulator.compressed())
            stop_decompression (true);

        read_completed();
    }

    if (status == TK_Complete || status == TK_Error)
        m_accumulator.clean();

    if (local_open)
        CloseLogFile();

    return status;
}


void BStreamFileToolkit::SetLogFile (char const * filename) alter {
    delete [] m_log_file;
    if (filename != null) {
        m_log_file = new char [strlen (filename) + 1];
        strcpy (m_log_file, filename);
    }
    else
        m_log_file = null;
}

TK_Status BStreamFileToolkit::OpenLogFile (char const * filename, char const * mode) alter {
    m_log_fp = fopen (filename, mode);

    if (m_log_fp == null)
        return Error ("Log file open failure");

    return TK_Normal;
}

void BStreamFileToolkit::LogEntry (char const * string) alter {
    register    char const *    cp = string;
    register    char            cc;

    if (!m_logging)
        return;
    if (m_log_fp == null) {
        Error ("Log file not open");
        return;
    }

    while ((cc = *cp++) != '\0') {
        if (cc == '\n')
            m_log_line_length = 0;
        else
            m_log_line_length++;
    }

    fprintf (m_log_fp, "%s", string);
    fflush (m_log_fp);
}

void BStreamFileToolkit::LogEntry (unsigned short const * string) alter {
    register    unsigned short const *    cp = string;
    register    unsigned short            cc;

    if (!m_logging)
        return;
    if (m_log_fp == null) {
        Error ("Log file not open");
        return;
    }

    while ((cc = *cp++) != '\0') {
		if (cc < 256)
			fputc ((char)cc, m_log_fp);
		else
			fputc ('.', m_log_fp);
    }
	fprintf (m_log_fp, "\n");
    m_log_line_length = 0;
	cp = string;
    while ((cc = *cp++) != '\0') {
		if (cc == '\n') {
			fprintf (m_log_fp, "/\n");
            m_log_line_length = 0;
		}
		else {
			fprintf (m_log_fp, "/%04x", cc);
            m_log_line_length += 5;
		}
	}
	fprintf (m_log_fp, "/");
    m_log_line_length++;
    fflush (m_log_fp);
}

void BStreamFileToolkit::CloseLogFile (void) alter {
    fclose (m_log_fp);
    m_log_fp = null;
}


static void log_bar (BStreamFileToolkit * tk, char piece, int length) {
    char            bar[128];
    register    int             i;

    if (!tk->GetLogging())
        return;

    if (length > 80)
        length = 80;
    else if (length < 30)
        length = 30;

    for (i=0; i<length; ++i)
        bar[i] = piece;
    bar[length] = '\n';
    bar[length+1] = '\0';

    tk->LogEntry (bar);
}


#define VAL(x) (-(*(x))->m_priority)  
#define MEDIANOF3INDEX(a,b,c) (((a)>(b))?(((b)>(c))?(1):(((a)>(c))?(2):(0))):(((a)>(c))?(0):((b)>(c))?(2):(1)))
#define SWAP_REVISITS(a, b) {register Internal_Revisit_Item *tmp = (a); (a) = (b); (b) = tmp;}



bool BStreamFileToolkit::issorted_revisit (
    Internal_Revisit_Item  **ptr, 
    int                    count) 
{
    until (--count == 0) {
        if (VAL(ptr) > VAL(ptr + 1)) {
            return false;
        }
        else
            ++ptr;
    }
    return true;
}



void BStreamFileToolkit::qsort_revisit (
    Internal_Revisit_Item **ileft,
    Internal_Revisit_Item **iright) 
{
    if (iright - ileft == 1) {
        if (VAL(ileft) > VAL(iright))
           SWAP_REVISITS (*ileft, *iright);
    }
    else if (ileft < iright) {
        Internal_Revisit_Item **left, **right;
        Internal_Revisit_Item **candidates[3];
        float   pivot;
        int     m3i;

        candidates[0] = left = ileft;                       
        candidates[1] = ileft + ((iright - ileft) / 2);	    
        candidates[2] = right = iright;                     

        
        m3i = MEDIANOF3INDEX(
            VAL(candidates[0]), 
            VAL(candidates[1]),
            VAL(candidates[2]) 
       );
        pivot = VAL(candidates[m3i]);
        SWAP_REVISITS (*candidates[m3i], *right);

        
        
        

        for (;;) {
            left--;
            while (VAL(++left) <= pivot) {
                if (left >= right) 
                   goto partition_done;
            }
            SWAP_REVISITS (*left, *right);
            

            right++;
            while (VAL(--right) >= pivot) {
                if (left >= right) 
                   goto partition_done;
            }
            SWAP_REVISITS (*left, *right);
            
        }
        partition_done:

        qsort_revisit(ileft, left - 1);
        qsort_revisit(left + 1,  iright);
        ASSERT (issorted_revisit(ileft, iright - ileft));
    }

} 


TK_Status BStreamFileToolkit::sort_revisit () {
    Internal_Revisit_Item *ptr;
    Internal_Revisit_Item **array;
    int i = 0, count = 0;

    if (m_revisit == null)
        return TK_Normal;

    for (ptr = m_revisit; ptr != null; ptr = ptr->m_next) 
        count++;
    array = new Internal_Revisit_Item *[count];
    if (array == null)
        return Error ();
    for (ptr = m_revisit; ptr != null; ptr = ptr->m_next) {
        array[i++] = ptr;
    }
    qsort_revisit (array, array + count - 1);
    for (i = 0; i < count - 1; i++) {
        array[i]->m_next = array[i+1];
    }
    array[count - 1]->m_next = null;
    m_revisit = array[0];
    delete [] array;

    return TK_Normal;
    
} 





void BStreamFileToolkit::AddSegment (ID_Key key) alter {
    Internal_Segment_List *         newseg;

    newseg = new Internal_Segment_List (key);
    newseg->m_next = m_active_segments;
    m_active_segments = newseg;
}

ID_Key BStreamFileToolkit::RemoveSegment (void) alter {
    Internal_Segment_List *         oldseg;
    ID_Key                          old_key;

    if ((oldseg = m_active_segments) == null)
        return -1;

    m_active_segments = m_active_segments->m_next;
    old_key = oldseg->key();
    delete oldseg;

    return old_key;
}


void BStreamFileToolkit::ReportQuantizationError (
        int bits_per_sample, 
        float const *bounding, 
        int num_dimensions)
{
    int i;
    float range;
    
    for (i = 0; i < num_dimensions; i++) {
        range = bounding[i+num_dimensions] - bounding[i];
        if (range < 0)
            range = -range;
        ReportQuantizationError (range / bits_per_sample);
    }
}

void BStreamFileToolkit::ClearLastKey () alter {
    m_last_keys[0] = (ID_Key) -1;
    m_last_keys_used = 0;
}

TK_Status BStreamFileToolkit::AppendLastKey (ID_Key key) alter {
    if (m_last_keys_used >= m_last_keys_allocated) {
        
        ID_Key *temp;
        if (m_last_keys_allocated > 100)
            m_last_keys_allocated += 16;
        else
            m_last_keys_allocated *= 2;
        temp = new ID_Key[ m_last_keys_allocated ];
        if (temp == null)
            return Error ("allocation failure in BStreamFileToolkit::append_last_key");
        memcpy (temp, m_last_keys, m_last_keys_used * sizeof (ID_Key));
        delete [] m_last_keys;
        m_last_keys = temp;
    }
    m_last_keys[m_last_keys_used++] = key;
    return TK_Normal;
}


void BStreamFileToolkit::SetLastKey (ID_Key key) alter {
    m_last_keys[0] = key;
    m_last_keys_used = 1;
}

TK_Status BStreamFileToolkit::GetLastKey (ID_Key alter &key) const {
    if (m_last_keys_used == 1) {
        key = m_last_keys[ 0 ];
        return TK_Normal;
    }
    else {
        key = -1;
        return Error ("BStreamFileToolkit::GetLastKey should only be called when the number of keys is exactly 1");
    }
}


void BStreamFileToolkit::SetWorldBounding (float const *bbox) {
    if (!m_world_bounding) {
        m_world_bounding = new float[6];
    }
    memcpy (m_world_bounding, bbox, 6 * sizeof (float));
}


void BStreamFileToolkit::SetWorldBoundingBySphere (float const *pt, float radius) {
    float bbox[6];

    bbox[0] = pt[0] - radius;
    bbox[1] = pt[1] - radius;
    bbox[2] = pt[2] - radius;
    bbox[3] = pt[0] + radius;
    bbox[4] = pt[1] + radius;
    bbox[5] = pt[2] + radius;
    SetWorldBounding(bbox);
}


void BStreamFileToolkit::set_context_key (ID_Key key) alter {
    m_context_key = key;
}



void BStreamFileToolkit::remember_item (ID_Key key) alter {
    if (key == -1 || find_item (key))
        return;

    m_visited_items.add_key (key);
}

bool BStreamFileToolkit::find_item (ID_Key key) const {
    return  m_visited_items.find_key (key) == TK_Normal;
}





TK_Status BStreamFileToolkit::IndexToKey (int index, ID_Key alter & key) const { 
    return m_translator.index_to_key (index + m_index_base, key);
}


TK_Status BStreamFileToolkit::KeyToIndex (ID_Key key, int alter & index) const { 
    return m_translator.key_to_index (key, index); 
}


TK_Status BStreamFileToolkit::tag (int variant) alter {
    TK_Status           status = TK_Normal;
    int                 index;
    int                 i;

    for (i = 0; i < m_last_keys_used; i++) {
        status = KeyToIndex (m_last_keys[i], index);

        if (status == TK_NotFound) {
            BBaseOpcodeHandler *    tagop = opcode_handler (TKE_Tag);
            if ((status = tagop->Write (*this)) != TK_Normal)
                return status;

            tagop->Reset();

            
            AddIndexKeyPair (index = NextTagIndex(), m_last_keys[i]);
        }

        if (variant != -1) {
            
            AddVariant (m_last_keys[i], variant, m_position + m_offset);
        }
    }

    return TK_Normal;
}


TK_Status BStreamFileToolkit::revisit (unsigned char opcode, float priority, int lod) alter {
    Internal_Revisit_Item *     item = new Internal_Revisit_Item;

    if (m_last_keys_used != 1) {
        if (m_last_keys_used > 1)
            return Error ("illegal attempt to record compound item for revisiting");
        else
            return Error ("illegal attempt to record nonexistent item for revisiting");
    }
    item->m_key = m_last_keys[ 0 ];
    item->m_lod = lod;
    item->m_owner = m_context_key;
    item->m_priority = priority;
    item->m_opcode = opcode;

    item->m_next = m_revisit;
    m_revisit = item;

    return TK_Normal;
}


void BStreamFileToolkit::AddExternalReference (char const * ref, ID_Key context) alter {
    Internal_ExRef_List *   exref = new Internal_ExRef_List (ref, context);

    if (m_external_references == null) {
        m_external_references = m_external_ref_tail = exref;
    }
    else {
        m_external_ref_tail->m_next = exref;
        m_external_ref_tail = exref;
    }
}

bool BStreamFileToolkit::NextExternalReference (void) alter {
    Internal_ExRef_List *   exref;

    if ((exref = m_external_references) != null) {
        if ((m_external_references = exref->m_next) == null)
            m_external_ref_tail = null;
        delete exref;
    }

    return  m_external_references != null;
}



inline int ghash (int val1, int val2, int val3) {
    register    int     tmp;

    tmp = val1 ^ val2 ^ val3;
    return tmp & 0x000000FF;
}



void BStreamFileToolkit::record_instance (ID_Key key, int variant,
                                          BBaseOpcodeHandler const * object, int val1, int val2, int val3) alter {
    register    int                         index = ghash (val1, val2, val3);
    Recorded_Instance *         instance = new Recorded_Instance (key, variant, object->Opcode(),
                                                                              val1, val2, val3);

    instance->m_next = m_instance_hash[index];
    m_instance_hash[index] = instance;
}


bool BStreamFileToolkit::find_instance (BBaseOpcodeHandler * object, int val1, int val2, int val3) const {
    register    int                         index = ghash (val1, val2, val3);
    Recorded_Instance *         instance;

    if ((instance = m_instance_hash[index]) != null) do {
        if (instance->m_opcode == object->Opcode() &&
            instance->m_values[0] == val1 && instance->m_values[1] == val2 && instance->m_values[2] == val3) {
            if (object->Match_Instance (*this, instance)) {
#ifdef _DEBUG
                instance->m_times_used++;
#endif
                return true;
            }
        }
    } while ((instance = instance->m_next) != null);

    return false;
}


inline int hash_key (ID_Key key) { return (int)((key ^ (key >> 16)) & 1023); }


Internal_Translator::~Internal_Translator () {
    clean ();
}

void Internal_Translator::clean (void) alter {
    register    Hash_Block *        index_block;
    register    int                 i;

    for (i=0; i<1024; i++) {
        if ((index_block = m_blocks[i]) != null) do {
            m_blocks[i] = index_block->m_next;
            delete index_block;
        } until ((index_block = m_blocks[i]) == null);
    }

    for (i=0; i<m_size; i++)
        delete m_pairs[i].m_extra;
    delete [] m_pairs;
    m_pairs = 0;
    m_size = 0;
    m_used = 0;
}


TK_Status Internal_Translator::add_pair (int index, ID_Key key) alter {
    if (index >= m_size) {
        Index_Key_Pair *    old_pairs = m_pairs;
        int                 old_size = m_size;
        int                 i;

        m_size = index + 4096;
        m_pairs = new Index_Key_Pair [m_size];
        if (old_size > 0)
            memcpy (m_pairs, old_pairs, old_size * sizeof (Index_Key_Pair));
        for (i=old_size; i<m_size; ++i) {
            m_pairs[i].m_index = -1;
            m_pairs[i].m_key   = -1;
            m_pairs[i].m_extra = null;
        }

        delete [] old_pairs;
    }

    m_pairs[index].m_index = index;
    m_pairs[index].m_key = key;

    if (index >= m_used)
        m_used = index + 1;

    register    int                     hash = hash_key (key);
    register    Hash_Block *            index_block = m_blocks[hash];

    if (index_block == null || index_block->m_used == 32) {
        index_block = new Hash_Block;
        index_block->m_next = m_blocks[hash];
        m_blocks[hash] = index_block;
        index_block->m_used = 0;
    }
    index_block->m_indices[index_block->m_used++] = index;

    return TK_Normal;
}

TK_Status Internal_Translator::index_to_key (int index, ID_Key alter & key) const {
    ASSERT (index >= 0 && index < m_size);
    if (index >= m_size)
        return TK_NotFound;
    if (!m_pairs || 
		m_pairs[index].m_index != index)    
        return TK_NotFound;

    key = m_pairs[index].m_key;
    return TK_Normal;
}

TK_Status Internal_Translator::key_to_index (ID_Key key, int alter & index) const {
    register    Hash_Block *            block = m_blocks[hash_key (key)];

    if (block != null) do {
        register    int             i;

        for (i=0; i<block->m_used; i++) {
            register    int             test = block->m_indices[i];

            if (m_pairs[test].m_key == key) {
                if (m_pairs[test].m_index == test) {    
                    index = test;
                    return TK_Normal;
                }
            }
        }
    } until ((block = block->m_next) == null);

    return TK_NotFound;
}

TK_Status Internal_Translator::add_variant (ID_Key key, int variant, int value1, int value2) alter {
    TK_Status           status = TK_Normal;
    int                 index;

    if ((status = key_to_index (key, index)) != TK_Normal)
        return status;

    if (m_pairs[index].m_extra == null && (value1 >= 0 || value2 >= 0)) {
        m_pairs[index].m_extra = new IT_Index_Key_Extra;
        memset (m_pairs[index].m_extra, 0, sizeof (IT_Index_Key_Extra));
    }

    if (value1 >= 0 || value2 >= 0) {
        if (variant < 0)
            variant = 0;
        if (value1 >= 0)
            m_pairs[index].m_extra->m_variants[variant][0] = value1;
        if (value2 >= 0)
            m_pairs[index].m_extra->m_variants[variant][1] = value2;
    }
    return TK_Normal;
}

TK_Status Internal_Translator::add_bounds (ID_Key key, float const * bounds) alter {
    TK_Status           status = TK_Normal;
    int                 index, i;

    if ((status = key_to_index (key, index)) != TK_Normal)
        return status;

    if (m_pairs[index].m_extra == null && bounds != null) {
        m_pairs[index].m_extra = new IT_Index_Key_Extra;
        memset (m_pairs[index].m_extra, 0, sizeof (IT_Index_Key_Extra));
    }

    if (bounds != null) {
        m_pairs[index].m_extra->m_options |= Bounds_Valid;
        for (i=0; i<6; i++)
            m_pairs[index].m_extra->m_bounds[i] = bounds[i];
    }
    else
        m_pairs[index].m_extra->m_options &= ~Bounds_Valid;

    return TK_Normal;
}


TK_Status Internal_Translator::key_variant_offset (ID_Key key, int variant,
                                                   int alter & offset, int alter & length, int alter & index) const {
    TK_Status           status = TK_Normal;

    if ((status = key_to_index (key, index)) != TK_Normal)
        return status;

    if (m_pairs[index].m_extra != null) {
        offset = m_pairs[index].m_extra->m_variants[variant][0];
        length = m_pairs[index].m_extra->m_variants[variant][1];
    }
    else
        offset = length = 0;

    return TK_Normal;
}

TK_Status Internal_Translator::key_bounds (ID_Key key, float alter * bounds) const {
    TK_Status           status = TK_Normal;
    int                 index, i;

    if ((status = key_to_index (key, index)) != TK_Normal)
        return status;

    if (m_pairs[index].m_extra == null ||
        (m_pairs[index].m_extra->m_options & Bounds_Valid) == 0)
        return TK_NotFound;

    for (i=0; i<6; i++)
        bounds[i] = m_pairs[index].m_extra->m_bounds[i];

    return TK_Normal;
}


TK_Status BStreamFileToolkit::GetOffset (
    ID_Key                  key,
    int                     variant,
    int alter &             offset,
    int alter &             length,
    char const * alter &    filename) const {
    TK_Status               status = TK_Normal;
    int                     index;
    register    int                     i;

    if ((status = m_translator.key_variant_offset (key, variant, offset, length, index)) != TK_Normal) {
        filename = null;
        return status;
    }

    filename = null;
    if (m_file_names != null && m_file_count > 0) {
        filename = m_file_names[0];
        for (i = 1; i < m_file_count; ++i) {
            if (index < m_file_indices[i])
                break;
            filename = m_file_names[i];
        }
    }

    return TK_Normal;
}



Internal_Key_Record::~Internal_Key_Record () {
    clean ();
}

void Internal_Key_Record::clean (void) alter {
    register    Hash_Block *            block;
    register    int         i;

    for (i=0; i<1024; i++) {
        if ((block = m_blocks[i]) != null) do {
            m_blocks[i] = block->m_next;
            delete block;
        } until ((block = m_blocks[i]) == null);
    }
}


TK_Status Internal_Key_Record::add_key (ID_Key key) alter {
    register    int                     hash = hash_key (key);
    register    Hash_Block *            block = m_blocks[hash];

    if (block == null || block->m_used == 32) {
        block = new Hash_Block;
        block->m_next = m_blocks[hash];
        m_blocks[hash] = block;
        block->m_used = 0;
    }
    block->m_keys[block->m_used++] = key;

    return TK_Normal;
}


TK_Status Internal_Key_Record::find_key (ID_Key key) const {
    register    Hash_Block *            block = m_blocks[hash_key (key)];

    if (block != null) do {
        register    int                 i;

        for (i=0; i<block->m_used; i++) {
            if (block->m_keys[i] == key)
                return TK_Normal;
        }
    } until ((block = block->m_next) == null);

    return TK_NotFound;
}




TK_Status BStreamFileToolkit::PrepareBuffer (char * b, int s) alter {

    m_accumulator.set_data (b, s);

    return m_accumulator.consume();
}







TK_Status BStreamFileToolkit::Read_Stream_File () {
    char                    block[TK_DEFAULT_BUFFER_SIZE];
    int                     limit = TK_DEFAULT_BUFFER_SIZE;
    TK_Status               status = TK_Normal;
    bool                    version_mismatch = false;
    int                     flags;
    unsigned long           file_size = 0;
    unsigned long           processed = 0;
    TK_Progress_Callback    callback = 0;
    void *                  value = 0;
    bool                    first_file = true;
    ID_Key                  context = -1;
    unsigned short const *	wfilename = m_wfilename;
    char const *			filename = m_filename;
    int						version;

    if (GetLogging()) {
        char const *            logfile = GetLogFile();

        if (logfile == null)
            logfile = "hsf_import_log.txt";

        if (OpenLogFile (logfile, "w") != TK_Normal)
            return TK_Error;
    }

    limit = GetBufferLimit();
    flags = GetReadFlags();

    do {
        if (filename)
			SetNewFile (filename);
        if (context != -1)
            NewFileContext (context);


        if (GetLogging()) {
			if (filename) {
				LogEntry (filename);
				LogEntry ("\n");
				log_bar (this, '-', (int)strlen (filename));
			}
			else {
				LogEntry (wfilename);
				LogEntry ("\n");
			}
			SetOpcodeSequence();
        }

		if (wfilename != null) {
			if ((status = OpenFile (wfilename)) != TK_Normal)
				return status;
		}
		else if ((status = OpenFile (filename)) != TK_Normal)
            return status;

        if ((callback = GetProgressCallback()) != 0) {
            GetFileSize (file_size);
            value = GetProgressValue();
        }

        if (context != -1) {
            AddSegment (context);
            ActivateContext (context);
        }

        do {
            int                 amount;

            if (ReadBuffer (block, limit, amount) != TK_Normal)
                break;

			if ((version = ParseVersion (block)) != 0) {
				if (version > TK_File_Format_Version) {
					version_mismatch = true;
					flags |= TK_Ignore_Version;
					SetReadFlags (flags);
				}
			}

			if (version_mismatch) {
				try {
					status = ParseBuffer (block, amount);
					if (status == TK_Error) {
						status = TK_Version;
						break;
					}
				}
				catch (...) {
					status = TK_Version;
					break;
				}
			}
			else {
				status = ParseBuffer (block, amount);
				if (status == TK_Error)
					break;
			}

            if (callback) {
                processed += amount;

                if (!(*callback) (processed, file_size, value)) {
                    status = TK_Abort;
                    break;
                }
            }
        } while (status != TK_Complete);

        if (context != -1) {
            DeactivateContext (context);
            RemoveSegment ();
        }

        CloseFile ();

        if (status != TK_Complete)
            break;

        
        if (GetReadFlags(TK_Skip_External_References))
            break;

		wfilename = null;	

        if (filename == GetExternalReference()) {
            
            filename = null;
            if (!NextExternalReference())
                break;      
        }
        first_file = false;
        if ((filename = GetExternalReference()) != null) {
            if (GetLogging())
                log_bar (this, '=', 80);
        }
        context = GetExternalReferenceContext();
    } while (filename != null);

    if (GetLogging())
        CloseLogFile();

    SetReadFlags (flags);   

    return status;
}



TK_Status TK_Read_Stream_File (char const * filename, BStreamFileToolkit * tk) {
	if (tk == null)
		return TK_Read_Stream_File (filename);

	tk->Restart();
	tk->SetFilename (filename);

	return tk->Read_Stream_File ();
}

TK_Status TK_Read_Stream_File (char const * filename, int flags) {
    BStreamFileToolkit *    tk;
    TK_Status               status;

    tk = new BStreamFileToolkit;

	tk->SetFilename (filename);
    tk->SetReadFlags (flags);      

    status = tk->Read_Stream_File ();

    delete tk;

    return status;
}


TK_Status TK_Read_Stream_File (unsigned short const * filename, BStreamFileToolkit * tk) {
	if (tk == null)
		return TK_Read_Stream_File (filename);

	tk->Restart();
	tk->SetFilename (filename);

	return tk->Read_Stream_File ();
}

TK_Status TK_Read_Stream_File (unsigned short const * filename, int flags) {
	BStreamFileToolkit *    tk;
	TK_Status               status;

	tk = new BStreamFileToolkit;

	tk->SetFilename (filename);
	tk->SetReadFlags (flags);      

	status = tk->Read_Stream_File ();

	delete tk;

	return status;
}





TK_Status TK_Read_Stream_File (char const * filename, int flags, BStreamFileToolkit * tk) {
    if (tk != null)
        return TK_Read_Stream_File (filename, tk);
    else
        return TK_Read_Stream_File (filename, flags);
}
