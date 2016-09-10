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
// $Header: //DWF/Working_Area/Willie.Zhu/w3dtk/BOpcodeShell.cpp#1 $
//

#include "dwf/w3dtk/BStream.h"
#include "dwf/w3dtk/BOpcodeShell.h"
#include "dwf/w3dtk/BPolyhedron.h"
#include "dwf/w3dtk/BCompress.h"
#include "dwf/w3dtk/BPointCloud.h"
#include "dwf/w3dtk/BInternal.h"
#include <math.h>
#include <stdlib.h>

#include "hoops/hoops_tools.h"

#define ASSERT(x)


TK_Shell::TK_Shell ()
    : TK_Polyhedron (TKE_Shell) {

    _bOptimizeThisShell = false;
    _nOptimizeVertexTolerance = 0.0;
    _nOptimizeVertexNormalsTolerance  = 0.0;
    _nOptimizeTextureParametersTolerance = 0.0;

    m_flist = null;
    Reset();
}



TK_Shell::~TK_Shell() {
    delete [] m_flist;
}

void
TK_Shell::serialize( const void* pTag )
throw( DWFException )
{
#ifndef  BSTREAM_READONLY
        //
        // perform vertex & face optimization
        // using the HOOPS utility library
        //
    if (_bOptimizeThisShell)
    {
        _optimizeThisShell();
    }
#endif

    //
    // optimize the vertex normals
    // this function will discard vertex normals that
    // are the same as what HOOPS would calculate on its own
    //
    //_optimizeVertexNormals();

    //
    // base impl
    //
    TK_Polyhedron::serialize( pTag );
}

