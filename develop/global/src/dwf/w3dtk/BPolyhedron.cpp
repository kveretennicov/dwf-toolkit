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
// $Header: //DWF/Working_Area/Willie.Zhu/w3dtk/BPolyhedron.cpp#1 $
//

#include "dwf/w3dtk/BStream.h"
#include "dwf/w3dtk/BOpcodeShell.h"
#include "dwf/w3dtk/BCompress.h"
#include "dwf/w3dtk/BInternal.h"
#include <math.h>
#include <stdlib.h>
#define ASSERT(x)




////////////////////////////////////////////////////////////////////////////////


TK_Polyhedron::TK_Polyhedron(unsigned char op)
            : BBaseOpcodeHandler (op) {
    mp_points = null;
    mp_normals = null;
    mp_params = null;  
    mp_vfcolors = null;
    mp_vecolors = null;
    mp_vmcolors = null;
    mp_vfindices = null;
    mp_veindices = null;
    mp_vmindices = null;
    mp_vmvisibilities = null;
    mp_vmsizes = null;
    mp_vmsymbols = null;
    mp_vmsymbolstrings = null;

    mp_fcolors = null;
    mp_findices = null;
    mp_fnormals = null;
    mp_fvisibilities = null;
    mp_fpatterns = null;
    mp_fregions = null;

    mp_ecolors = null;
    mp_eindices = null;
    mp_enormals = null;
    mp_evisibilities = null;
    mp_epatterns = null;
    mp_epatternstrings = null;
    mp_eweights = null;
    
    mp_exists = null;
    mp_face_exists = null;
    mp_edge_exists = null;
    mp_workspace = null;
    mp_edge_enumeration = null;
    mp_instance = null;
    mp_collection_parts = null;
    Reset();
}


TK_Polyhedron::~TK_Polyhedron() {
    //optionals
    delete [] mp_points;
    delete [] mp_normals;
    delete [] mp_params;
    delete [] mp_vfcolors;
    delete [] mp_vecolors;
    delete [] mp_vmcolors;
    delete [] mp_vfindices;
    delete [] mp_veindices;
    delete [] mp_vmindices;
    delete [] mp_fcolors;
    delete [] mp_findices;
    delete [] mp_fregions;
    //other temporary data
    delete [] mp_exists;
    delete [] mp_face_exists;
    delete [] mp_workspace;
    delete [] mp_edge_enumeration;
    delete [] mp_edge_exists;
    delete mp_instance;
    delete [] mp_collection_parts;
}


bool TK_Polyhedron::has_face_attributes() const {
    return mp_fcolors != null || mp_findices != null || mp_fregions != null;
}


TK_Status TK_Polyhedron::InitSubop(BStreamFileToolkit &tk, int lodlevel, 
        bool is_firstpass) {

    int flags = tk.GetWriteFlags();

    mp_subop = 0;
    mp_subop2 = 0;

    if ((is_firstpass) &&
        (flags & TK_First_LOD_Is_Bounding_Box) &&
        (lodlevel != 0)) {
           mp_subop |= TKSH_BOUNDING_ONLY;
    }

    if (!(flags & TK_Full_Resolution_Vertices)) {
        mp_subop |= TKSH_COMPRESSED_POINTS;
    }

    if (flags & TK_Connectivity_Compression) {
        //may be turned off later if edgebreaker punts or if tk_shell::compute advanced
        //detects an invalid combination of flags and/or file write target version
        mp_subop |= TKSH_CONNECTIVITY_COMPRESSION;  
    }    

    //we always at least compress faces to use the appropriate type 
    //(e.g. 16-bit if less than 65k)
    mp_subop |= TKSH_COMPRESSED_FACES; 

    if (flags & TK_Global_Quantization &&
        tk.GetWorldBounding() != null) {
        mp_subop |= TKSH_EXPANDED;
        mp_subop2 |= TKSH2_GLOBAL_QUANTIZATION;
    }

    if (!(flags & TK_Disable_Tristrips))
        mp_subop |= TKSH_TRISTRIPS;

    return TK_Normal;
}


TK_Status TK_Polyhedron::SetPoints(int count, float const *points) alter {
    mp_pointcount = count;
    mp_points = new float [3*mp_pointcount];
    if (mp_points == null)
        return TK_Error;
    if (points != null)
        memcpy(mp_points, points, 3 * mp_pointcount * sizeof(float));
    return TK_Normal;
}

TK_Status TK_Polyhedron::SetVertexNormals(float const *normals) {
    int i;
    if (mp_normals == null) {
        mp_normals = new float [3*mp_pointcount];
    }
    if (mp_normals == null)
        return TK_Error;
    if (normals != null) {
        if (mp_exists == null) 
            set_exists();
        if (mp_exists == null) 
            return TK_Error;
        memcpy(mp_normals, normals, 3 * mp_pointcount * sizeof(float));
        mp_normalcount = 0;
        for (i = 0; i < mp_pointcount; i++) {
                        //use <0,0,0> as a special case to mean "no normal" in which case we do 
                        //not set the relevant mp_exists bit
                        if ((normals[3*i] != 0.0) || (normals[3*i+1] != 0.0) || (normals[3*i+2] != 0.0)) {
                                mp_normalcount++;
                                mp_exists[i] |= Vertex_Normal;
                        }
                }
    }
    return TK_Normal;
}


TK_Status TK_Polyhedron::SetVertexParameters(float const *params, int width) {
    int i;

    if (width <= 0)
		width = mp_paramwidth;
	else {
		if (mp_params != null && mp_paramwidth != width) {
			delete [] mp_params;
			mp_params = null;
		}
		mp_paramwidth = (char)width;
	}

    if (mp_params == null) {
        if ((mp_params = new float [width*mp_pointcount]) == null)
			return TK_Error;
    }

    if (params != null) {
        if (mp_exists == null) 
            set_exists();
        if (mp_exists == null) 
            return TK_Error;

        for (i = 0; i < mp_pointcount; i++)
            mp_exists[i] |= Vertex_Parameter;
        memcpy(mp_params, params, width*mp_pointcount * sizeof(float));
        mp_paramcount = mp_pointcount;
    }
    return TK_Normal;
}

TK_Status TK_Polyhedron::SetVertexFaceColors(float const *colors) {
    int i;
    if (mp_vfcolors == null) {
        mp_vfcolors = new float [3*mp_pointcount];
    }
    if (mp_vfcolors == null)
        return TK_Error;
    if (colors != null) {
        if (mp_exists == null) 
            set_exists();
        if (mp_exists == null) 
            return TK_Error;
        for (i = 0; i < mp_pointcount; i++)
            mp_exists[i] |= Vertex_Face_Color;
        memcpy(mp_vfcolors, colors, 3 * mp_pointcount * sizeof(float));
        mp_vfcolorcount = mp_pointcount;
    }
    return TK_Normal;
}

TK_Status TK_Polyhedron::SetVertexEdgeColors(float const *colors) {
    int i;
    if (mp_vecolors == null) {
        mp_vecolors = new float [3*mp_pointcount];
    }
    if (mp_vecolors == null)
        return TK_Error;
    if (colors != null) {
        if (mp_exists == null) 
            set_exists();
        if (mp_exists == null) 
            return TK_Error;
        for (i = 0; i < mp_pointcount; i++)
            mp_exists[i] |= Vertex_Edge_Color;
        memcpy(mp_vecolors, colors, 3 * mp_pointcount * sizeof(float));
        mp_vecolorcount = mp_pointcount;
    }
    return TK_Normal;
}

TK_Status TK_Polyhedron::SetVertexMarkerColors(float const *colors) {
    int i;
    if (mp_vmcolors == null) {
        mp_vmcolors = new float [3*mp_pointcount];
    }
    if (mp_vmcolors == null)
        return TK_Error;
    if (colors != null) {
        if (mp_exists == null) 
            set_exists();
        if (mp_exists == null) 
            return TK_Error;
        for (i = 0; i < mp_pointcount; i++)
            mp_exists[i] |= Vertex_Marker_Color;
        memcpy(mp_vmcolors, colors, 3 * mp_pointcount * sizeof(float));
        mp_vmcolorcount = mp_pointcount;
    }
    return TK_Normal;
}

TK_Status TK_Polyhedron::SetVertexColors(float const *colors) {
    if (SetVertexFaceColors(colors) != TK_Normal)
        return TK_Error;
    if (SetVertexEdgeColors(colors) != TK_Normal)
        return TK_Error;
    if (SetVertexMarkerColors(colors) != TK_Normal)
        return TK_Error;
    return TK_Normal;
}

TK_Status TK_Polyhedron::SetVertexFaceIndices(float const *indices) {
    int i;
    if (mp_vfindices == null) {
        mp_vfindices = new float [mp_pointcount];
    }
    if (mp_vfindices == null)
        return TK_Error;
    if (indices != null) {
        if (mp_exists == null) 
            set_exists();
        if (mp_exists == null) 
            return TK_Error;
        for (i = 0; i < mp_pointcount; i++)
            mp_exists[i] |= Vertex_Face_Index;
        memcpy(mp_vfindices, indices, mp_pointcount * sizeof(float));
        mp_vfindexcount = mp_pointcount;
    }
    return TK_Normal;
}

TK_Status TK_Polyhedron::SetVertexEdgeIndices(float const *indices) {
    int i;
    if (mp_veindices == null) {
        mp_veindices = new float [mp_pointcount];
    }
    if (mp_veindices == null)
        return TK_Error;
    if (indices != null) {
        if (mp_exists == null) 
            set_exists();
        if (mp_exists == null) 
            return TK_Error;
        for (i = 0; i < mp_pointcount; i++)
            mp_exists[i] |= Vertex_Edge_Index;
        memcpy(mp_veindices, indices, mp_pointcount * sizeof(float));
        mp_veindexcount = mp_pointcount;
    }
    return TK_Normal;
}

TK_Status TK_Polyhedron::SetVertexMarkerIndices(float const *indices) {
    int i;
    if (mp_vmindices == null) {
        mp_vmindices = new float [mp_pointcount];
    }
    if (mp_vmindices == null)
        return TK_Error;
    if (indices != null) {
        if (mp_exists == null) 
            set_exists();
        if (mp_exists == null) 
            return TK_Error;
        for (i = 0; i < mp_pointcount; i++)
            mp_exists[i] |= Vertex_Marker_Index;
        memcpy(mp_vmindices, indices, mp_pointcount * sizeof(float));
        mp_vmindexcount = mp_pointcount;
    }
    return TK_Normal;
}
TK_Status TK_Polyhedron::SetVertexIndices(float const *indices) {
    if (SetVertexFaceIndices(indices) != TK_Normal)
        return TK_Error;
    if (SetVertexEdgeIndices(indices) != TK_Normal)
        return TK_Error;
    if (SetVertexMarkerIndices(indices) != TK_Normal)
        return TK_Error;
    return TK_Normal;
}
TK_Status TK_Polyhedron::SetVertexMarkerVisibilities(char const *visibilities) {
    int i;
    if (mp_vmvisibilities == null) {
        mp_vmvisibilities = new char [mp_pointcount];
    }
    if (mp_vmvisibilities == null)
        return TK_Error;
    if (visibilities != null) {
        if (mp_exists == null) 
            set_exists();
        if (mp_exists == null) 
            return TK_Error;
        for (i = 0; i < mp_pointcount; i++)
            mp_exists[i] |= Vertex_Marker_Visibility;
        memcpy(mp_vmvisibilities, visibilities, mp_pointcount * sizeof(char));
        mp_vmvisibilitycount = mp_pointcount;
    }
    return TK_Normal;
}
TK_Status TK_Polyhedron::SetVertexMarkerSizes(float const *sizes) {
    int i;
    if (mp_vmsizes == null) {
        mp_vmsizes = new float [mp_pointcount];
    }
    if (mp_vmsizes == null)
        return TK_Error;
    if (sizes != null) {
        if (mp_exists == null) 
            set_exists();
        if (mp_exists == null) 
            return TK_Error;
        for (i = 0; i < mp_pointcount; i++)
            mp_exists[i] |= Vertex_Marker_Size;
        memcpy(mp_vmsizes, sizes, mp_pointcount * sizeof(float));
        mp_vmsizecount = mp_pointcount;
    }
    return TK_Normal;
}
TK_Status TK_Polyhedron::SetVertexMarkerSymbols(char const *symbols) {
    int i;
    if (mp_vmsymbols == null) {
        mp_vmsymbols = new unsigned char [mp_pointcount];
    }
    if (mp_vmsymbols == null)
        return TK_Error;
    if (symbols != null) {
        if (mp_exists == null) 
            set_exists();
        if (mp_exists == null) 
            return TK_Error;
        for (i = 0; i < mp_pointcount; i++)
            mp_exists[i] |= Vertex_Marker_Symbol;
        memcpy(mp_vmsymbols, symbols, mp_pointcount * sizeof(float));
        mp_vmsymbolcount = mp_pointcount;
    }
    return TK_Normal;
}
TK_Status TK_Polyhedron::SetVertexMarkerSymbolStrings(void) {
    if (mp_vmsymbolstrings == null) {
        mp_vmsymbolstrings = new char * [mp_pointcount];
        if (mp_vmsymbolstrings == null)
            return TK_Error;
        memset (mp_vmsymbolstrings, 0, mp_pointcount * sizeof(char *));
    }
    return TK_Normal;
}

TK_Status TK_Polyhedron::SetFaceColors(float const *colors) {
    int i;
    if (mp_fcolors == null) {
        mp_fcolors = new float [3*mp_facecount];
        if (mp_fcolors == null)
            return TK_Error;
    }
    if (colors != null) {
        if (mp_face_exists == null) 
            set_face_exists();
        if (mp_face_exists == null) 
            return TK_Error;
        for (i = 0; i < mp_facecount; i++)
            mp_face_exists[i] |= Face_Color;
        memcpy(mp_fcolors, colors, 3 * mp_facecount * sizeof(float));
        mp_fcolorcount = mp_facecount;
    }
    return TK_Normal;
}
TK_Status TK_Polyhedron::SetFaceIndices(float const *indices) {
    int i;
    if (mp_findices == null) {
        mp_findices = new float [mp_facecount];
        if (mp_findices == null)
            return TK_Error;
    }
    if (indices != null) {
        if (mp_face_exists == null) 
            set_face_exists();
        if (mp_face_exists == null) 
            return TK_Error;
        for (i = 0; i < mp_facecount; i++)
            mp_face_exists[i] |= Face_Index;
        memcpy(mp_findices, indices, mp_facecount * sizeof(float));
        mp_findexcount = mp_facecount;
    }
    return TK_Normal;
}
TK_Status TK_Polyhedron::SetFaceNormals(float const *normals) {
    int i;
    if (mp_fnormals == null) {
        mp_fnormals = new float [3*mp_facecount];
        if (mp_fnormals == null)
            return TK_Error;
    }
    if (normals != null) {
        if (mp_face_exists == null) 
            set_face_exists();
        if (mp_face_exists == null) 
            return TK_Error;
        for (i = 0; i < mp_facecount; i++)
            mp_face_exists[i] |= Face_Normal;
        memcpy(mp_fnormals, normals, 3 * mp_facecount * sizeof(float));
        mp_fnormalcount = mp_facecount;
    }
    return TK_Normal;
}
TK_Status TK_Polyhedron::SetFaceVisibilities(char const *visibilities) {
    int i;
    if (mp_fvisibilities == null) {
        mp_fvisibilities = new char [mp_facecount];
    }
    if (mp_fvisibilities == null)
        return TK_Error;
    if (visibilities != null) {
        if (mp_exists == null) 
            set_exists();
        if (mp_exists == null) 
            return TK_Error;
        for (i = 0; i < mp_facecount; i++)
            mp_face_exists[i] |= Face_Visibility;
        memcpy(mp_fvisibilities, visibilities, mp_facecount * sizeof(char));
        mp_fvisibilitycount = mp_facecount;
    }
    return TK_Normal;
}
TK_Status TK_Polyhedron::SetFacePatterns(char const *patterns) {
    int i;
    if (mp_fpatterns == null) {
        mp_fpatterns = new char [mp_facecount];
    }
    if (mp_fpatterns == null)
        return TK_Error;
    if (patterns != null) {
        if (mp_exists == null) 
            set_exists();
        if (mp_exists == null) 
            return TK_Error;
        for (i = 0; i < mp_facecount; i++)
            mp_face_exists[i] |= Face_Pattern;
        memcpy(mp_fpatterns, patterns, mp_facecount * sizeof(char));
        mp_fpatterncount = mp_facecount;
    }
    return TK_Normal;
}

TK_Status TK_Polyhedron::SetFaceRegions (int const * regions) {
    if (mp_fregions == null) {
        mp_fregions = new int [mp_facecount];

        if (mp_fregions == null)
            return TK_Error;
    }

    if (regions != null)
        memcpy (mp_fregions, regions, mp_facecount*sizeof(int));

    return TK_Normal;
}


TK_Status TK_Polyhedron::SetEdgeColors(float const *colors) {
    int i;
    if (mp_edge_exists == null) 
        SetEdgeExists();
    if (mp_edge_exists == null) 
        return TK_Error;
    if (mp_ecolors == null) {
        mp_ecolors = new float [3 * mp_edgecount];
    }
    if (mp_ecolors == null)
        return TK_Error;
    if (colors != null) {
        for (i = 0; i < mp_edgecount; i++)
            mp_edge_exists[i] |= Edge_Color;
        memcpy(mp_ecolors, colors, 3 * mp_edgecount * sizeof(float));
        mp_ecolorcount = mp_edgecount;
    }
    return TK_Normal;
}
TK_Status TK_Polyhedron::SetEdgeIndices(float const *indices) {
    int i;
    if (mp_edge_exists == null) 
        SetEdgeExists();
    if (mp_edge_exists == null) 
        return TK_Error;
    if (mp_eindices == null) {
        mp_eindices = new float [mp_edgecount];
    }
    if (mp_eindices == null)
        return TK_Error;
    if (indices != null) {
        for (i = 0; i < mp_edgecount; i++)
            mp_edge_exists[i] |= Edge_Index;
        memcpy(mp_eindices, indices, mp_edgecount * sizeof(float));
        mp_eindexcount = mp_edgecount;
    }
    return TK_Normal;
}
TK_Status TK_Polyhedron::SetEdgeNormals(float const *normals) {
    int i;
    if (mp_edge_exists == null) 
        SetEdgeExists();
    if (mp_edge_exists == null) 
        return TK_Error;
    if (mp_enormals == null) {
        mp_enormals = new float [3 * mp_edgecount];
    }
    if (mp_enormals == null)
        return TK_Error;
    if (normals != null) {
        for (i = 0; i < mp_edgecount; i++)
            mp_edge_exists[i] |= Edge_Normal;
        memcpy(mp_enormals, normals, 3 * mp_edgecount * sizeof(float));
        mp_enormalcount = mp_edgecount;
    }
    return TK_Normal;
}

TK_Status TK_Polyhedron::SetEdgeVisibilities(char const *visibilities) {
    int i;
    if (mp_edge_exists == null) 
        SetEdgeExists();
    if (mp_edge_exists == null) 
        return TK_Error;
    if (mp_evisibilities == null) {
        mp_evisibilities = new char [mp_edgecount];
    }
    if (mp_evisibilities == null)
        return TK_Error;
    if (visibilities != null) {
        for (i = 0; i < mp_edgecount; i++)
            mp_edge_exists[i] |= Edge_Visibility;
        memcpy(mp_evisibilities, visibilities, mp_edgecount * sizeof(char));
        mp_evisibilitycount = mp_edgecount;
    }
    return TK_Normal;
}
TK_Status TK_Polyhedron::SetEdgePatterns(char const *patterns) {
    int i;
    if (mp_edge_exists == null) 
        SetEdgeExists();
    if (mp_edge_exists == null) 
        return TK_Error;
    if (mp_epatterns == null) {
        mp_epatterns = new unsigned char [mp_edgecount];
    }
    if (mp_epatterns == null)
        return TK_Error;
    if (patterns != null) {
        for (i = 0; i < mp_edgecount; i++)
            mp_edge_exists[i] |= Edge_Pattern;
        memcpy(mp_epatterns, patterns, mp_edgecount * sizeof(char));
        mp_epatterncount = mp_edgecount;
    }
    return TK_Normal;
}
TK_Status TK_Polyhedron::SetEdgePatternStrings (void) {
    if (mp_edge_exists == null) {
        SetEdgeExists();
        if (mp_edge_exists == null) 
            return TK_Error;
    }
    if (mp_epatternstrings == null) {
        mp_epatternstrings = new char * [mp_edgecount];
        if (mp_epatternstrings == null)
            return TK_Error;
        memset (mp_epatternstrings, 0, mp_edgecount*sizeof(char *));
    }
    return TK_Normal;
}
TK_Status TK_Polyhedron::SetEdgeWeights(float const *weights) {
    int i;
    if (mp_edge_exists == null) 
        SetEdgeExists();
    if (mp_edge_exists == null) 
        return TK_Error;
    if (mp_eweights == null) {
        mp_eweights = new float [mp_edgecount];
    }
    if (mp_eweights == null)
        return TK_Error;
    if (weights != null) {
        for (i = 0; i < mp_edgecount; i++)
            mp_edge_exists[i] |= Edge_Weight;
        memcpy(mp_eweights, weights, mp_edgecount * sizeof(float));
        mp_eweightcount = mp_edgecount;
    }
    return TK_Normal;
}

TK_Status TK_Polyhedron::SetExists (unsigned int const * exists) alter {
    delete [] mp_exists;
    mp_exists = new unsigned int [mp_pointcount];
    if (mp_exists == null)
        return TK_Error;
    if (exists != null)
        memcpy (mp_exists, exists, mp_pointcount*sizeof(unsigned int));
    else
        memset (mp_exists, 0, mp_pointcount*sizeof(unsigned int));
    return TK_Normal;
}
TK_Status TK_Polyhedron::SetFaceExists (unsigned int const * exists) alter {
    delete [] mp_face_exists;
    mp_face_exists = new unsigned int [mp_facecount];
    if (mp_face_exists == null)
        return TK_Error;
    if (exists != null)
        memcpy (mp_face_exists, exists, mp_facecount*sizeof(unsigned int));
    else
        memset (mp_face_exists, 0, mp_facecount*sizeof(unsigned int));
    return TK_Normal;
}
TK_Status TK_Polyhedron::SetEdgeExists (unsigned int const * exists) alter {
    delete [] mp_edge_exists;
	mp_edge_exists = null;
    if (mp_edgecount == -1)
        EnumerateEdges();
    if (mp_edge_enumeration == null)
        return TK_Error;
    mp_edge_exists = new unsigned int [mp_edgecount];
    if (mp_edge_exists == null)
        return TK_Error;
    if (exists != null)
        memcpy (mp_edge_exists, exists, mp_edgecount*sizeof(unsigned int));
    else 
        memset (mp_edge_exists, 0, mp_edgecount*sizeof(unsigned int));
    LabelFakeEdges();
    return TK_Normal;
}

void TK_Polyhedron::set_exists (unsigned int const * exists) alter {
    SetExists(exists);
}
void TK_Polyhedron::set_face_exists (unsigned int const * exists) alter {
    SetFaceExists(exists);
}



#ifndef EPS
  #define EPS (1e-5)
#endif
/* for each unit vector, calculate theta and phi.  Theta runs 360 degrees [-pi,pi] around the equator,
 * with theta==0 corresponding to +x.  Phi runs 180 degrees [0,pi], longitudinally, with phi==0
 * corresponding to -y */
void TK_Polyhedron::normals_cartesian_to_polar(
                                unsigned int *exists, 
                                int flag, 
                                int count, 
                                float const *in, 
                                float alter *out)
{
    float const *end;

    end = &in[3*count];
    /* the following loop is carefully designed so that references don't interfere, 
     * even if "in" and "out" are initially pointing to the same thing */
    while (in < end) {
        if (!exists || (*exists++ & flag)) {
            //atan2 is not defined for [0,0]
            if (in[0] < -EPS || in[0] > EPS ||
                in[2] < -EPS || in[2] > EPS)
                out[0] = (float)atan2(in[2],in[0]); 
            else
                out[0] = 0;
            out[1] = (float)acos(in[1]);            
        }
        else {
            out[0] = out[1] = 0;
        }
        in += 3;
        out += 2;
    }
}

void TK_Polyhedron::normals_polar_to_cartesian(
                                unsigned int *exists, 
                                int flag, 
                                int count, 
                                float const *in, 
                                float alter *out)
{
    float theta, phi, sin_phi;
    float const *end;

    end = in;
    in = &in[2*(count-1)];
    out = &out[3*(count-1)];
    if (exists)
        exists += count-1;
    /* the following loop is carefully designed so that references don't interfere, 
     * even if "in" and "out" are initially pointing to the same thing */
    for (;;) {
        if (!exists || (*exists & flag)) {
            theta = in[0];
            phi = in[1];
            sin_phi = (float) sin (phi);
            out[0] = sin_phi * (float) cos (theta);
            out[1] = (float) cos (phi);
            out[2] = sin_phi * (float) sin (theta);
        }
        else {
            out[0] = out[1] = out[2] = 0;
        }
        if (in == end)
            break;
        if (exists)
            exists--;
        in -= 2;
        out -= 3;
    }
}

//protected virtual
TK_Status TK_Polyhedron::write_trivial_points (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_READONLY
    TK_Status       status = TK_Normal;

#ifndef BSTREAM_DISABLE_ASCII
	if( tk.GetAsciiMode() ) 
		return write_trivial_points_ascii (tk);
#endif

    switch (mp_substage) {
        case 0: {
            ASSERT(mp_compression_scheme == CS_TRIVIAL);

            if (mp_subop2 & TKSH2_GLOBAL_QUANTIZATION) {
                float const *global_bbox;

                global_bbox = tk.GetWorldBounding();
                //these things should've already been checked
                ASSERT(tk.GetTargetVersion() >= 806);
                ASSERT(global_bbox);
                memcpy(mp_bbox, global_bbox, 6 * sizeof(float));
            }
            mp_substage++;
        } nobreak;

        //allocate and fill the workspace
        case 1: {
            mp_bits_per_sample = (unsigned char)(tk.GetNumVertexBits() / 3);
            if ((tk.GetTargetVersion() >= 650) &&
                (mp_bits_per_sample != 8)) {
                /* at this point, we don't know if mp_bbox should be treated
                 * as valid or not. put it as both input and output and 
                 * quantize_and_pack_floats will figure it out. */
                status = quantize_and_pack_floats(
                            tk, 
                            mp_pointcount, 
                            3, 
                            mp_points, 
                            mp_bbox, /* may or may not be valid -- see above */
                            mp_bits_per_sample, 
                            mp_bbox,
                            &mp_workspace_allocated, 
                            &mp_workspace_used, 
                            &mp_workspace);
            }
            else {
                mp_bits_per_sample = (unsigned char) 8;
                status = trivial_compress_points(
                            tk, 
                            mp_pointcount, 
                            mp_points, 
                            mp_bbox, /* may or may not be valid */
                            null, 
                            0, 
                            &mp_workspace_allocated, 
                            &mp_workspace_used, 
                            &mp_workspace, 
                            mp_bbox);
            }
            if (status != TK_Normal)
                return status;
            tk.ReportQuantizationError(mp_bits_per_sample, mp_bbox);
            mp_substage++;
        } nobreak;

        //compression scheme
        case 2: {
            if ((status = PutData(tk, mp_compression_scheme)) != TK_Normal)
                return status;
            mp_substage++;
        } nobreak;

        //number of points
        case 3: {
            if ((status = PutData(tk, mp_pointcount)) != TK_Normal)
                return status;
            m_progress = 0;
            if (mp_pointcount == 0)
                return status;
            mp_substage++;
        } nobreak;

        //the bounding box
        case 4: {
            if (!(mp_subop2 & TKSH2_GLOBAL_QUANTIZATION)) {
                if ((status = PutData(tk, mp_bbox, 6)) != TK_Normal)
                    return status;
            }
            mp_substage++;
        } nobreak;

        //bits per sample
        case 5: {
            if ((status = PutData(tk, mp_bits_per_sample)) != TK_Normal)
                return status;
            mp_substage++;
        } nobreak;


        //workspace length (if applicable)
        case 6: {
            if (tk.GetTargetVersion() >= 650) {
                if ((status = PutData(tk, mp_workspace_used)) != TK_Normal)
                    return status;
            }
            mp_substage++;
        } nobreak;


        //the data
        case 7: {
            if ((status = PutData(tk, mp_workspace, mp_workspace_used)) != TK_Normal)
                return status;

            m_progress = 0;
            mp_substage = 0;
        } break;

        default:
            return tk.Error("internal error from TK_Polyhedron::write_trivial_points");
    }
    return status;
#else
    return tk.Error (stream_readonly);
#endif
} // end function TK_Polyhedron::write_trivial_points


TK_Status TK_Polyhedron::write_vertex_normals_compressed_all (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_READONLY
    TK_Status       status = TK_Normal;

	
#ifndef BSTREAM_DISABLE_ASCII
	if( tk.GetAsciiMode() ) 
		return write_vertex_normals_compressed_all_ascii (tk);
#endif

    ASSERT(mp_optopcode == OPT_ALL_NORMALS_COMPRESSED);
    switch (mp_substage) {

        //compression scheme
        case 1: {
            if ((status = PutData(tk, mp_compression_scheme)) != TK_Normal)
                return status;
            mp_substage++;
        }   nobreak;

        //bits per sample (set external to this function in "write vertex normals main")
        case 2: {
            if ((status = PutData(tk, mp_bits_per_sample)) != TK_Normal)
                return status;
            mp_substage++;
        } nobreak;

        //allocate and fill the workspace
        case 3: {
            switch (mp_compression_scheme) {
                case CS_TRIVIAL_POLAR:
                    normals_cartesian_to_polar(mp_exists, Vertex_Normal, mp_pointcount, mp_normals, mp_normals);
                    status = quantize_and_pack_floats(
                                tk, 
                                mp_pointcount, 
                                2, 
                                mp_normals, 
                                polar_bounds, 
                                null, 
                                Vertex_Normal, 
                                mp_bits_per_sample, 
                                null, 
                                &mp_workspace_allocated, 
                                &mp_workspace_used, 
                                &mp_workspace);
                    if (status != TK_Normal)
                        return status;
                    break;
                case CS_TRIVIAL:
                    if (tk.GetTargetVersion() >= 650) {
                        status = quantize_and_pack_floats(
                                    tk, 
                                    mp_pointcount, 
                                    3, 
                                    mp_normals, 
                                    normal_cube, 
                                    null, 
                                    Vertex_Normal, 
                                    mp_bits_per_sample, 
                                    null, 
                                    &mp_workspace_allocated, 
                                    &mp_workspace_used, 
                                    &mp_workspace);
                    }
                    else {
                        status = trivial_compress_points(
                                    tk, 
                                    mp_pointcount, 
                                    mp_normals, 
                                    normal_cube, 
                                    null, 
                                    Vertex_Normal,
                                    &mp_workspace_allocated, 
                                    &mp_workspace_used, 
                                    &mp_workspace, 
                                    null);
                    }
                    if (status != TK_Normal)
                        return status;
                    break;
                case CS_REPULSE:
                    #ifndef BSTREAM_DISABLE_REPULSE_COMPRESSION
                        mp_workspace_used = (mp_pointcount * mp_bits_per_sample + 7) / 8;
                        if (mp_workspace_used > mp_workspace_allocated) {
                            mp_workspace_allocated = mp_workspace_used;
                            ENSURE_ALLOCATED(mp_workspace, unsigned char, mp_workspace_allocated);
                        }
                        status = repulse_compress_normals(tk, mp_bits_per_sample);
                        if (status != TK_Normal)
                            return status;
                    #else
                        return tk.Error (stream_disable_repulse_compression);
                    #endif
                    break;
                default:
                    return tk.Error("internal error: unrecognized case in TK_Polyhedron::write_vertex_normals_compressed_all");
            }
            mp_substage++;
        } nobreak;

        //workspace length (if applicable)
        case 4: {
            if (tk.GetTargetVersion() >= 650) {
                if ((status = PutData(tk, mp_workspace_used)) != TK_Normal)
                    return status;
            }
            mp_substage++;
        } nobreak;

        //send all normals
        case 5: {
            if ((status = PutData(tk, mp_workspace, mp_workspace_used)) != TK_Normal)
                return status;
            mp_substage = 0;
        } break;

        default:
            return tk.Error("internal error from TK_Polyhedron::write_vertex_normals_compressed_all");
    }
    return status;
#else
    return tk.Error (stream_readonly);
#endif
} //end function write_vertex_normals_compressed_all


TK_Status TK_Polyhedron::write_vertex_normals_compressed (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_READONLY
    TK_Status       status = TK_Normal;
    unsigned char       byte;
    unsigned short      word;

		
#ifndef BSTREAM_DISABLE_ASCII
	if( tk.GetAsciiMode() ) 
		return write_vertex_normals_compressed_ascii (tk);
#endif

    ASSERT(mp_optopcode == OPT_NORMALS_COMPRESSED);
    switch (mp_substage) {
        //compression scheme
        case 1: {
            ASSERT(mp_compression_scheme == CS_TRIVIAL ||
                    mp_compression_scheme == CS_TRIVIAL_POLAR);
            if ((status = PutData(tk, mp_compression_scheme)) != TK_Normal)
                return status;
            mp_substage++;
        }   nobreak;

        //compression size
        case 2: {
            if ((status = PutData(tk, mp_bits_per_sample)) != TK_Normal)
                return status;
            mp_substage++;
        } nobreak;

        //if all points do NOT have normals, tell how many
        case 3: {
            if (mp_pointcount < 256) {
                byte = (unsigned char)mp_normalcount;
                if ((status = PutData(tk, byte)) != TK_Normal)
                    return status;
            }
            else if (mp_pointcount < 65536) {
                word = (unsigned short)mp_normalcount;
                if ((status = PutData(tk, word)) != TK_Normal)
                    return status;
            }
            else {
                if ((status = PutData(tk, mp_normalcount)) != TK_Normal)
                    return status;
            }
            mp_progress = 0;
            mp_substage++;
        }   nobreak;

        //and indices of existing normals
        case 4: {
            while (mp_progress < mp_pointcount) {
                if (mp_exists[mp_progress] & Vertex_Normal) {
                    if (mp_pointcount < 256) {
                        byte = (unsigned char)mp_progress;
                        if ((status = PutData(tk, byte)) != TK_Normal)
                            return status;
                    }
                    else if (mp_pointcount < 65536) {
                        word = (unsigned short)mp_progress;
                        if ((status = PutData(tk, word)) != TK_Normal)
                            return status;
                    }
                    else {
                        if ((status = PutData(tk, mp_progress)) != TK_Normal)
                            return status;
                    }
                }
                mp_progress++;
            }
            mp_progress = 0;
            mp_substage++;
        } nobreak;

        default:
            break;
    }

    if (tk.GetTargetVersion() >= 650) {
        switch (mp_substage) {
            //allocate and fill the workspace.  mp_workspace_used will reflect the number of normals that exist,
            //not the pointcount
            case 5: {
                if (mp_compression_scheme == CS_TRIVIAL_POLAR) {
                    normals_cartesian_to_polar(mp_exists, Vertex_Normal, mp_pointcount, mp_normals, mp_normals);
                    status = quantize_and_pack_floats(
                                    tk, 
                                    mp_pointcount, 
                                    2, 
                                    mp_normals,                 
                                    polar_bounds, 
                                    mp_exists, 
                                    Vertex_Normal, 
                                    mp_bits_per_sample,                          
                                    null, 
                                    &mp_workspace_allocated, 
                                    &mp_workspace_used, 
                                    &mp_workspace);
                }
                else {
                    status = quantize_and_pack_floats(
                                    tk, 
                                    mp_pointcount, 
                                    3, 
                                    mp_normals,                 
                                    normal_cube, 
                                    mp_exists, 
                                    Vertex_Normal, 
                                    mp_bits_per_sample,                          
                                    null, 
                                    &mp_workspace_allocated, 
                                    &mp_workspace_used, 
                                    &mp_workspace);
                }
                if (status != TK_Normal)
                    return status;
                mp_substage++;
            } nobreak;

            //the workspace length
            case 6: {
                if ((status = PutData(tk, mp_workspace_used)) != TK_Normal)
                    return status;
                mp_progress = 0;
                mp_substage++;
            } nobreak;

            //the raw data
            case 7: {
                if ((status = PutData(tk, mp_workspace, mp_workspace_used)) != TK_Normal)
                    return status;
                mp_progress = 0;
                mp_substage = 0;
            } break;

            default:
                return tk.Error("internal error from TK_Polyhedron::write_vertex_normals_compressed");
        }
    }
    else {
        switch (mp_substage) {
            //allocate and fill the workspace
            case 5: {
                ASSERT(mp_compression_scheme == CS_TRIVIAL);

                status = trivial_compress_points(
                                tk, 
                                mp_pointcount, 
                                mp_normals,                      
                                normal_cube, 
                                mp_exists, 
                                Vertex_Normal, 
                                &mp_workspace_allocated, 
                                &mp_workspace_used, 
                                &mp_workspace, 
                                null);
                if (status != TK_Normal)
                    return status;
                mp_substage++;
            } nobreak;

            //individual normals
            case 6: {
                while (mp_progress < mp_pointcount) {
                    if (mp_exists[mp_progress] & Vertex_Normal) {
                        if ((status = PutData(tk, &mp_workspace[3*mp_progress], 3)) != TK_Normal)
                            return status;
                    }
                    mp_progress++;
                }
                mp_progress = 0;
                mp_substage = 0;
            } break;

            default:
                return tk.Error("internal error from TK_Polyhedron::write_vertex_normals_compressed (version<650)");
        }
    }
    return status;
#else
    return tk.Error (stream_readonly);
#endif
} //end function write_vertex_normals_compressed


TK_Status TK_Polyhedron::write_vertex_normals_all (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_READONLY
    TK_Status       status = TK_Normal;

			
#ifndef BSTREAM_DISABLE_ASCII
	if( tk.GetAsciiMode() ) 
		return write_vertex_normals_all_ascii (tk);
#endif

    ASSERT(mp_optopcode == OPT_ALL_NORMALS ||
            mp_optopcode == OPT_ALL_NORMALS_POLAR);
    switch (mp_substage) {
        case 1: {
            if (mp_optopcode == OPT_ALL_NORMALS_POLAR) {
                normals_cartesian_to_polar(mp_exists, Vertex_Normal, mp_pointcount, mp_normals, mp_normals);
            }
            mp_substage++;
        } nobreak;
            
        //send all normals (normal count is implicit)
        case 2: {
            if (mp_optopcode == OPT_ALL_NORMALS_POLAR) {
                if ((status = PutData(tk, mp_normals, 2 * mp_pointcount)) != TK_Normal)
                    return status;
            }
            else {
                if ((status = PutData(tk, mp_normals, 3 * mp_pointcount)) != TK_Normal)
                    return status;
            }
            mp_substage = 0;
        } break;

        default:
            return tk.Error("internal error from TK_Polyhedron::write_vertex_normals_all");
    }    
    return status;
#else
    return tk.Error (stream_readonly);
#endif
}


TK_Status TK_Polyhedron::write_vertex_normals (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_READONLY
    TK_Status       status = TK_Normal;
    unsigned char       byte;
    unsigned short      word;
		
#ifndef BSTREAM_DISABLE_ASCII
	if( tk.GetAsciiMode() ) 
		return write_vertex_normals_ascii (tk);
#endif

    ASSERT(mp_optopcode == OPT_NORMALS ||
            mp_optopcode == OPT_NORMALS_POLAR);
    switch (mp_substage) {

        //if all points do NOT have normals, tell how many
        case 1: {
            if (mp_pointcount < 256) {
                byte = (unsigned char)mp_normalcount;
                if ((status = PutData(tk, byte)) != TK_Normal)
                    return status;
            }
            else if (mp_pointcount < 65536) {
                word = (unsigned short)mp_normalcount;
                if ((status = PutData(tk, word)) != TK_Normal)
                    return status;
            }
            else {
                if ((status = PutData(tk, mp_normalcount)) != TK_Normal)
                    return status;
            }
            mp_progress = 0;
            mp_substage++;
        }   nobreak;

        //and indices of existing normals
        case 2: {
            while (mp_progress < mp_pointcount) {
                if (mp_exists[mp_progress] & Vertex_Normal) {
                    if (mp_pointcount < 256) {
                        byte = (unsigned char)mp_progress;
                        if ((status = PutData(tk, byte)) != TK_Normal)
                            return status;
                    }
                    else if (mp_pointcount < 65536) {
                        word = (unsigned short)mp_progress;
                        if ((status = PutData(tk, word)) != TK_Normal)
                            return status;
                    }
                    else {
                        if ((status = PutData(tk, mp_progress)) != TK_Normal)
                            return status;
                    }
                }
                mp_progress++;
            }
            //prepare for the next case
            if (mp_optopcode == OPT_NORMALS_POLAR) {
                normals_cartesian_to_polar(mp_exists, Vertex_Normal, mp_pointcount, mp_normals, mp_normals);
            }
            mp_progress = 0;
            mp_substage++;
        } nobreak;

        //individual normals
        case 3: {
            while (mp_progress < mp_pointcount) {
                if (mp_exists[mp_progress] & Vertex_Normal) {
                    if (mp_optopcode == OPT_NORMALS_POLAR)
                        status = PutData(tk, &mp_normals[2*mp_progress], 2);
                    else
                        status = PutData(tk, &mp_normals[3*mp_progress], 3);
                    if (status != TK_Normal)
                        return status;
                }
                mp_progress++;
            }
            mp_progress = 0;
            mp_substage = 0;
        } break;

        default:
            return tk.Error("internal error from TK_Polyhedron::write_vertex_normals");
    }
    return status;
#else
    return tk.Error (stream_readonly);
#endif
}

TK_Status TK_Polyhedron::write_vertex_normals_main (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_READONLY
    TK_Status       status = TK_Normal;

#ifndef BSTREAM_DISABLE_ASCII
	if( tk.GetAsciiMode() ) 
		return write_vertex_normals_main_ascii (tk);
#endif

    if (mp_normalcount <= 0)
        return TK_Normal;

    if (mp_substage == 0) {
#ifdef BSTREAM_DISABLE_QUANTIZATION
        if (tk.GetTargetVersion() >= 907) {
            if (mp_normalcount == mp_pointcount)
                mp_optopcode = OPT_ALL_NORMALS_POLAR;
            else
                mp_optopcode = OPT_NORMALS_POLAR;
        }
        else {
            if (mp_normalcount == mp_pointcount)
                mp_optopcode = OPT_ALL_NORMALS;
            else
                mp_optopcode = OPT_NORMALS;
        }
#else
        if (tk.GetWriteFlags() & TK_Full_Resolution_Normals) {
            if (tk.GetTargetVersion() >= 907) {
                if (mp_normalcount == mp_pointcount)
                    mp_optopcode = OPT_ALL_NORMALS_POLAR;
                else
                    mp_optopcode = OPT_NORMALS_POLAR;
            }
            else {
                if (mp_normalcount == mp_pointcount)
                    mp_optopcode = OPT_ALL_NORMALS;
                else
                    mp_optopcode = OPT_NORMALS;
            }
        }
        else {
            #ifndef BSTREAM_DISABLE_REPULSE_COMPRESSION
                if (tk.GetNumNormalBits() < 16) {
                    mp_compression_scheme = (unsigned char)CS_REPULSE;
                    if (tk.GetNumNormalBits() >= 10)
                        mp_bits_per_sample = (unsigned char)10; // xyz unified into one sample
                    else
                        mp_bits_per_sample = (unsigned char)8; // xyz unified into one sample
                    /* since CS_REPULSE allows for an escape sequence, we can use OPT_ALL_NORMALS_COMPRESSED even
                     * if a few of the normals are missing */
                    if (mp_normalcount > mp_pointcount / 2)
                        mp_optopcode = OPT_ALL_NORMALS_COMPRESSED;
                    else
                        mp_optopcode = OPT_NORMALS_COMPRESSED;
                }
                else 
            #endif 
            {
                if (mp_normalcount == mp_pointcount)
                    mp_optopcode = OPT_ALL_NORMALS_COMPRESSED;
                else
                    mp_optopcode = OPT_NORMALS_COMPRESSED;
                if (tk.GetTargetVersion() >= 907) {
                    mp_compression_scheme = CS_TRIVIAL_POLAR;
                    mp_bits_per_sample = (unsigned char)(tk.GetNumNormalBits() / 2); // theta and phi encoded separately 
                }
                else {
                    mp_compression_scheme = CS_TRIVIAL;
                    if (tk.GetTargetVersion() >= 650)
                        mp_bits_per_sample = (unsigned char)(tk.GetNumNormalBits() / 3); // xyz encoded separately 
                    else
                        mp_bits_per_sample = (unsigned char)8;
                }
            }
        }
#endif
        if ((status = PutData(tk, mp_optopcode)) != TK_Normal)
            return status;
        mp_substage++;
    }

    switch (mp_optopcode) {
        case OPT_NORMALS_POLAR:
        case OPT_NORMALS: {
            if ((status = write_vertex_normals(tk)) != TK_Normal)
                return status;
            mp_substage = 0;       
        } break;

        case OPT_ALL_NORMALS_POLAR:
        case OPT_ALL_NORMALS: {
            if ((status = write_vertex_normals_all(tk)) != TK_Normal)
                return status;
            mp_substage = 0;       
        } break;

        case OPT_NORMALS_COMPRESSED: {
            if ((status = write_vertex_normals_compressed(tk)) != TK_Normal)
                return status;
            mp_substage = 0;       
        } break;

        case OPT_ALL_NORMALS_COMPRESSED: {
            if ((status = write_vertex_normals_compressed_all(tk)) != TK_Normal)
                return status;
            mp_substage = 0;       
        } break;

        default:
            return tk.Error("internal error from TK_Polyhedron::write_vertex_normals_main");

    }
    return status;
#else
    return tk.Error (stream_readonly);
#endif
} //end function TK_Polyhedron::write_vertex_normals_main


TK_Status TK_Polyhedron::write_vertex_parameters_all (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_READONLY
    TK_Status       status = TK_Normal;

#ifndef BSTREAM_DISABLE_ASCII
	if( tk.GetAsciiMode() ) 
		return write_vertex_parameters_all_ascii (tk);
#endif

    ASSERT(mp_optopcode == OPT_ALL_PARAMETERS_COMPRESSED ||
            mp_optopcode == OPT_ALL_PARAMETERS);
    if (mp_optopcode == OPT_ALL_PARAMETERS) {
        switch (mp_substage) {

            //case 0: handled external to this switch

            //parameter width
            case 1: {
                if ((status = PutData(tk, mp_paramwidth)) != TK_Normal)
                    return status;
                mp_substage++;
            }   nobreak;

            //all parameters 
            case 2: {
                if ((status = PutData(tk, mp_params, mp_paramwidth * mp_pointcount)) != TK_Normal)
                    return status;
                mp_substage = 0;
            } break;

            default:
                return tk.Error("internal error from TK_Polyhedron::write_vertex_parameters_all (uncompressed)");
	}

    }
    else if (tk.GetTargetVersion() >= 650) {
        switch (mp_substage) {

            //case 0: handled external to this switch

            //compression scheme
            case 1: {
                ASSERT(mp_compression_scheme == CS_TRIVIAL);
                if ((status = PutData(tk, mp_compression_scheme)) != TK_Normal)
                    return status;
                mp_substage++;
            }   nobreak;

            //allocate and fill the workspace
            case 2: {
                status = quantize_and_pack_floats(
                                tk, 
                                mp_pointcount, 
                                mp_paramwidth, 
                                mp_params,                  
                                null, 
                                mp_bits_per_sample,                  
                                mp_bbox, 
                                &mp_workspace_allocated, 
                                &mp_workspace_used, 
                                &mp_workspace);
                if (status != TK_Normal)
                    return status;
                mp_substage++;
            } nobreak;

            //parameter width
            case 3: {
                if (tk.GetTargetVersion() >= 1175) {
                    if ((status = PutData(tk, mp_paramwidth)) != TK_Normal)
                        return status;

                    _W3DTK_REQUIRE_VERSION( 1175 );

                }
                mp_substage++;
            }   nobreak;

            //the bounding box (note: order different from ver<650)
            case 4: {
                if ((status = PutData(tk, mp_bbox, 2*mp_paramwidth)) != TK_Normal)
                    return status;
                mp_substage++;
            } nobreak;

            //compression size
            case 5: {
                if ((status = PutData(tk, mp_bits_per_sample)) != TK_Normal)
                    return status;
                mp_substage++;
            } nobreak;

            //the workspace length
            case 6: {
                if ((status = PutData(tk, mp_workspace_used)) != TK_Normal)
                    return status;
                mp_substage++;
            } nobreak;

            //all parameters 
            case 7: {
                if ((status = PutData(tk, mp_workspace, mp_workspace_used)) != TK_Normal)
                    return status;
                mp_substage = 0;
            } break;

            default:
                return tk.Error("internal error from TK_Polyhedron::write_vertex_parameters_all");
        }
    }
    else {
        mp_paramwidth = 3;
        switch (mp_substage) {

            //case 0: handled external to this switch

            //compression scheme
            case 1: {
                ASSERT(mp_compression_scheme == CS_TRIVIAL);
                if ((status = PutData(tk, mp_compression_scheme)) != TK_Normal)
                    return status;
                mp_substage++;
            }   nobreak;

            //compression size
            case 2: {
                ASSERT(mp_bits_per_sample == 8);
                if ((status = PutData(tk, mp_bits_per_sample)) != TK_Normal)
                    return status;
                mp_substage++;
            } nobreak;

            //allocate and fill the workspace
            case 3: {
                status = trivial_compress_points(
                                tk, 
                                mp_pointcount, 
                                mp_params,                    
                                null, //bounding box is unknown for parameters
                                mp_exists, 
                                Vertex_Parameter, 
                                &mp_workspace_allocated, 
                                &mp_workspace_used, 
                                &mp_workspace, 
                                mp_bbox);
                if (status != TK_Normal)
                    return status;
                mp_substage++;
            } nobreak;

            //the bounding box (note: order different from ver>=650)
            case 4: {
                if ((status = PutData(tk, mp_bbox, 2*mp_paramwidth)) != TK_Normal)
                    return status;
                mp_substage++;
            } nobreak;

            //all parameters 
            case 5: {
                if ((status = PutData(tk, mp_workspace, mp_workspace_used)) != TK_Normal)
                    return status;
                mp_substage = 0;
            } break;

            default:
                return tk.Error("internal error from TK_Polyhedron::write_vertex_parameters_all");
        }
    }
    return status;
#else
    return tk.Error (stream_readonly);
#endif
} // end function write_vertex_parameters_all


TK_Status TK_Polyhedron::write_vertex_parameters (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_READONLY
    TK_Status       status = TK_Normal;

	
#ifndef BSTREAM_DISABLE_ASCII
	if( tk.GetAsciiMode() ) 
		return write_vertex_parameters_ascii (tk);
#endif

    ASSERT(mp_optopcode == OPT_PARAMETERS_COMPRESSED ||
            mp_optopcode == OPT_PARAMETERS);
    if (mp_optopcode == OPT_PARAMETERS) {
        switch (mp_substage) {
            //case 0: handled external to this switch

            //number of parameters on each vertex
            case 1: {
                if ((status = PutData(tk, mp_paramwidth)) != TK_Normal)
                    return status;
                mp_substage++;
            } nobreak;

            //if all points do NOT have parameters, tell how many
            case 2: {
                if (mp_pointcount < 256) {
                    unsigned char       byte = (unsigned char)mp_paramcount;

                    if ((status = PutData(tk, byte)) != TK_Normal)
                        return status;
                }
                else if (mp_pointcount < 65536) {
                    unsigned short      word = (unsigned short)mp_paramcount;

                    if ((status = PutData(tk, word)) != TK_Normal)
                        return status;
                }
                else {
                    if ((status = PutData(tk, mp_paramcount)) != TK_Normal)
                        return status;
                }
                mp_progress = 0;
                mp_substage++;
            }   nobreak;

            //and indices of existing parameters
            case 3: {
                while (mp_progress < mp_pointcount) {
                    if (mp_exists[mp_progress] & Vertex_Parameter) {
                        if (mp_pointcount < 256) {
                            unsigned char       byte = (unsigned char)mp_progress;

                            if ((status = PutData(tk, byte)) != TK_Normal)
                                return status;
                        }
                        else if (mp_pointcount < 65536) {
                            unsigned short      word = (unsigned short)mp_progress;

                            if ((status = PutData(tk, word)) != TK_Normal)
                                return status;
                        }
                        else {
                            if ((status = PutData(tk, mp_progress)) != TK_Normal)
                                return status;
                        }
                    }
                    mp_progress++;
                }
                mp_progress = 0;
                mp_substage++;
            } nobreak;

            //write parameter data
            case 4: {
                while (mp_progress < mp_pointcount) {
                    if (mp_exists[mp_progress] & Vertex_Parameter) {
                        status = PutData(tk, &mp_params[mp_paramwidth*mp_progress], mp_paramwidth);
                        if (status != TK_Normal)
                            return status;
                    }
                    mp_progress++;
                }
                mp_progress = 0;
                mp_substage = 0;
            } break;

            default:
                return tk.Error("internal error from TK_Polyhedron::write_vertex_parameters (uncompressed)");
        }
    }
	else if (tk.GetTargetVersion() >= 650) {
        switch (mp_substage) {

            //case 0: handled external to this switch

            //compression scheme
            case 1: {
                ASSERT(mp_compression_scheme == CS_TRIVIAL);
                if ((status = PutData(tk, mp_compression_scheme)) != TK_Normal)
                    return status;
                mp_substage++;
            }   nobreak;

            //allocate and fill the workspace
            case 2: {
                status = quantize_and_pack_floats(
                                tk, 
                                mp_pointcount, 
                                mp_paramwidth, 
                                mp_params,          
                                null, 
                                mp_exists, 
                                Vertex_Parameter, 
                                mp_bits_per_sample,     
                                mp_bbox, 
                                &mp_workspace_allocated, 
                                &mp_workspace_used, 
                                &mp_workspace);
                if (status != TK_Normal)
                    return status;
                mp_substage++;
            } nobreak;

            //number of parameters on each vertex
            case 3: {
                if (tk.GetTargetVersion() >= 1175) {
                    if ((status = PutData(tk, mp_paramwidth)) != TK_Normal)
                        return status;

                    _W3DTK_REQUIRE_VERSION( 1175 );
                }
                mp_substage++;
            } nobreak;

            //the bounding box (note: order different from ver<650)
            case 4: {
                if ((status = PutData(tk, mp_bbox, 2*mp_paramwidth)) != TK_Normal)
                    return status;
                mp_substage++;
            } nobreak;

            //compression size
            case 5: {
                if ((status = PutData(tk, mp_bits_per_sample)) != TK_Normal)
                    return status;
                mp_substage++;
            } nobreak;

            //if all points do NOT have parameters, tell how many
            case 6: {
                if (mp_pointcount < 256) {
                    unsigned char       byte = (unsigned char)mp_paramcount;

                    if ((status = PutData(tk, byte)) != TK_Normal)
                        return status;
                }
                else if (mp_pointcount < 65536) {
                    unsigned short      word = (unsigned short)mp_paramcount;

                    if ((status = PutData(tk, word)) != TK_Normal)
                        return status;
                }
                else {
                    if ((status = PutData(tk, mp_paramcount)) != TK_Normal)
                        return status;
                }
                mp_progress = 0;
                mp_substage++;
            }   nobreak;

            //and indices of existing parameters
            case 7: {
                while (mp_progress < mp_pointcount) {
                    if (mp_exists[mp_progress] & Vertex_Parameter) {
                        if (mp_pointcount < 256) {
                            unsigned char       byte = (unsigned char)mp_progress;

                            if ((status = PutData(tk, byte)) != TK_Normal)
                                return status;
                        }
                        else if (mp_pointcount < 65536) {
                            unsigned short      word = (unsigned short)mp_progress;

                            if ((status = PutData(tk, word)) != TK_Normal)
                                return status;
                        }
                        else {
                            if ((status = PutData(tk, mp_progress)) != TK_Normal)
                                return status;
                        }
                    }
                    mp_progress++;
                }
                mp_progress = 0;
                mp_substage++;
            } nobreak;

            //workspace length
            case 8: {
                if ((status = PutData(tk, mp_workspace_used)) != TK_Normal)
                    return status;
                mp_substage++;
            } nobreak;

            //get parameter data
            case 9: {
                if ((status = PutData(tk, mp_workspace,  mp_workspace_used)) != TK_Normal)
                    return status;
                mp_progress = 0;
                mp_substage = 0;
            } break;

            default:
                return tk.Error("internal error from TK_Polyhedron::write_vertex_parameters");
        }
    }
    else {
        mp_paramwidth = 3;
        switch (mp_substage) {

            //case 0: handled external to this switch

            //compression scheme
            case 1: {
                ASSERT(mp_compression_scheme == CS_TRIVIAL);
                if ((status = PutData(tk, mp_compression_scheme)) != TK_Normal)
                    return status;
                mp_substage++;
            }   nobreak;

            //compression size
            case 2: {
                if ((status = PutData(tk, mp_bits_per_sample)) != TK_Normal)
                    return status;
                mp_substage++;
            } nobreak;

            //parameter width
            case 3: {
                if (tk.GetTargetVersion() >= 1175) {
                    if ((status = PutData(tk, mp_paramcount)) != TK_Normal)
                        return status;

                    _W3DTK_REQUIRE_VERSION( 1175 );
                }
                mp_substage++;
            } nobreak;

            //allocate and fill the workspace
            case 4: {
                ASSERT(mp_compression_scheme == CS_TRIVIAL);
                status = trivial_compress_points(
                                tk, 
                                mp_pointcount, 
                                mp_params,                           
                                null, 
                                mp_exists, 
                                Vertex_Parameter, 
                                &mp_workspace_allocated, 
                                &mp_workspace_used, 
                                &mp_workspace, 
                                mp_bbox);
                if (status != TK_Normal)
                    return status;
                mp_substage++;
            } nobreak;

            //the bounding box (note: order different from ver>=650)
            case 5: {
                if ((status = PutData(tk, mp_bbox, 2*mp_paramwidth)) != TK_Normal)
                    return status;
                mp_substage++;
            } nobreak;

            //if all points do NOT have parameters, tell how many
            case 6: {
                if (mp_pointcount < 256) {
                    unsigned char       byte = (unsigned char)mp_paramcount;

                    if ((status = PutData(tk, byte)) != TK_Normal)
                        return status;
                }
                else if (mp_pointcount < 65536) {
                    unsigned short      word = (unsigned short)mp_paramcount;

                    if ((status = PutData(tk, word)) != TK_Normal)
                        return status;
                }
                else {
                    if ((status = PutData(tk, mp_paramcount)) != TK_Normal)
                        return status;
                }
                mp_progress = 0;
                mp_substage++;
            }   nobreak;

            //and indices of existing parameters
            case 7: {
                while (mp_progress < mp_pointcount) {
                    if (mp_exists[mp_progress] & Vertex_Parameter) {
                        if (mp_pointcount < 256) {
                            unsigned char       byte = (unsigned char)mp_progress;

                            if ((status = PutData(tk, byte)) != TK_Normal)
                                return status;
                        }
                        else if (mp_pointcount < 65536) {
                            unsigned short      word = (unsigned short)mp_progress;

                            if ((status = PutData(tk, word)) != TK_Normal)
                                return status;
                        }
                        else {
                            if ((status = PutData(tk, mp_progress)) != TK_Normal)
                                return status;
                        }
                    }
                    mp_progress++;
                }
                mp_progress = 0;
                mp_substage++;
            } nobreak;

            //put individual parameters
            case 8: {
                while (mp_progress < mp_pointcount) {
                    if (mp_exists[mp_progress] & Vertex_Parameter) {
                        if ((status = PutData(tk, &mp_workspace[3*mp_progress], 3)) != TK_Normal)
                            return status;
                    }
                    mp_progress++;
                }
                mp_progress = 0;
                mp_substage = 0;
            } break;

            default:
                return tk.Error("internal error from TK_Polyhedron::write_vertex_parameters (ver<650)");
        }
    }
    return status;
#else
    return tk.Error (stream_readonly);
#endif
} //end function write_vertex_parameters


TK_Status TK_Polyhedron::write_vertex_parameters_main (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_READONLY
    TK_Status       status = TK_Normal;

		
#ifndef BSTREAM_DISABLE_ASCII
	if( tk.GetAsciiMode() ) 
		return write_vertex_parameters_main_ascii (tk);
#endif

    if (mp_substage == 0) {
        if (mp_paramcount > 0) {
            if ((tk.GetWriteFlags() & TK_Full_Resolution_Parameters) && 
                tk.GetTargetVersion() >= 1175) {
                _W3DTK_REQUIRE_VERSION( 1175 );
                mp_compression_scheme = (unsigned char) CS_NONE;
                if (mp_paramcount == mp_pointcount) {
                    mp_optopcode = OPT_ALL_PARAMETERS;
                }
                else
                    mp_optopcode = OPT_PARAMETERS;
            }
            else {
                mp_compression_scheme = (unsigned char) CS_TRIVIAL;
                if (mp_paramcount == mp_pointcount) {
                    mp_optopcode = OPT_ALL_PARAMETERS_COMPRESSED;
                }
                else
                    mp_optopcode = OPT_PARAMETERS_COMPRESSED;
            }

            if ((status = PutData(tk, mp_optopcode)) != TK_Normal)
                return status;
        }
        if (tk.GetTargetVersion() >= 650)
        //regardless of the number of parameters that there actually are, always divide by 3
            mp_bits_per_sample = (unsigned char)(tk.GetNumParameterBits() / 3); 
        else
            mp_bits_per_sample = 8;
        mp_substage++;
    }

    switch (mp_optopcode) {
        case OPT_PARAMETERS:
        case OPT_PARAMETERS_COMPRESSED: {
            if ((status = write_vertex_parameters(tk)) != TK_Normal)
                return status;
            mp_substage = 0;       
        } break;

        case OPT_ALL_PARAMETERS:
        case OPT_ALL_PARAMETERS_COMPRESSED: {
            if ((status = write_vertex_parameters_all(tk)) != TK_Normal)
                return status;
            mp_substage = 0;       
        } break;

        default:
            return tk.Error("internal error in write_vertex_parameters_main");
    }
    return status;
#else
    return tk.Error (stream_readonly);
#endif
} //end function TK_Polyhedron::write_vertex_parameters_main


TK_Status TK_Polyhedron::write_face_colors_all (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_READONLY
    TK_Status       status = TK_Normal;
		
#ifndef BSTREAM_DISABLE_ASCII
	if( tk.GetAsciiMode() ) 
		return write_face_colors_all_ascii (tk);
#endif

    if (tk.GetTargetVersion() >= 650) {
        switch (mp_substage) {

            //case 0: handled external to this switch

            //compression scheme
            case 1: {
                ASSERT(mp_compression_scheme == CS_TRIVIAL);
                if ((status = PutData(tk, mp_compression_scheme)) != TK_Normal)
                    return status;
                mp_substage++;
            }   nobreak;

            //allocate and fill the workspace
            case 2: {
                status = quantize_and_pack_floats(
                                tk, 
                                mp_facecount, 
                                3, 
                                mp_fcolors,                            
                                color_cube, 
                                mp_face_exists, 
                                Face_Color, 
                                mp_bits_per_sample,                          
                                null, 
                                &mp_workspace_allocated, 
                                &mp_workspace_used, 
                                &mp_workspace);
                if (status != TK_Normal)
                    return status;
                mp_substage++;
            } nobreak;

            //compression size
            case 3: {
                if ((status = PutData(tk, mp_bits_per_sample)) != TK_Normal)
                    return status;
                mp_substage++;
            } nobreak;

            //the workspace length
            case 4: {
                if ((status = PutData(tk, mp_workspace_used)) != TK_Normal)
                    return status;
                mp_substage++;
            } nobreak;

            //all colors
            case 5: {
                if ((status = PutData(tk, mp_workspace, mp_workspace_used)) != TK_Normal)
                    return status;
                mp_substage = 0;
            } break;

            default:
                return tk.Error("internal error in function TK_Polyhedron::write_face_colors (all)");
        }
    }
    else {
        switch (mp_substage) {

            //case 0: handled external to this switch

            //allocate and fill the workspace
            case 1: {
                ASSERT(mp_compression_scheme == CS_TRIVIAL);
                status = trivial_compress_points(
                                    tk, 
                                    mp_facecount, 
                                    mp_fcolors,     
                                    color_cube, 
                                    null, 
                                    0, 
                                    &mp_workspace_allocated, 
                                    &mp_workspace_used, 
                                    &mp_workspace, 
                                    null);
                if (status != TK_Normal)
                    return status;
                mp_substage++;
            } nobreak;

            //all colors
            case 2: {
                if ((status = PutData(tk, mp_workspace, mp_workspace_used)) != TK_Normal)
                    return status;
                mp_substage = 0;
            } break;

            default:
                return tk.Error("internal error in function TK_Polyhedron::write_face_colors (all, version<650)");
        }
    }
    return status;
#else
    return tk.Error (stream_readonly);
#endif
} //end function TK_Polyhedron::write_face_colors_all


TK_Status TK_Polyhedron::write_face_colors (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_READONLY
    TK_Status       status = TK_Normal;
    int             count;

#ifndef BSTREAM_DISABLE_ASCII
	if( tk.GetAsciiMode() ) 
		return write_face_colors_ascii (tk);
#endif

    if (tk.GetTargetVersion() >= 650) {
        switch (mp_substage) {

            //case 0: handled external to this switch

            //compression scheme
            case 1: {
                ASSERT(mp_compression_scheme == CS_TRIVIAL);
                if ((status = PutData(tk, mp_compression_scheme)) != TK_Normal)
                    return status;
                mp_substage++;
            }   nobreak;

            //allocate and fill the workspace
            case 2: {
                status = quantize_and_pack_floats(
                                tk, 
                                mp_facecount, 
                                3, 
                                mp_fcolors,            
                                color_cube, 
                                mp_face_exists, 
                                Face_Color, 
                                mp_bits_per_sample,                     
                                null, 
                                &mp_workspace_allocated,
                                &mp_workspace_used,
                                &mp_workspace);
                if (status != TK_Normal)
                    return status;
                mp_substage++;
            } nobreak;

            //compression size
            case 3: {
                if ((status = PutData(tk, mp_bits_per_sample)) != TK_Normal)
                    return status;
                mp_substage++;
            } nobreak;

            //if all faces do NOT have colors, tell how many
            case 4: {
                count = mp_fcolorcount;
                if (mp_facecount < 256) {
                    unsigned char       byte = (unsigned char)count;

                    if ((status = PutData(tk, byte)) != TK_Normal)
                        return status;
                }
                else if (mp_facecount < 65536) {
                    unsigned short      word = (unsigned short)count;

                    if ((status = PutData(tk, word)) != TK_Normal)
                        return status;
                }
                else {
                    if ((status = PutData(tk, count)) != TK_Normal)
                        return status;
                }
                mp_progress = 0;
                mp_substage++;
            }   nobreak;

            //and indices of existing colors
            case 5: {
                while (mp_progress < mp_facecount) {
                    if ((mp_face_exists[mp_progress] & Face_Color) == Face_Color) {
                        if (mp_facecount < 256) {
                            unsigned char       byte = (unsigned char)mp_progress;

                            if ((status = PutData(tk, byte)) != TK_Normal)
                                return status;
                        }
                        else if (mp_facecount < 65536) {
                            unsigned short      word = (unsigned short)mp_progress;

                            if ((status = PutData(tk, word)) != TK_Normal)
                                return status;
                        }
                        else {
                            if ((status = PutData(tk, mp_progress)) != TK_Normal)
                                return status;
                        }
                    }
                    mp_progress++;
                }
                mp_progress = 0;
                mp_substage++;
            } nobreak;

            //the workspace length
            case 6: {
                if ((status = PutData(tk, mp_workspace_used)) != TK_Normal)
                    return status;
                mp_substage++;
            } nobreak;

            //the color data
            case 7: {
                if ((status = PutData(tk, mp_workspace, mp_workspace_used)) != TK_Normal)
                    return status;
                mp_substage = 0;
            } break;

            default:
                return tk.Error("internal error in function TK_Polyhedron::write_face_colors (partial, version>650)");
        }
    }
    else {
        switch (mp_substage) {

            //case 0: handled external to this switch

            //if all faces do NOT have colors, tell how many
            case 1: {
                count = mp_fcolorcount;
                if (mp_facecount < 256) {
                    unsigned char       byte = (unsigned char)count;

                    if ((status = PutData(tk, byte)) != TK_Normal)
                        return status;
                }
                else if (mp_facecount < 65536) {
                    unsigned short      word = (unsigned short)count;

                    if ((status = PutData(tk, word)) != TK_Normal)
                        return status;
                }
                else {
                    if ((status = PutData(tk, count)) != TK_Normal)
                        return status;
                }
                mp_progress = 0;
                mp_substage++;
            }   nobreak;

            //and colors of existing colors
            case 2: {
                while (mp_progress < mp_facecount) {
                    if ((mp_face_exists[mp_progress] & Face_Color) == Face_Color) {
                        if (mp_facecount < 256) {
                            unsigned char       byte = (unsigned char)mp_progress;

                            if ((status = PutData(tk, byte)) != TK_Normal)
                                return status;
                        }
                        else if (mp_facecount < 65536) {
                            unsigned short      word = (unsigned short)mp_progress;

                            if ((status = PutData(tk, word)) != TK_Normal)
                                return status;
                        }
                        else {
                            if ((status = PutData(tk, mp_progress)) != TK_Normal)
                                return status;
                        }
                    }
                    mp_progress++;
                }
                mp_progress = 0;
                mp_substage++;
            } nobreak;

            //allocate and fill the worksapce
            case 3: {
                ASSERT(mp_compression_scheme == CS_TRIVIAL);
                status = trivial_compress_points(
                                tk, 
                                mp_facecount, 
                                mp_fcolors,         
                                color_cube, 
                                mp_face_exists, 
                                Face_Color, 
                                &mp_workspace_allocated, 
                                &mp_workspace_used, 
                                &mp_workspace, 
                                null);
                if (status != TK_Normal)
                    return status;
                mp_substage++;
            } nobreak;

            //the individual colors
            case 4: {
                while (mp_progress < mp_facecount) {
                    if ((mp_face_exists[mp_progress] & Face_Color) == Face_Color) {
                        if ((status = PutData(tk, &mp_workspace[3 * mp_progress], 3)) != TK_Normal)
                            return status;
                    }
                    mp_progress++;
                }
                mp_progress = 0;
                mp_substage = 0;
            } break;

            default:
                return tk.Error("internal error in function TK_Polyhedron::write_face_colors");
        }
    }
    return status;
#else
    return tk.Error (stream_readonly);
#endif
} //end function TK_Polyhedron::write_face_colors


TK_Status TK_Polyhedron::write_face_colors_main (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_READONLY
    TK_Status       status = TK_Normal;


#ifndef BSTREAM_DISABLE_ASCII
	if( tk.GetAsciiMode() ) 
		return write_face_colors_main_ascii (tk);
#endif

    if (mp_substage == 0) {
        if (mp_fcolorcount == mp_facecount) {
            // all faces are assigned colors
            mp_optopcode = (unsigned char)OPT_ALL_FACE_COLORS;
        }
        else {
            // at least one face has no assigned color
            mp_optopcode = (unsigned char)OPT_FACE_COLORS;
        }
        if (tk.GetTargetVersion() >= 650)
            mp_bits_per_sample = (unsigned char)(tk.GetNumColorBits() / 3);
        else 
            mp_bits_per_sample = (unsigned char)8;

        if ((status = PutData(tk, mp_optopcode)) != TK_Normal)
            return status;
        mp_substage++;
    }

    if (mp_optopcode == OPT_ALL_FACE_COLORS) {
        if ((status = write_face_colors_all(tk)) != TK_Normal)
            return status;
    }
    else {
        if ((status = write_face_colors(tk)) != TK_Normal)
            return status;
    }
    mp_substage = 0;
    return status;
#else
    return tk.Error (stream_readonly);
#endif
} //end function TK_Polyhedron::write_face_colors_main


TK_Status TK_Polyhedron::write_face_indices_all (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_READONLY
    TK_Status       status = TK_Normal;

	
#ifndef BSTREAM_DISABLE_ASCII
	if( tk.GetAsciiMode() ) 
		return write_face_indices_all_ascii (tk);
#endif

    if (tk.GetTargetVersion() >= 650) {
        switch (mp_substage) {

            //case 0: handled external to this switch

            //compression scheme
            case 1: {
                ASSERT(mp_compression_scheme == CS_TRIVIAL);
                if ((status = PutData(tk, mp_compression_scheme)) != TK_Normal)
                    return status;
                mp_substage++;
            }   nobreak;

            //allocate and fill the workspace
            case 2: {
                status = quantize_and_pack_floats(
                                tk, 
                                mp_facecount, 
                                1, 
                                mp_findices,         
                                null, 
                                mp_face_exists, 
                                Face_Index, 
                                mp_bits_per_sample,                
                                mp_bbox, 
                                &mp_workspace_allocated, 
                                &mp_workspace_used, 
                                &mp_workspace);
                if (status != TK_Normal)
                    return status;
                mp_substage++;
            } nobreak;

            //bounding box
            case 3: {
                if ((status = PutData(tk, mp_bbox, 6)) != TK_Normal)
                    return status;
                mp_substage++;
            } nobreak;

            //compression size
            case 4: {
                if ((status = PutData(tk, mp_bits_per_sample)) != TK_Normal)
                    return status;
                mp_substage++;
            } nobreak;

            //the workspace length
            case 5: {
                if ((status = PutData(tk, mp_workspace_used)) != TK_Normal)
                    return status;
                mp_substage++;
            } nobreak;

            //all indices
            case 6: {
                if ((status = PutData(tk, mp_workspace, mp_workspace_used)) != TK_Normal)
                    return status;
                mp_substage = 0;
            } break;

            default:
                return tk.Error("internal error in function TK_Polyhedron::write_face_indices (all)");
        }
    }
    else {
        switch (mp_substage) {

            //case 0: handled external to this switch

            //all indices
            case 1: {
                if ((status = PutData(tk, mp_findices, mp_facecount)) != TK_Normal)
                    return status;
                mp_substage = 0;
            } break;

            default:
                return tk.Error("internal error in function TK_Polyhedron::write_face_indices (all, version<650)");
        }
    }
    return status;
#else
    return tk.Error (stream_readonly);
#endif
} //end function TK_Polyhedron::write_face_indices_all


TK_Status TK_Polyhedron::write_face_indices (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_READONLY
    TK_Status       status = TK_Normal;
    int             count;
	
#ifndef BSTREAM_DISABLE_ASCII
	if( tk.GetAsciiMode() ) 
		return write_face_indices_ascii (tk);
#endif

    if (tk.GetTargetVersion() >= 650) {
        switch (mp_substage) {

            //case 0: handled external to this switch

            //compression scheme
            case 1: {
                ASSERT(mp_compression_scheme == CS_TRIVIAL);
                if ((status = PutData(tk, mp_compression_scheme)) != TK_Normal)
                    return status;
                mp_substage++;
            }   nobreak;

            //allocate and fill the workspace
            case 2: {
                status = quantize_and_pack_floats(
                                tk, 
                                mp_facecount, 
                                1, 
                                mp_findices,                   
                                null, 
                                mp_face_exists, 
                                Face_Index, 
                                mp_bits_per_sample,                    
                                mp_bbox, 
                                &mp_workspace_allocated, 
                                &mp_workspace_used, 
                                &mp_workspace);
                if (status != TK_Normal)
                    return status;
                mp_substage++;
            } nobreak;

            //compression size
            case 3: {
                if ((status = PutData(tk, mp_bbox, 6)) != TK_Normal)
                    return status;
                mp_substage++;
            } nobreak;

            //compression size
            case 4: {
                if ((status = PutData(tk, mp_bits_per_sample)) != TK_Normal)
                    return status;
                mp_substage++;
            } nobreak;

            //if all faces do NOT have indices, tell how many
            case 5: {
                count = mp_findexcount;
                if (mp_facecount < 256) {
                    unsigned char       byte = (unsigned char)count;

                    if ((status = PutData(tk, byte)) != TK_Normal)
                        return status;
                }
                else if (mp_facecount < 65536) {
                    unsigned short      word = (unsigned short)count;

                    if ((status = PutData(tk, word)) != TK_Normal)
                        return status;
                }
                else {
                    if ((status = PutData(tk, count)) != TK_Normal)
                        return status;
                }
                mp_progress = 0;
                mp_substage++;
            }   nobreak;

            //and indices of existing indices
            case 6: {
                while (mp_progress < mp_facecount) {
                    if ((mp_face_exists[mp_progress] & Face_Index) == Face_Index) {
                        if (mp_facecount < 256) {
                            unsigned char       byte = (unsigned char)mp_progress;

                            if ((status = PutData(tk, byte)) != TK_Normal)
                                return status;
                        }
                        else if (mp_facecount < 65536) {
                            unsigned short      word = (unsigned short)mp_progress;

                            if ((status = PutData(tk, word)) != TK_Normal)
                                return status;
                        }
                        else {
                            if ((status = PutData(tk, mp_progress)) != TK_Normal)
                                return status;
                        }
                    }
                    mp_progress++;
                }
                mp_progress = 0;
                mp_substage++;
            } nobreak;

            //the workspace length
            case 7: {
                if ((status = PutData(tk, mp_workspace_used)) != TK_Normal)
                    return status;
                mp_substage++;
            } nobreak;

            //the index data
            case 8: {
                if ((status = PutData(tk, mp_workspace, mp_workspace_used)) != TK_Normal)
                    return status;
                mp_substage = 0;
            } break;

            default:
                return tk.Error("internal error in function TK_Polyhedron::write_face_indices (partial, version>650)");
        }
    }
    else {
        switch (mp_substage) {

            //case 0: handled external to this switch

            //if all faces do NOT have indices, tell how many
            case 1: {
                count = mp_findexcount;
                if (mp_facecount < 256) {
                    unsigned char       byte = (unsigned char)count;

                    if ((status = PutData(tk, byte)) != TK_Normal)
                        return status;
                }
                else if (mp_facecount < 65536) {
                    unsigned short      word = (unsigned short)count;

                    if ((status = PutData(tk, word)) != TK_Normal)
                        return status;
                }
                else {
                    if ((status = PutData(tk, count)) != TK_Normal)
                        return status;
                }
                mp_progress = 0;
                mp_substage++;
            }   nobreak;

            //and indices of existing indices
            case 2: {
                while (mp_progress < mp_facecount) {
                    if ((mp_face_exists[mp_progress] & Face_Index) == Face_Index) {
                        if (mp_facecount < 256) {
                            unsigned char       byte = (unsigned char)mp_progress;

                            if ((status = PutData(tk, byte)) != TK_Normal)
                                return status;
                        }
                        else if (mp_facecount < 65536) {
                            unsigned short      word = (unsigned short)mp_progress;

                            if ((status = PutData(tk, word)) != TK_Normal)
                                return status;
                        }
                        else {
                            if ((status = PutData(tk, mp_progress)) != TK_Normal)
                                return status;
                        }
                    }
                    mp_progress++;
                }
                mp_progress = 0;
                mp_substage++;
            } nobreak;

            //the individual indices
            case 3: {
                while (mp_progress < mp_facecount) {
                    if ((mp_face_exists[mp_progress] & Face_Index) == Face_Index) {
                        if ((status = PutData(tk, &mp_findices[mp_progress], 1)) != TK_Normal)
                            return status;
                    }
                    mp_progress++;
                }
                mp_progress = 0;
                mp_substage = 0;
            } break;

            default:
                return tk.Error("internal error in function TK_Polyhedron::write_face_indices");
        }
    }
    return status;
#else
    return tk.Error (stream_readonly);
#endif
} //end function TK_Polyhedron::write_face_indices


TK_Status TK_Polyhedron::write_face_indices_main (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_READONLY
    TK_Status       status = TK_Normal;
		
#ifndef BSTREAM_DISABLE_ASCII
	if( tk.GetAsciiMode() ) 
		return write_face_indices_main_ascii (tk);
#endif

    if (mp_substage == 0) {
        if (mp_findexcount == mp_facecount) {
            // all faces are assigned indices
            mp_optopcode = OPT_ALL_FACE_INDICES;
        }
        else {
            // at least one face has no assigned index
            mp_optopcode = OPT_FACE_INDICES;
        }
        if (tk.GetTargetVersion() >= 650)
            mp_bits_per_sample = (unsigned char)(tk.GetNumIndexBits());
        else 
            mp_bits_per_sample = 32; //this value shouldn't be used, since prior to 650, indices were uncompressed

        if ((status = PutData(tk, mp_optopcode)) != TK_Normal)
            return status;
        mp_substage++;
    }

    if (mp_optopcode == OPT_ALL_FACE_INDICES) {
        if ((status = write_face_indices_all(tk)) != TK_Normal)
            return status;
    }
    else {
        if ((status = write_face_indices(tk)) != TK_Normal)
            return status;
    }
	mp_substage = 0;
    return status;
#else
    return tk.Error (stream_readonly);
#endif
} //end function TK_Polyhedron::write_face_indices_main





TK_Status TK_Polyhedron::write_face_regions (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_READONLY
    TK_Status       status = TK_Normal;
		
#ifndef BSTREAM_DISABLE_ASCII
	if( tk.GetAsciiMode() ) 
		return write_face_regions_ascii (tk);
#endif

    if (tk.GetTargetVersion() < 705)
        return TK_Normal;

    switch (mp_substage) {
        case 0: {
            mp_optopcode = OPT_FACE_REGIONS;
            if ((status = PutData(tk, mp_optopcode)) != TK_Normal)
                return status;
            mp_compression_scheme = 0;

            // analyze data and maybe determine a simple packing...
            // 0:   simple array, 1 region per face
            // 1:   consecutive regions, 0 based, need count of regions and number of faces in each
            // 2:   same as 1, but not zero based, so also need starting index
            // 3:   non-consecutive, possibly disjoint regions (each part looks like a separate region)
            //          needs count of regions, (region number and number of faces) in each
            bool    consecutive = true;
            int     count = 1;
            int     index = mp_fregions[0];
            int     lowest = mp_fregions[0];
            int     highest = mp_fregions[0];
            int     longest = 0;
            int     start = 0;
            int     ii;

            for (ii=1; ii<mp_facecount; ii++) {
                if (mp_fregions[ii] != index) {
                    ++count;

                    if (ii-start > longest)
                        longest = ii-start;
                    start = ii;

                    if (mp_fregions[ii] < lowest)
                        lowest = mp_fregions[ii];
                    else if (mp_fregions[ii] > highest)
                        highest = mp_fregions[ii];

                    if (mp_fregions[ii] != index+1) {
                        consecutive = false;
                    }
                    index = mp_fregions[ii];
                }
            }
            if (mp_facecount-start > longest)
                longest = mp_facecount-start;

            if (consecutive) {
                mp_compression_scheme = 1;
                if (mp_fregions[0] != 0)
                    mp_compression_scheme = 2;
            }
            else {
               if (count < mp_facecount/2) // worth the extra overhead?
                    mp_compression_scheme = 3;
            }

            // mp_compression_scheme:
            //  0: lowest, highest                  (region values)
            //  1: count, longest                   (count and lengths)
            //  2: count, lowest, longest           (count, first index, and lengths)
            //  3: count, lowest, highest, longest  (count, regions values, and lengths)

            // lowest, highest, and/or longest should tell use if we can pack into something smaller than ints
            int     size = 1;   // assume byte

            if (mp_compression_scheme != 1) {
                if (lowest < -65536 || lowest > 65535)
                    size = 4;
                else if (lowest < -128 || lowest > 127)
                    size = 2;
            }
            if (mp_compression_scheme == 0 || mp_compression_scheme == 3) {
                if (highest < -65536 || highest > 65535)
                    size = 4;
                else if (highest < -128 || highest > 127) {
                    if (size < 2)
                        size = 2;
                }
            }
            if (mp_compression_scheme > 0) {
                if (longest < -65536 || longest > 65535 || count > 65535)
                    size = 4;
                else if (longest < -128 || longest > 127 || count > 127) {
                    if (size < 2)
                        size = 2;
                }
            }

            // fill in workspace as an array of ints, which will then
            // be reduced to look like an array of chars or shorts if appropriate
            switch (mp_compression_scheme) {
                case 0: {
                    mp_workspace_used = 4 * mp_facecount;
                    if (mp_workspace_allocated < mp_workspace_used) {
                        mp_workspace_allocated = mp_workspace_used;
                        ENSURE_ALLOCATED (mp_workspace, unsigned char, mp_workspace_allocated);
                    }
                    memcpy (mp_workspace, mp_fregions, mp_workspace_used);
                }   break;

                case 1: {
                    mp_workspace_used = 4*(1 + count);
                    if (mp_workspace_allocated < mp_workspace_used) {
                        mp_workspace_allocated = mp_workspace_used;
                        ENSURE_ALLOCATED (mp_workspace, unsigned char, mp_workspace_allocated);
                    }
                    int * tmp = (int *)mp_workspace;
                    *tmp++ = count;
                    index = mp_fregions[0];
                    start = 0;
                    for (ii=1; ii<mp_facecount; ii++) {
                        if (mp_fregions[ii] != index) {
                            *tmp++ = ii - start;
                            start = ii;
                            index = mp_fregions[ii];
                        }
                    }
                    *tmp++ = mp_facecount - start;
                }   break;

                case 2: {
                    mp_workspace_used = 4*(1 + 1 + count);
                    if (mp_workspace_allocated < mp_workspace_used) {
                        mp_workspace_allocated = mp_workspace_used;
                        ENSURE_ALLOCATED (mp_workspace, unsigned char, mp_workspace_allocated);
                    }
                    int * tmp = (int *)mp_workspace;
                    *tmp++ = count;
                    *tmp++ = index = mp_fregions[0];
                    start = 0;
                    for (ii=1; ii<mp_facecount; ii++) {
                        if (mp_fregions[ii] != index) {
                            *tmp++ = ii - start;
                            start = ii;
                            index = mp_fregions[ii];
                        }
                    }
                    *tmp++ = mp_facecount - start;
                }   break;

                case 3: {
                    mp_workspace_used = 4*(1 + 2 * count);
                    if (mp_workspace_allocated < mp_workspace_used) {
                        mp_workspace_allocated = mp_workspace_used;
                        ENSURE_ALLOCATED (mp_workspace, unsigned char, mp_workspace_allocated);
                    }
                    int * tmp = (int *)mp_workspace;
                    *tmp++ = count;
                    *tmp++ = index = mp_fregions[0];
                    start = 0;
                    for (ii=1; ii<mp_facecount; ii++) {
                        if (mp_fregions[ii] != index) {
                            *tmp++ = ii - start;
                            *tmp++ = index = mp_fregions[ii];
                            start = ii;
                        }
                    }
                    *tmp++ = mp_facecount - start;
                }   break;
            }

            // compress ints if appropriate
            if (size == 1) {
                mp_compression_scheme |= 8;
                int * tmp = (int *)mp_workspace;
                char * cmp = (char *)mp_workspace;
                for (ii=0; ii<mp_workspace_used; ii += 4)
                    *cmp++ = (char)*tmp++;
            }
            else if (size == 2) {
                mp_compression_scheme |= 16;
                int * tmp = (int *)mp_workspace;
                short * cmp = (short *)mp_workspace;
                for (ii=0; ii<mp_workspace_used; ii += 4)
                    *cmp++ = (short)*tmp++;
            }

            mp_substage++;
        }   nobreak;

        case 1: {
            if ((status = PutData(tk, mp_compression_scheme)) != TK_Normal)
                return status;
            mp_substage++;
        }   nobreak;

        case 2: {
            switch (mp_compression_scheme & 0x18) {    // size of each value    
                case 8: {
                    if ((status = PutData(tk, mp_workspace, mp_workspace_used/4)) != TK_Normal)
                        return status;
                }   break;

                case 16: {
                    if ((status = PutData(tk, (short *)mp_workspace, mp_workspace_used/4)) != TK_Normal)
                        return status;
                }   break;

                case 0: {
                    if ((status = PutData(tk, (int *)mp_workspace, mp_workspace_used/4)) != TK_Normal)
                        return status;
                }   break;
            }

            mp_substage = 0;
        }   break;

        default:
            return tk.Error ("internal error in TK_Polyhedron::write_face_regions");
    }

    return status;
#else
    return tk.Error (stream_readonly);
#endif
} //end function TK_Polyhedron::write_face_regions



TK_Status TK_Polyhedron::write_vertex_colors_all (BStreamFileToolkit & tk, unsigned char op) alter {
#ifndef BSTREAM_READONLY
    TK_Status       status = TK_Normal;
    float           *array;
    unsigned int    mask;

			
#ifndef BSTREAM_DISABLE_ASCII
	if( tk.GetAsciiMode() ) 
		return write_vertex_colors_all_ascii (tk,op);
#endif


    switch (op) {
        case OPT_ALL_VFCOLORS:
            array = mp_vfcolors;
            mask = Vertex_Face_Color;
            break;
        case OPT_ALL_VECOLORS:
            array = mp_vecolors;
            mask = Vertex_Edge_Color;
            break;
        case OPT_ALL_VMCOLORS:
            array = mp_vmcolors;
            mask = Vertex_Marker_Color;
            break;
        default:
            return tk.Error("internal error in write_vertex_colors_all: unrecognized case");
    }

    if (tk.GetTargetVersion() >= 650) {
        switch (mp_substage) {
            //compression scheme
            case 1: {
                ASSERT(mp_compression_scheme == CS_TRIVIAL);
                if ((status = PutData(tk, mp_compression_scheme)) != TK_Normal)
                    return status;
                mp_substage++;
            }   nobreak;

            //bits per sample
            case 2: {
                if ((status = PutData(tk, mp_bits_per_sample)) != TK_Normal)
                    return status;
                mp_substage++;
            } nobreak;

            //allocate and fill the workspace
            case 3: {
                status = quantize_and_pack_floats(
                                tk, 
                                mp_pointcount, 
                                3, 
                                array,      
                                color_cube, 
                                mp_exists, 
                                mask, 
                                mp_bits_per_sample,                      
                                null, 
                                &mp_workspace_allocated, 
                                &mp_workspace_used, 
                                &mp_workspace);
                if (status != TK_Normal)
                    return status;
                mp_substage++;
            } nobreak;

            //workspace length
            case 4: {
                if ((status = PutData(tk, mp_workspace_used)) != TK_Normal)
                    return status;
                mp_substage++;
            } nobreak;

            //all colors OR individual colors
            case 5: {
                if ((status = PutData(tk, mp_workspace, mp_workspace_used)) != TK_Normal)
                    return status;
                mp_substage = 0;
            } break;

            default:
                return tk.Error("internal error in write_vertex_colors_all");
        }
    }
    else {
        switch (mp_substage) {
            //allocate and fill the workspace
            case 1: {
                status = trivial_compress_points (
                                tk, 
                                mp_pointcount, 
                                array,                       
                                color_cube, 
                                mp_exists, 
                                mask, 
                                &mp_workspace_allocated, 
                                &mp_workspace_used, 
                                &mp_workspace, 
                                null);
                if (status != TK_Normal)
                    return status;
                mp_substage++;
            } nobreak;

            //all colors
            case 2: {
                if ((status = PutData(tk, mp_workspace, mp_workspace_used)) != TK_Normal)
                    return status;
                mp_substage = 0;
            } break;

            default:
                return tk.Error("internal error in write_vertex_colors_all (version<650)");
        }

    }
    return status;
#else
	UNREFERENCED (op);
    return tk.Error (stream_readonly);
#endif
} //end function TK_Polyhedron::write_vertex_colors_all


TK_Status TK_Polyhedron::write_vertex_colors (BStreamFileToolkit & tk, unsigned char op) alter {
#ifndef BSTREAM_READONLY
    TK_Status       status = TK_Normal;
    int             count;
    float           *array;
    unsigned int    mask;

			
#ifndef BSTREAM_DISABLE_ASCII
	if( tk.GetAsciiMode() ) 
		return write_vertex_colors_ascii (tk,op);
#endif

    switch (op) {
        case OPT_VERTEX_FCOLORS:
            count = mp_vfcolorcount;
            array = mp_vfcolors;
            mask = Vertex_Face_Color;
            break;
        case OPT_VERTEX_ECOLORS:
            count = mp_vecolorcount;
            array = mp_vecolors;
            mask = Vertex_Edge_Color;
            break;
        case OPT_VERTEX_MCOLORS:
            count = mp_vmcolorcount;
            array = mp_vmcolors;
            mask = Vertex_Marker_Color;
            break;
        default:
            return tk.Error("internal error in write_vertex_colors: unrecognized case");
    }

    if (tk.GetTargetVersion() >= 650) {
        switch (mp_substage) {
            //compression scheme
            case 1: {
                ASSERT(mp_compression_scheme == CS_TRIVIAL);
                if ((status = PutData(tk, mp_compression_scheme)) != TK_Normal)
                    return status;
                mp_substage++;
            }   nobreak;

            //bits per sample
            case 2: {
                if ((status = PutData(tk, mp_bits_per_sample)) != TK_Normal)
                    return status;
                mp_substage++;
            } nobreak;

            //the count
            case 3: {
                if (mp_pointcount < 256) {
                    unsigned char       byte = (unsigned char)count;

                    if ((status = PutData(tk, byte)) != TK_Normal)
                        return status;
                }
                else if (mp_pointcount < 65536) {
                    unsigned short      word = (unsigned short)count;

                    if ((status = PutData(tk, word)) != TK_Normal)
                        return status;
                }
                else {
                    if ((status = PutData(tk, count)) != TK_Normal)
                        return status;
                }
                mp_progress = 0;
                mp_substage++;
            }   nobreak;

            //and indices of existing values
            case 4: {
                while (mp_progress < mp_pointcount) {
                    if ((mp_exists[mp_progress] & mask) == mask) {
                        if (mp_pointcount < 256) {
                        unsigned char       byte = (unsigned char)mp_progress;

                        if ((status = PutData(tk, byte)) != TK_Normal)
                            return status;
                    }
                    else if (mp_pointcount < 65536) {
                        unsigned short      word = (unsigned short)mp_progress;

                        if ((status = PutData(tk, word)) != TK_Normal)
                            return status;
                    }
                    else {
                        if ((status = PutData(tk, mp_progress)) != TK_Normal)
                            return status;
                        }
                    }
                    mp_progress++;
                }
                mp_progress = 0;
                mp_substage++;
            } nobreak;

            //allocate and fill the workspace
            case 5: {
                //afterwards, workspacelen will reflect the number of colors that exist,
                //not the pointcount
                status = quantize_and_pack_floats(
                                tk, 
                                mp_pointcount, 
                                3, 
                                array, 
                                color_cube, 
                                mp_exists, 
                                mask, 
                                mp_bits_per_sample,                  
                                null, 
                                &mp_workspace_allocated, 
                                &mp_workspace_used, 
                                &mp_workspace);
                if (status != TK_Normal)
                    return status;
                mp_substage++;
            } nobreak;

            //workspace length
            case 6: {
                if ((status = PutData(tk, mp_workspace_used)) != TK_Normal)
                    return status;
                mp_substage++;
            } nobreak;

            //all colors OR individual colors
            case 7: {
                if ((status = PutData(tk, mp_workspace, mp_workspace_used)) != TK_Normal)
                    return status;
                mp_substage = 0;
            } break;

            default:
                return tk.Error("internal error in TK_Polyhedron::write_vertex_colors");
        }
    }
    else {
        switch (mp_substage) {
            //the count
            case 1: {
                if (mp_pointcount < 256) {
                    unsigned char       byte = (unsigned char)count;

                    if ((status = PutData(tk, byte)) != TK_Normal)
                        return status;
                }
                else if (mp_pointcount < 65536) {
                    unsigned short      word = (unsigned short)count;

                    if ((status = PutData(tk, word)) != TK_Normal)
                        return status;
                }
                else {
                    if ((status = PutData(tk, count)) != TK_Normal)
                        return status;
                }
                mp_progress = 0;
                mp_substage++;
            }   nobreak;

            //and indices of existing values
            case 2: {
                while (mp_progress < mp_pointcount) {
                    if ((mp_exists[mp_progress] & mask) == mask) {
                        if (mp_pointcount < 256) {
                            unsigned char       byte = (unsigned char)mp_progress;

                            if ((status = PutData(tk, byte)) != TK_Normal)
                                return status;
                        }
                        else if (mp_pointcount < 65536) {
                            unsigned short      word = (unsigned short)mp_progress;

                            if ((status = PutData(tk, word)) != TK_Normal)
                                return status;
                        }
                        else {
                            if ((status = PutData(tk, mp_progress)) != TK_Normal)
                                return status;
                        }
                    }
                    mp_progress++;
                }
                mp_progress = 0;
                mp_substage++;
            } nobreak;

            //allocate and fill the workspace
            case 3: {
                status = trivial_compress_points (
                                tk, 
                                mp_pointcount, 
                                array,                  
                                color_cube, 
                                mp_exists, 
                                mask, 
                                &mp_workspace_allocated, 
                                &mp_workspace_used, 
                                &mp_workspace, 
                                null);
                if (status != TK_Normal)
                    return status;
                mp_substage++;
            } nobreak;

            //all colors OR individual colors
            case 4: {
                while (mp_progress < mp_pointcount) {
                    if ((mp_exists[mp_progress] & mask) == mask) {
                        if ((status = PutData(tk, &mp_workspace[3*mp_progress], 3)) != TK_Normal)
                            return status;
                    }
                    mp_progress++;
                }
                mp_progress = 0;
                mp_substage = 0;
            } break;

            default:
                return tk.Error("internal error in TK_Polyhedron::write_vertex_colors (version<650)");
        }
    }
    return status;
#else
	UNREFERENCED (op);
    return tk.Error (stream_readonly);
#endif
} //end function TK_Polyhedron::write_vertex_colors


TK_Status TK_Polyhedron::write_vertex_colors_main (BStreamFileToolkit & tk, unsigned char base_op) alter {
#ifndef BSTREAM_READONLY
    TK_Status       status = TK_Normal;
    int             count;

				
#ifndef BSTREAM_DISABLE_ASCII
	if( tk.GetAsciiMode() ) 
		return write_vertex_colors_main_ascii (tk,base_op);
#endif

    if (mp_substage == 0) {
        /* determine some of the parameters of how the current opcode is to be written */
        switch (base_op) {
            case OPT_ALL_VFCOLORS:
                count = mp_vfcolorcount;
                break;
            case OPT_ALL_VECOLORS:
                count = mp_vecolorcount;
                break;
            case OPT_ALL_VMCOLORS:
                count = mp_vmcolorcount;
                break;
            default:
                return tk.Error("internal error in write_vertex_colors_main: unrecognized case");
        }
        mp_optopcode = base_op;
        /* the following makes the (valid) assumption that 
         * [OPT_VERTEX_FCOLORS,OPT_VERTEX_ECOLORS,OPT_VERTEX_MCOLORS] ==
         * [OPT_ALL_VFCOLORS,OPT_ALL_VECOLORS,OPT_ALL_VMCOLORS] + 1
         * (Odd values indicates an attribute is present on all vertices)
         */
        if (count != mp_pointcount)
            mp_optopcode++;
        if ((status = PutData(tk, mp_optopcode)) != TK_Normal)
            return status;
        if (tk.GetTargetVersion() >= 650)
            mp_bits_per_sample = (unsigned char)(tk.GetNumColorBits()/3);
        else
            mp_bits_per_sample = 8;
        mp_compression_scheme = CS_TRIVIAL;
        mp_substage++;
    }

    if (mp_optopcode == base_op) {
        if ((status = write_vertex_colors_all(tk, mp_optopcode)) != TK_Normal)
            return status;
    }
    else {
        if ((status = write_vertex_colors(tk, mp_optopcode)) != TK_Normal)
            return status;
    }
    return status;
#else
	UNREFERENCED (base_op);
    return tk.Error (stream_readonly);
#endif
} //end function TK_Polyhedron::write_vertex_colors_main



TK_Status TK_Polyhedron::write_vertex_indices_all (BStreamFileToolkit & tk, unsigned char op) alter {
#ifndef BSTREAM_READONLY
    TK_Status       status = TK_Normal;
    float           *array;
    unsigned int    mask;

				
#ifndef BSTREAM_DISABLE_ASCII
	if( tk.GetAsciiMode() ) 
		return write_vertex_indices_all_ascii (tk,op);
#endif

    switch (op) {
        case OPT_ALL_VFINDICES:
            array = mp_vfindices;
            mask = Vertex_Face_Index;
            break;
        case OPT_ALL_VEINDICES:
            array = mp_veindices;
            mask = Vertex_Edge_Index;
            break;
        case OPT_ALL_VMINDICES:
            array = mp_vmindices;
            mask = Vertex_Marker_Index;
            break;
        default:
            return tk.Error("internal error in write_vertex_indices_all: unrecognized case");
    }

    if (tk.GetTargetVersion() >= 650) {
        switch (mp_substage) {
            //compression scheme
            case 1: {
                ASSERT(mp_compression_scheme == CS_TRIVIAL);
                if ((status = PutData(tk, mp_compression_scheme)) != TK_Normal)
                    return status;
                mp_substage++;
            }   nobreak;

            //allocate and fill the workspace
            case 2: {
                status = quantize_and_pack_floats(
                                tk, 
                                mp_pointcount, 
                                1, 
                                array,                            
                                null, 
                                mp_exists, 
                                mask, 
                                mp_bits_per_sample, 
                                mp_bbox, 
                                &mp_workspace_allocated, 
                                &mp_workspace_used, 
                                &mp_workspace);
                if (status != TK_Normal)
                    return status;
                mp_substage++;
            } nobreak;

            //bits per sample
            case 3: {
                if ((status = PutData(tk, mp_bbox, 6)) != TK_Normal)
                    return status;
                mp_substage++;
            } nobreak;

            //bits per sample
            case 4: {
                if ((status = PutData(tk, mp_bits_per_sample)) != TK_Normal)
                    return status;
                mp_substage++;
            } nobreak;

            //workspace length
            case 5: {
                if ((status = PutData(tk, mp_workspace_used)) != TK_Normal)
                    return status;
                mp_substage++;
            } nobreak;

            //all indices OR individual indices
            case 6: {
                if ((status = PutData(tk, mp_workspace, mp_workspace_used)) != TK_Normal)
                    return status;
                mp_substage = 0;
            } break;

            default:
                return tk.Error("internal error in write_vertex_indices_all");
        }
    }
    else {
        switch (mp_substage) {
            //all indices
            case 1: {
                if ((status = PutData(tk, array, mp_pointcount)) != TK_Normal)
                    return status;
                mp_substage = 0;
            } break;

            default:
                return tk.Error("internal error in write_vertex_indices_all (version<650)");
        }

    }
    return status;
#else
	UNREFERENCED (op);
    return tk.Error (stream_readonly);
#endif
} //end function TK_Polyhedron::write_vertex_indices_all


TK_Status TK_Polyhedron::write_vertex_indices (BStreamFileToolkit & tk, unsigned char op) alter {
#ifndef BSTREAM_READONLY
    TK_Status       status = TK_Normal;
    int             count;
    float           *array;
    unsigned int    mask;

#ifndef BSTREAM_DISABLE_ASCII
	if( tk.GetAsciiMode() ) 
		return write_vertex_indices_ascii (tk, op);
#endif

    switch (op) {
        case OPT_VERTEX_FINDICES:
            count = mp_vfindexcount;
            array = mp_vfindices;
            mask = Vertex_Face_Index;
            break;
        case OPT_VERTEX_EINDICES:
            count = mp_veindexcount;
            array = mp_veindices;
            mask = Vertex_Edge_Index;
            break;
        case OPT_VERTEX_MINDICES:
            count = mp_vmindexcount;
            array = mp_vmindices;
            mask = Vertex_Marker_Index;
            break;
        default:
            return tk.Error("internal error in write_vertex_indices: unrecognized case");
    }

    if (tk.GetTargetVersion() >= 650) {
        switch (mp_substage) {
            //compression scheme
            case 1: {
                ASSERT(mp_compression_scheme == CS_TRIVIAL);
                if ((status = PutData(tk, mp_compression_scheme)) != TK_Normal)
                    return status;
                mp_substage++;
            }   nobreak;

            //allocate and fill the workspace
            case 2: {
                //afterwards, workspacelen will reflect the number of indices that exist,
                //not the pointcount
                status = quantize_and_pack_floats(
                                tk, 
                                mp_pointcount, 
                                1, 
                                array, 
                                null, 
                                mp_exists, 
                                mask, 
                                mp_bits_per_sample, 
                                mp_bbox, 
                                &mp_workspace_allocated, 
                                &mp_workspace_used, 
                                &mp_workspace);
                if (status != TK_Normal)
                    return status;
                mp_substage++;
            } nobreak;

            //bounding
            case 3: {
                if ((status = PutData(tk, mp_bbox, 6)) != TK_Normal)
                    return status;
                mp_substage++;
            } nobreak;

            //bits per sample
            case 4: {
                if ((status = PutData(tk, mp_bits_per_sample)) != TK_Normal)
                    return status;
                mp_substage++;
            } nobreak;

            //the count
            case 5: {
                if (mp_pointcount < 256) {
                    unsigned char       byte = (unsigned char)count;

                    if ((status = PutData(tk, byte)) != TK_Normal)
                        return status;
                }
                else if (mp_pointcount < 65536) {
                    unsigned short      word = (unsigned short)count;

                    if ((status = PutData(tk, word)) != TK_Normal)
                        return status;
                }
                else {
                    if ((status = PutData(tk, count)) != TK_Normal)
                        return status;
                }
                mp_progress = 0;
                mp_substage++;
            }   nobreak;

            //and indices of existing values
            case 6: {
                while (mp_progress < mp_pointcount) {
                    if ((mp_exists[mp_progress] & mask) == mask) {
                        if (mp_pointcount < 256) {
                            unsigned char       byte = (unsigned char)mp_progress;

                            if ((status = PutData(tk, byte)) != TK_Normal)
                                return status;
                        }
                        else if (mp_pointcount < 65536) {
                            unsigned short      word = (unsigned short)mp_progress;

                            if ((status = PutData(tk, word)) != TK_Normal)
                                return status;
                        }
                        else {
                            if ((status = PutData(tk, mp_progress)) != TK_Normal)
                                return status;
                        }
                    }
                    mp_progress++;
                }
                mp_progress = 0;
                mp_substage++;
            } nobreak;

            //workspace length
            case 7: {
                if ((status = PutData(tk, mp_workspace_used)) != TK_Normal)
                    return status;
                mp_substage++;
            } nobreak;

            //all indices OR individual indices
            case 8: {
                if ((status = PutData(tk, mp_workspace, mp_workspace_used)) != TK_Normal)
                    return status;
                mp_substage = 0;
            } break;

            default:
                return tk.Error("internal error in TK_Polyhedron::write_vertex_indices");
        }
    }
    else {
        switch (mp_substage) {
            //the count
            case 1: {
                if (mp_pointcount < 256) {
                    unsigned char       byte = (unsigned char)count;

                    if ((status = PutData(tk, byte)) != TK_Normal)
                        return status;
                }
                else if (mp_pointcount < 65536) {
                    unsigned short      word = (unsigned short)count;

                    if ((status = PutData(tk, word)) != TK_Normal)
                        return status;
                }
                else {
                    if ((status = PutData(tk, count)) != TK_Normal)
                        return status;
                }
                mp_progress = 0;
                mp_substage++;
            }   nobreak;

            //and indices of existing values
            case 2: {
                while (mp_progress < mp_pointcount) {
                    if ((mp_exists[mp_progress] & mask) == mask) {
                        if (mp_pointcount < 256) {
                            unsigned char       byte = (unsigned char)mp_progress;

                            if ((status = PutData(tk, byte)) != TK_Normal)
                                return status;
                        }
                        else if (mp_pointcount < 65536) {
                            unsigned short      word = (unsigned short)mp_progress;

                            if ((status = PutData(tk, word)) != TK_Normal)
                                return status;
                        }
                        else {
                            if ((status = PutData(tk, mp_progress)) != TK_Normal)
                                return status;
                        }
                    }
                    mp_progress++;
                }
                mp_progress = 0;
                mp_substage++;
            } nobreak;

            //individual indices
            case 3: {
                while (mp_progress < mp_pointcount) {
                    if ((mp_exists[mp_progress] & mask) == mask) {
                        if ((status = PutData(tk, &array[mp_progress], 1)) != TK_Normal)
                            return status;
                    }
                    mp_progress++;
                }
                mp_progress = 0;
                mp_substage = 0;
            } break;

            default:
                return tk.Error("internal error in TK_Polyhedron::write_vertex_indices (version<650)");
        }
    }
    return status;
#else
	UNREFERENCED (op);
    return tk.Error (stream_readonly);
#endif
} //end function TK_Polyhedron::write_vertex_indices


TK_Status TK_Polyhedron::write_vertex_indices_main (BStreamFileToolkit & tk, unsigned char base_op) alter {
#ifndef BSTREAM_READONLY
    TK_Status       status = TK_Normal;
    int             count;

				
#ifndef BSTREAM_DISABLE_ASCII
	if( tk.GetAsciiMode() ) 
		return write_vertex_indices_main_ascii (tk, base_op);
#endif

    if (mp_substage == 0) {
        /* determine some of the parameters of how the current opcode is to be written */
        switch (base_op) {
            case OPT_ALL_VFINDICES:
                count = mp_vfindexcount;
                break;
            case OPT_ALL_VEINDICES:
                count = mp_veindexcount;
                break;
            case OPT_ALL_VMINDICES:
                count = mp_vmindexcount;
                break;
            default:
                return tk.Error("internal error in write_vertex_indices_main: unrecognized case");
        }
        mp_optopcode = base_op;
        /* the following makes the (valid) assumption that 
         * [OPT_VERTEX_FINDICES,OPT_VERTEX_EINDICES,OPT_VERTEX_MINDICES] ==
         * [OPT_ALL_VFINDICES,OPT_ALL_VEINDICES,OPT_ALL_VMINDICES] + 1
         * (Odd values indicates an attribute is present on all vertices)
         */
        if (count != mp_pointcount)
            mp_optopcode++;
        if ((status = PutData(tk, mp_optopcode)) != TK_Normal)
            return status;
        if (tk.GetTargetVersion() >= 650)
            mp_bits_per_sample = (unsigned char)(tk.GetNumIndexBits()); 
        else
            mp_bits_per_sample = 32; //this value shouldn't be used, since prior to 650, indices were uncompressed.
        mp_compression_scheme = CS_TRIVIAL;
        mp_substage++;
    }

    if (mp_optopcode == base_op) {
        if ((status = write_vertex_indices_all(tk, mp_optopcode)) != TK_Normal)
            return status;
    }
    else {
        if ((status = write_vertex_indices(tk, mp_optopcode)) != TK_Normal)
            return status;
    }
    return status;
#else
	UNREFERENCED (base_op);
    return tk.Error (stream_readonly);
#endif
}


TK_Status TK_Polyhedron::write_collection(BStreamFileToolkit & tk, bool write_tag, int variant) alter {
#ifndef BSTREAM_READONLY
    TK_Status       status = TK_Normal;
    TK_Terminator   terminate(TKE_Termination);

#ifndef BSTREAM_DISABLE_ASCII
	if( tk.GetAsciiMode() ) 
		return write_collection_ascii(tk, write_tag, variant);
#endif

    while (mp_substage < mp_num_collection_parts) {
        if ((status = mp_collection_parts[mp_substage]->Write(tk)) != TK_Normal)
            return status;
        mp_substage++;
    }
    if (mp_substage == mp_num_collection_parts) {
        if ((status = terminate.Write(tk)) != TK_Normal)
            return status;
        mp_substage++;
    }
    if (write_tag && mp_substage == mp_num_collection_parts + 1) {
        if ((status = Tag (tk, variant)) != TK_Normal)
            return status;
        //note: unlike the standard version of write, write_tag is not followed by
        //a record_instance here because a collection can never be part of an instance.
        mp_substage++;
    }
    mp_substage = 0;
    return status;
#else
	UNREFERENCED (write_tag);
	UNREFERENCED (variant);
    return tk.Error (stream_readonly);
#endif
}



TK_Status TK_Polyhedron::write_vertex_marker_visibilities (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_READONLY
    TK_Status       status = TK_Normal;

					
#ifndef BSTREAM_DISABLE_ASCII
	if( tk.GetAsciiMode() ) 
		return write_vertex_marker_visibilities_ascii (tk);
#endif

    if (mp_vmvisibilitycount == mp_pointcount) {
        switch (mp_substage) {
            //opcode
            case 0: {
                mp_optopcode = OPT_ALL_VMVISIBILITIES;
                if ((status = PutData(tk, mp_optopcode)) != TK_Normal)
                    return status;
                mp_substage++;
            }   nobreak;

            //compression scheme
            case 1: {
                mp_compression_scheme = CS_NONE;
                if ((status = PutData(tk, mp_compression_scheme)) != TK_Normal)
                    return status;
                mp_substage++;
            }   nobreak;

            //all visibilities
            case 2: {
                if ((status = PutData(tk, mp_vmvisibilities, mp_vmvisibilitycount)) != TK_Normal)
                    return status;
                mp_substage = 0;
            } break;

            default:
                return tk.Error("internal error in write_vertex_marker_visibilities (1)");
        }
    }
    else {
        switch (mp_substage) {
            //opcode
            case 0: {
                mp_optopcode = OPT_VERTEX_MARKER_VISIBILITIES;
                if ((status = PutData(tk, mp_optopcode)) != TK_Normal)
                    return status;
                mp_substage++;
            }   nobreak;

            //compression scheme
            case 1: {
                mp_compression_scheme = CS_NONE;
                if ((status = PutData(tk, mp_compression_scheme)) != TK_Normal)
                    return status;
                mp_substage++;
            }   nobreak;

            //count
            case 2: {
                if ((status = PutData(tk, mp_vmvisibilitycount)) != TK_Normal)
                    return status;
                mp_substage++;
                mp_progress = 0;
            }   nobreak;

            //and indices of existing values
            case 3: {
                while (mp_progress < mp_pointcount) {
                    if (mp_exists[mp_progress] & Vertex_Marker_Visibility) {
                        if (mp_pointcount < 256) {
                            unsigned char       byte = (unsigned char)mp_progress;

                            if ((status = PutData(tk, byte)) != TK_Normal)
                                return status;
                        }
                        else if (mp_pointcount < 65536) {
                            unsigned short      word = (unsigned short)mp_progress;

                            if ((status = PutData(tk, word)) != TK_Normal)
                                return status;
                        }
                        else {
                            if ((status = PutData(tk, mp_progress)) != TK_Normal)
                                return status;
                        }
                    }
                    mp_progress++;
                }
                mp_progress = 0;
                mp_substage++;
            } nobreak;

            //individual indices
            case 4: {
                while (mp_progress < mp_pointcount) {
                    if (mp_exists[mp_progress] & Vertex_Marker_Visibility) {
                        if ((status = PutData(tk, mp_vmvisibilities[mp_progress])) != TK_Normal)
                            return status;
                    }
                    mp_progress++;
                }
                mp_progress = 0;
                mp_substage = 0;
            } break;

            default:
                return tk.Error("internal error in write_vertex_marker_visibilities (2)");
        }
    }
    return status;
#else
    return tk.Error (stream_readonly);
#endif
} //end function TK_Polyhedron::write_vertex_marker_visibilities


TK_Status TK_Polyhedron::write_vertex_marker_symbols (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_READONLY
    TK_Status       status = TK_Normal;

						
#ifndef BSTREAM_DISABLE_ASCII
	if( tk.GetAsciiMode() ) 
		return write_vertex_marker_symbols_ascii (tk);
#endif

    if (mp_vmsymbolcount == mp_pointcount && mp_vmsymbolstrings == null) {
        switch (mp_substage) {
            //opcode
            case 0: {
                mp_optopcode = OPT_ALL_VMSYMBOLS;
                if ((status = PutData(tk, mp_optopcode)) != TK_Normal)
                    return status;
                mp_substage++;
            }   nobreak;

            //compression scheme
            case 1: {
                mp_compression_scheme = CS_NONE;
                if ((status = PutData(tk, mp_compression_scheme)) != TK_Normal)
                    return status;
                mp_substage++;
            }   nobreak;

            //all symbols
            case 2: {
                if ((status = PutData(tk, mp_vmsymbols, mp_vmsymbolcount)) != TK_Normal)
                    return status;
                mp_substage = 0;
            } break;

            default:
                return tk.Error("internal error in write_vertex_marker_symbols (1)");
        }
    }
    else {
        switch (mp_substage) {
            //opcode
            case 0: {
                mp_optopcode = OPT_VERTEX_MARKER_SYMBOLS;
                if ((status = PutData(tk, mp_optopcode)) != TK_Normal)
                    return status;
                mp_substage++;
            }   nobreak;

            //compression scheme
            case 1: {
                mp_compression_scheme = CS_NONE;
                if ((status = PutData(tk, mp_compression_scheme)) != TK_Normal)
                    return status;
                mp_substage++;
            }   nobreak;

            //count
            case 2: {
                if ((status = PutData(tk, mp_vmsymbolcount)) != TK_Normal)
                    return status;
                mp_substage++;
                mp_progress = 0;
            }   nobreak;

            //and indices of existing values
            case 3: {
                while (mp_progress < mp_pointcount) {
                    if (mp_exists[mp_progress] & Vertex_Marker_Symbol) {
                        if (mp_pointcount < 256) {
                            unsigned char       byte = (unsigned char)mp_progress;

                            if ((status = PutData(tk, byte)) != TK_Normal)
                                return status;
                        }
                        else if (mp_pointcount < 65536) {
                            unsigned short      word = (unsigned short)mp_progress;

                            if ((status = PutData(tk, word)) != TK_Normal)
                                return status;
                        }
                        else {
                            if ((status = PutData(tk, mp_progress)) != TK_Normal)
                                return status;
                        }
                    }
                    mp_progress++;
                }
                mp_progress = 0;
                mp_subprogress = 0;
                mp_substage++;
            } nobreak;

            //individual indices
            case 4: {
                while (mp_progress < mp_pointcount) {
                    if (mp_exists[mp_progress] & Vertex_Marker_Symbol) {
                        switch (mp_subprogress) {
                            case 0: {
                                if (mp_vmsymbols[mp_progress] != 0x00FF) {
                                    if ((status = PutData(tk, mp_vmsymbols[mp_progress])) != TK_Normal)
                                        return status;
                                    break;
                                }
                                else if (tk.GetTargetVersion() < 1160) {
                                    _W3DTK_REQUIRE_VERSION( 1160 );
                                    if ((status = PutData(tk, (unsigned char)0)) != TK_Normal)
                                        return status;
                                    break;
                                }
                                else {
                                    unsigned char   byte = (unsigned char)0x00FF;

                                    if ((status = PutData(tk, byte)) != TK_Normal)
                                        return status;
                                }
                                mp_subprogress++;
                            }   nobreak;
                            case 1: {
                                unsigned char   byte = (unsigned char)strlen(mp_vmsymbolstrings[mp_progress]);

                                if ((status = PutData(tk, byte)) != TK_Normal)
                                    return status;
                                mp_subprogress++;
                            }   nobreak;
                            case 2: {
                                if ((status = PutData(tk, mp_vmsymbolstrings[mp_progress], (int)strlen(mp_vmsymbolstrings[mp_progress]))) != TK_Normal)
                                    return status;
                                mp_subprogress = 0;
                            }   break;
                        }
                    }
                    mp_progress++;
                }
                mp_progress = 0;
                mp_substage = 0;
            } break;

            default:
                return tk.Error("internal error in write_vertex_marker_symbols (2)");
        }
    }
    return status;
#else
    return tk.Error (stream_readonly);
#endif
} 


TK_Status TK_Polyhedron::write_vertex_marker_sizes (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_READONLY
    TK_Status       status = TK_Normal;

							
#ifndef BSTREAM_DISABLE_ASCII
	if( tk.GetAsciiMode() ) 
		return write_vertex_marker_sizes_ascii (tk);
#endif

    if (mp_vmsizecount == mp_pointcount) {
        switch (mp_substage) {
            //opcode
            case 0: {
                mp_optopcode = OPT_ALL_VMSIZES;
                if ((status = PutData(tk, mp_optopcode)) != TK_Normal)
                    return status;
                mp_substage++;
            }   nobreak;

            //compression scheme
            case 1: {
                mp_compression_scheme = CS_NONE;
                if ((status = PutData(tk, mp_compression_scheme)) != TK_Normal)
                    return status;
                mp_substage++;
            }   nobreak;

            //all sizes
            case 2: {
                if ((status = PutData(tk, mp_vmsizes, mp_vmsizecount)) != TK_Normal)
                    return status;
                mp_substage = 0;
            } break;

            default:
                return tk.Error("internal error in write_vertex_marker_sizes (1)");
        }
    }
    else {
        switch (mp_substage) {
            //opcode
            case 0: {
                mp_optopcode = OPT_VERTEX_MARKER_SIZES;
                if ((status = PutData(tk, mp_optopcode)) != TK_Normal)
                    return status;
                mp_substage++;
            }   nobreak;

            //compression scheme
            case 1: {
                mp_compression_scheme = CS_NONE;
                if ((status = PutData(tk, mp_compression_scheme)) != TK_Normal)
                    return status;
                mp_substage++;
            }   nobreak;

            //count
            case 2: {
                if ((status = PutData(tk, mp_vmsizecount)) != TK_Normal)
                    return status;
                mp_substage++;
                mp_progress = 0;
            }   nobreak;

            //and indices of existing values
            case 3: {
                while (mp_progress < mp_pointcount) {
                    if (mp_exists[mp_progress] & Vertex_Marker_Size) {
                        if (mp_pointcount < 256) {
                            unsigned char       byte = (unsigned char)mp_progress;

                            if ((status = PutData(tk, byte)) != TK_Normal)
                                return status;
                        }
                        else if (mp_pointcount < 65536) {
                            unsigned short      word = (unsigned short)mp_progress;

                            if ((status = PutData(tk, word)) != TK_Normal)
                                return status;
                        }
                        else {
                            if ((status = PutData(tk, mp_progress)) != TK_Normal)
                                return status;
                        }
                    }
                    mp_progress++;
                }
                mp_progress = 0;
                mp_substage++;
            } nobreak;

            //individual indices
            case 4: {
                while (mp_progress < mp_pointcount) {
                    if (mp_exists[mp_progress] & Vertex_Marker_Size) {
                        if ((status = PutData(tk, mp_vmsizes[mp_progress])) != TK_Normal)
                            return status;
                    }
                    mp_progress++;
                }
                mp_progress = 0;
                mp_substage = 0;
            } break;

            default:
                return tk.Error("internal error in write_vertex_marker_sizes (2)");
        }
    }
    return status;
#else
    return tk.Error (stream_readonly);
#endif
} //end function TK_Polyhedron::write_vertex_marker_sizes


TK_Status TK_Polyhedron::write_face_visibilities (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_READONLY
    TK_Status       status = TK_Normal;
		
#ifndef BSTREAM_DISABLE_ASCII
	if( tk.GetAsciiMode() ) 
		return write_face_visibilities_ascii (tk);
#endif

    if (mp_fvisibilitycount == mp_facecount) {
        switch (mp_substage) {
            //opcode
            case 0: {
                mp_optopcode = OPT_ALL_FACE_VISIBILITIES;
                if ((status = PutData(tk, mp_optopcode)) != TK_Normal)
                    return status;
                mp_substage++;
            }   nobreak;

            //compression scheme
            case 1: {
                mp_compression_scheme = CS_NONE;
                if ((status = PutData(tk, mp_compression_scheme)) != TK_Normal)
                    return status;
                mp_substage++;
            }   nobreak;

            //all visibilities
            case 2: {
                if ((status = PutData(tk, mp_fvisibilities, mp_fvisibilitycount)) != TK_Normal)
                    return status;
                mp_substage = 0;
            } break;

            default:
                return tk.Error("internal error in write_face_visibilities (1)");
        }
    }
    else {
        switch (mp_substage) {
            //opcode
            case 0: {
                mp_optopcode = OPT_FACE_VISIBILITIES;
                if ((status = PutData(tk, mp_optopcode)) != TK_Normal)
                    return status;
                mp_substage++;
            }   nobreak;

            //compression scheme
            case 1: {
                mp_compression_scheme = CS_NONE;
                if ((status = PutData(tk, mp_compression_scheme)) != TK_Normal)
                    return status;
                mp_substage++;
            }   nobreak;

            //count
            case 2: {
                if ((status = PutData(tk, mp_fvisibilitycount)) != TK_Normal)
                    return status;
                mp_substage++;
                mp_progress = 0;
            }   nobreak;

            //and indices of existing values
            case 3: {
                while (mp_progress < mp_facecount) {
                    if (mp_face_exists[mp_progress] & Face_Visibility) {
                        if (mp_facecount < 256) {
                            unsigned char       byte = (unsigned char)mp_progress;

                            if ((status = PutData(tk, byte)) != TK_Normal)
                                return status;
                        }
                        else if (mp_facecount < 65536) {
                            unsigned short      word = (unsigned short)mp_progress;

                            if ((status = PutData(tk, word)) != TK_Normal)
                                return status;
                        }
                        else {
                            if ((status = PutData(tk, mp_progress)) != TK_Normal)
                                return status;
                        }
                    }
                    mp_progress++;
                }
                mp_progress = 0;
                mp_substage++;
            } nobreak;

            //individual indices
            case 4: {
                while (mp_progress < mp_facecount) {
                    if (mp_face_exists[mp_progress] & Face_Visibility) {
                        if ((status = PutData(tk, mp_fvisibilities[mp_progress])) != TK_Normal)
                            return status;
                    }
                    mp_progress++;
                }
                mp_progress = 0;
                mp_substage = 0;
            } break;

            default:
                return tk.Error("internal error in write_face_visibilities (2)");
        }
    }
    return status;
#else
    return tk.Error (stream_readonly);
#endif
} //end function TK_Polyhedron::write_face_visibilities


TK_Status TK_Polyhedron::write_face_patterns (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_READONLY
    TK_Status       status = TK_Normal;

#ifndef BSTREAM_DISABLE_ASCII
	if( tk.GetAsciiMode() ) 
		return write_face_patterns_ascii (tk);
#endif

    if (mp_fpatterncount == mp_facecount) {
        switch (mp_substage) {
            //opcode
            case 0: {
                mp_optopcode = OPT_ALL_FACE_PATTERNS;
                if ((status = PutData(tk, mp_optopcode)) != TK_Normal)
                    return status;
                mp_substage++;
            }   nobreak;

            //compression scheme
            case 1: {
                mp_compression_scheme = CS_NONE;
                if ((status = PutData(tk, mp_compression_scheme)) != TK_Normal)
                    return status;
                mp_substage++;
            }   nobreak;

            //all patterns
            case 2: {
                if ((status = PutData(tk, mp_fpatterns, mp_fpatterncount)) != TK_Normal)
                    return status;
                mp_substage = 0;
            } break;

            default:
                return tk.Error("internal error in write_face_patterns (1)");
        }
    }
    else {
        switch (mp_substage) {
            //opcode
            case 0: {
                mp_optopcode = OPT_FACE_PATTERNS;
                if ((status = PutData(tk, mp_optopcode)) != TK_Normal)
                    return status;
                mp_substage++;
            }   nobreak;

            //compression scheme
            case 1: {
                mp_compression_scheme = CS_NONE;
                if ((status = PutData(tk, mp_compression_scheme)) != TK_Normal)
                    return status;
                mp_substage++;
            }   nobreak;

            //count
            case 2: {
                if ((status = PutData(tk, mp_fpatterncount)) != TK_Normal)
                    return status;
                mp_substage++;
                mp_progress = 0;
            }   nobreak;

            //and indices of existing values
            case 3: {
                while (mp_progress < mp_facecount) {
                    if (mp_face_exists[mp_progress] & Face_Pattern) {
                        if (mp_facecount < 256) {
                            unsigned char       byte = (unsigned char)mp_progress;

                            if ((status = PutData(tk, byte)) != TK_Normal)
                                return status;
                        }
                        else if (mp_facecount < 65536) {
                            unsigned short      word = (unsigned short)mp_progress;

                            if ((status = PutData(tk, word)) != TK_Normal)
                                return status;
                        }
                        else {
                            if ((status = PutData(tk, mp_progress)) != TK_Normal)
                                return status;
                        }
                    }
                    mp_progress++;
                }
                mp_progress = 0;
                mp_substage++;
            } nobreak;

            //individual indices
            case 4: {
                while (mp_progress < mp_facecount) {
                    if (mp_face_exists[mp_progress] & Face_Pattern) {
                        if ((status = PutData(tk, mp_fpatterns[mp_progress])) != TK_Normal)
                            return status;
                    }
                    mp_progress++;
                }
                mp_progress = 0;
                mp_substage = 0;
            } break;

            default:
                return tk.Error("internal error in write_face_patterns (2)");
        }
    }
    return status;
#else
    return tk.Error (stream_readonly);
#endif
} //end function TK_Polyhedron::write_face_patterns


TK_Status TK_Polyhedron::write_face_normals (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_READONLY
    TK_Status       status = TK_Normal;

	
#ifndef BSTREAM_DISABLE_ASCII
	if( tk.GetAsciiMode() ) 
		return write_face_normals_ascii (tk);
#endif

    if (mp_fnormalcount == mp_facecount) {
        switch (mp_substage) {
            //opcode
            case 0: {
                mp_optopcode = OPT_ALL_FACE_NORMALS_POLAR;
                if ((status = PutData(tk, mp_optopcode)) != TK_Normal)
                    return status;
                mp_substage++;
            }   nobreak;

            //compression scheme
            case 1: {
                mp_compression_scheme = CS_NONE;
                if ((status = PutData(tk, mp_compression_scheme)) != TK_Normal)
                    return status;
                normals_cartesian_to_polar(null, Face_Normal, mp_facecount, mp_fnormals, mp_fnormals);
                mp_substage++;
            }   nobreak;

            //all normals
            case 2: {
                if ((status = PutData(tk, mp_fnormals, 2*mp_fnormalcount)) != TK_Normal)
                    return status;
                mp_substage = 0;
            } break;

            default:
                return tk.Error("internal error in write_face_normals (1)");
        }
    }
    else {
        switch (mp_substage) {
            //opcode
            case 0: {
                mp_optopcode = OPT_FACE_NORMALS_POLAR;
                if ((status = PutData(tk, mp_optopcode)) != TK_Normal)
                    return status;
                mp_substage++;
            }   nobreak;

            //compression scheme
            case 1: {
                mp_compression_scheme = CS_NONE;
                if ((status = PutData(tk, mp_compression_scheme)) != TK_Normal)
                    return status;
                mp_substage++;
            }   nobreak;

            //count
            case 2: {
                if ((status = PutData(tk, mp_fnormalcount)) != TK_Normal)
                    return status;
                mp_substage++;
                mp_progress = 0;
            }   nobreak;

            //and indices of existing values
            case 3: {
                while (mp_progress < mp_facecount) {
                    if (mp_face_exists[mp_progress] & Face_Normal) {
                        if (mp_facecount < 256) {
                            unsigned char       byte = (unsigned char)mp_progress;

                            if ((status = PutData(tk, byte)) != TK_Normal)
                                return status;
                        }
                        else if (mp_facecount < 65536) {
                            unsigned short      word = (unsigned short)mp_progress;

                            if ((status = PutData(tk, word)) != TK_Normal)
                                return status;
                        }
                        else {
                            if ((status = PutData(tk, mp_progress)) != TK_Normal)
                                return status;
                        }
                    }
                    mp_progress++;
                }
                normals_cartesian_to_polar(mp_face_exists, Face_Normal, mp_facecount, mp_fnormals, mp_fnormals);
                mp_progress = 0;
                mp_substage++;
            } nobreak;

            //individual indices
            case 4: {
                while (mp_progress < mp_facecount) {
                    if (mp_face_exists[mp_progress] & Face_Normal) {
                        if ((status = PutData(tk, &mp_fnormals[2*mp_progress], 2)) != TK_Normal)
                            return status;
                    }
                    mp_progress++;
                }
                mp_progress = 0;
                mp_substage = 0;
            } break;

            default:
                return tk.Error("internal error in write_face_normals (2)");
        }
    }
    return status;
#else
    return tk.Error (stream_readonly);
#endif
} //end function TK_Polyhedron::write_face_normals


TK_Status TK_Polyhedron::write_edge_visibilities (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_READONLY
    TK_Status       status = TK_Normal;

#ifndef BSTREAM_DISABLE_ASCII
	if( tk.GetAsciiMode() ) 
		return write_edge_visibilities_ascii (tk);
#endif

    if (mp_evisibilitycount == mp_edgecount) {
        switch (mp_substage) {
            //opcode
            case 0: {
                mp_optopcode = OPT_ALL_EDGE_VISIBILITIES;
                if ((status = PutData(tk, mp_optopcode)) != TK_Normal)
                    return status;
                mp_substage++;
            }   nobreak;

            //compression scheme
            case 1: {
                mp_compression_scheme = CS_NONE;
                if ((status = PutData(tk, mp_compression_scheme)) != TK_Normal)
                    return status;
                mp_substage++;
            }   nobreak;

            //all visibilities
            case 2: {
                if ((status = PutData(tk, mp_evisibilities, mp_evisibilitycount)) != TK_Normal)
                    return status;
                mp_substage = 0;
            } break;

            default:
                return tk.Error("internal error in write_edge_visibilities (1)");
        }
    }
    else {
        switch (mp_substage) {
            //opcode
            case 0: {
                mp_optopcode = OPT_EDGE_VISIBILITIES;
                if ((status = PutData(tk, mp_optopcode)) != TK_Normal)
                    return status;
                mp_substage++;
            }   nobreak;

            //compression scheme
            case 1: {
                mp_compression_scheme = CS_NONE;
                if ((status = PutData(tk, mp_compression_scheme)) != TK_Normal)
                    return status;
                mp_substage++;
            }   nobreak;

            //count
            case 2: {
                if ((status = PutData(tk, mp_evisibilitycount)) != TK_Normal)
                    return status;
                mp_substage++;
                mp_progress = 0;
            }   nobreak;

            //and indices of existing values
            case 3: {
                while (mp_progress < mp_edgecount) {
                    if (mp_edge_exists[mp_progress] & Edge_Visibility) {
                        if (mp_edgecount < 256) {
                            unsigned char       byte = (unsigned char)mp_progress;

                            if ((status = PutData(tk, byte)) != TK_Normal)
                                return status;
                        }
                        else if (mp_edgecount < 65536) {
                            unsigned short      word = (unsigned short)mp_progress;

                            if ((status = PutData(tk, word)) != TK_Normal)
                                return status;
                        }
                        else {
                            if ((status = PutData(tk, mp_progress)) != TK_Normal)
                                return status;
                        }
                    }
                    mp_progress++;
                }
                mp_progress = 0;
                mp_substage++;
            } nobreak;

            //individual indices
            case 4: {
                while (mp_progress < mp_edgecount) {
                    if (mp_edge_exists[mp_progress] & Edge_Visibility) {
                        if ((status = PutData(tk, mp_evisibilities[mp_progress])) != TK_Normal)
                            return status;
                    }
                    mp_progress++;
                }
                mp_progress = 0;
                mp_substage = 0;
            } break;

            default:
                return tk.Error("internal error in write_edge_visibilities (2)");
        }
    }
    return status;
#else
    return tk.Error (stream_readonly);
#endif
} //end function TK_Polyhedron::write_edge_visibilities


TK_Status TK_Polyhedron::write_edge_patterns (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_READONLY
    TK_Status       status = TK_Normal;

	
#ifndef BSTREAM_DISABLE_ASCII
	if( tk.GetAsciiMode() ) 
		return write_edge_patterns_ascii (tk);
#endif

    if (mp_epatterncount == mp_edgecount && mp_epatternstrings == null) {
        switch (mp_substage) {
            //opcode
            case 0: {
                mp_optopcode = OPT_ALL_EDGE_PATTERNS;
                if ((status = PutData(tk, mp_optopcode)) != TK_Normal)
                    return status;
                mp_substage++;
            }   nobreak;

            //compression scheme
            case 1: {
                mp_compression_scheme = CS_NONE;
                if ((status = PutData(tk, mp_compression_scheme)) != TK_Normal)
                    return status;
                mp_substage++;
            }   nobreak;

            //all patterns
            case 2: {
                if ((status = PutData(tk, mp_epatterns, mp_epatterncount)) != TK_Normal)
                    return status;
                mp_substage = 0;
            } break;

            default:
                return tk.Error("internal error in write_edge_patterns (1)");
        }
    }
    else {
        switch (mp_substage) {
            //opcode
            case 0: {
                mp_optopcode = OPT_EDGE_PATTERNS;
                if ((status = PutData(tk, mp_optopcode)) != TK_Normal)
                    return status;
                mp_substage++;
            }   nobreak;

            //compression scheme
            case 1: {
                mp_compression_scheme = CS_NONE;
                if ((status = PutData(tk, mp_compression_scheme)) != TK_Normal)
                    return status;
                mp_substage++;
            }   nobreak;

            //count
            case 2: {
                if ((status = PutData(tk, mp_epatterncount)) != TK_Normal)
                    return status;
                mp_substage++;
                mp_progress = 0;
            }   nobreak;

            //and indices of existing values
            case 3: {
                while (mp_progress < mp_edgecount) {
                    if (mp_edge_exists[mp_progress] & Edge_Pattern) {
                        if (mp_edgecount < 256) {
                            unsigned char       byte = (unsigned char)mp_progress;

                            if ((status = PutData(tk, byte)) != TK_Normal)
                                return status;
                        }
                        else if (mp_edgecount < 65536) {
                            unsigned short      word = (unsigned short)mp_progress;

                            if ((status = PutData(tk, word)) != TK_Normal)
                                return status;
                        }
                        else {
                            if ((status = PutData(tk, mp_progress)) != TK_Normal)
                                return status;
                        }
                    }
                    mp_progress++;
                }
                mp_progress = 0;
                mp_substage++;
            } nobreak;

            //individual indices
            case 4: {
                while (mp_progress < mp_edgecount) {
                    if (mp_edge_exists[mp_progress] & Edge_Pattern) {
                        switch (mp_subprogress) {
                            case 0: {
                                if (mp_epatterns[mp_progress] != 0x00FF) {
                                    if ((status = PutData(tk, mp_epatterns[mp_progress])) != TK_Normal)
                                        return status;
                                    break;
                                }
                                else if (tk.GetTargetVersion() < 1160) {
                                    _W3DTK_REQUIRE_VERSION( 1160 );
                                    if ((status = PutData(tk, (unsigned char)0)) != TK_Normal)
                                        return status;
                                    break;
                                }
                                else {
                                    unsigned char   byte = (unsigned char)0x00FF;

                                    if ((status = PutData(tk, byte)) != TK_Normal)
                                        return status;
                                }
                                mp_subprogress++;
                            }   nobreak;
                            case 1: {
                                unsigned char   byte = (unsigned char)strlen(mp_epatternstrings[mp_progress]);

                                if ((status = PutData(tk, byte)) != TK_Normal)
                                    return status;
                                mp_subprogress++;
                            }   nobreak;
                            case 2: {
                                if ((status = PutData(tk, mp_epatternstrings[mp_progress], (int)strlen(mp_epatternstrings[mp_progress]))) != TK_Normal)
                                    return status;
                                mp_subprogress = 0;
                            }   break;
                        }
                    }
                    mp_progress++;
                }
                mp_progress = 0;
                mp_substage = 0;
            } break;

            default:
                return tk.Error("internal error in write_edge_patterns (2)");
        }
    }
    return status;
#else
    return tk.Error (stream_readonly);
#endif
} //end function TK_Polyhedron::write_edge_patterns


TK_Status TK_Polyhedron::write_edge_colors (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_READONLY
    TK_Status       status = TK_Normal;
	
#ifndef BSTREAM_DISABLE_ASCII
	if( tk.GetAsciiMode() ) 
		return write_edge_colors_ascii (tk);
#endif

    if (mp_ecolorcount == mp_edgecount) {
        switch (mp_substage) {
            //opcode
            case 0: {
                mp_optopcode = OPT_ALL_EDGE_COLORS;
                if ((status = PutData(tk, mp_optopcode)) != TK_Normal)
                    return status;
                mp_substage++;
            }   nobreak;

            //compression scheme
            case 1: {
                mp_compression_scheme = CS_NONE;
                if ((status = PutData(tk, mp_compression_scheme)) != TK_Normal)
                    return status;
                mp_substage++;
            }   nobreak;

            //all colors
            case 2: {
                if ((status = PutData(tk, mp_ecolors, 3 * mp_ecolorcount)) != TK_Normal)
                    return status;
                mp_substage = 0;
            } break;

            default:
                return tk.Error("internal error in write_edge_colors (1)");
        }
    }
    else {
        switch (mp_substage) {
            //opcode
            case 0: {
                mp_optopcode = OPT_EDGE_COLORS;
                if ((status = PutData(tk, mp_optopcode)) != TK_Normal)
                    return status;
                mp_substage++;
            }   nobreak;

            //compression scheme
            case 1: {
                mp_compression_scheme = CS_NONE;
                if ((status = PutData(tk, mp_compression_scheme)) != TK_Normal)
                    return status;
                mp_substage++;
            }   nobreak;

            //count
            case 2: {
                if ((status = PutData(tk, mp_ecolorcount)) != TK_Normal)
                    return status;
                mp_substage++;
                mp_progress = 0;
            }   nobreak;

            //and indices of existing values
            case 3: {
                while (mp_progress < mp_edgecount) {
                    if (mp_edge_exists[mp_progress] & Edge_Color) {
                        if (mp_edgecount < 256) {
                            unsigned char       byte = (unsigned char)mp_progress;

                            if ((status = PutData(tk, byte)) != TK_Normal)
                                return status;
                        }
                        else if (mp_edgecount < 65536) {
                            unsigned short      word = (unsigned short)mp_progress;

                            if ((status = PutData(tk, word)) != TK_Normal)
                                return status;
                        }
                        else {
                            if ((status = PutData(tk, mp_progress)) != TK_Normal)
                                return status;
                        }
                    }
                    mp_progress++;
                }
                mp_progress = 0;
                mp_substage++;
            } nobreak;

            //individual indices
            case 4: {
                while (mp_progress < mp_edgecount) {
                    if (mp_edge_exists[mp_progress] & Edge_Color) {
                        if ((status = PutData(tk, &mp_ecolors[3*mp_progress], 3)) != TK_Normal)
                            return status;
                    }
                    mp_progress++;
                }
                mp_progress = 0;
                mp_substage = 0;
            } break;

            default:
                return tk.Error("internal error in write_edge_colors (2)");
        }
    }
    return status;
#else
    return tk.Error (stream_readonly);
#endif
} //end function TK_Polyhedron::write_edge_colors


TK_Status TK_Polyhedron::write_edge_indices (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_READONLY
    TK_Status       status = TK_Normal;
		
#ifndef BSTREAM_DISABLE_ASCII
	if( tk.GetAsciiMode() ) 
		return write_edge_indices_ascii (tk);
#endif

    if (mp_eindexcount == mp_edgecount) {
        switch (mp_substage) {
            //opcode
            case 0: {
                mp_optopcode = OPT_ALL_EDGE_INDICES;
                if ((status = PutData(tk, mp_optopcode)) != TK_Normal)
                    return status;
                mp_substage++;
            }   nobreak;

            //compression scheme
            case 1: {
                mp_compression_scheme = CS_NONE;
                if ((status = PutData(tk, mp_compression_scheme)) != TK_Normal)
                    return status;
                mp_substage++;
            }   nobreak;

            //all indices
            case 2: {
                if ((status = PutData(tk, mp_eindices, mp_eindexcount)) != TK_Normal)
                    return status;
                mp_substage = 0;
            } break;

            default:
                return tk.Error("internal error in write_edge_indices (1)");
        }
    }
    else {
        switch (mp_substage) {
            //opcode
            case 0: {
                mp_optopcode = OPT_EDGE_INDICES;
                if ((status = PutData(tk, mp_optopcode)) != TK_Normal)
                    return status;
                mp_substage++;
            }   nobreak;

            //compression scheme
            case 1: {
                mp_compression_scheme = CS_NONE;
                if ((status = PutData(tk, mp_compression_scheme)) != TK_Normal)
                    return status;
                mp_substage++;
            }   nobreak;

            //count
            case 2: {
                if ((status = PutData(tk, mp_eindexcount)) != TK_Normal)
                    return status;
                mp_substage++;
                mp_progress = 0;
            }   nobreak;

            //and indices of existing values
            case 3: {
                while (mp_progress < mp_edgecount) {
                    if (mp_edge_exists[mp_progress] & Edge_Index) {
                        if (mp_edgecount < 256) {
                            unsigned char       byte = (unsigned char)mp_progress;

                            if ((status = PutData(tk, byte)) != TK_Normal)
                                return status;
                        }
                        else if (mp_edgecount < 65536) {
                            unsigned short      word = (unsigned short)mp_progress;

                            if ((status = PutData(tk, word)) != TK_Normal)
                                return status;
                        }
                        else {
                            if ((status = PutData(tk, mp_progress)) != TK_Normal)
                                return status;
                        }
                    }
                    mp_progress++;
                }
                mp_progress = 0;
                mp_substage++;
            } nobreak;

            //individual indices
            case 4: {
                while (mp_progress < mp_edgecount) {
                    if (mp_edge_exists[mp_progress] & Edge_Index) {
                        if ((status = PutData(tk, mp_eindices[mp_progress])) != TK_Normal)
                            return status;
                    }
                    mp_progress++;
                }
                mp_progress = 0;
                mp_substage = 0;
            } break;

            default:
                return tk.Error("internal error in write_edge_indices (2)");
        }
    }
    return status;
#else
    return tk.Error (stream_readonly);
#endif
} //end function TK_Polyhedron::write_edge_indices


TK_Status TK_Polyhedron::write_edge_weights (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_READONLY
    TK_Status       status = TK_Normal;
			
#ifndef BSTREAM_DISABLE_ASCII
	if( tk.GetAsciiMode() ) 
		return write_edge_weights_ascii (tk);
#endif

    if (mp_eweightcount == mp_edgecount) {
        switch (mp_substage) {
            //opcode
            case 0: {
                mp_optopcode = OPT_ALL_EDGE_WEIGHTS;
                if ((status = PutData(tk, mp_optopcode)) != TK_Normal)
                    return status;
                mp_substage++;
            }   nobreak;

            //compression scheme
            case 1: {
                mp_compression_scheme = CS_NONE;
                if ((status = PutData(tk, mp_compression_scheme)) != TK_Normal)
                    return status;
                mp_substage++;
            }   nobreak;

            //all weights
            case 2: {
                if ((status = PutData(tk, mp_eweights, mp_eweightcount)) != TK_Normal)
                    return status;
                mp_substage = 0;
            } break;

            default:
                return tk.Error("internal error in write_edge_weights (1)");
        }
    }
    else {
        switch (mp_substage) {
            //opcode
            case 0: {
                mp_optopcode = OPT_EDGE_WEIGHTS;
                if ((status = PutData(tk, mp_optopcode)) != TK_Normal)
                    return status;
                mp_substage++;
            }   nobreak;

            //compression scheme
            case 1: {
                mp_compression_scheme = CS_NONE;
                if ((status = PutData(tk, mp_compression_scheme)) != TK_Normal)
                    return status;
                mp_substage++;
            }   nobreak;

            //count
            case 2: {
                if ((status = PutData(tk, mp_eweightcount)) != TK_Normal)
                    return status;
                mp_substage++;
                mp_progress = 0;
            }   nobreak;

            //and indices of existing values
            case 3: {
                while (mp_progress < mp_edgecount) {
                    if (mp_edge_exists[mp_progress] & Edge_Weight) {
                        if (mp_edgecount < 256) {
                            unsigned char       byte = (unsigned char)mp_progress;

                            if ((status = PutData(tk, byte)) != TK_Normal)
                                return status;
                        }
                        else if (mp_edgecount < 65536) {
                            unsigned short      word = (unsigned short)mp_progress;

                            if ((status = PutData(tk, word)) != TK_Normal)
                                return status;
                        }
                        else {
                            if ((status = PutData(tk, mp_progress)) != TK_Normal)
                                return status;
                        }
                    }
                    mp_progress++;
                }
                mp_progress = 0;
                mp_substage++;
            } nobreak;

            //individual indices
            case 4: {
                while (mp_progress < mp_edgecount) {
                    if (mp_edge_exists[mp_progress] & Edge_Weight) {
                        if ((status = PutData(tk, mp_eweights[mp_progress])) != TK_Normal)
                            return status;
                    }
                    mp_progress++;
                }
                mp_progress = 0;
                mp_substage = 0;
            } break;

            default:
                return tk.Error("internal error in write_edge_weights (2)");
        }
    }
    return status;
#else
    return tk.Error (stream_readonly);
#endif
} //end function TK_Polyhedron::write_edge_weights


TK_Status TK_Polyhedron::write_edge_normals (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_READONLY
    TK_Status       status = TK_Normal;

#ifndef BSTREAM_DISABLE_ASCII
	if( tk.GetAsciiMode() ) 
		return write_edge_normals_ascii (tk);
#endif

    if (mp_enormalcount == mp_edgecount) {
        switch (mp_substage) {
            //opcode
            case 0: {
                mp_optopcode = OPT_ALL_EDGE_NORMALS_POLAR;
                if ((status = PutData(tk, mp_optopcode)) != TK_Normal)
                    return status;
                mp_substage++;
            }   nobreak;

            //compression scheme
            case 1: {
                mp_compression_scheme = CS_NONE;
                if ((status = PutData(tk, mp_compression_scheme)) != TK_Normal)
                    return status;
                normals_cartesian_to_polar(null, Edge_Normal, mp_edgecount, mp_enormals, mp_enormals);
                mp_substage++;
            }   nobreak;

            //all normals
            case 2: {
                if ((status = PutData(tk, mp_enormals, mp_enormalcount*2)) != TK_Normal)
                    return status;
                mp_substage = 0;
            } break;

            default:
                return tk.Error("internal error in write_edge_normals (1)");
        }
    }
    else {
        switch (mp_substage) {
            //opcode
            case 0: {
                mp_optopcode = OPT_EDGE_NORMALS_POLAR;
                if ((status = PutData(tk, mp_optopcode)) != TK_Normal)
                    return status;
                mp_substage++;
            }   nobreak;

            //compression scheme
            case 1: {
                mp_compression_scheme = CS_NONE;
                if ((status = PutData(tk, mp_compression_scheme)) != TK_Normal)
                    return status;
                mp_substage++;
            }   nobreak;

            //count
            case 2: {
                if ((status = PutData(tk, mp_enormalcount)) != TK_Normal)
                    return status;
                mp_substage++;
                mp_progress = 0;
            }   nobreak;

            //and indices of existing values
            case 3: {
                while (mp_progress < mp_edgecount) {
                    if (mp_edge_exists[mp_progress] & Edge_Normal) {
                        if (mp_edgecount < 256) {
                            unsigned char       byte = (unsigned char)mp_progress;

                            if ((status = PutData(tk, byte)) != TK_Normal)
                                return status;
                        }
                        else if (mp_edgecount < 65536) {
                            unsigned short      word = (unsigned short)mp_progress;

                            if ((status = PutData(tk, word)) != TK_Normal)
                                return status;
                        }
                        else {
                            if ((status = PutData(tk, mp_progress)) != TK_Normal)
                                return status;
                        }
                    }
                    mp_progress++;
                }
                mp_progress = 0;
                normals_cartesian_to_polar(mp_edge_exists, Edge_Normal, mp_edgecount, mp_enormals, mp_enormals);
                mp_substage++;
            } nobreak;

            //individual indices
            case 4: {
                while (mp_progress < mp_edgecount) {
                    if (mp_edge_exists[mp_progress] & Edge_Normal) {
                        if ((status = PutData(tk, &mp_enormals[2*mp_progress], 2)) != TK_Normal)
                            return status;
                    }
                    mp_progress++;
                }
                mp_progress = 0;
                mp_substage = 0;
            } break;

            default:
                return tk.Error("internal error in write_edge_normals (2)");
        }
    }
    return status;
#else
    return tk.Error (stream_readonly);
#endif
} //end function TK_Polyhedron::write_edge_normals


TK_Status TK_Polyhedron::Write (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_READONLY
    TK_Status       status = TK_Normal;

#ifndef BSTREAM_DISABLE_ASCII
	if( tk.GetAsciiMode() ) 
		return WriteAscii (tk);
#endif

    switch (mp_stage) {
        case 0: {
            if (mp_normalcount > 0) {
                if ((status = write_vertex_normals_main(tk)) != TK_Normal)
                    return status;
            }
            mp_stage++;
        }   nobreak;

        case 1: {
            if (mp_paramcount > 0) {
                if ((status = write_vertex_parameters_main(tk)) != TK_Normal)
                    return status;
            }
            mp_stage++;
        }   nobreak;

        case 2: {
            if (mp_vfcolorcount > 0) {
                if ((status = write_vertex_colors_main(tk, OPT_ALL_VFCOLORS)) != TK_Normal)
                    return status;
            }
            mp_stage++;
        }   nobreak;

        case 3: {
            if (mp_vecolorcount > 0) {
                if ((status = write_vertex_colors_main(tk, OPT_ALL_VECOLORS)) != TK_Normal)
                    return status;
            }
            mp_stage++;
        }   nobreak;

        case 4: {
            if (mp_vmcolorcount > 0) {
                if ((status = write_vertex_colors_main(tk, OPT_ALL_VMCOLORS)) != TK_Normal)
                    return status;
            }
            mp_stage++;
        }   nobreak;

        case 5: {
            if (mp_vfindexcount > 0) {
                if ((status = write_vertex_indices_main(tk, OPT_ALL_VFINDICES)) != TK_Normal)
                    return status;
            }
            mp_stage++;
        }   nobreak;

        case 6: {
            if (mp_veindexcount > 0) {
                if ((status = write_vertex_indices_main(tk, OPT_ALL_VEINDICES)) != TK_Normal)
                    return status;
            }
            mp_stage++;
        }   nobreak;

        case 7: {
            if (mp_vmindexcount > 0) {
                if ((status = write_vertex_indices_main(tk, OPT_ALL_VMINDICES)) != TK_Normal)
                    return status;
            }
            mp_stage++;
        }   nobreak;

        case 8: {
            if (mp_fcolorcount > 0) {
                if ((status = write_face_colors_main(tk)) != TK_Normal)
                    return status;
            }
            mp_stage++;
        }   nobreak;

        case 9: {
            if (mp_findexcount > 0) {
                if ((status = write_face_indices_main(tk)) != TK_Normal)
                    return status;
            }
            mp_stage++;
        }   nobreak;

        case 10: {
            if (mp_fregions != null) {
                if ((status = write_face_regions(tk)) != TK_Normal)
                    return status;
            }
            mp_stage++;
        }   nobreak;

        case 11: {
            if (tk.GetTargetVersion() >= 1105 &&
                mp_vmvisibilities != null) {
                if ((status = write_vertex_marker_visibilities(tk)) != TK_Normal)
                    return status;
                _W3DTK_REQUIRE_VERSION( 1105 );
            }
            mp_stage++;
        }   nobreak;

        case 12: {
            if (tk.GetTargetVersion() >= 1105 &&
                mp_vmsymbols != null) {
                if ((status = write_vertex_marker_symbols(tk)) != TK_Normal)
                    return status;
                _W3DTK_REQUIRE_VERSION( 1105 );
            }
            mp_stage++;
        }   nobreak;

        case 13: {
            if (tk.GetTargetVersion() >= 1105 &&
                mp_vmsizes != null) {
                if ((status = write_vertex_marker_sizes(tk)) != TK_Normal)
                    return status;
                _W3DTK_REQUIRE_VERSION( 1105 );
            }
            mp_stage++;
        }   nobreak;

        case 14: {
            if (tk.GetTargetVersion() >= 1105 &&
                mp_fvisibilities != null) {
                if ((status = write_face_visibilities(tk)) != TK_Normal)
                    return status;
                _W3DTK_REQUIRE_VERSION( 1105 );
            }
            mp_stage++;
        }   nobreak;

        case 15: {
            if (tk.GetTargetVersion() >= 1105 &&
                mp_fpatterns != null) {
                if ((status = write_face_patterns(tk)) != TK_Normal)
                    return status;
                _W3DTK_REQUIRE_VERSION( 1105 );
            }
            mp_stage++;
        }   nobreak;

        case 16: {
            if (tk.GetTargetVersion() >= 1105 &&
                mp_fnormals != null) {
                if ((status = write_face_normals(tk)) != TK_Normal)
                    return status;
                _W3DTK_REQUIRE_VERSION( 1105 );
            }
            mp_stage++;
        }   nobreak;

        case 17: {
            if (tk.GetTargetVersion() >= 1105 &&
                mp_evisibilities != null) {
                if ((status = write_edge_visibilities(tk)) != TK_Normal)
                    return status;
                _W3DTK_REQUIRE_VERSION( 1105 );
            }
            mp_stage++;
        }   nobreak;

        case 18: {
            if (tk.GetTargetVersion() >= 1105 &&
                mp_epatterns != null) {
                if ((status = write_edge_patterns(tk)) != TK_Normal)
                    return status;
                _W3DTK_REQUIRE_VERSION( 1105 );
            }
            mp_stage++;
        }   nobreak;

        case 19: {
            if (tk.GetTargetVersion() >= 1105 &&
                mp_ecolors != null) {
                if ((status = write_edge_colors(tk)) != TK_Normal)
                    return status;
                _W3DTK_REQUIRE_VERSION( 1105 );
            }
            mp_stage++;
        }   nobreak;

        case 20: {
            if (tk.GetTargetVersion() >= 1105 &&
                mp_eindices != null) {
                if ((status = write_edge_indices(tk)) != TK_Normal)
                    return status;
                _W3DTK_REQUIRE_VERSION( 1105 );
            }
            mp_stage++;
        }   nobreak;

        case 21: {
            if (tk.GetTargetVersion() >= 1105 &&
                mp_eweights != null) {
                if ((status = write_edge_weights(tk)) != TK_Normal)
                    return status;
                _W3DTK_REQUIRE_VERSION( 1105 );
            }
            mp_stage++;
        }   nobreak;

        case 22: {
            if (tk.GetTargetVersion() >= 1105 &&
                mp_enormals != null) {
                if ((status = write_edge_normals(tk)) != TK_Normal)
                    return status;
                _W3DTK_REQUIRE_VERSION( 1105 );
            }
            mp_stage++;
        }   nobreak;

        case 23: {
            if (mp_num_collection_parts != 0 && tk.GetTargetVersion() >= 705) {
                char        op = OPT_ENTITY_ATTRIBUTES;

                if ((status = PutData(tk, op)) != TK_Normal)
                    return status;
            }
            mp_stage++;
        }   nobreak;


        case 24: {
            if (mp_num_collection_parts != 0 && mp_num_collection_parts != 0) {
                if ((status = write_collection(tk)) != TK_Normal)
                    return status;
            }
            mp_stage++;
        }   nobreak;

        case 25: {
            mp_optopcode = OPT_TERMINATE;
            if ((status = PutData(tk, mp_optopcode)) != TK_Normal)
                return status;
            mp_stage = 0;
        } break;

        default:
            return tk.Error();
    }
    return status;
#else
    return tk.Error (stream_readonly);
#endif
} // end function TK_Polyhedron::Write





//protected virtual
TK_Status TK_Polyhedron::read_trivial_points (BStreamFileToolkit & tk) alter {
    TK_Status       status;

#ifndef BSTREAM_DISABLE_ASCII
	if( tk.GetAsciiMode() ) 
		return read_trivial_points_ascii (tk);
#endif
    switch (mp_substage) {
        //unused
        case 0: {
            mp_substage++;
        }   nobreak;

        //number of points
        case 1: {
            if ((status = GetData(tk, mp_pointcount)) != TK_Normal)
                return status;
            if (mp_pointcount == 0)
                return status;
             mp_substage++;
        } nobreak;

        // get bounding
        case 2: {
            if (mp_subop2 & TKSH2_GLOBAL_QUANTIZATION) {
                float const *global_bbox = tk.GetWorldBounding();
                if (global_bbox == null)
                    return tk.Error("internal error: global quantization requested but no bounding is available");
                memcpy(mp_bbox, global_bbox, 6 * sizeof(float));
            }
            else {
                if ((status = GetData(tk, mp_bbox, 6)) != TK_Normal)
                    return status;
            }
            mp_substage++;
        }   nobreak;

        //number of point
        case 3: {
            if ((status = GetData(tk, mp_bits_per_sample)) != TK_Normal)
                return status;
            mp_substage++;
        } nobreak;

        //workspace length (if applicable)
        case 4: {
            if (tk.GetVersion() >= 650) {
                if ((status = GetData(tk, mp_workspace_used)) != TK_Normal)
                    return status;
            }
            else {
                mp_workspace_used = 3 * mp_pointcount;
            }
            if (mp_workspace_used > mp_workspace_allocated) {
                mp_workspace_allocated = mp_workspace_used;
                ENSURE_ALLOCATED(mp_workspace, unsigned char, mp_workspace_allocated);
            }            
            mp_substage++;
        } nobreak;

        //fill the workspace
        case 5: {
            if ((status = GetData(tk, mp_workspace, mp_workspace_used)) != TK_Normal)
                return status;
            //process the workspace into points and pointcount, then deallocate
            if ((tk.GetVersion() >= 650) &&
                (mp_bits_per_sample != 8)) {
                status = unquantize_and_unpack_floats(tk, mp_pointcount, 3, mp_bits_per_sample, 
                                mp_bbox, mp_workspace, &mp_points);
            }
            else {
                status = trivial_decompress_points(
                                tk, mp_pointcount, mp_workspace,
                                &mp_points , mp_bbox);
            }
            if (status != TK_Normal)
                return status;

            mp_substage = 0; // done reading points 
        }   break;  // move on to reading faces 

        default:
            return tk.Error();
    }
    return status;

} //end function TK_Polyhedron::read_trivial_points


TK_Status TK_Polyhedron::read_vertex_normals_compressed_all (BStreamFileToolkit & tk) alter {
    TK_Status       status = TK_Normal;

#ifndef BSTREAM_DISABLE_ASCII
	if( tk.GetAsciiMode() ) 
		return read_vertex_normals_compressed_all_ascii (tk);
#endif

    ASSERT(mp_optopcode == OPT_ALL_NORMALS_COMPRESSED);
    switch (mp_substage) {
        //compression scheme
        case 0: {
            if ((status = GetData(tk, mp_compression_scheme)) != TK_Normal)
                return status;
            mp_substage++;
        }   nobreak;

        // get number of bits per sample
        case 1: {
            if ((status = GetData(tk, mp_bits_per_sample)) != TK_Normal)
                return status;
            mp_substage++;
        }   nobreak;

        case 2: {
            if (tk.GetVersion() >= 650) {
                if ((status = GetData(tk, mp_workspace_used)) != TK_Normal)
                    return status;
            }
            else {
                ASSERT(mp_compression_scheme != CS_TRIVIAL_POLAR);
                if (mp_compression_scheme == CS_TRIVIAL)
                    mp_workspace_used = 3 * mp_pointcount;
                else
                    mp_workspace_used = (mp_pointcount * mp_bits_per_sample + 7) / 8;
            }
            if (mp_workspace_used > mp_workspace_allocated) {
                mp_workspace_allocated = mp_workspace_used;
                ENSURE_ALLOCATED(mp_workspace, unsigned char, mp_workspace_allocated);
            }            
            mp_substage++;
        }   nobreak;

        //get all the compressed normals OR the individual normals
        case 3: {
            if ((status = GetData(tk, mp_workspace, mp_workspace_used)) != TK_Normal)
                return status;
            mp_substage++;
        }   nobreak;

        //process the workspace
        case 4: {
            if (mp_compression_scheme == CS_TRIVIAL ||
                mp_compression_scheme == CS_TRIVIAL_POLAR) {

                unsigned int *ptr, *end;
                ptr = mp_exists;
                end = ptr + mp_pointcount;
                while (ptr < end) {
                    *ptr |= Vertex_Normal;
                    ptr++;
                }
                mp_normalcount = mp_pointcount;

                if (tk.GetVersion() >= 650) {
                    if (mp_compression_scheme == CS_TRIVIAL_POLAR) {
                        float *temparray = null;
                        status = unquantize_and_unpack_floats(tk, mp_pointcount, 2, mp_bits_per_sample, 
                                polar_bounds, mp_workspace, &temparray);
                        mp_normals = new float[ 3 * mp_pointcount ];
                        normals_polar_to_cartesian(null, Vertex_Normal, mp_pointcount, temparray, mp_normals);
                        delete [] temparray;
                    }
                    else {
                        status = unquantize_and_unpack_floats(tk, mp_pointcount, 3, mp_bits_per_sample, 
                                normal_cube, mp_workspace, &mp_normals);
                    }
                }
                else {
                    status = trivial_decompress_points(
                            tk, mp_pointcount, mp_workspace,
                            &mp_normals , normal_cube);
                }
                if (status != TK_Normal)
                    return status;
            }
            else {
                #ifndef BSTREAM_DISABLE_REPULSE_COMPRESSION
                    ASSERT(mp_compression_scheme == CS_REPULSE);
                    ASSERT(mp_normals == null);
                    set_normals();  // allocate space
                    mp_normalcount = 0;
                    if ((status = repulse_decompress_normals(tk, mp_bits_per_sample)) != TK_Normal)
                        return status;
                #else
                    mp_normalcount = 0;
                #endif
            }
            mp_substage = 0;
        }   break;

        default:
            return tk.Error();
    }
    return status;

}  // end function read_vertex_normals_compressed_all


TK_Status TK_Polyhedron::read_vertex_normals_compressed (BStreamFileToolkit & tk) alter {
    TK_Status       status;
    unsigned char   byte;
    unsigned short  word;
    int             index;
    int             i, j;
    float           *temparray = null;

	
#ifndef BSTREAM_DISABLE_ASCII
	if( tk.GetAsciiMode() ) 
		return read_vertex_normals_compressed_ascii (tk);
#endif
 
    ASSERT(mp_optopcode == OPT_NORMALS_COMPRESSED);
    switch (mp_substage) {

        //compression scheme
        case 0: {
            if ((status = GetData(tk, mp_compression_scheme)) != TK_Normal)
                return status;
            ASSERT(mp_compression_scheme == CS_TRIVIAL ||
                    mp_compression_scheme == CS_TRIVIAL_POLAR);
            mp_substage++;
        }   nobreak;

        // get compression size
        case 1: {
            if ((status = GetData(tk, mp_bits_per_sample)) != TK_Normal)
                return status;
            mp_substage++;
        }   nobreak;

        //if all points do NOT have normals, number of individual normals
        case 2: {
            if (mp_pointcount < 256) {
                if ((status = GetData(tk, byte)) != TK_Normal)
                    return status;
                mp_normalcount = byte;
            }
            else if (mp_pointcount < 65536) {
                if ((status = GetData(tk, word)) != TK_Normal)
                    return status;
                mp_normalcount = word;
            }
            else {
                if ((status = GetData(tk, mp_normalcount)) != TK_Normal)
                    return status;
            }
            if (mp_normalcount > mp_pointcount)
                return tk.Error ("invalid vertex normal count in TK_Polyhedron::read_vertex_normals_compressed");
            mp_progress = 0;
            mp_substage++;
        }   nobreak;

        // and indices of existing normals
        case 3: {
            // read "normalcount" worth of indices & flag them
            while (mp_progress < mp_normalcount) {
                if (mp_pointcount < 256) {
                    if ((status = GetData(tk, byte)) != TK_Normal)
                        return status;
                    index = byte;
                }
                else if (mp_pointcount < 65536) {
                    if ((status = GetData(tk, word)) != TK_Normal)
                        return status;
                    index = word;
                }
                else {
                    if ((status = GetData(tk, index)) != TK_Normal)
                        return status;
                }
                if (index > mp_pointcount)
                    return tk.Error("invalid vertex normal index (2)");
                mp_exists[index] |= Vertex_Normal;
                mp_progress++;
            }
            mp_progress = 0;
            mp_substage++;
        }   nobreak;
    
        default:
            break;
    }

    if (tk.GetVersion() >= 650) {
        switch (mp_substage) {

            //case 0:
            //case 1:
            //case 2:
            //case 3: already handled

            //the data length
            case 4: {
                if ((status = GetData(tk, mp_workspace_used)) != TK_Normal)
                    return status;
                if (mp_workspace_used > mp_workspace_allocated) {
                    mp_workspace_allocated = mp_workspace_used;
                    ENSURE_ALLOCATED(mp_workspace, unsigned char, 3*mp_pointcount);
                }
                mp_progress = 0;
                mp_substage++;
            }   nobreak;

            //the raw normals data
            case 5: {
                if ((status = GetData(tk, mp_workspace, mp_workspace_used)) != TK_Normal)
                    return status;
                mp_progress = 0;
                mp_substage++;
            }   nobreak;

            //unquantize into a temporary array, then spread the values accross mp_normals
            case 6: {
                if (mp_compression_scheme == CS_TRIVIAL_POLAR) {
                    temparray = new float[ 3*mp_pointcount ];
                    status = unquantize_and_unpack_floats(tk, mp_normalcount, 2, mp_bits_per_sample, 
                            polar_bounds, mp_workspace, &temparray);
                    normals_polar_to_cartesian(null, Vertex_Normal, mp_normalcount, temparray, temparray);
                }
                else {
                    status = unquantize_and_unpack_floats(tk, mp_normalcount, 3, mp_bits_per_sample, 
                                    normal_cube, mp_workspace, &temparray);
                }
                if (status != TK_Normal)
                    return status;

                i = 0;
                j = 0;
                mp_normals = new float[ 3*mp_pointcount ];
                while (i < mp_pointcount) {
                    if (mp_exists[i] & Vertex_Normal) {
                        mp_normals[ i*3 + 0 ] = temparray[ j*3 + 0 ];
                        mp_normals[ i*3 + 1 ] = temparray[ j*3 + 1 ];
                        mp_normals[ i*3 + 2 ] = temparray[ j*3 + 2 ];
                        j++;
                    }
                    i++;
                }
                delete [] temparray;
                temparray = null;
                mp_substage = 0;
            }   break;

            default:
                return tk.Error("internal error in function read_vertex_normals_compressed");
        }
    }
    else { //version < 650
        ASSERT(mp_compression_scheme != CS_TRIVIAL_POLAR);
        switch (mp_substage) {

            //case 0:
            //case 1:
            //case 2:
            //case 3: already handled

            //the individual normals
            case 4: {
                mp_workspace_used = mp_pointcount * 3;
                if (mp_workspace_used > mp_workspace_allocated) {
                    mp_workspace_allocated = mp_workspace_used;
                    ENSURE_ALLOCATED(mp_workspace, unsigned char, 3*mp_pointcount);
                }

                // read normals associated with the flagged indices
                while (mp_progress < mp_pointcount) {
                    if (mp_exists[mp_progress] & Vertex_Normal) {
                        if ((status = GetData(tk, &mp_workspace[3*mp_progress], 3)) != TK_Normal)
                            return status;
                    }
                    mp_progress++;
                }
                mp_progress = 0;
                mp_substage++;
            }   nobreak;

            //process the workspace
            case 5: {
                status = trivial_decompress_points(
                        tk, mp_pointcount, mp_workspace,
                        &mp_normals, normal_cube);
                if (status != TK_Normal)
                    return status;
                mp_substage = 0;
            }   break;

            default:
                return tk.Error(
                    "internal error in function read_vertex_normals_compressed (version<650)");
        }
    }
    return status;

} //end function read_vertex_normals_compressed


TK_Status TK_Polyhedron::read_vertex_normals_all (BStreamFileToolkit & tk) alter {
    TK_Status       status;

	
#ifndef BSTREAM_DISABLE_ASCII
	if( tk.GetAsciiMode() ) 
		return read_vertex_normals_all_ascii (tk);
#endif

    ASSERT(mp_optopcode == OPT_ALL_NORMALS ||
            mp_optopcode == OPT_ALL_NORMALS_POLAR);
    switch (mp_substage) {
        case 0: {
            set_normals();
            mp_substage++;
        } nobreak;

        case 1: {
            if (mp_optopcode == OPT_ALL_NORMALS_POLAR) {
                if ((status = GetData(tk, mp_normals, 2 * mp_pointcount)) != TK_Normal)
                    return status;
                normals_polar_to_cartesian(null, Vertex_Normal, mp_pointcount, mp_normals, mp_normals);
            }
            else {
                if ((status = GetData(tk, mp_normals, 3 * mp_pointcount)) != TK_Normal)
                    return status;
            }
            mp_substage = 0;
            mp_normalcount = mp_pointcount;
        }   break; 

        default:
            return tk.Error();
    }
    return status;

} //end function read_vertex_normals_all


TK_Status TK_Polyhedron::read_vertex_normals (BStreamFileToolkit & tk) alter {
    TK_Status       status;
    unsigned char   byte;
    unsigned short  word;
    int             index;

	
#ifndef BSTREAM_DISABLE_ASCII
	if( tk.GetAsciiMode() ) 
		return read_vertex_normals_ascii (tk);
#endif

    ASSERT(mp_optopcode == OPT_NORMALS ||
            mp_optopcode == OPT_NORMALS_POLAR);
    switch (mp_substage) {

        //if all points do NOT have normals, number of individual normals
        case 0: {
            if (mp_pointcount < 256) {
                if ((status = GetData(tk, byte)) != TK_Normal)
                    return status;
                mp_normalcount = byte;
            }
            else if (mp_pointcount < 65536) {
                if ((status = GetData(tk, word)) != TK_Normal)
                    return status;
                mp_normalcount = word;
            }
            else {
                if ((status = GetData(tk, mp_normalcount)) != TK_Normal)
                    return status;
            }
            if (mp_normalcount > mp_pointcount)
                return tk.Error ("invalid vertex normal count in TK_Polyhedron::read_vertex_normals");
            mp_progress = 0;
            set_normals();
            mp_substage++;
        }   nobreak;

        // and indices of existing normals
        case 1: {
            // read "normalcount" worth of indices & flag them
            while (mp_progress < mp_normalcount) {
                if (mp_pointcount < 256) {
                    if ((status = GetData(tk, byte)) != TK_Normal)
                        return status;
                    index = byte;
                }
                else if (mp_pointcount < 65536) {
                    if ((status = GetData(tk, word)) != TK_Normal)
                        return status;
                    index = word;
                }
                else {
                    if ((status = GetData(tk, index)) != TK_Normal)
                        return status;
                }
                if (index > mp_pointcount)
                    return tk.Error("invalid vertex normal index");
                mp_exists[index] |= Vertex_Normal;
                mp_progress++;
            }
            mp_progress = 0;
            mp_substage++;
        }   nobreak;

        //the individual normals
        case 2: {
            // read normals associated with the flagged indices
            while (mp_progress < mp_pointcount) {
                if (mp_exists[mp_progress] & Vertex_Normal) {
                    if (mp_optopcode == OPT_NORMALS_POLAR) {
                        if ((status = GetData(tk, &mp_normals[2*mp_progress], 2)) != TK_Normal)
                            return status;
                    }
                    else {
                        if ((status = GetData(tk, &mp_normals[3*mp_progress], 3)) != TK_Normal)
                            return status;
                    }
                }
                mp_progress++;
            }
            if (mp_optopcode == OPT_NORMALS_POLAR)
                normals_polar_to_cartesian(mp_exists, Vertex_Normal, mp_pointcount, mp_normals, mp_normals);
            mp_progress = 0;
            mp_substage = 0;
        }   break;

        default:
            return tk.Error();
    }    

    return TK_Normal;

} //end function TK_Polyhedron::read_vertex_normals


TK_Status TK_Polyhedron::read_vertex_parameters_all(BStreamFileToolkit & tk) alter {
    TK_Status       status;
		
#ifndef BSTREAM_DISABLE_ASCII
	if( tk.GetAsciiMode() ) 
		return read_vertex_parameters_all_ascii (tk);
#endif
    
    ASSERT(mp_optopcode == OPT_ALL_PARAMETERS_COMPRESSED ||
           mp_optopcode == OPT_ALL_PARAMETERS);
    if (mp_optopcode == OPT_ALL_PARAMETERS) {
        switch (mp_substage) {

            // get parameter width
            case 0: {
                if ((status = GetData(tk, mp_paramwidth)) != TK_Normal)
                    return status;
                SetVertexParameters(null, mp_paramwidth);
                mp_substage++;
            }   nobreak;

            //get all the params
            case 1: {
                if ((status = GetData(tk, mp_params, mp_pointcount*mp_paramwidth)) != TK_Normal)
                    return status;
                mp_paramcount = mp_pointcount;
                mp_substage = 0;
            }   break;

            default:
                return tk.Error("internal error in read_vertex_parameters_all (uncompressed)");
        }
    }
    else if (tk.GetVersion() >= 650) {
        switch (mp_substage) {
            //compression scheme
            case 0: {
                if ((status = GetData(tk, mp_compression_scheme)) != TK_Normal)
                    return status;
                ASSERT(mp_compression_scheme == CS_TRIVIAL);
                mp_substage++;
            }   nobreak;

            // get parameter width
            case 1: {
                if (tk.GetVersion() >= 1175) {
                    if ((status = GetData(tk, mp_paramwidth)) != TK_Normal)
                        return status;
                }
                else
                    mp_paramwidth = 3;
                mp_substage++;
            }   nobreak;

            //the bounding box (note: order different from ver<650)
            case 2: {
                if ((status = GetData(tk, mp_bbox, 2*mp_paramwidth)) != TK_Normal)
                    return status;
                mp_substage++;
            } nobreak;

            // get compression size
            case 3: {
                if ((status = GetData(tk, mp_bits_per_sample)) != TK_Normal)
                    return status;
                mp_substage++;
            }   nobreak;

            //get the data length (if appropriate)
            case 4: {
                if ((status = GetData(tk, mp_workspace_used)) != TK_Normal)
                    return status;
                if (mp_workspace_used > mp_workspace_allocated) {
                    mp_workspace_allocated = mp_workspace_used;
                    ENSURE_ALLOCATED(mp_workspace, unsigned char, mp_workspace_allocated);
                }
                mp_substage++;
            }   nobreak;

            //get all the compressed params
            case 5: {
                if ((status = GetData(tk, mp_workspace, mp_workspace_used)) != TK_Normal)
                    return status;
                mp_substage++;
            }   nobreak;

            //process the workspace
            case 6: {
                status = unquantize_and_unpack_floats(tk, mp_pointcount, mp_paramwidth, mp_bits_per_sample, 
                        mp_bbox, mp_workspace, &mp_params);
                if (status != TK_Normal)
                    return status;
                mp_paramcount = mp_pointcount;
                mp_substage = 0;
            }   break;  

            default:
                return tk.Error();
        }
    }
    else { //version < 650
        mp_paramwidth = 3;
        switch (mp_substage) {
            //compression scheme
            case 0: {
                if ((status = GetData(tk, mp_compression_scheme)) != TK_Normal)
                    return status;
                ASSERT(mp_compression_scheme == CS_TRIVIAL);
                mp_substage++;
            }   nobreak;

            // get compression size
            case 1: {
                if ((status = GetData(tk, mp_bits_per_sample)) != TK_Normal)
                    return status;
                mp_substage++;
            }   nobreak;

            //the bounding box (note: order different from ver >= 650)
            case 2: {
                if ((status = GetData(tk, mp_bbox, 6)) != TK_Normal)
                    return status;
                mp_substage++;
            } nobreak;

            //get the data length (if appropriate)
            case 3: {
                mp_workspace_used = mp_paramwidth*mp_pointcount;
                if (mp_workspace_used > mp_workspace_allocated) {
                    mp_workspace_allocated = mp_workspace_used;
                    ENSURE_ALLOCATED(mp_workspace, unsigned char, mp_workspace_allocated);
                }            
                mp_substage++;
            }   nobreak;

            //get all the compressed params
            case 4: {
                if ((status = GetData(tk, mp_workspace, mp_workspace_used)) != TK_Normal)
                    return status;
                mp_substage++;
            }   nobreak;

            //process the workspace
            case 5: {
                status = trivial_decompress_points(
                            tk, mp_pointcount, mp_workspace,
                            &mp_params , mp_bbox);
                if (status != TK_Normal)
                    return status;
                mp_paramcount = mp_pointcount;
                mp_substage = 0;
            }   break;  

            default:
                return tk.Error();
        }
    }
    return TK_Normal;

} // end function TK_Polyhedron::read_vertex_parameters_all


TK_Status TK_Polyhedron::read_vertex_parameters(BStreamFileToolkit & tk) alter {
    TK_Status       status;
    int             i,j,k;
    float           *temparray = null;

		
#ifndef BSTREAM_DISABLE_ASCII
	if( tk.GetAsciiMode() ) 
		return read_vertex_parameters_ascii (tk);
#endif

    ASSERT(mp_optopcode == OPT_PARAMETERS_COMPRESSED ||
	    mp_optopcode == OPT_PARAMETERS);

    if (mp_optopcode == OPT_PARAMETERS) {
        switch (mp_substage) {

            // get parameter width
            case 0: {
                if ((status = GetData(tk, mp_paramwidth)) != TK_Normal)
                    return status;
                mp_substage++;
            }   nobreak;

            //if all points do NOT have params, number of individual params
            case 1: {
                if (mp_pointcount < 256) {
                    unsigned char       byte;

                    if ((status = GetData(tk, byte)) != TK_Normal)
                        return status;
                    mp_paramcount = byte;
                }
                else if (mp_pointcount < 65536) {
                    unsigned short      word;

                    if ((status = GetData(tk, word)) != TK_Normal)
                        return status;
                    mp_paramcount = word;
                }
                else {
                    if ((status = GetData(tk, mp_paramcount)) != TK_Normal)
                        return status;
                }
                if (mp_paramcount > mp_pointcount)
                    return tk.Error ("invalid vertex parameter count in TK_Polyhedron::read_vertex_parameters");
                mp_progress = 0;
                mp_substage++;
            }   nobreak;

            // and indices of existing params
            case 2: {
                // get "paramcount" worth of indices & flag them
                while (mp_progress < mp_paramcount) {
                    int     index;

                    if (mp_pointcount < 256) {
                        unsigned char       byte;

                        if ((status = GetData(tk, byte)) != TK_Normal)
                            return status;
                        index = byte;
                    }
                    else if (mp_pointcount < 65536) {
                        unsigned short      word;

                        if ((status = GetData(tk, word)) != TK_Normal)
                            return status;
                        index = word;
                    }
                    else {
                        if ((status = GetData(tk, index)) != TK_Normal)
                            return status;
                    }
                    if (index > mp_pointcount)
                        return tk.Error("invalid vertex parameter index");
                    mp_exists[index] |= Vertex_Parameter;
                    mp_progress++;
                }
                mp_progress = 0;
				SetVertexParameters(null, mp_paramwidth);
                mp_substage++;
            }   nobreak;

        //the individual parameters
	    case 3: {
        // read paramters associated with the flagged indices
		while (mp_progress < mp_pointcount) {
		    if (mp_exists[mp_progress] & Vertex_Parameter) {
			if ((status = GetData(tk, &mp_params[mp_paramwidth*mp_progress], mp_paramwidth)) != TK_Normal)
			    return status;
		    }
		    mp_progress++;
		}
		mp_progress = 0;
		mp_substage = 0;
	    }   break;

            default:
                return tk.Error("internal error in read_vertex_parameters (uncompressed)");
	}
    }
    else if (tk.GetVersion() >= 650) {
        switch (mp_substage) {
            //compression scheme
            case 0: {
                if ((status = GetData(tk, mp_compression_scheme)) != TK_Normal)
                    return status;
                ASSERT(mp_compression_scheme == CS_TRIVIAL);
                mp_substage++;
            }   nobreak;

            // get parameter width
            case 1: {
                if (tk.GetVersion() >= 1175) {
                    if ((status = GetData(tk, mp_paramwidth)) != TK_Normal)
                        return status;
                }
                else
                    mp_paramwidth = 3;
                mp_substage++;
            }   nobreak;

            //the bounding box (note: order different from ver<650)
            case 2: {
                if ((status = GetData(tk, mp_bbox, 2*mp_paramwidth)) != TK_Normal)
                    return status;
                mp_substage++;
            } nobreak;

            // get compression size
            case 3: {
                if ((status = GetData(tk, mp_bits_per_sample)) != TK_Normal)
                    return status;
                mp_substage++;
            }   nobreak;

            //if all points do NOT have params, number of individual params
            case 4: {
                if (mp_pointcount < 256) {
                    unsigned char       byte;

                    if ((status = GetData(tk, byte)) != TK_Normal)
                        return status;
                    mp_paramcount = byte;
                }
                else if (mp_pointcount < 65536) {
                    unsigned short      word;

                    if ((status = GetData(tk, word)) != TK_Normal)
                        return status;
                    mp_paramcount = word;
                }
                else {
                    if ((status = GetData(tk, mp_paramcount)) != TK_Normal)
                        return status;
                }
                if (mp_paramcount > mp_pointcount)
                    return tk.Error ("invalid vertex parameter count in TK_Polyhedron::read_vertex_parameters");
                mp_progress = 0;
                mp_substage++;
            }   nobreak;

            // and indices of existing params
            case 5: {
                // get "paramcount" worth of indices & flag them
                while (mp_progress < mp_paramcount) {
                    int     index;

                    if (mp_pointcount < 256) {
                        unsigned char       byte;

                        if ((status = GetData(tk, byte)) != TK_Normal)
                            return status;
                        index = byte;
                    }
                    else if (mp_pointcount < 65536) {
                        unsigned short      word;

                        if ((status = GetData(tk, word)) != TK_Normal)
                            return status;
                        index = word;
                    }
                    else {
                        if ((status = GetData(tk, index)) != TK_Normal)
                            return status;
                    }
                    if (index > mp_pointcount)
                        return tk.Error("invalid vertex parameter index");
                    mp_exists[index] |= Vertex_Parameter;
                    mp_progress++;
                }
                mp_progress = 0;
                mp_substage++;
            }   nobreak;

            //get the workspace length
            case 6: {
                if ((status = GetData(tk, mp_workspace_used)) != TK_Normal)
                    return status;
                // good time to get buffer ready
                if (mp_workspace_used > mp_workspace_allocated) {
                    mp_workspace_allocated = mp_workspace_used;
                    ENSURE_ALLOCATED(mp_workspace, unsigned char, mp_workspace_allocated);
                }            
                mp_substage++;
            }   nobreak;

            //get the raw data
            case 7: {
                if ((status = GetData(tk, mp_workspace, mp_workspace_used)) != TK_Normal)
                    return status;
                mp_substage++;
            }   nobreak;

            //process the workspace
            case 8: {
                status = unquantize_and_unpack_floats(tk, mp_pointcount, mp_paramwidth, mp_bits_per_sample, 
                                mp_bbox, mp_workspace, &temparray);
                if (status != TK_Normal)
                    return status;

                i = 0;
                j = 0;
                ENSURE_ALLOCATED(mp_params, float, mp_paramwidth*mp_pointcount);
                while (i < mp_pointcount) {
                    while (i < (mp_pointcount-1) && !(mp_exists[i] & Vertex_Parameter))
                        i++;
                    for (k = 0; k < mp_paramwidth; k++)
                        mp_params[ i*mp_paramwidth + k ] = temparray[ j*mp_paramwidth + k ];
                    i++;
                    j++;
                }
                delete [] temparray;
                temparray = null;

                mp_substage = 0;
            }   break;  

            default:
                return tk.Error("internal error in function read_vertex_parameters");
        }
    }
    else {
        mp_paramwidth = 3;
        switch (mp_substage) {
            //compression scheme
            case 0: {
                if ((status = GetData(tk, mp_compression_scheme)) != TK_Normal)
                    return status;
                ASSERT(mp_compression_scheme == CS_TRIVIAL);
                mp_substage++;
            }   nobreak;

            // get compression size
            case 1: {
                if ((status = GetData(tk, mp_bits_per_sample)) != TK_Normal)
                    return status;
                mp_substage++;
            }   nobreak;

            //the bounding box (note: order different from ver >= 650)
            case 2: {
                if ((status = GetData(tk, mp_bbox, 2*mp_paramwidth)) != TK_Normal)
                    return status;
                mp_substage++;
            } nobreak;

            //if all points do NOT have params, number of individual params
            case 3: {
                if (mp_pointcount < 256) {
                    unsigned char       byte;

                    if ((status = GetData(tk, byte)) != TK_Normal)
                        return status;
                    mp_paramcount = byte;
                }
                else if (mp_pointcount < 65536) {
                    unsigned short      word;

                    if ((status = GetData(tk, word)) != TK_Normal)
                        return status;
                    mp_paramcount = word;
                }
                else {
                    if ((status = GetData(tk, mp_paramcount)) != TK_Normal)
                        return status;
                }
                if (mp_paramcount > mp_pointcount)
                    return tk.Error ("invalid vertex parameter count in TK_Polyhedron::read_vertex_parameters");
                mp_progress = 0;
                mp_substage++;
            }   nobreak;

            // and indices of existing params
            case 4: {
                // get "paramcount" worth of indices & flag them
                while (mp_progress < mp_paramcount) {
                    int     index;

                    if (mp_pointcount < 256) {
                        unsigned char       byte;

                        if ((status = GetData(tk, byte)) != TK_Normal)
                            return status;
                        index = byte;
                    }
                    else if (mp_pointcount < 65536) {
                        unsigned short      word;

                        if ((status = GetData(tk, word)) != TK_Normal)
                            return status;
                        index = word;
                    }
                    else {
                        if ((status = GetData(tk, index)) != TK_Normal)
                            return status;
                    }
                    if (index > mp_pointcount)
                        return tk.Error("invalid vertex parameter index (2)");
                    mp_exists[index] |= Vertex_Parameter;
                    mp_progress++;
                }
                mp_progress = 0;
                // good time to get buffer ready
                mp_workspace_used = mp_pointcount * mp_paramwidth;
                if (mp_workspace_used > mp_workspace_allocated) {
                    mp_workspace_allocated = mp_workspace_used;
                    ENSURE_ALLOCATED(mp_workspace, unsigned char, mp_workspace_allocated);
                }            
                mp_substage++;
            }   nobreak;

            //get the individual params
            case 5: {
                // get params associated with the flagged indices
                while (mp_progress < mp_pointcount) {
                    if (mp_exists[mp_progress] & Vertex_Parameter) {
                        if ((status = GetData(tk, &mp_workspace[mp_paramwidth*mp_progress], 3)) != TK_Normal)
                            return status;
                        }
                        mp_progress++;
                    }
                    mp_progress = 0;
                    mp_substage++;
                }   nobreak;

            //process the workspace
            case 6: {
                status = trivial_decompress_points(
                                tk, mp_pointcount, mp_workspace,
                                &mp_params , mp_bbox);
                if (status != TK_Normal)
                    return status;
                mp_substage = 0;
            }   break;  

            default:
                return tk.Error("internal error in function read_vertex_parameters (version<650)");

        }
    }
    return TK_Normal;

} // end function TK_Polyhedron::read_vertex_parameters


TK_Status TK_Polyhedron::read_vertex_colors_all(BStreamFileToolkit & tk,
                                                 unsigned char op) alter {
    TK_Status       status;
    float           *array = null;
    unsigned int    mask;
    int             i;

	
		
#ifndef BSTREAM_DISABLE_ASCII
	if( tk.GetAsciiMode() ) 
		return read_vertex_colors_all_ascii (tk, op);
#endif

    if (tk.GetVersion() >= 650) {
        switch (mp_substage) {
            //compression scheme
            case 0: {
                ASSERT(mp_compression_scheme == CS_TRIVIAL);
                if ((status = GetData(tk, mp_compression_scheme)) != TK_Normal)
                    return status;
                mp_substage++;
            }   nobreak;

            //bits per sample
            case 1: {
                if ((status = GetData(tk, mp_bits_per_sample)) != TK_Normal)
                    return status;
                mp_substage++;
            } nobreak;

            //workspace length
            case 2: {
                if ((status = GetData(tk, mp_workspace_used)) != TK_Normal)
                    return status;
                // good time to get buffer ready
                if (mp_workspace_used > mp_workspace_allocated) {
                    mp_workspace_allocated = mp_workspace_used;
                    ENSURE_ALLOCATED(mp_workspace, unsigned char, mp_workspace_allocated);
                }            
                mp_substage++;
            } nobreak;


            //get all the colors OR the individual colors
            case 3: {
                if ((status = GetData(tk, mp_workspace, mp_workspace_used)) != TK_Normal)
                    return status;
                mp_substage++;
            }   nobreak;

            //process the workspace
            case 4: {
                status = unquantize_and_unpack_floats(tk, mp_pointcount, 3, mp_bits_per_sample, 
                                color_cube, mp_workspace, &array);
                if (status != TK_Normal)
                    return status;

                switch (op) {
                    case OPT_ALL_VFCOLORS:
                        mask = Vertex_Face_Color;
                        mp_vfcolors = array;
                        mp_vfcolorcount = mp_pointcount;
                        break;
                    case OPT_ALL_VECOLORS:
                        mask = Vertex_Edge_Color;
                        mp_vecolors = array;
                        mp_vecolorcount = mp_pointcount;
                        break;
                    case OPT_ALL_VMCOLORS:
                        mask = Vertex_Marker_Color;
                        mp_vmcolors =  array;
                        mp_vmcolorcount = mp_pointcount;
                        break;
                    default:
                        return tk.Error("internal error (2) in TK_Polyhedron::read_vertex_colors_all: unrecognized case");
                }
                for (i = 0; i < mp_pointcount; i++)
                    mp_exists[i] |= mask;

                mp_substage = 0;
            }   break;

            default:
                return tk.Error("internal error (1) in function read_vertex_colors_all");
        }
    }
    else {
        switch (mp_substage) {

             case 0: {
                // good time to get buffer ready
                mp_workspace_used = 3 * mp_pointcount;
                if (mp_workspace_used > mp_workspace_allocated) {
                    mp_workspace_allocated = mp_workspace_used;
                    ENSURE_ALLOCATED(mp_workspace, unsigned char, mp_workspace_allocated);
                }            
                mp_substage++;
            }   nobreak;


            //get all the colors OR the individual colors
            case 1: {
                if ((status = GetData(tk, mp_workspace, mp_workspace_used)) != TK_Normal)
                    return status;
                mp_substage++;
            }   nobreak;

            //process the workspace
            case 2: {
                status = trivial_decompress_points(
                                tk, mp_pointcount, mp_workspace,
                                &array , color_cube);
                if (status != TK_Normal)
                    return status;

                switch (op) {
                    case OPT_ALL_VFCOLORS:
                        mask = Vertex_Face_Color;
                        mp_vfcolors = array;
                        mp_vfcolorcount = mp_pointcount;
                        break;
                    case OPT_ALL_VECOLORS:
                        mask = Vertex_Edge_Color;
                        mp_vecolors = array;
                        mp_vecolorcount = mp_pointcount;
                        break;
                    case OPT_ALL_VMCOLORS:
                        mask = Vertex_Marker_Color;
                        mp_vmcolors =  array;
                        mp_vecolorcount = mp_pointcount;
                        break;
                    default:
                        return tk.Error("internal error (2) in TK_Polyhedron::read_vertex_colors_all: unrecognized case (version < 650)");
                }
                for (i = 0; i < mp_pointcount; i++)
                    mp_exists[i] |= mask;

                mp_substage = 0;
            }   break;

            default:
                return tk.Error("internal error (1) in function TK_Polyhedron::read_vertex_colors_all(version < 650)");
        }
    }
    return TK_Normal;

} // end function TK_Polyhedron::read_vertex_colors_all


TK_Status TK_Polyhedron::read_vertex_colors(BStreamFileToolkit & tk, unsigned char op) alter {
    TK_Status       status;
    float           *array = null, *temparray = null;
    int             i,j,count=0;
    unsigned int    mask;

		
#ifndef BSTREAM_DISABLE_ASCII
	if( tk.GetAsciiMode() ) 
		return read_vertex_colors_ascii (tk, op);
#endif

    switch (op) {
        case OPT_VERTEX_FCOLORS:
            mask = Vertex_Face_Color;
            break;
        case OPT_VERTEX_ECOLORS:
            mask = Vertex_Edge_Color;
            break;
        case OPT_VERTEX_MCOLORS:
            mask = Vertex_Marker_Color;
            break;
        default:
            return tk.Error("internal error in TK_Polyhedron::read_vertex_colors: unrecognized case");
    }

    if (tk.GetVersion() >= 650) {
        switch (mp_substage) {
            //compression scheme
            case 0: {
                ASSERT(mp_compression_scheme == CS_TRIVIAL);
                if ((status = GetData(tk, mp_compression_scheme)) != TK_Normal)
                    return status;
                mp_substage++;
            }   nobreak;

            //bits per sample
            case 1: {
                if ((status = GetData(tk, mp_bits_per_sample)) != TK_Normal)
                    return status;
                mp_substage++;
            } nobreak;

            //all points do NOT have colors, number of individual colors
            case 2: {
                if (mp_pointcount < 256) {
                    unsigned char       byte;

                    if ((status = GetData(tk, byte)) != TK_Normal)
                        return status;
                    count = byte;
                }
                else if (mp_pointcount < 65536) {
                    unsigned short      word;

                    if ((status = GetData(tk, word)) != TK_Normal)
                        return status;
                    count = word;
                }
                else {
                    if ((status = GetData(tk, count)) != TK_Normal)
                        return status;
                }
                if (count > mp_pointcount)
                    return tk.Error ("invalid vertex color count in TK_Polyhedron::read_vertex_colors");
                switch (op) {
                    case OPT_VERTEX_FCOLORS:
                        mp_vfcolorcount = count;
                        break;
                    case OPT_VERTEX_ECOLORS:
                        mp_vecolorcount = count;
                        break;
                    case OPT_VERTEX_MCOLORS:
                        mp_vmcolorcount = count;
                        break;
                    default:
                        return tk.Error("internal error (3) in TK_Polyhedron::read_vertex_colors: unrecognized case");
                }
                mp_progress = 0; //just in case
                mp_substage++;
            }   nobreak;

            // and indices of existing colors
            case 3: {
                // recover count in case we hit a pending and are returning to this loop
                switch (op) {
                    case OPT_VERTEX_FCOLORS:    count = mp_vfcolorcount;    break;
                    case OPT_VERTEX_ECOLORS:    count = mp_vecolorcount;    break;
                    case OPT_VERTEX_MCOLORS:    count = mp_vmcolorcount;    break;
                }
                while (mp_progress < count) {
                    int     index;

                    if (mp_pointcount < 256) {
                        unsigned char       byte;

                        if ((status = GetData(tk, byte)) != TK_Normal)
                            return status;
                        index = byte;
                    }
                    else if (mp_pointcount < 65536) {
                        unsigned short      word;

                        if ((status = GetData(tk, word)) != TK_Normal)
                            return status;
                        index = word;
                    }
                    else {
                        if ((status = GetData(tk, index)) != TK_Normal)
                            return status;
                    }
                    if (index > mp_pointcount)
                        return tk.Error("invalid vertex index during set color");
                    mp_exists[index] |= mask;
                    mp_progress++;
                }
                mp_progress = 0;
                mp_substage++;
            }   nobreak;


            //workspace length
            case 4: {
                if ((status = GetData(tk, mp_workspace_used)) != TK_Normal)
                    return status;
                // good time to get buffer ready
                if (mp_workspace_used > mp_workspace_allocated) {
                    mp_workspace_allocated = mp_workspace_used;
                    ENSURE_ALLOCATED(mp_workspace, unsigned char, mp_workspace_allocated);
                }            
                mp_substage++;
            } nobreak;


            //get all the colors OR the individual colors
            case 5: {
                if ((status = GetData(tk, mp_workspace, mp_workspace_used)) != TK_Normal)
                    return status;
                mp_substage++;
            }   nobreak;

            //process the workspace
            case 6: {
                status = unquantize_and_unpack_floats(tk, mp_pointcount, 3, mp_bits_per_sample, 
                                color_cube, mp_workspace, &temparray);
                if (status != TK_Normal)
                    return status;

                switch (op) {
                    case OPT_VERTEX_FCOLORS:
                        ENSURE_ALLOCATED(mp_vfcolors, float, 3*mp_pointcount);
                        array = mp_vfcolors;
                        break;
                    case OPT_VERTEX_ECOLORS:
                        ENSURE_ALLOCATED(mp_vecolors, float, 3*mp_pointcount);
                        array = mp_vecolors;
                        break;
                    case OPT_VERTEX_MCOLORS:
                        ENSURE_ALLOCATED(mp_vmcolors, float, 3*mp_pointcount);
                        array = mp_vmcolors;
                        break;
                    default:
                        return tk.Error("internal error (2) in TK_Polyhedron::read_vertex_colors: unrecognized case");
                }

                i = 0;
                j = 0;
                while (i < mp_pointcount) {
                    while (i < (mp_pointcount-1) && !(mp_exists[i] & mask))
                        i++;
                    array[ i*3 + 0 ] = temparray[ j*3 + 0 ];
                    array[ i*3 + 1 ] = temparray[ j*3 + 1 ];
                    array[ i*3 + 2 ] = temparray[ j*3 + 2 ];
                    i++;
                    j++;
                }
                delete [] temparray;
                temparray = null;

                mp_substage = 0;
            }   break;

            default:
                return tk.Error("internal error (1) in function read_vertex_colors");
        }
    }
    else {
        switch (mp_substage) {
            //all points do NOT have colors, number of individual colors
            case 0: {
                if (mp_pointcount < 256) {
                    unsigned char       byte;

                    if ((status = GetData(tk, byte)) != TK_Normal)
                        return status;

                    count = byte;
                }
                else if (mp_pointcount < 65536) {
                    unsigned short      word;

                    if ((status = GetData(tk, word)) != TK_Normal)
                        return status;

                    count = word;
                }
                else {
                    if ((status = GetData(tk, count)) != TK_Normal)
                        return status;
                }
                if (count > mp_pointcount)
                    return tk.Error ("invalid vertex color count in TK_Polyhedron::read_vertex_colors");
                switch (op) {
                    case OPT_VERTEX_FCOLORS:
                        mp_vfcolorcount = count;
                        break;
                    case OPT_VERTEX_ECOLORS:
                        mp_vecolorcount = count;
                        break;
                    case OPT_VERTEX_MCOLORS:
                        mp_vmcolorcount = count;
                        break;
                    default:
                        return tk.Error("internal error (3) in TK_Polyhedron::read_vertex_colors: unrecognized case (version<650)");
                }
                mp_progress = 0;
                mp_substage++;
            }   nobreak;

            // and indices of existing colors
            case 1: {
                // recover count in case we hit a pending and are returning to this loop
                switch (op) {
                    case OPT_VERTEX_FCOLORS:    count = mp_vfcolorcount;    break;
                    case OPT_VERTEX_ECOLORS:    count = mp_vecolorcount;    break;
                    case OPT_VERTEX_MCOLORS:    count = mp_vmcolorcount;    break;
                }

                while (mp_progress < count) {
                    int     index;

                    if (mp_pointcount < 256) {
                        unsigned char       byte;

                        if ((status = GetData(tk, byte)) != TK_Normal)
                            return status;

                        index = byte;
                    }
                    else if (mp_pointcount < 65536) {
                        unsigned short      word;

                        if ((status = GetData(tk, word)) != TK_Normal)
                            return status;

                        index = word;
                    }
                    else {
                        if ((status = GetData(tk, index)) != TK_Normal)
                            return status;
                    }
                    if (index > mp_pointcount)
                        return tk.Error("invalid vertex index during set color(2)");
                    mp_exists[index] |= mask;
                    mp_progress++;
                }
                mp_progress = 0; //just in case
                // good time to get buffer ready
                mp_workspace_used = mp_pointcount * 3;
                if (mp_workspace_used > mp_workspace_allocated) {
                    mp_workspace_allocated = mp_workspace_used;
                    ENSURE_ALLOCATED(mp_workspace, unsigned char, mp_workspace_allocated);
                }            
                mp_substage++;
            }   nobreak;

            //get all the colors OR the individual colors
            case 2: {
                while (mp_progress < mp_pointcount) {
                    if ((mp_exists[mp_progress] & mask) == mask) {
                        if ((status = GetData(tk, &mp_workspace[3*mp_progress], 3)) != TK_Normal)
                            return status;
                    }
                    mp_progress++;
                }
                mp_progress = 0;
                mp_substage++;
            }   nobreak;

            //process the workspace
            case 3: {
                status = trivial_decompress_points(
                                tk, mp_pointcount, mp_workspace,
                                &array , color_cube);
                if (status != TK_Normal)
                    return status;

                switch (op) {
                    case OPT_VERTEX_FCOLORS:
                        mp_vfcolors = array;
                        break;
                    case OPT_VERTEX_ECOLORS:
                        mp_vecolors = array;
                        break;
                    case OPT_VERTEX_MCOLORS:
                        mp_vmcolors = array;
                        break;
                    default:
                        return tk.Error("internal error (2) in TK_Polyhedron::read_vertex_colors: unrecognized case (version < 650)");
                }

                mp_substage = 0;
            }   break;

            default:
                return tk.Error("internal error (1) in function TK_Polyhedron::read_vertex_colors(version < 650)");
        }
    }
    return TK_Normal;

} // end function TK_Polyhedron::read_vertex_colors


TK_Status TK_Polyhedron::read_vertex_indices_all(BStreamFileToolkit & tk,
                                                 unsigned char op) alter {
    TK_Status       status;
    float           *array = null;
    unsigned int    mask;
    int             i;


#ifndef BSTREAM_DISABLE_ASCII
	if( tk.GetAsciiMode() ) 
		return read_vertex_indices_all_ascii (tk, op);
#endif



    if (tk.GetVersion() >= 650) {
        switch (mp_substage) {
            //compression scheme
            case 0: {
                ASSERT(mp_compression_scheme == CS_TRIVIAL);
                if ((status = GetData(tk, mp_compression_scheme)) != TK_Normal)
                    return status;
                mp_substage++;
            }   nobreak;

            //bounding box
            case 1: {
                if ((status = GetData(tk, mp_bbox, 6)) != TK_Normal)
                    return status;
                mp_substage++;
            } nobreak;

            //bits per sample
            case 2: {
                if ((status = GetData(tk, mp_bits_per_sample)) != TK_Normal)
                    return status;
                mp_substage++;
            } nobreak;

            //workspace length
            case 3: {
                if ((status = GetData(tk, mp_workspace_used)) != TK_Normal)
                    return status;
                // good time to get buffer ready
                if (mp_workspace_used > mp_workspace_allocated) {
                    mp_workspace_allocated = mp_workspace_used;
                    ENSURE_ALLOCATED(mp_workspace, unsigned char, mp_workspace_allocated);
                }            
                mp_substage++;
            } nobreak;


            //get all the index data
            case 4: {
                if ((status = GetData(tk, mp_workspace, mp_workspace_used)) != TK_Normal)
                    return status;
                mp_substage++;
            }   nobreak;

            //process the workspace
            case 5: {
                status = unquantize_and_unpack_floats(tk, mp_pointcount, 1, mp_bits_per_sample, 
                                mp_bbox, mp_workspace, &array);
                if (status != TK_Normal)
                    return status;

                switch (op) {
                    case OPT_ALL_VFINDICES:
                        mask = Vertex_Face_Index;
                        mp_vfindices = array;
                        mp_vfindexcount = mp_pointcount;
                        break;
                    case OPT_ALL_VEINDICES:
                        mask = Vertex_Edge_Index;
                        mp_veindices = array;
                        mp_veindexcount = mp_pointcount;
                        break;
                    case OPT_ALL_VMINDICES:
                        mask = Vertex_Marker_Index;
                        mp_vmindices =  array;
                        mp_vmindexcount = mp_pointcount;
                        break;
                    default:
                        return tk.Error("internal error (2) in TK_Polyhedron::read_vertex_indices_all: unrecognized case");
                }
                for (i = 0; i < mp_pointcount; i++)
                    mp_exists[i] |= mask;

                mp_substage = 0;
            }   break;

            default:
                return tk.Error("internal error (1) in function read_vertex_indices_all");
        }
    }
    else {
        switch (mp_substage) {
            case 0: {
                switch (op) {
                    case OPT_ALL_VFINDICES:
                        ENSURE_ALLOCATED(mp_vfindices, float, mp_pointcount);
                        break;
                    case OPT_ALL_VEINDICES:
                        ENSURE_ALLOCATED(mp_veindices, float, mp_pointcount);
                        break;
                    case OPT_ALL_VMINDICES:
                        ENSURE_ALLOCATED(mp_vmindices, float, mp_pointcount);
                        break;
                    default:
                        return tk.Error("internal error (1) in TK_Polyhedron::read_vertex_indices_all: unrecognized case");
                }
                mp_substage++;
            }   nobreak;

            //get all the indices (uncompressed)
            case 1: {
                switch (op) {
                    case OPT_ALL_VFINDICES:
                        mask = Vertex_Face_Index;
                        array = mp_vfindices;
                        mp_vfindexcount = mp_pointcount;
                        break;
                    case OPT_ALL_VEINDICES:
                        mask = Vertex_Edge_Index;
                        array = mp_veindices;
                        mp_veindexcount = mp_pointcount;
                        break;
                    case OPT_ALL_VMINDICES:
                        mask = Vertex_Marker_Index;
                        array = mp_vmindices;
                        mp_vmindexcount = mp_pointcount;
                        break;
                    default:
                        return tk.Error("internal error (2) in TK_Polyhedron::read_vertex_indices_all: unrecognized case");
                }
                if ((status = GetData(tk, array, mp_pointcount)) != TK_Normal)
                    return status;
                for (i = 0; i < mp_pointcount; i++)
                    mp_exists[i] |= mask;
                mp_substage = 0;
            }   break;

            default:
                return tk.Error("internal error in TK_Polyhedron::read_vertex_indices_all(ver<650)");
        }
    }
    return TK_Normal;

} // end function TK_Polyhedron::read_vertex_indices_all


TK_Status TK_Polyhedron::read_vertex_indices(BStreamFileToolkit & tk, unsigned char op) alter {
    TK_Status       status;
    float           *array = null, *temparray = null;
    int             i,j,count=0;
    unsigned int    mask;


#ifndef BSTREAM_DISABLE_ASCII
	if( tk.GetAsciiMode() ) 
		return read_vertex_indices_ascii (tk, op);
#endif


    switch (op) {
        case OPT_VERTEX_FINDICES:
            mask = Vertex_Face_Index;
            break;
        case OPT_VERTEX_EINDICES:
            mask = Vertex_Edge_Index;
            break;
        case OPT_VERTEX_MINDICES:
            mask = Vertex_Marker_Index;
            break;
        default:
            return tk.Error("internal error in TK_Polyhedron::read_vertex_indices: unrecognized case");
    }

    if (tk.GetVersion() >= 650) {
        switch (mp_substage) {
            //compression scheme
            case 0: {
                ASSERT(mp_compression_scheme == CS_TRIVIAL);
                if ((status = GetData(tk, mp_compression_scheme)) != TK_Normal)
                    return status;
                mp_substage++;
            }   nobreak;

            //bounding box
            case 1: {
                if ((status = GetData(tk, mp_bbox, 6)) != TK_Normal)
                    return status;
                mp_substage++;
            } nobreak;

            //bits per sample
            case 2: {
                if ((status = GetData(tk, mp_bits_per_sample)) != TK_Normal)
                    return status;
                mp_substage++;
            } nobreak;

            //all points do NOT have indices, number of individual indices
            case 3: {
                if (mp_pointcount < 256) {
                    unsigned char       byte;

                    if ((status = GetData(tk, byte)) != TK_Normal)
                        return status;
                    count = byte;
                }
                else if (mp_pointcount < 65536) {
                    unsigned short      word;

                    if ((status = GetData(tk, word)) != TK_Normal)
                        return status;
                    count = word;
                }
                else {
                    if ((status = GetData(tk, count)) != TK_Normal)
                        return status;
                }
                if (count > mp_pointcount)
                    return tk.Error ("invalid vertex index count in TK_Polyhedron::read_vertex_indices");
                switch (op) {
                    case OPT_VERTEX_FINDICES:
                        mp_vfindexcount = count;
                        break;
                    case OPT_VERTEX_EINDICES:
                        mp_veindexcount = count;
                        break;
                    case OPT_VERTEX_MINDICES:
                        mp_vmindexcount = count;
                        break;
                    default:
                        return tk.Error("internal error (3) in TK_Polyhedron::read_vertex_indices: unrecognized case");
                }
                mp_progress = 0; //just in case
                mp_substage++;
            }   nobreak;

            // and indices of existing indices
            case 4: {
                // recover count in case we hit a pending and are returning to this loop
                switch (op) {
                    case OPT_VERTEX_FINDICES:   count = mp_vfindexcount;    break;
                    case OPT_VERTEX_EINDICES:   count = mp_veindexcount;    break;
                    case OPT_VERTEX_MINDICES:   count = mp_vmindexcount;    break;
                }
                while (mp_progress < count) {
                    int     index;

                    if (mp_pointcount < 256) {
                        unsigned char       byte;

                        if ((status = GetData(tk, byte)) != TK_Normal)
                            return status;
                        index = byte;
                    }
                    else if (mp_pointcount < 65536) {
                        unsigned short      word;

                        if ((status = GetData(tk, word)) != TK_Normal)
                            return status;
                        index = word;
                    }
                    else {
                        if ((status = GetData(tk, index)) != TK_Normal)
                            return status;
                    }
                    if (index > mp_pointcount)
                        return tk.Error("invalid vertex index during set color by index");
                    mp_exists[index] |= mask;
                    mp_progress++;
                }
                mp_progress = 0;
                mp_substage++;
            }   nobreak;


            //workspace length
            case 5: {
                if ((status = GetData(tk, mp_workspace_used)) != TK_Normal)
                    return status;
                // good time to get buffer ready
                if (mp_workspace_used > mp_workspace_allocated) {
                    mp_workspace_allocated = mp_workspace_used;
                    ENSURE_ALLOCATED(mp_workspace, unsigned char, mp_workspace_allocated);
                }            
                mp_substage++;
            } nobreak;


            //get all the indices OR the individual indices
            case 6: {
                if ((status = GetData(tk, mp_workspace, mp_workspace_used)) != TK_Normal)
                    return status;
                mp_substage++;
            }   nobreak;

            //process the workspace
            case 7: {
                status = unquantize_and_unpack_floats(tk, mp_pointcount, 1, mp_bits_per_sample, 
                               mp_bbox, mp_workspace, &temparray);
                if (status != TK_Normal)
                    return status;

                switch (op) {
                    case OPT_VERTEX_FINDICES:
                        ENSURE_ALLOCATED(mp_vfindices, float, mp_pointcount);
                        array = mp_vfindices;
                        break;
                    case OPT_VERTEX_EINDICES:
                        ENSURE_ALLOCATED(mp_veindices, float, mp_pointcount);
                        array = mp_veindices;
                        break;
                    case OPT_VERTEX_MINDICES:
                        ENSURE_ALLOCATED(mp_vmindices, float, mp_pointcount);
                        array = mp_vmindices;
                        break;
                    default:
                        return tk.Error("internal error (2) in TK_Polyhedron::read_vertex_indices: unrecognized case");
                }

                i = 0;
                j = 0;
                while (i < mp_pointcount) {
                    while (i < (mp_pointcount-1) && !(mp_exists[i] & mask))
                        i++;
                    array[ i ] = temparray[ j ];
                    i++;
                    j++;
                }
                delete [] temparray;
                temparray = null;

                mp_substage = 0;
            }   break;

            default:
                return tk.Error("internal error (1) in function read_vertex_indices");
        }
    }
    else {
        switch (mp_substage) {
            //all points do NOT have indices, number of individual indices
            case 0: {
                if (mp_pointcount < 256) {
                    unsigned char       byte;

                    if ((status = GetData(tk, byte)) != TK_Normal)
                        return status;
                    count = byte;
                }
                else if (mp_pointcount < 65536) {
                    unsigned short      word;

                    if ((status = GetData(tk, word)) != TK_Normal)
                        return status;
                    count = word;
                }
                else {
                    if ((status = GetData(tk, count)) != TK_Normal)
                        return status;
                }
                if (count > mp_pointcount)
                    return tk.Error ("invalid vertex index count in TK_Polyhedron::read_vertex_indices");
                switch (op) {
                    case OPT_VERTEX_FINDICES:
                        mp_vfindexcount = count;
                        ENSURE_ALLOCATED(mp_vfindices, float, mp_pointcount);
                        break;
                    case OPT_VERTEX_EINDICES:
                        mp_veindexcount = count;
                        ENSURE_ALLOCATED(mp_veindices, float, mp_pointcount);
                        break;
                    case OPT_VERTEX_MINDICES:
                        mp_vmindexcount = count;
                        ENSURE_ALLOCATED(mp_vmindices, float, mp_pointcount);
                        break;
                    default:
                        return tk.Error("internal error (1) in TK_Polyhedron::read_vertex_indices: unrecognized case (version<650)");
                }
                mp_progress = 0;
                mp_substage++;
            }   nobreak;

            // and indices of existing colors-by-index
            case 1: {
                // recover count in case we hit a pending and are returning to this loop
                switch (op) {
                    case OPT_VERTEX_FINDICES:   count = mp_vfindexcount;    break;
                    case OPT_VERTEX_EINDICES:   count = mp_veindexcount;    break;
                    case OPT_VERTEX_MINDICES:   count = mp_vmindexcount;    break;
                }

                while (mp_progress < count) {
                    int     index;

                    if (mp_pointcount < 256) {
                        unsigned char       byte;

                        if ((status = GetData(tk, byte)) != TK_Normal)
                            return status;
                        index = byte;
                    }
                    else if (mp_pointcount < 65536) {
                        unsigned short      word;

                        if ((status = GetData(tk, word)) != TK_Normal)
                            return status;
                        index = word;
                    }
                    else {
                        if ((status = GetData(tk, index)) != TK_Normal)
                            return status;
                    }
                    if (index > mp_pointcount)
                        return tk.Error("invalid vertex index during set color by index (2)");
                    mp_exists[index] |= mask;
                    mp_progress++;
                }
                mp_progress = 0;
                mp_substage++;
            }   nobreak;

            //get the individual indices (uncompressed)
            case 2: {
                switch (op) {
                    case OPT_VERTEX_FINDICES:
                        array = mp_vfindices;
                        break;
                    case OPT_VERTEX_EINDICES:
                        array = mp_veindices;
                        break;
                    case OPT_VERTEX_MINDICES:
                        array = mp_vmindices;
                        break;
                    default:
                        return tk.Error("internal error (2) in TK_Polyhedron::read_vertex_indices: unrecognized case (version < 650)");
                }
                while (mp_progress < mp_pointcount) {
                    if ((mp_exists[mp_progress] & mask) == mask) {
                        if ((status = GetData(tk, &array[mp_progress], 1)) != TK_Normal)
                            return status;
                    }
                    mp_progress++;
                }
                mp_progress = 0;
                mp_substage = 0;
            }   break;

            default:
                return tk.Error("internal error (3) in function TK_Polyhedron::read_vertex_indices(version < 650)");
        }
    }
    return TK_Normal;

} // end function TK_Polyhedron::read_vertex_indices


TK_Status TK_Polyhedron::read_face_colors_all(BStreamFileToolkit & tk) alter {
    TK_Status       status;
    int             i;


#ifndef BSTREAM_DISABLE_ASCII
	if( tk.GetAsciiMode() ) 
		return read_face_colors_all_ascii (tk);
#endif


    ASSERT(mp_optopcode == OPT_ALL_FACE_COLORS);

    if (tk.GetVersion() >= 650) {

        switch (mp_substage) {
            case 0: {
                if ((status = GetData(tk, mp_compression_scheme)) != TK_Normal)
                    return status;
                mp_substage++;
            }   nobreak;

            case 1: {
                if ((status = GetData(tk, mp_bits_per_sample)) != TK_Normal)
                    return status;
                mp_substage++;
            }   nobreak;

            //the workspace length
            case 2: {
                if ((status = GetData(tk, mp_workspace_used)) != TK_Normal)
                    return status;
                if (mp_workspace_used > mp_workspace_allocated) {
                    mp_workspace_allocated = mp_workspace_used;
                    ENSURE_ALLOCATED(mp_workspace, unsigned char, mp_workspace_used);
                }
                mp_substage++;
            }   nobreak;

            //get color data
            case 3: {
                if ((status = GetData(tk, mp_workspace, mp_workspace_used)) != TK_Normal)
                    return status;
                mp_substage++;
            }   nobreak;

            //process the workspace
            case 4: {
                status = unquantize_and_unpack_floats(tk, mp_facecount, 3, mp_bits_per_sample, 
                                color_cube, mp_workspace, &mp_fcolors);
                if (status != TK_Normal)
                    return status;
                mp_fcolorcount = mp_facecount;
                for (i = 0; i < mp_facecount; i++)
                    mp_face_exists[i] |= Face_Color;
                mp_substage = 0;
            }   break;

            default:
                return tk.Error("internal error in TK_Polyhedron::read_face_colors");
        }
    }
    else { //version < 650
        switch (mp_substage) {

            //allocate the workspace
            case 0: {
                mp_workspace_used = 3 * mp_facecount;
                if (mp_workspace_used > mp_workspace_allocated) {
                    mp_workspace_allocated = mp_workspace_used;
                    ENSURE_ALLOCATED(mp_workspace, unsigned char, mp_workspace_used);
                }
                mp_substage++;
            }   nobreak;

            //get the data
            case 1: {
                if ((status = GetData(tk, mp_workspace, mp_workspace_used)) != TK_Normal)
                    return status;
                mp_substage++;
            }   nobreak;

            //process the data
            case 2: {
                status = trivial_decompress_points(
                                    tk, mp_facecount, mp_workspace,
                                    &mp_fcolors, color_cube);
                if (status != TK_Normal)
                    return status;
                mp_fcolorcount = mp_facecount;
                for (i = 0; i < mp_facecount; i++)
                    mp_face_exists[i] |= Face_Color;
                mp_substage = 0;
            }   break;

            default:
                return tk.Error("internal error in TK_Polyhedron::read_face_colors_all(ver<650)");
        }
    }
    return TK_Normal;

} // end function TK_Polyhedron::read_face_colors_all


TK_Status TK_Polyhedron::read_face_colors(BStreamFileToolkit & tk) alter {
    TK_Status       status;
    int             i, j;
    float           *temparray = null;


#ifndef BSTREAM_DISABLE_ASCII
	if( tk.GetAsciiMode() ) 
		return read_face_colors_ascii (tk);
#endif


    ASSERT(mp_optopcode == OPT_FACE_COLORS);
    if (tk.GetVersion() >= 650) {

        switch (mp_substage) {
            case 0: {
                if ((status = GetData(tk, mp_compression_scheme)) != TK_Normal)
                    return status;
                mp_substage++;
            }   nobreak;

            case 1: {
                if ((status = GetData(tk, mp_bits_per_sample)) != TK_Normal)
                    return status;
                mp_substage++;
            }   nobreak;

            //if all faces do NOT have colors, number of individual colors
            case 2: {
                if (mp_facecount < 256) {
                    unsigned char       byte;

                    if ((status = GetData(tk, byte)) != TK_Normal)
                        return status;
                    mp_fcolorcount = byte;
                }
                else if (mp_facecount < 65536) {
                    unsigned short      word;

                    if ((status = GetData(tk, word)) != TK_Normal)
                        return status;
                    mp_fcolorcount = word;
                }
                else {
                    if ((status = GetData(tk, mp_fcolorcount)) != TK_Normal)
                        return status;
                }
                if (mp_fcolorcount > mp_facecount)
                    return tk.Error ("invalid face color count in TK_Polyhedron::read_face_colors");
                mp_progress = 0;

                // good time to get the buffer ready
                if (mp_fcolors == null)
                    set_fcolors();
                mp_substage++;
            }   nobreak;

            // and indices of existing colors
            case 3: {
                // get "count" worth of colors & flag them
                while (mp_progress < mp_fcolorcount) {
                    int     color;

                    if (mp_facecount < 256) {
                        unsigned char       byte;

                        if ((status = GetData(tk, byte)) != TK_Normal)
                            return status;
                        color = byte;
                    }
                    else if (mp_facecount < 65536) {
                        unsigned short      word;

                        if ((status = GetData(tk, word)) != TK_Normal)
                            return status;
                        color = word;
                    }
                    else {
                        if ((status = GetData(tk, color)) != TK_Normal)
                            return status;
                    }
                    mp_face_exists[color] |= Face_Color;
                    mp_progress++;
                }
                mp_progress = 0;
                mp_substage++;
            }   nobreak;

            //the workspace length
            case 4: {
                if ((status = GetData(tk, mp_workspace_used)) != TK_Normal)
                    return status;                
                if (mp_workspace_used > mp_workspace_allocated) {
                    mp_workspace_allocated = mp_workspace_used;
                    ENSURE_ALLOCATED(mp_workspace, unsigned char, mp_workspace_used);
                }
                mp_substage++;
            }   nobreak;

            //get color data
            case 5: {
                if ((status = GetData(tk, mp_workspace, mp_workspace_used)) != TK_Normal)
                    return status;
                mp_substage++;
            }   nobreak;

            //process the workspace
            case 6: {
                status = unquantize_and_unpack_floats(tk, mp_fcolorcount, 3, mp_bits_per_sample, 
                                color_cube, mp_workspace, &temparray);
                if (status != TK_Normal)
                    return status;

                i = 0;
                j = 0;
                ENSURE_ALLOCATED(mp_fcolors, float, mp_facecount * 3);
                while (i < mp_facecount) {
                    while (i < (mp_facecount-1) && !(mp_face_exists[i] & Face_Color))
                        i++;
                    mp_fcolors[ i*3 + 0 ] = temparray[ j*3 + 0 ];
                    mp_fcolors[ i*3 + 1 ] = temparray[ j*3 + 1 ];
                    mp_fcolors[ i*3 + 2 ] = temparray[ j*3 + 2 ];
                    i++;
                    j++;
                }
                delete [] temparray;
                temparray = null;
                mp_substage = 0;
            }   break;

            default:
                return tk.Error("internal error in TK_Polyhedron::read_face_colors");
        }
    }
    else { //version < 650
        switch (mp_substage) {
            //if all faces do NOT have colors, number of individual colors
            case 0: {
                if (mp_facecount < 256) {
                    unsigned char       byte;

                    if ((status = GetData(tk, byte)) != TK_Normal)
                        return status;
                    mp_fcolorcount = byte;
                }
                else if (mp_facecount < 65536) {
                    unsigned short      word;

                    if ((status = GetData(tk, word)) != TK_Normal)
                        return status;
                    mp_fcolorcount = word;
                }
                else {
                    if ((status = GetData(tk, mp_fcolorcount)) != TK_Normal)
                        return status;
                }
                if (mp_fcolorcount > mp_facecount)
                    return tk.Error ("invalid face color count in TK_Polyhedron::read_face_colors");
                mp_progress = 0;

                // good time to get the buffer ready
                if (mp_fcolors == null)
                    set_fcolors();
                mp_substage++;
            }   nobreak;

            // and colors of existing colors
            case 1: {
                // get "count" worth of colors & flag them
                while (mp_progress < mp_fcolorcount) {
                    int     color;

                    if (mp_facecount < 256) {
                        unsigned char       byte;

                        if ((status = GetData(tk, byte)) != TK_Normal)
                            return status;
                        color = byte;
                    }
                    else if (mp_facecount < 65536) {
                        unsigned short      word;

                        if ((status = GetData(tk, word)) != TK_Normal)
                            return status;
                        color = word;
                    }
                    else {
                        if ((status = GetData(tk, color)) != TK_Normal)
                            return status;
                    }
                    mp_face_exists[color] |= Face_Color;
                    mp_progress++;
                }
                mp_progress = 0;
                // good time to get buffer ready
                mp_workspace_used = mp_facecount * 3;
                if (mp_workspace_used > mp_workspace_allocated) {
                    mp_workspace_allocated = mp_workspace_used;
                    ENSURE_ALLOCATED(mp_workspace, unsigned char, mp_workspace_allocated);
                }            
                mp_substage++;
            }   nobreak;

            //get the individual colors
            case 2: {
                // read normals associated with the flagged indices
                while (mp_progress < mp_facecount) {
                    if (mp_face_exists[mp_progress] & Face_Color) {
                        if ((status = GetData(tk, &mp_workspace[3*mp_progress], 3)) != TK_Normal)
                            return status;
                    }
                    mp_progress++;
                }
                mp_progress = 0;
                mp_substage++;
            }   nobreak;

            //process the workspace
            case 3: {
                status = trivial_decompress_points(
                                tk, mp_facecount, mp_workspace,
                                &mp_fcolors, color_cube);
                if (status != TK_Normal)
                    return status;
                mp_substage = 0;
            }   break;

            default:
                return tk.Error("internal error in TK_Polyhedron::read_face_colors(ver<650)");
        }
    }
    return TK_Normal;

} // end function TK_Polyhedron::read_face_colors 


static void snap_to_int (int count, float *array)
{
	const float eps = 1e-5f;
	float diff, nearest;

	while (count-- != 0) {
		nearest = (float)(int)(*array + 0.5);
		diff = *array - nearest;
		if (-eps < diff && diff < eps)
			*array = nearest;
		array++;
	}
}


TK_Status TK_Polyhedron::read_face_indices_all(BStreamFileToolkit & tk) alter {
    TK_Status       status;
    int             i;


#ifndef BSTREAM_DISABLE_ASCII
	if( tk.GetAsciiMode() ) 
		return read_face_indices_all_ascii (tk);
#endif

    ASSERT(mp_optopcode == OPT_ALL_FACE_INDICES);
    if (tk.GetVersion() >= 650) {

        switch (mp_substage) {
            case 0: {
                if ((status = GetData(tk, mp_compression_scheme)) != TK_Normal)
                    return status;
                mp_substage++;
            }   nobreak;

            case 1: {
                if ((status = GetData(tk, mp_bbox, 6)) != TK_Normal)
                    return status;
                mp_substage++;
            }   nobreak;

            case 2: {
                if ((status = GetData(tk, mp_bits_per_sample)) != TK_Normal)
                    return status;
                mp_substage++;
            }   nobreak;

            //the workspace length
            case 3: {
                if ((status = GetData(tk, mp_workspace_used)) != TK_Normal)
                    return status;
                if (mp_workspace_used > mp_workspace_allocated) {
                    mp_workspace_allocated = mp_workspace_used;
                    ENSURE_ALLOCATED(mp_workspace, unsigned char, mp_workspace_used);
                }
                mp_substage++;
            }   nobreak;

            //get index data
            case 4: {
                if ((status = GetData(tk, mp_workspace, mp_workspace_used)) != TK_Normal)
                    return status;
                mp_substage++;
            }   nobreak;

            //process the workspace
            case 5: {
                status = unquantize_and_unpack_floats(tk, mp_facecount, 1, mp_bits_per_sample, 
                                mp_bbox, mp_workspace, &mp_findices);
                if (status != TK_Normal)
                    return status;
                snap_to_int (mp_facecount, mp_findices);

                mp_findexcount = mp_facecount;
                for (i = 0; i < mp_facecount; i++)
                    mp_face_exists[i] |= Face_Index;
                mp_substage = 0;
            }   break;

            default:
                return tk.Error("internal error in TK_Polyhedron::read_face_indices");
        }
    }
    else { //version < 650
        switch (mp_substage) {
            case 0: {
                ENSURE_ALLOCATED(mp_findices, float, mp_facecount);
                mp_substage++;
            }   nobreak;

            //get all the indices OR the individual indices
            case 1: {
                if ((status = GetData(tk, mp_findices, mp_facecount)) != TK_Normal)
                    return status;
                mp_findexcount = mp_facecount;
                for (i = 0; i < mp_facecount; i++)
                    mp_face_exists[i] |= Face_Index;
                mp_substage = 0;
            }   break;

            default:
                return tk.Error("internal error in TK_Polyhedron::read_face_indices_all(ver<650)");
        }
    }
    return TK_Normal;

} // end function TK_Polyhedron::read_face_indices_all


TK_Status TK_Polyhedron::read_face_indices(BStreamFileToolkit & tk) alter {
    TK_Status       status;
    int             i, j;
    float           *temparray = null;

#ifndef BSTREAM_DISABLE_ASCII
	if( tk.GetAsciiMode() ) 
		return read_face_indices_ascii (tk);
#endif

    ASSERT(mp_optopcode == OPT_FACE_INDICES);
    if (tk.GetVersion() >= 650) {

        switch (mp_substage) {
            case 0: {
                if ((status = GetData(tk, mp_compression_scheme)) != TK_Normal)
                    return status;
                mp_substage++;
            }   nobreak;

            case 1: {
                if ((status = GetData(tk, mp_bbox, 6)) != TK_Normal)
                    return status;
                mp_substage++;
            }   nobreak;

            case 2: {
                if ((status = GetData(tk, mp_bits_per_sample)) != TK_Normal)
                    return status;
                mp_substage++;
            }   nobreak;

            //if all faces do NOT have indices, number of individual indices
            case 3: {
                if (mp_facecount < 256) {
                    unsigned char       byte;

                    if ((status = GetData(tk, byte)) != TK_Normal)
                        return status;
                    mp_findexcount = byte;
                }
                else if (mp_facecount < 65536) {
                    unsigned short      word;

                    if ((status = GetData(tk, word)) != TK_Normal)
                        return status;
                    mp_findexcount = word;
                }
                else {
                    if ((status = GetData(tk, mp_findexcount)) != TK_Normal)
                        return status;
                }
                if (mp_findexcount > mp_facecount)
                    return tk.Error ("invalid face index count in TK_Polyhedron::read_face_indices");
                mp_progress = 0;

                // good time to get the buffer ready
                if (mp_findices == null)
                    set_findices();
                mp_substage++;
            }   nobreak;

            // and indices of existing indices
            case 4: {
                // get "count" worth of indices & flag them
                while (mp_progress < mp_findexcount) {
                    int     index;

                    if (mp_facecount < 256) {
                        unsigned char       byte;

                        if ((status = GetData(tk, byte)) != TK_Normal)
                            return status;
                        index = byte;
                    }
                    else if (mp_facecount < 65536) {
                        unsigned short      word;

                        if ((status = GetData(tk, word)) != TK_Normal)
                            return status;
                        index = word;
                    }
                    else {
                        if ((status = GetData(tk, index)) != TK_Normal)
                            return status;
                    }
                    mp_face_exists[index] |= Face_Index;
                    mp_progress++;
                }
                mp_progress = 0;
                mp_substage++;
            }   nobreak;

            //the workspace length
            case 5: {
                if ((status = GetData(tk, mp_workspace_used)) != TK_Normal)
                    return status;
                if (mp_workspace_used > mp_workspace_allocated) {
                    mp_workspace_allocated = mp_workspace_used;
                    ENSURE_ALLOCATED(mp_workspace, unsigned char, mp_workspace_used);
                }
                mp_substage++;
            }   nobreak;

            //get index data
            case 6: {
                if ((status = GetData(tk, mp_workspace, mp_workspace_used)) != TK_Normal)
                    return status;
                mp_substage++;
            }   nobreak;

            //process the workspace
            case 7: {
                status = unquantize_and_unpack_floats(tk, mp_findexcount, 1, mp_bits_per_sample, 
                                mp_bbox, mp_workspace, &temparray);
                if (status != TK_Normal)
                    return status;
                snap_to_int (mp_findexcount, temparray);

                i = 0;
                j = 0;
                ENSURE_ALLOCATED(mp_findices, float, mp_facecount);
                while (i < mp_facecount) {
                    while (i < (mp_facecount-1) && !(mp_face_exists[i] & Face_Index))
                        i++;
                    mp_findices[ i ] = temparray[ j ];
                    i++;
                    j++;
                }
                delete [] temparray;
                temparray = null;
                mp_substage = 0;
            }   break;

            default:
                return tk.Error("internal error in TK_Polyhedron::read_face_indices");
        }
    }
    else { //version < 650
        switch (mp_substage) {
            //if all faces do NOT have indices, number of individual indices
            case 0: {
                if (mp_facecount < 256) {
                    unsigned char       byte;

                    if ((status = GetData(tk, byte)) != TK_Normal)
                        return status;
                    mp_findexcount = byte;
                }
                else if (mp_facecount < 65536) {
                    unsigned short      word;

                    if ((status = GetData(tk, word)) != TK_Normal)
                        return status;
                    mp_findexcount = word;
                }
                else {
                    if ((status = GetData(tk, mp_findexcount)) != TK_Normal)
                        return status;
                }
                if (mp_findexcount > mp_facecount)
                    return tk.Error ("invalid face index count in TK_Polyhedron::read_face_indices");
                mp_progress = 0;

                // good time to get the buffer ready
                if (mp_findices == null)
                    set_findices();
                mp_substage++;
            }   nobreak;

            // and indices of existing indices
            case 1: {
                // get "count" worth of indices & flag them
                while (mp_progress < mp_findexcount) {
                    int     index;

                    if (mp_facecount < 256) {
                        unsigned char       byte;

                        if ((status = GetData(tk, byte)) != TK_Normal)
                            return status;
                        index = byte;
                    }
                    else if (mp_facecount < 65536) {
                        unsigned short      word;

                        if ((status = GetData(tk, word)) != TK_Normal)
                            return status;
                        index = word;
                    }
                    else {
                        if ((status = GetData(tk, index)) != TK_Normal)
                            return status;
                    }
                    mp_face_exists[index] |= Face_Index;
                    mp_progress++;
                }
                mp_progress = 0;
                mp_substage++;
            }   nobreak;

            //get all the indices OR the individual indices
            case 2: {
                // get indices associated with the flagged indices
                while (mp_progress < mp_facecount) {
                    if (mp_face_exists[mp_progress] & Face_Index) {
                        if ((status = GetData(tk, &mp_findices[mp_progress], 1)) != TK_Normal)
                            return status;
                    }
                    mp_progress++;
                }
                mp_progress = 0;
                mp_substage = 0;
            }   break;

            default:
                return tk.Error("internal error in TK_Polyhedron::read_face_indices(ver<650)");
        }
    }
    return TK_Normal;

} // end function TK_Polyhedron::read_face_indices




TK_Status TK_Polyhedron::read_face_regions (BStreamFileToolkit & tk) alter {
    TK_Status       status = TK_Normal;

	
#ifndef BSTREAM_DISABLE_ASCII
	if( tk.GetAsciiMode() ) 
		return read_face_regions_ascii (tk);
#endif

    switch (mp_substage) {
        case 0: {
            if ((status = GetData(tk, mp_compression_scheme)) != TK_Normal)
                return status;
            SetFaceRegions();   // allocate space
            mp_substage++;
        }   nobreak;

        case 1: {
            // mp_workspace_used gets region count (scheme 0 doesn't need it though)
            if ((mp_compression_scheme & 0x03) != 0) {
                switch (mp_compression_scheme & 0x18) {
                    case 0x00: {
                        if ((status = GetData(tk, mp_workspace_used)) != TK_Normal)
                            return status;
                    }   break;

                    case 0x08: {
                        unsigned char       byte;
                        if ((status = GetData(tk, byte)) != TK_Normal)
                            return status;
                        mp_workspace_used = byte;
                    }   break;

                    case 0x10: {
                        unsigned short      word;
                        if ((status = GetData(tk, word)) != TK_Normal)
                            return status;
                        mp_workspace_used = word;
                    }   break;

                    default: {
                        return tk.Error ("unknown region compression");
                    }   nobreak;
                }
            }
            // mp_workspace_used is remaining amount of data items
            switch (mp_compression_scheme & 0x03) {
                case 0: {
                    mp_workspace_used = mp_facecount;
                }   break;

                case 1: {
                    mp_workspace_used = mp_workspace_used;
                }   break;

                case 2: {
                    mp_workspace_used = 1 + mp_workspace_used;
                }   break;

                case 3: {
                    mp_workspace_used = 2 * mp_workspace_used;
                }   break;

                default: {
                    return tk.Error ("unknown region compression");
                }   nobreak;
            }
            // allocate space enough for ints (even though we may read bytes or shorts, it will get expanded)
            mp_workspace_used *= 4;
            if (mp_workspace_used > mp_workspace_allocated) {
                mp_workspace_allocated = mp_workspace_used;
                ENSURE_ALLOCATED (mp_workspace, unsigned char, mp_workspace_allocated);
            }
            mp_substage++;
        }   nobreak;

        case 2: {
            int count = mp_workspace_used/4;
            int num_ints, i;

            // get appropriate data items and expand to ints if needed
            switch (mp_compression_scheme & 0x18) {
                case 0x00: {
                    if ((status = GetData(tk, (int *)mp_workspace, count)) != TK_Normal)
                        return status;
                }   break;

                case 0x08: {
                    if ((status = GetData(tk, mp_workspace, count)) != TK_Normal)
                        return status;
                    int * tmp = (int *)mp_workspace + count;
                    char * src = (char *)mp_workspace + count;
                    while (count-- > 0)
                        *--tmp = *--src;
                }   break;

                case 0x10: {
                    if ((status = GetData(tk, (short *)mp_workspace, count)) != TK_Normal)
                        return status;
                    int * tmp = (int *)mp_workspace + count;
                    short * src = (short *)mp_workspace + count;
                    while (count-- > 0)
                        *--tmp = *--src;
                }   break;
            }
            // and decompress
            num_ints = mp_workspace_used/4;
            switch (mp_compression_scheme & 0x03) {
                case 0: {
                    memcpy (mp_fregions, mp_workspace, mp_facecount * 4);
                }   break;

                case 1: {
                    int * tmp = mp_fregions;
                    int * src = (int *)mp_workspace;
                    for (i = 0; i < num_ints; i++) {
                        count = *src++;
                        while (count-- > 0)
                            *tmp++ = i;
                    }
                }   break;

                case 2: {
                    int * tmp = mp_fregions;
                    int * src = (int *)mp_workspace;
                    int offset = *src++;
                    for (i = 0; i < num_ints-1; i++) {
                        count = *src++;
                        while (count-- > 0)
                            *tmp++ = i + offset;
                    }
                }   break;

                case 3: {
                    int index;
                    int num_pairs = num_ints / 2;
                    int *tmp = mp_fregions;
                    int *src = (int *)mp_workspace;

                    for (i = 0; i < num_pairs; i++) {
                        index = *src++;
                        count = *src++;
                        while (count-- > 0)
                            *tmp++ = index;
                    }
                }   break;
            }
            mp_substage = 0;
        }   break;

        default:
            return tk.Error ("internal error in TK_Polyhedron::read_face_regions");
    }
    return status;

} // end function TK_Polyhedron::read_face_regions


TK_Status TK_Polyhedron::read_vertex_marker_visibilities (BStreamFileToolkit & tk) alter {
    TK_Status       status = TK_Normal;
    int i;


#ifndef BSTREAM_DISABLE_ASCII
	if( tk.GetAsciiMode() ) 
		return read_vertex_marker_visibilities_ascii (tk);
#endif

    if (mp_optopcode == OPT_ALL_VMVISIBILITIES) {
        switch (mp_substage) {
            //compression scheme
            case 0: {
                if ((status = GetData(tk, mp_compression_scheme)) != TK_Normal)
                    return status;
                mp_substage++;
                SetVertexMarkerVisibilities();
            }   nobreak;

            //all visibilities
            case 1: {
                mp_vmvisibilitycount = mp_pointcount;
                if ((status = GetData(tk, mp_vmvisibilities, mp_vmvisibilitycount)) != TK_Normal)
                    return status;
                for (i = 0; i < mp_pointcount; i++)
                    mp_exists[i] |= Vertex_Marker_Visibility;
                mp_substage = 0;
            } break;

            default:
                return tk.Error("internal error in read_vertex_marker_visibilities (1)");
        }
    }
    else {
        switch (mp_substage) {
            //compression scheme
            case 0: {
                if ((status = GetData(tk, mp_compression_scheme)) != TK_Normal)
                    return status;
                mp_substage++;
            }   nobreak;

            //count
            case 1: {
                if ((status = GetData(tk, mp_vmvisibilitycount)) != TK_Normal)
                    return status;
                mp_substage++;
                mp_progress = 0;
            }   nobreak;

            //and indices of existing values
            case 2: {
                while (mp_progress < mp_vmvisibilitycount) {
                    int     index;

                    if (mp_pointcount < 256) {
                        unsigned char       byte;

                        if ((status = GetData(tk, byte)) != TK_Normal)
                            return status;
                        index = byte;
                    }
                    else if (mp_pointcount < 65536) {
                        unsigned short      word;

                        if ((status = GetData(tk, word)) != TK_Normal)
                            return status;
                        index = word;
                    }
                    else {
                        if ((status = GetData(tk, index)) != TK_Normal)
                            return status;
                    }
                    if (index > mp_pointcount)
                        return tk.Error("invalid vertex index during read vertex marker visibility");
                    mp_exists[index] |= Vertex_Marker_Visibility;
                    mp_progress++;
                }
                mp_progress = 0;
                SetVertexMarkerVisibilities();
                mp_substage++;
            } nobreak;

            //individual visibility settings
            case 3: {
                while (mp_progress < mp_pointcount) {
                    if (mp_exists[mp_progress] & Vertex_Marker_Visibility) {
                        if ((status = GetData(tk, mp_vmvisibilities[mp_progress])) != TK_Normal)
                            return status;
                    }
                    mp_progress++;
                }
                mp_progress = 0;
                mp_substage = 0;
            } break;

            default:
                return tk.Error("internal error in read_vertex_marker_visibilities (2)");
        }
    }
    return status;
} //end function TK_Polyhedron::read_vertex_marker_visibilities


TK_Status TK_Polyhedron::read_vertex_marker_symbols (BStreamFileToolkit & tk) alter {
    TK_Status       status = TK_Normal;
    int i;


#ifndef BSTREAM_DISABLE_ASCII
	if( tk.GetAsciiMode() ) 
		return read_vertex_marker_symbols_ascii (tk);
#endif

    if (mp_optopcode == OPT_ALL_VMSYMBOLS) {
        switch (mp_substage) {
            //compression scheme
            case 0: {
                if ((status = GetData(tk, mp_compression_scheme)) != TK_Normal)
                    return status;
                SetVertexMarkerSymbols();
                mp_substage++;
            }   nobreak;

            //all symbols
            case 1: {
                mp_vmsymbolcount = mp_pointcount;
                if ((status = GetData(tk, mp_vmsymbols, mp_vmsymbolcount)) != TK_Normal)
                    return status;
                for (i = 0; i < mp_pointcount; i++)
                    mp_exists[i] |= Vertex_Marker_Symbol;
                mp_substage = 0;
            } break;

            default:
                return tk.Error("internal error in read_vertex_marker_symbols (1)");
        }
    }
    else {
        switch (mp_substage) {
            //compression scheme
            case 0: {
                if ((status = GetData(tk, mp_compression_scheme)) != TK_Normal)
                    return status;
                mp_substage++;
            }   nobreak;

            //count
            case 1: {
                if ((status = GetData(tk, mp_vmsymbolcount)) != TK_Normal)
                    return status;
                mp_substage++;
                mp_progress = 0;
            }   nobreak;

            //and indices of existing values
            case 2: {
                while (mp_progress < mp_vmsymbolcount) {
                    int     index;

                    if (mp_pointcount < 256) {
                        unsigned char       byte;

                        if ((status = GetData(tk, byte)) != TK_Normal)
                            return status;
                        index = byte;
                    }
                    else if (mp_pointcount < 65536) {
                        unsigned short      word;

                        if ((status = GetData(tk, word)) != TK_Normal)
                            return status;
                        index = word;
                    }
                    else {
                        if ((status = GetData(tk, index)) != TK_Normal)
                            return status;
                    }
                    if (index > mp_pointcount)
                        return tk.Error("invalid vertex index during read vertex marker symbol");
                    mp_exists[index] |= Vertex_Marker_Symbol;
                    mp_progress++;
                }
                mp_progress = 0;
                mp_subprogress = 0;
                SetVertexMarkerSymbols();
                mp_substage++;
            } nobreak;

            //individual symbols
            case 3: {
                while (mp_progress < mp_pointcount) {
                    if (mp_exists[mp_progress] & Vertex_Marker_Symbol) {
                        switch (mp_subprogress) {
                            case 0: {
                                if ((status = GetData(tk, mp_vmsymbols[mp_progress])) != TK_Normal)
                                    return status;
                                if (mp_vmsymbols[mp_progress] != 0xFF)
                                    break;
                                SetVertexMarkerSymbolStrings();
                                mp_subprogress++;
                            }   nobreak;
                            case 1: {
                                unsigned char   byte;
                                int             length;

                                if ((status = GetData(tk, byte)) != TK_Normal)
                                    return status;
                                length = (int)byte;
                                mp_vmsymbolstrings[mp_progress] = new char[length+1];
                                mp_vmsymbolstrings[mp_progress][length] = '\0';
                                mp_vmsymbolstrings[mp_progress][0] = (char)length;
                                mp_subprogress++;
                            }   nobreak;
                            case 2: {
                                int             length = (int)mp_vmsymbolstrings[mp_progress][0];

                                if ((status = GetData(tk, mp_vmsymbolstrings[mp_progress], length)) != TK_Normal)
                                    return status;
                                mp_subprogress = 0;
                            }   break;
                        }
                    }
                    mp_progress++;
                }
                mp_progress = 0;
                mp_substage = 0;
            } break;

            default:
                return tk.Error("internal error in read_vertex_marker_symbols (2)");
        }
    }
    return status;
} //end function TK_Polyhedron::read_vertex_marker_symbols


TK_Status TK_Polyhedron::read_vertex_marker_sizes (BStreamFileToolkit & tk) alter {
    TK_Status       status = TK_Normal;
    int i;

#ifndef BSTREAM_DISABLE_ASCII
	if( tk.GetAsciiMode() ) 
		return read_vertex_marker_sizes_ascii (tk);
#endif

    if (mp_optopcode == OPT_ALL_VMSIZES) {
        switch (mp_substage) {
            //compression scheme
            case 0: {
                if ((status = GetData(tk, mp_compression_scheme)) != TK_Normal)
                    return status;
                SetVertexMarkerSizes();
                mp_substage++;
            }   nobreak;

            //all sizes
            case 1: {
                mp_vmsizecount = mp_pointcount;
                if ((status = GetData(tk, mp_vmsizes, mp_vmsizecount)) != TK_Normal)
                    return status;
                for (i = 0; i < mp_pointcount; i++)
                    mp_exists[i] |= Vertex_Marker_Size;
                mp_substage = 0;
            } break;

            default:
                return tk.Error("internal error in read_vertex_marker_sizes (1)");
        }
    }
    else {
        switch (mp_substage) {
            //compression scheme
            case 0: {
                if ((status = GetData(tk, mp_compression_scheme)) != TK_Normal)
                    return status;
                mp_substage++;
            }   nobreak;

            //count
            case 1: {
                if ((status = GetData(tk, mp_vmsizecount)) != TK_Normal)
                    return status;
                mp_substage++;
                mp_progress = 0;
            }   nobreak;

            //and indices of existing values
            case 2: {
                while (mp_progress < mp_vmsizecount) {
                    int     index;

                    if (mp_pointcount < 256) {
                        unsigned char       byte;

                        if ((status = GetData(tk, byte)) != TK_Normal)
                            return status;
                        index = byte;
                    }
                    else if (mp_pointcount < 65536) {
                        unsigned short      word;

                        if ((status = GetData(tk, word)) != TK_Normal)
                            return status;
                        index = word;
                    }
                    else {
                        if ((status = GetData(tk, index)) != TK_Normal)
                            return status;
                    }
                    if (index > mp_pointcount)
                        return tk.Error("invalid vertex index during read vertex marker sizes");
                    mp_exists[index] |= Vertex_Marker_Size;
                    mp_progress++;
                }
                mp_progress = 0;
                SetVertexMarkerSizes();
                mp_substage++;
            } nobreak;

            //individual sizes
            case 3: {
                while (mp_progress < mp_pointcount) {
                    if (mp_exists[mp_progress] & Vertex_Marker_Size) {
                        if ((status = GetData(tk, mp_vmsizes[mp_progress])) != TK_Normal)
                            return status;
                    }
                    mp_progress++;
                }
                mp_progress = 0;
                mp_substage = 0;
            } break;

            default:
                return tk.Error("internal error in read_vertex_marker_sizes (2)");
        }
    }
    return status;
} //end function TK_Polyhedron::read_vertex_marker_sizes


TK_Status TK_Polyhedron::read_face_visibilities (BStreamFileToolkit & tk) alter {
    TK_Status       status = TK_Normal;
    int i;


#ifndef BSTREAM_DISABLE_ASCII
	if( tk.GetAsciiMode() ) 
		return read_face_visibilities_ascii (tk);
#endif


    if (mp_optopcode == OPT_ALL_FACE_VISIBILITIES) {
        switch (mp_substage) {
            //compression scheme
            case 0: {
                if ((status = GetData(tk, mp_compression_scheme)) != TK_Normal)
                    return status;
                SetFaceVisibilities();
                mp_substage++;
            }   nobreak;

            //all visibilities
            case 1: {
                mp_fvisibilitycount = mp_facecount;
                if ((status = GetData(tk, mp_fvisibilities, mp_fvisibilitycount)) != TK_Normal)
                    return status;
                for (i = 0; i < mp_facecount; i++)
                    mp_face_exists[i] |= Face_Visibility;
                mp_substage = 0;
            } break;

            default:
                return tk.Error("internal error in read_face_visibilities (1)");
        }
    }
    else {
        switch (mp_substage) {
            //compression scheme
            case 0: {
                if ((status = GetData(tk, mp_compression_scheme)) != TK_Normal)
                    return status;
                mp_substage++;
            }   nobreak;

            //count
            case 1: {
                if ((status = GetData(tk, mp_fvisibilitycount)) != TK_Normal)
                    return status;
                mp_substage++;
                mp_progress = 0;
            }   nobreak;

            //and indices of existing values
            case 2: {
                while (mp_progress < mp_fvisibilitycount) {
                    int     index;

                    if (mp_facecount < 256) {
                        unsigned char       byte;

                        if ((status = GetData(tk, byte)) != TK_Normal)
                            return status;
                        index = byte;
                    }
                    else if (mp_facecount < 65536) {
                        unsigned short      word;

                        if ((status = GetData(tk, word)) != TK_Normal)
                            return status;
                        index = word;
                    }
                    else {
                        if ((status = GetData(tk, index)) != TK_Normal)
                            return status;
                    }
                    if (index > mp_facecount)
                        return tk.Error("invalid face index during read face visibility");
                    mp_face_exists[index] |= Face_Visibility;
                    mp_progress++;
                }
                mp_progress = 0;
                SetFaceVisibilities();
                mp_substage++;
            } nobreak;

            //individual indices
            case 3: {
                while (mp_progress < mp_facecount) {
                    if (mp_face_exists[mp_progress] & Face_Visibility) {
                        if ((status = GetData(tk, mp_fvisibilities[mp_progress])) != TK_Normal)
                            return status;
                    }
                    mp_progress++;
                }
                mp_progress = 0;
                mp_substage = 0;
            } break;

            default:
                return tk.Error("internal error in read_face_visibilities (2)");
        }
    }
    return status;
} //end function TK_Polyhedron::read_face_visibilities


TK_Status TK_Polyhedron::read_face_patterns (BStreamFileToolkit & tk) alter {
    TK_Status       status = TK_Normal;
    int i;


#ifndef BSTREAM_DISABLE_ASCII
	if( tk.GetAsciiMode() ) 
		return read_face_patterns_ascii (tk);
#endif


    if (mp_optopcode == OPT_ALL_FACE_PATTERNS) {
        switch (mp_substage) {
            //compression scheme
            case 0: {
                if ((status = GetData(tk, mp_compression_scheme)) != TK_Normal)
                    return status;
                SetFacePatterns();
                mp_substage++;
            }   nobreak;

            //all patterns
            case 1: {
                mp_fpatterncount = mp_facecount;
                if ((status = GetData(tk, mp_fpatterns, mp_fpatterncount)) != TK_Normal)
                    return status;
                for (i = 0; i < mp_facecount; i++)
                    mp_face_exists[i] |= Face_Pattern;
                mp_substage = 0;
            } break;

            default:
                return tk.Error("internal error in read_face_patterns (1)");
        }
    }
    else {
        switch (mp_substage) {
            //compression scheme
            case 0: {
                if ((status = GetData(tk, mp_compression_scheme)) != TK_Normal)
                    return status;
                mp_substage++;
            }   nobreak;

            //count
            case 1: {
                if ((status = GetData(tk, mp_fpatterncount)) != TK_Normal)
                    return status;
                mp_substage++;
                mp_progress = 0;
            }   nobreak;

            //and indices of existing values
            case 2: {
                while (mp_progress < mp_fpatterncount) {
                    int     index;

                    if (mp_facecount < 256) {
                        unsigned char       byte;

                        if ((status = GetData(tk, byte)) != TK_Normal)
                            return status;
                        index = byte;
                    }
                    else if (mp_facecount < 65536) {
                        unsigned short      word;

                        if ((status = GetData(tk, word)) != TK_Normal)
                            return status;
                        index = word;
                    }
                    else {
                        if ((status = GetData(tk, index)) != TK_Normal)
                            return status;
                    }
                    if (index > mp_facecount)
                        return tk.Error("invalid face index during read face pattern");
                    mp_face_exists[index] |= Face_Pattern;
                    mp_progress++;
                }
                mp_progress = 0;
                SetFacePatterns();
                mp_substage++;
            } nobreak;

            //individual indices
            case 3: {
                while (mp_progress < mp_facecount) {
                    if (mp_face_exists[mp_progress] & Face_Pattern) {
                        if ((status = GetData(tk, mp_fpatterns[mp_progress])) != TK_Normal)
                            return status;
                    }
                    mp_progress++;
                }
                mp_progress = 0;
                mp_substage = 0;
            } break;

            default:
                return tk.Error("internal error in read_face_patterns (2)");
        }
    }
    return status;
} //end function TK_Polyhedron::read_face_patterns


TK_Status TK_Polyhedron::read_face_normals (BStreamFileToolkit & tk) alter {
    TK_Status       status = TK_Normal;
    int i;


#ifndef BSTREAM_DISABLE_ASCII
	if( tk.GetAsciiMode() ) 
		return read_face_normals_ascii (tk);
#endif

    if (mp_optopcode == OPT_ALL_FACE_NORMALS_POLAR) {
        switch (mp_substage) {
            //compression scheme
            case 0: {
                if ((status = GetData(tk, mp_compression_scheme)) != TK_Normal)
                    return status;
                SetFaceNormals();
                mp_substage++;
            }   nobreak;

            //all normals
            case 1: {
                mp_fnormalcount = mp_facecount;
                if ((status = GetData(tk, mp_fnormals, 2 * mp_fnormalcount)) != TK_Normal)
                    return status;
                normals_polar_to_cartesian(null, Face_Normal, mp_facecount, mp_fnormals, mp_fnormals);
                for (i = 0; i < mp_facecount; i++)
                    mp_face_exists[i] |= Face_Normal;
                mp_substage = 0;
            } break;

            default:
                return tk.Error("internal error in read_face_normals (1)");
        }
    }
    else {
        switch (mp_substage) {
            //compression scheme
            case 0: {
                if ((status = GetData(tk, mp_compression_scheme)) != TK_Normal)
                    return status;
                mp_substage++;
            }   nobreak;

            //count
            case 1: {
                if ((status = GetData(tk, mp_fnormalcount)) != TK_Normal)
                    return status;
                mp_substage++;
                mp_progress = 0;
            }   nobreak;

            //and indices of existing values
            case 2: {
                while (mp_progress < mp_fnormalcount) {
                    int     index;

                    if (mp_facecount < 256) {
                        unsigned char       byte;

                        if ((status = GetData(tk, byte)) != TK_Normal)
                            return status;
                        index = byte;
                    }
                    else if (mp_facecount < 65536) {
                        unsigned short      word;

                        if ((status = GetData(tk, word)) != TK_Normal)
                            return status;
                        index = word;
                    }
                    else {
                        if ((status = GetData(tk, index)) != TK_Normal)
                            return status;
                    }
                    if (index > mp_facecount)
                        return tk.Error("invalid face index during read face normals");
                    mp_face_exists[index] |= Face_Normal;
                    mp_progress++;
                }
                mp_progress = 0;
                SetFaceNormals();
                mp_substage++;
            } nobreak;

            //individual indices
            case 3: {
                while (mp_progress < mp_facecount) {
                    if (mp_face_exists[mp_progress] & Face_Normal) {
                        if ((status = GetData(tk, &mp_fnormals[2*mp_progress], 2)) != TK_Normal)
                            return status;
                    }
                    mp_progress++;
                }
                normals_polar_to_cartesian(mp_face_exists, Face_Normal, mp_facecount, mp_fnormals, mp_fnormals);
                mp_progress = 0;
                mp_substage = 0;
            } break;

            default:
                return tk.Error("internal error in read_face_normals (2)");
        }
    }
    return status;
} //end function TK_Polyhedron::read_face_normals


TK_Status TK_Polyhedron::read_edge_visibilities (BStreamFileToolkit & tk) alter {
    TK_Status       status = TK_Normal;
    int i;


#ifndef BSTREAM_DISABLE_ASCII
	if( tk.GetAsciiMode() ) 
		return read_edge_visibilities_ascii (tk);
#endif

    if (mp_optopcode == OPT_ALL_EDGE_VISIBILITIES) {
        switch (mp_substage) {
            //compression scheme
            case 0: {
                if ((status = GetData(tk, mp_compression_scheme)) != TK_Normal)
                    return status;
                SetEdgeVisibilities();
                mp_substage++;
            }   nobreak;

            //all visibilities
            case 1: {
                mp_evisibilitycount = mp_edgecount;
                if ((status = GetData(tk, mp_evisibilities, mp_evisibilitycount)) != TK_Normal)
                    return status;
                for (i = 0; i < mp_edgecount; i++)
                    mp_edge_exists[i] |= Edge_Visibility;
                mp_substage = 0;
            } break;

            default:
                return tk.Error("internal error in read_edge_visibilities (1)");
        }
    }
    else {
        switch (mp_substage) {
            //compression scheme
            case 0: {
                if ((status = GetData(tk, mp_compression_scheme)) != TK_Normal)
                    return status;
                mp_substage++;
            }   nobreak;

            //count
            case 1: {
                if ((status = GetData(tk, mp_evisibilitycount)) != TK_Normal)
                    return status;
                mp_substage++;
                mp_progress = 0;
            }   nobreak;

            //and indices of existing values
            case 2: {
                while (mp_progress < mp_evisibilitycount) {
                    int     index;

                    if (mp_edgecount < 256) {
                        unsigned char       byte;

                        if ((status = GetData(tk, byte)) != TK_Normal)
                            return status;
                        index = byte;
                    }
                    else if (mp_edgecount < 65536) {
                        unsigned short      word;

                        if ((status = GetData(tk, word)) != TK_Normal)
                            return status;
                        index = word;
                    }
                    else {
                        if ((status = GetData(tk, index)) != TK_Normal)
                            return status;
                    }
                    if (index > mp_edgecount)
                        return tk.Error("invalid edge index during read edge visibility");
                    mp_edge_exists[index] |= Edge_Visibility;
                    mp_progress++;
                }
                mp_progress = 0;
                SetEdgeVisibilities();
                mp_substage++;
            } nobreak;

            //individual indices
            case 3: {
                while (mp_progress < mp_edgecount) {
                    if (mp_edge_exists[mp_progress] & Edge_Visibility) {
                        if ((status = GetData(tk, mp_evisibilities[mp_progress])) != TK_Normal)
                            return status;
                    }
                    mp_progress++;
                }
                mp_progress = 0;
                mp_substage = 0;
            } break;

            default:
                return tk.Error("internal error in read_edge_visibilities (2)");
        }
    }
    return status;
} //end function TK_Polyhedron::read_edge_visibilities


TK_Status TK_Polyhedron::read_edge_patterns (BStreamFileToolkit & tk) alter {
    TK_Status       status = TK_Normal;
    int i;


#ifndef BSTREAM_DISABLE_ASCII
	if( tk.GetAsciiMode() ) 
		return read_edge_patterns_ascii (tk);
#endif

    if (mp_optopcode == OPT_ALL_EDGE_PATTERNS) {
        switch (mp_substage) {
            //compression scheme
            case 0: {
                if ((status = GetData(tk, mp_compression_scheme)) != TK_Normal)
                    return status;
                SetEdgePatterns();
                mp_substage++;
            }   nobreak;

            //all patterns
            case 1: {
                mp_epatterncount = mp_edgecount;
                if ((status = GetData(tk, mp_epatterns, mp_epatterncount)) != TK_Normal)
                    return status;
                for (i = 0; i < mp_edgecount; i++)
                    mp_edge_exists[i] |= Edge_Pattern;
                mp_substage = 0;
            } break;

            default:
                return tk.Error("internal error in read_edge_patterns (1)");
        }
    }
    else {
        switch (mp_substage) {
            //compression scheme
            case 0: {
                if ((status = GetData(tk, mp_compression_scheme)) != TK_Normal)
                    return status;
                mp_substage++;
            }   nobreak;

            //count
            case 1: {
                if ((status = GetData(tk, mp_epatterncount)) != TK_Normal)
                    return status;
                mp_substage++;
                mp_progress = 0;
            }   nobreak;

            //and indices of existing values
            case 2: {
                while (mp_progress < mp_epatterncount) {
                    int     index;

                    if (mp_edgecount < 256) {
                        unsigned char       byte;

                        if ((status = GetData(tk, byte)) != TK_Normal)
                            return status;
                        index = byte;
                    }
                    else if (mp_edgecount < 65536) {
                        unsigned short      word;

                        if ((status = GetData(tk, word)) != TK_Normal)
                            return status;
                        index = word;
                    }
                    else {
                        if ((status = GetData(tk, index)) != TK_Normal)
                            return status;
                    }
                    if (index > mp_edgecount)
                        return tk.Error("invalid edge index during read edge patterns");
                    mp_edge_exists[index] |= Edge_Pattern;
                    mp_progress++;
                }
                mp_progress = 0;
                SetEdgePatterns();
                mp_substage++;
            } nobreak;

            //individual indices
            case 3: {
                while (mp_progress < mp_edgecount) {
                    if (mp_edge_exists[mp_progress] & Edge_Pattern) {
                        switch (mp_subprogress) {
                            case 0: {
                                if ((status = GetData(tk, mp_epatterns[mp_progress])) != TK_Normal)
                                    return status;
                                if (mp_epatterns[mp_progress] != 0xFF)
                                    break;
                                SetEdgePatternStrings();
                                mp_subprogress++;
                            }   nobreak;
                            case 1: {
                                unsigned char   byte;
                                int             length;

                                if ((status = GetData(tk, byte)) != TK_Normal)
                                    return status;
                                length = (int)byte;
                                mp_epatternstrings[mp_progress] = new char[length+1];
                                mp_epatternstrings[mp_progress][length] = '\0';
                                mp_epatternstrings[mp_progress][0] = (char)length;
                                mp_subprogress++;
                            }   nobreak;
                            case 2: {
                                int             length = (int)mp_epatternstrings[mp_progress][0];

                                if ((status = GetData(tk, mp_epatternstrings[mp_progress], length)) != TK_Normal)
                                    return status;
                                mp_subprogress = 0;
                            }   break;
                        }
                    }
                    mp_progress++;
                }
                mp_progress = 0;
                mp_substage = 0;
            } break;

            default:
                return tk.Error("internal error in read_edge_patterns (2)");
        }
    }
    return status;
} //end function TK_Polyhedron::read_edge_patterns


TK_Status TK_Polyhedron::read_edge_colors (BStreamFileToolkit & tk) alter {
    TK_Status       status = TK_Normal;
    int i;


#ifndef BSTREAM_DISABLE_ASCII
	if( tk.GetAsciiMode() ) 
		return read_edge_colors_ascii (tk);
#endif

    if (mp_optopcode == OPT_ALL_EDGE_COLORS) {
        switch (mp_substage) {
            //compression scheme
            case 0: {
                if ((status = GetData(tk, mp_compression_scheme)) != TK_Normal)
                    return status;
                SetEdgeColors();
                mp_substage++;
            }   nobreak;

            //all colors
            case 1: {
                mp_ecolorcount = mp_edgecount;
                if ((status = GetData(tk, mp_ecolors, 3*mp_ecolorcount)) != TK_Normal)
                    return status;
                for (i = 0; i < mp_edgecount; i++)
                    mp_edge_exists[i] |= Edge_Color;
                mp_substage = 0;
            } break;

            default:
                return tk.Error("internal error in read_edge_colors (1)");
        }
    }
    else {
        switch (mp_substage) {
            //compression scheme
            case 0: {
                if ((status = GetData(tk, mp_compression_scheme)) != TK_Normal)
                    return status;
                mp_substage++;
            }   nobreak;

            //count
            case 1: {
                if ((status = GetData(tk, mp_ecolorcount)) != TK_Normal)
                    return status;
                mp_substage++;
                mp_progress = 0;
            }   nobreak;

            //and indices of existing values
            case 2: {
                while (mp_progress < mp_ecolorcount) {
                    int     index;

                    if (mp_edgecount < 256) {
                        unsigned char       byte;

                        if ((status = GetData(tk, byte)) != TK_Normal)
                            return status;
                        index = byte;
                    }
                    else if (mp_edgecount < 65536) {
                        unsigned short      word;

                        if ((status = GetData(tk, word)) != TK_Normal)
                            return status;
                        index = word;
                    }
                    else {
                        if ((status = GetData(tk, index)) != TK_Normal)
                            return status;
                    }
                    if (index > mp_edgecount)
                        return tk.Error("invalid edge index during read edge colors");
                    mp_edge_exists[index] |= Edge_Color;
                    mp_progress++;
                }
                mp_progress = 0;
                SetEdgeColors();
                mp_substage++;
            } nobreak;

            //individual indices
            case 3: {
                while (mp_progress < mp_edgecount) {
                    if (mp_edge_exists[mp_progress] & Edge_Color) {
                        if ((status = GetData(tk, &mp_ecolors[3*mp_progress], 3)) != TK_Normal)
                            return status;
                    }
                    mp_progress++;
                }
                mp_progress = 0;
                mp_substage = 0;
            } break;

            default:
                return tk.Error("internal error in read_edge_colors (2)");
        }
    }
    return status;
} //end function TK_Polyhedron::read_edge_colors


TK_Status TK_Polyhedron::read_edge_indices (BStreamFileToolkit & tk) alter {
    TK_Status       status = TK_Normal;
    int i;

#ifndef BSTREAM_DISABLE_ASCII
	if( tk.GetAsciiMode() ) 
		return read_edge_indices_ascii (tk);
#endif

    if (mp_optopcode == OPT_ALL_EDGE_INDICES) {
        switch (mp_substage) {
            //compression scheme
            case 0: {
                if ((status = GetData(tk, mp_compression_scheme)) != TK_Normal)
                    return status;
                SetEdgeIndices();
                mp_substage++;
            }   nobreak;

            //all indices
            case 1: {
                mp_eindexcount = mp_edgecount;
                if ((status = GetData(tk, mp_eindices, mp_eindexcount)) != TK_Normal)
                    return status;
                for (i = 0; i < mp_edgecount; i++)
                    mp_edge_exists[i] |= Edge_Index;
                mp_substage = 0;
            } break;

            default:
                return tk.Error("internal error in read_edge_indices (1)");
        }
    }
    else {
        switch (mp_substage) {
            //compression scheme
            case 0: {
                if ((status = GetData(tk, mp_compression_scheme)) != TK_Normal)
                    return status;
                mp_substage++;
            }   nobreak;

            //count
            case 1: {
                if ((status = GetData(tk, mp_eindexcount)) != TK_Normal)
                    return status;
                mp_substage++;
                mp_progress = 0;
            }   nobreak;

            //and indices of existing values
            case 2: {
                while (mp_progress < mp_eindexcount) {
                    int     index;

                    if (mp_edgecount < 256) {
                        unsigned char       byte;

                        if ((status = GetData(tk, byte)) != TK_Normal)
                            return status;
                        index = byte;
                    }
                    else if (mp_edgecount < 65536) {
                        unsigned short      word;

                        if ((status = GetData(tk, word)) != TK_Normal)
                            return status;
                        index = word;
                    }
                    else {
                        if ((status = GetData(tk, index)) != TK_Normal)
                            return status;
                    }
                    if (index > mp_edgecount)
                        return tk.Error("invalid edge index during read edge indices");
                    mp_edge_exists[index] |= Edge_Index;
                    mp_progress++;
                }
                mp_progress = 0;
                SetEdgeIndices();
                mp_substage++;
            } nobreak;

            //individual indices
            case 3: {
                while (mp_progress < mp_edgecount) {
                    if (mp_edge_exists[mp_progress] & Edge_Index) {
                        if ((status = GetData(tk, mp_eindices[mp_progress])) != TK_Normal)
                            return status;
                    }
                    mp_progress++;
                }
                mp_progress = 0;
                mp_substage = 0;
            } break;

            default:
                return tk.Error("internal error in read_edge_indices (2)");
        }
    }
    return status;
} //end function TK_Polyhedron::read_edge_indices


TK_Status TK_Polyhedron::read_edge_weights (BStreamFileToolkit & tk) alter {
    TK_Status       status = TK_Normal;
    int             i;

	
#ifndef BSTREAM_DISABLE_ASCII
	if( tk.GetAsciiMode() ) 
		return read_edge_weights_ascii (tk);
#endif

    if (mp_optopcode == OPT_ALL_EDGE_WEIGHTS) {
        switch (mp_substage) {
            //compression scheme
            case 0: {
                if ((status = GetData(tk, mp_compression_scheme)) != TK_Normal)
                    return status;
                SetEdgeWeights();
                mp_substage++;
            }   nobreak;

            //all weights
            case 1: {
                mp_eweightcount = mp_edgecount;
                if ((status = GetData(tk, mp_eweights, mp_edgecount)) != TK_Normal)
                    return status;
                for (i = 0; i < mp_edgecount; i++)
                    mp_edge_exists[i] |= Edge_Weight;
                mp_substage = 0;
            } break;

            default:
                return tk.Error("internal error in read_edge_weights (1)");
        }
    }
    else {
        switch (mp_substage) {
            //compression scheme
            case 0: {
                if ((status = GetData(tk, mp_compression_scheme)) != TK_Normal)
                    return status;
                mp_substage++;
            }   nobreak;

            //count
            case 1: {
                if ((status = GetData(tk, mp_eweightcount)) != TK_Normal)
                    return status;
                mp_substage++;
                mp_progress = 0;
            }   nobreak;

            //and indices of existing values
            case 2: {
                while (mp_progress < mp_eweightcount) {
                    int     index;

                    if (mp_edgecount < 256) {
                        unsigned char       byte;

                        if ((status = GetData(tk, byte)) != TK_Normal)
                            return status;
                        index = byte;
                    }
                    else if (mp_edgecount < 65536) {
                        unsigned short      word;

                        if ((status = GetData(tk, word)) != TK_Normal)
                            return status;
                        index = word;
                    }
                    else {
                        if ((status = GetData(tk, index)) != TK_Normal)
                            return status;
                    }
                    if (index > mp_edgecount)
                        return tk.Error("invalid edge index during read edge weights");
                    mp_edge_exists[index] |= Edge_Weight;
                    mp_progress++;
                }
                mp_progress = 0;
                SetEdgeWeights();
                mp_substage++;
            } nobreak;

            //individual indices
            case 3: {
                while (mp_progress < mp_edgecount) {
                    if (mp_edge_exists[mp_progress] & Edge_Weight) {
                        if ((status = GetData(tk, mp_eweights[mp_progress])) != TK_Normal)
                            return status;
                    }
                    mp_progress++;
                }
                mp_progress = 0;
                mp_substage = 0;
            } break;

            default:
                return tk.Error("internal error in read_edge_weights (2)");
        }
    }
    return status;
} //end function TK_Polyhedron::read_edge_weights


TK_Status TK_Polyhedron::read_edge_normals (BStreamFileToolkit & tk) alter {
    TK_Status       status = TK_Normal;
    int i;

	
#ifndef BSTREAM_DISABLE_ASCII
	if( tk.GetAsciiMode() ) 
		return read_edge_normals_ascii (tk);
#endif

    if (mp_optopcode == OPT_ALL_EDGE_NORMALS_POLAR) {
        switch (mp_substage) {
            //compression scheme
            case 0: {
                if ((status = GetData(tk, mp_compression_scheme)) != TK_Normal)
                    return status;
                SetEdgeNormals();
                mp_substage++;
            }   nobreak;

            //all normals
            case 1: {
                mp_enormalcount = mp_edgecount;
                if ((status = GetData(tk, mp_enormals, 2*mp_enormalcount)) != TK_Normal)
                    return status;
                normals_polar_to_cartesian(null, Edge_Normal, mp_edgecount, mp_enormals, mp_enormals);
                for (i = 0; i < mp_edgecount; i++)
                    mp_edge_exists[i] |= Edge_Weight;
                mp_substage = 0;
            } break;

            default:
                return tk.Error("internal error in read_edge_normals (1)");
        }
    }
    else {
        switch (mp_substage) {
            //compression scheme
            case 0: {
                if ((status = GetData(tk, mp_compression_scheme)) != TK_Normal)
                    return status;
                mp_substage++;
            }   nobreak;

            //count
            case 1: {
                if ((status = GetData(tk, mp_enormalcount)) != TK_Normal)
                    return status;
                mp_substage++;
                mp_progress = 0;
            }   nobreak;

            //and indices of existing values
            case 2: {
                while (mp_progress < mp_enormalcount) {
                    int     index;

                    if (mp_edgecount < 256) {
                        unsigned char       byte;

                        if ((status = GetData(tk, byte)) != TK_Normal)
                            return status;
                        index = byte;
                    }
                    else if (mp_edgecount < 65536) {
                        unsigned short      word;

                        if ((status = GetData(tk, word)) != TK_Normal)
                            return status;
                        index = word;
                    }
                    else {
                        if ((status = GetData(tk, index)) != TK_Normal)
                            return status;
                    }
                    if (index > mp_edgecount)
                        return tk.Error("invalid edge index during read edge normals");
                    mp_edge_exists[index] |= Edge_Normal;
                    mp_progress++;
                }
                mp_progress = 0;
                SetEdgeNormals();
                mp_substage++;
            } nobreak;

            //individual indices
            case 3: {
                while (mp_progress < mp_edgecount) {
                    if (mp_edge_exists[mp_progress] & Edge_Normal) {
                        if ((status = GetData(tk, mp_enormals[mp_progress])) != TK_Normal)
                            return status;
                    }
                    mp_progress++;
                }
                normals_polar_to_cartesian(mp_edge_exists, Edge_Normal, mp_edgecount, mp_enormals, mp_enormals);
                mp_progress = 0;
                mp_substage = 0;
            } break;

            default:
                return tk.Error("internal error in read_edge_normals (2)");
        }
    }
    return status;
} //end function TK_Polyhedron::read_edge_normals


TK_Status TK_Polyhedron::read_collection (BStreamFileToolkit & tk) alter {
    TK_Status       status = TK_Normal;
    unsigned char   opcode = '\0';
    int i;

	
#ifndef BSTREAM_DISABLE_ASCII
	if( tk.GetAsciiMode() ) 
		return read_collection_ascii (tk);
#endif


    if (mp_num_collection_parts == 0) {
        mp_num_collection_parts = 10;
        mp_collection_parts = new BBaseOpcodeHandler *[mp_num_collection_parts];
        for (i = 0; i < 10; i++)
            mp_collection_parts[i] = null;
    }

    forever {
        if (mp_substage == mp_num_collection_parts) {
            BBaseOpcodeHandler **temp;
            int used = mp_num_collection_parts;
            mp_num_collection_parts *= 2;
            temp = new BBaseOpcodeHandler *[mp_num_collection_parts];
            for (i = 0; i < used; i++)
                temp[i] = mp_collection_parts[i];
            for (i = used; i < mp_num_collection_parts; i++)
                temp[i] = null;
            delete [] mp_collection_parts;
            mp_collection_parts = temp;
        }
        if (mp_collection_parts[mp_substage] == null) {
            if ((status = GetData (tk, opcode)) != TK_Normal)
                return status;
            if (opcode == TKE_Termination) {
                mp_num_collection_parts = mp_substage;
                mp_substage = 0;
                return TK_Normal;
            }
            status = tk.GetOpcodeHandler(opcode)->Clone(tk,&mp_collection_parts[mp_substage]);
            if (status != TK_Normal) {
                // We can't use the current handler recursively
                if (opcode == Opcode())
                    return tk.Error();
                // the registered opcode must be able to clone itself.
                // If it can't, just use the registered handler to consume the data.
                tk.GetOpcodeHandler(opcode)->Read (tk);
                tk.GetOpcodeHandler(opcode)->Reset();
                //pretend we never saw anything
                mp_collection_parts[mp_substage] = null;
                mp_substage--; 
            }
        }
        if ((status = mp_collection_parts[mp_substage]->Read(tk)) != TK_Normal)
            return status;
        mp_substage++;
    }
} // end function TK_Polyhedron::read_collection


//protected virtual
TK_Status TK_Polyhedron::Read (BStreamFileToolkit & tk) alter {
    TK_Status       status = TK_Normal;

	
#ifndef BSTREAM_DISABLE_ASCII
	if( tk.GetAsciiMode() ) 
		return ReadAscii (tk);
#endif

    if (mp_exists == null)
        SetExists();

    for (;;) {
        if (mp_stage == 0) {
            if ((status = GetData(tk, mp_optopcode)) != TK_Normal)
                return status;
            mp_stage++;
        } 

        if (mp_optopcode == OPT_TERMINATE)
            break;

        switch (mp_optopcode) {
            case OPT_NORMALS_POLAR:
            case OPT_NORMALS: {
                if ((status = read_vertex_normals(tk)) != TK_Normal)
                    return status;
                mp_stage = 0;       
            } break;

            case OPT_ALL_NORMALS_POLAR:
            case OPT_ALL_NORMALS: {
                if ((status = read_vertex_normals_all(tk)) != TK_Normal)
                    return status;
                mp_stage = 0;       
            } break;

            case OPT_NORMALS_COMPRESSED: {
                if ((status = read_vertex_normals_compressed(tk)) != TK_Normal)
                    return status;
                mp_stage = 0;       
            } break;

            case OPT_ALL_NORMALS_COMPRESSED: {
                if ((status = read_vertex_normals_compressed_all(tk)) != TK_Normal)
                    return status;
                mp_stage = 0;       
            } break;

            case OPT_PARAMETERS:
            case OPT_PARAMETERS_COMPRESSED: {
                if ((status = read_vertex_parameters(tk)) != TK_Normal)
                    return status;
                mp_stage = 0;       
                
            } break;

            case OPT_ALL_PARAMETERS:
            case OPT_ALL_PARAMETERS_COMPRESSED: {
                if ((status = read_vertex_parameters_all(tk)) != TK_Normal)
                    return status;
                mp_stage = 0;       
                
            } break;

            case OPT_ALL_VECOLORS:
            case OPT_ALL_VFCOLORS:
            case OPT_ALL_VMCOLORS: {
                if ((status = read_vertex_colors_all(tk, mp_optopcode)) != TK_Normal)
                    return status;
                mp_stage = 0;       
            } break;

            case OPT_VERTEX_MCOLORS:
            case OPT_VERTEX_ECOLORS:
            case OPT_VERTEX_FCOLORS: {
                if ((status = read_vertex_colors(tk, mp_optopcode)) != TK_Normal)
                    return status;
                mp_stage = 0;       
            } break;

            case OPT_ALL_VMINDICES:
            case OPT_ALL_VEINDICES:
            case OPT_ALL_VFINDICES: {
                if ((status = read_vertex_indices_all(tk, mp_optopcode)) != TK_Normal)
                    return status;
                mp_stage = 0;    
            } break;

            case OPT_VERTEX_MINDICES:
            case OPT_VERTEX_EINDICES:
            case OPT_VERTEX_FINDICES: {
                if ((status = read_vertex_indices(tk, mp_optopcode)) != TK_Normal)
                    return status;
                mp_stage = 0;    
            } break;

            case OPT_FACE_COLORS: {
                if (mp_face_exists == null)
                    SetFaceExists();
                if ((status = read_face_colors(tk)) != TK_Normal)
                    return status;
                mp_stage = 0;       
            } break;

            case OPT_ALL_FACE_COLORS: {
                if (mp_face_exists == null)
                    SetFaceExists();
                if ((status = read_face_colors_all(tk)) != TK_Normal)
                    return status;
                mp_stage = 0;       
            } break;

            case OPT_FACE_INDICES: {
                if (mp_face_exists == null)
                    SetFaceExists();
                if ((status = read_face_indices(tk)) != TK_Normal)
                    return status;
                mp_stage = 0;       
            } break;

            case OPT_ALL_FACE_INDICES: {
                if (mp_face_exists == null)
                    SetFaceExists();
                if ((status = read_face_indices_all(tk)) != TK_Normal)
                    return status;
                mp_stage = 0;       
            } break;

            case OPT_FACE_REGIONS: {
                if (mp_face_exists == null)
                    SetFaceExists();
                if ((status = read_face_regions(tk)) != TK_Normal)
                    return status;
                mp_stage = 0;       
            } break;

            case OPT_ENTITY_ATTRIBUTES: {
                if ((status = read_collection(tk)) != TK_Normal)
                    return status;
                mp_stage = 0;       
            } break;

            case OPT_ALL_VMVISIBILITIES:
            case OPT_VERTEX_MARKER_VISIBILITIES: {
                if ((status = read_vertex_marker_visibilities (tk)) != TK_Normal)
                    return status;
                mp_stage = 0;       
            } break;

            case OPT_ALL_VMSYMBOLS:     
            case OPT_VERTEX_MARKER_SYMBOLS: {
                if ((status = read_vertex_marker_symbols (tk)) != TK_Normal)
                    return status;
                mp_stage = 0;       
            } break;         

            case OPT_ALL_VMSIZES:       
            case OPT_VERTEX_MARKER_SIZES: {
                if ((status = read_vertex_marker_sizes (tk)) != TK_Normal)
                    return status;
                mp_stage = 0;       
            } break;         

            case OPT_ALL_FACE_VISIBILITIES:
            case OPT_FACE_VISIBILITIES: {
                if (mp_face_exists == null)
                    SetFaceExists();
                if ((status = read_face_visibilities (tk)) != TK_Normal)
                    return status;
                mp_stage = 0;       
            } break;

            case OPT_ALL_FACE_PATTERNS:      
            case OPT_FACE_PATTERNS: {  
                if (mp_face_exists == null)
                    SetFaceExists();
                if ((status = read_face_patterns (tk)) != TK_Normal)
                    return status;
                mp_stage = 0;       
            } break;

            case OPT_ALL_FACE_NORMALS_POLAR: 
            case OPT_FACE_NORMALS_POLAR: {
                if (mp_face_exists == null)
                    SetFaceExists();
                if ((status = read_face_normals (tk)) != TK_Normal)
                    return status;
                mp_stage = 0;       
            } break;


            case OPT_ALL_EDGE_VISIBILITIES:
            case OPT_EDGE_VISIBILITIES: {
                if (mp_edge_exists == null)
                    SetEdgeExists();
                if ((status = read_edge_visibilities (tk)) != TK_Normal)
                    return status;
                mp_stage = 0;       
            } break;

            case OPT_ALL_EDGE_PATTERNS:      
            case OPT_EDGE_PATTERNS: {
                if (mp_edge_exists == null)
                    SetEdgeExists();
                if ((status = read_edge_patterns (tk)) != TK_Normal)
                    return status;
                mp_stage = 0;       
            } break;          


            case OPT_ALL_EDGE_COLORS:        
            case OPT_EDGE_COLORS: {
                if (mp_edge_exists == null)
                    SetEdgeExists();
                if ((status = read_edge_colors (tk)) != TK_Normal)
                    return status;
                mp_stage = 0;       
            } break;          
            
            case OPT_ALL_EDGE_INDICES:       
            case OPT_EDGE_INDICES: {
                if (mp_edge_exists == null)
                    SetEdgeExists();
                if ((status = read_edge_indices (tk)) != TK_Normal)
                    return status;
                mp_stage = 0;       
            } break;          
           
            case OPT_ALL_EDGE_NORMALS_POLAR: 
            case OPT_EDGE_NORMALS_POLAR: {
                if (mp_edge_exists == null)
                    SetEdgeExists();
                if ((status = read_edge_normals (tk)) != TK_Normal)
                    return status;
                mp_stage = 0;       
            } break;          

            case OPT_ALL_EDGE_WEIGHTS:       
            case OPT_EDGE_WEIGHTS: {
                if (mp_edge_exists == null)
                    SetEdgeExists();
                if ((status = read_edge_weights (tk)) != TK_Normal)
                    return status;
                mp_stage = 0;       
            } break;               

            default:        
                return tk.Error("unrecognized case in TK_Polyhedron::Read");
        }
    }

    return status;
} //end function TK_Polyhedron::Read


//protected virtual
TK_Status TK_Polyhedron::mangle_points(BStreamFileToolkit & tk, int pointmap_count, int *pointmap) {
    int i, index;
    float *temparray;

    temparray = new float[ pointmap_count * 3 ];
    if (!temparray) {
        return tk.Error("Out of Memory.\n Allocation failed in TK_Polyhedron::mangle_points");
    }
    //the vertex locations
    for (i = 0; i < pointmap_count; i++) {
        index = pointmap[i];
        temparray[ i*3 + 0 ] = mp_points[ index*3 + 0 ]; 
        temparray[ i*3 + 1 ] = mp_points[ index*3 + 1 ]; 
        temparray[ i*3 + 2 ] = mp_points[ index*3 + 2 ]; 
    }
    delete [] mp_points;
    mp_points = temparray;
    mp_pointcount = pointmap_count;
    return TK_Normal;

} //end function TK_Polyhedron::mangle_points


//protected virtual
TK_Status TK_Polyhedron::mangle_point_attributes(BStreamFileToolkit & tk, int pointmap_count, int *pointmap) {
    int i, j, index;
    float *temparray;
    unsigned int *uintarray;
    char *chararray;
    unsigned char *uchararray;
    bool recount = false;

    //Normals    
    if (HasVertexNormals()) {
        temparray = new float[ pointmap_count * 3 ];
        if (!temparray) {
            return tk.Error("Out of Memory in TK_Polyhedron::mangle_point_attributes(0)");
        }
        for (i = 0; i < pointmap_count; i++) {
            index = pointmap[i];
            temparray[ i*3 + 0 ] = mp_normals[ index*3 + 0 ]; 
            temparray[ i*3 + 1 ] = mp_normals[ index*3 + 1 ]; 
            temparray[ i*3 + 2 ] = mp_normals[ index*3 + 2 ]; 
        }
        delete [] mp_normals;
        if (mp_normalcount != pointmap_count)
            recount = true;
        mp_normalcount = pointmap_count;
        mp_normals = temparray;
    } 
    //Parameters (a.k.a. texture coordinates)
    if (HasVertexParameters()) {
		if (mp_paramwidth <= 0)
			return tk.Error("invalid vertex parameter width in TK_Polyhedron::mangle_point_attributes");
		int w = mp_paramwidth;
        temparray = new float[ pointmap_count * w ];
        if (!temparray) {
            return tk.Error("Out of Memory in TK_Polyhedron::mangle_point_attributes(1)");
        }
        for (i = 0; i < pointmap_count; i++) {
            index = pointmap[i];
			for (j = 0 ; j < w ; j++)
				temparray[ i*w + j ] = mp_params[ index*w + j ]; 
        }
        delete [] mp_params;
        if (mp_paramcount != pointmap_count)
            recount = true;
        mp_params = temparray;
    } 
    //Colors applied to faces
    if (HasVertexFaceColors()) {
        temparray = new float[ pointmap_count * 3 ];
        if (!temparray) {
            return tk.Error("Out of Memory in TK_Polyhedron::mangle_point_attributes(2)");
        }
        for (i = 0; i < pointmap_count; i++) {
            index = pointmap[i];
            temparray[ i*3 + 0 ] = mp_vfcolors[ index*3 + 0 ]; 
            temparray[ i*3 + 1 ] = mp_vfcolors[ index*3 + 1 ]; 
            temparray[ i*3 + 2 ] = mp_vfcolors[ index*3 + 2 ]; 
        }
        delete [] mp_vfcolors;
        if (mp_vfcolorcount != pointmap_count)
            recount = true;
        mp_vfcolors = temparray;
    } 
    //Colors applied to edges
    if (HasVertexEdgeColors()) {
        temparray = new float[ pointmap_count * 3 ];
        if (!temparray) {
            return tk.Error("Out of Memory in TK_Polyhedron::mangle_point_attributes(3)");
        }
        for (i = 0; i < pointmap_count; i++) {
            index = pointmap[i];
            temparray[ i*3 + 0 ] = mp_vecolors[ index*3 + 0 ]; 
            temparray[ i*3 + 1 ] = mp_vecolors[ index*3 + 1 ]; 
            temparray[ i*3 + 2 ] = mp_vecolors[ index*3 + 2 ]; 
        }
        delete [] mp_vecolors;
        if (mp_vecolorcount != pointmap_count)
            recount = true;
        mp_vecolors = temparray;
    } 
    //Colors applied to markers
    if (HasVertexMarkerColors()) {
        temparray = new float[ pointmap_count * 3 ];
        if (!temparray) {
            return tk.Error("Out of Memory in TK_Polyhedron::mangle_point_attributes(4)");
        }
        for (i = 0; i < pointmap_count; i++) {
            index = pointmap[i];
            temparray[ i*3 + 0 ] = mp_vmcolors[ index*3 + 0 ]; 
            temparray[ i*3 + 1 ] = mp_vmcolors[ index*3 + 1 ]; 
            temparray[ i*3 + 2 ] = mp_vmcolors[ index*3 + 2 ]; 
        }
        delete [] mp_vmcolors;
        if (mp_vmcolorcount != pointmap_count)
            recount = true;
        mp_vmcolors = temparray;
    }
    //Colors-by-index applied to faces
    if (HasVertexFaceIndices()) {
        temparray = new float[ pointmap_count ];
        if (!temparray) {
            return tk.Error("Out of Memory in TK_Polyhedron::mangle_point_attributes(5)");
        }
        for (i = 0; i < pointmap_count; i++) {
            index = pointmap[i];
            temparray[ i ] = mp_vfindices[ index ]; 
        }
        delete [] mp_vfindices;
        if (mp_vfindexcount != pointmap_count)
            recount = true;
        mp_vfindices = temparray;
    } 
    //Colors-by-index applied to edges
    if (HasVertexEdgeIndices()) {
        temparray = new float[ pointmap_count ];
        if (!temparray) {
            return tk.Error("Out of Memory in TK_Polyhedron::mangle_point_attributes(6)");
        }
        for (i = 0; i < pointmap_count; i++) {
            index = pointmap[i];
            temparray[ i ] = mp_veindices[ index ]; 
        }
        delete [] mp_veindices;
        if (mp_veindexcount != pointmap_count)
            recount = true;
        mp_veindices = temparray;
    } 
    //Colors-by-index applied to markers
    if (HasVertexMarkerIndices()) {
        temparray = new float[ pointmap_count ];
        if (!temparray) {
            return tk.Error("Out of Memory in TK_Polyhedron::mangle_point_attributes(7)");
        }
        for (i = 0; i < pointmap_count; i++) {
            index = pointmap[i];
            temparray[ i ] = mp_vmindices[ index ]; 
        }
        delete [] mp_vmindices;
        if (mp_vmindexcount != pointmap_count)
            recount = true;
        mp_vmindices = temparray;
    } 
    //vertex marker visibilities
    if (HasVertexMarkerVisibilities()) {
        chararray = new char[ pointmap_count ];
        if (!chararray) {
            return tk.Error("Out of Memory in TK_Polyhedron::mangle_point_attributes(7)");
        }
        for (i = 0; i < pointmap_count; i++) {
            index = pointmap[i];
            chararray[ i ] = mp_vmvisibilities[ index ]; 
        }
        delete [] mp_vmvisibilities;
        if (mp_vmindexcount != pointmap_count)
            recount = true;
        mp_vmvisibilities = chararray;
    } 
    //vertex marker symbols
    if (HasVertexMarkerSymbols()) {
        uchararray = new unsigned char[ pointmap_count ];
        if (!uchararray) {
            return tk.Error("Out of Memory in TK_Polyhedron::mangle_point_attributes(7)");
        }
        for (i = 0; i < pointmap_count; i++) {
            index = pointmap[i];
            uchararray[ i ] = mp_vmsymbols[ index ]; 
        }
        delete [] mp_vmsymbols;
        if (mp_vmindexcount != pointmap_count)
            recount = true;
        mp_vmsymbols = uchararray;
    } 
    //vertex marker sizes
    if (HasVertexMarkerSizes()) {
        temparray = new float[ pointmap_count ];
        if (!temparray) {
            return tk.Error("Out of Memory in TK_Polyhedron::mangle_point_attributes(7)");
        }
        for (i = 0; i < pointmap_count; i++) {
            index = pointmap[i];
            temparray[ i ] = mp_vmsizes[ index ]; 
        }
        delete [] mp_vmsizes;
        if (mp_vmindexcount != pointmap_count)
            recount = true;
        mp_vmsizes = temparray;
    } 
    //The existance flags
    if (mp_exists != null) {
        uintarray = new unsigned int[ pointmap_count ];
        if (!uintarray) {
            return tk.Error("Out of Memory.\n Allocation of unsigned int array failed in TK_Polyhedron::mangle_points");
        }
        if (recount) {
            /* if any removed unreferenced vertices had attributes, we do a recount */
            mp_normalcount = 0; 
            mp_paramcount = 0;  
            mp_vfcolorcount = 0;
            mp_vecolorcount = 0;
            mp_vmcolorcount = 0;
            mp_vfindexcount = 0;
            mp_veindexcount = 0;
            mp_vmindexcount = 0;
            for (i = 0; i < pointmap_count; i++) {
                index = pointmap[i];
                uintarray[ i ] = mp_exists[ index ];
                if (mp_exists[ index ] != Vertex_None) {
                    if (mp_exists[ index ] & Vertex_Normal) 
                        mp_normalcount++; 
                    if (mp_exists[ index ] & Vertex_Parameter) 
                        mp_paramcount++;  
                    if (mp_exists[ index ] & Vertex_Any_Color) {
                        if (mp_exists[ index ] & Vertex_Face_Color) 
                            mp_vfcolorcount++;
                        if (mp_exists[ index ] & Vertex_Edge_Color) 
                            mp_vecolorcount++;
                        if (mp_exists[ index ] & Vertex_Marker_Color) 
                            mp_vmcolorcount++;
                        if (mp_exists[ index ] & Vertex_Face_Index) 
                            mp_vfindexcount++;
                        if (mp_exists[ index ] & Vertex_Edge_Index) 
                            mp_veindexcount++;
                        if (mp_exists[ index ] & Vertex_Marker_Index) 
                            mp_vmindexcount++;
                    }
                }
            }
        }
        else {
            for (i = 0; i < pointmap_count; i++) {
                index = pointmap[i];
                uintarray[ i ] = mp_exists[ index ];
            }
        }
        delete [] mp_exists;
        mp_exists = uintarray;
    }
    mp_pointcount = pointmap_count;
    return TK_Normal;

} //end function TK_Polyhedron::mangle_point_attributes


//protected virtual
TK_Status TK_Polyhedron::mangle_face_attributes(BStreamFileToolkit & tk, int facemap_count, int *facemap) {
    int i, index;
    float *temparray;
    char *chararray;

    //Face Colors    
    if (HasFaceColors()) {
        temparray = new float[ facemap_count * 3 ];
        if (!temparray) {
            return tk.Error("Out of Memory (0).\n Allocation failed in TK_Polyhedron::mangle_faces");
        }
        for (i = 0; i < facemap_count; i++) {
            index = facemap[i];
            temparray[ i*3 + 0 ] = mp_fcolors[ index*3 + 0 ]; 
            temparray[ i*3 + 1 ] = mp_fcolors[ index*3 + 1 ]; 
            temparray[ i*3 + 2 ] = mp_fcolors[ index*3 + 2 ]; 
        }
        delete [] mp_fcolors;
        mp_fcolors = temparray;
    } 
    //Face Colors-by-index
    if (HasFaceIndices()) {
        temparray = new float[ facemap_count ];
        if (!temparray) {
            return tk.Error("Out of Memory (1).\n Allocation failed in TK_Polyhedron::mangle_faces");
        }
        for (i = 0; i < facemap_count; i++) {
            index = facemap[i];
            temparray[ i ] = mp_findices[ index ]; 
        }
        delete [] mp_findices;
        mp_findices = temparray;
    }
    //Face Normals    
    if (HasFaceNormals()) {
        temparray = new float[ facemap_count * 3 ];
        if (!temparray) {
            return tk.Error("Out of Memory (2).\n Allocation failed in TK_Polyhedron::mangle_faces");
        }
        if (mp_fnormalcount == mp_facecount) {
            for (i = 0; i < facemap_count; i++) {
                index = facemap[i];
                temparray[ i*3 + 0 ] = mp_fnormals[ index*3 + 0 ]; 
                temparray[ i*3 + 1 ] = mp_fnormals[ index*3 + 1 ]; 
                temparray[ i*3 + 2 ] = mp_fnormals[ index*3 + 2 ]; 
            }
        }
        else {
            for (i = 0; i < facemap_count; i++) {
                index = facemap[i];
                if (mp_face_exists[index] & Face_Normal) {
                    temparray[ i*3 + 0 ] = mp_fnormals[ index*3 + 0 ]; 
                    temparray[ i*3 + 1 ] = mp_fnormals[ index*3 + 1 ]; 
                    temparray[ i*3 + 2 ] = mp_fnormals[ index*3 + 2 ]; 
                }
            }
        }
        delete [] mp_fnormals;
        mp_fnormals = temparray;
    } 
    //Face Visibilities
    if (HasFaceVisibilities()) {
        chararray = new char[ facemap_count ];
        if (!chararray) {
            return tk.Error("Out of Memory (3).\n Allocation failed in TK_Polyhedron::mangle_faces");
        }
        if (mp_fvisibilitycount == mp_facecount) {
            for (i = 0; i < facemap_count; i++) {
                index = facemap[i];
                chararray[ i ] = mp_fvisibilities[ index ]; 
            }
        }
        else {
            for (i = 0; i < facemap_count; i++) {
                index = facemap[i];
                if (mp_face_exists[index] & Face_Visibility)
                    chararray[ i ] = mp_fvisibilities[ index ]; 
            }
        }
        delete [] mp_fvisibilities;
        mp_fvisibilities = chararray;
    }
    //Face Patterns
    if (HasFacePatterns()) {
        chararray = new char[ facemap_count ];
        if (!chararray) {
            return tk.Error("Out of Memory (4).\n Allocation failed in TK_Polyhedron::mangle_faces");
        }
        if (mp_fpatterncount == mp_facecount) {
            for (i = 0; i < facemap_count; i++) {
                index = facemap[i];
                chararray[ i ] = mp_fpatterns[ index ]; 
            }
        }
        else {
            for (i = 0; i < facemap_count; i++) {
                index = facemap[i];
                if (mp_face_exists[index] & Face_Pattern)
                    chararray[ i ] = mp_fpatterns[ index ]; 
            }
        }
        delete [] mp_fpatterns;
        mp_fpatterns = chararray;
    }

    //Face Regions    
    if (HasFaceRegions()) {
        int *temp_ints;

        temp_ints = new int [facemap_count];
        if (!temp_ints) {
            return tk.Error("Out of Memory (5).\n Allocation failed in TK_Polyhedron::mangle_faces");
        }
        for (i=0; i<facemap_count; i++) {
            temp_ints[i] = mp_fregions[facemap[i]]; 
        }
        delete [] mp_fregions;
        mp_fregions = temp_ints;
    } 

    //The existance flags
    if (mp_face_exists != null) {
        unsigned int *uintarray;

        uintarray = new unsigned int[ facemap_count ];
        if (!uintarray) {
            return tk.Error("Out of Memory (6).\n Allocation of unsigned int array failed in TK_Polyhedron::mangle_faces");
        }
        for (i = 0; i < facemap_count; i++) {
            index = facemap[i];
            uintarray[ i ] = mp_face_exists[ index ]; 
        }
        delete [] mp_face_exists;
        mp_face_exists = uintarray;

        // fix up counts based on new flags
        mp_fcolorcount = 0;
        mp_findexcount = 0;
        mp_fnormalcount = 0;
        mp_fvisibilitycount = 0;
        mp_fpatterncount = 0;
        for (i=0; i<facemap_count; i++) {
            if (mp_face_exists[i] & Face_Color)
                mp_fcolorcount++;
            if (mp_face_exists[i] & Face_Index)
                mp_findexcount++;
            if (mp_face_exists[i] & Face_Normal)
                mp_fnormalcount++;
            if (mp_face_exists[i] & Face_Visibility)
                mp_fvisibilitycount++;
            if (mp_face_exists[i] & Face_Pattern)
                mp_fpatterncount++;
        }
    }
    mp_facecount = facemap_count;
    return TK_Normal;

} //end function TK_Polyhedron::mangle_face_attributes


BBaseOpcodeHandler *TK_Polyhedron::PopObject() {
    BBaseOpcodeHandler *obj;

    if (mp_num_collection_parts == 0)
        return null;
    obj = mp_collection_parts[ mp_num_collection_parts - 1 ];
    mp_collection_parts[ mp_num_collection_parts - 1 ] = null;
    mp_num_collection_parts--;
    return obj;

} //end function TK_Polyhedron::PopObject



TK_Status TK_Polyhedron::AppendObject(BStreamFileToolkit & tk, BBaseOpcodeHandler *obj) {
    BBaseOpcodeHandler **temp;
    int i;

    mp_subop |= TKSH_EXPANDED;
    mp_subop2 |= TKSH2_COLLECTION;
    temp = new BBaseOpcodeHandler *[ mp_num_collection_parts + 1 ];
    if (temp == null)
        return tk.Error("memory allocation failed in function TK_Polyhedron::AppendObject");
    if (mp_num_collection_parts != 0) {
        for (i = 0; i < mp_num_collection_parts; i++)
            temp[i] = mp_collection_parts[i];
        delete [] mp_collection_parts;
    }
    temp[mp_num_collection_parts] = obj;
    mp_collection_parts = temp;
    mp_num_collection_parts++;
    return TK_Normal;

} //end function TK_Polyhedron::AppendObject


bool TK_Polyhedron::AllowInstancing (BStreamFileToolkit & tk) const 
{
    if ((tk.GetWriteFlags() & TK_Disable_Instancing) ||
        ((mp_present & ~Vertex_Normal) | mp_face_present | mp_edge_present) ||
        mp_pointcount < 10 || 
        mp_num_collection_parts > 0)
        return false;
    else
        return true;
}


void TK_Polyhedron::Reset (void) alter 
{
    delete [] mp_points;
    delete [] mp_normals;
    delete [] mp_params;
    delete [] mp_vfcolors;
    delete [] mp_vecolors;
    delete [] mp_vmcolors;
    delete [] mp_vfindices;
    delete [] mp_veindices;
    delete [] mp_vmindices;
    delete [] mp_vmvisibilities;
    delete [] mp_vmsizes;
    delete [] mp_vmsymbols;
    if (mp_vmsymbolstrings != null) {
        int i;
        for (i = 0; i < mp_pointcount; i++)
            delete [] mp_vmsymbolstrings[i];
        delete [] mp_vmsymbolstrings;
        mp_vmsymbolstrings = null;
    }

    delete [] mp_fcolors;
    delete [] mp_findices;
    delete [] mp_fnormals;
    delete [] mp_fvisibilities;
    delete [] mp_fpatterns;
    delete [] mp_fregions;

    delete [] mp_ecolors;
    delete [] mp_eindices;
    delete [] mp_enormals;
    delete [] mp_evisibilities;
    delete [] mp_epatterns;
    if (mp_epatternstrings != null) {
        int i;
        for (i = 0; i < mp_edgecount; i++)
            delete [] mp_epatternstrings[i];
        delete [] mp_epatternstrings;
        mp_epatternstrings = null;
    }
    delete [] mp_eweights;

    delete [] mp_exists;
    delete [] mp_edge_exists;
    delete [] mp_face_exists;
    delete [] mp_workspace;
    delete [] mp_edge_enumeration;
    delete mp_instance;
    if (mp_collection_parts != null) {
        int i;
        for (i = 0; i < mp_num_collection_parts; i++)
            delete mp_collection_parts[i];
        delete [] mp_collection_parts;
        mp_collection_parts = null;
    }

    mp_pointcount = 0;
    mp_points = null;
    mp_normals = null;
    mp_params = null;
    mp_vfcolors = null;
    mp_vecolors = null;
    mp_vmcolors = null;
    mp_vfindices = null;
    mp_veindices = null;
    mp_vmindices = null;
    mp_vmvisibilities = null;
    mp_vmsizes = null;
    mp_vmsymbols = null;

    mp_facecount = 0;
    mp_fcolors  = null;
    mp_findices  = null;
    mp_fnormals = null;
    mp_fvisibilities = null;
    mp_fpatterns = null;
    mp_fregions  = null;

    mp_edgecount = -1;
    mp_ecolors = null;
    mp_eindices = null;
    mp_enormals = null;
    mp_evisibilities = null;
    mp_epatterns = null;
    mp_eweights = null;

    mp_exists = null;
    mp_face_exists = null;
    mp_edge_exists = null;

    mp_normalcount = 0;
    mp_paramcount = 0;
    mp_paramwidth = 3;  //for backwards compatibility.
    mp_vfcolorcount = 0;
    mp_vecolorcount = 0;
    mp_vmcolorcount = 0;
    mp_vfindexcount = 0;
    mp_veindexcount = 0;
    mp_vmindexcount = 0;
    mp_vmvisibilitycount = 0;
    mp_vmsizecount = 0;
    mp_vmsymbolcount = 0;

    mp_fcolorcount = 0;
    mp_findexcount = 0;
    mp_fnormalcount = 0;
    mp_fvisibilitycount = 0;
    mp_fpatterncount = 0;

    mp_ecolorcount = 0;
    mp_eindexcount = 0;
    mp_enormalcount = 0;
    mp_evisibilitycount = 0;
    mp_epatterncount = 0;
    mp_eweightcount = 0;

    mp_workspace = null;
    mp_edge_enumeration = null;
    mp_instance = null;
    mp_present = 0;
    mp_face_present = 0;
    mp_edge_present = 0;
    mp_workspace_used = 0;
    mp_workspace_allocated = 0;
    mp_bits_per_sample = 0;
    mp_num_collection_parts = 0;
    mp_bbox[0] = 0; mp_bbox[1] = 0; mp_bbox[2] = 0; 
    mp_bbox[3] = 0; mp_bbox[4] = 0; mp_bbox[5] = 0; 
    mp_key = -1;

    mp_stage = mp_substage = mp_progress = 0;
    mp_optopcode = mp_compression_scheme = 0;
    mp_subop = 0;
    mp_subop2 = 0;

    BBaseOpcodeHandler::Reset ();
}


////////////////////////////////////////////////////////////////////////////////


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


TK_Status TK_Mesh::EnumerateEdges () alter 
{
    int quad_count, enumcount, i, j;

    quad_count = (m_rows-1)*(m_columns-1);
    mp_edgecount = 3*quad_count + (m_rows-1) + (m_columns-1);

    mp_edge_enumeration = new int[ 2*mp_edgecount ];
    enumcount = 0;
    //diagonal
    for (i = 1; i < m_rows; i++) {
        for (j = 0; j < m_columns-1; j++) {
            mp_edge_enumeration[ enumcount++ ] = i*m_columns + j;
            mp_edge_enumeration[ enumcount++ ] = i*m_columns + j - m_columns + 1; //vertex diagonal 
        }
    }
    
    for (i = 0; i < m_rows; i++) {
        for (j = 1; j < m_columns; j++) {
            mp_edge_enumeration[ enumcount++ ] = i*m_columns + j;
            mp_edge_enumeration[ enumcount++ ] = i*m_columns + j - 1; //vertex to left 
        }
    }
    //vertical    
    for (i = 1; i < m_rows; i++) {
        for (j = 0; j < m_columns; j++) {
            mp_edge_enumeration[ enumcount++ ] = i*m_columns + j;
            mp_edge_enumeration[ enumcount++ ] = i*m_columns + j - m_columns; //vertex down 
        }
    }
    //sort first by lower index, then by upper index    
    qsort(mp_edge_enumeration, enumcount/2, 2*sizeof(int), (qsortfunc) edge_enum_compare);
    //no need to remove duplicates -- there won't be any    
    return TK_Normal;
}


TK_Status TK_Mesh::Write (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_READONLY
    TK_Status       status = TK_Normal;

#ifndef BSTREAM_DISABLE_ASCII
	if( tk.GetAsciiMode() ) 
		return WriteAscii (tk);
#endif

    if (mp_instance != null)
        return mp_instance->Write (tk);

    switch (m_stage) {
        case 0: {
            if (mp_subop2 & TKSH2_GLOBAL_QUANTIZATION) {
                if (tk.GetTargetVersion() < 806 ||
                    !tk.GetWorldBounding()) {
                    mp_subop2 &= ~TKSH2_GLOBAL_QUANTIZATION;
                    if (mp_subop2 == 0)
                        mp_subop &= ~TKSH_EXPANDED;
                }
            }
            m_stage++;
        }   nobreak;

        //opcode
        case 1: {
            if ((status = PutOpcode (tk)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        //subop
        case 2: {  
            if ((status = PutData(tk, mp_subop)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        //subop2 (if applicable)
        case 3: {
            if (mp_subop & TKSH_EXPANDED) {
                if ((status = PutData(tk, mp_subop2)) != TK_Normal)
                    return status;
            }
            m_stage++;
        }   nobreak;

        //number rows
        case 4: {
            if ((status = PutData (tk, m_rows)) != TK_Normal)
                return status;
            m_progress = 0;
            m_stage++;
        }   nobreak;

        //number of columns
        case 5: {
            if ((status = PutData (tk, m_columns)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        //points
        case 6: {
            if (mp_subop & TKSH_COMPRESSED_POINTS) {
                mp_compression_scheme = CS_TRIVIAL;
                if ((status = write_trivial_points(tk)) != TK_Normal)
                    return status;
            }
            else
                if ((status = PutData (tk, mp_points, 3 * m_rows * m_columns)) != TK_Normal)
                    return status;
            m_stage++;
        }   nobreak;

        //attributes
        case 7: {
            if (mp_subop & TKSH_HAS_OPTIONALS) {
                if ((status = TK_Polyhedron::Write(tk)) != TK_Normal)
                        return status;
            }
            m_stage++;
        }   nobreak;

        //instance registration (if applicable)
        case 8: {
            if ((tk.GetWriteFlags() & TK_Disable_Instancing) == 0) {
                if ((status = Tag (tk)) != TK_Normal)
                    return status;

                Record_Instance (tk, mp_key, -1,
                                 m_rows, m_columns, mp_present);
            }
            else if (Tagging (tk))
                if ((status = Tag (tk)) != TK_Normal)
                    return status;

            m_stage = -1;
        }   break;

        default:
            return tk.Error();
    }
    return status;
#else
    return tk.Error (stream_readonly);
#endif
} // end function TK_Mesh::Write


TK_Status TK_Mesh::Read (BStreamFileToolkit & tk) alter {
    TK_Status       status = TK_Normal;

		
#ifndef BSTREAM_DISABLE_ASCII
	if( tk.GetAsciiMode() ) 
		return ReadAscii (tk);
#endif

    switch (m_stage) {
        case 0: {
            if (tk.GetVersion() > 100) {  //subop info added after 1.00
                if ((status = GetData (tk, mp_subop)) != TK_Normal)
                    return status;
            }
            else
                mp_subop = 0; //assume defaults for early file versions
            m_stage++;
        }   nobreak;

        case 1: {
            if ((status = GetData (tk, m_rows)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 2: {
            if ((status = GetData (tk, m_columns)) != TK_Normal)
                return status;
            mp_pointcount = m_rows * m_columns;
            if (!validate_count (mp_pointcount))
                return tk.Error("bad Mesh Point count");
            mp_facecount = 2 * (m_rows-1) * (m_columns-1);
            m_stage++;
        }   nobreak;

        case 3: {
            if (mp_subop & TKSH_COMPRESSED_POINTS){
                if ((status = GetData(tk, mp_compression_scheme)) != TK_Normal)
                    return status;
                // no need to set_points() here since unpack floats (called from read_trivial) will do that for us
            }
            else {
                set_points(mp_pointcount);
            }
            m_stage++;
        }   nobreak;

        case 4: {
            if (mp_subop & TKSH_COMPRESSED_POINTS) {
                if ((status = read_trivial_points(tk)) != TK_Normal)
                    return status;
            }
            else
                if ((status = GetData (tk, mp_points, 3 * mp_pointcount)) != TK_Normal)
                    return status;

            m_stage++;
        }   nobreak;

        case 5: {
            // here is where we (optionally) get vertex and face attributes 
            if (mp_subop & TKSH_HAS_OPTIONALS) {
                if ((status = TK_Polyhedron::Read(tk)) != TK_Normal)
                    return status;
            }
            m_stage = -1;
        }   break;

        default:
            return tk.Error();
    }
    return status;

} // end function TK_Mesh::Read


void TK_Mesh::Reset (void) alter {
    m_rows = 0;
    m_columns = 0;

    TK_Polyhedron::Reset();
}

