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
// $Header: //DWF/Development/Components/Internal/DWF Toolkit/v7.7/develop/global/src/dwf/w3dtk/BOpcodeShellAscii.cpp#2 $
//

#include "dwf/w3dtk/BStream.h"
#include "dwf/w3dtk/BOpcodeShell.h"
#include "dwf/w3dtk/BPolyhedron.h"
#include "dwf/w3dtk/BCompress.h"
#include "dwf/w3dtk/BPointCloud.h"
#include "dwf/w3dtk/BInternal.h"
#include <stdlib.h>

#define ASSERT(x)


//protected virtual
TK_Status TK_Shell::write_uncompressed_points_ascii (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII

    TK_Status       status = TK_Normal;
	PutTab t0(&tk);
    
	switch (m_substage) {
		case 0: {
			if ((status = PutStartXMLTag(tk,"Points")) != TK_Normal)
				return status;
			m_substage++;
				}   nobreak;
		case 1: {
			PutTab t(&tk);
            if ((status = PutAsciiData(tk,"Point_Count", mp_pointcount)) != TK_Normal)
                return status;
            m_substage++;
        }   nobreak;
    
        case 2: {
			PutTab t(&tk);
            if (mp_pointcount != 0)
                if ((status = PutAsciiData(tk,"Coordinates", mp_points, 3*mp_pointcount)) != TK_Normal)
                    return status;
			m_substage++;
				} nobreak;

		case 3: {
			if ((status = PutEndXMLTag(tk,"Points")) != TK_Normal)
				return status;
			m_substage = 0;
				}   break;

        default:
            return tk.Error();
    }
	
    return status;
#else
	return tk.Error(stream_disable_ascii);
#endif 


} // end function TK_Shell::write_uncompressed_points