void
TK_Shell::_optimizeThisShell()
throw( DWFException )
{
#ifndef BSTREAM_READONLY

    //
    // allocate storage for optimized vertices
    //
    float* anNewPoints = new float[3*mp_pointcount];
    if (anNewPoints == null)
    {
        _DWFCORE_THROW( DWFMemoryException, L"Failed to allocate new point array" );
    }

    //
    // allocate storage for optimized face list
    //
    int* anNewFaceList = new int[m_flistlen];
    if (anNewFaceList == null)
    {
        delete [] anNewPoints;

        _DWFCORE_THROW( DWFMemoryException, L"Failed to allocate new face list" );
    }

    //
    // allocate storage for vertex mapping
    //
    int* anVertexMap = new int[mp_pointcount];
    if (anVertexMap == null)
    {
        delete [] anNewFaceList;
        delete [] anNewPoints;

        _DWFCORE_THROW( DWFMemoryException, L"Failed to allocate new vertex map" );
    }

    //
    // allocate storage for face mapping
    //
    int* anFaceMap = new int[mp_facecount];
    if (anFaceMap == null)
    {
        delete [] anVertexMap;
        delete [] anNewFaceList;
        delete [] anNewPoints;

        _DWFCORE_THROW( DWFMemoryException, L"Failed to allocate new vertex map" );
    }

    //
    // allocate storage for new normals (if applicable)
    //
    float* anNewNormals = null;
    if (mp_normals)
    {
        anNewNormals = new float[3*mp_pointcount];
        if (anNewNormals == null)
        {
            delete [] anFaceMap;
            delete [] anVertexMap;
            delete [] anNewFaceList;
            delete [] anNewPoints;

            _DWFCORE_THROW( DWFMemoryException, L"Failed to allocate new normals array" );
        }
    }

    //
    // allocate storage for new parameters (if applicable)
    //
    float* anNewParameters = null;
    if (mp_params)
    {
		//
		// check and assign for u,v or u,v,w
		//

        anNewParameters = new float[(int)mp_paramwidth * mp_pointcount];
        if (anNewParameters == null)
        {
            delete [] anNewNormals;
            delete [] anFaceMap;
            delete [] anVertexMap;
            delete [] anNewFaceList;
            delete [] anNewPoints;

            _DWFCORE_THROW( DWFMemoryException, L"Failed to allocate new texture parameters array" );
        }
    }

    //
    // the results
    //
    int nNewPoints = 0;
    int nNewFaces = 0;
    bool bVertex = false;

    //
    //
    //
    HU_OptimizeShell(  mp_pointcount,                       // number of shell vertices
                      (HT_Point*)mp_points,                 // shell vertex triplets
                      (HT_Vector*)mp_normals,               // vertex normal triplets
                      (mp_params == NULL) 
							? 0 : (int)mp_paramwidth,       // parameter width
                       mp_params,                           // vertex parameters
                       m_flistlen,                          // number of faces
                       m_flist,                             // shell face list
                       NULL,                                // collapsible function
                      _nOptimizeVertexTolerance,            // vertex tolerance
                      _nOptimizeVertexNormalsTolerance,     // vertex normal tolerance
                      _nOptimizeTextureParametersTolerance, // texture parameter tolerance
                       true,                                // eliminate orphan points
                      &nNewPoints,                          // number of new shell vertices
                      (HT_Point*)anNewPoints,               // new shell vertex triplets
                      &nNewFaces,                           // number of new faces
                       anNewFaceList,                       // new shell face list
                       anVertexMap,                         // map of original vertices to new or -1
                       anFaceMap );                         // map of original faces to new or -1

        //
        // shell vertices were optimized
        //
    if (nNewPoints < mp_pointcount)
    {
        //
        // if the vertex list changes, use the new face list
        //
        bVertex = true;

        int nOriginalPoints = mp_pointcount;

        //
        // reset the handler
        //
        delete [] mp_exists;
        mp_exists = null;

        //
        // set optimized vertices
        //
        delete [] mp_points;
        mp_points = null;
        mp_pointcount = 0;
        SetPoints( nNewPoints, anNewPoints );

            //
            // rejigger normals and parameters
            //
        if (anNewNormals || anNewParameters)
        {
            int iPoint = 0;
            int iVertex = 0;
            int iPointVertex = 0;
            int iNormal = 0;
                //
                // run the length of the vertex mapping
                //
            for (;iPoint < nOriginalPoints; iPoint++)
            {
                //
                // get the new vertex index
                //
                iVertex = anVertexMap[iPoint];

                    //
                    // was the vertex remapped?
                    //
                if (iVertex != -1)
                {

                    iNormal = iVertex * 3;

                    iPointVertex = 3*iPoint;

                        //
                        // map the old vertex normal (@ iPoint) to the new one (@ iVertex)
                        //
                    if (anNewNormals)
                    {
                        anNewNormals[iNormal] = mp_normals[iPointVertex];               // i
                        anNewNormals[iNormal + 1] = mp_normals[iPointVertex + 1];       // j
                        anNewNormals[iNormal + 2] = mp_normals[iPointVertex + 2];       // k
                    }

                        //
                        // map the old parameter coordinate (@ iPoint) to the new one (@ iVertex)
                        //
                    if (anNewParameters)
                    {
                        anNewParameters[iVertex * mp_paramwidth] = mp_params[(int)mp_paramwidth*iPoint];             // u
                        anNewParameters[iVertex * mp_paramwidth + 1] = mp_params[(int)mp_paramwidth*iPoint + 1];     // v
						
						//
						// Assign w only if it was passed in
						//
                        
						if((int)mp_paramwidth == 3)
							anNewParameters[iVertex * mp_paramwidth + 2] = mp_params[(int)mp_paramwidth*iPoint + 2];     // w
                    }
                }
            }

                //
                // set new normals
                //
            if (anNewNormals)
            {
                delete [] mp_normals;
                mp_normals = 0;
                SetVertexNormals( anNewNormals );
            }

                //
                // set new params
                //
            if (anNewParameters)
            {
                delete [] mp_params;
                mp_params = 0;
                SetVertexParameters( anNewParameters );
            }
        }

            //
            // readjust vertex colors
            //
		if(HasVertexColors())
        {
            int iPoint = 0;
            int iVertex = 0;
            int iPointVertex = 0;
            int iColor = 0;

			float* anVertexFaceColors = null;
			float* anVertexEdgeColors = null;
			float* anVertexMarkerColors = null;

			if(HasVertexFaceColors())
			{
				anVertexFaceColors = new float[mp_pointcount * 3];
			}

			if(HasVertexEdgeColors())
			{
				anVertexEdgeColors = new float[mp_pointcount * 3];
			}

			if(HasVertexMarkerColors())
			{
				anVertexMarkerColors = new float[mp_pointcount * 3];
			}		
	
				//
				// run the length of the vertex mapping
				//
			for (;iPoint < nOriginalPoints; iPoint++)
			{
				//
				// get the new vertex index
				//
				iVertex = anVertexMap[iPoint];

					//
					// was the vertex remapped?
					//
				if (iVertex != -1)
				{

					iColor = iVertex * 3;

					iPointVertex = 3*iPoint;

					//
					// map the old vertex color (@ iPoint) to the new one (@ iVertex)
					//
					if(anVertexFaceColors)
					{
						float* colors = GetVertexFaceColors();
						anVertexFaceColors[iColor] = colors[iPointVertex];               // i
						anVertexFaceColors[iColor + 1] = colors[iPointVertex + 1];       // j
						anVertexFaceColors[iColor + 2] = colors[iPointVertex + 2];       // k
					}

					if(anVertexEdgeColors)
					{
						float* colors = GetVertexEdgeColors();
						anVertexEdgeColors[iColor] = colors[iPointVertex];               // i
						anVertexEdgeColors[iColor + 1] = colors[iPointVertex + 1];       // j
						anVertexEdgeColors[iColor + 2] = colors[iPointVertex + 2];       // k
					}

					if(anVertexMarkerColors)
					{
						float* colors = GetVertexMarkerColors();
						anVertexMarkerColors[iColor] = colors[iPointVertex];               // i
						anVertexMarkerColors[iColor + 1] = colors[iPointVertex + 1];       // j
						anVertexMarkerColors[iColor + 2] = colors[iPointVertex + 2];       // k
					}
				}
			}	

			if(anVertexFaceColors)
			{
				//
				// set new vertex face colors
				//
				delete [] mp_vfcolors;
				mp_vfcolors = 0;
				SetVertexFaceColors( anVertexFaceColors );
				delete [] anVertexFaceColors;
			}
				
			if(anVertexEdgeColors)
			{
				//
				// set new vertex edge colors
				//
				delete [] mp_vecolors;
				mp_vecolors = 0;
				SetVertexEdgeColors( anVertexEdgeColors );	
				delete [] anVertexEdgeColors;
			}
			
			if(anVertexMarkerColors)
			{					
				//
				// set new vertex marker colors
				//			
				delete [] mp_vmcolors;
				mp_vmcolors = 0;
				SetVertexMarkerColors( anVertexMarkerColors );
				delete [] anVertexMarkerColors;
			}						
        }

        //
        // always delete first
        // (because we want this null if no faces left)
        //
        delete [] m_flist;
        m_flist = null;
        m_flistlen = 0;

        if (nNewFaces > 0)
        {
            SetFaces( nNewFaces, anNewFaceList );

                //
                // readjust the face colors
                //
            if (mp_fcolors)
            {
                int nOriginalFaces = mp_facecount;
                float* anNewFaceColors = new float[3*nNewFaces];
                if (anNewFaceColors == NULL)
                {
                    delete [] anNewNormals;
                    delete [] anFaceMap;
                    delete [] anVertexMap;
                    delete [] anNewFaceList;
                    delete [] anNewPoints;

                    _DWFCORE_THROW( DWFMemoryException, L"Failed to allocate array" );
                }

                int iNewFace = 0;
                int iOriginalFace = 0;
                int iNewFaceColor = 0;
                
                for (; iOriginalFace < nOriginalFaces; iOriginalFace++)
                {
                    //
                    // get the new face index
                    //
                    iNewFace = anFaceMap[iOriginalFace];

                        //
                        // was the face remapped?
                        //
                    if (iNewFace != -1)
                    {
                        anNewFaceColors[(iNewFaceColor * 3)] = mp_fcolors[(iOriginalFace * 3)];
                        anNewFaceColors[(iNewFaceColor * 3) + 1] = mp_fcolors[(iOriginalFace * 3) + 1];
                        anNewFaceColors[(iNewFaceColor * 3) + 2] = mp_fcolors[(iOriginalFace * 3) + 2];

                        iNewFaceColor++;
                    }
                }

                delete [] mp_fcolors;
                mp_fcolors = 0;
                SetFaceColors( anNewFaceColors );
                delete [] anNewFaceColors;
            }
        }
    }


    //
    // clean up
    //
    delete [] anNewParameters;
    delete [] anNewNormals;
    delete [] anFaceMap;
    delete [] anVertexMap;
    delete [] anNewFaceList;
    delete [] anNewPoints;
#endif
}

