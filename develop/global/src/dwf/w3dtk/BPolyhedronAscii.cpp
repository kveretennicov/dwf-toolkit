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
// $Header: //DWF/Development/Components/Internal/DWF Toolkit/v7.7/develop/global/src/dwf/w3dtk/BPolyhedronAscii.cpp#2 $
//


#include "dwf/w3dtk/BStream.h"
#include "dwf/w3dtk/BOpcodeShell.h"
#include "dwf/w3dtk/BCompress.h"
#include "dwf/w3dtk/BInternal.h"
#include <stdlib.h>
#include <ctype.h>


#define ASSERT(x)

#define H_SAFE_DELETE_ARRAY(x_array)	if(x_array)				\
									{						\
										delete[] x_array;	\
										x_array = NULL;		\
									}

static bool strni_equal (
                register    char const      *a,
                register    char const      *b,
                register    int             max )
{

    do {
        if (--max < 0) return true;
        if ( tolower(*a) != tolower(*b)) return false;
        ++a;
    }
    until (*b++ == '\0');

    return true;
}

static char const * opcode_list[256] = {
    "Termination         ", "Pause               ", "-- 0x02 --          ", "-- 0x03 --          ", //^ abc
    "-- 0x04 --          ", "-- 0x05 --          ", "-- 0x06 --          ", "Callback            ", //^defg
    "Color_By_Index      ", "Color_By_Index_16   ", "Color_By_FIndex     ", "Color_By_Value      ", //^hijk
    "Color_Map           ", "Edge_Pattern        ", "Edge_Weight         ", "[Rsrv:Complex_Clip] ", //^lmno

    "PolyPolyline        ", "-- 0x011 --         ", "External_Reference  ", "-- 0x13 --          ", //^pqrs
    "Thumbnail           ", "URL                 ", "Unicode_Options     ", "-- 0x17 --          ", //^tuvw
    "XML                 ", "LOD                 ", "Sphere              ", "-- 0x1B --          ", //^xyz[
    "-- 0x1C --          ", "-- 0x1D --          ", "-- 0x1E --          ", "-- 0x1F --          ", //^|]^_

    "Text_Spacing        ", "Selectability       ", "Color               ", "Window_Frame        ", //  !"#
    "Texture_Matrix      ", "Modelling_Matrix    ", "Repeat_Object       ", "-- ' --             ", // $%&'
    "Open_Segment        ", "Close_Segment       ", "Text_Alignment      ", "Marker_Size         ", // ()*+
    "<Streaming_Mode>    ", "Line_Pattern        ", "Local_Light         ", "Cutting_Plane       ", // ,-./

    "Priority            ", "-- 1 --             ", "-- 2 --             ", "-- 3 --             ", // 0123
    "-- 4 --             ", "-- 5 --             ", "-- 6 --             ", "-- 7 --             ", // 4567
    "-- 8 --             ", "-- 9 --             ", "Geometry_Attibutes  ", "Comment             ", // 89:;
    "Include_Segment     ", "Line_Weight         ", "Camera              ", "Conditional         ", // <=>?

    "Marker_Symbol       ", "NURBS_Surface       ", "Bounding_Info       ", "Circle              ", // @ABC
    "Dictionary          ", "Ellipse             ", "Text_Font           ", "Polygon             ", // DEFG
    "Heuristics          ", "File_Info           ", "Line_Style          ", "Renumber_Key_Global ", // HIJK
    "Polyline            ", "Mesh                ", "NURBS_Curve         ", "Clip_Region         ", // LMNO

    "Face_Pattern        ", "PolyCylinder        ", "Rendering_Options   ", "Shell               ", // PQRS
    "Text                ", "User_Options        ", "Visibility          ", "Window              ", // TUVW
    "Marker              ", "Cylinder            ", "Start_Compression   ", "Start_User_Data     ", // XYZ[
    "Circular_Chord      ", "Stop_User_Data      ", "Spot_Light          ", "Dictionary_Locater  ", // \]^_

    "-- ` --             ", "Area_Light          ", "Bounding            ", "Circular_Arc        ", // `abc
    "Distant_Light       ", "Elliptical_Arc      ", "Font                ", "Grid                ", // defg
    "Handedness          ", "Image               ", "Glyph Definition    ", "Renumber_Key_Local  ", // hijk
    "Line                ", "-- m --             ", "User_Index          ", "Clip_Rectangle      ", // lmno

    "Window_Pattern      ", "Tag                 ", "-- r --             ", "Reopen_Segment      ", // pqrs
    "Texture             ", "-- u --             ", "User_Value          ", "Circular_Wedge      ", // tuvw
    "Text_With_Encoding  ", "-- y --             ", "Stop_Compression    ", "Style_Segment       ", // xyz{
    "Text_Path           ", "View                ", "Color_RGB           ", "Delete_Object       ", // |}~

    "-- 0x80 --          ", "-- 0x81 --          ", "-- 0x82 --          ", "-- 0x83 --          ",
    "-- 0x84 --          ", "-- 0x85 --          ", "-- 0x86 --          ", "-- 0x87 --          ",
    "-- 0x88 --          ", "-- 0x88 --          ", "-- 0x8A --          ", "-- 0x8B --          ",
    "-- 0x8C --          ", "-- 0x8D --          ", "-- 0x8E --          ", "-- 0x8F --          ",
    "-- 0x90 --          ", "-- 0x91 --          ", "-- 0x92 --          ", "-- 0x93 --          ",
    "-- 0x94 --          ", "-- 0x95 --          ", "-- 0x96 --          ", "-- 0x97 --          ",
    "-- 0x98 --          ", "-- 0x98 --          ", "-- 0x9A --          ", "-- 0x9B --          ",
    "-- 0x9C --          ", "-- 0x9D --          ", "-- 0x9E --          ", "-- 0x9F --          ",

    "-< 0xA0 >-          ", "-< 0xA1 >-          ", "-< 0xA2 >-          ", "-< 0xA3 >-          ",	//  ¡¢?    "-< 0xA4 >-          ", "-< 0xA5 >-          ", "-< 0xA6 >-          ", "-< 0xA7 >-          ",	// ¤¥¦§
    "-< 0xA4 >-          ", "-< 0xA5 >-          ", "-< 0xA6 >-          ", "-< 0xA7 >-          ",	// ¤¥¦§
    "-< 0xA8 >-          ", "-< 0xA8 >-          ", "-< 0xAA >-          ", "-< 0xAB >-          ",	// ¨©ª«
    "-< 0xAC >-          ", "-< 0xAD >-          ", "-< 0xAE >-          ", "-< 0xAF >-          ",	// ¬­®¯
    "-< 0xB0 >-          ", "-< 0xB1 >-          ", "-< 0xB2 >-          ", "-< 0xB3 >-          ",	// °±²³
    "-< 0xB4 >-          ", "-< 0xB5 >-          ", "-< 0xB6 >-          ", "-< 0xB7 >-          ",	// ´µ¶·
    "-< 0xB8 >-          ", "-< 0xB8 >-          ", "-< 0xBA >-          ", "-< 0xBB >-          ",	// ¸¹º»
    "-< 0xBC >-          ", "-< 0xBD >-          ", "-< 0xBE >-          ", "-< 0xBF >-          ",	// ¼½¾¿
    "-< 0xC0 >-          ", "-< 0xC1 >-          ", "-< 0xC2 >-          ", "-< 0xC3 >-          ",	// ÀÁÂÃ
    "-< 0xC4 >-          ", "-< 0xC5 >-          ", "-< 0xC6 >-          ", "-< 0xC7 >-          ",	// ÄÅÆÇ
    "-< 0xC8 >-          ", "-< 0xC8 >-          ", "-< 0xCA >-          ", "-< 0xCB >-          ",	// ÈÉÊË
    "-< 0xCC >-          ", "-< 0xCD >-          ", "-< 0xCE >-          ", "-< 0xCF >-          ",	// ÌÍÎÏ
    "-< 0xD0 >-          ", "-< 0xD1 >-          ", "-< 0xD2 >-          ", "-< 0xD3 >-          ",	// ÐÑÒÓ
    "-< 0xD4 >-          ", "-< 0xD5 >-          ", "-< 0xD6 >-          ", "-< 0xD7 >-          ",	// ÔÕÖ×
    "-< 0xD8 >-          ", "-< 0xD8 >-          ", "-< 0xDA >-          ", "-< 0xDB >-          ",	// ØÙÚÛ
    "-< 0xDC >-          ", "-< 0xDD >-          ", "-< 0xDE >-          ", "-< 0xDF >-          ",	// ÜÝÞß
    "-< 0xE0 >-          ", "-< 0xE1 >-          ", "-< 0xE2 >-          ", "-< 0xE3 >-          ",	// àáâã
    "-< 0xE4 >-          ", "-< 0xE5 >-          ", "-< 0xE6 >-          ", "-< 0xE7 >-          ",	// äåæç
    "-< 0xE8 >-          ", "-< 0xE8 >-          ", "-< 0xEA >-          ", "-< 0xEB >-          ",	// èéêë
    "-< 0xEC >-          ", "-< 0xED >-          ", "-< 0xEE >-          ", "-< 0xEF >-          ",	// ìíîï

    "-- 0xF0 --          ", "-- 0xF1 --          ", "-- 0xF2 --          ", "-- 0xF3 --          ",	// ðñòó
    "-- 0xF4 --          ", "-- 0xF5 --          ", "-- 0xF6 --          ", "-- 0xF7 --          ",	// ôõö÷
    "-- 0xF8 --          ", "-- 0xF8 --          ", "-- 0xFA --          ", "-- 0xFB --          ",	// øùúû
    "-- 0xFC --          ", "-- 0xFD --          ", "[Pseudo-Handler]    ", "<Extended>          "	// üý?
};

////////////////////////////////////////////////////////////////////////////////

//protected virtual
TK_Status TK_Polyhedron::write_trivial_points_ascii (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII

	TK_Status       status = TK_Normal;

	switch (mp_substage) {
//compression scheme
		case 0: {
			if ((status = PutStartXMLTag(tk,"Points")) != TK_Normal)
				return status;
			mp_substage++;
				}   nobreak;

        case 1: {
			PutTab t(&tk);
            if ((status = PutAsciiData(tk,"Compression_Scheme", (int)mp_compression_scheme)) != TK_Normal)
                return status;
            mp_substage++;
        } nobreak;

		 case 2: {
			 PutTab t(&tk);
			if ((status = PutAsciiData(tk, "Point_Count", mp_pointcount)) != TK_Normal)
				 return status;
			mp_substage++;
		}   nobreak;

		 case 3: {
			 PutTab t(&tk);
			if( mp_pointcount != 0)
				 if ((status = PutAsciiData(tk, "Coordinates",  mp_points, 3 * mp_pointcount)) != TK_Normal)
					 return status;
			mp_substage++;
				 }   nobreak;
		 case 4: {
			 if ((status = PutEndXMLTag(tk,"Points")) != TK_Normal)
				 return status;
			 mp_substage = 0;
				 }   break;

        default:
            return tk.Error("internal error from TK_Polyhedron::write_trivial_points");
    }
	
    return status;
#else
	return tk.Error(stream_disable_ascii);
#endif 

} // end function TK_Polyhedron::write_trivial_points


TK_Status TK_Polyhedron::write_vertex_normals_compressed_all_ascii (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII
    
	TK_Status       status = TK_Normal;
	PutTab t0(&tk);
    ASSERT(mp_optopcode == OPT_ALL_NORMALS_COMPRESSED);
    switch (mp_substage) {
    	
    	//compression scheme
		case 1: {
			if ((status = PutStartXMLTag(tk,"Vertex_Normals")) != TK_Normal)
				return status;
			mp_substage++;
		}   nobreak;

        case 2: {
			PutTab t(&tk);
			if ((status = PutAsciiData(tk,"Compression_Scheme", (int)mp_compression_scheme)) != TK_Normal)
                return status;
            mp_substage++;
        }   nobreak;
        
         case 3: {
			 PutTab t(&tk);
            if ((status = PutAsciiData(tk,"Bits_Per_Sample",(int) mp_bits_per_sample)) != TK_Normal)
                return status;
            mp_substage++;
        } nobreak;

		 case 4: {
			 PutTab t(&tk);
			 if ((status = PutAsciiData(tk, "Vertex_Normal_Count", mp_pointcount)) != TK_Normal)
				 return status;
			 mp_substage++;
				 }   nobreak;

		 case 5: {
			 PutTab t(&tk);
			 if( mp_pointcount != 0)
				 if ((status = PutAsciiData(tk, "Vertex_Normals",  mp_normals, 3 * mp_pointcount)) != TK_Normal)
					 return status;
			 mp_substage++;
				 }   nobreak;
		 case 6: {
			 if ((status = PutEndXMLTag(tk,"Vertex_Normals")) != TK_Normal)
				 return status;
			 mp_substage = 0;
		 }   break;

        default:
            return tk.Error("internal error from TK_Polyhedron::write_vertex_normals_compressed_all");
    }
	
    return status;
#else
	return tk.Error(stream_disable_ascii);
#endif 

} //end function write_vertex_normals_compressed_all


TK_Status TK_Polyhedron::write_vertex_normals_compressed_ascii (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII
 
	TK_Status       status = TK_Normal;
	
 	ASSERT(mp_optopcode == OPT_NORMALS_COMPRESSED);
    switch (mp_substage) {
		
		 case 1: {
			if ((status = PutStartXMLTag(tk,"Vertex_Normals")) != TK_Normal)
				return status;
			mp_substage++;
				}   nobreak;

		 case 2: {
			 PutTab t(&tk);
			 if ((status = PutAsciiData(tk, "Vertex_Normal_Count", mp_normalcount)) != TK_Normal)
				 return status;
		  	 mp_substage++;
			}   nobreak;

		 case 3: {
			 PutTab t(&tk);
			 if( mp_normalcount != 0)
				 if ((status = PutAsciiData(tk, "Vertex_Normals",  mp_normals, 3 * mp_normalcount)) != TK_Normal)
					 return status;
			 mp_substage++;
				 }   nobreak;
		 case 4: {

			 if ((status = PutEndXMLTag(tk,"Vertex_Normals")) != TK_Normal)
				 return status;
			 mp_substage = 0;
				 }   break;

            default:
                return tk.Error("internal error from TK_Polyhedron::write_vertex_normals_compressed (version<650)");
        
    }
	
    return status;
#else
	return tk.Error(stream_disable_ascii);
#endif 

} //end function write_vertex_normals_compressed


TK_Status TK_Polyhedron::write_vertex_normals_all_ascii (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII
    
	TK_Status       status = TK_Normal;

    ASSERT(mp_optopcode == OPT_ALL_NORMALS ||
            mp_optopcode == OPT_ALL_NORMALS_POLAR);
    switch (mp_substage) {
		case 1: {
			if ((status = PutStartXMLTag(tk,"Vertex_Normals")) != TK_Normal)
				return status;
			mp_substage++;
				}   nobreak;

        case 2: {
			PutTab t(&tk);
            if (mp_optopcode == OPT_ALL_NORMALS_POLAR) {
                normals_cartesian_to_polar(mp_exists, Vertex_Normal, mp_pointcount, mp_normals, mp_normals);
            }
            mp_substage++;
        } nobreak;
            
        //send all normals (normal count is implicit)
        case 3: {
			PutTab t(&tk);
            if (mp_optopcode == OPT_ALL_NORMALS_POLAR) {
                if ((status = PutAsciiData(tk,"Normals", mp_normals, 2 * mp_pointcount)) != TK_Normal)
                    return status;
            }
            else {
                if ((status = PutAsciiData(tk, "Normals",mp_normals, 3 * mp_pointcount)) != TK_Normal)
                    return status;
            }
			mp_substage++;
				}   nobreak;
		case 4: {
			if ((status = PutEndXMLTag(tk,"Vertex_Normals")) != TK_Normal)
				return status;
			mp_substage = 0;
				}   break;

        default:
            return tk.Error("internal error from TK_Polyhedron::write_vertex_normals_all");
    }    
	
    return status;
#else
	return tk.Error(stream_disable_ascii);
#endif 

}