// write_bounding
// protected virtual
// On the first pass we (depending on the toolkit's write flags) write only the 
// bounding information.  Since all we have here is the bbox, we don't pay attention
// to TKSH2_GLOBAL_QUANTIZATION
TK_Status TK_Shell::write_bounding_ascii (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII

    TK_Status       status = TK_Normal;
	
	PutTab t0(&tk);

    switch (m_substage) {
        //opcode
        case 0: {  
            if ((status = PutAsciiOpcode (tk, 0)) != TK_Normal)
                return status;
            m_substage++;
        }   nobreak;

        //subop
        case 1: {  
			PutTab t(&tk);
            ASSERT(!(mp_subop & TKSH_EXPANDED));
            if ((status = PutAsciiFlag (tk,"SubOptions", mp_subop)) != TK_Normal)
                return status;
            m_substage++;
        }   nobreak;

        //lodlevel
        case 2: {
			PutTab t(&tk);
            if ((status = PutAsciiData(tk,"LOD_Level", m_lodlevel)) != TK_Normal)
                return status;
            m_substage++;
        }   nobreak;

        //bounding box (represented as 2 corners)
        case 3: {
			PutTab t(&tk);
            if ((status = PutAsciiData(tk,"Bounding_Box", mp_bbox, 6)) != TK_Normal)
                return status;
			m_substage++;
        } nobreak;

        //tag
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
	return tk.Error(stream_disable_ascii);
#endif 

} //end function TK_Shell::write_bounding


//protected virtual
//We might be writing tristrips or even trifans here.  It should require no differences, though.
TK_Status TK_Shell::write_uncompressed_faces_ascii (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII

	TK_Status       status = TK_Normal;
	
	PutTab t0(&tk);


	switch (m_substage) {

		//compression scheme
		case 0: {
			if ((status = PutStartXMLTag(tk,"Faces")) != TK_Normal)
				return status;
			m_substage++;
				}   nobreak;

		case 1: {
			PutTab t(&tk);
			mp_compression_scheme = CS_TRIVIAL;
			if ((status = PutAsciiData(tk,"Compression_Scheme", (int)mp_compression_scheme)) != TK_Normal)
				return status;
			m_substage++;
				}   nobreak;

			//workspace length
		case 2: {
			PutTab t(&tk);
			if ((status = PutAsciiData(tk,"Face_List_Length", m_flistlen)) != TK_Normal)
				return status;
			m_substage++;
				}   nobreak;

			//the data
		case 3: {
			if (m_flistlen != 0)
			{
				PutTab t(&tk);
				if ((status = PutAsciiData(tk,"Face_List", m_flist, m_flistlen)) != TK_Normal)
					return status;
			}
			m_substage++;
			}   nobreak;
		
		case 4: {

			if ((status = PutEndXMLTag(tk,"Faces")) != TK_Normal)
				return status;
			m_substage = 0;
				}   break;

		default:
			return tk.Error();
	}
	
	return status;
#else
	return tk.Error(stream_disable_ascii);
#endif 

} // end function TK_Shell::write_uncompressed_faces



// write null
// protected nonvirtual
// On the first pass we write-only the bounding information.
TK_Status TK_Shell::write_null_ascii (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII

    TK_Status       status = TK_Normal;
	
    switch (m_substage) {
        //opcode
        case 0: {  
            if ((status = PutAsciiOpcode (tk, 0)) != TK_Normal)
                return status;
            m_substage++;
        }   nobreak;

        //subop
        case 1: {
			PutTab t(&tk);
            //TKSH2_NULL makes a number of attributes meaningless
            mp_subop &= ~(TKSH_BOUNDING_ONLY | 
                TKSH_COMPRESSED_POINTS | 
                TKSH_COMPRESSED_FACES |
                TKSH_HAS_OPTIONALS |
                TKSH_TRISTRIPS |
                TKSH_BOUNDING_ONLY |
                TKSH_CONNECTIVITY_COMPRESSION); 
            if ((status = PutAsciiFlag(tk,"SubOptions", mp_subop)) != TK_Normal)
                return status;
            m_substage++;
        }   nobreak;

        //subop2
        case 2: {  
			PutTab t(&tk);
            mp_subop &= ~(
                TKSH2_COLLECTION |          
                TKSH2_HAS_NEGATIVE_FACES |
                TKSH2_GLOBAL_QUANTIZATION);
			
            if ((status = PutAsciiHex(tk,"SubOptions2",(int) mp_subop2)) != TK_Normal)
                return status;
            m_substage++;
        }   nobreak;

        //lodlevel
        case 3: {
			PutTab t(&tk);
            if ((status = PutAsciiData(tk,"LOD_Level", (int)m_lodlevel)) != TK_Normal)
                return status;
            m_substage++;
        }   nobreak;

        //tag
        case 4: {
            if ((status = Tag(tk, -1)) != TK_Normal)
                return status;
            m_substage++;
        }  nobreak;

		case 5: {
			if ((status = PutAsciiOpcode (tk,0,true,true)) != TK_Normal)
				return status;
			m_substage = -1;
				}   break;

        default:
            return tk.Error();
    }
	
    return status;
#else
	return tk.Error(stream_disable_ascii);
#endif 

} //end function TK_Shell::write_null

TK_Status TK_Shell::compute_advanced_ascii(BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII
    TK_Status       status = TK_Normal;
	UNREFERENCED (tk);
	return status;

#else
	return tk.Error(stream_disable_ascii);
#endif 
}

TK_Status TK_Shell::write_advanced_ascii(BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII

    TK_Status       status = TK_Normal;

	PutTab t(&tk);

    switch (m_substage) {

        case 0: {
            if ((status = PutAsciiData(tk,"Compression_Scheme",(int) mp_compression_scheme)) != TK_Normal)
                return status;
            m_substage++;
        } nobreak;

        //length of data
        case 1: {
            if ((status = PutAsciiData(tk,"Workspace_Used", mp_workspace_used)) != TK_Normal)
                return status;
            m_progress = 0;
            m_substage++;
        } nobreak;

        //the data
        case 2: {
            if ((status = PutAsciiData(tk,"WorkSpace", mp_workspace, mp_workspace_used)) != TK_Normal)
                return status;

            m_progress = 0;
            // prior to ver 6.51, full-resolution vertices and edgebreaker were mutually exclusive
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
                if ((status = PutAsciiData(tk,"Points", mp_points, 3*mp_pointcount)) != TK_Normal)
                    return status;
			m_substage = 0;
				}   break;
        default:
            return tk.Error();
    }
	
    return status;
#else
return tk.Error(stream_disable_ascii);
#endif 

} //end function TK_Shell::write_advanced


//public virtual
TK_Status TK_Shell::WriteAscii (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII
    TK_Status       status = TK_Normal;
    int             index;
    int             *ptr;
	PutTab t0(&tk);

    //(note: careful! last stage behaves differently from the others--mstage=-1 and break)
    switch (m_stage) {
        //preprocessing. 
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

            // the success/failure of edgebreaker (if appropriate) may affect the flags we
            // put in subop.
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

        //opcode
        case 1: {
			
            int     count_it = 0;

            // This might not catch a truly empty shell with Priority_Heuristic disabled,
            // not worth worrying about for now
            if (m_lodlevel == 0 && (GetPointCount() != 0 || Pass (tk) != 0))
                count_it = 1;

            if ((status = PutAsciiOpcode (tk, count_it)) != TK_Normal)
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

        //file scope id
        //we indicate that we are currently refining a previously given HOOPS object
        case 4: {
			PutTab t(&tk);
            if (!(mp_subop & TKSH_FIRSTPASS)) {
                if ((status = tk.KeyToIndex(mp_key, index)) != TK_Normal)
                    return tk.Error();
                if ((status = PutAsciiData(tk,"Index", index)) != TK_Normal)
                    return status;
            }
            m_stage++;
        } nobreak;

        //lod level
        case 5: {
			PutTab t(&tk);
			 if ((status = PutAsciiData(tk,"LOD_Level", (int)m_lodlevel)) != TK_Normal)
                    return status;
            m_stage++;
        } nobreak;

        case 6: {
            if (mp_subop2 & TKSH2_COLLECTION) {
                return write_collection (tk, true, ((mp_subop & TKSH_FIRSTPASS) ? -1 : m_lodlevel));
            }
            else switch (mp_compression_scheme) {
				// ASCII: Ignore the compression for readability - Pavan Totala (31-May-06)
                case CS_EDGEBREAKER:
                case CS_TRIVIAL:
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
                    //do nothing, it was handled by write advanced in the previous stage
                    //break;      
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

        //optionals
        //(optionally) vertex and face attributes 
        case 8: {
            if (mp_subop & TKSH_HAS_OPTIONALS) {
                if ((status = TK_Polyhedron::Write(tk)) != TK_Normal)
                    return status;
            }
            m_stage++;
        }   nobreak;

		case 9: {
			if ((status = PutAsciiOpcode (tk,1,true,true)) != TK_Normal)
				return status;
			m_stage++;
				}   nobreak;

		// We have moved the TKE_Shell End opcode before the TKE_Tag because once the program writes TKE_Tag and then 
		// TKE_Shell end opcode. But while reading there is nothing which calles read of TKE_Tag so the ReadEndoPcode 
		// reads TKE_Tag instead of TKE_Shell which causes the error. This doesnt happen in Binary read. Pavan T 21 Jul 06
        //tag

        case 10: {
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
return tk.Error(stream_disable_ascii);
#endif 

} // end function TK_Shell::Write 


//protected virtual
TK_Status TK_Shell::read_advanced_ascii (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII
#ifndef BSTREAM_DISABLE_EDGEBREAKER
    TK_Status       status = TK_Normal;
    int             success = 1;
    const float     *global_bbox;
    bool            by_tristrips = false;
    eb_decompress_configs configs = {
        null, null, null, null, null
    };

	switch (m_substage) {
        //length of the data
        case 0: {
			if ((status = GetAsciiData(tk, "Workspace_Used:", mp_workspace_used)) != TK_Normal)
                return status;
            if (mp_workspace_allocated < mp_workspace_used) {
                mp_workspace_allocated = mp_workspace_used;
                ENSURE_ALLOCATED(mp_workspace, unsigned char, mp_workspace_allocated);
            }
            m_substage++;
        } nobreak;

        case 1: {

            //get the workspace data
            if ((status = GetAsciiData(tk, "WorkSpace:", mp_workspace, mp_workspace_used)) != TK_Normal)
                return status;

            if (mp_subop2 & TKSH2_GLOBAL_QUANTIZATION) {
                global_bbox = tk.GetWorldBounding();
                //these things should have already been checked at the start of shell write
                ASSERT(configs.target_version >= 806);
                ASSERT(global_bbox);
                configs.bounding = (ET_Bounding *)global_bbox;
            }

            //process the workspace into points and pointcount
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

            // prior to ver 6.51, full-resolution vertices and edgebreaker were mutually exclusive
            if ((mp_subop & TKSH_COMPRESSED_POINTS) ||
                (tk.GetVersion() < 651)) {
                m_substage = 0;
                return status;
            }
            else
                m_substage++;
        }   nobreak;

        case 2: {
            if ((status = GetAsciiData(tk, "Points:",  mp_points, 3 * mp_pointcount)) != TK_Normal)
                return status;
            m_substage = 0; // done reading points 
        }   break;  

        default:
            return tk.Error("internal error: unrecognized case in TK_Shell::read_advanced");
    }
    return status;
#else
    return tk.Error (stream_disable_edgebreaker);
#endif

#else
	return tk.Error(stream_disable_ascii);
#endif 

} // end function TK_Shell::read_advanced


//protected virtual
TK_Status TK_Shell::read_uncompressed_faces_ascii (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII

	TK_Status       status = TK_Normal;

	switch (m_substage) {
		//compression scheme
		case 0: {
			if ((status = ReadAsciiWord(tk,0)) != TK_Normal)
				return status;
			m_substage++;
				}   nobreak;

		case 1: {
			if ((status = GetAsciiData(tk, "Compression_Scheme",mp_compression_scheme)) != TK_Normal)
				return status;
			ASSERT(mp_compression_scheme == CS_TRIVIAL);
			m_substage++;
				}   nobreak;

		case 2: {
			if ((status = GetAsciiData(tk,"Face_List_Length", m_flistlen)) != TK_Normal)
				return status;

			ENSURE_ALLOCATED(m_flist, int, m_flistlen);
			m_substage++;
			}   nobreak;

			//the data
		case 3: {
			if (m_flistlen != 0)
			{
				if ((status = GetAsciiData(tk,"Face_List", m_flist, m_flistlen)) != TK_Normal)
					return status;
			}
			m_substage++;
			} nobreak;

		case 4: {
			if ((status = ReadAsciiWord(tk,0)) != TK_Normal)
				return status;
			m_substage = 0;
				}   break;

		default:
			return tk.Error();
	}
	return status;
#else
	return tk.Error(stream_disable_ascii);
#endif 

} // end function TK_Shell::read_uncompressed_faces




//protected virtual
TK_Status TK_Shell::read_uncompressed_points_ascii (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII

    TK_Status       status = TK_Normal;

    switch (m_substage) {

		case 0: {
			if ((status = ReadAsciiWord(tk,0)) != TK_Normal)
				return status;
			m_substage++;
				}   nobreak;

        case 1: {
            if ((status = GetAsciiData(tk, "Point_Count", mp_pointcount)) != TK_Normal)
                return status;
            if (!validate_count (mp_pointcount))
                return tk.Error("bad Shell Point count");
            set_points(mp_pointcount);
            m_substage++;
        }   nobreak;

        case 2: {
			if( mp_pointcount != 0)
				if ((status = GetAsciiData(tk, "Coordinates",  mp_points, 3 * mp_pointcount)) != TK_Normal)
					return status;
			m_substage++;
				} nobreak;

		case 3: {
			if ((status = ReadAsciiWord(tk,0)) != TK_Normal)
				return status;
			m_substage = 0;
				}   break;

        default:
            return tk.Error();
    }
    return status;
#else
	return tk.Error(stream_disable_ascii);
#endif 

} //end function TK_Shell::read_uncompressed_points




//protected virtual
TK_Status TK_Shell::read_bounding_ascii (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII
    TK_Status       status = TK_Normal;
    switch (m_substage) {
        case 0: {
            if (mp_subop & TKSH_EXPANDED) {
				int i_subop2 = (int) mp_subop2;
				if ((status = GetAsciiHex(tk, "SubOptions:", i_subop2)) != TK_Normal)
                    return status;
            }
            m_substage++;
        }   nobreak;

        case 1: {
			if ((status = GetAsciiData(tk, "LOD_Level:", m_lodlevel)) != TK_Normal)
                return status;
            m_substage++;
        }   nobreak;

        case 2: {
			if ((status = GetAsciiData(tk, "Bounding_Box:",mp_bbox, 6)) != TK_Normal)
                return status;
            m_substage = 0; // done reading bounding 
        }   break;

        default:
            return tk.Error();
    }
    return status;
#else
	return tk.Error(stream_disable_ascii);
#endif 

} // end function TK_Shell::read_bounding


//public virtual
TK_Status TK_Shell::ReadAscii (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII

    TK_Status       status = TK_Normal;
    int             i, len;

    //subop
    //Stage 0 has to be handled differently because, depending on the results,
    //   it can send us into "bounding only" mode.
    if (m_stage == 0) {
		if ((status = GetAsciiHex(tk, "SubOptions", mp_subop)) != TK_Normal)
            return status;
        m_stage++;
    }
    if (mp_subop & TKSH_BOUNDING_ONLY)
        return read_bounding(tk);

    switch (m_stage) {

        //subop2 (if applicable)
        case 1: {
            if (mp_subop & TKSH_EXPANDED) {
				if ((status = GetAsciiHex(tk, "SubOptions2", mp_subop2)) != TK_Normal)
                    return status;
            }
            m_stage++;
        } nobreak;

        //file scope id -- we are refining a previously given opcode
        case 2: {
            if (!(mp_subop & TKSH_FIRSTPASS)) {
				if ((status = GetAsciiData(tk, "Index", m_int)) != TK_Normal)
                    return status;
                if ((status = tk.IndexToKey(m_int, mp_key)) != TK_Normal)
                    return tk.Error();
            }
            m_stage++;
        } nobreak;

        //lod level
        case 3: {
			if ((status = GetAsciiData(tk, "LOD_Level", m_int)) != TK_Normal)
                return status;
			m_lodlevel = (char)m_int;
            m_stage++;
        } nobreak;

        //compression scheme
        case 4: {
            if (mp_subop2 & TKSH2_NULL)
			{
				status = ReadEndOpcode(tk);
					return status;
			}
			
			if (mp_subop2 & TKSH2_COLLECTION)
			{
               status = read_collection(tk);
				m_stage = -1;
			   	   return status;
			}

			mp_compression_scheme = CS_NONE;
            m_stage++;
        } nobreak;

        //points (or faces and points together in the case of CS_EDGEBREAKER)
        case 5: {
            switch (mp_compression_scheme) {
                case CS_EDGEBREAKER:
                case CS_TRIVIAL:
                case CS_NONE:
                    if ((status = read_uncompressed_points(tk)) != TK_Normal)
                        return status;
                    break;
                default:
                    return tk.Error();
            }
            m_stage++;
        }   nobreak;

        //faces (if applicable)
        case 6: {
            switch (mp_compression_scheme) {
                case CS_EDGEBREAKER:
                    //do nothing.  Already handled by read advanced
                   // break;
                case CS_TRIVIAL:
                default:
                if ((status = read_uncompressed_faces(tk)) != TK_Normal)
                    return status;
            }
            if (mp_subop & TKSH_TRISTRIPS) {
                i = 0;
                while (i < m_flistlen) {
                    if (m_flist[i] > 0)
                        len = m_flist[i];   //a triangle strip
                    else
                        len = - m_flist[i]; //a triangle fan
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
                        //a continuation (to define a hole in) the previous face
                        i += -m_flist[i] + 1;
                    }
                }
            }
            m_stage++;
        }   nobreak;

        //optionals
        case 7: {
            // here is where we (optionally) read vertex and face attributes 
            if (mp_subop & TKSH_HAS_OPTIONALS) {
                if ((status = TK_Polyhedron::Read(tk)) != TK_Normal)
                    return status;
            }
            // and now any debug loggin we want
            if (tk.GetLogging()) {
                char            buffer[64];
                int                 index;

                if (tk.GetLoggingOptions (TK_Logging_Tagging) &&
                    (mp_subop & TKSH_FIRSTPASS) == 0) {
                    if ((status = tk.KeyToIndex(mp_key, index)) != TK_Normal)
                        return tk.Error();

                    sprintf (buffer, "[%d:%d]", index, m_lodlevel);
                    tk.LogEntry (buffer);
                }
            }
			m_stage++;
		}   nobreak;

		case 8: {
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


} //end function TK_Shell::Read