void
TK_Shell::_optimizeVertexNormals()
{
}

TK_Status TK_Shell::SetFaces (int length, int const *list) alter {
    int i;

    if (m_flist == null) {
        m_flistlen = length;
        m_flist = new int [m_flistlen];
    }
    if (m_flist == null)
        return TK_Error;
    if (list != null) {
        memcpy (m_flist, list, m_flistlen*sizeof(int));
        /* now calculate a face count */
        if (mp_subop & TKSH_TRISTRIPS) {
            mp_facecount = 0;
            i = 0;
            while (i < m_flistlen) {
                mp_facecount += m_flist[i] - 2;
                if (m_flist[i] > 0)
		    // a triangle strip
		    i += m_flist[i] + 1;
		else
		    // a triangle fan
                    i += -m_flist[i] + 1;
            }
        }
        else {
            mp_facecount = 0;
            i = 0;
            while (i < m_flistlen) {
                if (m_flist[i] > 0) {
                    mp_facecount++;
                    i += m_flist[i] + 1;
                }
                else {
                    //a continuation (to define a hole in) the previous face
                    i += -m_flist[i] + 1;
                }
            }
        }
    }
    return TK_Normal;
}

#ifdef _MSC_VER
  typedef int (__cdecl *qsortfunc)(const void *, const void *);
#else
  typedef int (*qsortfunc)(const void *, const void *);
#endif


static int edge_enum_compare(const int *enum1, const int *enum2) 
{
    if (enum1[0] == enum2[0]) {
        if (enum1[1] > enum2[1])
            return 1;
        else
            return -1;
    }
    else if (enum1[0] > enum2[0]) 
        return 1;
    else 
        return -1;
}


TK_Status TK_Shell::EnumerateEdges () alter 
{
    int i, len, enumcount, temp;
    int *fptr, *end, *enumptr;

    if (m_flistlen == 0)
        return TK_Error;
    fptr = m_flist;
    end = fptr + m_flistlen;
    enumcount = 0; 
    mp_edge_enumeration = new int[ m_flistlen * 4 ]; 
    if (mp_subop & TKSH_TRISTRIPS) {
        while (fptr < end) {
            if (fptr[0] > 0) {
                len = fptr[0]; 
                for (i = 3; i <= len; i++) {
                    mp_edge_enumeration[ enumcount++ ] = fptr[i-2];
                    mp_edge_enumeration[ enumcount++ ] = fptr[i-1];
                    mp_edge_enumeration[ enumcount++ ] = fptr[i-2];
                    mp_edge_enumeration[ enumcount++ ] = fptr[i];
                }
                mp_edge_enumeration[ enumcount++ ] = fptr[len-1];
                mp_edge_enumeration[ enumcount++ ] = fptr[len];
            }
            else {
                len = -fptr[0]; 
                for (i = 3; i <= len; i++) {
                    mp_edge_enumeration[ enumcount++ ] = fptr[i-1];
                    mp_edge_enumeration[ enumcount++ ] = fptr[i];
                    mp_edge_enumeration[ enumcount++ ] = fptr[i-1];
                    mp_edge_enumeration[ enumcount++ ] = fptr[1];
                }
                mp_edge_enumeration[ enumcount++ ] = fptr[len];
                mp_edge_enumeration[ enumcount++ ] = fptr[1];
            }
            fptr += len + 1;
        }
    }
    else {
        while (fptr < end) {
            if (fptr[0] > 0)
                len = fptr[0]; 
            else
                len = -fptr[0]; 
            for (i = 1; i <= len; i++) {
                if (i == 1)
                    mp_edge_enumeration[ enumcount++ ] = fptr[len];
                else
                    mp_edge_enumeration[ enumcount++ ] = fptr[i-1];
                mp_edge_enumeration[ enumcount++ ] = fptr[i];
            }
            fptr += len + 1;
        }
    }
    
    for (i = 0; i < enumcount; i += 2) {
        if (mp_edge_enumeration[i] > mp_edge_enumeration[i+1]) {
            temp = mp_edge_enumeration[i];
            mp_edge_enumeration[i] = mp_edge_enumeration[i+1];
            mp_edge_enumeration[i+1] = temp;
        }
    }
    
    qsort(mp_edge_enumeration, enumcount/2, 2*sizeof(int), (qsortfunc) edge_enum_compare);
    
    enumptr = mp_edge_enumeration + 2;
    for (i = 2; i < enumcount; i += 2) {
        if (enumptr[-2] == mp_edge_enumeration[i] &&
            enumptr[-1] == mp_edge_enumeration[i+1])
            continue;
        *enumptr++ = mp_edge_enumeration[i];
        *enumptr++ = mp_edge_enumeration[i+1];
    }
    mp_edgecount = (int)((enumptr - mp_edge_enumeration) / 2);
    return TK_Normal;
}


#ifndef BSTREAM_READONLY