TK_Status TK_Polyhedron::write_vertex_normals_ascii (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII
    
	TK_Status       status = TK_Normal;
    unsigned char       byte;
    unsigned short      word;

    ASSERT(mp_optopcode == OPT_NORMALS ||
            mp_optopcode == OPT_NORMALS_POLAR);
    switch (mp_substage) {

        //if all points do NOT have normals, tell how many
		case 1: {
			if ((status = PutStartXMLTag(tk,"Vertex_Normals")) != TK_Normal)
				return status;
			mp_substage++;
				}   nobreak;

        case 2: {
			PutTab t(&tk);
            if (mp_pointcount < 256) {
                byte = (unsigned char)mp_normalcount;
                if ((status = PutAsciiData(tk,"Normal_Count", byte)) != TK_Normal)
                    return status;
            }
            else if (mp_pointcount < 65536) {
                word = (unsigned short)mp_normalcount;
                if ((status = PutAsciiData(tk,"Normal_Count", word)) != TK_Normal)
                    return status;
            }
            else {
                if ((status = PutAsciiData(tk,"Normal_Count", mp_normalcount)) != TK_Normal)
                    return status;
            }
            mp_progress = 0;
            mp_substage++;
        }   nobreak;

        //and indices of existing normals
        case 3: {
			PutTab t(&tk);
            while (mp_progress < mp_pointcount) {
                if (mp_exists[mp_progress] & Vertex_Normal) {
                    if (mp_pointcount < 256) {
                        byte = (unsigned char)mp_progress;
                        if ((status = PutAsciiData(tk,"Progress", byte)) != TK_Normal)
                            return status;
                    }
                    else if (mp_pointcount < 65536) {
                        word = (unsigned short)mp_progress;
                        if ((status = PutAsciiData(tk,"Progress", word)) != TK_Normal)
                            return status;
                    }
                    else {
                        if ((status = PutAsciiData(tk,"Progress", mp_progress)) != TK_Normal)
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
        case 4: {
			PutTab t(&tk);
            while (mp_progress < mp_pointcount) {
                if (mp_exists[mp_progress] & Vertex_Normal) {
                    if (mp_optopcode == OPT_NORMALS_POLAR)
                        status = PutAsciiData(tk,"Normals", &mp_normals[2*mp_progress], 2);
                    else
                        status = PutAsciiData(tk,"Normals", &mp_normals[3*mp_progress], 3);
                    if (status != TK_Normal)
                        return status;
                }
                mp_progress++;
            }
            mp_progress = 0;
			mp_substage++;
				}   nobreak;
		case 5: {
			if ((status = PutEndXMLTag(tk,"Vertex_Normals")) != TK_Normal)
				return status;
			mp_substage = 0;
				}   break;

        default:
            return tk.Error("internal error from TK_Polyhedron::write_vertex_normals");
    }
	
    return status;
#else
	return tk.Error(stream_disable_ascii);
#endif 
}

TK_Status TK_Polyhedron::write_vertex_normals_main_ascii (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII

	TK_Status       status = TK_Normal;
	
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
		PutTab t(&tk);
        if ((status = PutAsciiHex(tk,"Optional_Opcode", mp_optopcode)) != TK_Normal)
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
	return tk.Error(stream_disable_ascii);
#endif 

} //end function TK_Polyhedron::write_vertex_normals_main


TK_Status TK_Polyhedron::write_vertex_parameters_all_ascii (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII

	TK_Status       status = TK_Normal;
	
	ASSERT(mp_optopcode == OPT_ALL_PARAMETERS_COMPRESSED ||
            mp_optopcode == OPT_ALL_PARAMETERS);
    if (mp_optopcode == OPT_ALL_PARAMETERS) {
        switch (mp_substage) {

            //case 0: handled external to this switch

            //parameter width

			case 1: {
				if ((status = PutStartXMLTag(tk,"Parameters")) != TK_Normal)
					return status;
				mp_substage++;
					}   nobreak;

            case 2: {
				PutTab t(&tk);
                if ((status = PutAsciiData(tk,"Parameter_Width", (int)mp_paramwidth)) != TK_Normal)
			                return status;
			
                mp_substage++;
            }   nobreak;

            //all parameters 
            case 3: {
				PutTab t(&tk);
                if ((status = PutAsciiData(tk,"All_Parameters", mp_params, ((int)mp_paramwidth) * mp_pointcount)) != TK_Normal)
                    return status;
				mp_substage++;
					}   nobreak;
			
			case 4: {

				if ((status = PutEndXMLTag(tk,"Parameters")) != TK_Normal)
					return status;
				mp_substage = 0;
					}   break;

            default:
                return tk.Error("internal error from TK_Polyhedron::write_vertex_parameters_all (uncompressed)");
		}
    }
    else if (tk.GetTargetVersion() >= 650) {
        switch (mp_substage) {

            //case 0: handled external to this switch

            //compression scheme
			case 1: {
				if ((status = PutStartXMLTag(tk,"Parameters")) != TK_Normal)
					return status;
				mp_substage++;
					}   nobreak;

            case 2: {
				PutTab t(&tk);
                ASSERT(mp_compression_scheme == CS_TRIVIAL);
                if ((status = PutAsciiData(tk,"Compression_Scheme", (int)mp_compression_scheme)) != TK_Normal)
                    return status;
                mp_substage++;
            }   nobreak;

            //parameter width
            case 3: {
				PutTab t(&tk);
            if (tk.GetTargetVersion() >= 1175) {
			//	int int_paramwidth = atoi(&mp_paramwidth);
                if ((status = PutAsciiData(tk,"Parameter_Width", (int)mp_paramwidth)) != TK_Normal)
                    return status;
                _W3DTK_REQUIRE_VERSION( 1175 );
            }
            mp_substage++;
            }   nobreak;
				//all parameters 
			case 4: {
				PutTab t(&tk);
			//	int int_paramwidth = atoi(&mp_paramwidth);
				if ((status = PutAsciiData(tk,"All_Parameters", mp_params, (int)mp_paramwidth * mp_pointcount)) != TK_Normal)
					return status;
				mp_substage++;
					} nobreak;
            //the bounding box (note: order different from ver<650)
            case 5: {
				PutTab t(&tk);
			//	int int_paramwidth = atoi(&mp_paramwidth);
                if ((status = PutAsciiData(tk,"Bounding_Box", mp_bbox, (2*(int)mp_paramwidth))) != TK_Normal)
                    return status;
                mp_substage++;
            } nobreak;

            //compression size
            case 6: {
				PutTab t(&tk);
                if ((status = PutAsciiData(tk,"Bits_Per_Sample",(int) mp_bits_per_sample)) != TK_Normal)
                    return status;
				mp_substage++;
					}   nobreak;
			
			case 7: {
				if ((status = PutEndXMLTag(tk,"Parameters")) != TK_Normal)
					return status;
				mp_substage = 0;
					}   break;
            //the workspace length
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
				if ((status = PutStartXMLTag(tk,"Parameters")) != TK_Normal)
					return status;
				mp_substage++;
					}   nobreak;
            case 2: {
                ASSERT(mp_compression_scheme == CS_TRIVIAL);
                if ((status = PutAsciiData(tk,"Compression_Scheme", (int)mp_compression_scheme)) != TK_Normal)
                    return status;
                mp_substage++;
            }   nobreak;

            //compression size
            case 3: {
                ASSERT(mp_bits_per_sample == 8);
                if ((status = PutAsciiData(tk,"Bits_Per_Sample", (int)mp_bits_per_sample)) != TK_Normal)
                    return status;
                mp_substage++;
            } nobreak;

           //the bounding box (note: order different from ver>=650)
            case 4: {
				int int_paramwidth = atoi(&mp_paramwidth);
                if ((status = PutAsciiData(tk,"Bounding_Box", mp_bbox, 2*int_paramwidth)) != TK_Normal)
                    return status;
                mp_substage++;
            } nobreak;

            //all parameters 
			case 5: {
				int int_paramwidth = atoi(&mp_paramwidth);
				if ((status = PutAsciiData(tk,"All_Parameters", mp_params, int_paramwidth * mp_pointcount)) != TK_Normal)
					return status;
				mp_substage++;
					}   nobreak;
			case 6:{			
				if ((status = PutEndXMLTag(tk,"Parameters")) != TK_Normal)
					return status;
				mp_substage = 0;
					}   break;
            default:
                return tk.Error("internal error from TK_Polyhedron::write_vertex_parameters_all");
        }
    }
	
    return status;
#else
return tk.Error(stream_disable_ascii);
#endif 

} // end function write_vertex_parameters_all


TK_Status TK_Polyhedron::write_vertex_parameters_ascii (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII

	TK_Status       status = TK_Normal;

    ASSERT(mp_optopcode == OPT_PARAMETERS_COMPRESSED ||
            mp_optopcode == OPT_PARAMETERS);
    if (mp_optopcode == OPT_PARAMETERS) {
        switch (mp_substage) {
            //case 0: handled external to this switch

            //number of parameters on each vertex
			case 1: {
				if ((status = PutStartXMLTag(tk,"Parameters")) != TK_Normal)
					return status;
				mp_substage++;
					}   nobreak;

            case 2: {
				PutTab t(&tk);
				int int_paramwidth = atoi(&mp_paramwidth);
                if ((status = PutAsciiData(tk,"Parameter_Width", int_paramwidth)) != TK_Normal)
                    return status;
                mp_substage++;
            } nobreak;

            //if all points do NOT have parameters, tell how many
            case 3: {
				PutTab t(&tk);
                if (mp_pointcount < 256) {
                    unsigned char       byte = (unsigned char)mp_paramcount;
                    if ((status = PutAsciiData(tk,"Parameter_Count", byte)) != TK_Normal)
                        return status;
                }
                else if (mp_pointcount < 65536) {
                    unsigned short      word = (unsigned short)mp_paramcount;

                    if ((status = PutAsciiData(tk,"Parameter_Count", word)) != TK_Normal)
                        return status;
                }
                else {
                    if ((status = PutAsciiData(tk,"Parameter_Count", mp_paramcount)) != TK_Normal)
                        return status;
                }
                mp_progress = 0;
                mp_substage++;
            }   nobreak;

            //and indices of existing parameters
            case 4: {
				PutTab t(&tk);
                while (mp_progress < mp_pointcount) {
                    if (mp_exists[mp_progress] & Vertex_Parameter) {
                        if (mp_pointcount < 256) {
                            unsigned char       byte = (unsigned char)mp_progress;

                            if ((status = PutAsciiData(tk,"Parameter_Indices", byte)) != TK_Normal)
                                return status;
                        }
                        else if (mp_pointcount < 65536) {
                            unsigned short      word = (unsigned short)mp_progress;

                            if ((status = PutAsciiData(tk,"Parameter_Indices", word)) != TK_Normal)
                                return status;
                        }
                        else {
                            if ((status = PutAsciiData(tk,"Parameter_Indices", mp_progress)) != TK_Normal)
                                return status;
                        }
                    }
                    mp_progress++;
                }
                mp_progress = 0;
                mp_substage++;
            } nobreak;

            //write parameter data
            case 5: {
				PutTab t(&tk);
                while (mp_progress < mp_pointcount) {
                    if (mp_exists[mp_progress] & Vertex_Parameter) {
                        status = PutAsciiData(tk, "Parameters",&mp_params[((int)(mp_paramcount))*mp_progress], ((int)(mp_paramcount)));
                        if (status != TK_Normal)
                            return status;
                    }
                    mp_progress++;
                }
                mp_progress = 0;
				mp_substage++;
					}   nobreak;
			
			case 6: {
				if ((status = PutEndXMLTag(tk,"Parameters")) != TK_Normal)
					return status;
				mp_substage = 0;
					}   break;

            default:
                return tk.Error("internal error from TK_Polyhedron::write_vertex_parameters (uncompressed)");
        }
    }
	else if (tk.GetTargetVersion() >= 650) {
        switch (mp_substage) {

            //case 0: handled external to this switch

            //compression scheme
			case 1: {
				if ((status = PutStartXMLTag(tk,"Parameters")) != TK_Normal)
					return status;
				mp_substage++;
					}   nobreak;

            case 2: {
				PutTab t(&tk);
                ASSERT(mp_compression_scheme == CS_TRIVIAL);
                if ((status = PutAsciiData(tk,"Compression_Scheme", (int)mp_compression_scheme)) != TK_Normal)
                    return status;
                mp_substage++;
            }   nobreak;

            //number of parameters on each vertex
            case 3: {
				PutTab t(&tk);
                if (tk.GetTargetVersion() >= 1175) {
					int int_paramwidth = atoi(&mp_paramwidth);
                    if ((status = PutAsciiData(tk,"Parameter_Width",int_paramwidth)) != TK_Normal)
                        return status;
                    _W3DTK_REQUIRE_VERSION( 1175 );
                }
                mp_substage++;
            } nobreak;

            //the bounding box (note: order different from ver<650)
            case 4: {
				PutTab t(&tk);
				int int_paramwidth = atoi(&mp_paramwidth);
				if ((status = PutAsciiData(tk,"Bounding_Box", mp_bbox, 2*int_paramwidth)) != TK_Normal)
                    return status;
                mp_substage++;
            } nobreak;

            //compression size
            case 5: {
				PutTab t(&tk);
                if ((status = PutAsciiData(tk,"Bits_Per_Sample", (int)mp_bits_per_sample)) != TK_Normal)
                    return status;
                mp_substage++;
            } nobreak;

            //if all points do NOT have parameters, tell how many
            case 6: {
				PutTab t(&tk);
                if (mp_pointcount < 256) {
                    unsigned char       byte = (unsigned char)mp_paramcount;

                    if ((status = PutAsciiData(tk,"Parameter_Count", byte)) != TK_Normal)
                        return status;
                }
                else if (mp_pointcount < 65536) {
                    unsigned short      word = (unsigned short)mp_paramcount;

                    if ((status = PutAsciiData(tk,"Parameter_Count", word)) != TK_Normal)
                        return status;
                }
                else {
                    if ((status = PutAsciiData(tk,"Parameter_Count", mp_paramcount)) != TK_Normal)
                        return status;
                }
                mp_progress = 0;
                mp_substage++;
            }   nobreak;

            //and indices of existing parameters
            case 7: {
				PutTab t(&tk);
                while (mp_progress < mp_pointcount) {
                    if (mp_exists[mp_progress] & Vertex_Parameter) {
                        if (mp_pointcount < 256) {
                            unsigned char       byte = (unsigned char)mp_progress;

                            if ((status = PutAsciiData(tk,"Parameter_Indices", byte)) != TK_Normal)
                                return status;
                        }
                        else if (mp_pointcount < 65536) {
                            unsigned short      word = (unsigned short)mp_progress;

                            if ((status = PutAsciiData(tk,"Parameter_Indices", word)) != TK_Normal)
                                return status;
                        }
                        else {
                            if ((status = PutAsciiData(tk,"Parameter_Indices", mp_progress)) != TK_Normal)
                                return status;
                        }
                    }
                    mp_progress++;
                }
                mp_progress = 0;
                mp_substage++;
            } nobreak;

			case 8: {
				PutTab t(&tk);
				int int_paramwidth = atoi(&mp_paramwidth);
				if ((status = PutAsciiData(tk,"All_Parameters", mp_params, int_paramwidth * mp_pointcount)) != TK_Normal)
					return status;
				mp_substage++;
					}   nobreak;

			case 9: {
				if ((status = PutEndXMLTag(tk,"Parameters")) != TK_Normal)
					return status;
				mp_substage = 0;
					}   break;

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
				if ((status = PutStartXMLTag(tk,"Parameters")) != TK_Normal)
					return status;
				mp_substage++;
					}   nobreak;

            case 2: {
				PutTab t(&tk);
                ASSERT(mp_compression_scheme == CS_TRIVIAL);
                if ((status = PutAsciiData(tk,"Compression_Scheme", (int)mp_compression_scheme)) != TK_Normal)
                    return status;
                mp_substage++;
            }   nobreak;

            //compression size
            case 3: {
				PutTab t(&tk);
                if ((status = PutAsciiData(tk,"Bits_Per_Sample", (int)mp_bits_per_sample)) != TK_Normal)
                    return status;
                mp_substage++;
            } nobreak;

            //parameter width
            case 4: {
				PutTab t(&tk);
                if (tk.GetTargetVersion() >= 1175) {
                    if ((status = PutAsciiData(tk,"Parameter_count", mp_paramcount)) != TK_Normal)
                        return status;
                    _W3DTK_REQUIRE_VERSION( 1175 );
                }
                mp_substage++;
            } nobreak;

			//the bounding box (note: order different from ver>=650)
            case 5: {
				PutTab t(&tk);
                if ((status = PutAsciiData(tk,"Bounding_Box", mp_bbox, 2*((int)(mp_paramwidth)))) != TK_Normal)
                    return status;
                mp_substage++;
            } nobreak;

            //if all points do NOT have parameters, tell how many
            case 6: {
				PutTab t(&tk);
                if (mp_pointcount < 256) {
                    unsigned char       byte = (unsigned char)mp_paramcount;

                    if ((status = PutAsciiData(tk,"Parameter_Count", byte)) != TK_Normal)
                        return status;
                }
                else if (mp_pointcount < 65536) {
                    unsigned short      word = (unsigned short)mp_paramcount;

                    if ((status = PutAsciiData(tk,"Parameter_Count", word)) != TK_Normal)
                        return status;
                }
                else {
                    if ((status = PutAsciiData(tk,"Parameter_Count", mp_paramcount)) != TK_Normal)
                        return status;
                }
                mp_progress = 0;
                mp_substage++;
            }   nobreak;

            //and indices of existing parameters
            case 7: {
				PutTab t(&tk);
                while (mp_progress < mp_pointcount) {
                    if (mp_exists[mp_progress] & Vertex_Parameter) {
                        if (mp_pointcount < 256) {
                            unsigned char       byte = (unsigned char)mp_progress;

                            if ((status = PutAsciiData(tk,"Parameter_Indices", byte)) != TK_Normal)
                                return status;
                        }
                        else if (mp_pointcount < 65536) {
                            unsigned short      word = (unsigned short)mp_progress;

                            if ((status = PutAsciiData(tk,"Parameter_Indices", word)) != TK_Normal)
                                return status;
                        }
                        else {
                            if ((status = PutAsciiData(tk,"Parameter_Indices", mp_progress)) != TK_Normal)
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
				PutTab t(&tk);
			int int_paramwidth = atoi(&mp_paramwidth);
			if ((status = PutAsciiData(tk,"All_Parameters", mp_params, int_paramwidth * mp_pointcount)) != TK_Normal)
					return status;
			    mp_progress = 0;
				mp_substage++;
					}   break;

			case 9: {
				if ((status = PutEndXMLTag(tk,"Parameters")) != TK_Normal)
					return status;
			    mp_progress = 0;					
				mp_substage = 0;
					}   break;

            default:
                return tk.Error("internal error from TK_Polyhedron::write_vertex_parameters (ver<650)");
        }
    }
	
    return status;
#else
return tk.Error(stream_disable_ascii);
#endif 

} //end function write_vertex_parameters


TK_Status TK_Polyhedron::write_vertex_parameters_main_ascii (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII

	TK_Status       status = TK_Normal;
	
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

            if ((status = PutAsciiHex(tk,"Optional_Opcode", mp_optopcode)) != TK_Normal)
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
	return tk.Error(stream_disable_ascii);
#endif 

} //end function TK_Polyhedron::write_vertex_parameters_main


TK_Status TK_Polyhedron::write_face_colors_all_ascii (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII

	TK_Status       status = TK_Normal;
	
    if (tk.GetTargetVersion() >= 650) {
        switch (mp_substage) {

            //case 0: handled external to this switch

            //compression scheme
			case 1: {
				if ((status = PutStartXMLTag(tk,"Face_Color")) != TK_Normal)
					return status;
				mp_substage++;
					}   nobreak;

            case 2: {
				PutTab t(&tk);
                ASSERT(mp_compression_scheme == CS_TRIVIAL);
                if ((status = PutAsciiData(tk,"Compression_Scheme", (int)mp_compression_scheme)) != TK_Normal)
                    return status;
                mp_substage++;
            }   nobreak;

            case 3: {
				PutTab t(&tk);
                if ((status = PutAsciiData(tk,"Bits_Per_Sample", (int)mp_bits_per_sample)) != TK_Normal)
                    return status;
                mp_substage++;
            } nobreak;

            //the workspace length
			case 4: {
				PutTab t(&tk);
				if ((status = PutAsciiData(tk, "Face_Count", mp_facecount)) != TK_Normal)
					return status;
			
				mp_substage++;
					}   nobreak;

			case 5: {
				PutTab t(&tk);
				if( mp_facecount != 0)
					if ((status = PutAsciiData(tk, "Face_Color",  mp_fcolors, 3 * mp_facecount)) != TK_Normal)
						return status;
				mp_substage++;
					}   nobreak;
			
			case 6: {
				if ((status = PutEndXMLTag(tk,"Face_Color")) != TK_Normal)
					return status;
				mp_substage = 0;
					}   break;

            default:
                return tk.Error("internal error in function TK_Polyhedron::write_face_colors (all)");
        }
    }
    else {
        switch (mp_substage) {

            //case 0: handled external to this switch

           //all colors
			case 1: {
				if ((status = PutStartXMLTag(tk,"Face_Color")) != TK_Normal)
					return status;
				mp_substage++;
					}   nobreak;

			case 2: {
				PutTab t(&tk);
				if( mp_facecount != 0)
					if ((status = PutAsciiData(tk, "Face_Color",  mp_fcolors, 3 * mp_facecount)) != TK_Normal)
						return status;
				mp_substage++;
					}   nobreak;
			case 6: {

				if ((status = PutEndXMLTag(tk,"Face_Color")) != TK_Normal)
					return status;
				mp_substage = 0;
					}   break;

           default:
                return tk.Error("internal error in function TK_Polyhedron::write_face_colors (all, version<650)");
        }
    }
	
    return status;
#else
	return tk.Error(stream_disable_ascii);
#endif 

} //end function TK_Polyhedron::write_face_colors_all


TK_Status TK_Polyhedron::write_face_colors_ascii (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII

	TK_Status       status = TK_Normal;
    int             count;
	
    if (tk.GetTargetVersion() >= 650) {
        switch (mp_substage) {

            //case 0: handled external to this switch

            //compression scheme
			case 1: {
				if ((status = PutStartXMLTag(tk,"Face_Color")) != TK_Normal)
					return status;
				mp_substage++;
					}   nobreak;

            case 2: {
				PutTab t(&tk);
                ASSERT(mp_compression_scheme == CS_TRIVIAL);
                if ((status = PutAsciiData(tk,"Compression_Scheme", (int)mp_compression_scheme)) != TK_Normal)
                    return status;
                mp_substage++;
            }   nobreak;

            //compression size
            case 3: {
				PutTab t(&tk);
                if ((status = PutAsciiData(tk,"Bits_Per_Sample", (int)mp_bits_per_sample)) != TK_Normal)
                    return status;
                mp_substage++;
            } nobreak;

            //if all faces do NOT have colors, tell how many
            case 4: {
				PutTab t(&tk);
                count = mp_fcolorcount;
                if (mp_facecount < 256) {
                    unsigned char       byte = (unsigned char)count;

                    if ((status = PutAsciiData(tk,"Count", byte)) != TK_Normal)
                        return status;
                }
                else if (mp_facecount < 65536) {
                    unsigned short      word = (unsigned short)count;

                    if ((status = PutAsciiData(tk,"Count", word)) != TK_Normal)
                        return status;
                }
                else {
                    if ((status = PutAsciiData(tk,"Count", count)) != TK_Normal)
                        return status;
                }
                mp_progress = 0;
                mp_substage++;
            }   nobreak;

            //and indices of existing colors
            case 5: {
				PutTab t(&tk);
                while (mp_progress < mp_facecount) {
                    if ((mp_face_exists[mp_progress] & Face_Color) == Face_Color) {
                        if (mp_facecount < 256) {
                            unsigned char       byte = (unsigned char)mp_progress;

                            if ((status = PutAsciiData(tk,"Progress", byte)) != TK_Normal)
                                return status;
                        }
                        else if (mp_facecount < 65536) {
                            unsigned short      word = (unsigned short)mp_progress;

                            if ((status = PutAsciiData(tk,"Progress", word)) != TK_Normal)
                                return status;
                        }
                        else {
                            if ((status = PutAsciiData(tk,"Progress", mp_progress)) != TK_Normal)
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
				PutTab t(&tk);
				if ((status = PutAsciiData(tk, "Face_Count", mp_facecount)) != TK_Normal)
					return status;
			
				mp_substage++;
					}   nobreak;

			case 7: {
				PutTab t(&tk);
				if( mp_facecount != 0)
					if ((status = PutAsciiData(tk, "Face_Color",  mp_fcolors, 3 * mp_facecount)) != TK_Normal)
						return status;
				mp_substage++;
				}   nobreak;
			
			case 8: {
				if ((status = PutEndXMLTag(tk,"Face_Color")) != TK_Normal)
					return status;
				mp_substage = 0;
					}   break;

            default:
                return tk.Error("internal error in function TK_Polyhedron::write_face_colors (partial, version>650)");
        }
    }
    else {
        switch (mp_substage) {

            //case 0: handled external to this switch

            //if all faces do NOT have colors, tell how many
			case 1: {
				if ((status = PutStartXMLTag(tk,"Face_Color")) != TK_Normal)
					return status;
				mp_substage++;
					}   nobreak;

            case 2: {
				PutTab t(&tk);
                count = mp_fcolorcount;
                if (mp_facecount < 256) {
                    unsigned char       byte = (unsigned char)count;

                    if ((status = PutAsciiData(tk,"Count", byte)) != TK_Normal)
                        return status;
                }
                else if (mp_facecount < 65536) {
                    unsigned short      word = (unsigned short)count;

                    if ((status = PutAsciiData(tk,"Count", word)) != TK_Normal)
                        return status;
                }
                else {
                    if ((status = PutAsciiData(tk,"Count", count)) != TK_Normal)
                        return status;
                }
                mp_progress = 0;
                mp_substage++;
            }   nobreak;

            //and colors of existing colors
            case 3: {
				PutTab t(&tk);
                while (mp_progress < mp_facecount) {
                    if ((mp_face_exists[mp_progress] & Face_Color) == Face_Color) {
                        if (mp_facecount < 256) {
                            unsigned char       byte = (unsigned char)mp_progress;

                            if ((status = PutAsciiData(tk,"Progress", byte)) != TK_Normal)
                                return status;
                        }
                        else if (mp_facecount < 65536) {
                            unsigned short      word = (unsigned short)mp_progress;

                            if ((status = PutAsciiData(tk,"Progress", word)) != TK_Normal)
                                return status;
                        }
                        else {
                            if ((status = PutAsciiData(tk,"Progress", mp_progress)) != TK_Normal)
                                return status;
                        }
                    }
                    mp_progress++;
                }
                mp_progress = 0;
                mp_substage++;
            } nobreak;
          
          
            //the individual colors
            case 4: {
				PutTab t(&tk);
			if( mp_facecount != 0)
					if ((status = PutAsciiData(tk, "Face_Color",  mp_fcolors, 3 * mp_facecount)) != TK_Normal)
						return status;
			
                mp_progress = 0;
				mp_substage++;
					}   nobreak;

			case 5: {
				if ((status = PutEndXMLTag(tk,"Face_Color")) != TK_Normal)
					return status;
                mp_progress = 0;
				mp_substage = 0;
					}   break;

            default:
                return tk.Error("internal error in function TK_Polyhedron::write_face_colors");
        }
    }
	
    return status;
#else
return tk.Error(stream_disable_ascii);
#endif 
} //end function TK_Polyhedron::write_face_colors


TK_Status TK_Polyhedron::write_face_colors_main_ascii (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII

	TK_Status       status = TK_Normal;

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

        if ((status = PutAsciiHex(tk,"Optional_Opcode", mp_optopcode)) != TK_Normal)
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
	
    return status;
#else
	return tk.Error(stream_disable_ascii);
#endif 
} //end function TK_Polyhedron::write_face_colors_main


TK_Status TK_Polyhedron::write_face_indices_all_ascii (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII

	TK_Status       status = TK_Normal;

	if (tk.GetTargetVersion() >= 650) {
        switch (mp_substage) {

            //case 0: handled external to this switch
            //compression scheme

			case 1: {
				PutTab t(&tk);
				if ((status = PutStartXMLTag(tk,"Face_Indices")) != TK_Normal)
					return status;
				mp_substage++;
					}   nobreak;


            case 2: {
				PutTab t(&tk);
                ASSERT(mp_compression_scheme == CS_TRIVIAL);
                if ((status = PutAsciiData(tk,"Compression_Scheme", (int)mp_compression_scheme)) != TK_Normal)
                    return status;
                mp_substage++;
            }   nobreak;

          //bounding box
            case 3: {
				PutTab t(&tk);
                if ((status = PutAsciiData(tk,"Bounding_Box", mp_bbox, 6)) != TK_Normal)
                    return status;
                mp_substage++;
            } nobreak;

            //compression size
            case 4: {
				PutTab t(&tk);
                if ((status = PutAsciiData(tk,"Bits_Per_Sample", (int)mp_bits_per_sample)) != TK_Normal)
                    return status;
                mp_substage++;
            } nobreak;
			
			case 5: {
				PutTab t(&tk);
				if ((status = PutAsciiData(tk, "Face_Count", mp_facecount)) != TK_Normal)
					return status;
			
				mp_substage++;
					}   nobreak;

			case 6: {
				PutTab t(&tk);
				if( mp_facecount != 0)
					if ((status = PutAsciiData(tk, "Face_Colors_by_Index", mp_findices,  mp_facecount)) != TK_Normal)
						return status;
				mp_substage++;
					}   nobreak;

			case 7: {

				if ((status = PutEndXMLTag(tk,"Face_Indices")) != TK_Normal)
					return status;
				mp_substage = 0;
					}   break;

            default:
                return tk.Error("internal error in function TK_Polyhedron::write_face_indices (all)");
        }
    }
    else {
        switch (mp_substage) {

            //case 0: handled external to this switch

            //all indices
		
			case 1: {
				if ((status = PutStartXMLTag(tk,"Face_Indices")) != TK_Normal)
					return status;
				mp_substage++;
					}   nobreak;

            case 2: {
				PutTab t(&tk);
                if ((status = PutAsciiData(tk,"Face_Colors_by_Index", mp_findices, mp_facecount)) != TK_Normal)
                    return status;
				mp_substage++;
					}   nobreak;

			case 3: {
				if ((status = PutEndXMLTag(tk,"Face_Indices")) != TK_Normal)
					return status;
				mp_substage = 0;
					}   break;
            default:
                return tk.Error("internal error in function TK_Polyhedron::write_face_indices (all, version<650)");
        }
    }
	
    return status;
#else
	return tk.Error(stream_disable_ascii);
#endif 
} //end function TK_Polyhedron::write_face_indices_all


TK_Status TK_Polyhedron::write_face_indices_ascii (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII

	TK_Status       status = TK_Normal;
	
    int             count;
    if (tk.GetTargetVersion() >= 650) {
        switch (mp_substage) {

            //case 0: handled external to this switch

            //compression scheme
			case 1: {
				if ((status = PutStartXMLTag(tk,"Face_Indices")) != TK_Normal)
					return status;
				mp_substage++;
					}   nobreak;

            case 2: {
				PutTab t(&tk);
                ASSERT(mp_compression_scheme == CS_TRIVIAL);
                if ((status = PutAsciiData(tk,"Compression_Scheme", (int)mp_compression_scheme)) != TK_Normal)
                    return status;
                mp_substage++;
            }   nobreak;

             //compression size
            case 3: {
				PutTab t(&tk);
                if ((status = PutAsciiData(tk,"Bounding_Box", mp_bbox, 6)) != TK_Normal)
                    return status;
                mp_substage++;
            } nobreak;

            //compression size
            case 4: {
				PutTab t(&tk);
                if ((status = PutAsciiData(tk,"Bits_Per_Sample",(int) mp_bits_per_sample)) != TK_Normal)
                    return status;
                mp_substage++;
            } nobreak;

            //if all faces do NOT have indices, tell how many
            case 5: {
				PutTab t(&tk);
                count = mp_findexcount;
                if (mp_facecount < 256) {
                    unsigned char       byte = (unsigned char)count;

                    if ((status = PutAsciiData(tk,"Count", byte)) != TK_Normal)
                        return status;
                }
                else if (mp_facecount < 65536) {
                    unsigned short      word = (unsigned short)count;

                    if ((status = PutAsciiData(tk,"Count", word)) != TK_Normal)
                        return status;
                }
                else {
                    if ((status = PutAsciiData(tk,"Count", count)) != TK_Normal)
                        return status;
                }
                mp_progress = 0;
                mp_substage++;
            }   nobreak;

            //and indices of existing indices
            case 6: {
				PutTab t(&tk);
                while (mp_progress < mp_facecount) {
                    if ((mp_face_exists[mp_progress] & Face_Index) == Face_Index) {
                        if (mp_facecount < 256) {
                            unsigned char       byte = (unsigned char)mp_progress;

                            if ((status = PutAsciiData(tk,"Progress", byte)) != TK_Normal)
                                return status;
                        }
                        else if (mp_facecount < 65536) {
                            unsigned short      word = (unsigned short)mp_progress;

                            if ((status = PutAsciiData(tk,"Progress", word)) != TK_Normal)
                                return status;
                        }
                        else {
                            if ((status = PutAsciiData(tk,"Progress", mp_progress)) != TK_Normal)
                                return status;
                        }
                    }
                    mp_progress++;
                }
                mp_progress = 0;
                mp_substage++;
            } nobreak;

			case 7: {
				PutTab t(&tk);
				if ((status = PutAsciiData(tk, "Face_Count", mp_pointcount)) != TK_Normal)
					return status;
			
				mp_substage++;
					}   nobreak;

			case 8: {
				PutTab t(&tk);
				if( mp_pointcount != 0)
					if ((status = PutAsciiData(tk, "Face_Colors_by_Index", mp_normals, 3 * mp_pointcount)) != TK_Normal)
						return status;
				mp_substage++;
					} nobreak;
			case 9: {
				if ((status = PutEndXMLTag(tk,"Face_Indices")) != TK_Normal)
					return status;
				mp_substage = 0;
					}   break;
            default:
                return tk.Error("internal error in function TK_Polyhedron::write_face_indices (partial, version>650)");
        }
    }
    else {
        switch (mp_substage) {

            //case 0: handled external to this switch

            //if all faces do NOT have indices, tell how many
			case 1: {
				if ((status = PutStartXMLTag(tk,"Face_Indices")) != TK_Normal)
					return status;
				mp_substage++;
					}   nobreak;

            case 2: {
				PutTab t(&tk);
                count = mp_findexcount;
                if (mp_facecount < 256) {
                    unsigned char       byte = (unsigned char)count;

                    if ((status = PutAsciiData(tk,"Count", byte)) != TK_Normal)
                        return status;
                }
                else if (mp_facecount < 65536) {
                    unsigned short      word = (unsigned short)count;

                    if ((status = PutAsciiData(tk,"Count", word)) != TK_Normal)
                        return status;
                }
                else {
                    if ((status = PutAsciiData(tk,"Count", count)) != TK_Normal)
                        return status;
                }
                mp_progress = 0;
                mp_substage++;
            }   nobreak;

            //and indices of existing indices
            case 3: {
				PutTab t(&tk);
                while (mp_progress < mp_facecount) {
                    if ((mp_face_exists[mp_progress] & Face_Index) == Face_Index) {
                        if (mp_facecount < 256) {
                            unsigned char       byte = (unsigned char)mp_progress;

                            if ((status = PutAsciiData(tk,"Progress", byte)) != TK_Normal)
                                return status;
                        }
                        else if (mp_facecount < 65536) {
                            unsigned short      word = (unsigned short)mp_progress;

                            if ((status = PutAsciiData(tk,"Progress", word)) != TK_Normal)
                                return status;
                        }
                        else {
                            if ((status = PutAsciiData(tk,"Progress", mp_progress)) != TK_Normal)
                                return status;
                        }
                    }
                    mp_progress++;
                }
                mp_progress = 0;
                mp_substage++;
            } nobreak;

            //the individual indices
            case 4: {
				PutTab t(&tk);
                while (mp_progress < mp_facecount) {
                    if ((mp_face_exists[mp_progress] & Face_Index) == Face_Index) {
                        if ((status = PutAsciiData(tk,"Face_Colors_by_Index", &mp_findices[mp_progress], 1)) != TK_Normal)
                            return status;
                    }
                    mp_progress++;
                }
                mp_progress = 0;
				mp_substage++;
					} nobreak;
			case 5: {

				if ((status = PutEndXMLTag(tk,"Face_Indices")) != TK_Normal)
					return status;
				mp_substage = 0;
					}   break;

            default:
                return tk.Error("internal error in function TK_Polyhedron::write_face_indices");
        }
    }
	
    return status;
#else
return tk.Error(stream_disable_ascii);
#endif 
} //end function TK_Polyhedron::write_face_indices


TK_Status TK_Polyhedron::write_face_indices_main_ascii (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII

	TK_Status       status = TK_Normal;

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

        if ((status = PutAsciiHex(tk,"Optional_Opcode", mp_optopcode)) != TK_Normal)
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
	return tk.Error(stream_disable_ascii);
#endif 
} //end function TK_Polyhedron::write_face_indices_main

TK_Status TK_Polyhedron::write_face_regions_ascii (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII

	TK_Status       status = TK_Normal;

	if (tk.GetTargetVersion() < 705)
        return TK_Normal;

    switch (mp_substage) {
		case 1: {
			if ((status = PutStartXMLTag(tk,"Face_Regions")) != TK_Normal)
				return status;
			mp_substage++;
				}   nobreak;

		case 2: {
			PutTab t(&tk);
			mp_optopcode = OPT_FACE_REGIONS;
			if ((status = PutAsciiHex(tk,"Optional_Opcode", mp_optopcode)) != TK_Normal)
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
			mp_substage++;
				}   nobreak;

        case 3: {
			PutTab t(&tk);
            if ((status = PutAsciiData(tk,"Compression_Scheme", (int)mp_compression_scheme)) != TK_Normal)
                return status;

			if( mp_facecount != 0)
				ENSURE_ALLOCATED (mp_fregions, int, mp_facecount);

            mp_substage++;
        }   nobreak;

		case 4: {
			if( mp_facecount != 0 )
			{
				PutTab t(&tk);
				if ((status = PutAsciiData(tk, "Face_Regions",  mp_fregions, mp_facecount)) != TK_Normal)
						return status;
			}
			mp_substage++;
		} nobreak;

		case 5: {
			if ((status = PutEndXMLTag(tk,"Face_Indices")) != TK_Normal)
				return status;
			mp_substage = 0;
				}   break;

        default:
            return tk.Error ("internal error in TK_Polyhedron::write_face_regions");
    }
	
    return status;
#else
	return tk.Error(stream_disable_ascii);
#endif 
} //end function TK_Polyhedron::write_face_regions



TK_Status TK_Polyhedron::write_vertex_colors_all_ascii (BStreamFileToolkit & tk, unsigned char op) alter {
#ifndef BSTREAM_DISABLE_ASCII
   
	TK_Status       status = TK_Normal;
    float           *array;
    unsigned int    mask;
    char			tag1[256];

	switch (op) {
        case OPT_ALL_VFCOLORS:
            array = mp_vfcolors;
            mask = Vertex_Face_Color;
			strcpy(tag1,"Vertex_Face_Color");
            break;
        case OPT_ALL_VECOLORS:
            array = mp_vecolors;
            mask = Vertex_Edge_Color;
			strcpy(tag1,"Vertex_Edge_Color");
            break;
        case OPT_ALL_VMCOLORS:
            array = mp_vmcolors;
            mask = Vertex_Marker_Color;
			strcpy(tag1,"Vertex_Marker_Color");
            break;
        default:
            return tk.Error("internal error in write_vertex_colors_all: unrecognized case");
    }

    if (tk.GetTargetVersion() >= 650) {
        switch (mp_substage) {
            //compression scheme
			
			case 1: {
				if ((status = PutStartXMLTag(tk,"Vertex_Color")) != TK_Normal)
					return status;
				mp_substage++;
					}   nobreak;
            case 2: {
				PutTab t(&tk);
                ASSERT(mp_compression_scheme == CS_TRIVIAL);
                if ((status = PutAsciiData(tk,"Compression_Scheme", (int)mp_compression_scheme)) != TK_Normal)
                    return status;
                mp_substage++;
            }   nobreak;

            //bits per sample
            case 3: {
				PutTab t(&tk);
                if ((status = PutAsciiData(tk,"Bits_Per_Sample", (int)mp_bits_per_sample)) != TK_Normal)
                    return status;
                mp_substage++;
            } nobreak;

            //workspace length
			case 4: {
				PutTab t(&tk);
				if ((status = PutAsciiData(tk, "Point_Count", mp_pointcount)) != TK_Normal)
					return status;
				mp_substage++;
					}   nobreak;

			case 5: {
				PutTab t(&tk);
				if( mp_pointcount != 0)
					if ((status = PutAsciiData(tk, tag1,  array,  3*mp_pointcount)) != TK_Normal)
						return status;
				mp_substage++;
					} nobreak;
			case 6: {

				if ((status = PutEndXMLTag(tk,"Vertex_Color")) != TK_Normal)
					return status;
				mp_substage = 0;
					}   break;
            default:
                return tk.Error("internal error in write_vertex_colors_all");
        }
    }
    else {
        switch (mp_substage) {
			
			case 1: {
				if ((status = PutStartXMLTag(tk,"Vertex_Color")) != TK_Normal)
					return status;
				mp_substage = 0;
					}   break;

         	case 2: {
				PutTab t(&tk);
				if( mp_pointcount != 0)
					if ((status = PutAsciiData(tk, tag1,  array, 3 * mp_pointcount)) != TK_Normal)
						return status;
				mp_substage++;
					} nobreak;
			case 3: {

				if ((status = PutEndXMLTag(tk,"Vertex_Color")) != TK_Normal)
					return status;
				mp_substage = 0;
					}   break;
            default:
                return tk.Error("internal error in write_vertex_colors_all (version<650)");
        }

    }
	
    return status;
#else
	UNREFERENCED (op);
	return tk.Error(stream_disable_ascii);
#endif 
} //end function TK_Polyhedron::write_vertex_colors_all


TK_Status TK_Polyhedron::write_vertex_colors_ascii (BStreamFileToolkit & tk, unsigned char op) alter {
#ifndef BSTREAM_DISABLE_ASCII
    
	TK_Status       status = TK_Normal;
    int             count;
    float           *array;
    unsigned int    mask;
    char			tag1[1024];
    char			tag2[1024];
	
    switch (op) {
        case OPT_VERTEX_FCOLORS:
            count = mp_vfcolorcount;
            array = mp_vfcolors;
            mask = Vertex_Face_Color;
			strcpy(tag1, "Vertex_Face_Color");
			strcpy(tag2, "Vertex_Face_Color_count");
            break;
        case OPT_VERTEX_ECOLORS:
            count = mp_vecolorcount;
            array = mp_vecolors;
            mask = Vertex_Edge_Color;
			strcpy(tag1, "Vertex_Edge_Color");
			strcpy(tag2, "Vertex_Edge_Color_Count");
            break;
        case OPT_VERTEX_MCOLORS:
            count = mp_vmcolorcount;
            array = mp_vmcolors;
            mask = Vertex_Marker_Color;
			strcpy(tag1, "Vertex_Marker_Color");
			strcpy(tag2, "Vertex_Marker_Color_Count");
            break;
        default:
            return tk.Error("internal error in write_vertex_colors: unrecognized case");
    }

    if (tk.GetTargetVersion() >= 650) {
        switch (mp_substage) {
            //compression scheme
			case 1: {
				if ((status = PutStartXMLTag(tk,"Vertex_Marker_Color")) != TK_Normal)
					return status;
				mp_substage++;
					}   nobreak;

            case 2: {
				PutTab t(&tk);
                ASSERT(mp_compression_scheme == CS_TRIVIAL);
                if ((status = PutAsciiData(tk,"Compression_Scheme", (int)mp_compression_scheme)) != TK_Normal)
                    return status;
                mp_substage++;
            }   nobreak;

            //bits per sample
            case 3: {
				PutTab t(&tk);
                if ((status = PutAsciiData(tk,"Bits_Per_Sample", (int)mp_bits_per_sample)) != TK_Normal)
                    return status;
                mp_substage++;
            } nobreak;

            //the count
            case 4: {
				PutTab t(&tk);
                if (mp_pointcount < 256) {
                    unsigned char       byte = (unsigned char)count;

                    if ((status = PutAsciiData(tk,"Count", byte)) != TK_Normal)
                        return status;
                }
                else if (mp_pointcount < 65536) {
                    unsigned short      word = (unsigned short)count;

                    if ((status = PutAsciiData(tk,"Count", word)) != TK_Normal)
                        return status;
                }
                else {
                    if ((status = PutAsciiData(tk,"Count", count)) != TK_Normal)
                        return status;
                }
                mp_progress = 0;
                mp_substage++;
            }   nobreak;

            //and indices of existing values
            case 5: {
				PutTab t(&tk);
                while (mp_progress < mp_pointcount) {
                    if ((mp_exists[mp_progress] & mask) == mask) {
                        if (mp_pointcount < 256) {
                        unsigned char       byte = (unsigned char)mp_progress;

                        if ((status = PutAsciiData(tk,"Progress", byte)) != TK_Normal)
                            return status;
                    }
                    else if (mp_pointcount < 65536) {
                        unsigned short      word = (unsigned short)mp_progress;

                        if ((status = PutAsciiData(tk,"Progress", word)) != TK_Normal)
                            return status;
                    }
                    else {
                        if ((status = PutAsciiData(tk,"Progress", mp_progress)) != TK_Normal)
                            return status;
                        }
                    }
                    mp_progress++;
                }
                mp_progress = 0;
                mp_substage++;
            } nobreak;

        	case 6: {
				PutTab t(&tk);
				if ((status = PutAsciiData(tk, tag2, count)) != TK_Normal)
					return status;
			
				mp_substage++;
					}   nobreak;

			case 7: {
				PutTab t(&tk);
				if( count != 0)
					if ((status = PutAsciiData(tk, tag1,  array, 3*count)) != TK_Normal)
						return status;
				mp_substage = 0;
					}   break;
			case 8: {
				if ((status = PutEndXMLTag(tk,"Vertex_Marker_Color")) != TK_Normal)
					return status;
				mp_substage = 0;
					}   break;

			default:
				return tk.Error("internal error in TK_Polyhedron::write_vertex_colors");
		}
	}
    else {
        switch (mp_substage) {
            //the count

			case 1: {
				if ((status = PutStartXMLTag(tk,"Vertex_Marker_Color")) != TK_Normal)
					return status;
				mp_substage++;
					}   nobreak;

            case 2: {
				PutTab t(&tk);
                if (mp_pointcount < 256) {
                    unsigned char       byte = (unsigned char)count;

                    if ((status = PutAsciiData(tk,"Count", byte)) != TK_Normal)
                        return status;
                }
                else if (mp_pointcount < 65536) {
                    unsigned short      word = (unsigned short)count;

                    if ((status = PutAsciiData(tk,"Count", word)) != TK_Normal)
                        return status;
                }
                else {
                    if ((status = PutAsciiData(tk,"Count", count)) != TK_Normal)
                        return status;
                }
                mp_progress = 0;
                mp_substage++;
            }   nobreak;

            //and indices of existing values
            case 3: {
				PutTab t(&tk);
                while (mp_progress < mp_pointcount) {
                    if ((mp_exists[mp_progress] & mask) == mask) {
                        if (mp_pointcount < 256) {
                            unsigned char       byte = (unsigned char)mp_progress;

                            if ((status = PutAsciiData(tk,"Progress", byte)) != TK_Normal)
                                return status;
                        }
                        else if (mp_pointcount < 65536) {
                            unsigned short      word = (unsigned short)mp_progress;

                            if ((status = PutAsciiData(tk,"Progress", word)) != TK_Normal)
                                return status;
                        }
                        else {
                            if ((status = PutAsciiData(tk,"Progress", mp_progress)) != TK_Normal)
                                return status;
                        }
                    }
                    mp_progress++;
                }
                mp_progress = 0;
                mp_substage++;
            } nobreak;

           
            //all colors OR individual colors
            case 4: {
				PutTab t(&tk);
				 while (mp_progress < mp_pointcount) {
                    if ((mp_exists[mp_progress] & mask) == mask) {

				/*while (mp_progress < mp_pointcount) {
					if ((mp_exists[mp_progress] & mask) == mask) {
						if( mp_pointcount != 0)*/
					if ((status = PutAsciiData(tk, tag1, &array[3*mp_progress], 3)) != TK_Normal)
						return status;
				    }
                    mp_progress++;
                }
                mp_progress = 0;
                mp_substage = 0;
            } break;
			
			case 5: {
				if ((status = PutEndXMLTag(tk,"Vertex_Marker_Color")) != TK_Normal)
					return status;
				mp_substage = 0;
					}   break;

            default:
                return tk.Error("internal error in TK_Polyhedron::write_vertex_colors (version<650)");
        
	  }
   
	}
	
	return status;
#else
UNREFERENCED (op);
	return tk.Error(stream_disable_ascii);
#endif 
} //end function TK_Polyhedron::write_vertex_colors

TK_Status TK_Polyhedron::write_vertex_indices_all_ascii (BStreamFileToolkit & tk, unsigned char op) alter {
#ifndef BSTREAM_DISABLE_ASCII
   
	TK_Status       status = TK_Normal;
    float           *array;
    unsigned int    mask;
	char            tag1[256];
	
	switch (op) {
		case OPT_ALL_VFINDICES:
			array = mp_vfindices;
			mask = Vertex_Face_Index;
			strcpy(tag1,"Vertex_Face_Index");
			break;
		case OPT_ALL_VEINDICES:
			array = mp_veindices;
			mask = Vertex_Edge_Index;
			strcpy(tag1,"Vertex_Edge_Index");
			break;
		case OPT_ALL_VMINDICES:
			array = mp_vmindices;
			mask = Vertex_Marker_Index;
			strcpy(tag1,"Vertex_Marker_Index");

			break;
		default:
			return tk.Error("internal error in write_vertex_indices_all: unrecognized case");
	}

    if (tk.GetTargetVersion() >= 650) {
        switch (mp_substage) {
            //compression scheme
			case 1: {
				if ((status = PutStartXMLTag(tk,"Vertex_Indices")) != TK_Normal)
					return status;
				mp_substage++;
					}   nobreak;

            case 2: {
				PutTab t(&tk);
                ASSERT(mp_compression_scheme == CS_TRIVIAL);
                if ((status = PutAsciiData(tk,"Compression_Scheme", (int)mp_compression_scheme)) != TK_Normal)
                    return status;
                mp_substage++;
            }   nobreak;

  		//bits per sample
            case 3: {
				PutTab t(&tk);
                if ((status = PutAsciiData(tk,"Bounding_Box", mp_bbox, 6)) != TK_Normal)
                    return status;
                mp_substage++;
            } nobreak;
           //bits per sample
            case 4: {
				PutTab t(&tk);
                if ((status = PutAsciiData(tk,"Bits_Per_Sample",(int) mp_bits_per_sample)) != TK_Normal)
                    return status;
                mp_substage++;
            } nobreak;

            //workspace length
			case 5: {
				PutTab t(&tk);
				if ((status = PutAsciiData(tk, "Point_Count", mp_pointcount)) != TK_Normal)
					return status;
				mp_substage++;
					}   nobreak;

			case 6: {
				PutTab t(&tk);
				if( mp_pointcount != 0)
					if ((status = PutAsciiData(tk, tag1,  array,  mp_pointcount)) != TK_Normal)
						return status;
				mp_substage++;
					}   nobreak;
			case 7: {
				if ((status = PutEndXMLTag(tk,"Vertex_Indices")) != TK_Normal)
					return status;
				mp_substage = 0;
					}   break;
            default:
                return tk.Error("internal error in write_vertex_indices_all");
        }
    }
    else {
        switch (mp_substage) {
            //all indices
			case 1: {
				if ((status = PutStartXMLTag(tk,"Vertex_Indices")) != TK_Normal)
					return status;
				mp_substage++;
					}   nobreak;

            case 2: {
				PutTab t(&tk);
                if ((status = PutAsciiData(tk,"Index_Array", array, mp_pointcount)) != TK_Normal)
                    return status;
				mp_substage++;
					}   nobreak;
			case 3: {
				if ((status = PutEndXMLTag(tk,"Vertex_Indices")) != TK_Normal)
					return status;
				mp_substage = 0;
					}   break;
            default:
                return tk.Error("internal error in write_vertex_indices_all (version<650)");
        }

    }
	
    return status;
#else
	UNREFERENCED (op);
	return tk.Error(stream_disable_ascii);
#endif 
} //end function TK_Polyhedron::write_vertex_indices_all


TK_Status TK_Polyhedron::write_vertex_indices_ascii (BStreamFileToolkit & tk, unsigned char op) alter {
#ifndef BSTREAM_DISABLE_ASCII

	TK_Status       status = TK_Normal;
    int             count;
    float           *array;
	char            tag1[256];
	char            tag2[256];
	unsigned int    mask;
	
	switch (op) {
        case OPT_VERTEX_FINDICES:
            count = mp_vfindexcount;
            array = mp_vfindices;
            mask = Vertex_Face_Index;
			strcpy(tag1,"Vertex_Face_Index");
			strcpy(tag2,"Vertex_Face_Index_Count");
            break;
        case OPT_VERTEX_EINDICES:
            count = mp_veindexcount;
            array = mp_veindices;
            mask = Vertex_Edge_Index;
			strcpy(tag1,"Vertex_Edge_Index");
			strcpy(tag2,"Vertex_Edge_Index_Count");
            break;
        case OPT_VERTEX_MINDICES:
            count = mp_vmindexcount;
            array = mp_vmindices;
            mask = Vertex_Marker_Index;
			strcpy(tag1,"Vertex_Marker_Index");
			strcpy(tag2,"Vertex_Marker_Index_Count");
            break;
        default:
            return tk.Error("internal error in write_vertex_indices: unrecognized case");
    }

    if (tk.GetTargetVersion() >= 650) {
        switch (mp_substage) {
            //compression scheme
			case 1: {
				if ((status = PutStartXMLTag(tk,"Vertex_Indices")) != TK_Normal)
					return status;
				mp_substage++;
					}   nobreak;

            case 2: {
				PutTab t(&tk);
                ASSERT(mp_compression_scheme == CS_TRIVIAL);
                if ((status = PutAsciiData(tk,"Compression_Scheme", (int)mp_compression_scheme)) != TK_Normal)
                    return status;
                mp_substage++;
            }   nobreak;

            case 3: {
				PutTab t(&tk);
                if ((status = PutAsciiData(tk,"Bounding_Box", mp_bbox, 6)) != TK_Normal)
                    return status;
                mp_substage++;
            } nobreak;

            //bits per sample
            case 4: {
				PutTab t(&tk);
                if ((status = PutAsciiData(tk,"Bits_Per_Sample", (int)mp_bits_per_sample)) != TK_Normal)
                    return status;
                mp_substage++;
            } nobreak;

            //the count
            case 5: {
				PutTab t(&tk);
                if (mp_pointcount < 256) {
                    unsigned char       byte = (unsigned char)count;

                    if ((status = PutAsciiData(tk,"Count", byte)) != TK_Normal)
                        return status;
                }
                else if (mp_pointcount < 65536) {
                    unsigned short      word = (unsigned short)count;

                    if ((status = PutAsciiData(tk,"Count", word)) != TK_Normal)
                        return status;
                }
                else {
                    if ((status = PutAsciiData(tk,"Count", count)) != TK_Normal)
                        return status;
                }
                mp_progress = 0;
                mp_substage++;
            }   nobreak;

            //and indices of existing values
            case 6: {
				PutTab t(&tk);
                while (mp_progress < mp_pointcount) {
                    if ((mp_exists[mp_progress] & mask) == mask) {
                        if (mp_pointcount < 256) {
                            unsigned char       byte = (unsigned char)mp_progress;

                            if ((status = PutAsciiData(tk,"Progress", byte)) != TK_Normal)
                                return status;
                        }
                        else if (mp_pointcount < 65536) {
                            unsigned short      word = (unsigned short)mp_progress;

                            if ((status = PutAsciiData(tk,"Progress", word)) != TK_Normal)
                                return status;
                        }
                        else {
                            if ((status = PutAsciiData(tk,"Progress", mp_progress)) != TK_Normal)
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
				PutTab t(&tk);
				if ((status = PutAsciiData(tk, tag2, count)) != TK_Normal)
					return status;
				mp_substage++;
				}   nobreak;

			case 8: {
				PutTab t(&tk);
				if( count != 0)
					if ((status = PutAsciiData(tk, tag1,  array,  count)) != TK_Normal)
						return status;
				mp_substage++;
					}   nobreak;
			case 9: {
				if ((status = PutEndXMLTag(tk,"Vertex_Indices")) != TK_Normal)
					return status;
				mp_substage = 0;
					}   break;

            default:
                return tk.Error("internal error in TK_Polyhedron::write_vertex_indices");
        }
    }
    else {
        switch (mp_substage) {
            //the count
			case 1: {
				if ((status = PutStartXMLTag(tk,"Vertex_Indices")) != TK_Normal)
					return status;
				mp_substage++;
					}   nobreak;

            case 2: {
				PutTab t(&tk);
                if (mp_pointcount < 256) {
                    unsigned char       byte = (unsigned char)count;

                    if ((status = PutAsciiData(tk,"Count", byte)) != TK_Normal)
                        return status;
                }
                else if (mp_pointcount < 65536) {
                    unsigned short      word = (unsigned short)count;

                    if ((status = PutAsciiData(tk,"Count", word)) != TK_Normal)
                        return status;
                }
                else {
                    if ((status = PutAsciiData(tk,"Count", count)) != TK_Normal)
                        return status;
                }
                mp_progress = 0;
                mp_substage++;
            }   nobreak;

            //and indices of existing values
            case 3: {
				PutTab t(&tk);
                while (mp_progress < mp_pointcount) {
                    if ((mp_exists[mp_progress] & mask) == mask) {
                        if (mp_pointcount < 256) {
                            unsigned char       byte = (unsigned char)mp_progress;

                            if ((status = PutAsciiData(tk,"Progress", byte)) != TK_Normal)
                                return status;
                        }
                        else if (mp_pointcount < 65536) {
                            unsigned short      word = (unsigned short)mp_progress;

                            if ((status = PutAsciiData(tk,"Progress", word)) != TK_Normal)
                                return status;
                        }
                        else {
                            if ((status = PutAsciiData(tk,"Progress", mp_progress)) != TK_Normal)
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
				PutTab t(&tk);
                while (mp_progress < mp_pointcount) {
                    if ((mp_exists[mp_progress] & mask) == mask) {
                        if ((status = PutAsciiData(tk,"Progress", &array[mp_progress], 1)) != TK_Normal)
                            return status;
                    }
                    mp_progress++;
                }
                mp_progress = 0;
				mp_substage++;
					}   nobreak;
			case 5: {
				if ((status = PutEndXMLTag(tk,"Vertex_Indices")) != TK_Normal)
					return status;
				mp_substage = 0;
					}   break;

            default:
                return tk.Error("internal error in TK_Polyhedron::write_vertex_indices (version<650)");
        }
    }
	
    return status;
#else
UNREFERENCED (op);
return tk.Error(stream_disable_ascii);
#endif 
} //end function TK_Polyhedron::write_vertex_indices


TK_Status TK_Polyhedron::write_vertex_indices_main_ascii (BStreamFileToolkit & tk, unsigned char base_op) alter {
#ifndef BSTREAM_DISABLE_ASCII
    
	TK_Status       status = TK_Normal;
    int             count;
	
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
        if ((status = PutAsciiHex(tk,"Optional_Opcode", mp_optopcode)) != TK_Normal)
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
	return tk.Error(stream_disable_ascii);
#endif 
}


TK_Status TK_Polyhedron::write_collection_ascii (BStreamFileToolkit & tk, bool write_tag, int variant) alter {
#ifndef BSTREAM_DISABLE_ASCII

	TK_Status       status = TK_Normal;
	TK_Terminator   terminate(TKE_Termination, false);
	

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

	if ((status = PutAsciiOpcode (tk,1,true,true)) != TK_Normal)
		return status;

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
	UNREFERENCED (variant);
	UNREFERENCED (write_tag);
	return tk.Error(stream_disable_ascii);
#endif 

}

TK_Status TK_Polyhedron::write_vertex_marker_visibilities_ascii (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII

	TK_Status       status = TK_Normal;
	
    if (mp_vmvisibilitycount == mp_pointcount) {
        switch (mp_substage) {
            //opcode
		
		   case 0: {
				if ((status = PutStartXMLTag(tk,"Vertex_Marker_Visibilities")) != TK_Normal)
					return status;
				mp_substage = 0;
					}   break;

            case 1: {
				PutTab t(&tk);
                mp_optopcode = OPT_ALL_VMVISIBILITIES;
                if ((status = PutAsciiHex(tk,"Optional_Opcode", mp_optopcode)) != TK_Normal)
                    return status;
                mp_substage++;
            }   nobreak;

            //compression scheme
            case 2: {
				PutTab t(&tk);
                mp_compression_scheme = CS_NONE;
                if ((status = PutAsciiData(tk,"Compression_Scheme", (int)mp_compression_scheme)) != TK_Normal)
                    return status;
                mp_substage++;
            }   nobreak;

            //all visibilities
            case 3: {
				PutTab t(&tk);
                if ((status = PutAsciiData(tk,"Visibilities", mp_vmvisibilities, mp_vmvisibilitycount)) != TK_Normal)
                    return status;
				mp_substage++;
					}   nobreak;

			case 4: {
				if ((status = PutEndXMLTag(tk,"Vertex_Marker_Visibilities")) != TK_Normal)
					return status;
				mp_substage = 0;
					}   break;

            default:
                return tk.Error("internal error in write_vertex_marker_visibilities (1)");
        }
    }
    else {
        switch (mp_substage) {
            //opcode
			case 0: {
				 if ((status = PutStartXMLTag(tk,"Vertex_Marker_Visibilities")) != TK_Normal)
					 return status;
				 mp_substage = 0;
					 }   break;

            case 1: {
				PutTab t(&tk);
                mp_optopcode = OPT_VERTEX_MARKER_VISIBILITIES;
                if ((status = PutAsciiHex(tk,"Optional_Opcode", mp_optopcode)) != TK_Normal)
                    return status;
                mp_substage++;
            }   nobreak;

            //compression scheme
            case 2: {
				PutTab t(&tk);
                mp_compression_scheme = CS_NONE;
                if ((status = PutAsciiData(tk,"Compression_Scheme", (int)mp_compression_scheme)) != TK_Normal)
                    return status;
                mp_substage++;
            }   nobreak;

            //count
            case 3: {
				PutTab t(&tk);
                if ((status = PutAsciiData(tk,"Visibilities", mp_vmvisibilitycount)) != TK_Normal)
                    return status;
                mp_substage++;
                mp_progress = 0;
            }   nobreak;

            //and indices of existing values
            case 4: {
				PutTab t(&tk);
                while (mp_progress < mp_pointcount) {
                    if (mp_exists[mp_progress] & Vertex_Marker_Visibility) {
                        if (mp_pointcount < 256) {
                            unsigned char       byte = (unsigned char)mp_progress;

                            if ((status = PutAsciiData(tk,"Progress", byte)) != TK_Normal)
                                return status;
                        }
                        else if (mp_pointcount < 65536) {
                            unsigned short      word = (unsigned short)mp_progress;

                            if ((status = PutAsciiData(tk,"Progress", word)) != TK_Normal)
                                return status;
                        }
                        else {
                            if ((status = PutAsciiData(tk,"Progress", mp_progress)) != TK_Normal)
                                return status;
                        }
                    }
                    mp_progress++;
                }
                mp_progress = 0;
                mp_substage++;
            } nobreak;

            //individual indices
            case 5: {
				PutTab t(&tk);
                while (mp_progress < mp_pointcount) {
                    if (mp_exists[mp_progress] & Vertex_Marker_Visibility) {
                        if ((status = PutAsciiData(tk,"Visibilities", mp_vmvisibilities[mp_progress])) != TK_Normal)
                            return status;
                    }
                    mp_progress++;
                }
                mp_progress = 0;
				mp_substage++;
					}   nobreak;

			case 6: {
				if ((status = PutEndXMLTag(tk,"Vertex_Marker_Visibilities")) != TK_Normal)
					return status;
				mp_substage = 0;
					}   break;
            default:
                return tk.Error("internal error in write_vertex_marker_visibilities (2)");
        }
    }
	
    return status;
#else
return tk.Error(stream_disable_ascii);
#endif 
} //end function TK_Polyhedron::write_vertex_marker_visibilities


TK_Status TK_Polyhedron::write_vertex_marker_symbols_ascii (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII

	TK_Status       status = TK_Normal;

    if (mp_vmsymbolcount == mp_pointcount && mp_vmsymbolstrings == null) {
        switch (mp_substage) {
            //opcode
			case 0: {
				if ((status = PutStartXMLTag(tk,"Vertex_Marker_Symbols")) != TK_Normal)
					return status;
				mp_substage++;
					}   nobreak;

            case 1: {
				PutTab t(&tk);
                mp_optopcode = OPT_ALL_VMSYMBOLS;
                if ((status = PutAsciiHex(tk,"Optional_Opcode", mp_optopcode)) != TK_Normal)
                    return status;
                mp_substage++;
            }   nobreak;

            //compression scheme
            case 2: {
				PutTab t(&tk);
                mp_compression_scheme = CS_NONE;
                if ((status = PutAsciiData(tk,"Compression_Scheme", (int)mp_compression_scheme)) != TK_Normal)
                    return status;
                mp_substage++;
            }   nobreak;

            //all symbols
            case 3: {
				PutTab t(&tk);
                if ((status = PutAsciiData(tk,"Symbols", mp_vmsymbols, mp_vmsymbolcount)) != TK_Normal)
                    return status;
				mp_substage++;
					}   nobreak;
			case 4: {

				if ((status = PutEndXMLTag(tk,"Vertex_Marker_Symbols")) != TK_Normal)
					return status;
				mp_substage = 0;
					}   break;
            default:
                return tk.Error("internal error in write_vertex_marker_symbols (1)");
        }
    }
    else {
        switch (mp_substage) {
            //opcode
			case 0: {
				if ((status = PutStartXMLTag(tk,"Vertex_Marker_Symbols")) != TK_Normal)
					return status;
				mp_substage++;
					}   nobreak;

            case 1: {
				PutTab t(&tk);
                mp_optopcode = OPT_VERTEX_MARKER_SYMBOLS;
                if ((status = PutAsciiHex(tk,"Optional_Opcode", mp_optopcode)) != TK_Normal)
                    return status;
                mp_substage++;
            }   nobreak;

            //compression scheme
            case 2: {
				PutTab t(&tk);
                mp_compression_scheme = CS_NONE;
                if ((status = PutAsciiData(tk,"Compression_Scheme", (int)mp_compression_scheme)) != TK_Normal)
                    return status;
                mp_substage++;
            }   nobreak;

            //count
            case 3: {
				PutTab t(&tk);
                if ((status = PutAsciiData(tk,"Symbol_Count", mp_vmsymbolcount)) != TK_Normal)
                    return status;
                mp_substage++;
                mp_progress = 0;
            }   nobreak;

            //and indices of existing values
            case 4: {
				PutTab t(&tk);
                while (mp_progress < mp_pointcount) {
                    if (mp_exists[mp_progress] & Vertex_Marker_Symbol) {
                        if (mp_pointcount < 256) {
                            unsigned char       byte = (unsigned char)mp_progress;

                            if ((status = PutAsciiData(tk,"Progress", byte)) != TK_Normal)
                                return status;
                        }
                        else if (mp_pointcount < 65536) {
                            unsigned short      word = (unsigned short)mp_progress;

                            if ((status = PutAsciiData(tk,"Progress", word)) != TK_Normal)
                                return status;
                        }
                        else {
                            if ((status = PutAsciiData(tk,"Progress", mp_progress)) != TK_Normal)
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
            case 5: {
				PutTab t(&tk);
                while (mp_progress < mp_pointcount) {
                    if (mp_exists[mp_progress] & Vertex_Marker_Symbol) {
                        switch (mp_subprogress) {
                            case 0: {
                                if (mp_vmsymbols[mp_progress] != 0x00FF) {
                                    if ((status = PutAsciiData(tk,"Symbols", mp_vmsymbols[mp_progress])) != TK_Normal)
                                        return status;
                                    break;
                                }
                                else if (tk.GetTargetVersion() < 1160) {
                                    _W3DTK_REQUIRE_VERSION( 1160 );
                                    if ((status = PutAsciiData(tk, "Symbols",(unsigned char)0)) != TK_Normal)
                                        return status;
                                    break;
                                }
                                else {
                                    unsigned char   byte = (unsigned char)0x00FF;

                                    if ((status = PutAsciiData(tk,"Symbols", byte)) != TK_Normal)
                                        return status;
                                }
                                mp_subprogress++;
                            }   nobreak;
                            case 1: {
                                unsigned char   byte = (unsigned char)strlen(mp_vmsymbolstrings[mp_progress]);

                                if ((status = PutAsciiData(tk,"String_Length", byte)) != TK_Normal)
                                    return status;
                                mp_subprogress++;
                            }   nobreak;
                            case 2: {
								if ((status = PutAsciiData(tk, "Symbol_Strings", mp_vmsymbolstrings[mp_progress], (int)strlen(mp_vmsymbolstrings[mp_progress]))) != TK_Normal)
                                    return status;
                                mp_subprogress = 0;
                            }   break;
                        }
                    }
                    mp_progress++;
                }
                mp_progress = 0;
				mp_substage++;
					}   nobreak;
			case 6: {
				if ((status = PutEndXMLTag(tk,"Vertex_Marker_Symbols")) != TK_Normal)
					return status;
				mp_substage = 0;
					}   break;
            default:
                return tk.Error("internal error in write_vertex_marker_symbols (2)");
        }
    }
	
    return status;
#else
return tk.Error(stream_disable_ascii);
#endif 
} //end function TK_Polyhedron::write_vertex_marker_symbols


TK_Status TK_Polyhedron::write_vertex_marker_sizes_ascii (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII

	TK_Status       status = TK_Normal;
	
	

    if (mp_vmsizecount == mp_pointcount) {
        switch (mp_substage) {
            //opcode
			case 0: {
				if ((status = PutStartXMLTag(tk,"Vertex_Marker_Sizes")) != TK_Normal)
					return status;
				mp_substage++;
					}   nobreak;

            case 1: {
				PutTab t(&tk);
                mp_optopcode = OPT_ALL_VMSIZES;
                if ((status = PutAsciiHex(tk,"Optional_Opcode", mp_optopcode)) != TK_Normal)
                    return status;
                mp_substage++;
            }   nobreak;

            //compression scheme
            case 2: {
				PutTab t(&tk);
                mp_compression_scheme = CS_NONE;
                if ((status = PutAsciiData(tk,"Compression_Scheme", (int)mp_compression_scheme)) != TK_Normal)
                    return status;
                mp_substage++;
            }   nobreak;

            //all sizes
            case 3: {
				PutTab t(&tk);
                if ((status = PutAsciiData(tk,"Sizes", mp_vmsizes, mp_vmsizecount)) != TK_Normal)
                    return status;
				mp_substage++;
					}   nobreak;
			case 4: {

				if ((status = PutEndXMLTag(tk,"Vertex_Marker_Sizes")) != TK_Normal)
					return status;
				mp_substage = 0;
					}   break;

            default:
                return tk.Error("internal error in write_vertex_marker_sizes (1)");
        }
    }
    else {
        switch (mp_substage) {
            //opcode
			case 0: {
				if ((status = PutStartXMLTag(tk,"Vertex_Marker_Sizes")) != TK_Normal)
					return status;
				mp_substage++;
					}   nobreak;

            case 1: {
				PutTab t(&tk);
                mp_optopcode = OPT_VERTEX_MARKER_SIZES;
                if ((status = PutAsciiHex(tk,"Optional_Opcode", mp_optopcode)) != TK_Normal)
                    return status;
                mp_substage++;
            }   nobreak;

            //compression scheme
            case 2: {
				PutTab t(&tk);
                mp_compression_scheme = CS_NONE;
                if ((status = PutAsciiData(tk,"Compression_Scheme", (int)mp_compression_scheme)) != TK_Normal)
                    return status;
                mp_substage++;
            }   nobreak;

            //count
            case 3: {
				PutTab t(&tk);
                if ((status = PutAsciiData(tk,"Size_Count", mp_vmsizecount)) != TK_Normal)
                    return status;
                mp_substage++;
                mp_progress = 0;
            }   nobreak;

            //and indices of existing values
            case 4: {
				PutTab t(&tk);
                while (mp_progress < mp_pointcount) {
                    if (mp_exists[mp_progress] & Vertex_Marker_Size) {
                        if (mp_pointcount < 256) {
                            unsigned char       byte = (unsigned char)mp_progress;

                            if ((status = PutAsciiData(tk,"Progress", byte)) != TK_Normal)
                                return status;
                        }
                        else if (mp_pointcount < 65536) {
                            unsigned short      word = (unsigned short)mp_progress;

                            if ((status = PutAsciiData(tk,"Progress", word)) != TK_Normal)
                                return status;
                        }
                        else {
                            if ((status = PutAsciiData(tk,"Progress", mp_progress)) != TK_Normal)
                                return status;
                        }
                    }
                    mp_progress++;
                }
                mp_progress = 0;
                mp_substage++;
            } nobreak;

            //individual indices
            case 5: {
				PutTab t(&tk);
                while (mp_progress < mp_pointcount) {
                    if (mp_exists[mp_progress] & Vertex_Marker_Size) {
                        if ((status = PutAsciiData(tk,"Sizes", mp_vmsizes[mp_progress])) != TK_Normal)
                            return status;
                    }
                    mp_progress++;
                }
                mp_progress = 0;
				mp_substage++;
					}   nobreak;
			case 6: {
				if ((status = PutEndXMLTag(tk,"Vertex_Marker_Sizes")) != TK_Normal)
					return status;
				mp_substage = 0;
					}   break;

            default:
                return tk.Error("internal error in write_vertex_marker_sizes (2)");
        }
    }
	
    return status;
#else
return tk.Error(stream_disable_ascii);
#endif 
} //end function TK_Polyhedron::write_vertex_marker_sizes


TK_Status TK_Polyhedron::write_face_visibilities_ascii (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII

	TK_Status       status = TK_Normal;
	
	

    if (mp_fvisibilitycount == mp_facecount) {
        switch (mp_substage) {
            //opcode
			case 0: {
				if ((status = PutStartXMLTag(tk,"Face_Visibilities")) != TK_Normal)
					return status;
				mp_substage++;
					}   nobreak;

            case 1: {
				PutTab t(&tk);
                mp_optopcode = OPT_ALL_FACE_VISIBILITIES;
                if ((status = PutAsciiHex(tk,"Optional_Opcode", mp_optopcode)) != TK_Normal)
                    return status;
                mp_substage++;
            }   nobreak;

            //compression scheme
            case 2: {
				PutTab t(&tk);
                mp_compression_scheme = CS_NONE;
                if ((status = PutAsciiData(tk,"Compression_Scheme", (int)mp_compression_scheme)) != TK_Normal)
                    return status;
                mp_substage++;
            }   nobreak;

            //all visibilities
            case 3: {
				PutTab t(&tk);
                if ((status = PutAsciiData(tk,"Visibilities", mp_fvisibilities, mp_fvisibilitycount)) != TK_Normal)
                    return status;
				mp_substage++;
					}   nobreak;
			case 4: {

				if ((status = PutEndXMLTag(tk,"Face_Visibilities")) != TK_Normal)
					return status;
				mp_substage = 0;
					}   break;

            default:
                return tk.Error("internal error in write_face_visibilities (1)");
        }
    }
    else {
        switch (mp_substage) {
            //opcode
			case 0: {
				if ((status = PutStartXMLTag(tk,"Face_Visibilities")) != TK_Normal)
					return status;
				mp_substage++;
					}   nobreak;

            case 1: {
				PutTab t(&tk);
                mp_optopcode = OPT_FACE_VISIBILITIES;
                if ((status = PutAsciiHex(tk,"Optional_Opcode", mp_optopcode)) != TK_Normal)
                    return status;
                mp_substage++;
            }   nobreak;

            //compression scheme
            case 2: {
				PutTab t(&tk);
                mp_compression_scheme = CS_NONE;
                if ((status = PutAsciiData(tk,"Compression_Scheme", (int)mp_compression_scheme)) != TK_Normal)
                    return status;
                mp_substage++;
            }   nobreak;

            //count
            case 3: {
				PutTab t(&tk);
                if ((status = PutAsciiData(tk,"Visiblity_Count", mp_fvisibilitycount)) != TK_Normal)
                    return status;
                mp_substage++;
                mp_progress = 0;
            }   nobreak;

            //and indices of existing values
            case 4: {
				PutTab t(&tk);
                while (mp_progress < mp_facecount) {
                    if (mp_face_exists[mp_progress] & Face_Visibility) {
                        if (mp_facecount < 256) {
                            unsigned char       byte = (unsigned char)mp_progress;

                            if ((status = PutAsciiData(tk,"Progress", byte)) != TK_Normal)
                                return status;
                        }
                        else if (mp_facecount < 65536) {
                            unsigned short      word = (unsigned short)mp_progress;

                            if ((status = PutAsciiData(tk,"Progress", word)) != TK_Normal)
                                return status;
                        }
                        else {
                            if ((status = PutAsciiData(tk,"Progress", mp_progress)) != TK_Normal)
                                return status;
                        }
                    }
                    mp_progress++;
                }
                mp_progress = 0;
                mp_substage++;
            } nobreak;

            //individual indices
            case 5: {
				PutTab t(&tk);
                while (mp_progress < mp_facecount) {
                    if (mp_face_exists[mp_progress] & Face_Visibility) {
                        if ((status = PutAsciiData(tk,"Visibilities", mp_fvisibilities[mp_progress])) != TK_Normal)
                            return status;
                    }
                    mp_progress++;
                }
                mp_progress = 0;
				mp_substage++;
					}   nobreak;
			case 6: {
				if ((status = PutEndXMLTag(tk,"Face_Visibilities")) != TK_Normal)
					return status;
				mp_substage = 0;
					}   break;
            default:
                return tk.Error("internal error in write_face_visibilities (2)");
        }
    }
	
    return status;
#else
return tk.Error(stream_disable_ascii);
#endif 
} //end function TK_Polyhedron::write_face_visibilities


TK_Status TK_Polyhedron::write_face_patterns_ascii (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII

	TK_Status       status = TK_Normal;
	
    if (mp_fpatterncount == mp_facecount) {
        switch (mp_substage) {
            //opcode
			case 0: {
				if ((status = PutStartXMLTag(tk,"Face_Patterns")) != TK_Normal)
					return status;
				mp_substage++;
					}   nobreak;

            case 1: {
				PutTab t(&tk);
                mp_optopcode = OPT_ALL_FACE_PATTERNS;
                if ((status = PutAsciiHex(tk,"Optional_Opcode", mp_optopcode)) != TK_Normal)
                    return status;
                mp_substage++;
            }   nobreak;

            //compression scheme
            case 2: {
				PutTab t(&tk);
                mp_compression_scheme = CS_NONE;
                if ((status = PutAsciiData(tk,"Compression_Scheme",(int) mp_compression_scheme)) != TK_Normal)
                    return status;
                mp_substage++;
            }   nobreak;

            //all patterns
            case 3: {
				PutTab t(&tk);
                if ((status = PutAsciiData(tk,"Face_Patterns", mp_fpatterns, mp_fpatterncount)) != TK_Normal)
                    return status;
				mp_substage++;
					}   nobreak;
			
			case 4: {
				if ((status = PutEndXMLTag(tk,"Face_Patterns")) != TK_Normal)
					return status;
				mp_substage = 0;
					}   break;

            default:
                return tk.Error("internal error in write_face_patterns (1)");
        }
    }
    else {
        switch (mp_substage) {
            //opcode
			case 0: {
				if ((status = PutStartXMLTag(tk,"Face_Patterns")) != TK_Normal)
					return status;
				mp_substage++;
					}   nobreak;

            case 1: {
				PutTab t(&tk);
                mp_optopcode = OPT_FACE_PATTERNS;
                if ((status = PutAsciiHex(tk,"Optional_Opcode", mp_optopcode)) != TK_Normal)
                    return status;
                mp_substage++;
            }   nobreak;

            //compression scheme
            case 2: {
				PutTab t(&tk);
                mp_compression_scheme = CS_NONE;
                if ((status = PutAsciiData(tk,"Compression_Scheme", (int)mp_compression_scheme)) != TK_Normal)
                    return status;
                mp_substage++;
            }   nobreak;

            //count
            case 3: {
				PutTab t(&tk);
                if ((status = PutAsciiData(tk,"Pattern_Count", mp_fpatterncount)) != TK_Normal)
                    return status;
                mp_substage++;
                mp_progress = 0;
            }   nobreak;

            //and indices of existing values
            case 4: {
				PutTab t(&tk);
                while (mp_progress < mp_facecount) {
                    if (mp_face_exists[mp_progress] & Face_Pattern) {
                        if (mp_facecount < 256) {
                            unsigned char       byte = (unsigned char)mp_progress;

                            if ((status = PutAsciiData(tk,"Progress", byte)) != TK_Normal)
                                return status;
                        }
                        else if (mp_facecount < 65536) {
                            unsigned short      word = (unsigned short)mp_progress;

                            if ((status = PutAsciiData(tk,"Progress", word)) != TK_Normal)
                                return status;
                        }
                        else {
                            if ((status = PutAsciiData(tk,"Progress", mp_progress)) != TK_Normal)
                                return status;
                        }
                    }
                    mp_progress++;
                }
                mp_progress = 0;
                mp_substage++;
            } nobreak;

            //individual indices
            case 5: {
				PutTab t(&tk);
                while (mp_progress < mp_facecount) {
                    if (mp_face_exists[mp_progress] & Face_Pattern) {
                        if ((status = PutAsciiData(tk,"Face_Pattern", mp_fpatterns[mp_progress])) != TK_Normal)
                            return status;
                    }
                    mp_progress++;
                }
                mp_progress = 0;
				mp_substage++;
					}   nobreak;

			case 6: {

				if ((status = PutEndXMLTag(tk,"Face_Patterns")) != TK_Normal)
					return status;
				mp_substage = 0;
					}   break;
            default:
                return tk.Error("internal error in write_face_patterns (2)");
        }
    }
	
    return status;
#else
return tk.Error(stream_disable_ascii);
#endif 
} //end function TK_Polyhedron::write_face_patterns


TK_Status TK_Polyhedron::write_face_normals_ascii (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII

	TK_Status       status = TK_Normal;
	
	if (mp_fnormalcount == mp_facecount) {
        switch (mp_substage) {
            //opcode
			case 0: {
				if ((status = PutStartXMLTag(tk,"Face_Normals")) != TK_Normal)
					return status;
				mp_substage++;
					}   nobreak;

            case 1: {
				PutTab t(&tk);
                mp_optopcode = OPT_ALL_FACE_NORMALS_POLAR;
                if ((status = PutAsciiHex(tk,"Optional_Opcode", mp_optopcode)) != TK_Normal)
                    return status;
                mp_substage++;
            }   nobreak;

            //compression scheme
            case 2: {
				PutTab t(&tk);
                mp_compression_scheme = CS_NONE;
                if ((status = PutAsciiData(tk,"Compression_Scheme", (int)mp_compression_scheme)) != TK_Normal)
                    return status;
                normals_cartesian_to_polar(null, Face_Normal, mp_facecount, mp_fnormals, mp_fnormals);
                mp_substage++;
            }   nobreak;

            //all normals
            case 3: {
				PutTab t(&tk);
                if ((status = PutAsciiData(tk,"Face_Normals", mp_fnormals, 2*mp_fnormalcount)) != TK_Normal)
                    return status;
				mp_substage++;
					}   nobreak;
			case 4: {
				if ((status = PutEndXMLTag(tk,"Face_Normals")) != TK_Normal)
					return status;
				mp_substage = 0;
					}   break;

            default:
                return tk.Error("internal error in write_face_normals (1)");
        }
    }
    else {
        switch (mp_substage) {
            //opcode
			case 0: {
				if ((status = PutStartXMLTag(tk,"Face_Normals")) != TK_Normal)
					return status;
				mp_substage++;
					}   nobreak;

            case 1: {
				PutTab t(&tk);
                mp_optopcode = OPT_FACE_NORMALS_POLAR;
                if ((status = PutAsciiHex(tk,"Optional_Opcode", mp_optopcode)) != TK_Normal)
                    return status;
                mp_substage++;
            }   nobreak;

            //compression scheme
            case 2: {
				PutTab t(&tk);
                mp_compression_scheme = CS_NONE;
                if ((status = PutAsciiData(tk,"Compression_Scheme",(int) mp_compression_scheme)) != TK_Normal)
                    return status;
                mp_substage++;
            }   nobreak;

            //count
            case 3: {
				PutTab t(&tk);
                if ((status = PutAsciiData(tk,"Face_Normal_Count", mp_fnormalcount)) != TK_Normal)
                    return status;
                mp_substage++;
                mp_progress = 0;
            }   nobreak;

            //and indices of existing values
            case 4: {
				PutTab t(&tk);
                while (mp_progress < mp_facecount) {
                    if (mp_face_exists[mp_progress] & Face_Normal) {
                        if (mp_facecount < 256) {
                            unsigned char       byte = (unsigned char)mp_progress;

                            if ((status = PutAsciiData(tk,"Progress", byte)) != TK_Normal)
                                return status;
                        }
                        else if (mp_facecount < 65536) {
                            unsigned short      word = (unsigned short)mp_progress;

                            if ((status = PutAsciiData(tk,"Progress", word)) != TK_Normal)
                                return status;
                        }
                        else {
                            if ((status = PutAsciiData(tk,"Progress", mp_progress)) != TK_Normal)
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
            case 5: {
				PutTab t(&tk);
                while (mp_progress < mp_facecount) {
                    if (mp_face_exists[mp_progress] & Face_Normal) {
                        if ((status = PutAsciiData(tk,"Face_Normals", &mp_fnormals[2*mp_progress], 2)) != TK_Normal)
                            return status;
                    }
                    mp_progress++;
                }
                mp_progress = 0;
				mp_substage++;
					}   nobreak;

			case 6: {
				if ((status = PutEndXMLTag(tk,"Face_Normals")) != TK_Normal)
					return status;
				mp_substage = 0;
					}   break;
            default:
                return tk.Error("internal error in write_face_normals (2)");
        }
    }
	
    return status;
#else
return tk.Error(stream_disable_ascii);
#endif 
} //end function TK_Polyhedron::write_face_normals


TK_Status TK_Polyhedron::write_edge_visibilities_ascii (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII

	TK_Status       status = TK_Normal;
	
    if (mp_evisibilitycount == mp_edgecount) {
        switch (mp_substage) {
            //opcode
			case 0: {
				if ((status = PutStartXMLTag(tk,"Edge_Visibilities")) != TK_Normal)
					return status;
				mp_substage++;
					}   nobreak;

            case 1: {
				PutTab t(&tk);
                mp_optopcode = OPT_ALL_EDGE_VISIBILITIES;
                if ((status = PutAsciiHex(tk,"Optional_Opcode", mp_optopcode)) != TK_Normal)
                    return status;
                mp_substage++;
            }   nobreak;

            //compression scheme
            case 2: {
				PutTab t(&tk);
                mp_compression_scheme = CS_NONE;
                if ((status = PutAsciiData(tk,"Compression_Scheme", (int)mp_compression_scheme)) != TK_Normal)
                    return status;
                mp_substage++;
            }   nobreak;

            //all visibilities
            case 3: {
				PutTab t(&tk);
                if ((status = PutAsciiData(tk,"Visibilities", mp_evisibilities, mp_evisibilitycount)) != TK_Normal)
                    return status;
				mp_substage++;
					}   nobreak;
			case 4: {

				if ((status = PutEndXMLTag(tk,"Edge_Visibilities")) != TK_Normal)
					return status;
				mp_substage = 0;
					}   break;
            default:
                return tk.Error("internal error in write_edge_visibilities (1)");
        }
    }
    else {
        switch (mp_substage) {
            //opcode
			case 0: {
				if ((status = PutStartXMLTag(tk,"Edge_Visibilities")) != TK_Normal)
					return status;
				mp_substage++;
					}   nobreak;

            case 1: {
				PutTab t(&tk);
                mp_optopcode = OPT_EDGE_VISIBILITIES;
                if ((status = PutAsciiHex(tk,"Optional_Opcode", mp_optopcode)) != TK_Normal)
                    return status;
                mp_substage++;
            }   nobreak;

            //compression scheme
            case 2: {
				PutTab t(&tk);
                mp_compression_scheme = CS_NONE;
                if ((status = PutAsciiData(tk,"Compression_Scheme", (int)mp_compression_scheme)) != TK_Normal)
                    return status;
                mp_substage++;
            }   nobreak;

            //count
            case 3: {
				PutTab t(&tk);
                if ((status = PutAsciiData(tk,"Visiblity_Count", mp_evisibilitycount)) != TK_Normal)
                    return status;
                mp_substage++;
                mp_progress = 0;
            }   nobreak;

            //and indices of existing values
            case 4: {
				PutTab t(&tk);
                while (mp_progress < mp_edgecount) {
                    if (mp_edge_exists[mp_progress] & Edge_Visibility) {
                        if (mp_edgecount < 256) {
                            unsigned char       byte = (unsigned char)mp_progress;

                            if ((status = PutAsciiData(tk,"Progress", byte)) != TK_Normal)
                                return status;
                        }
                        else if (mp_edgecount < 65536) {
                            unsigned short      word = (unsigned short)mp_progress;

                            if ((status = PutAsciiData(tk,"Progress", word)) != TK_Normal)
                                return status;
                        }
                        else {
                            if ((status = PutAsciiData(tk,"Progress", mp_progress)) != TK_Normal)
                                return status;
                        }
                    }
                    mp_progress++;
                }
                mp_progress = 0;
                mp_substage++;
            } nobreak;

            //individual indices
            case 5: {
				PutTab t(&tk);
                while (mp_progress < mp_edgecount) {
                    if (mp_edge_exists[mp_progress] & Edge_Visibility) {
                        if ((status = PutAsciiData(tk,"Visibilities", mp_evisibilities[mp_progress])) != TK_Normal)
                            return status;
                    }
                    mp_progress++;
                }
                mp_progress = 0;
				mp_substage++;
					}   nobreak;
			case 6: {
				if ((status = PutEndXMLTag(tk,"Edge_Visibilities")) != TK_Normal)
					return status;
				mp_substage = 0;
					}   break;
            default:
                return tk.Error("internal error in write_edge_visibilities (2)");
        }
    }
	
    return status;
#else
return tk.Error(stream_disable_ascii);
#endif 
} //end function TK_Polyhedron::write_edge_visibilities


TK_Status TK_Polyhedron::write_edge_patterns_ascii (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII

	TK_Status       status = TK_Normal;
	
    if (mp_epatterncount == mp_edgecount && mp_epatternstrings == null) {
        switch (mp_substage) {
            //opcode
			case 0: {
				if ((status = PutStartXMLTag(tk,"Edge_Patterns")) != TK_Normal)
					return status;
				mp_substage++;
					}   nobreak;

            case 1: {
				PutTab t(&tk);
                mp_optopcode = OPT_ALL_EDGE_PATTERNS;
                if ((status = PutAsciiHex(tk,"Optional_Opcode", mp_optopcode)) != TK_Normal)
                    return status;
                mp_substage++;
            }   nobreak;

            //compression scheme
            case 2: {
				PutTab t(&tk);
                mp_compression_scheme = CS_NONE;
                if ((status = PutAsciiData(tk,"Compression_Scheme", (int)mp_compression_scheme)) != TK_Normal)
                    return status;
                mp_substage++;
            }   nobreak;

            //all patterns
            case 3: {
				PutTab t(&tk);
                if ((status = PutAsciiData(tk,"Patterns", mp_epatterns, mp_epatterncount)) != TK_Normal)
                    return status;
				mp_substage++;
					}   nobreak;
		
			case 4: {
				if ((status = PutEndXMLTag(tk,"Edge_Patterns")) != TK_Normal)
					return status;
				mp_substage = 0;
					}   break;

            default:
                return tk.Error("internal error in write_edge_patterns (1)");
        }
    }
    else {
        switch (mp_substage) {
            //opcode
			case 0: {
				if ((status = PutStartXMLTag(tk,"Edge_Patterns")) != TK_Normal)
					return status;
				mp_substage++;
					}   nobreak;

            case 1: {
				PutTab t(&tk);
                mp_optopcode = OPT_EDGE_PATTERNS;
                if ((status = PutAsciiHex(tk,"Optional_Opcode", mp_optopcode)) != TK_Normal)
                    return status;
                mp_substage++;
            }   nobreak;

            //compression scheme
            case 2: {
				PutTab t(&tk);
                mp_compression_scheme = CS_NONE;
                if ((status = PutAsciiData(tk,"Compression_Scheme", (int)mp_compression_scheme)) != TK_Normal)
                    return status;
                mp_substage++;
            }   nobreak;

            //count
            case 3: {
				PutTab t(&tk);
                if ((status = PutAsciiData(tk,"PatternCount", mp_epatterncount)) != TK_Normal)
                    return status;
                mp_substage++;
                mp_progress = 0;
            }   nobreak;

            //and indices of existing values
            case 4: {
				PutTab t(&tk);
                while (mp_progress < mp_edgecount) {
                    if (mp_edge_exists[mp_progress] & Edge_Pattern) {
                        if (mp_edgecount < 256) {
                            unsigned char       byte = (unsigned char)mp_progress;

                            if ((status = PutAsciiData(tk,"Progress", byte)) != TK_Normal)
                                return status;
                        }
                        else if (mp_edgecount < 65536) {
                            unsigned short      word = (unsigned short)mp_progress;

                            if ((status = PutAsciiData(tk,"Progress", word)) != TK_Normal)
                                return status;
                        }
                        else {
                            if ((status = PutAsciiData(tk,"Progress", mp_progress)) != TK_Normal)
                                return status;
                        }
                    }
                    mp_progress++;
                }
                mp_progress = 0;
                mp_substage++;
            } nobreak;

            //individual indices
            case 5: {
				PutTab t(&tk);
                while (mp_progress < mp_edgecount) {
                    if (mp_edge_exists[mp_progress] & Edge_Pattern) {
                        switch (mp_subprogress) {
                            case 0: {
                                if (mp_epatterns[mp_progress] != 0x00FF) {
									if ((status = PutAsciiData(tk, "Patterns", mp_epatterns[mp_progress])) != TK_Normal)
                                        return status;
                                    break;
                                }
                                else if (tk.GetTargetVersion() < 1160) {
                                    _W3DTK_REQUIRE_VERSION( 1160 );
                                    if ((status = PutAsciiData(tk, "Patterns",(unsigned char)0)) != TK_Normal)
                                        return status;
                                    break;
                                }
                                else {
                                    unsigned char   byte = (unsigned char)0x00FF;

                                    if ((status = PutAsciiData(tk,"Patterns", byte)) != TK_Normal)
                                        return status;
                                }
                                mp_subprogress++;
                            }   nobreak;
                            case 1: {
                                unsigned char   byte = (unsigned char)strlen(mp_epatternstrings[mp_progress]);

                                if ((status = PutAsciiData(tk,"String_Length", byte)) != TK_Normal)
                                    return status;
                                mp_subprogress++;
                            }   nobreak;
                            case 2: {
                                if ((status = PutAsciiData(tk,"Pattern_Strings", mp_epatternstrings[mp_progress], (int)strlen(mp_epatternstrings[mp_progress]))) != TK_Normal)
                                    return status;
                                mp_subprogress = 0;
                            }   break;
                        }
                    }
                    mp_progress++;
                }
                mp_progress = 0;
				mp_substage++;
					}   nobreak;

			case 6: {
				if ((status = PutEndXMLTag(tk,"Edge_Patterns")) != TK_Normal)
					return status;
				mp_substage = 0;
					}   break;
            default:
                return tk.Error("internal error in write_edge_patterns (2)");
        }
    }
	
    return status;
#else
return tk.Error(stream_disable_ascii);
#endif 
} //end function TK_Polyhedron::write_edge_patterns


TK_Status TK_Polyhedron::write_edge_colors_ascii (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII
    
	TK_Status       status = TK_Normal;

    if (mp_ecolorcount == mp_edgecount) {
        switch (mp_substage) {
            //opcode
			case 0: {
				if ((status = PutStartXMLTag(tk,"Face_Color")) != TK_Normal)
					return status;
				mp_substage++;
					}   nobreak;

            case 1: {
				PutTab t(&tk);
                mp_optopcode = OPT_ALL_EDGE_COLORS;
                if ((status = PutAsciiHex(tk,"Optional_Opcode", mp_optopcode)) != TK_Normal)
                    return status;
                mp_substage++;
            }   nobreak;

            //compression scheme
            case 2: {
				PutTab t(&tk);
                mp_compression_scheme = CS_NONE;
                if ((status = PutAsciiData(tk,"Compression_Scheme", (int)mp_compression_scheme)) != TK_Normal)
                    return status;
                mp_substage++;
            }   nobreak;

            //all colors
            case 3: {
				PutTab t(&tk);
                if ((status = PutAsciiData(tk,"Colors", mp_ecolors, 3 * mp_ecolorcount)) != TK_Normal)
                    return status;
				mp_substage++;
					}   nobreak;
			case 4: {
				if ((status = PutEndXMLTag(tk,"Face_Color")) != TK_Normal)
					return status;
				mp_substage = 0;
					}   break;

            default:
                return tk.Error("internal error in write_edge_colors (1)");
        }
    }
    else {
        switch (mp_substage) {
            //opcode
			case 0: {
				if ((status = PutStartXMLTag(tk,"Face_Color")) != TK_Normal)
					return status;
				mp_substage++;
					}   nobreak;

            case 1: {
				PutTab t(&tk);
                mp_optopcode = OPT_EDGE_COLORS;
                if ((status = PutAsciiHex(tk,"Optional_Opcode", mp_optopcode)) != TK_Normal)
                    return status;
                mp_substage++;
            }   nobreak;

            //compression scheme
            case 2: {
				PutTab t(&tk);
                mp_compression_scheme = CS_NONE;
                if ((status = PutAsciiData(tk,"Compression_Scheme", (int)mp_compression_scheme)) != TK_Normal)
                    return status;
                mp_substage++;
            }   nobreak;

            //count
            case 3: {
				PutTab t(&tk);
                if ((status = PutAsciiData(tk,"Color_Count", mp_ecolorcount)) != TK_Normal)
                    return status;
                mp_substage++;
                mp_progress = 0;
            }   nobreak;

            //and indices of existing values
            case 4: {
				PutTab t(&tk);
                while (mp_progress < mp_edgecount) {
                    if (mp_edge_exists[mp_progress] & Edge_Color) {
                        if (mp_edgecount < 256) {
                            unsigned char       byte = (unsigned char)mp_progress;

                            if ((status = PutAsciiData(tk,"Progress", byte)) != TK_Normal)
                                return status;
                        }
                        else if (mp_edgecount < 65536) {
                            unsigned short      word = (unsigned short)mp_progress;

                            if ((status = PutAsciiData(tk,"Progress", word)) != TK_Normal)
                                return status;
                        }
                        else {
                            if ((status = PutAsciiData(tk,"Progress", mp_progress)) != TK_Normal)
                                return status;
                        }
                    }
                    mp_progress++;
                }
                mp_progress = 0;
                mp_substage++;
            } nobreak;

            //individual indices
            case 5: {
				PutTab t(&tk);
                while (mp_progress < mp_edgecount) {
                    if (mp_edge_exists[mp_progress] & Edge_Color) {
                        if ((status = PutAsciiData(tk,"Colors", &mp_ecolors[3*mp_progress],3)) != TK_Normal)
                            return status;
                    }
                    mp_progress++;
                }
                mp_progress = 0;
				mp_substage++;
					}   nobreak;
			case 6: {
				if ((status = PutEndXMLTag(tk,"Face_Color")) != TK_Normal)
					return status;
				mp_substage = 0;
					}   break;

            default:
                return tk.Error("internal error in write_edge_colors (2)");
        }
    }
	
    return status;
#else
return tk.Error(stream_disable_ascii);
#endif 
} //end function TK_Polyhedron::write_edge_colors


TK_Status TK_Polyhedron::write_edge_indices_ascii (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII

	TK_Status       status = TK_Normal;
	
    if (mp_eindexcount == mp_edgecount) {
        switch (mp_substage) {
            //opcode
			case 0: {
				if ((status = PutStartXMLTag(tk,"Edge_Indices")) != TK_Normal)
					return status;
				mp_substage++;
					}   nobreak;

            case 1: {
				PutTab t(&tk);
                mp_optopcode = OPT_ALL_EDGE_INDICES;
                if ((status = PutAsciiHex(tk,"Optional_Opcode", mp_optopcode)) != TK_Normal)
                    return status;
                mp_substage++;
            }   nobreak;

            //compression scheme
            case 2: {
				PutTab t(&tk);
                mp_compression_scheme = CS_NONE;
                if ((status = PutAsciiData(tk,"Compression_Scheme", (int)mp_compression_scheme)) != TK_Normal)
                    return status;
                mp_substage++;
            }   nobreak;

            //all indices
            case 3: {
				PutTab t(&tk);
                if ((status = PutAsciiData(tk,"Indices", mp_eindices, mp_eindexcount)) != TK_Normal)
                    return status;
				mp_substage++;
					}   nobreak;
			case 4: {
				if ((status = PutEndXMLTag(tk,"Edge_Indices")) != TK_Normal)
					return status;
				mp_substage = 0;
					}   break;

            default:
                return tk.Error("internal error in write_edge_indices (1)");
        }
    }
    else {
        switch (mp_substage) {
            //opcode
			case 0: {
				if ((status = PutStartXMLTag(tk,"Edge_Indices")) != TK_Normal)
					return status;
				mp_substage++;
					}   nobreak;

            case 1: {
				PutTab t(&tk);
                mp_optopcode = OPT_EDGE_INDICES;
                if ((status = PutAsciiHex(tk,"Optional_Opcode", mp_optopcode)) != TK_Normal)
                    return status;
                mp_substage++;
            }   nobreak;

            //compression scheme
            case 2: {
				PutTab t(&tk);
                mp_compression_scheme = CS_NONE;
                if ((status = PutAsciiData(tk,"Compression_Scheme", (int)mp_compression_scheme)) != TK_Normal)
                    return status;
                mp_substage++;
            }   nobreak;

            //count
            case 3: {
				PutTab t(&tk);
                if ((status = PutAsciiData(tk,"Index_Count", mp_eindexcount)) != TK_Normal)
                    return status;
                mp_substage++;
                mp_progress = 0;
            }   nobreak;

            //and indices of existing values
            case 4: {
				PutTab t(&tk);
                while (mp_progress < mp_edgecount) {
                    if (mp_edge_exists[mp_progress] & Edge_Index) {
                        if (mp_edgecount < 256) {
                            unsigned char       byte = (unsigned char)mp_progress;

                            if ((status = PutAsciiData(tk,"Progress", byte)) != TK_Normal)
                                return status;
                        }
                        else if (mp_edgecount < 65536) {
                            unsigned short      word = (unsigned short)mp_progress;

                            if ((status = PutAsciiData(tk,"Progress", word)) != TK_Normal)
                                return status;
                        }
                        else {
                            if ((status = PutAsciiData(tk,"Progress", mp_progress)) != TK_Normal)
                                return status;
                        }
                    }
                    mp_progress++;
                }
                mp_progress = 0;
                mp_substage++;
            } nobreak;

            //individual indices
            case 5: {
				PutTab t(&tk);
                while (mp_progress < mp_edgecount) {
                    if (mp_edge_exists[mp_progress] & Edge_Index) {
                        if ((status = PutAsciiData(tk,"Indices", mp_eindices[mp_progress])) != TK_Normal)
                            return status;
                    }
                    mp_progress++;
                }
                mp_progress = 0;
				mp_substage++;
					}   nobreak;
			case 6: {
				if ((status = PutEndXMLTag(tk,"Edge_Indices")) != TK_Normal)
					return status;
				mp_substage = 0;
					}   break;

            default:
                return tk.Error("internal error in write_edge_indices (2)");
        }
    }
	
    return status;
#else
return tk.Error(stream_disable_ascii);
#endif 
} //end function TK_Polyhedron::write_edge_indices


TK_Status TK_Polyhedron::write_edge_weights_ascii (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII

	TK_Status       status = TK_Normal;
	
	

    if (mp_eweightcount == mp_edgecount) {
        switch (mp_substage) {
            //opcode
			case 0: {
				if ((status = PutStartXMLTag(tk,"Edge_Weights")) != TK_Normal)
					return status;
				mp_substage++;
					}   nobreak;

            case 1: {
				PutTab t(&tk);
                mp_optopcode = OPT_ALL_EDGE_WEIGHTS;
                if ((status = PutAsciiHex(tk,"Optional_Opcode", mp_optopcode)) != TK_Normal)
                    return status;
                mp_substage++;
            }   nobreak;

            //compression scheme
            case 2: {
				PutTab t(&tk);
                mp_compression_scheme = CS_NONE;
                if ((status = PutAsciiData(tk,"Compression_Scheme",(int) mp_compression_scheme)) != TK_Normal)
                    return status;
                mp_substage++;
            }   nobreak;

            //all weights
            case 3: {
				PutTab t(&tk);
                if ((status = PutAsciiData(tk,"Weights", mp_eweights, mp_eweightcount)) != TK_Normal)
                    return status;
				mp_substage++;
					}   nobreak;

			case 4: {
				if ((status = PutEndXMLTag(tk,"Edge_Weights")) != TK_Normal)
					return status;
				mp_substage = 0;
					}   break;


            default:
                return tk.Error("internal error in write_edge_weights (1)");
        }
    }
    else {
        switch (mp_substage) {
            //opcode
			case 0: {
				if ((status = PutStartXMLTag(tk,"Edge_Weights")) != TK_Normal)
					return status;
				mp_substage++;
					}   nobreak;

            case 1: {
				PutTab t(&tk);
                mp_optopcode = OPT_EDGE_WEIGHTS;
                if ((status = PutAsciiHex(tk,"Optional_Opcode", mp_optopcode)) != TK_Normal)
                    return status;
                mp_substage++;
            }   nobreak;

            //compression scheme
            case 2: {
				PutTab t(&tk);
                mp_compression_scheme = CS_NONE;
                if ((status = PutAsciiData(tk,"Compression_Scheme", (int)mp_compression_scheme)) != TK_Normal)
                    return status;
                mp_substage++;
            }   nobreak;

            //count
            case 3: {
				PutTab t(&tk);
                if ((status = PutAsciiData(tk,"WeightCount", mp_eweightcount)) != TK_Normal)
                    return status;
                mp_substage++;
                mp_progress = 0;
            }   nobreak;

            //and indices of existing values
            case 4: {
				PutTab t(&tk);
                while (mp_progress < mp_edgecount) {
                    if (mp_edge_exists[mp_progress] & Edge_Weight) {
                        if (mp_edgecount < 256) {
                            unsigned char       byte = (unsigned char)mp_progress;

                            if ((status = PutAsciiData(tk,"Progress", byte)) != TK_Normal)
                                return status;
                        }
                        else if (mp_edgecount < 65536) {
                            unsigned short      word = (unsigned short)mp_progress;

                            if ((status = PutAsciiData(tk,"Progress", word)) != TK_Normal)
                                return status;
                        }
                        else {
                            if ((status = PutAsciiData(tk,"Progress", mp_progress)) != TK_Normal)
                                return status;
                        }
                    }
                    mp_progress++;
                }
                mp_progress = 0;
                mp_substage++;
            } nobreak;

            //individual indices
            case 5: {
				PutTab t(&tk);
                while (mp_progress < mp_edgecount) {
                    if (mp_edge_exists[mp_progress] & Edge_Weight) {
                        if ((status = PutAsciiData(tk,"Weights", mp_eweights[mp_progress])) != TK_Normal)
                            return status;
                    }
                    mp_progress++;
                }
                mp_progress = 0;
				mp_substage++;;
					}   nobreak;
			
			case 6: {
				if ((status = PutEndXMLTag(tk,"Edge_Weights")) != TK_Normal)
					return status;
				mp_substage = 0;
					}   break;
            default:
                return tk.Error("internal error in write_edge_weights (2)");
        }
    }
	
    return status;
#else
return tk.Error(stream_disable_ascii);
#endif 
} //end function TK_Polyhedron::write_edge_weights


TK_Status TK_Polyhedron::write_edge_normals_ascii (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII

	TK_Status       status = TK_Normal;
	
	

    if (mp_enormalcount == mp_edgecount) {
        switch (mp_substage) {
            //opcode
			case 0: {
				if ((status = PutStartXMLTag(tk,"Edge_Normals")) != TK_Normal)
					return status;
				mp_substage++;
					}   nobreak;

            case 1: {
				PutTab t(&tk);
                mp_optopcode = OPT_ALL_EDGE_NORMALS_POLAR;
                if ((status = PutAsciiHex(tk,"Optional_Opcode", mp_optopcode)) != TK_Normal)
                    return status;
                mp_substage++;
            }   nobreak;

            //compression scheme
            case 2: {
				PutTab t(&tk);
                mp_compression_scheme = CS_NONE;
                if ((status = PutAsciiData(tk,"Compression_Scheme", (int)mp_compression_scheme)) != TK_Normal)
                    return status;
                normals_cartesian_to_polar(null, Edge_Normal, mp_edgecount, mp_enormals, mp_enormals);
                mp_substage++;
            }   nobreak;

            //all normals
            case 3: {
				PutTab t(&tk);
                if ((status = PutAsciiData(tk,"Edge_Normals", mp_enormals, mp_enormalcount*2)) != TK_Normal)
                    return status;
				mp_substage++;
					}   nobreak;

			case 4: {

				if ((status = PutEndXMLTag(tk,"Edge_Normals")) != TK_Normal)
					return status;
				mp_substage = 0;
					}   break;

            default:
                return tk.Error("internal error in write_edge_normals (1)");
        }
    }
    else {
        switch (mp_substage) {
            //opcode
			case 0: {
				if ((status = PutStartXMLTag(tk,"Edge_Normals")) != TK_Normal)
					return status;
				mp_substage++;
					}   nobreak;

            case 1: {
				PutTab t(&tk);
                mp_optopcode = OPT_EDGE_NORMALS_POLAR;
                if ((status = PutAsciiHex(tk,"Optional_Opcode", mp_optopcode)) != TK_Normal)
                    return status;
                mp_substage++;
            }   nobreak;

            //compression scheme
            case 2: {
				PutTab t(&tk);
                mp_compression_scheme = CS_NONE;
                if ((status = PutAsciiData(tk,"Compression_Scheme",(int) mp_compression_scheme)) != TK_Normal)
                    return status;
                mp_substage++;
            }   nobreak;

            //count
            case 3: {
				PutTab t(&tk);
                if ((status = PutAsciiData(tk,"Edge_Normal_Count", mp_enormalcount)) != TK_Normal)
                    return status;
                mp_substage++;
                mp_progress = 0;
            }   nobreak;

            //and indices of existing values
            case 4: {
				PutTab t(&tk);
                while (mp_progress < mp_edgecount) {
                    if (mp_edge_exists[mp_progress] & Edge_Normal) {
                        if (mp_edgecount < 256) {
                            unsigned char       byte = (unsigned char)mp_progress;

                            if ((status = PutAsciiData(tk,"Progress", byte)) != TK_Normal)
                                return status;
                        }
                        else if (mp_edgecount < 65536) {
                            unsigned short      word = (unsigned short)mp_progress;

                            if ((status = PutAsciiData(tk,"Progress", word)) != TK_Normal)
                                return status;
                        }
                        else {
                            if ((status = PutAsciiData(tk,"Progress", mp_progress)) != TK_Normal)
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
            case 5: {
				PutTab t(&tk);
                while (mp_progress < mp_edgecount) {
                    if (mp_edge_exists[mp_progress] & Edge_Normal) {
                        if ((status = PutAsciiData(tk,"Edge_Normals", &mp_enormals[2*mp_progress], 2)) != TK_Normal)
                            return status;
                    }
                    mp_progress++;
                }
                mp_progress = 0;
				mp_substage++;
		}   nobreak;
			case 6: {
				if ((status = PutEndXMLTag(tk,"Edge_Normals")) != TK_Normal)
					return status;
				mp_substage = 0;
					}   break;

            default:
                return tk.Error("internal error in write_edge_normals (2)");
        }
    }
	
    return status;
#else
return tk.Error(stream_disable_ascii);
#endif 
} //end function TK_Polyhedron::write_edge_normals


TK_Status TK_Polyhedron::WriteAscii (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII
    
	TK_Status       status = TK_Normal;	
	
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

                if ((status = PutAsciiData(tk,"OPT_ENTITY_ATTRIBUTES", op)) != TK_Normal)
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
            if ((status = PutAsciiHex(tk,"Optional_Opcode", mp_optopcode)) != TK_Normal)
                return status;
			mp_substage = 0;
				 }   break;
        default:
            return tk.Error();
    }
    return status;
#else
return tk.Error(stream_disable_ascii);
#endif 
} // end function TK_Polyhedron::Write



//protected virtual
TK_Status TK_Polyhedron::read_trivial_points_ascii (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII

	TK_Status       status = TK_Normal;
    switch (mp_substage) {
		case 0: {
				if ((status = ReadAsciiWord(tk,0)) != TK_Normal)
					return status;
				mp_substage++;
			}   nobreak;
        //unused
        case 1: {
            mp_substage++;
        }   nobreak;
        
        //compression scheme
        case 2: {
            if ((status = GetAsciiData(tk,"Compression_Scheme", mp_compression_scheme)) != TK_Normal)
                return status;
            mp_substage++;
        } nobreak;

        //number of points
        case 3: {
			if ((status = GetAsciiData(tk, "Point_Count", mp_pointcount)) != TK_Normal)
                return status;
            if (mp_pointcount == 0)
                return status;
             mp_substage++;
        } nobreak;

        // get bounding
		case 4: {
			if( mp_pointcount != 0)
				if ((status = GetAsciiData(tk, "Points",  mp_points, 3 * mp_pointcount)) != TK_Normal)
					return status;
			  mp_substage++; 
        }   nobreak;

		case 5: {
			if ((status = ReadAsciiWord(tk,0)) != TK_Normal)
				return status;
			mp_substage = 0;
				}   break;
         default:
            return tk.Error();
    }
    return status;
#else
	return tk.Error(stream_disable_ascii);
#endif 
} //end function TK_Polyhedron::read_trivial_points


TK_Status TK_Polyhedron::read_vertex_normals_compressed_all_ascii (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII

    TK_Status       status;
    ASSERT(mp_optopcode == OPT_ALL_NORMALS_COMPRESSED);
    switch (mp_substage) {
        //compression scheme
		case 0: {
				if ((status = ReadAsciiWord(tk,0)) != TK_Normal)
					return status;
				mp_substage++;
			}   nobreak;

        case 1: {
			if ((status = GetAsciiData(tk, "Compression_Scheme", mp_compression_scheme)) != TK_Normal)
                return status;
            mp_substage++;
        }   nobreak;

        // get number of bits per sample
        case 2: {
			if ((status = GetAsciiData(tk, "Bits_Per_Sample", mp_bits_per_sample)) != TK_Normal)
                return status;
            mp_substage++;
        }   nobreak;
        
        case 3: {
			if ((status = GetAsciiData(tk, "Vertex_Normal_Count", mp_pointcount)) != TK_Normal)
                return status;

			mp_normalcount = mp_pointcount;
			if(mp_pointcount != 0)
				ENSURE_ALLOCATED(mp_normals, float,  3*mp_pointcount);

            mp_substage++;
        }   nobreak;

        // get number of bits per sample
        case 4: {
			if(mp_pointcount != 0){
				if ((status = GetAsciiData(tk, "Vertex_Normals",mp_normals, 3* mp_pointcount)) != TK_Normal)
					return status;
			}
             mp_substage++; 
        }   nobreak;

		case 5: {
			if ((status = ReadAsciiWord(tk,0)) != TK_Normal)
				return status;
			mp_substage = 0;
				}   break;
         default:
            return tk.Error();
    }
    return status;
#else
	return tk.Error(stream_disable_ascii);
#endif 

}  // end function read_vertex_normals_compressed_all

TK_Status TK_Polyhedron::read_vertex_normals_compressed_ascii (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII
    
	TK_Status       status = TK_Normal;
 
    ASSERT(mp_optopcode == OPT_NORMALS_COMPRESSED);
    switch (mp_substage) {

        //compression scheme
		case 0: {
				if ((status = ReadAsciiWord(tk,0)) != TK_Normal)
					return status;
				mp_substage++;
			}   nobreak;

        case 1: {
			if ((status = GetAsciiData(tk, "Vertex_Normal_Count", mp_normalcount)) != TK_Normal)
                return status;

			if (mp_normalcount != 0)
				ENSURE_ALLOCATED(mp_normals, float, 3 * mp_normalcount);

			mp_substage++;
        }   nobreak;

        // get compression size
        case 2: {
        	if (mp_normalcount != 0){
				if ((status = GetAsciiData(tk, "Vertex_Normals",mp_normals, 3 * mp_normalcount)) != TK_Normal)
					return status;
			}
                
            mp_substage++; 
        }   nobreak;

		case 3: {
			if ((status = ReadAsciiWord(tk,0)) != TK_Normal)
				return status;
			mp_substage = 0;
				}   break;

		default:
        return tk.Error(
            "internal error in function read_vertex_normals_compressed (version<650)");
        
    }
    return status;
#else
	return tk.Error(stream_disable_ascii);
#endif 
} //end function read_vertex_normals_compressed


TK_Status TK_Polyhedron::read_vertex_normals_all_ascii (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII

	TK_Status       status;
    ASSERT(mp_optopcode == OPT_ALL_NORMALS ||
            mp_optopcode == OPT_ALL_NORMALS_POLAR);
    switch (mp_substage) {
		
		case 0: {
				if ((status = ReadAsciiWord(tk,0)) != TK_Normal)
					return status;
				mp_substage++;
			}   nobreak;
		case 1: {
            set_normals();
            mp_substage++;
        } nobreak;
      
        case 2: {
            if (mp_optopcode == OPT_ALL_NORMALS_POLAR) {
				if ((status = GetAsciiData(tk, "Normals", mp_normals, 2 * mp_pointcount)) != TK_Normal)
                    return status;
                normals_polar_to_cartesian(null, Vertex_Normal, mp_pointcount, mp_normals, mp_normals);
            }
            else {
                if ((status = GetAsciiData(tk, "Normals", mp_normals, 3 * mp_pointcount)) != TK_Normal)
                    return status;
            }
           
            mp_normalcount = mp_pointcount;
         mp_substage++; 
        }   nobreak;

		case 3: {
			if ((status = ReadAsciiWord(tk,0)) != TK_Normal)
				return status;
			mp_substage = 0;
				}   break;
        default:
            return tk.Error();
    }
    return status;
#else
	return tk.Error(stream_disable_ascii);
#endif 
} //end function read_vertex_normals_all


TK_Status TK_Polyhedron::read_vertex_normals_ascii (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII
    
	TK_Status       status;
    unsigned char   byte;
    int             index;

    ASSERT(mp_optopcode == OPT_NORMALS ||
            mp_optopcode == OPT_NORMALS_POLAR);
    switch (mp_substage) {

        //if all points do NOT have normals, number of individual normals
		case 0: {
				if ((status = ReadAsciiWord(tk,0)) != TK_Normal)
					return status;
				mp_substage++;
			}   nobreak;

        case 1: {
            if (mp_pointcount < 256) {
				if ((status = GetAsciiData(tk, "Normal_Count", byte)) != TK_Normal)
                    return status;
                mp_normalcount = m_byte;
            }
            else if (mp_pointcount < 65536) {
                if ((status = GetAsciiData(tk, "Normal_Count", m_unsigned_short)) != TK_Normal)
                    return status;
                mp_normalcount = m_unsigned_short;
            }
            else {
                if ((status = GetAsciiData(tk, "Normal_Count", mp_normalcount)) != TK_Normal)
                    return status;
            }
            if (mp_normalcount > mp_pointcount)
                return tk.Error ("invalid vertex normal count in TK_Polyhedron::read_vertex_normals");
            mp_progress = 0;
            set_normals();
            mp_substage++;
        }   nobreak;

        // and indices of existing normals
        case 2: {
            // read "normalcount" worth of indices & flag them
            while (mp_progress < mp_normalcount) {
                if (mp_pointcount < 256) {
					if ((status = GetAsciiData(tk, "Progress", m_byte)) != TK_Normal)
                        return status;
                    index = m_byte;
                }
                else if (mp_pointcount < 65536) {
                    if ((status = GetAsciiData(tk, "Progress", m_unsigned_short)) != TK_Normal)
                        return status;
                    index = m_unsigned_short;
                }
                else {
                    if ((status = GetAsciiData(tk, "Progress", m_int)) != TK_Normal)
                        return status;
					index = m_int;
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
        case 3: {
            // read normals associated with the flagged indices
            while (mp_progress < mp_pointcount) {
                if (mp_exists[mp_progress] & Vertex_Normal) {
                    if (mp_optopcode == OPT_NORMALS_POLAR) {
						if ((status = GetAsciiData(tk, "Normals", &mp_normals[2*mp_progress], 2)) != TK_Normal)
                            return status;
                    }
                    else {
						if ((status = GetAsciiData(tk, "Normals", &mp_normals[3*mp_progress],3)) != TK_Normal)
                            return status;
                    }
                }
                mp_progress++;
            }
            if (mp_optopcode == OPT_NORMALS_POLAR)
                normals_polar_to_cartesian(mp_exists, Vertex_Normal, mp_pointcount, mp_normals, mp_normals);
            mp_progress = 0;
             mp_substage++; 
        }   nobreak;

		case 4: {
			if ((status = ReadAsciiWord(tk,0)) != TK_Normal)
				return status;
			mp_substage = 0;
				}   break;

        default:
            return tk.Error();
    }    

    return TK_Normal;
#else
	return tk.Error(stream_disable_ascii);
#endif 
} //end function TK_Polyhedron::read_vertex_normals


TK_Status TK_Polyhedron::read_vertex_parameters_all_ascii(BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII
   
	TK_Status       status;
    ASSERT(mp_optopcode == OPT_ALL_PARAMETERS_COMPRESSED ||
           mp_optopcode == OPT_ALL_PARAMETERS);
    if (mp_optopcode == OPT_ALL_PARAMETERS) {
        switch (mp_substage) {

            // get parameter width
			case 0: {
				if ((status = ReadAsciiWord(tk,0)) != TK_Normal)
					return status;
				mp_substage++;
					}   nobreak;

            case 1: {
				if ((status = GetAsciiData(tk, "Parameter_Width", m_int)) != TK_Normal)
                    return status;
				mp_paramwidth = (char) m_int;
                SetVertexParameters(null, mp_paramwidth);
                mp_substage++;
            }   nobreak;

            //get all the params
            case 2: {
				if ((status = GetAsciiData(tk, "All_Parameters", mp_params, mp_pointcount*mp_paramwidth)) != TK_Normal)
                    return status;
				mp_substage++;
			} nobreak;

			case 3: {
				if ((status = ReadAsciiWord(tk,0)) != TK_Normal)
					return status;
				  mp_substage++; 
				  }   nobreak;

			case 4: {
				if ((status = ReadAsciiWord(tk,0)) != TK_Normal)
					return status;
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
				if ((status = ReadAsciiWord(tk,0)) != TK_Normal)
					return status;
				mp_substage++;
					}   nobreak;

            case 1: {
				if ((status = GetAsciiData(tk, "Compression_Scheme", mp_compression_scheme)) != TK_Normal)
                    return status;
                ASSERT(mp_compression_scheme == CS_TRIVIAL);
                mp_substage++;
            }   nobreak;

            // get parameter width
            case 2: {
                if (tk.GetVersion() >= 1175) {
					if ((status = GetAsciiData(tk, "Parameter_Width", m_int)) != TK_Normal)
                        return status;
					mp_paramwidth = (char) m_int;
				 }
                else
                    mp_paramwidth = 3;
				
				ENSURE_ALLOCATED(mp_params, float, m_int * mp_pointcount);
				mp_paramcount = mp_pointcount;

                mp_substage++;
            }   nobreak;

			case 3: {
				if ((status = GetAsciiData(tk,"All_Parameters", mp_params, m_int * mp_pointcount)) != TK_Normal)
					return status;
				mp_substage++;
			} nobreak;
				
            //the bounding box (note: order different from ver<650)
            case 4: {
				if ((status = GetAsciiData(tk, "Bounding_Box", mp_bbox, 2* (int)mp_paramwidth)) != TK_Normal)
                    return status;
                mp_substage++;
            } nobreak;

            // get compression size
            case 5: {
				if ((status = GetAsciiData(tk, "Bits_Per_Sample", mp_bits_per_sample)) != TK_Normal)
                    return status;
				mp_substage++;
					} nobreak;

			case 6: {
				if ((status = ReadAsciiWord(tk,0)) != TK_Normal)
					return status;
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
				if ((status = ReadAsciiWord(tk,0)) != TK_Normal)
					return status;
				mp_substage++;
				}   nobreak;

            case 1: {
				if ((status = GetAsciiData(tk, "Compression_Scheme", mp_compression_scheme)) != TK_Normal)
                    return status;
                ASSERT(mp_compression_scheme == CS_TRIVIAL);
                mp_substage++;
            }   nobreak;

            // get compression size
            case 2: {
				if ((status = GetAsciiData(tk, "Bits_Per_Sample", mp_bits_per_sample)) != TK_Normal)
                    return status;
                mp_substage++;
            }   nobreak;

            //the bounding box (note: order different from ver >= 650)
            case 3: {
				if ((status = GetAsciiData(tk, "Bounding_Box", mp_bbox, 6)) != TK_Normal)
                    return status;

				int int_paramwidth = atoi(&mp_paramwidth);
				ENSURE_ALLOCATED(mp_params, float, int_paramwidth * mp_pointcount);

                mp_substage++;
            } nobreak;

			case 4: {
				int int_paramwidth = atoi(&mp_paramwidth);
				if ((status = GetAsciiData(tk,"All_Parameters", mp_params, int_paramwidth * mp_pointcount)) != TK_Normal)
					return status;
                mp_paramcount = mp_pointcount;
				mp_substage++;
				} nobreak;

			case 5: {
				// read the end XML tag </All_Parameters>
				if ((status = ReadAsciiWord(tk,0)) != TK_Normal)
					return status;
				  mp_substage++; 
				}   nobreak;

			case 6: {
				if ((status = ReadAsciiWord(tk,0)) != TK_Normal)
					return status;
				mp_substage = 0;
					}   break;
            default:
                return tk.Error();
        }
    }
    return TK_Normal;
#else
return tk.Error(stream_disable_ascii);
#endif 
} // end function TK_Polyhedron::read_vertex_parameters_all



TK_Status TK_Polyhedron::read_vertex_parameters_ascii(BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII

	TK_Status       status;
    ASSERT(mp_optopcode == OPT_PARAMETERS_COMPRESSED ||
	    mp_optopcode == OPT_PARAMETERS);

    if (mp_optopcode == OPT_PARAMETERS) {
        switch (mp_substage) {

            // get parameter width
			case 0: {
				if ((status = ReadAsciiWord(tk,0)) != TK_Normal)
					return status;
				mp_substage++;
			}   nobreak;

            case 1: {
				if ((status = GetAsciiData(tk, "Parameter_Width", mp_paramwidth)) != TK_Normal)
				          return status;
				
                mp_substage++;
            }   nobreak;

            //if all points do NOT have params, number of individual params
            case 2: {
                if (mp_pointcount < 256) {
					if ((status = GetAsciiData(tk, "Parameter_Count", m_byte)) != TK_Normal)
                        return status;
                    mp_paramcount = m_byte;
                }
                else if (mp_pointcount < 65536) {
		            if ((status = GetAsciiData(tk, "Parameter_Count", m_unsigned_short)) != TK_Normal)
                        return status;
                    mp_paramcount = m_unsigned_short;
                }
                else {
                    if ((status = GetAsciiData(tk, "Parameter_Count", mp_paramcount)) != TK_Normal)
                        return status;
                }
                if (mp_paramcount > mp_pointcount)
                    return tk.Error ("invalid vertex parameter count in TK_Polyhedron::read_vertex_parameters");
                mp_progress = 0;
                mp_substage++;
            }   nobreak;

            // and indices of existing params
            case 3: {
				int int_paramwidth = atoi(&mp_paramwidth);
                // get "paramcount" worth of indices & flag them
                while (mp_progress < mp_paramcount) {
                    int     index;

                    if (mp_pointcount < 256) {
						if ((status = GetAsciiData(tk, "Parameter_Indices", m_byte)) != TK_Normal)
                            return status;
                        index = m_byte;
                    }
                    else if (mp_pointcount < 65536) {
                       

                        if ((status = GetAsciiData(tk, "Parameter_Indices", m_unsigned_short)) != TK_Normal)
                            return status;
                        index = m_unsigned_short;
                    }
                    else {
                        if ((status = GetAsciiData(tk, "Parameter_Indices", m_int)) != TK_Normal)
                            return status;
						index  = m_int;
                    }
                    if (index > mp_pointcount)
                        return tk.Error("invalid vertex parameter index");
                    mp_exists[index] |= Vertex_Parameter;
                    mp_progress++;
                }
                mp_progress = 0;
				SetVertexParameters(null, int_paramwidth);
                mp_substage++;
            }   nobreak;

	    //the individual parameters
	    case 4: {
		// read paramters associated with the flagged indices
		int int_paramwidth = atoi(&mp_paramwidth);
		while (mp_progress < mp_pointcount) {
		    if (mp_exists[mp_progress] & Vertex_Parameter) {
				if ((status = GetAsciiData(tk, "Parameters", &mp_params[int_paramwidth*mp_progress], int_paramwidth)) != TK_Normal)
			    return status;
		    }
		    mp_progress++;
		}
		mp_progress = 0;
		  mp_substage++; 
				}   nobreak;

			case 5: {
				if ((status = ReadAsciiWord(tk,0)) != TK_Normal)
					return status;
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
				if ((status = ReadAsciiWord(tk,0)) != TK_Normal)
					return status;
				mp_substage++;
			}   nobreak;

            case 1: {
				if ((status = GetAsciiData(tk, "Compression_Scheme", mp_compression_scheme)) != TK_Normal)
                    return status;
                ASSERT(mp_compression_scheme == CS_TRIVIAL);
                mp_substage++;
            }   nobreak;

            // get parameter width
            case 2: {
                if (tk.GetVersion() >= 1175) {
					if ((status = GetAsciiData(tk, "Parameter_Width", mp_paramwidth)) != TK_Normal)
					             return status;
                }
                else
                    mp_paramwidth = 3;
                mp_substage++;
            }   nobreak;

            //the bounding box (note: order different from ver<650)
            case 3: {
				int int_paramwidth = atoi(&mp_paramwidth);
				if ((status = GetAsciiData(tk, "Bounding_Box", mp_bbox, 2*int_paramwidth)) != TK_Normal)
                    return status;
                mp_substage++;
            } nobreak;

            // get compression size
            case 4: {
				if ((status = GetAsciiData(tk, "Bits_Per_Sample", mp_bits_per_sample)) != TK_Normal)
                    return status;
                mp_substage++;
            }   nobreak;

            //if all points do NOT have params, number of individual params
            case 5: {
                if (mp_pointcount < 256) {
					if ((status = GetAsciiData(tk, "Parameter_Count", m_byte)) != TK_Normal)
                        return status;
                    mp_paramcount = m_byte;
                }
                else if (mp_pointcount < 65536) {

                    if ((status = GetAsciiData(tk, "Parameter_Count",m_unsigned_short)) != TK_Normal)
                        return status;
                    mp_paramcount = m_unsigned_short;
                }
                else {
                    if ((status = GetAsciiData(tk, "Parameter_Count", mp_paramcount)) != TK_Normal)
                        return status;
                }
                if (mp_paramcount > mp_pointcount)
                    return tk.Error ("invalid vertex parameter count in TK_Polyhedron::read_vertex_parameters");
                mp_progress = 0;
                mp_substage++;
            }   nobreak;

            // and indices of existing params
            case 6: {
                // get "paramcount" worth of indices & flag them
                while (mp_progress < mp_paramcount) {
                    int     index;

                    if (mp_pointcount < 256) {
						if ((status = GetAsciiData(tk,"Parameter_Indices", m_byte)) != TK_Normal)
                            return status;
                        index = m_byte;
                    }
                    else if (mp_pointcount < 65536) {
                        if ((status = GetAsciiData(tk,"Parameter_Indices", m_unsigned_short)) != TK_Normal)
                            return status;
                        index = m_unsigned_short;
                    }
                    else {
                        if ((status = GetAsciiData(tk, "Parameter_Indices", m_int)) != TK_Normal)
                            return status;
						index = m_int; 
                    }
                    if (index > mp_pointcount)
                        return tk.Error("invalid vertex parameter index");
                    mp_exists[index] |= Vertex_Parameter;
                    mp_progress++;
                }
				int int_paramwidth = atoi(&mp_paramwidth);
				ENSURE_ALLOCATED(mp_params, float, int_paramwidth * mp_pointcount);
                mp_progress = 0;
                mp_substage++;
            }   nobreak;
                     		
			case 7: {
				int int_paramwidth = atoi(&mp_paramwidth);
				if ((status = GetAsciiData(tk,"All_Parameters", mp_params, int_paramwidth * mp_pointcount)) != TK_Normal)
					return status;
				  mp_substage++; 
				}   nobreak;

			case 8: {
				if ((status = ReadAsciiWord(tk,0)) != TK_Normal)
					return status;
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
				if ((status = ReadAsciiWord(tk,0)) != TK_Normal)
					return status;
				mp_substage++;
			}   nobreak;

            case 1: {
				if ((status = GetAsciiData(tk, "Compression_Scheme", mp_compression_scheme)) != TK_Normal)
                    return status;
                ASSERT(mp_compression_scheme == CS_TRIVIAL);
                mp_substage++;
            }   nobreak;

            // get compression size
            case 2: {
				if ((status = GetAsciiData(tk, "Bits_Per_Sample", mp_bits_per_sample)) != TK_Normal)
                    return status;
                mp_substage++;
            }   nobreak;

			//parameter width
			case 3: {
				if (tk.GetVersion() >= 1175) {
					if ((status = GetAsciiData(tk,"Parameter_count", mp_paramcount)) != TK_Normal)
						return status;
				}
				mp_substage++;
			} nobreak;

            //the bounding box (note: order different from ver >= 650)
            case 4: {
				int int_paramwidth = atoi(&mp_paramwidth);
				if ((status = GetAsciiData(tk, "Bounding_Box", mp_bbox, 2*int_paramwidth)) != TK_Normal)
                    return status;
                mp_substage++;
            } nobreak;

            //if all points do NOT have params, number of individual params
            case 5: {
                if (mp_pointcount < 256) {
					if ((status = GetAsciiData(tk, "Parameter_Count", m_byte)) != TK_Normal)
                        return status;
                    mp_paramcount = m_byte;
                }
                else if (mp_pointcount < 65536) {
					if ((status = GetAsciiData(tk, "Parameter_Count", m_unsigned_short)) != TK_Normal)
                        return status;
                    mp_paramcount = m_unsigned_short;
                }
                else {
					if ((status = GetAsciiData(tk, "Parameter_Count", mp_paramcount)) != TK_Normal)
                        return status;
                }
                if (mp_paramcount > mp_pointcount)
                    return tk.Error ("invalid vertex parameter count in TK_Polyhedron::read_vertex_parameters");
                mp_progress = 0;
                mp_substage++;
            }   nobreak;

            // and indices of existing params
            case 6: {
                // get "paramcount" worth of indices & flag them
                while (mp_progress < mp_paramcount) {
                    int     index;

                    if (mp_pointcount < 256) {
						if ((status = GetAsciiData(tk, "Parameter_Indices", m_byte)) != TK_Normal)
                            return status;
                        index = m_byte;
                    }
                    else if (mp_pointcount < 65536) {
                        if ((status = GetAsciiData(tk, "Parameter_Indices", m_unsigned_short)) != TK_Normal)
                            return status;
                        index = m_unsigned_short;
                    }
                    else {
                        if ((status = GetAsciiData(tk, "Parameter_Indices", m_int)) != TK_Normal)
                            return status;
						index = m_int; 
                    }
                    if (index > mp_pointcount)
                        return tk.Error("invalid vertex parameter index (2)");
                    mp_exists[index] |= Vertex_Parameter;
                    mp_progress++;
                }
				int int_paramwidth = atoi(&mp_paramwidth);
				ENSURE_ALLOCATED(mp_params, float, int_paramwidth * mp_pointcount);

                mp_progress = 0;
                mp_substage++;
            }   nobreak;

				//put individual parameters
			case 7: {
				int int_paramwidth = atoi(&mp_paramwidth);
				if ((status = GetAsciiData(tk,"All_Parameters", mp_params, int_paramwidth * mp_pointcount)) != TK_Normal)
					return status;
				mp_progress = 0;
				  mp_substage++; 
				}   nobreak;

			case 8: {
				if ((status = ReadAsciiWord(tk,0)) != TK_Normal)
					return status;
				mp_substage = 0;
					}   break;

            default:
                return tk.Error("internal error in function read_vertex_parameters (version<650)");

        }
    }
    return TK_Normal;
#else
return tk.Error(stream_disable_ascii);
#endif 
} // end function TK_Polyhedron::read_vertex_parameters


TK_Status TK_Polyhedron::read_vertex_colors_all_ascii(BStreamFileToolkit & tk, unsigned char op) alter {
#ifndef BSTREAM_DISABLE_ASCII

	TK_Status       status;
    unsigned int    mask;
    int             i=0;

    if (tk.GetVersion() >= 650) {
        switch (mp_substage) {
			case 0: {
				if ((status = ReadAsciiWord(tk,0)) != TK_Normal)
					return status;
				mp_substage++;
			}   nobreak;

            //compression scheme
            case 1: {
                ASSERT(mp_compression_scheme == CS_TRIVIAL);
				if ((status = GetAsciiData(tk, "Compression_Scheme", mp_compression_scheme)) != TK_Normal)
                    return status;
                mp_substage++;
            }   nobreak;

            //bits per sample
            case 2: {
				if ((status = GetAsciiData(tk, "Bits_Per_Sample", mp_bits_per_sample)) != TK_Normal)
                    return status;
                mp_substage++;
            } nobreak;

			case 3: {
				if ((status = GetAsciiData(tk, "Point_Count", mp_pointcount)) != TK_Normal)
					return status;

				if( mp_pointcount != 0)
				{
					switch (op) {
						case OPT_ALL_VFCOLORS:
							ENSURE_ALLOCATED(mp_vfcolors, float, 3*mp_pointcount);
							break;
						case OPT_ALL_VECOLORS:
							ENSURE_ALLOCATED(mp_vecolors, float, 3*mp_pointcount);
							break;
						case OPT_ALL_VMCOLORS:
							ENSURE_ALLOCATED(mp_vmcolors, float, 3*mp_pointcount);
							break;
						default:
							return tk.Error("internal error in write_vertex_colors_all: unrecognized case");
					}
				}

				mp_substage++;
			}   nobreak;

			case 4: {
				if( mp_pointcount != 0)
				{
					switch (op) 
					{
						case OPT_ALL_VFCOLORS:
							{
								if ((status = GetAsciiData(tk, "Vertex_Face_Color",  mp_vfcolors,  3*mp_pointcount)) != TK_Normal)
									return status;
								mask = Vertex_Face_Color;
								mp_vfcolorcount = mp_pointcount;
							}
							break;
						case OPT_ALL_VECOLORS:
							{
								if ((status = GetAsciiData(tk, "Vertex_Edge_Color",  mp_vecolors,  3*mp_pointcount)) != TK_Normal)
									return status;
								mask = Vertex_Edge_Color;
								mp_vecolorcount = mp_pointcount;
							}
							break;
						case OPT_ALL_VMCOLORS:
							{
								if ((status = GetAsciiData(tk, "Vertex_Marker_Color",  mp_vmcolors,  3*mp_pointcount)) != TK_Normal)
									return status;
								mask = Vertex_Marker_Color;
								mp_vmcolorcount = mp_pointcount;
							}
							break;
						default:
							return tk.Error("internal error in write_vertex_colors_all: unrecognized case");
					}
					for (i = 0; i < mp_pointcount; i++)
						mp_exists[i] |= mask;
				}
				  mp_substage++;
			} nobreak;

			case 5: {
				if ((status = ReadAsciiWord(tk,0)) != TK_Normal)
					return status;
				mp_substage = 0;
			}   break;

            default:
                return tk.Error("internal error (1) in function read_vertex_colors_all");
        }
    }
    else {
        switch (mp_substage) {

			case 0: {
				if ((status = ReadAsciiWord(tk,0)) != TK_Normal)
					return status;

				if( mp_pointcount != 0)
				{
					switch (op) {
						case OPT_ALL_VFCOLORS:
							ENSURE_ALLOCATED(mp_vfcolors, float, 3*mp_pointcount);
							break;
						case OPT_ALL_VECOLORS:
							ENSURE_ALLOCATED(mp_vecolors, float, 3*mp_pointcount);
							break;
						case OPT_ALL_VMCOLORS:
							ENSURE_ALLOCATED(mp_vmcolors, float, 3*mp_pointcount);
							break;
						default:
							return tk.Error("internal error in write_vertex_colors_all: unrecognized case");
					}
				}
				mp_substage++;
			}   nobreak;

			case 1: 
			{
				if( mp_pointcount != 0)
				{
					switch (op) 
					{
					case OPT_ALL_VFCOLORS:
						{
							if ((status = GetAsciiData(tk, "Vertex_Face_Color",  mp_vfcolors,  3*mp_pointcount)) != TK_Normal)
								return status;
							mask = Vertex_Face_Color;
							mp_vfcolorcount = mp_pointcount;
						}
						break;
					case OPT_ALL_VECOLORS:
						{
							if ((status = GetAsciiData(tk, "Vertex_Edge_Color",  mp_vecolors,  3*mp_pointcount)) != TK_Normal)
								return status;
							mask = Vertex_Edge_Color;
							mp_vecolorcount = mp_pointcount;
						}
						break;
					case OPT_ALL_VMCOLORS:
						{
							if ((status = GetAsciiData(tk, "Vertex_Marker_Color",  mp_vmcolors,  3*mp_pointcount)) != TK_Normal)
								return status;
							mask = Vertex_Marker_Color;
							mp_vmcolorcount = mp_pointcount;
						}
						break;
					default:
						return tk.Error("internal error in write_vertex_colors_all: unrecognized case");
					}
					for (i = 0; i < mp_pointcount; i++)
						mp_exists[i] |= mask;
				}
				mp_substage++;
			} nobreak;

			case 3: {
				if ((status = ReadAsciiWord(tk,0)) != TK_Normal)
					return status;
				mp_substage = 0;
					}   break;

			 default:
                return tk.Error("internal error (1) in function TK_Polyhedron::read_vertex_colors_all(version < 650)");
        }
    }
    return TK_Normal;
#else
	UNREFERENCED (op);
	return tk.Error(stream_disable_ascii);
#endif 
} // end function TK_Polyhedron::read_vertex_colors_all

TK_Status TK_Polyhedron::read_vertex_colors_ascii(BStreamFileToolkit & tk, unsigned char op) alter {
#ifndef BSTREAM_DISABLE_ASCII

	TK_Status       status;
    float           *array = null;
    int				count = 0;
    unsigned int    mask;

    switch (op) {
        case OPT_VERTEX_FCOLORS:	mask = Vertex_Face_Color;	break;
        case OPT_VERTEX_ECOLORS:	mask = Vertex_Edge_Color;	break;
        case OPT_VERTEX_MCOLORS:	mask = Vertex_Marker_Color;	break;
        default:
            return tk.Error("internal error in TK_Polyhedron::read_vertex_colors: unrecognized case");
    }

    if (tk.GetVersion() >= 650) {
        switch (mp_substage) {
			case 0: {
				if ((status = ReadAsciiWord(tk,0)) != TK_Normal)
					return status;
				mp_substage++;
			}   nobreak;

            //compression scheme
            case 1: {
                ASSERT(mp_compression_scheme == CS_TRIVIAL);
				if ((status = GetAsciiData(tk,"Compression_Scheme", mp_compression_scheme)) != TK_Normal)
                    return status;
                mp_substage++;
            }   nobreak;

            //bits per sample
            case 2: {
				if ((status = GetAsciiData(tk, "Bits_Per_Sample", mp_bits_per_sample)) != TK_Normal)
                    return status;
                mp_substage++;
            } nobreak;

            //all points do NOT have colors, number of individual colors
            case 3: {
                if (mp_pointcount < 256) {
					if ((status = GetAsciiData(tk, "Count", m_byte)) != TK_Normal)
                        return status;
                    count = m_byte;
                }
                else if (mp_pointcount < 65536) {
					if ((status = GetAsciiData(tk, "Count", m_unsigned_short)) != TK_Normal)
                        return status;
                    count = m_unsigned_short;
                }
                else {
					if ((status = GetAsciiData(tk, "Count", count)) != TK_Normal)
                        return status;
                }
                if (count > mp_pointcount)
                    return tk.Error ("invalid vertex color count in TK_Polyhedron::read_vertex_colors");

                switch (op) {
                    case OPT_VERTEX_FCOLORS:	mp_vfcolorcount = count;	break;
                    case OPT_VERTEX_ECOLORS:	mp_vecolorcount = count;	break;
                    case OPT_VERTEX_MCOLORS:	mp_vmcolorcount = count;	break;
                    default:
                        return tk.Error("internal error (3) in TK_Polyhedron::read_vertex_colors: unrecognized case");
                }
                mp_progress = 0; //just in case
                mp_substage++;
            }   nobreak;

            // and indices of existing colors
            case 4: {
                // recover count in case we hit a pending and are returning to this loop
                switch (op) {
                    case OPT_VERTEX_FCOLORS:    count = mp_vfcolorcount;    break;
                    case OPT_VERTEX_ECOLORS:    count = mp_vecolorcount;    break;
                    case OPT_VERTEX_MCOLORS:    count = mp_vmcolorcount;    break;
                }
                while (mp_progress < count) {
                    int     index;

                    if (mp_pointcount < 256) {
                       

						if ((status = GetAsciiData(tk, "Progress", m_byte)) != TK_Normal)
                            return status;
                        index = m_byte;
                    }
                    else if (mp_pointcount < 65536) {
                       

						if ((status = GetAsciiData(tk, "Progress", m_unsigned_short)) != TK_Normal)
                            return status;
                        index = m_unsigned_short;
                    }
                    else {
						if ((status = GetAsciiData(tk, "Progress", m_int)) != TK_Normal)
                            return status;
						index = m_int; 
                    }
                    if (index > mp_pointcount)
                        return tk.Error("invalid vertex index during set color");
                    mp_exists[index] |= mask;
                    mp_progress++;
                }
                mp_progress = 0;
                mp_substage++;
            }   nobreak;


			case 5: {
				char which_count[256];
				switch (op) {
					case OPT_VERTEX_FCOLORS:	strcpy(which_count, "Vertex_Face_Color_Count");		break;
					case OPT_VERTEX_ECOLORS:	strcpy(which_count, "Vertex_Edge_Color_Count");		break;
					case OPT_VERTEX_MCOLORS:	strcpy(which_count, "Vertex_Marker_Color_Count");	break;
					default:
						return tk.Error("internal error (2) in TK_Polyhedron::read_vertex_colors: unrecognized case");
				}

				if ((status = GetAsciiData(tk, which_count, count)) != TK_Normal)
					return status;

				if( count != 0)
				{
					switch (op) {
						case OPT_VERTEX_FCOLORS:	ENSURE_ALLOCATED(mp_vfcolors, float, 3*mp_pointcount);	break;
						case OPT_VERTEX_ECOLORS:	ENSURE_ALLOCATED(mp_vecolors, float, 3*mp_pointcount);	break;
						case OPT_VERTEX_MCOLORS:	ENSURE_ALLOCATED(mp_vmcolors, float, 3*mp_pointcount);	break;
						default:
							return tk.Error("internal error (2) in TK_Polyhedron::read_vertex_colors: unrecognized case");
					}
				}

				mp_substage++;
			}   nobreak;

			case 6: {
				char what[256];
				switch (op) {
						case OPT_VERTEX_FCOLORS:	
							strcpy(what, "Vertex_Face_Color");
							array = mp_vfcolors;
							break;
						case OPT_VERTEX_ECOLORS:	
							strcpy(what, "Vertex_Edge_Color");
							array = mp_vecolors;
							break;
						case OPT_VERTEX_MCOLORS:	
							strcpy(what, "Vertex_Marker_Color");
							array = mp_vmcolors;
							break;
						default:
							return tk.Error("internal error in TK_Polyhedron::read_vertex_indices: unrecognized case");
					}

				if( count != 0)
					if ((status = GetAsciiData(tk, what,  array, 3*count)) != TK_Normal)  // Should it be 3*count ????
    					return status;
				mp_substage++;
			}   nobreak;
   				
			case 7:{	

				/*	// Should we doing something with the 'mask here? - Rajesh B (15-Aug-06)
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
					*/

			  mp_substage++; 
			}   nobreak;

			case 8: {
				if ((status = ReadAsciiWord(tk,0)) != TK_Normal)
					return status;
				mp_substage = 0;
					}   break;
             default:
                return tk.Error("internal error (1) in function read_vertex_colors");
        }
    }
    else {
        switch (mp_substage) {
			case 0: {
				if ((status = ReadAsciiWord(tk,0)) != TK_Normal)
					return status;
				mp_substage++;
			}   nobreak;
            //all points do NOT have colors, number of individual colors
            case 1: {
                if (mp_pointcount < 256) {
					if ((status = GetAsciiData(tk, "Count", m_byte)) != TK_Normal)
                        return status;
                    count = m_byte;
                }
                else if (mp_pointcount < 65536) {
					if ((status = GetAsciiData(tk, "Count", m_unsigned_short)) != TK_Normal)
                        return status;
                    count = m_unsigned_short;
                }
                else {
					if ((status = GetAsciiData(tk, "Count", m_int)) != TK_Normal)
                        return status;
					count = m_int;
                }
                if (count > mp_pointcount)
                    return tk.Error ("invalid vertex color count in TK_Polyhedron::read_vertex_colors");
                switch (op) {
                    case OPT_VERTEX_FCOLORS:	mp_vfcolorcount = count;	break;
                    case OPT_VERTEX_ECOLORS:	mp_vecolorcount = count;	break;
					case OPT_VERTEX_MCOLORS:	mp_vmcolorcount = count;	break;
                    default:
                        return tk.Error("internal error (3) in TK_Polyhedron::read_vertex_colors: unrecognized case (version<650)");
                }
                mp_progress = 0;
                mp_substage++;
            }   nobreak;

            // and indices of existing colors
            case 2: {
                // recover count in case we hit a pending and are returning to this loop
                switch (op) {
                    case OPT_VERTEX_FCOLORS:    count = mp_vfcolorcount;    break;
                    case OPT_VERTEX_ECOLORS:    count = mp_vecolorcount;    break;
                    case OPT_VERTEX_MCOLORS:    count = mp_vmcolorcount;    break;
                }

                while (mp_progress < count) {
                    int     index;

                    if (mp_pointcount < 256) {
						if ((status = GetAsciiData(tk, "Progress", m_byte)) != TK_Normal)
                            return status;

                        index = m_byte;
                    }
                    else if (mp_pointcount < 65536) {
                       

						if ((status = GetAsciiData(tk, "Progress", m_unsigned_short)) != TK_Normal)
                            return status;
                        index = m_unsigned_short;
                    }
                    else {
						if ((status = GetAsciiData(tk, "Progress", m_int)) != TK_Normal)
                            return status;
						index = m_int;
                    }
                    if (index > mp_pointcount)
                        return tk.Error("invalid vertex index during set color(2)");
                    mp_exists[index] |= mask;
                    mp_progress++;
                }

				if( count != 0)
				{
					switch (op) {
						case OPT_VERTEX_FCOLORS:	ENSURE_ALLOCATED(mp_vfcolors, float, 3*mp_pointcount);	break;
						case OPT_VERTEX_ECOLORS:	ENSURE_ALLOCATED(mp_vecolors, float, 3*mp_pointcount);	break;
						case OPT_VERTEX_MCOLORS:	ENSURE_ALLOCATED(mp_vmcolors, float, 3*mp_pointcount);	break;
						default:
							return tk.Error("internal error (2) in TK_Polyhedron::read_vertex_colors: unrecognized case");
					}
				}

                mp_progress = 0; //just in case
                mp_substage++;
            }   nobreak;

            //get all the colors OR the individual colors
            case 3: {

				char what[256];
				switch (op) {
					case OPT_VERTEX_FCOLORS:	
						strcpy(what, "Vertex_Face_Color");
						array = mp_vfcolors;
						break;
					case OPT_VERTEX_ECOLORS:	
						strcpy(what, "Vertex_Edge_Color");
						array = mp_vecolors;
						break;
					case OPT_VERTEX_MCOLORS:	
						strcpy(what, "Vertex_Marker_Color");
						array = mp_vmcolors;
						break;
					default:
						return tk.Error("internal error in TK_Polyhedron::read_vertex_indices: unrecognized case");
				}

				while (mp_progress < mp_pointcount) {
					if ((mp_exists[mp_progress] & mask) == mask) {
						if( mp_pointcount != 0)
					if ((status = GetAsciiData(tk, what, &array[3*mp_progress], 3)) != TK_Normal)
						return status;
				}
				mp_progress++;
			 }
            mp_progress = 0;
            mp_substage++;
            }   nobreak;

			case 4: {
				if ((status = ReadAsciiWord(tk,0)) != TK_Normal)
					return status;
				mp_substage = 0;
			}   break;

            default:
                return tk.Error("internal error (1) in function TK_Polyhedron::read_vertex_colors(version < 650)");
        }
    }
    return TK_Normal;
#else
UNREFERENCED (op);
return tk.Error(stream_disable_ascii);
#endif 
} // end function TK_Polyhedron::read_vertex_colors

TK_Status TK_Polyhedron::write_vertex_colors_main_ascii (BStreamFileToolkit & tk, unsigned char base_op) alter {
#ifndef BSTREAM_DISABLE_ASCII

	TK_Status       status = TK_Normal;
    int             count;
	
	


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
        if ((status = PutAsciiHex(tk,"Optional_Opcode", mp_optopcode)) != TK_Normal)
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
	return tk.Error(stream_disable_ascii);
#endif 
} //end function TK_Polyhedron::write_vertex_colors_main

TK_Status TK_Polyhedron::read_vertex_indices_all_ascii(BStreamFileToolkit & tk, unsigned char op) alter {
#ifndef BSTREAM_DISABLE_ASCII

	TK_Status       status;
    unsigned int    mask;

    if (tk.GetVersion() >= 650) {
        switch (mp_substage) {
			case 0: {
				if ((status = ReadAsciiWord(tk,0)) != TK_Normal)
					return status;
				mp_substage++;
			}   nobreak;

            //compression scheme
            case 1: {
                ASSERT(mp_compression_scheme == CS_TRIVIAL);
                if ((status = GetAsciiData(tk, "Compression_Scheme", mp_compression_scheme)) != TK_Normal)
                    return status;
                mp_substage++;
            }   nobreak;

			//bounding box
            case 2: {
                if ((status = GetAsciiData(tk, "Bounding_Box", mp_bbox, 6)) != TK_Normal)
                    return status;
                mp_substage++;
            } nobreak;
            
            //bits per sample
            case 3: {
                if ((status = GetAsciiData(tk, "Bits_Per_Sample", mp_bits_per_sample)) != TK_Normal)
                    return status;
                mp_substage++;
            } nobreak;

			case 4: {
				if ((status = GetAsciiData(tk, "Point_Count", mp_pointcount)) != TK_Normal)
					return status;

				if( mp_pointcount != 0)
				{
					switch (op) {
						case OPT_ALL_VFINDICES:
							ENSURE_ALLOCATED(mp_vfindices, float, mp_pointcount);
							mp_vfindexcount = mp_pointcount;
							break;
						case OPT_ALL_VEINDICES:
							ENSURE_ALLOCATED(mp_veindices, float, mp_pointcount);
							mp_veindexcount = mp_pointcount;
							break;
						case OPT_ALL_VMINDICES:
							ENSURE_ALLOCATED(mp_vmindices, float, mp_pointcount);
							mp_vmindexcount = mp_pointcount;
							break;
						default:
							return tk.Error("internal error (1) in TK_Polyhedron::read_vertex_indices_all: unrecognized case");
					}
				}
				mp_substage++;
			}   nobreak;

			case 5: {
				if( mp_pointcount != 0)
				{
					switch (op) {
						case OPT_ALL_VFINDICES:
							{
								if ((status = GetAsciiData(tk, "Vertex_Face_Index",  mp_vfindices,  mp_pointcount)) != TK_Normal)
									return status;
								mask = Vertex_Face_Index;
							}
							break;
						case OPT_ALL_VEINDICES:
							{
								mask = Vertex_Edge_Index;
								if ((status = GetAsciiData(tk, "Vertex_Edge_Index",  mp_veindices,  mp_pointcount)) != TK_Normal)
									return status;
							}
							break;
						case OPT_ALL_VMINDICES:
							{
								mask = Vertex_Marker_Index;
								if ((status = GetAsciiData(tk, "Vertex_Marker_Index",  mp_vmindices,  mp_pointcount)) != TK_Normal)
									return status;
							}
							break;
						default:
							return tk.Error("internal error (1) in TK_Polyhedron::read_vertex_indices_all: unrecognized case");
					}

					for (int i = 0; i < mp_pointcount; i++)
						mp_exists[i] |= mask;
				}

				mp_substage++;
			}   nobreak;

			case 6: {
				if ((status = ReadAsciiWord(tk,0)) != TK_Normal)
					return status;
				mp_substage = 0;
			}   break;
			default:
                return tk.Error("internal error (1) in function read_vertex_indices_all");
        }
    }
    else {
        switch (mp_substage) {
			case 0: {
				if ((status = ReadAsciiWord(tk,0)) != TK_Normal)
					return status;
				mp_substage++;
			}   nobreak;

            case 1: {
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
            case 2: {
               
				if( mp_pointcount != 0)
				{
					switch (op) {
						case OPT_ALL_VFINDICES:
							{
								mask = Vertex_Face_Index;
								if ((status = GetAsciiData(tk, "Vertex_Face_Index",  mp_vfindices,  mp_pointcount)) != TK_Normal)
									return status;
								mp_vfindexcount = mp_pointcount;
							}
							break;
						case OPT_ALL_VEINDICES:
							{
								mask = Vertex_Edge_Index;
								if ((status = GetAsciiData(tk, "Vertex_Edge_Index",  mp_veindices,  mp_pointcount)) != TK_Normal)
									return status;
								mp_veindexcount = mp_pointcount;
							}
							break;
						case OPT_ALL_VMINDICES:
							{
								mask = Vertex_Marker_Index;
								if ((status = GetAsciiData(tk, "Vertex_Marker_Index",  mp_vmindices,  mp_pointcount)) != TK_Normal)
									return status;
								mp_vmindexcount = mp_pointcount;
							}
							break;
						default:
							return tk.Error("internal error (1) in TK_Polyhedron::read_vertex_indices_all: unrecognized case");
						}

					for (int i = 0; i < mp_pointcount; i++)
						mp_exists[i] |= mask;
				}
	            mp_substage++; 
			}   nobreak;

			case 3: {
				if ((status = ReadAsciiWord(tk,0)) != TK_Normal)
					return status;
				mp_substage = 0;
					}   break;

            default:
                return tk.Error("internal error in TK_Polyhedron::read_vertex_indices_all(ver<650)");
        }
    }
    return TK_Normal;
#else
	UNREFERENCED (op);
	return tk.Error(stream_disable_ascii);
#endif 
} // end function TK_Polyhedron::read_vertex_indices_all



TK_Status TK_Polyhedron::read_vertex_indices_ascii(BStreamFileToolkit & tk, unsigned char op) alter {
#ifndef BSTREAM_DISABLE_ASCII

	TK_Status       status;
    float           *array = null;
    int				count = 0;
    unsigned int    mask;

    switch (op) {
        case OPT_VERTEX_FINDICES:	mask = Vertex_Face_Index;	break;
        case OPT_VERTEX_EINDICES:	mask = Vertex_Edge_Index;	break;
        case OPT_VERTEX_MINDICES:	mask = Vertex_Marker_Index;	break;
        default:
            return tk.Error("internal error in TK_Polyhedron::read_vertex_indices: unrecognized case");
    }

    if (tk.GetVersion() >= 650) {
        switch (mp_substage) {
            //compression scheme
			case 0: {
				if ((status = ReadAsciiWord(tk,0)) != TK_Normal)
					return status;
				mp_substage++;
			}   nobreak;

            case 1: {
                ASSERT(mp_compression_scheme == CS_TRIVIAL);
                if ((status = GetAsciiData(tk, "Compression_Scheme", mp_compression_scheme)) != TK_Normal)
                    return status;
                mp_substage++;
            }   nobreak;

            //bounding box
            case 2: {
                if ((status = GetAsciiData(tk, "Bounding_Box", mp_bbox, 6)) != TK_Normal)
                    return status;
                mp_substage++;
            } nobreak;

            //bits per sample
            case 3: {
                if ((status = GetAsciiData(tk, "Bits_Per_Sample", mp_bits_per_sample)) != TK_Normal)
                    return status;
                mp_substage++;
            } nobreak;

            //all points do NOT have indices, number of individual indices
            case 4: {
                if (mp_pointcount < 256) {
					if ((status = GetAsciiData(tk, "Count", m_byte)) != TK_Normal)
                        return status;
                    count = m_byte;
                }
                else if (mp_pointcount < 65536) {
					if ((status = GetAsciiData(tk, "Count", m_unsigned_short)) != TK_Normal)
                        return status;
                    count = m_unsigned_short;
                }
                else {
					if ((status = GetAsciiData(tk, "Count", m_int)) != TK_Normal)
                        return status;
					count = m_int;
                }
                if (count > mp_pointcount)
                    return tk.Error ("invalid vertex index count in TK_Polyhedron::read_vertex_indices");

                switch (op) {
                    case OPT_VERTEX_FINDICES:	mp_vfindexcount = count;	break;
                    case OPT_VERTEX_EINDICES:	mp_veindexcount = count;	break;
                    case OPT_VERTEX_MINDICES:	mp_vmindexcount = count;	break;
                    default:
                        return tk.Error("internal error (3) in TK_Polyhedron::read_vertex_indices: unrecognized case");
                }

                mp_progress = 0; //just in case
                mp_substage++;
            }   nobreak;

            // and indices of existing indices
            case 5: {
                // recover count in case we hit a pending and are returning to this loop
                switch (op) {
                    case OPT_VERTEX_FINDICES:   count = mp_vfindexcount;    break;
                    case OPT_VERTEX_EINDICES:   count = mp_veindexcount;    break;
                    case OPT_VERTEX_MINDICES:   count = mp_vmindexcount;    break;
                }
                while (mp_progress < count) {
                    int     index;

                    if (mp_pointcount < 256) {
						if ((status = GetAsciiData(tk, "Progress", m_byte)) != TK_Normal)
                            return status;
                        index = m_byte;
                    }
                    else if (mp_pointcount < 65536) {
						if ((status = GetAsciiData(tk, "Progress", m_unsigned_short)) != TK_Normal)
                            return status;
                        index = m_unsigned_short;
                    }
                    else {
						if ((status = GetAsciiData(tk, "Progress", m_int)) != TK_Normal)
                            return status;
						index = m_int;
                    }
                    if (index > mp_pointcount)
                        return tk.Error("invalid vertex index during set color by index");
                    mp_exists[index] |= mask;
                    mp_progress++;
                }
                mp_progress = 0;
                mp_substage++;
            }   nobreak;

		case 6: {
			char which_count[256];
			switch (op) {
				case OPT_VERTEX_FINDICES:	strcpy(which_count, "Vertex_Face_Index_Count");		break;
				case OPT_VERTEX_EINDICES:	strcpy(which_count, "Vertex_Edge_Index_Count");		break;
				case OPT_VERTEX_MINDICES:	strcpy(which_count, "Vertex_Marker_Index_Count");	break;
				default:
					return tk.Error("internal error in TK_Polyhedron::read_vertex_indices: unrecognized case");
			}

			if ((status = GetAsciiData(tk, which_count, count)) != TK_Normal)
				return status;

			if( count != 0)
			{
				switch (op) {
				case OPT_VERTEX_FINDICES:	ENSURE_ALLOCATED(mp_vfindices, float, count);		break;
				case OPT_VERTEX_EINDICES:	ENSURE_ALLOCATED(mp_veindices, float, count);		break;
				case OPT_VERTEX_MINDICES:	ENSURE_ALLOCATED(mp_vmindices, float, count);		break;
				default:
					return tk.Error("internal error in TK_Polyhedron::read_vertex_indices: unrecognized case");
				}
			}
			mp_substage++;
		}   nobreak;

		case 7: {
			char what[256];
			switch (op) {
				case OPT_VERTEX_FINDICES:	
					strcpy(what, "Vertex_Face_Index");
					array = mp_vfindices;
					break;
				case OPT_VERTEX_EINDICES:	
					strcpy(what, "Vertex_Edge_Index");
					array = mp_veindices;
					break;
				case OPT_VERTEX_MINDICES:	
					strcpy(what, "Vertex_Marker_Index");
					array = mp_vmindices;
					break;
				default:
					return tk.Error("internal error in TK_Polyhedron::read_vertex_indices: unrecognized case");
			}

			if( count != 0)
			{
				if ((status = GetAsciiData(tk, what,  array,  count)) != TK_Normal)
					return status;
			}

			mp_substage++; 
		}   nobreak;

		case 8: {
				if ((status = ReadAsciiWord(tk,0)) != TK_Normal)
					return status;
				mp_substage = 0;
		}   break;

		default:
                return tk.Error("internal error (1) in function read_vertex_indices");
        }
    }
    else {
        switch (mp_substage) {
			case 0: {
				if ((status = ReadAsciiWord(tk,0)) != TK_Normal)
					return status;
				mp_substage++;
			}   nobreak;

            //all points do NOT have indices, number of individual indices
            case 1: {
                if (mp_pointcount < 256) {
					if ((status = GetAsciiData(tk, "Count", m_byte)) != TK_Normal)
                        return status;
                    count = m_byte;
                }
                else if (mp_pointcount < 65536) {
					if ((status = GetAsciiData(tk, "Count", m_unsigned_short)) != TK_Normal)
                        return status;
                    count = m_unsigned_short;
                }
                else {
					if ((status = GetAsciiData(tk, "Count", m_int)) != TK_Normal)
                        return status;
					count  = m_int;
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
            case 2: {
                // recover count in case we hit a pending and are returning to this loop
                switch (op) {
                    case OPT_VERTEX_FINDICES:   count = mp_vfindexcount;    break;
                    case OPT_VERTEX_EINDICES:   count = mp_veindexcount;    break;
                    case OPT_VERTEX_MINDICES:   count = mp_vmindexcount;    break;
                }

                while (mp_progress < count) {
                    int     index;

                    if (mp_pointcount < 256) {
						if ((status = GetAsciiData(tk, "Progress", m_byte)) != TK_Normal)
                            return status;
                        index = m_byte;
                    }
                    else if (mp_pointcount < 65536) {
						if ((status = GetAsciiData(tk, "Progress", m_unsigned_short)) != TK_Normal)
                            return status;
                        index = m_unsigned_short;
                    }
                    else {
						if ((status = GetAsciiData(tk, "Progress", m_int)) != TK_Normal)
                            return status;
						index = m_int;
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
            case 3: {
				 char what[256];
				 switch (op) {
					case OPT_VERTEX_FINDICES:	
						strcpy(what, "Vertex_Face_Index");
						array = mp_vfindices;
						break;
					case OPT_VERTEX_EINDICES:	
						strcpy(what, "Vertex_Edge_Index");
						array = mp_veindices;
						break;
					case OPT_VERTEX_MINDICES:	
						strcpy(what, "Vertex_Marker_Index");
						array = mp_vmindices;
						break;
					default:
						return tk.Error("internal error in TK_Polyhedron::read_vertex_indices: unrecognized case");
				}

                while (mp_progress < mp_pointcount) {
                    if ((mp_exists[mp_progress] & mask) == mask) {
                        if ((status = GetAsciiData(tk, what, &array[mp_progress], 1)) != TK_Normal)
                            return status;
                    }
                    mp_progress++;
                }
                mp_progress = 0;
                  mp_substage++; 
				}   nobreak;

			case 4: {
				if ((status = ReadAsciiWord(tk,0)) != TK_Normal)
					return status;
				mp_substage = 0;
					}   break;

            default:
                return tk.Error("internal error (3) in function TK_Polyhedron::read_vertex_indices(version < 650)");
        }
    }
    return TK_Normal;
#else
	UNREFERENCED (op);
	return tk.Error(stream_disable_ascii);
#endif 
} // end function TK_Polyhedron::read_vertex_indices


TK_Status TK_Polyhedron::read_face_colors_all_ascii(BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII

	TK_Status       status;
    ASSERT(mp_optopcode == OPT_ALL_FACE_COLORS);

    if (tk.GetVersion() >= 650) {

        switch (mp_substage) {
			case 0: {
				if ((status = ReadAsciiWord(tk,0)) != TK_Normal)
					return status;
				mp_substage++;
			}   nobreak;

            case 1: {
                if ((status = GetAsciiData(tk, "Compression_Scheme", mp_compression_scheme)) != TK_Normal)
                    return status;
                mp_substage++;
            }   nobreak;

            case 2: {
                if ((status = GetAsciiData(tk, "Bits_Per_Sample", mp_bits_per_sample)) != TK_Normal)
                    return status;
                mp_substage++;
            }   nobreak;
			
			case 3: {
				if ((status = GetAsciiData(tk, "Face_Count", mp_facecount)) != TK_Normal)
					return status;
				
				if( mp_facecount != 0)
					ENSURE_ALLOCATED(mp_fcolors, float, 3 * mp_facecount);

				mp_substage++;
			}   nobreak;

			case 4: {
				if( mp_facecount != 0){
					status = GetAsciiData(tk, "Face_Color",  mp_fcolors, 3 * mp_facecount);
					if (status != TK_Normal)
						return status;
					mp_fcolorcount = mp_facecount;
					for (int i = 0; i < mp_facecount; i++)
						mp_face_exists[i] |= Face_Color;
				}
			   mp_substage++; 
			}   nobreak;

			case 5: {
				if ((status = ReadAsciiWord(tk,0)) != TK_Normal)
					return status;
				mp_substage = 0;
					}   break;
            default:
                return tk.Error("internal error in TK_Polyhedron::read_face_colors");
        }
    }
    else { //version < 650
        switch (mp_substage) {

		 case 0: {
				if ((status = ReadAsciiWord(tk,0)) != TK_Normal)
					return status;

				if( mp_facecount != 0)
					ENSURE_ALLOCATED(mp_fcolors, float, 3 * mp_facecount);

				mp_substage++;
			}   nobreak;

		   case 1: {
			   if( mp_facecount != 0){
				   status = GetAsciiData(tk, "Face_Color",  mp_fcolors, 3 * mp_facecount);
				   if (status != TK_Normal)
					   return status;
				   mp_fcolorcount = mp_facecount;
				   for (int i = 0; i < mp_facecount; i++)
					   mp_face_exists[i] |= Face_Color;
				   mp_substage = 0;
				}
			    mp_substage++; 
			}   nobreak;

			case 2: {
				if ((status = ReadAsciiWord(tk,0)) != TK_Normal)
					return status;
				mp_substage = 0;
					}   break;

            default:
                return tk.Error("internal error in TK_Polyhedron::read_face_colors_all(ver<650)");
        }
    }
    return TK_Normal;
#else
	return tk.Error(stream_disable_ascii);
#endif 
} // end function TK_Polyhedron::read_face_colors_all

TK_Status TK_Polyhedron::read_face_colors_ascii(BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII

	TK_Status       status;
    ASSERT(mp_optopcode == OPT_FACE_COLORS);
    if (tk.GetVersion() >= 650) {

        switch (mp_substage) {
			case 0: {
				if ((status = ReadAsciiWord(tk,0)) != TK_Normal)
					return status;
				mp_substage++;
			}   nobreak;

            case 1: {
                if ((status = GetAsciiData(tk, "Compression_Scheme", mp_compression_scheme)) != TK_Normal)
                    return status;
                mp_substage++;
            }   nobreak;

            case 2: {
                if ((status = GetAsciiData(tk, "Bits_Per_Sample", mp_bits_per_sample)) != TK_Normal)
                    return status;
                mp_substage++;
            }   nobreak;

            //if all faces do NOT have colors, number of individual colors
            case 3: {
                if (mp_facecount < 256) {
					if ((status = GetAsciiData(tk,"Count", m_byte)) != TK_Normal)
                        return status;
                    mp_fcolorcount = m_byte;
                }
                else if (mp_facecount < 65536) {
					if ((status = GetAsciiData(tk, "Count", m_unsigned_short)) != TK_Normal)
                        return status;
                    mp_fcolorcount = m_unsigned_short;
                }
                else {
					if ((status = GetAsciiData(tk, "Count", m_int )) != TK_Normal)
                        return status;
					mp_fcolorcount = m_int;
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
            case 4: {
                // get "count" worth of colors & flag them
                while (mp_progress < mp_fcolorcount) {
                    int     color;

                    if (mp_facecount < 256) {
						if ((status = GetAsciiData(tk, "Progress", m_byte)) != TK_Normal)
                            return status;
                        color = m_byte;
                    }
                    else if (mp_facecount < 65536) {
						if ((status = GetAsciiData(tk, "Progress", m_unsigned_short)) != TK_Normal)
                            return status;
                        color = m_unsigned_short;
                    }
                    else {
						if ((status = GetAsciiData(tk, "Progress", m_int )) != TK_Normal)
                            return status;
						color = m_int;
                    }
                    mp_face_exists[color] |= Face_Color;
                    mp_progress++;
                }
                mp_progress = 0;
                mp_substage++;
            }   nobreak;

			case 5: {
				if ((status = GetAsciiData(tk, "Face_Count", mp_facecount)) != TK_Normal)
					return status;
			
				if( mp_facecount != 0)
					ENSURE_ALLOCATED(mp_fcolors, float, mp_facecount * 3);
			       
				mp_substage++;
			}   nobreak;

			case 6: {
				if( mp_facecount != 0){
					if ((status = GetAsciiData(tk, "Face_Color",  mp_fcolors, 3 * mp_facecount)) != TK_Normal)
						return status;
					}
				   mp_substage++; 
				}   nobreak;

			case 7: {
				if ((status = ReadAsciiWord(tk,0)) != TK_Normal)
					return status;
				mp_substage = 0;
					}   break;
		
		/*	case 6: {
             int   i = 0;
             int   j = 0;
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
				mp_substage = 0; // done reading points 
					} break;
*/
            default:
                return tk.Error("internal error in TK_Polyhedron::read_face_colors");
        }
    }

    else { //version < 650
        switch (mp_substage) {
			case 0: {
				if ((status = ReadAsciiWord(tk,0)) != TK_Normal)
					return status;
				mp_substage++;
			}   nobreak;

            //if all faces do NOT have colors, number of individual colors
            case 1: {
                if (mp_facecount < 256) {
					if ((status = GetAsciiData(tk, "Count", m_byte)) != TK_Normal)
                        return status;
                    mp_fcolorcount = m_byte;
                }
                else if (mp_facecount < 65536) {
					if ((status = GetAsciiData(tk, "Count", m_unsigned_short)) != TK_Normal)
                        return status;
                    mp_fcolorcount = m_unsigned_short;
                }
                else {
					if ((status = GetAsciiData(tk, "Count", m_int)) != TK_Normal)
                        return status;
					mp_fcolorcount = m_int;
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
            case 2: {
                // get "count" worth of colors & flag them
                while (mp_progress < mp_fcolorcount) {
                    int     color;

                    if (mp_facecount < 256) {
						if ((status = GetAsciiData(tk, "Progress", m_byte)) != TK_Normal)
                            return status;
                        color = m_byte;
                    }
                    else if (mp_facecount < 65536) {
						if ((status = GetAsciiData(tk, "Progress", m_unsigned_short)) != TK_Normal)
                            return status;
                        color = m_unsigned_short;
                    }
                    else {
						if ((status = GetAsciiData(tk, "Progress", m_int)) != TK_Normal)
                            return status;
						color = m_int;
                    }
                    mp_face_exists[color] |= Face_Color;
                    mp_progress++;
                }
                mp_progress = 0;
                      
				if( mp_facecount != 0)
					ENSURE_ALLOCATED(mp_fcolors, float, mp_facecount);
 				
                mp_substage++;
            }   nobreak;

		   case 3: {
			   if( mp_facecount != 0){
				   if ((status = GetAsciiData(tk, "Face_Color",  mp_fcolors, 3 * mp_facecount)) != TK_Normal)
					   return status;
				}
				mp_progress = 0;
				mp_substage++; 
			}   nobreak;

			case 4: {
				if ((status = ReadAsciiWord(tk,0)) != TK_Normal)
					return status;
				mp_substage = 0;
					}   break;

            default:
                return tk.Error("internal error in TK_Polyhedron::read_face_colors(ver<650)");
        }
    }
    return TK_Normal;
#else
return tk.Error(stream_disable_ascii);
#endif 
} // end function TK_Polyhedron::read_face_colors

TK_Status TK_Polyhedron::read_face_indices_all_ascii(BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII

	TK_Status       status;
    int             i;
    ASSERT(mp_optopcode == OPT_ALL_FACE_INDICES);
    if (tk.GetVersion() >= 650) {

        switch (mp_substage) {

			case 0: {
				if ((status = ReadAsciiWord(tk,0)) != TK_Normal)
					return status;
				mp_substage++;
			}   nobreak;

            case 1: {
                if ((status = GetAsciiData(tk, "Compression_Scheme", mp_compression_scheme)) != TK_Normal)
                    return status;
                mp_substage++;
            }   nobreak;

            case 2: {
                if ((status = GetAsciiData(tk, "Bounding_Box", mp_bbox, 6)) != TK_Normal)
                    return status;
                mp_substage++;
            }   nobreak;

            case 3: {
                if ((status = GetAsciiData(tk, "Bits_Per_Sample", mp_bits_per_sample)) != TK_Normal)
                    return status;
                mp_substage++;
            }   nobreak;

			case 4: {
				if ((status = GetAsciiData(tk, "Face_Count", mp_facecount)) != TK_Normal)
					return status;		
				if( mp_facecount != 0)
					ENSURE_ALLOCATED(mp_findices, float, mp_facecount);
				mp_substage++;
			}   nobreak;

			case 5: {
				if( mp_facecount != 0){
					status = GetAsciiData(tk, "Face_Colors_by_Index",  mp_findices,  mp_facecount);
					if (status != TK_Normal)
						return status;
					mp_findexcount = mp_facecount;
					for (i = 0; i < mp_facecount; i++)
						mp_face_exists[i] |= Face_Index;
				}
				mp_substage++;
			} nobreak;

			case 6: {
				if ((status = ReadAsciiWord(tk,0)) != TK_Normal)
					return status;
				mp_substage = 0;
					}   break;

            default:
                return tk.Error("internal error in TK_Polyhedron::read_face_indices");
        }
    }
    else { //version < 650
        switch (mp_substage) {
			case 0: {
				if ((status = ReadAsciiWord(tk,0)) != TK_Normal)
					return status;
				mp_substage++;
			}   nobreak;

            case 1: {
                ENSURE_ALLOCATED(mp_findices, float, mp_facecount);
                mp_substage++;
            }   nobreak;

            //get all the indices OR the individual indices
			case 2: {
				if ((status = ReadAsciiWord(tk,0)) != TK_Normal)
					return status;
				mp_substage++;
					}   nobreak;

            case 3: {
				if( mp_facecount != 0 )
				{
					if ((status = GetAsciiData(tk, "Face_Colors_by_Index", mp_findices, mp_facecount)) != TK_Normal)
						return status;
					mp_findexcount = mp_facecount;
					for (i = 0; i < mp_facecount; i++)
						mp_face_exists[i] |= Face_Index;
					mp_substage++;
				}
				} nobreak;

			case 4: {
				if ((status = ReadAsciiWord(tk,0)) != TK_Normal)
					return status;
				mp_substage = 0;
					}   break;

            default:
                return tk.Error("internal error in TK_Polyhedron::read_face_indices_all(ver<650)");
        }
    }
    return TK_Normal;
#else
	return tk.Error(stream_disable_ascii);
#endif 
} // end function TK_Polyhedron::read_face_indices_all


TK_Status TK_Polyhedron::read_face_indices_ascii(BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII

	TK_Status       status;
    ASSERT(mp_optopcode == OPT_FACE_INDICES);
    if (tk.GetVersion() >= 650) {

        switch (mp_substage) {
				
			case 0: {
					if ((status = ReadAsciiWord(tk,0)) != TK_Normal)
						return status;
					mp_substage++;
						}   nobreak;

            case 1: {
                if ((status = GetAsciiData(tk, "Compression_Scheme", mp_compression_scheme)) != TK_Normal)
                    return status;
                mp_substage++;
            }   nobreak;

            case 2: {
                if ((status = GetAsciiData(tk, "Bounding_Box", mp_bbox, 6)) != TK_Normal)
                    return status;
                mp_substage++;
            }   nobreak;

            case 3: {
                if ((status = GetAsciiData(tk, "Bits_Per_Sample", mp_bits_per_sample)) != TK_Normal)
                    return status;
                mp_substage++;
            }   nobreak;

            //if all faces do NOT have indices, number of individual indices
            case 4: {
                if (mp_facecount < 256) {
					if ((status = GetAsciiData(tk, "Count", m_byte)) != TK_Normal)
                        return status;
                    mp_findexcount = m_byte;
                }
                else if (mp_facecount < 65536) {
					if ((status = GetAsciiData(tk, "Count",  m_unsigned_short)) != TK_Normal)
                        return status;
                    mp_findexcount = m_unsigned_short;
                }
                else {
					if ((status = GetAsciiData(tk, "Count", m_int)) != TK_Normal)
                        return status;
					mp_findexcount = m_int;
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
            case 5: {
                // get "count" worth of indices & flag them
                while (mp_progress < mp_findexcount) {
                    int     index;

                    if (mp_facecount < 256) {
						if ((status = GetAsciiData(tk, "Progress", m_byte)) != TK_Normal)
                            return status;
                        index = m_byte;
                    }
                    else if (mp_facecount < 65536) {
						if ((status = GetAsciiData(tk, "Progress", m_unsigned_short)) != TK_Normal)
                            return status;
                        index = m_unsigned_short;
                    }
                    else {
						if ((status = GetAsciiData(tk, "Progress", index)) != TK_Normal)
                            return status;
						index = m_int;
                    }
                    mp_face_exists[index] |= Face_Index;
                    mp_progress++;
                }
                mp_progress = 0;
                mp_substage++;
            }   nobreak;

			case 6: {
				if ((status = GetAsciiData(tk, "Face_Count", mp_facecount)) != TK_Normal)
					return status;
			
				if( mp_facecount != 0)
					ENSURE_ALLOCATED(mp_findices, float, mp_facecount);

				mp_substage++;
					} nobreak;

			case 7: {
				if( mp_facecount != 0){
					if ((status = GetAsciiData(tk, "Face_Colors_by_Index",  mp_findices,  mp_facecount)) != TK_Normal)
						return status;
				}
				mp_substage++; 
			}   nobreak;

			case 8: {
				if ((status = ReadAsciiWord(tk,0)) != TK_Normal)
					return status;
				mp_substage = 0;
					}   break;
            default:
                return tk.Error("internal error in TK_Polyhedron::read_face_indices");
        }
    }
    else { //version < 650
        switch (mp_substage) {
			case 0: {
				if ((status = ReadAsciiWord(tk,0)) != TK_Normal)
					return status;
				mp_substage++;
			}   nobreak;

            //if all faces do NOT have indices, number of individual indices
            case 1: {
                if (mp_facecount < 256) {
					if ((status = GetAsciiData(tk, "Count",m_byte)) != TK_Normal)
                        return status;
                    mp_findexcount = m_byte;
                }
                else if (mp_facecount < 65536) {
                    if ((status = GetAsciiData(tk, "Count", m_unsigned_short)) != TK_Normal)
                        return status;
                    mp_findexcount = m_unsigned_short;
                }
                else {
                    if ((status = GetAsciiData(tk, "Count", m_int)) != TK_Normal)
                        return status;
					mp_findexcount = m_int;
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
            case 2: {
                // get "count" worth of indices & flag them
                while (mp_progress < mp_findexcount) {
                    int     index;

                    if (mp_facecount < 256) {
						if ((status = GetAsciiData(tk, "Progress", m_byte)) != TK_Normal)
                            return status;
                        index = m_byte;
                    }
                    else if (mp_facecount < 65536) {
                        if ((status = GetAsciiData(tk, "Progress", m_unsigned_short)) != TK_Normal)
                            return status;
                        index = m_unsigned_short;
                    }
                    else {
                        if ((status = GetAsciiData(tk, "Progress", m_int)) != TK_Normal)
                            return status;
						index = m_int;
                    }
                    mp_face_exists[index] |= Face_Index;
                    mp_progress++;
                }
                mp_progress = 0;
                mp_substage++;
            }   nobreak;

            //get all the indices OR the individual indices
            case 3: {
                // get indices associated with the flagged indices
                while (mp_progress < mp_facecount) {
                    if (mp_face_exists[mp_progress] & Face_Index) {
						if ((status = GetAsciiData(tk, "Face_Colors_by_Index", &mp_findices[mp_progress], 1)) != TK_Normal)
                            return status;
                    }
                    mp_progress++;
                }
                mp_progress = 0;
                mp_substage++; 
				}   nobreak;

			case 4: {
				if ((status = ReadAsciiWord(tk,0)) != TK_Normal)
					return status;
				mp_substage = 0;
					}   break;

            default:
                return tk.Error("internal error in TK_Polyhedron::read_face_indices(ver<650)");
        }
    }
    return TK_Normal;
#else
	return tk.Error(stream_disable_ascii);
#endif 
} // end function TK_Polyhedron::read_face_indices




TK_Status TK_Polyhedron::read_face_regions_ascii (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII

	   TK_Status       status = TK_Normal;
    switch (mp_substage) {
		
		case 0: {
				if ((status = ReadAsciiWord(tk,0)) != TK_Normal)
					return status;
				mp_substage++;
			}   nobreak;

		 case 1: {
            if ((status = GetAsciiData(tk, "Compression_Scheme", mp_compression_scheme)) != TK_Normal)
                return status;
            SetFaceRegions();   // allocate space
            mp_substage++;
           
        } nobreak;
		case 2: {
			if( mp_facecount != 0){
				if ((status = GetAsciiData(tk, "Face_Regions",  mp_fregions, mp_facecount)) != TK_Normal)
					return status;
			}
			mp_substage++; 
		}   nobreak;

		case 3: {
			if ((status = ReadAsciiWord(tk,0)) != TK_Normal)
				return status;
			mp_substage = 0;
				}   break;

        default:
            return tk.Error ("internal error in TK_Polyhedron::read_face_regions");
    }
    return status;
#else
	return tk.Error(stream_disable_ascii);
#endif 
} // end function TK_Polyhedron::read_face_regions


TK_Status TK_Polyhedron::read_vertex_marker_visibilities_ascii (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII

	TK_Status       status = TK_Normal;
    int i;

    if (mp_optopcode == OPT_ALL_VMVISIBILITIES) {
        switch (mp_substage) {
			case 0: {
				if ((status = ReadAsciiWord(tk,0)) != TK_Normal)
					return status;
				mp_substage++;
			}   nobreak;

            //compression scheme
            case 1: {
                if ((status = GetAsciiData(tk, "Compression_Scheme", mp_compression_scheme)) != TK_Normal)
                    return status;
                mp_substage++;
                SetVertexMarkerVisibilities();
            }   nobreak;

            //all visibilities
            case 2: {
                mp_vmvisibilitycount = mp_pointcount;
				if ((status = GetAsciiData(tk,"Visibilities", mp_vmvisibilities, mp_vmvisibilitycount)) != TK_Normal)
                    return status;
                for (i = 0; i < mp_pointcount; i++)
                    mp_exists[i] |= Vertex_Marker_Visibility;
                  mp_substage++; 
				}   nobreak;

			case 3: {
				if ((status = ReadAsciiWord(tk,0)) != TK_Normal)
					return status;
				mp_substage = 0;
					}   break;

            default:
                return tk.Error("internal error in read_vertex_marker_visibilities (1)");
        }
    }
    else {
        switch (mp_substage) {
            //compression scheme
			case 0: {
				if ((status = ReadAsciiWord(tk,0)) != TK_Normal)
					return status;
				mp_substage++;
			}   nobreak;

            case 1: {
                if ((status = GetAsciiData(tk, "Compression_Scheme", mp_compression_scheme)) != TK_Normal)
                    return status;
                mp_substage++;
            }   nobreak;

            //count
            case 2: {
                if ((status = GetAsciiData(tk, "Visibilities", mp_vmvisibilitycount)) != TK_Normal)
                    return status;
                mp_substage++;
                mp_progress = 0;
            }   nobreak;

            //and indices of existing values
            case 3: {
                while (mp_progress < mp_vmvisibilitycount) {
                    int     index;

                    if (mp_pointcount < 256) {
						if ((status = GetAsciiData(tk, "Progress", m_byte)) != TK_Normal)
                            return status;
                        index = m_byte;
                    }
                    else if (mp_pointcount < 65536) {
                        if ((status = GetAsciiData(tk, "Progress", m_unsigned_short)) != TK_Normal)
                            return status;
                        index = m_unsigned_short;
                    }
                    else {
                        if ((status = GetAsciiData(tk, "Progress", m_int)) != TK_Normal)
                            return status;
						index = m_int;
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
            case 4: {
                while (mp_progress < mp_pointcount) {
                    if (mp_exists[mp_progress] & Vertex_Marker_Visibility) {
						if ((status = GetAsciiData(tk, "Visibilities", mp_vmvisibilities[mp_progress])) != TK_Normal)
                            return status;
                    }
                    mp_progress++;
                }
                mp_progress = 0;
                mp_substage++; 
				}   nobreak;

			case 5: {
				if ((status = ReadAsciiWord(tk,0)) != TK_Normal)
					return status;
				mp_substage = 0;
					}   break;

            default:
                return tk.Error("internal error in read_vertex_marker_visibilities (2)");
        }
    }
    return status;
#else
	return tk.Error(stream_disable_ascii);
#endif 
} //end function TK_Polyhedron::read_vertex_marker_visibilities


TK_Status TK_Polyhedron::read_vertex_marker_symbols_ascii (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII

	TK_Status       status = TK_Normal;
    int i;

    if (mp_optopcode == OPT_ALL_VMSYMBOLS) {
        switch (mp_substage) {
			case 0: {
				if ((status = ReadAsciiWord(tk,0)) != TK_Normal)
					return status;
				mp_substage++;
			}   nobreak;

            //compression scheme
            case 1: {
                if ((status = GetAsciiData(tk, "Compression_Scheme", mp_compression_scheme)) != TK_Normal)
                    return status;
                SetVertexMarkerSymbols();
                mp_substage++;
            }   nobreak;

            //all symbols
            case 2: {
                mp_vmsymbolcount = mp_pointcount;
				if ((status = GetAsciiData(tk, "Symbols", mp_vmsymbols, mp_vmsymbolcount)) != TK_Normal)
                    return status;
                for (i = 0; i < mp_pointcount; i++)
                    mp_exists[i] |= Vertex_Marker_Symbol;
                  mp_substage++; 
				}   nobreak;

			case 3: {
				if ((status = ReadAsciiWord(tk,0)) != TK_Normal)
					return status;
				mp_substage = 0;
					}   break;

            default:
                return tk.Error("internal error in read_vertex_marker_symbols (1)");
        }
    }
    else {
        switch (mp_substage) {
			case 0: {
				if ((status = ReadAsciiWord(tk,0)) != TK_Normal)
					return status;
				mp_substage++;
			}   nobreak;
            //compression scheme
            case 1: {
                if ((status = GetAsciiData(tk, "Compression_Scheme", mp_compression_scheme)) != TK_Normal)
                    return status;
                mp_substage++;
            }   nobreak;

            //count
            case 2: {
				if ((status = GetAsciiData(tk, "Symbol_Count", mp_vmsymbolcount)) != TK_Normal)
                    return status;
                mp_substage++;
                mp_progress = 0;
            }   nobreak;

            //and indices of existing values
            case 3: {
                while (mp_progress < mp_vmsymbolcount) {
                    int     index;

                    if (mp_pointcount < 256) {
						if ((status = GetAsciiData(tk, "Progress", m_byte)) != TK_Normal)
                            return status;
                        index = m_byte;
                    }
                    else if (mp_pointcount < 65536) {
                        if ((status = GetAsciiData(tk, "Progress", m_unsigned_short)) != TK_Normal)
                            return status;
                        index = m_unsigned_short;
                    }
                    else {
                        if ((status = GetAsciiData(tk, "Progress", m_int)) != TK_Normal)
                            return status;
						index = m_int;
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
            case 4: {
                while (mp_progress < mp_pointcount) {
                    if (mp_exists[mp_progress] & Vertex_Marker_Symbol) {
                        switch (mp_subprogress) {
                            case 0: {
								if ((status = GetAsciiData(tk, "Symbols", mp_vmsymbols[mp_progress])) != TK_Normal)
                                    return status;
                                if (mp_vmsymbols[mp_progress] != 0xFF)
                                    break;
                                SetVertexMarkerSymbolStrings();
                                mp_subprogress++;
                            }   nobreak;
                            case 1: {
                                
                                int             length;

								if ((status = GetAsciiData(tk, "String_Length", m_int)) != TK_Normal)
                                    return status;
                                length = m_int;
                                mp_vmsymbolstrings[mp_progress] = new char[length+1];
                                mp_vmsymbolstrings[mp_progress][length] = '\0';
                                mp_vmsymbolstrings[mp_progress][0] = (char)length;
                                mp_subprogress++;
                            }   nobreak;
                            case 2: {
                                int             length = (int)mp_vmsymbolstrings[mp_progress][0];

								if ((status = GetAsciiData(tk, "Symbol_Strings", mp_vmsymbolstrings[mp_progress], length)) != TK_Normal)
                                    return status;
                                mp_subprogress = 0;
                            }   break;
                        }
                    }
                    mp_progress++;
                }
                mp_progress = 0;
                 mp_substage++; 
				}   nobreak;

			case 5: {
				if ((status = ReadAsciiWord(tk,0)) != TK_Normal)
					return status;
				mp_substage = 0;
					}   break;

            default:
                return tk.Error("internal error in read_vertex_marker_symbols (2)");
        }
    }
    return status;
#else
return tk.Error(stream_disable_ascii);
#endif 
} //end function TK_Polyhedron::read_vertex_marker_symbols


TK_Status TK_Polyhedron::read_vertex_marker_sizes_ascii (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII

	TK_Status       status = TK_Normal;
    int i;

    if (mp_optopcode == OPT_ALL_VMSIZES) {
        switch (mp_substage) {
            //compression scheme
			case 0: {
				if ((status = ReadAsciiWord(tk,0)) != TK_Normal)
					return status;
				mp_substage++;
			}   nobreak;
            case 1: {
                if ((status = GetAsciiData(tk, "Compression_Scheme", mp_compression_scheme)) != TK_Normal)
                    return status;
                SetVertexMarkerSizes();
                mp_substage++;
            }   nobreak;

            //all sizes
            case 2: {
                mp_vmsizecount = mp_pointcount;
				if ((status = GetAsciiData(tk, "Sizes", mp_vmsizes, mp_vmsizecount)) != TK_Normal)
                    return status;
                for (i = 0; i < mp_pointcount; i++)
                    mp_exists[i] |= Vertex_Marker_Size;
                 mp_substage++; 
				}   nobreak;

			case 3: {
				if ((status = ReadAsciiWord(tk,0)) != TK_Normal)
					return status;
				mp_substage = 0;
					}   break;

            default:
                return tk.Error("internal error in read_vertex_marker_sizes (1)");
        }
    }
    else {
        switch (mp_substage) {
			case 0: {
				if ((status = ReadAsciiWord(tk,0)) != TK_Normal)
					return status;
				mp_substage++;
			}   nobreak;
            //compression scheme
            case 1: {
                if ((status = GetAsciiData(tk, "Compression_Scheme", mp_compression_scheme)) != TK_Normal)
                    return status;
                mp_substage++;
            }   nobreak;

            //count
            case 2: {
				if ((status = GetAsciiData(tk, "Size_Count", mp_vmsizecount)) != TK_Normal)
                    return status;
                mp_substage++;
                mp_progress = 0;
            }   nobreak;

            //and indices of existing values
            case 3: {
                while (mp_progress < mp_vmsizecount) {
                    int     index;

                    if (mp_pointcount < 256) {
						if ((status = GetAsciiData(tk, "Progress", m_byte)) != TK_Normal)
                            return status;
                        index = m_byte;
                    }
                    else if (mp_pointcount < 65536) {
                        if ((status = GetAsciiData(tk, "Progress", m_unsigned_short)) != TK_Normal)
                            return status;
                        index = m_unsigned_short;
                    }
                    else {
                        if ((status = GetAsciiData(tk, "Progress", m_int)) != TK_Normal)
                            return status;
						index = m_int;
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
            case 4: {
                while (mp_progress < mp_pointcount) {
                    if (mp_exists[mp_progress] & Vertex_Marker_Size) {
						if ((status = GetAsciiData(tk, "Sizes", mp_vmsizes[mp_progress])) != TK_Normal)
                            return status;
                    }
                    mp_progress++;
                }
                mp_progress = 0;
                mp_substage++; 
				}   nobreak;

			case 5: {
				if ((status = ReadAsciiWord(tk,0)) != TK_Normal)
					return status;
				mp_substage = 0;
					}   break;

            default:
                return tk.Error("internal error in read_vertex_marker_sizes (2)");
        }
    }
    return status;
#else
	return tk.Error(stream_disable_ascii);
#endif 

} //end function TK_Polyhedron::read_vertex_marker_sizes


TK_Status TK_Polyhedron::read_face_visibilities_ascii (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII

	TK_Status       status = TK_Normal;
    int i;

    if (mp_optopcode == OPT_ALL_FACE_VISIBILITIES) {
        switch (mp_substage) {
			case 0: {
				if ((status = ReadAsciiWord(tk,0)) != TK_Normal)
					return status;
				mp_substage++;
			}   nobreak;
            //compression scheme
            case 1: {
                if ((status = GetAsciiData(tk, "Compression_Scheme", mp_compression_scheme)) != TK_Normal)
                    return status;
                SetFaceVisibilities();
                mp_substage++;
            }   nobreak;

            //all visibilities
            case 2: {
                mp_fvisibilitycount = mp_facecount;
				if ((status = GetAsciiData(tk, "Visibilities", mp_fvisibilities, mp_fvisibilitycount)) != TK_Normal)
                    return status;
                for (i = 0; i < mp_facecount; i++)
                    mp_face_exists[i] |= Face_Visibility;
                  mp_substage++; 
				}   nobreak;

			case 3: {
				if ((status = ReadAsciiWord(tk,0)) != TK_Normal)
					return status;
				mp_substage = 0;
					}   break;

            default:
                return tk.Error("internal error in read_face_visibilities (1)");
        }
    }
    else {
        switch (mp_substage) {
			case 0: {
				if ((status = ReadAsciiWord(tk,0)) != TK_Normal)
					return status;
				mp_substage++;
			}   nobreak;
            //compression scheme
            case 1: {
                if ((status = GetAsciiData(tk, "Compression_Scheme", mp_compression_scheme)) != TK_Normal)
                    return status;
                mp_substage++;
            }   nobreak;

            //count
            case 2: {
				if ((status = GetAsciiData(tk, "Visiblity_Count", mp_fvisibilitycount)) != TK_Normal)
                    return status;
                mp_substage++;
                mp_progress = 0;
            }   nobreak;

            //and indices of existing values
            case 3: {
                while (mp_progress < mp_fvisibilitycount) {
                    int     index;
                    if (mp_facecount < 256) {
						if ((status = GetAsciiData(tk, "Progress", m_byte)) != TK_Normal)
                            return status;
                        index = m_byte;
                    }
                    else if (mp_facecount < 65536) {
                        if ((status = GetAsciiData(tk, "Progress", m_unsigned_short)) != TK_Normal)
                            return status;
                        index = m_unsigned_short;
                    }
                    else {
                        if ((status = GetAsciiData(tk, "Progress", m_int)) != TK_Normal)
                            return status;
						index = m_int;
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
            case 4: {
                while (mp_progress < mp_facecount) {
                    if (mp_face_exists[mp_progress] & Face_Visibility) {
						if ((status = GetAsciiData(tk, "Visibilities",mp_fvisibilities[mp_progress])) != TK_Normal)
                            return status;
                    }
                    mp_progress++;
                }
                mp_progress = 0;
                  mp_substage++; 
				}   nobreak;

			case 5: {
				if ((status = ReadAsciiWord(tk,0)) != TK_Normal)
					return status;
				mp_substage = 0;
					}   break;

            default:
                return tk.Error("internal error in read_face_visibilities (2)");
        }
    }
    return status;
#else
	return tk.Error(stream_disable_ascii);
#endif 
} //end function TK_Polyhedron::read_face_visibilities


TK_Status TK_Polyhedron::read_face_patterns_ascii (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII

	TK_Status       status = TK_Normal;
    int i;

    if (mp_optopcode == OPT_ALL_FACE_PATTERNS) {
        switch (mp_substage) {
			case 0: {
				if ((status = ReadAsciiWord(tk,0)) != TK_Normal)
					return status;
				mp_substage++;
			}   nobreak;
            //compression scheme
            case 1: {
                if ((status = GetAsciiData(tk, "Compression_Scheme", mp_compression_scheme)) != TK_Normal)
                    return status;
                SetFacePatterns();
                mp_substage++;
            }   nobreak;

            //all patterns
            case 2: {
                mp_fpatterncount = mp_facecount;
				if ((status = GetAsciiData(tk, "Face_Patterns", mp_fpatterns, mp_fpatterncount)) != TK_Normal)
                    return status;
                for (i = 0; i < mp_facecount; i++)
                    mp_face_exists[i] |= Face_Pattern;
                 mp_substage++; 
				}   nobreak;

			case 3: {
				if ((status = ReadAsciiWord(tk,0)) != TK_Normal)
					return status;
				mp_substage = 0;
					}   break;

            default:
                return tk.Error("internal error in read_face_patterns (1)");
        }
    }
    else {
        switch (mp_substage) {
			case 0: {
				if ((status = ReadAsciiWord(tk,0)) != TK_Normal)
					return status;
				mp_substage++;
			}   nobreak;
            //compression scheme
            case 1: {
                if ((status = GetAsciiData(tk, "Compression_Scheme", mp_compression_scheme)) != TK_Normal)
                    return status;
                mp_substage++;
            }   nobreak;

            //count
            case 2: {
				if ((status = GetAsciiData(tk, "Pattern_Count", mp_fpatterncount)) != TK_Normal)
                    return status;
                mp_substage++;
                mp_progress = 0;
            }   nobreak;

            //and indices of existing values
            case 3: {
                while (mp_progress < mp_fpatterncount) {
                    int     index;

                    if (mp_facecount < 256) {
						if ((status = GetAsciiData(tk,"Progress",  m_byte)) != TK_Normal)
                            return status;
                        index = m_byte;
                    }
                    else if (mp_facecount < 65536) {
                        if ((status = GetAsciiData(tk, "Progress", m_unsigned_short)) != TK_Normal)
                            return status;
                        index = m_unsigned_short;
                    }
                    else {
                        if ((status = GetAsciiData(tk, "Progress", m_int)) != TK_Normal)
                            return status;
						index = m_int;
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
            case 4: {
                while (mp_progress < mp_facecount) {
                    if (mp_face_exists[mp_progress] & Face_Pattern) {
                        if ((status = GetAsciiData(tk, "Face_Pattern", mp_fpatterns[mp_progress])) != TK_Normal)
                            return status;
                    }
                    mp_progress++;
                }
                mp_progress = 0;
               mp_substage++; 
				}   nobreak;

			case 5: {
				if ((status = ReadAsciiWord(tk,0)) != TK_Normal)
					return status;
				mp_substage = 0;
					}   break;

            default:
                return tk.Error("internal error in read_face_patterns (2)");
        }
    }
    return status;
#else
	return tk.Error(stream_disable_ascii);
#endif 

} //end function TK_Polyhedron::read_face_patterns


TK_Status TK_Polyhedron::read_face_normals_ascii (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII

	TK_Status       status = TK_Normal;
    int i;

    if (mp_optopcode == OPT_ALL_FACE_NORMALS_POLAR) {
        switch (mp_substage) {
			case 0: {
				if ((status = ReadAsciiWord(tk,0)) != TK_Normal)
					return status;
				mp_substage++;
			}   nobreak;
            //compression scheme
            case 1: {
                if ((status = GetAsciiData(tk, "Compression_Scheme", mp_compression_scheme)) != TK_Normal)
                    return status;
                SetFaceNormals();
                mp_substage++;
            }   nobreak;

            //all normals
            case 2: {
                mp_fnormalcount = mp_facecount;
				if ((status = GetAsciiData(tk, "Face_Normals", mp_fnormals, 2 * mp_fnormalcount)) != TK_Normal)
                    return status;
                normals_polar_to_cartesian(null, Face_Normal, mp_facecount, mp_fnormals, mp_fnormals);
                for (i = 0; i < mp_facecount; i++)
                    mp_face_exists[i] |= Face_Normal;
                  mp_substage++; 
				}   nobreak;

			case 3: {
				if ((status = ReadAsciiWord(tk,0)) != TK_Normal)
					return status;
				mp_substage = 0;
					}   break;

            default:
                return tk.Error("internal error in read_face_normals (1)");
        }
    }
    else {
        switch (mp_substage) {
			case 0: {
				if ((status = ReadAsciiWord(tk,0)) != TK_Normal)
					return status;
				mp_substage++;
			}   nobreak;
            //compression scheme
            case 1: {
                if ((status = GetAsciiData(tk, "Compression_Scheme", mp_compression_scheme)) != TK_Normal)
                    return status;
                mp_substage++;
            }   nobreak;

            //count
            case 2: {
				if ((status = GetAsciiData(tk, "Face_Normal_Count", mp_fnormalcount)) != TK_Normal)
                    return status;
                mp_substage++;
                mp_progress = 0;
            }   nobreak;

            //and indices of existing values
            case 3: {
                while (mp_progress < mp_fnormalcount) {
                    int     index;

                    if (mp_facecount < 256) {
						if ((status = GetAsciiData(tk, "Progress", m_byte)) != TK_Normal)
                            return status;
                        index = m_byte;
                    }
                    else if (mp_facecount < 65536) {
                        if ((status = GetAsciiData(tk, "Progress", m_unsigned_short)) != TK_Normal)
                            return status;
                        index = m_unsigned_short;
                    }
                    else {
                        if ((status = GetAsciiData(tk, "Progress", m_int)) != TK_Normal)
                            return status;
						index = m_int;
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
            case 4: {
                while (mp_progress < mp_facecount) {
                    if (mp_face_exists[mp_progress] & Face_Normal) {
						if ((status = GetAsciiData(tk, "Face_Normals", &mp_fnormals[2*mp_progress], 2)) != TK_Normal)
                            return status;
                    }
                    mp_progress++;
                }
                normals_polar_to_cartesian(mp_face_exists, Face_Normal, mp_facecount, mp_fnormals, mp_fnormals);
                mp_progress = 0;
                 mp_substage++; 
				}   nobreak;

			case 5: {
				if ((status = ReadAsciiWord(tk,0)) != TK_Normal)
					return status;
				mp_substage = 0;
					}   break;

            default:
                return tk.Error("internal error in read_face_normals (2)");
        }
    }
    return status;
#else
	return tk.Error(stream_disable_ascii);
#endif 
} //end function TK_Polyhedron::read_face_normals


TK_Status TK_Polyhedron::read_edge_visibilities_ascii (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII

	TK_Status       status = TK_Normal;
    int i;

    if (mp_optopcode == OPT_ALL_EDGE_VISIBILITIES) {
        switch (mp_substage) {
			case 0: {
				if ((status = ReadAsciiWord(tk,0)) != TK_Normal)
					return status;
				mp_substage++;
			}   nobreak;

            //compression scheme
            case 1: {
                if ((status = GetAsciiData(tk, "Compression_Scheme", mp_compression_scheme)) != TK_Normal)
                    return status;
                SetEdgeVisibilities();
                mp_substage++;
            }   nobreak;

            //all visibilities
            case 2: {
                mp_evisibilitycount = mp_edgecount;
                if ((status = GetAsciiData(tk, "Visibilities", mp_evisibilities, mp_evisibilitycount)) != TK_Normal)
                    return status;
                for (i = 0; i < mp_edgecount; i++)
                    mp_edge_exists[i] |= Edge_Visibility;
                 mp_substage++; 
				}   nobreak;

			case 3: {
				if ((status = ReadAsciiWord(tk,0)) != TK_Normal)
					return status;
				mp_substage = 0;
					}   break;

            default:
                return tk.Error("internal error in read_edge_visibilities (1)");
        }
    }
    else {
        switch (mp_substage) {
			case 0: {
				if ((status = ReadAsciiWord(tk,0)) != TK_Normal)
					return status;
				mp_substage++;
			}   nobreak;

            //compression scheme
            case 1: {
                if ((status = GetAsciiData(tk, "Compression_Scheme", mp_compression_scheme)) != TK_Normal)
                    return status;
                mp_substage++;
            }   nobreak;

            //count
            case 2: {
				if ((status = GetAsciiData(tk, "Visiblity_Count",mp_evisibilitycount)) != TK_Normal)
                    return status;
                mp_substage++;
                mp_progress = 0;
            }   nobreak;

            //and indices of existing values
            case 3: {
                while (mp_progress < mp_evisibilitycount) {
                    int     index;

                    if (mp_edgecount < 256) {
						if ((status = GetAsciiData(tk, "Progress", m_byte)) != TK_Normal)
                            return status;
                        index = m_byte;
                    }
                    else if (mp_edgecount < 65536) {
                        if ((status = GetAsciiData(tk, "Progress", m_unsigned_short)) != TK_Normal)
                            return status;
                        index = m_unsigned_short;
                    }
                    else {
                        if ((status = GetAsciiData(tk, "Progress", m_int)) != TK_Normal)
                            return status;
						index = m_int;
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
            case 4: {
                while (mp_progress < mp_edgecount) {
                    if (mp_edge_exists[mp_progress] & Edge_Visibility) {
						if ((status = GetAsciiData(tk, "Visibilities", mp_evisibilities[mp_progress])) != TK_Normal)
                            return status;
                    }
                    mp_progress++;
                }
                mp_progress = 0;
                mp_substage++; 
				}   nobreak;

			case 5: {
				if ((status = ReadAsciiWord(tk,0)) != TK_Normal)
					return status;
				mp_substage = 0;
					}   break;

            default:
                return tk.Error("internal error in read_edge_visibilities (2)");
        }
    }
    return status;
#else
	return tk.Error(stream_disable_ascii);
#endif 
} //end function TK_Polyhedron::read_edge_visibilities


TK_Status TK_Polyhedron::read_edge_patterns_ascii (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII

	TK_Status       status = TK_Normal;
    int i;

    if (mp_optopcode == OPT_ALL_EDGE_PATTERNS) {
        switch (mp_substage) {
			case 0: {
				if ((status = ReadAsciiWord(tk,0)) != TK_Normal)
					return status;
				mp_substage++;
			}   nobreak;
            //compression scheme
            case 1: {
                if ((status = GetAsciiData(tk, "Compression_Scheme", mp_compression_scheme)) != TK_Normal)
                    return status;
                SetEdgePatterns();
                mp_substage++;
            }   nobreak;

            //all patterns
            case 2: {
                mp_epatterncount = mp_edgecount;
				if ((status = GetAsciiData(tk, "Patterns", mp_epatterns, mp_epatterncount)) != TK_Normal)
                    return status;
                for (i = 0; i < mp_edgecount; i++)
                    mp_edge_exists[i] |= Edge_Pattern;
                   mp_substage++; 
				}   nobreak;

			case 3: {
				if ((status = ReadAsciiWord(tk,0)) != TK_Normal)
					return status;
				mp_substage = 0;
					}   break;

            default:
                return tk.Error("internal error in read_edge_patterns (1)");
        }
    }
    else {
        switch (mp_substage) {
			case 0: {
				if ((status = ReadAsciiWord(tk,0)) != TK_Normal)
					return status;
				mp_substage++;
			}   nobreak;
            //compression scheme
            case 1: {
                if ((status = GetAsciiData(tk, "Compression_Scheme", mp_compression_scheme)) != TK_Normal)
                    return status;
                mp_substage++;
            }   nobreak;

            //count
            case 2: {
				if ((status = GetAsciiData(tk, "PatternCount", mp_epatterncount)) != TK_Normal)
                    return status;
                mp_substage++;
                mp_progress = 0;
            }   nobreak;

            //and indices of existing values
            case 3: {
                while (mp_progress < mp_epatterncount) {
                    int     index;
                    if (mp_edgecount < 256) {
						if ((status = GetAsciiData(tk, "Progress", m_byte)) != TK_Normal)
                            return status;
                        index = m_byte;
                    }
                    else if (mp_edgecount < 65536) {
                        if ((status = GetAsciiData(tk, "Progress", m_unsigned_short)) != TK_Normal)
                            return status;
                        index = m_unsigned_short;
                    }
                    else {
                        if ((status = GetAsciiData(tk, "Progress", m_int)) != TK_Normal)
                            return status;
						index = m_int;
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
            case 4: {
                while (mp_progress < mp_edgecount) {
                    if (mp_edge_exists[mp_progress] & Edge_Pattern) {
                        switch (mp_subprogress) {
                            case 0: {
								if ((status = GetAsciiData(tk, "Patterns", mp_epatterns[mp_progress])) != TK_Normal)
                                    return status;
                                if (mp_epatterns[mp_progress] != 0xFF)
                                    break;
                                SetEdgePatternStrings();
                                mp_subprogress++;
                            }   nobreak;
                            case 1: {
                               
                                int             length;

								if ((status = GetAsciiData(tk,  "String_Length", m_int)) != TK_Normal)
                                    return status;
                                length = m_int;
                                mp_epatternstrings[mp_progress] = new char[length+1];
                                mp_epatternstrings[mp_progress][length] = '\0';
                                mp_epatternstrings[mp_progress][0] = (char)length;
                                mp_subprogress++;
                            }   nobreak;
                            case 2: {
                                int             length = (int)mp_epatternstrings[mp_progress][0];

								if ((status = GetAsciiData(tk,  "Pattern_Strings",mp_epatternstrings[mp_progress], length)) != TK_Normal)
                                    return status;
                                mp_subprogress = 0;
                            }   break;
                        }
                    }
                    mp_progress++;
                }
                mp_progress = 0;
                mp_substage++; 
				}   nobreak;

			case 5: {
				if ((status = ReadAsciiWord(tk,0)) != TK_Normal)
					return status;
				mp_substage = 0;
					}   break;

            default:
                return tk.Error("internal error in read_edge_patterns (2)");
        }
    }
    return status;
#else
return tk.Error(stream_disable_ascii);
#endif 
} //end function TK_Polyhedron::read_edge_patterns


TK_Status TK_Polyhedron::read_edge_colors_ascii (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII

	TK_Status       status = TK_Normal;
    int i;
    if (mp_optopcode == OPT_ALL_EDGE_COLORS) {
        switch (mp_substage) {
			case 0: {
				if ((status = ReadAsciiWord(tk,0)) != TK_Normal)
					return status;
				mp_substage++;
			}   nobreak;
            //compression scheme
            case 1: {
                if ((status = GetAsciiData(tk, "Compression_Scheme", mp_compression_scheme)) != TK_Normal)
                    return status;
                SetEdgeColors();
                mp_substage++;
            }   nobreak;

            //all colors
            case 2: {
                mp_ecolorcount = mp_edgecount;
				if ((status = GetAsciiData(tk, "Colors", mp_ecolors, 3*mp_ecolorcount)) != TK_Normal)
                    return status;
                for (i = 0; i < mp_edgecount; i++)
                    mp_edge_exists[i] |= Edge_Color;
                  mp_substage++; 
				}   nobreak;

			case 3: {
				if ((status = ReadAsciiWord(tk,0)) != TK_Normal)
					return status;
				mp_substage = 0;
					}   break;

            default:
                return tk.Error("internal error in read_edge_colors (1)");
        }
    }
    else {
        switch (mp_substage) {
			case 0: {
				if ((status = ReadAsciiWord(tk,0)) != TK_Normal)
					return status;
				mp_substage++;
			}   nobreak;
            //compression scheme
            case 1: {
                if ((status = GetAsciiData(tk, "Compression_Scheme", mp_compression_scheme)) != TK_Normal)
                    return status;
                mp_substage++;
            }   nobreak;

            //count
            case 2: {
				if ((status = GetAsciiData(tk, "Color_Count",mp_ecolorcount)) != TK_Normal)
                    return status;
                mp_substage++;
                mp_progress = 0;
            }   nobreak;

            //and indices of existing values
            case 3: {
                while (mp_progress < mp_ecolorcount) {
                    int     index;

                    if (mp_edgecount < 256) {
						if ((status = GetAsciiData(tk,"Progress",  m_byte)) != TK_Normal)
                            return status;
                        index = m_byte;
                    }
                    else if (mp_edgecount < 65536) {
                        if ((status = GetAsciiData(tk, "Progress", m_unsigned_short)) != TK_Normal)
                            return status;
                        index = m_unsigned_short;
                    }
                    else {
                        if ((status = GetAsciiData(tk, "Progress", m_int)) != TK_Normal)
                            return status;
						index = m_int;
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
            case 4: {
                while (mp_progress < mp_edgecount) {
                    if (mp_edge_exists[mp_progress] & Edge_Color) {
						if ((status = GetAsciiData(tk, "Colors", &mp_ecolors[3*mp_progress], 3)) != TK_Normal)
                            return status;
                    }
                    mp_progress++;
                }
                mp_progress = 0;
                  mp_substage++; 
				}   nobreak;

			case 5: {
				if ((status = ReadAsciiWord(tk,0)) != TK_Normal)
					return status;
				mp_substage = 0;
					}   break;

            default:
                return tk.Error("internal error in read_edge_colors (2)");
        }
    }
    return status;
#else
	return tk.Error(stream_disable_ascii);
#endif 
} //end function TK_Polyhedron::read_edge_colors


TK_Status TK_Polyhedron::read_edge_indices_ascii (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII

	TK_Status       status = TK_Normal;
    int i;

    if (mp_optopcode == OPT_ALL_EDGE_INDICES) {
        switch (mp_substage) {
			case 0: {
				if ((status = ReadAsciiWord(tk,0)) != TK_Normal)
					return status;
				mp_substage++;
			}   nobreak;
            //compression scheme
            case 1: {
                if ((status = GetAsciiData(tk, "Compression_Scheme", mp_compression_scheme)) != TK_Normal)
                    return status;
                SetEdgeIndices();
                mp_substage++;
            }   nobreak;

            //all indices
            case 2: {
                mp_eindexcount = mp_edgecount;
				if ((status = GetAsciiData(tk, "Indices", mp_eindices, mp_eindexcount)) != TK_Normal)
                    return status;
                for (i = 0; i < mp_edgecount; i++)
                    mp_edge_exists[i] |= Edge_Index;
                  mp_substage++; 
				}   nobreak;

			case 3: {
				if ((status = ReadAsciiWord(tk,0)) != TK_Normal)
					return status;
				mp_substage = 0;
					}   break;
            default:
                return tk.Error("internal error in read_edge_indices (1)");
        }
    }
    else {
        switch (mp_substage) {
			case 0: {
				if ((status = ReadAsciiWord(tk,0)) != TK_Normal)
					return status;
				mp_substage++;
			}   nobreak;
            //compression scheme
            case 1: {
                if ((status = GetAsciiData(tk, "Compression_Scheme", mp_compression_scheme)) != TK_Normal)
                    return status;
                mp_substage++;
            }   nobreak;

            //count
            case 2: {
				if ((status = GetAsciiData(tk,"Index_Count", mp_eindexcount)) != TK_Normal)
                    return status;
                mp_substage++;
                mp_progress = 0;
            }   nobreak;

            //and indices of existing values
            case 3: {
                while (mp_progress < mp_eindexcount) {
                    int     index;

                    if (mp_edgecount < 256) {
						if ((status = GetAsciiData(tk,"Progress",m_byte)) != TK_Normal)
                            return status;
                        index = m_byte;
                    }
                    else if (mp_edgecount < 65536) {
						if ((status = GetAsciiData(tk, "Progress", m_unsigned_short)) != TK_Normal)
                            return status;
                        index = m_unsigned_short;
                    }
                    else {
                        if ((status = GetAsciiData(tk, "Progress", m_int)) != TK_Normal)
                            return status;
						index = m_int;
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
            case 4: {
                while (mp_progress < mp_edgecount) {
                    if (mp_edge_exists[mp_progress] & Edge_Index) {
						if ((status = GetAsciiData(tk,"Indices", mp_eindices[mp_progress])) != TK_Normal)
                            return status;
                    }
                    mp_progress++;
                }
                mp_progress = 0;
                  mp_substage++; 
				}   nobreak;

			case 5: {
				if ((status = ReadAsciiWord(tk,0)) != TK_Normal)
					return status;
				mp_substage = 0;
					}   break;

            default:
                return tk.Error("internal error in read_edge_indices (2)");
        }
    }
    return status;
#else
	return tk.Error(stream_disable_ascii);
#endif 

} //end function TK_Polyhedron::read_edge_indices


TK_Status TK_Polyhedron::read_edge_weights_ascii (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII

	TK_Status       status = TK_Normal;
    int             i;

    if (mp_optopcode == OPT_ALL_EDGE_WEIGHTS) {
        switch (mp_substage) {
			case 0: {
				if ((status = ReadAsciiWord(tk,0)) != TK_Normal)
					return status;
				mp_substage++;
			}   nobreak;
            //compression scheme
            case 1: {
                if ((status = GetAsciiData(tk, "Compression_Scheme", mp_compression_scheme)) != TK_Normal)
                    return status;
                SetEdgeWeights();
                mp_substage++;
            }   nobreak;

            //all weights
            case 2: {
                mp_eweightcount = mp_edgecount;
				if ((status = GetAsciiData(tk, "Weights", mp_eweights, mp_edgecount)) != TK_Normal)
                    return status;
                for (i = 0; i < mp_edgecount; i++)
                    mp_edge_exists[i] |= Edge_Weight;
                 mp_substage++; 
				}   nobreak;

			case 3: {
				if ((status = ReadAsciiWord(tk,0)) != TK_Normal)
					return status;
				mp_substage = 0;
					}   break;
            default:
                return tk.Error("internal error in read_edge_weights (1)");
        }
    }
    else {
        switch (mp_substage) {
			case 0: {
				if ((status = ReadAsciiWord(tk,0)) != TK_Normal)
					return status;
				mp_substage++;
			}   nobreak;
            //compression scheme
            case 1: {
                if ((status = GetAsciiData(tk, "Compression_Scheme", mp_compression_scheme)) != TK_Normal)
                    return status;
                mp_substage++;
            }   nobreak;

            //count
            case 2: {
				if ((status = GetAsciiData(tk, "WeightCount", mp_eweightcount)) != TK_Normal)
                    return status;
                mp_substage++;
                mp_progress = 0;
            }   nobreak;

            //and indices of existing values
            case 3: {
                while (mp_progress < mp_eweightcount) {
                    int     index;

                    if (mp_edgecount < 256) {
						if ((status = GetAsciiData(tk, "Progress", m_byte)) != TK_Normal)
                            return status;
                        index = m_byte;
                    }
                    else if (mp_edgecount < 65536) {
                        if ((status = GetAsciiData(tk,"Progress",  m_unsigned_short)) != TK_Normal)
                            return status;
                        index = m_unsigned_short;
                    }
                    else {
                        if ((status = GetAsciiData(tk,"Progress",  m_int)) != TK_Normal)
                            return status;
						index = m_int;
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
            case 4: {
                while (mp_progress < mp_edgecount) {
                    if (mp_edge_exists[mp_progress] & Edge_Weight) {
						if ((status = GetAsciiData(tk, "Weights", mp_eweights[mp_progress])) != TK_Normal)
                            return status;
                    }
                    mp_progress++;
                }
                mp_progress = 0;
               mp_substage++; 
				}   nobreak;

			case 5: {
				if ((status = ReadAsciiWord(tk,0)) != TK_Normal)
					return status;
				mp_substage = 0;
					}   break;
            default:
                return tk.Error("internal error in read_edge_weights (2)");
        }
    }
    return status;
#else
	return tk.Error(stream_disable_ascii);
#endif 

} //end function TK_Polyhedron::read_edge_weights


TK_Status TK_Polyhedron::read_edge_normals_ascii (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII

	TK_Status       status = TK_Normal;
    int i;

    if (mp_optopcode == OPT_ALL_EDGE_NORMALS_POLAR) {
        switch (mp_substage) {
			case 0: {
				if ((status = ReadAsciiWord(tk,0)) != TK_Normal)
					return status;
				mp_substage++;
			}   nobreak;
            //compression scheme
            case 1: {
                if ((status = GetAsciiData(tk, "Compression_Scheme", mp_compression_scheme)) != TK_Normal)
                    return status;
                SetEdgeNormals();
                mp_substage++;
            }   nobreak;

            //all normals
            case 2: {
                mp_enormalcount = mp_edgecount;
				if ((status = GetAsciiData(tk, "Edge_Normals",mp_enormals, 2*mp_enormalcount)) != TK_Normal)
                    return status;
                normals_polar_to_cartesian(null, Edge_Normal, mp_edgecount, mp_enormals, mp_enormals);
                for (i = 0; i < mp_edgecount; i++)
                    mp_edge_exists[i] |= Edge_Weight;
                  mp_substage++; 
				}   nobreak;

			case 3: {
				if ((status = ReadAsciiWord(tk,0)) != TK_Normal)
					return status;
				mp_substage = 0;
					}   break;

            default:
                return tk.Error("internal error in read_edge_normals (1)");
        }
    }
    else {
        switch (mp_substage) {
			case 0: {
				if ((status = ReadAsciiWord(tk,0)) != TK_Normal)
					return status;
				mp_substage++;
			}   nobreak;
            //compression scheme
            case 1: {
                if ((status = GetAsciiData(tk, "Compression_Scheme", mp_compression_scheme)) != TK_Normal)
                    return status;
                mp_substage++;
            }   nobreak;

            //count
            case 2: {
				if ((status = GetAsciiData(tk,"Edge_Normal_Count",mp_enormalcount)) != TK_Normal)
                    return status;
                mp_substage++;
                mp_progress = 0;
            }   nobreak;

            //and indices of existing values
            case 3: {
                while (mp_progress < mp_enormalcount) {
                    int     index;

                    if (mp_edgecount < 256) {
						if ((status = GetAsciiData(tk,"Progress",  m_byte)) != TK_Normal)
                            return status;
                        index = m_byte;
                    }
                    else if (mp_edgecount < 65536) {
                       if ((status = GetAsciiData(tk, "Progress", m_unsigned_short)) != TK_Normal)
                            return status;
                        index = m_unsigned_short;
                    }
                    else {
                        if ((status = GetAsciiData(tk, "Progress", m_int)) != TK_Normal)
                            return status;
						index = m_int;
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
            case 4: {
                while (mp_progress < mp_edgecount) {
                    if (mp_edge_exists[mp_progress] & Edge_Normal) {
						if ((status = GetAsciiData(tk, "Edge_Normals", mp_enormals[mp_progress])) != TK_Normal)
                            return status;
                    }
                    mp_progress++;
                }
                normals_polar_to_cartesian(mp_edge_exists, Edge_Normal, mp_edgecount, mp_enormals, mp_enormals);
                mp_progress = 0;
                  mp_substage++; 
				}   nobreak;

			case 5: {
				if ((status = ReadAsciiWord(tk,0)) != TK_Normal)
					return status;
				mp_substage = 0;
					}   break;
            default:
                return tk.Error("internal error in read_edge_normals (2)");
        }
    }
    return status;
#else
	return tk.Error(stream_disable_ascii);
#endif 

} //end function TK_Polyhedron::read_edge_normals


TK_Status TK_Polyhedron::read_collection_ascii (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII

	TK_Status					status = TK_Normal;
    unsigned   char				opcode = '\0';
    int				word_count = 0;
	   char				temp_opcode[512];	
//	   unsigned int		sequence;
	   unsigned char	opcode_word = '\0';
     int				i;

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
           /* if ((status = GetData (tk, opcode)) != TK_Normal)
                return status;*/

// The following code is added because this is a special case where one opcode is being called by another opcode
// So here we need to use the strategy that is used in "TK_Default::ReadAscii". This strategy reads the opcode and
// that what we want. The ""GetAsciidata" function reads the tags but can not deal with Opcode reading - Pavan Totala Jul-12-06 

			temp_opcode[0] = '\0';

			LookatData( tk, opcode_word );
			while( opcode_word == 13 || opcode_word == 10 || opcode_word == 9 ) { // read CR,LF
				GetData (tk, opcode_word);
				LookatData( tk, opcode_word );
			}

			if ((status = GetData (tk, opcode_word)) != TK_Normal)
				return status;

			if ( opcode_word == 0 )
				return TK_Complete;

			temp_opcode[word_count++] = opcode_word;
			while ( opcode_word != '>' ) {	// LF character
				if ((status = GetData (tk, opcode_word)) != TK_Normal)
					return status;
				temp_opcode[word_count++] = opcode_word;
			}

			if ( strni_equal( "<TKE_", temp_opcode,5) != true ) {
				return tk.Error(" error, expected <OPCODE\n");
			}

			if(!( RemoveAngularBrackets(temp_opcode)))
			status = TK_Error;

			size_t len = strlen(temp_opcode);
			while(len != 24)
			{
				temp_opcode[len] = ' ';
				len++;
			}
			temp_opcode[len] = 0;

			int i;
			for ( i=0; i<256; i++ ) {
				if ( strni_equal( (const char *)& temp_opcode[4], (const char *)opcode_list[i],(word_count)) == true )
						break;	// found opcode
				}
			opcode = (unsigned char)i;
			word_count = 0;

// End of added code

            if (opcode == TKE_Termination) {
                mp_num_collection_parts = mp_substage;
			
					switch (m_progress) {
					case 0: {
							   if ((status =  SkipNewlineAndTabs(tk, 0)) != TK_Normal)
									return status;
							   m_progress++;
							}nobreak;
					case 1: {
							   if ((status = ReadEndOpcode(tk))!= TK_Normal)
									return status;
							   m_progress = 0;
							}break;
					default:
							return tk.Error("internal error in read_collection_ascii");
					}
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

#else
	return tk.Error(stream_disable_ascii);
#endif 

} // end function TK_Polyhedron::read_collection


//protected virtual
TK_Status TK_Polyhedron::ReadAscii (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII

	TK_Status       status		= TK_Normal;

    if (mp_exists == null)
        SetExists();

    for (;;) {
        if (mp_stage == 0) {
			if ((status = GetAsciiHex(tk,"Optional_Opcode", m_int)) != TK_Normal)
				return status;
			mp_optopcode = (unsigned char) m_int;
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
#else
	return tk.Error(stream_disable_ascii);
#endif 

} //end function TK_Polyhedron::Read

TK_Status TK_Mesh::WriteAscii (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII

	TK_Status       status = TK_Normal;

    if (mp_instance != null)
        return mp_instance->Write (tk);

	PutTab t0(&tk);

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
            if ((status = PutAsciiOpcode (tk)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        //subop
        case 2: {  
			PutTab t(&tk);
            if ((status = PutAsciiFlag (tk,"SubOptions", mp_subop)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        //subop2 (if applicable)
        case 3: {
			PutTab t(&tk);
            if (mp_subop & TKSH_EXPANDED) {
                if ((status = PutAsciiHex(tk,"SubOptions2", mp_subop2)) != TK_Normal)
                    return status;
            }
            m_stage++;
        }   nobreak;

        //number rows
        case 4: {
			PutTab t(&tk);
            if ((status = PutAsciiData (tk,"Rows", m_rows)) != TK_Normal)
                return status;
            m_progress = 0;
            m_stage++;
        }   nobreak;

        //number of columns
        case 5: {
			PutTab t(&tk);
            if ((status = PutAsciiData (tk,"Columns", m_columns)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        //points
        case 6: {
			PutTab t(&tk);
           /* if (mp_subop & TKSH_COMPRESSED_POINTS) {
                mp_compression_scheme = CS_TRIVIAL;
                if ((status = write_trivial_points(tk)) != TK_Normal)
                    return status;
            }
            else*/
                if ((status = PutAsciiData (tk, "Points",mp_points, 3 * m_rows * m_columns)) != TK_Normal)
                    return status;
            m_stage++;
        }   nobreak;

        //attributes
        case 7: {
			PutTab t(&tk);
            if (mp_subop & TKSH_HAS_OPTIONALS) {
                if ((status = TK_Polyhedron::Write(tk)) != TK_Normal)
                        return status;
				}
				m_stage++;
			}   nobreak;

		case 8: {
			if ((status = PutAsciiOpcode (tk,1,true,true)) != TK_Normal)
				return status;
			m_stage++;
				}  nobreak;

        //instance registration (if applicable)
        case 9: {
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
	return tk.Error(stream_disable_ascii);
#endif 

} // end function TK_Mesh::Write


TK_Status TK_Mesh::ReadAscii (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII

	TK_Status       status = TK_Normal;

    switch (m_stage) {
        case 0: {
            if (tk.GetVersion() > 100) {  //subop info added after 1.00
				if( (status = GetAsciiHex( tk, "SubOptions", mp_subop )) != TK_Normal ){
					return tk.Error("TK_Mesh::expected SubOptions: or its value not found");
				}
            }
            else
                mp_subop = 0; //assume defaults for early file versions
            m_stage++;
        }   nobreak;

        case 1: {

			if( (status = GetAsciiData( tk, "Rows", m_rows )) != TK_Normal ) {
				return tk.Error("TK_Mesh::expected Rows: or its value not found");
			}

            m_stage++;
        }   nobreak;

        case 2: {

			if( (status = GetAsciiData( tk, "Columns", m_columns )) != TK_Normal ) {
				return tk.Error("TK_Mesh::expected Columns: or its value not found");
			}

            mp_pointcount = m_rows * m_columns;
            if (!validate_count (mp_pointcount))
                return tk.Error("bad Mesh Point count");
            mp_facecount = 2 * (m_rows-1) * (m_columns-1);
            m_stage++;
        }   nobreak;

        case 3: {
    //        if (mp_subop & TKSH_COMPRESSED_POINTS){
				//if ((status = GetAsciiData(tk, "Compression_Scheme", mp_compression_scheme)) != TK_Normal)
    //                return status;
    //            // no need to set_points() here since unpack floats (called from read_trivial) will do that for us
    //        }
    //        else {
                set_points(mp_pointcount);
        //    }
            m_stage++;
        }   nobreak;

        case 4: {
           /* if (mp_subop & TKSH_COMPRESSED_POINTS) {
                if ((status = read_trivial_points(tk)) != TK_Normal)
                    return status;
            }
            else*/
                if ((status = GetAsciiData (tk, "Points", mp_points, 3 * mp_pointcount)) != TK_Normal)
                    return status;

            m_stage++;
        }   nobreak;

        case 5: {
            // here is where we (optionally) get vertex and face attributes 
            if (mp_subop & TKSH_HAS_OPTIONALS) {
                if ((status = TK_Polyhedron::Read(tk)) != TK_Normal)
                    return status;
            }
                 m_stage++;
        }   nobreak;
		
		case 6: {
			if ((status = ReadEndOpcode(tk))!= TK_Normal)
					return status;

			m_stage = -1;
		}   break;

        default:
            return tk.Error();
    }
    return status;
#else
	return tk.Error(stream_disable_ascii);
#endif 

} // end function TK_Mesh::Read