static TK_Status trivial_compress_faces (
        BStreamFileToolkit &tk,
        int flistlen,
        int const *flist, 
        int has_negative_faces,
        int *workspace_len_in_out,
        int *workspace_used_out,
        unsigned char **workspace_in_out)
{
    int i, max, len;
    int bits_per_sample, bytes_per_sample;
    unsigned char *out = null;
    int const *iptr = null, *iptrend = null;

    max = 0;
    for (i = 0; i < flistlen; i++)
        if (flist[i] > max)
            max = flist[i];

    if (tk.GetTargetVersion() >= 650 && has_negative_faces) {
        if (max > 32767)
            bits_per_sample = 32;
        else if (max > 127)
            bits_per_sample = 16;
        else 
            bits_per_sample = 8;
    }
    else {
        if (max > 65535)
            bits_per_sample = 32;
        else if (max > 255)
            bits_per_sample = 16;
        else 
            bits_per_sample = 8;
    }

    bytes_per_sample = bits_per_sample / 8;
    len = (flistlen * bytes_per_sample) + 1;
    if (len > *workspace_len_in_out) {
        *workspace_len_in_out = len; 
        ENSURE_ALLOCATED(*workspace_in_out, unsigned char, len);
    }
    if (workspace_used_out != null)
        *workspace_used_out = len;
    out = *workspace_in_out;

    *out++ = (unsigned char)bits_per_sample;

    iptrend = flist + flistlen;
    switch (bytes_per_sample) {
        case 1:
            for (iptr = flist; iptr < iptrend; iptr++) {
                *out++ = (unsigned char)(*iptr);
            }
            break;

        case 2:
            for (iptr = flist; iptr < iptrend; iptr++) {
                *out++ = (unsigned char)(*iptr & 0xff);
                *out++ = (unsigned char)(*iptr >> 8);
            }
            break;

        case 4:
            for (iptr = flist; iptr < iptrend; iptr++) {
                *out++ = (unsigned char)*iptr;
                *out++ = (unsigned char)(*iptr >> 8);
                *out++ = (unsigned char)(*iptr >> 16);
                *out++ = (unsigned char)(*iptr >> 24);
            }
            break;

        default:
            return TK_Error;

    }
    return TK_Normal;
    /****
#else
    UNREFERENCED(workspace_in_out);
    UNREFERENCED(workspace_used_out);
    UNREFERENCED(workspace_len_in_out);
    UNREFERENCED(has_negative_faces);
    UNREFERENCED(flist);
    UNREFERENCED(flistlen);
    UNREFERENCED(trivial_compress_faces);
    return tk.Error (stream_readonly);
#endif
    ****/
} 
#endif



static TK_Status trivial_decompress_faces (
        BStreamFileToolkit &tk,
        int buflen,
        void const *buf_in,
        int has_negative_faces,
        int *flistlen_out,
        int **flist_out)
{
    unsigned char *cptr = null, *cptrend = null;
    int *iptr = null;
    int bits_per_sample, bytes_per_sample;
    char sbyte;
    short sshort;

    cptr = (unsigned char *) buf_in;
    cptrend = cptr + buflen;

    bits_per_sample = *cptr++;
	if(bits_per_sample<8)
		return tk.Error("Invalid bits_per_sample detected in trivial_decompress_faces.");
    bytes_per_sample = bits_per_sample / 8;
    *flistlen_out = (buflen - 1) / bytes_per_sample;
    ENSURE_ALLOCATED(iptr, int, *flistlen_out);
    *flist_out = iptr;

    
    if (tk.GetVersion() >= 650 && has_negative_faces) {
        switch (bytes_per_sample) {
            case 1:
                for (; cptr < cptrend; cptr += 1) {
                    sbyte = cptr[0];
                    *iptr = sbyte;
                    iptr++;
                }
                break;

            case 2:
                for (; cptr < cptrend; cptr += 2) {
                    sshort = (short)(cptr[0] | (cptr[1] << 8));
                    *iptr = sshort;
                    iptr++;
                }
                break;

            case 4:
                for (; cptr < cptrend; cptr += 4) {
                    *iptr = (cptr[0]) | 
                            (cptr[1] << 8) |
                            (cptr[2] << 16) |
                            (cptr[3] << 24);
                    iptr++;
                }
                break;

            default:
                return TK_Error;

        }
    }
    else {
        switch (bytes_per_sample) {
            case 1:
                for (; cptr < cptrend; cptr += 1) {
                    *iptr = cptr[0];
                    iptr++;
                }
                break;

            case 2:
                for (; cptr < cptrend; cptr += 2) {
                    *iptr = cptr[0];
                    *iptr += cptr[1] << 8;
                    iptr++;
                }
                break;

            case 4:
                for (; cptr < cptrend; cptr += 4) {
                    *iptr = cptr[0];
                    *iptr += cptr[1] << 8;
                    *iptr += cptr[2] << 16;
                    *iptr += cptr[3] << 24;
                    iptr++;
                }
                break;

            default:
                return TK_Error;

        }
    }
    return TK_Normal;

} 



TK_Status TK_Shell::write_uncompressed_points (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_READONLY
    TK_Status       status = TK_Normal;

#ifndef BSTREAM_DISABLE_ASCII
	if( tk.GetAsciiMode() ) 
		return write_uncompressed_points_ascii (tk);
#endif

    switch (m_substage) {
        case 0: {
            if ((status = PutData(tk, mp_pointcount)) != TK_Normal)
                return status;
            m_substage++;
        }   nobreak;
    
        case 1: {
            if (mp_pointcount != 0)
                if ((status = PutData(tk, mp_points, 3*mp_pointcount)) != TK_Normal)
                    return status;
            m_substage = 0;
        } break;

        default:
            return tk.Error();
    }
    return status;
#else
    return tk.Error (stream_readonly);
#endif
} 




TK_Status TK_Shell::write_uncompressed_faces (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_READONLY
    TK_Status       status = TK_Normal;

#ifndef BSTREAM_DISABLE_ASCII
	if( tk.GetAsciiMode() ) 
		return write_uncompressed_faces_ascii(tk);
#endif

    switch (m_substage) {
        
        case 0: {
            int has_negative_faces = mp_subop2 & TKSH2_HAS_NEGATIVE_FACES;
            status = trivial_compress_faces(
                                tk, 
                                m_flistlen,
                                m_flist, 
                                has_negative_faces,
                                &mp_workspace_allocated, 
                                &mp_workspace_used, 
                                &mp_workspace);
            if (status != TK_Normal)
                return status;
            m_substage++;
        } nobreak;

        
        case 1: {
            mp_compression_scheme = CS_TRIVIAL;
            if ((status = PutData(tk, mp_compression_scheme)) != TK_Normal)
                return status;
            m_substage++;
        }   nobreak;

        
        case 2: {
            if ((status = PutData(tk, mp_workspace_used)) != TK_Normal)
                return status;
            m_substage++;
        }   nobreak;

        
        case 3: {
            if ((status = PutData(tk, mp_workspace, mp_workspace_used)) != TK_Normal)
                return status;

            m_substage = 0;
        } break;

        default:
            return tk.Error();
    }

    return status;
#else
    return tk.Error (stream_readonly);
#endif
} 









TK_Status TK_Shell::write_bounding (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_READONLY
    TK_Status       status = TK_Normal;

#ifndef BSTREAM_DISABLE_ASCII
	if( tk.GetAsciiMode() ) 
		return write_bounding_ascii (tk);
#endif

    switch (m_substage) {
        
        case 0: {  
            if ((status = PutOpcode (tk, 0)) != TK_Normal)
                return status;
            m_substage++;
        }   nobreak;

        
        case 1: {  
            ASSERT(!(mp_subop & TKSH_EXPANDED));
            if ((status = PutData(tk, mp_subop)) != TK_Normal)
                return status;
            m_substage++;
        }   nobreak;

        
        case 2: {
            if ((status = PutData(tk, m_lodlevel)) != TK_Normal)
                return status;
            m_substage++;
        }   nobreak;

        
        case 3: {
            if ((status = PutData(tk, mp_bbox, 6)) != TK_Normal)
                return status;
        } nobreak;

        
        case 4: {
            if ((status = Tag(tk, -1)) != TK_Normal)
                return status;
            m_substage = 0;
        }   break;

        default:
            return tk.Error();
    }

    return status;
#else
    return tk.Error (stream_readonly);
#endif
} 





TK_Status TK_Shell::write_null (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_READONLY
    TK_Status       status = TK_Normal;

#ifndef BSTREAM_DISABLE_ASCII
	if( tk.GetAsciiMode() ) 
		return write_null_ascii (tk);
#endif
    switch (m_substage) {
        
        case 0: {  
            if ((status = PutOpcode (tk, 0)) != TK_Normal)
                return status;
            m_substage++;
        }   nobreak;

        
        case 1: {
            
            mp_subop &= ~(TKSH_BOUNDING_ONLY | 
                TKSH_COMPRESSED_POINTS | 
                TKSH_COMPRESSED_FACES |
                TKSH_HAS_OPTIONALS |
                TKSH_TRISTRIPS |
                TKSH_BOUNDING_ONLY |
                TKSH_CONNECTIVITY_COMPRESSION); 
            if ((status = PutData(tk, mp_subop)) != TK_Normal)
                return status;
            m_substage++;
        }   nobreak;

        
        case 2: {  
            mp_subop &= ~(
                TKSH2_COLLECTION |          
                TKSH2_HAS_NEGATIVE_FACES |
                TKSH2_GLOBAL_QUANTIZATION);
            if ((status = PutData(tk, mp_subop2)) != TK_Normal)
                return status;
            m_substage++;
        }   nobreak;

        
        case 3: {
            if ((status = PutData(tk, m_lodlevel)) != TK_Normal)
                return status;
            m_substage++;
        }   nobreak;

        
        case 4: {
            if ((status = Tag(tk, -1)) != TK_Normal)
                return status;
            m_substage = 0;
        }   break;

        default:
            return tk.Error();
    }

    return status;
#else
    return tk.Error (stream_readonly);
#endif
} 



TK_Status TK_Shell::compute_advanced(BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_READONLY
#ifndef BSTREAM_DISABLE_ASCII
	if( tk.GetAsciiMode() ) 
		return compute_advanced_ascii(tk);
#endif

#ifndef BSTREAM_DISABLE_EDGEBREAKER
    int required, success, pointmap_count;
    int *pointmap = null;
    float *normals = null;
    float *points = mp_points;
    int flags = tk.GetWriteFlags();
    int num_normal_bits;
    const float *global_bbox;
    TK_Status status = TK_Normal;
    int i;

    eb_compress_configs configs = {
        EB_DEFAULT, EB_DEFAULT, EB_DEFAULT, 
        EB_DEFAULT, EB_DEFAULT, EB_DEFAULT, 
        EB_DEFAULT,                         
        null,                               
        0,                                  
        null, null, null,                   
        null,                               
        0                                   
    };

    configs.target_version = tk.GetTargetVersion();
    
    
    if ((configs.target_version < 651) &&
        (!(mp_subop & TKSH_COMPRESSED_POINTS)))
        return TK_Abort;  
    
    if (m_flistlen < 150)
        return TK_Abort;
    ASSERT(mp_compression_scheme == CS_EDGEBREAKER);
    
    if (has_face_attributes())
        return TK_Abort;

    
    if (mp_subop & TKSH_TRISTRIPS)
        configs.hints |= HINT_INPUT_BY_TRISTRIPS;
    if (!(mp_subop & TKSH_COMPRESSED_POINTS)) {
        points = null;
    } else {
        configs.x_quantization = tk.GetNumVertexBits() / 3;
        configs.y_quantization = tk.GetNumVertexBits() / 3;
        configs.z_quantization = tk.GetNumVertexBits() / 3;
    }
    if (configs.target_version >= 651) {
        num_normal_bits = tk.GetNumNormalBits();
        if (flags & TK_Full_Resolution_Normals) {
            normals = null; 
        }
        else if (mp_normalcount != mp_pointcount) {
            normals = null; 
        }
        #ifndef BSTREAM_DISABLE_REPULSE_COMPRESSION
        else if (num_normal_bits == 8 || num_normal_bits == 10) {
            normals = null; 
        }
        #endif
        else if ((configs.target_version < 1000) && num_normal_bits > 48) {
            normals = null; 
        }
        else {
            configs.x_quantization_normals = tk.GetNumNormalBits() / 3;
            configs.y_quantization_normals = tk.GetNumNormalBits() / 3;
            configs.z_quantization_normals = tk.GetNumNormalBits() / 3;
            normals = mp_normals;
        }
    }
    else {
        
        
        normals = null; 
    }
    if (mp_subop2 & TKSH2_GLOBAL_QUANTIZATION) {
        global_bbox = tk.GetWorldBounding();
        
        ASSERT(configs.target_version >= 806);
        ASSERT(global_bbox);
        configs.bounding = (ET_Bounding *)global_bbox;
    }
    success = show_edgebreaker_compress_size(
                mp_pointcount, m_flistlen, m_flist, 
                &required, &pointmap_count, &configs);
    if (!success) {
        status = TK_Abort;
        goto done;
    }

    ASSERT(mp_workspace == null);
    if (mp_workspace_allocated < required) {
        mp_workspace_allocated = required;
        ENSURE_ALLOCATED(mp_workspace, unsigned char, required);
    }
    mp_workspace_used = required;
    ENSURE_ALLOCATED(pointmap, int, pointmap_count);
    success = edgebreaker_compress(
                    mp_pointcount, points, normals,
                    m_flistlen, m_flist, 
                    &mp_workspace_used, mp_workspace, 
                    &pointmap_count, pointmap, &configs);
    if (!success) {
        mp_workspace_used = 0;
        status = TK_Abort;
        goto done;
    }
    else {
	if (normals != null) {
	    delete [] mp_normals;
	    mp_normals = null;
	    mp_normalcount = 0;
	    for (i = 0; i < mp_pointcount; i++) {
		mp_exists[i] &= ~Vertex_Normal;
	    }
	}
        mangle_points(tk, pointmap_count, pointmap);
        mangle_point_attributes(tk, pointmap_count, pointmap);
        
        
        mp_subop &= ~TKSH_TRISTRIPS;
    }
  done:
    delete [] pointmap;
    pointmap = null;
    return status;
#else
    return tk.Error (stream_disable_edgebreaker);
#endif
#else
    return tk.Error (stream_readonly);
#endif
} 


TK_Status TK_Shell::write_advanced(BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_READONLY
    TK_Status       status = TK_Normal;

#ifndef BSTREAM_DISABLE_ASCII
	if( tk.GetAsciiMode() ) 
		return write_advanced_ascii (tk);
#endif

    switch (m_substage) {

        case 0: {
            if ((status = PutData(tk, mp_compression_scheme)) != TK_Normal)
                return status;
            m_substage++;
        } nobreak;

        
        case 1: {
            if ((status = PutData(tk, mp_workspace_used)) != TK_Normal)
                return status;
            m_progress = 0;
            m_substage++;
        } nobreak;

        
        case 2: {
            if ((status = PutData(tk, mp_workspace, mp_workspace_used)) != TK_Normal)
                return status;

            m_progress = 0;
            
            if ((mp_subop & TKSH_COMPRESSED_POINTS) ||
                (tk.GetTargetVersion() < 651)) {
                m_substage = 0;
                return status;
            }
            else
                m_substage++;
        } nobreak;

        case 3: {
            if (mp_pointcount != 0)
                if ((status = PutData(tk, mp_points, 3*mp_pointcount)) != TK_Normal)
                    return status;
            m_substage = 0;
        } break;

        default:
            return tk.Error();
    }
    return status;
#else
    return tk.Error (stream_readonly);
#endif
} 



TK_Status TK_Shell::Write (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_READONLY
    TK_Status       status = TK_Normal;

    int             index;
    int             *ptr;

#ifndef BSTREAM_DISABLE_ASCII
	if( tk.GetAsciiMode() ) 
		return WriteAscii (tk);
#endif

    
    switch (m_stage) {
        
        case 0: {
            if ((status = tk.KeyToIndex(mp_key, index)) == TK_NotFound)
                mp_subop |= TKSH_FIRSTPASS;
            else
                mp_subop2 &= ~TKSH2_NULL;
            

            if (mp_instance != null)
                return mp_instance->Write (tk);

            if (mp_subop2 & TKSH2_NULL)
                return write_null(tk);

            if (mp_subop & TKSH_BOUNDING_ONLY)
                return write_bounding(tk);

            if (HasOptionals())
                mp_subop |= TKSH_HAS_OPTIONALS;

            if (mp_pointcount == 0)
                mp_subop &= ~(TKSH_CONNECTIVITY_COMPRESSION|TKSH_COMPRESSED_POINTS);

            
            
            if (mp_subop & TKSH_CONNECTIVITY_COMPRESSION) {
                mp_compression_scheme = CS_EDGEBREAKER;
                if (compute_advanced(tk) != TK_Normal) {
                    if (mp_subop & TKSH_COMPRESSED_POINTS)
                        mp_compression_scheme = CS_TRIVIAL;
                    else
                        mp_compression_scheme = CS_NONE;
                    mp_subop &= ~TKSH_CONNECTIVITY_COMPRESSION;
                }
            }
            else if (mp_subop & TKSH_COMPRESSED_POINTS)
                mp_compression_scheme = CS_TRIVIAL;
            else
                mp_compression_scheme = CS_NONE;

            if (mp_subop2 & TKSH2_GLOBAL_QUANTIZATION) {
                if (tk.GetTargetVersion() < 806 ||
                    !tk.GetWorldBounding()) {
                    mp_subop2 &= ~TKSH2_GLOBAL_QUANTIZATION;
                    if (mp_subop2 == 0)
                        mp_subop &= ~TKSH_EXPANDED;
                }
            }

            if (m_flist != null) {
                ptr = m_flist; 
                while (ptr < m_flist + m_flistlen) {
                    if (*ptr < 0) {
                        mp_subop2 |= TKSH2_HAS_NEGATIVE_FACES;
                        break;
                    }
                    ptr += *ptr + 1;
                }
            }

            if (mp_subop2 != 0)
                mp_subop |= TKSH_EXPANDED;
            m_stage++;    
        }

        
        case 1: {
            int     count_it = 0;

            
            
            if (m_lodlevel == 0 && (GetPointCount() != 0 || Pass (tk) != 0))
                count_it = 1;

            if ((status = PutOpcode (tk, count_it)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        
        case 2: {
            if ((status = PutData(tk, mp_subop)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        
        case 3: {
            if (mp_subop & TKSH_EXPANDED) {
                if ((status = PutData(tk, mp_subop2)) != TK_Normal)
                    return status;
            }
            m_stage++;
        }   nobreak;

        
        
        case 4: {
            if (!(mp_subop & TKSH_FIRSTPASS)) {
                if ((status = tk.KeyToIndex(mp_key, index)) != TK_Normal)
                    return tk.Error();
                if ((status = PutData(tk, index)) != TK_Normal)
                    return status;
            }
            m_stage++;
        } nobreak;

        
        case 5: {
            if ((status = PutData(tk, m_lodlevel)) != TK_Normal)
                    return status;
            m_stage++;
        } nobreak;

        case 6: {
            if (mp_subop2 & TKSH2_COLLECTION) {
                return write_collection (tk, true, ((mp_subop & TKSH_FIRSTPASS) ? -1 : m_lodlevel));
            }
            else switch (mp_compression_scheme) {
                case CS_EDGEBREAKER:
                    if ((status = write_advanced(tk)) != TK_Normal)
                        return status;
                    break;
                case CS_TRIVIAL:
                    if ((status = write_trivial_points(tk)) != TK_Normal)
                        return status;
                    break;
                case CS_NONE:
                    if ((status = write_uncompressed_points(tk)) != TK_Normal)
                        return status;
                    break;
                default:
                    return tk.Error("internal error in TK_Shell::write");
            }
            m_stage++;
        } nobreak;

        case 7: {
            switch (mp_compression_scheme) {
                case CS_EDGEBREAKER:
                    
                    break;      
                case CS_TRIVIAL:
                case CS_NONE:
                    if ((status = write_uncompressed_faces(tk)) != TK_Normal)
                        return status;
                    break;
                default:
                    return tk.Error("internal error(2) in TK_Shell::write");
            }
            m_stage++;
        } nobreak;

        
        
        case 8: {
            if (mp_subop & TKSH_HAS_OPTIONALS) {
                if ((status = TK_Polyhedron::Write(tk)) != TK_Normal)
                    return status;
            }
            m_stage++;
        }   nobreak;

        
        case 9: {
            int level = ((mp_subop & TKSH_FIRSTPASS) ? -1 : m_lodlevel);
            if ((status = Tag(tk, level)) != TK_Normal)
                return status;
            if (AllowInstancing(tk))
                Record_Instance (tk, mp_key, level,
                         mp_pointcount, m_flistlen, mp_present);


            if (tk.GetLogging()) {
                char            buffer[64];

                if (tk.GetLoggingOptions (TK_Logging_Tagging) &&
                    (mp_subop & TKSH_FIRSTPASS) == 0) {
                    if ((status = tk.KeyToIndex(mp_key, index)) != TK_Normal)
						sprintf (buffer, "[error: not first pass, but couldn't find key]");
					else
						sprintf (buffer, "[%d:%d]", index, m_lodlevel);
                    tk.LogEntry (buffer);
                }
            }
            m_stage = -1;
        }   break;


        default:
            return tk.Error();
    }
    return status;
#else
    return tk.Error (stream_readonly);
#endif
} 



TK_Status TK_Shell::read_advanced (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_EDGEBREAKER

#ifndef BSTREAM_DISABLE_ASCII
	if( tk.GetAsciiMode() ) 
		return read_advanced_ascii (tk);
#endif
    TK_Status       status = TK_Normal;
    int             success = 1;
    const float     *global_bbox;
    bool            by_tristrips = false;
    eb_decompress_configs configs = {
        null, null, null, null, null
    };

    switch (m_substage) {
        
        case 0: {
            if ((status = GetData(tk, mp_workspace_used)) != TK_Normal)
                return status;
            if (mp_workspace_allocated < mp_workspace_used) {
                mp_workspace_allocated = mp_workspace_used;
                ENSURE_ALLOCATED(mp_workspace, unsigned char, mp_workspace_allocated);
            }
            m_substage++;
        } nobreak;

        case 1: {

            
            if ((status = GetData(tk, mp_workspace, mp_workspace_used)) != TK_Normal)
                return status;

            if (mp_subop2 & TKSH2_GLOBAL_QUANTIZATION) {
                global_bbox = tk.GetWorldBounding();
                
                ASSERT(configs.target_version >= 806);
                ASSERT(global_bbox);
                configs.bounding = (ET_Bounding *)global_bbox;
            }

            
            show_edgebreaker_decompress_size(mp_workspace_used, mp_workspace, 
                    &mp_pointcount, &mp_normalcount, &m_flistlen);
            if (mp_pointcount != 0)
                SetPoints(mp_pointcount, null);
            if (m_flistlen != 0)
                SetFaces(m_flistlen, null);
            if (mp_normalcount != 0)
                SetVertexNormals(null);
            success = edgebreaker_decompress(mp_workspace_used, mp_workspace, 
                    &mp_pointcount, mp_points, mp_normals,
                    &by_tristrips, &m_flistlen, m_flist, &configs);
            if (!success)
                return tk.Error("edgebreaker read failed, called from TK_Shell::read_advanced");

            if (by_tristrips)
                mp_subop |= TKSH_TRISTRIPS;

            
            if ((mp_subop & TKSH_COMPRESSED_POINTS) ||
                (tk.GetVersion() < 651)) {
                m_substage = 0;
                return status;
            }
            else
                m_substage++;
        }   nobreak;

        case 2: {
            if ((status = GetData(tk, mp_points, 3 * mp_pointcount)) != TK_Normal)
                return status;
            m_substage = 0; 
        }   break;  

        default:
            return tk.Error("internal error: unrecognized case in TK_Shell::read_advanced");
    }
    return status;
#else
    return tk.Error (stream_disable_edgebreaker);
#endif

} 





TK_Status TK_Shell::read_uncompressed_points (BStreamFileToolkit & tk) alter {
    TK_Status       status = TK_Normal;

#ifndef BSTREAM_DISABLE_ASCII
	if( tk.GetAsciiMode() ) 
		return read_uncompressed_points_ascii (tk);
#endif
    switch (m_substage) {

        case 0: {
            if ((status = GetData(tk, mp_pointcount)) != TK_Normal)
                return status;
            if (!validate_count (mp_pointcount))
                return tk.Error("bad Shell Point count");
            set_points(mp_pointcount);
            m_substage++;
        }   nobreak;

        case 1: {
            if ((status = GetData(tk, mp_points, 3 * mp_pointcount)) != TK_Normal)
                return status;
            m_substage = 0; 
        }   break;  

        default:
            return tk.Error();
    }
    return status;

} 



TK_Status TK_Shell::read_uncompressed_faces (BStreamFileToolkit & tk) alter {
    TK_Status       status = TK_Normal;

#ifndef BSTREAM_DISABLE_ASCII
	if( tk.GetAsciiMode() ) 
		return read_uncompressed_faces_ascii (tk);
#endif

    switch (m_substage) {
        
        case 0: {
            if ((status = GetData(tk, mp_compression_scheme)) != TK_Normal)
                return status;
            ASSERT(mp_compression_scheme == CS_TRIVIAL);
            m_substage++;
        }   nobreak;

        
        case 1: {
            if ((status = GetData(tk, mp_workspace_used)) != TK_Normal)
                return status;
            if (mp_workspace_used > mp_workspace_allocated) {
                mp_workspace_allocated = mp_workspace_used;
                ENSURE_ALLOCATED(mp_workspace, unsigned char, mp_workspace_allocated);
            }
            m_substage++;
        } nobreak;

        
        case 2: {
            if ((status = GetData(tk, mp_workspace, mp_workspace_used)) != TK_Normal)
                return status;
            m_substage++;
        }   nobreak;

        
        case 3: {
            int has_negative_faces = mp_subop2 & TKSH2_HAS_NEGATIVE_FACES;
            status = trivial_decompress_faces(
                            tk, mp_workspace_used, mp_workspace, has_negative_faces,
                            &m_flistlen, &m_flist);
            if (status != TK_Normal)
                return status;

            m_substage = 0; 
        }   break;  

        default:
            return tk.Error();
    }
    return status;

} 



TK_Status TK_Shell::read_bounding (BStreamFileToolkit & tk) alter {
    TK_Status       status = TK_Normal;

#ifndef BSTREAM_DISABLE_ASCII
	if( tk.GetAsciiMode() ) 
		return read_bounding_ascii (tk);
#endif

    switch (m_substage) {
        case 0: {
            if (mp_subop & TKSH_EXPANDED) {
                if ((status = GetData(tk, mp_subop2)) != TK_Normal)
                    return status;
            }
            m_substage++;
        }   nobreak;

        case 1: {
            if ((status = GetData(tk, m_lodlevel)) != TK_Normal)
                return status;
            m_substage++;
        }   nobreak;

        case 2: {
            if ((status = GetData(tk, mp_bbox, 6)) != TK_Normal)
                return status;
            m_substage = 0; 
        }   break;

        default:
            return tk.Error();
    }
    return status;

} 



TK_Status TK_Shell::Read (BStreamFileToolkit & tk) alter {
    TK_Status       status = TK_Normal;
    int             index;
    int             i, len;

#ifndef BSTREAM_DISABLE_ASCII
	if( tk.GetAsciiMode() ) 
		return ReadAscii (tk);
#endif

    
    
    
    if (m_stage == 0) {
        if ((status = GetData(tk, mp_subop)) != TK_Normal)
            return status;
        m_stage++;
    }
    if (mp_subop & TKSH_BOUNDING_ONLY)
        return read_bounding(tk);

    switch (m_stage) {

        
        case 1: {
            if (mp_subop & TKSH_EXPANDED) {
                if ((status = GetData(tk, mp_subop2)) != TK_Normal)
                    return status;
            }
            m_stage++;
        } nobreak;

        
        case 2: {
            if (!(mp_subop & TKSH_FIRSTPASS)) {
                if ((status = GetData(tk, index)) != TK_Normal)
                    return status;
                if ((status = tk.IndexToKey(index, mp_key)) != TK_Normal)
                    return tk.Error();
            }
            m_stage++;
        } nobreak;

        
        case 3: {
            if ((status = GetData(tk, m_lodlevel)) != TK_Normal)
                return status;
            m_stage++;
        } nobreak;

        
        case 4: {
            if (mp_subop2 & TKSH2_NULL)
                return status;
            if (mp_subop2 & TKSH2_COLLECTION)
                return read_collection(tk);

            if (mp_subop & (TKSH_COMPRESSED_POINTS|TKSH_CONNECTIVITY_COMPRESSION)) {
                if ((status = GetData(tk, mp_compression_scheme)) != TK_Normal)
                    return status;
            }
            else
                mp_compression_scheme = CS_NONE;
            m_stage++;
        } nobreak;

        
        case 5: {
            switch (mp_compression_scheme) {
                case CS_EDGEBREAKER:
                    if ((status = read_advanced(tk)) != TK_Normal)
                        return status;
                    break;
                case CS_TRIVIAL:
                    if ((status = read_trivial_points(tk)) != TK_Normal)
                        return status;
                    break;
                case CS_NONE:
                    if ((status = read_uncompressed_points(tk)) != TK_Normal)
                        return status;
                    break;
                default:
                    return tk.Error();
            }
            m_stage++;
        }   nobreak;

        
        case 6: {
            switch (mp_compression_scheme) {
                case CS_EDGEBREAKER:
                    
                    break;
                case CS_TRIVIAL:
                default:
                if ((status = read_uncompressed_faces(tk)) != TK_Normal)
                    return status;
            }
            if (mp_subop & TKSH_TRISTRIPS) {
                i = 0;
                while (i < m_flistlen) {
                    if (m_flist[i] > 0)
                        len = m_flist[i];   
                    else
                        len = - m_flist[i]; 
                    mp_facecount += len - 2;
                    i += len + 1;
                }
            }
            else {
                i = 0;
                while (i < m_flistlen) {
                    if (m_flist[i] > 0) {
                        mp_facecount++;
                        i += m_flist[i] + 1;
                    }
                    else {
                        
                        i += -m_flist[i] + 1;
                    }
                }
            }
            m_stage++;
        }   nobreak;

        
        case 7: {
            
            if (mp_subop & TKSH_HAS_OPTIONALS) {
                if ((status = TK_Polyhedron::Read(tk)) != TK_Normal)
                    return status;
            }


            
            if (tk.GetLogging()) {
                char            buffer[64];

                if (tk.GetLoggingOptions (TK_Logging_Tagging) &&
                    (mp_subop & TKSH_FIRSTPASS) == 0) {
                    if ((status = tk.KeyToIndex(mp_key, index)) != TK_Normal)
                        return tk.Error();

                    sprintf (buffer, "[%d:%d]", index, m_lodlevel);
                    tk.LogEntry (buffer);
                }
            }
            m_stage = -1;
        }   break;

        default:
            return tk.Error();
    }
    return status;

} 




void TK_Shell::Reset (void) alter
{
    _bOptimizeThisShell = false;
    _nOptimizeVertexTolerance = 0.0;
    _nOptimizeVertexNormalsTolerance  = 0.0;
    _nOptimizeTextureParametersTolerance = 0.0;

    m_substage = 0;
    m_flistlen = 0;
    delete [] m_flist;
    m_flist = null;
    m_lodlevel = 0;
    TK_Polyhedron::Reset();
}
