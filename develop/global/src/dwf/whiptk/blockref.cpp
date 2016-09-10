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
// $Header: /NewRoot/DWF Toolkit/v6/develop/global/src/dwf/whiptk/blockref.cpp 1     9/12/04 8:51p Evansg $

#include "whiptk/pch.h"


static bool BLOCK_VARIABLE_RELATION[36][17] =
{
/*Fields                                  Graphic_hdr  Overlay_hdr Redline_Hdr thumbnail  preview   overlay_preview      font     graphics   overlay    redline      user       null     global_sheet   global  signature*/
/*File_Offset                         */ {    true,         true,      true,      true,     true,        true,           true,      true,      true,      true,      true,      true,      true,         true,      true },
/*Block_Size                          */ {    true,         true,      true,      true,     true,        true,           true,      true,      true,      true,      true,      true,      true,         true,      true },
/*Block_Guid                          */ {    true,         true,      true,      true,     true,        true,           true,      true,      true,      true,      true,      false,     true,         true,      true },
/*Creation_Time                       */ {    true,         true,      true,      true,     true,        true,           true,      true,      true,      true,      true,      false,     true,         true,      true },
/*Modification_Time                   */ {    true,         true,      true,      true,     true,        true,           true,      true,      true,      true,      true,      false,     true,         true,      true },
/*Encryption                          */ {    true,         true,      true,      true,     true,        true,           false,     false,     false,     false,     false,     false,     false,        false,     false},
/*Block_Validity_Flag                 */ {    true,         true,      true,      true,     true,        true,           true,      true,      true,      true,      true,      true,      true,         true,      true },
/*Block_Visibility_Flag               */ {    true,         true,      true,      false,    false,       false,          false,     false,     false,     false,     false,     true,      false,        false,     false},
/*Block_Meaning                       */ {    false,        true,      false,     false,    false,       false,          false,     false,     false,     false,     false,     false,     false,        false,     false},
/*Parent_Block_Guid                   */ {    false,        true,      true,      true,     true,        true,           false,     false,     false,     false,     true,      false,     false,        false,     true },
/*Related_Overlay_Header_Block_Guid   */ {    false,        false,     false,     false,    false,       true,           false,     false,     false,     false,     false,     false,     false,        false,     false},
/*Sheet_Print_Sequence                */ {    false,        false,     false,     false,    false,       false,          false,     false,     false,     false,     false,     false,     true,         false,     false},
/*Print_Sequence_Modification_Time    */ {    false,        false,     false,     false,    false,       false,          false,     false,     false,     false,     false,     false,     true,         false,     false},
/*Plans_And_Website_Guid              */ {    false,        false,     false,     false,    false,       false,          false,     false,     false,     false,     false,     false,     false,        true,      false},
/*Last_Sync_Time                      */ {    false,        false,     false,     false,    false,       false,          false,     false,     false,     false,     false,     false,     false,        true,      false},
/*Gettinig_Mini_Dwf_Flag              */ {    false,        false,     false,     false,    false,       false,          false,     false,     false,     false,     false,     false,     false,        true,      false},
/*Block_Modified_TimeStamp            */ {    false,        false,     false,     false,    false,       false,          false,     false,     false,     false,     false,     false,     false,        true,      false},
/*Dwf_Container_Id                    */ {    false,        false,     false,     false,    false,       false,          false,     false,     false,     false,     false,     false,     false,        true,      false},
/*Dwf_Container_Modification_Time     */ {    false,        false,     false,     false,    false,       false,          false,     false,     false,     false,     false,     false,     false,        true,      false},
/*Dwf_Discipline_Guid                 */ {    false,        false,     false,     false,    false,       false,          false,     false,     false,     false,     false,     false,     false,        true,      false},
/*Dwf_Discipline_Modification_Time    */ {    false,        false,     false,     false,    false,       false,          false,     false,     false,     false,     false,     false,     false,        true,      false},
/*ZValue                              */ {    true,         true,      true,      true,     true,        true,           false,     false,     false,     false,     false,     false,     false,        false,     false},
/*Scan_Flag                           */ {    true,         true,      true,      false,    false,       false,          false,     false,     false,     false,     false,     false,     false,        false,     false},
/*Mirror_Flag                         */ {    true,         true,      true,      false,    false,       false,          false,     false,     false,     false,     false,     false,     false,        false,     false},
/*Inversion_Flag                      */ {    true,         true,      true,      false,    false,       false,          false,     false,     false,     false,     false,     false,     false,        false,     false},
/*Paper_Scale                         */ {    true,         true,      true,      false,    false,       false,          false,     false,     false,     false,     false,     false,     false,        false,     false},
/*Orientation                         */ {    true,         true,      true,      false,    false,       false,          false,     false,     false,     false,     false,     false,     false,        false,     false},
/*Paper_Rotation                      */ {    true,         true,      true,      false,    false,       false,          false,     false,     false,     false,     false,     false,     false,        false,     false},
/*Alignment                           */ {    true,         true,      true,      false,    false,       false,          false,     false,     false,     false,     false,     false,     false,        false,     false},
/*Inked_Area                          */ {    true,         true,      true,      false,    false,       false,          false,     false,     false,     false,     false,     false,     false,        false,     false},
/*Dpi_Resolution                      */ {    true,         true,      true,      false,    false,       false,          false,     false,     false,     false,     false,     false,     false,        false,     false},
/*Paper_Offset                        */ {    true,         true,      true,      false,    false,       false,          false,     false,     false,     false,     false,     false,     false,        false,     false},
/*Clipping_Rectangle                  */ {    true,         true,      true,      false,    false,       false,          false,     false,     false,     false,     false,     false,     false,        false,     false},
/*Password                            */ {    false,        true,      true,      false,    false,       false,          false,     false,     false,     false,     false,     false,     false,        false,     false},
/*Image_Size_Representation           */ {    false,        false,     false,     true,     true,        true,           false,     false,     false,     false,     false,     false,     false,        false,     false},
/*Targeted_Matrix_Representation      */ {    false,        false,     false,     true,     true,        true,           false,     false,     false,     false,     false,     false,     false,        false,     false},
} ;


#define ADDARRAYBYTELENGTH(VARIABLETYPE, VARIABLENAME, ARRAYLENGTH)                         \
    if(Verify(VARIABLENAME, m_format)) opcode_size +=  ARRAYLENGTH * sizeof(VARIABLETYPE)   //

#define ADDBYTELENGTH(VARIABLETYPE, VARIABLENAME)                                           \
    if(Verify(VARIABLENAME, m_format)) opcode_size +=  sizeof(VARIABLETYPE)                 //

#define WRITEBINARY(VARIABLETYPE, VARIABLENAME)                                             \
    if(Verify(VARIABLENAME, ((WT_BlockRef *) this)->get_format()))                          \
            WD_CHECK (file.write((VARIABLETYPE) m_##VARIABLENAME));                         //

#define WRITEARRAYBINARY(VARIABLETYPE, VARIABLENAME, ARRAYLENGTH)                           \
    if(Verify(VARIABLENAME, ((WT_BlockRef *) this)->get_format()))        {                 \
        for(int zz=0; zz < ARRAYLENGTH; zz++)                            {                  \
            WD_CHECK (file.write((VARIABLETYPE) m_##VARIABLENAME[zz]));}}                   //

#define WRITEASCII(VARIABLETYPE, VARIABLENAME)                                              \
    if(Verify(VARIABLENAME, ((WT_BlockRef *) this)->get_format()))         {                \
        WD_CHECK (file.write_padded_ascii((VARIABLETYPE) m_##VARIABLENAME));                \
        WD_CHECK (file.write((WT_Byte) ' '));                              }                //

#define WRITEARRAYASCII(VARIABLETYPE, VARIABLENAME, ARRAYLENGTH)                            \
    if(Verify(VARIABLENAME, ((WT_BlockRef *) this)->get_format()))   {                      \
        for(int zz=0; zz < ARRAYLENGTH; zz++)                      {                        \
            WD_CHECK (file.write_padded_ascii((VARIABLETYPE) m_##VARIABLENAME[zz]));        \
            WD_CHECK (file.write((WT_Byte) ' '));                   }}                      //

#define SERIALIZE(VARIABLETYPE, VARIABLENAME)                                               \
    if(Verify(VARIABLENAME, ((WT_BlockRef *) this)->get_format()))  {                       \
        if (!file.heuristics().allow_binary_data())                                         \
            WD_CHECK (file.write((WT_Byte) ' '));                                           \
        WD_CHECK(m_##VARIABLENAME.serialize(file));                                         \
        if (!file.heuristics().allow_binary_data())                                         \
            WD_CHECK (file.write((WT_Byte) ' '));       }                                   //

#define SERIALIZEARRAY(VARIABLETYPE, VARIABLENAME, ARRAYLENGTH)                             \
    if(Verify(VARIABLENAME, ((WT_BlockRef *) this)->get_format()))          {               \
        for(int zz=0; zz < ARRAYLENGTH; zz++)                              {                \
            if (!file.heuristics().allow_binary_data())                                     \
                WD_CHECK (file.write((WT_Byte) ' '));                                       \
            WD_CHECK (m_##VARIABLENAME[ARRAYLENGTH].serialize(file));}                      \
            if (!file.heuristics().allow_binary_data())                                     \
                WD_CHECK (file.write((WT_Byte) ' '));                       }               //

#define READBINARY(VARIABLETYPE, VARIABLENAME)                                              \
    if(Verify(VARIABLENAME, ((WT_BlockRef *) this)->get_format()))                          \
            WD_CHECK (file.read((VARIABLETYPE) m_##VARIABLENAME));                          //

#define READARRAYBINARY(VARIABLETYPE, VARIABLENAME, ARRAYLENGTH)                            \
    if(Verify(VARIABLENAME, ((WT_BlockRef *) this)->get_format()))                          \
        for(zz=0; zz < ARRAYLENGTH; zz++)                                                   \
            WD_CHECK (file.read((VARIABLETYPE) m_##VARIABLENAME[zz]));                      //

#define READASCII(VARIABLETYPE, VARIABLENAME)                                               \
    if(Verify(VARIABLENAME, ((WT_BlockRef *) this)->get_format()))                          \
            WD_CHECK (file.read_ascii((VARIABLETYPE) m_##VARIABLENAME));                    //

#define READARRAYASCII(VARIABLETYPE, VARIABLENAME, ARRAYLENGTH)                             \
    if(Verify(VARIABLENAME, ((WT_BlockRef *) this)->get_format()))                          \
        for(zz=0; zz < ARRAYLENGTH; zz++)                                                   \
            WD_CHECK (file.read_ascii((VARIABLETYPE) m_##VARIABLENAME[zz]));                //

#define MATERIALIZE(VARIABLETYPE, VARIABLENAME)                                             \
        if(Verify(VARIABLENAME, ((WT_BlockRef *) this)->get_format()))      {               \
            m_optioncode = WD_Null;                                                         \
            m_optioncode = new WT_Opcode;                                                   \
            if (!m_optioncode)                                                              \
                return WT_Result::Out_Of_Memory_Error;                                      \
            WD_CHECK (m_optioncode->get_opcode(file) );                                     \
            WD_CHECK (m_##VARIABLENAME.materialize(*m_optioncode, file));                   \
            delete m_optioncode;                                                            \
            m_optioncode = WD_Null;                                         }               //

#define MATERIALIZEARRAY(VARIABLETYPE, VARIABLENAME, ARRAYLENGTH)                           \
    if(Verify(VARIABLENAME, ((WT_BlockRef *) this)->get_format()))              {           \
        for(zz=0; zz < ARRAYLENGTH; zz++)                                   {               \
            WD_CHECK(m_##VARIABLENAME[zz].materialize(m_optioncode, file));                 \
            m_optioncode = WD_Null;                                                         \
            m_optioncode = new WT_Opcode;                                                   \
            if (!m_optioncode)                                                              \
                return WT_Result::Out_Of_Memory_Error;                                      \
            WD_CHECK (m_optioncode->get_opcode(file) );                                     \
            WD_CHECK (m_##VARIABLENAME.materialize(*m_optioncode, file));                   \
            delete m_optioncode;                                                            \
            m_optioncode = WD_Null;                                         }   }           //

#define DEFINEBINARYREADCASE(TIHSSTAGE, VARIABLETYPE, VARIABLENAME)                         \
        m_stage = TIHSSTAGE;                                                                \
        case TIHSSTAGE:                                                                     \
            READBINARY(VARIABLETYPE, VARIABLENAME);                                         //

#define DEFINEBINARYARRAYREADCASE(TIHSSTAGE, VARIABLETYPE, VARIABLENAME, ARRAYLENGTH)       \
        m_stage = TIHSSTAGE;                                                                \
        case TIHSSTAGE:                                                                     \
            READARRAYBINARY(VARIABLETYPE, VARIABLENAME, ARRAYLENGTH)                        //

#define DEFINEMATERIALIZECASE(TIHSSTAGE, VARIABLETYPE, VARIABLENAME)                        \
        m_stage = TIHSSTAGE;                                                                \
        case TIHSSTAGE:                                                                     \
            MATERIALIZE(VARIABLETYPE, VARIABLENAME)                                         //

#define DEFINEASCIIREADCASE(TIHSSTAGE, VARIABLETYPE, VARIABLENAME)                          \
        m_stage = TIHSSTAGE;                                                                \
        case TIHSSTAGE:                                                                     \
            READASCII(VARIABLETYPE, VARIABLENAME);                                          //

#define DEFINEASCIIARRAYREADCASE(TIHSSTAGE, VARIABLETYPE, VARIABLENAME, ARRAYLENGTH)        \
        m_stage = TIHSSTAGE;                                                                \
        case TIHSSTAGE:                                                                     \
            READARRAYASCII(VARIABLETYPE, VARIABLENAME, ARRAYLENGTH)                         //

#define BINARYSIZEOFPASSWORD                                                                \
    (WT_Integer32) (sizeof(WT_Integer32) + sizeof(WT_Unsigned_Integer16) +                  \
    (32 * sizeof(WT_Byte)) + (2 * sizeof(WT_Byte)))                                         //

#define BINARYSIZEOFFILETIME                                                                \
    (WT_Integer32) (sizeof(WT_Integer32) + sizeof(WT_Unsigned_Integer16) +                  \
    sizeof(WT_Unsigned_Integer32) + sizeof(WT_Unsigned_Integer32) + (2 * sizeof(WT_Byte)))  //

#define OTHERBINARYSIZES                                                                    \
    (WT_Integer32) (sizeof(WT_Integer32) + sizeof(WT_Unsigned_Integer16) +                  \
    sizeof(WT_Unsigned_Integer16) + (2 * sizeof(WT_Byte)))                                  //

#define VERIFY(VARIABLENAME)                                                                \
    Verify(VARIABLENAME, ((WT_BlockRef *) this)->get_format())                              //

#define SERIALIZEBINARYBOOLVALUE(VARIABLENAME)                                              \
        if(Verify(VARIABLENAME, ((WT_BlockRef *) this)->get_format()))  {                   \
            if(m_##VARIABLENAME) WD_CHECK(file.write((WT_Byte) '1'));                       \
            else WD_CHECK(file.write((WT_Byte) '0'));                                       \
        }                                                                                   //

#define SERIALIZEBINARYDOUBLEARRAY(VARIABLENAME, ARRAYLENGTH)                               \
        if(Verify(VARIABLENAME, ((WT_BlockRef *) this)->get_format()))                  {   \
            for(zz=0; zz < ARRAYLENGTH; zz++)                                               \
            WD_CHECK(file.write(sizeof(double), (WT_Byte *)&m_##VARIABLENAME[zz]));     }   //

#define SERIALIZEBINARYDOUBLE(VARIABLENAME)                                                 \
        if(Verify(VARIABLENAME, ((WT_BlockRef *) this)->get_format()))              {       \
            WD_CHECK(file.write(sizeof(double), (WT_Byte *)&m_##VARIABLENAME));     }       //

#define SERIALIZEASCIIBOOLVALUE(VARIABLENAME)                                               \
        if(Verify(VARIABLENAME, ((WT_BlockRef *) this)->get_format()))  {                   \
            if(m_##VARIABLENAME) WD_CHECK(file.write((WT_Byte) '1'));                       \
            else WD_CHECK(file.write((WT_Byte) '0'));                                       \
            WD_CHECK (file.write((WT_Byte) ' '));                           }               //

#define SERIALIZEASCIIDOUBLEARRAY(VARIABLENAME, ARRAYLENGTH)                                \
        if(Verify(VARIABLENAME, ((WT_BlockRef *) this)->get_format()))    {                 \
            for(zz=0; zz < ARRAYLENGTH; zz++)                            {                  \
            WD_CHECK (file.write_padded_ascii(m_##VARIABLENAME[zz]));                       \
            WD_CHECK (file.write((WT_Byte) ' '));                            }}             //

#define DEFINEASCIIBOOLREADCASE(STAGE, VARIABLENAME)                                        \
        m_stage = STAGE;                                                                    \
        case STAGE:                                                                         \
        if(Verify(VARIABLENAME, ((WT_BlockRef *) this)->get_format()))  {                   \
            WD_CHECK(file.read_ascii(a_word));                                              \
            if(a_word == 1)         set_##VARIABLENAME(WD_True);                            \
            else if (a_word == 0)   set_##VARIABLENAME(WD_False);         }                 //


#define DEFINEASCIIDOUBLEREADCASE(STAGE, VARIABLENAME)                                      \
        m_stage = STAGE;                                                                    \
        case STAGE:                                                                         \
        if(Verify(VARIABLENAME, ((WT_BlockRef *) this)->get_format()))  {                   \
            WD_CHECK(file.read_ascii(m_##VARIABLENAME[0]));                                 \
            WT_Byte a_byte;                                                                 \
            WD_CHECK(file.read((WT_Byte&) a_byte));                                          \
            if (a_byte != ' ')                                                              \
                return WT_Result::Corrupt_File_Error;                                       \
            WD_CHECK(file.read_ascii(m_##VARIABLENAME[1]));                 }               //

#define DEFINEBINARYDOUBLDREADCASE(STAGE, VARIABLENAME)                                     \
        m_stage = STAGE;                                                                    \
        case STAGE:                                                                         \
        if(Verify(VARIABLENAME, ((WT_BlockRef *) this)->get_format()))          {           \
            WD_CHECK(file.read(sizeof(double), (WT_Byte *)&m_##VARIABLENAME[0]));           \
            WD_CHECK(file.read(sizeof(double), (WT_Byte *)&m_##VARIABLENAME[1])); }         //

#define DEFINEBINARYBOOLREADCASE(STAGE, VARIABLENAME)                                       \
        m_stage = STAGE;                                                                    \
        case STAGE:                                                                         \
        a_byte = '0';                                                                       \
        if(Verify(VARIABLENAME, ((WT_BlockRef *) this)->get_format()))  {                   \
            WD_CHECK(file.read(a_byte));                                                    \
            if(a_byte == '1')       set_##VARIABLENAME(WD_True);                            \
            else if (a_byte == '0') set_##VARIABLENAME(WD_False);         }                 //

#define COPYVARVALUE(COPYFROM, VARIABLENAME)                                                \
        if(Verify(VARIABLENAME, ((WT_BlockRef *) this)->get_format()))              {       \
        ((WT_BlockRef *) this)->set_##VARIABLENAME(((WT_BlockRef &)COPYFROM).get_##VARIABLENAME());  \
        }                                                      //

#define COPYARRAYVARVALUE(COPYFROM, VARIABLENAME, ARRAYLENGTH)                              \
        if(Verify(VARIABLENAME, ((WT_BlockRef *) this)->get_format()))              {       \
        for(int zz=0; zz<ARRAYLENGTH; zz++)                                                 \
            ((WT_BlockRef *) this)->get_##VARIABLENAME()[zz] = ((WT_BlockRef &)COPYFROM).get_##VARIABLENAME()[zz];   \
        }   //

#define COMPAREARRAYVARVALUE(COPYFROM, VARIABLENAME, ARRAYLENGTH)                           \
        if(Verify(VARIABLENAME, ((WT_BlockRef *) this)->get_format()))              {       \
        for(int zz=0; zz<ARRAYLENGTH; zz++)                                                 \
            if(((WT_BlockRef *) this)->get_##VARIABLENAME()[zz] != ((WT_BlockRef &)COPYFROM).get_##VARIABLENAME()[zz]) \
                return WD_False;   }   //

#define COMPAREVARVALUE(COPYFROM, VARIABLENAME)                                             \
    if(Verify(VARIABLENAME, ((WT_BlockRef *) this)->get_format()))              {           \
        if(((WT_BlockRef *) this)->get_##VARIABLENAME() != ((WT_BlockRef &)COPYFROM).get_##VARIABLENAME()) \
            return WD_False;                                                        }         //

///////////////////////////////////////////////////////////////////////////
WT_BlockRef::WT_BlockRef()
    :   m_format(Null),
        m_total_binary_opcode_size(0),
        m_index(0),
        m_file_offset(0),
        m_block_size(0),
        m_validity(WD_False),
        m_visibility(WD_False),
        m_sheet_print_sequence(0),
        m_flag_mini_dwf(WD_False),
        m_zValue(0),
        m_scan_flag(WD_False),
        m_mirror_flag(WD_False),
        m_inversion_flag(WD_False),
        m_paper_scale(0.0),
        m_rotation(0),
        m_dpi_resolution(0),
        m_stage(Starting)
{
    m_inked_area[0]             = 0.0;
    m_inked_area[1]             = 0.0;
    m_paper_offset[0]           = 0.0;
    m_paper_offset[1]           = 0.0;
    m_clip_rectangle[0]         = WT_Logical_Point(0, 0);
    m_clip_rectangle[1]         = WT_Logical_Point(0, 0);
    m_image_representation[0]   = 0;
    m_image_representation[1]   = 0;
    m_image_representation[2]   = 0;
    m_targeted_matrix_rep.set_identity();
    InitializeTotalBinaryOpcodeSize();
}

///////////////////////////////////////////////////////////////////////////
void    WT_BlockRef::set_format(WT_BlockRef_Format format) {
    m_format = format;
    ((WT_BlockRef *) this)->InitializeTotalBinaryOpcodeSize();
}

///////////////////////////////////////////////////////////////////////////
WT_BlockRef::WT_BlockRef(WT_BlockRef const & blockref)
    : WT_Item()
    , WT_Attribute()
{
    ((WT_BlockRef *) this)->set_format(blockref.get_format());
    m_file_offset = blockref.get_file_offset();
    m_block_size = blockref.get_block_size();
    COPYVARVALUE(blockref, block_guid);
    COPYVARVALUE(blockref, creation_time);
    COPYVARVALUE(blockref, modification_time);
    COPYVARVALUE(blockref, encryption);
    COPYVARVALUE(blockref, validity);
    COPYVARVALUE(blockref, visibility);
    COPYVARVALUE(blockref, block_meaning);
    COPYVARVALUE(blockref, parent_block_guid);
    COPYVARVALUE(blockref, related_overlay_hdr_block_guid);
    COPYVARVALUE(blockref, sheet_print_sequence);
    COPYVARVALUE(blockref, print_sequence_modified_time);
    COPYVARVALUE(blockref, plans_and_specs_website_guid);
    COPYVARVALUE(blockref, last_sync_time);
    COPYVARVALUE(blockref, flag_mini_dwf);
    COPYVARVALUE(blockref, modified_block_timestamp);
    COPYVARVALUE(blockref, dwf_container_guid);
    COPYVARVALUE(blockref, container_modified_time);
    COPYVARVALUE(blockref, dwf_discipline_guid);
    COPYVARVALUE(blockref, dwf_discipline_modified_time);
    COPYVARVALUE(blockref, zValue);
    COPYVARVALUE(blockref, scan_flag);
    COPYVARVALUE(blockref, mirror_flag);
    COPYVARVALUE(blockref, inversion_flag);
    COPYVARVALUE(blockref, paper_scale);
    COPYVARVALUE(blockref, orientation);
    COPYVARVALUE(blockref, rotation);
    COPYVARVALUE(blockref, alignment);
    COPYVARVALUE(blockref, inked_area);
    COPYVARVALUE(blockref, dpi_resolution);
    COPYVARVALUE(blockref, paper_offset);
    COPYVARVALUE(blockref, clip_rectangle);
    COPYVARVALUE(blockref, password);
    COPYVARVALUE(blockref, image_representation);
    m_targeted_matrix_rep.set(blockref.get_targeted_matrix_rep());
}

///////////////////////////////////////////////////////////////////////////
WT_BlockRef::WT_BlockRef(WT_BlockRef_Format format)
    :   m_format(format),
        m_total_binary_opcode_size(0),
        m_file_offset(0),
        m_block_size(0),
        m_validity(WD_False),
        m_visibility(WD_False),
        m_sheet_print_sequence(0),
        m_flag_mini_dwf(WD_False),
        m_zValue(0),
        m_scan_flag(WD_False),
        m_mirror_flag(WD_False),
        m_inversion_flag(WD_False),
        m_paper_scale(0.0),
        m_rotation(0),
        m_dpi_resolution(0),
        m_stage(Starting)
{
    m_inked_area[0]             = 0.0;
    m_inked_area[1]             = 0.0;
    m_paper_offset[0]           = 0.0;
    m_paper_offset[1]           = 0.0;
    m_clip_rectangle[0]         = WT_Logical_Point(0, 0);
    m_clip_rectangle[1]         = WT_Logical_Point(0, 0);
    m_image_representation[0]   = 0;
    m_image_representation[1]   = 0;
    m_image_representation[2]   = 0;
    m_targeted_matrix_rep.set_identity();
}

///////////////////////////////////////////////////////////////////////////
WT_Object::WT_ID    WT_BlockRef::object_id() const
{
    return WT_Object::BlockRef_ID;
}

///////////////////////////////////////////////////////////////////////////
WT_Object::WT_Type    WT_BlockRef::object_type() const
{
    return WT_Object::Attribute;
}

///////////////////////////////////////////////////////////////////////////
//Invoke this overloded function when you actually want to
//serialize blockref instance (not as part of directory and
//not for updating an existing blockref serialized instance
//with the updated block size information.
WT_Result WT_BlockRef::serialize(WT_File & file) const
{
    if (file.heuristics().target_version() >= REVISION_WHEN_PACKAGE_FORMAT_BEGINS)
        return WT_Result::Toolkit_Usage_Error;
    else
        return serialize(file, WD_False, WD_False);
}

//since m_total_binary_opcode_size is only used while serializing
//blockrefs as part of directory, we always take in to account the
//size of file_offset for computing the m_total_binary_opcode_size value.
void WT_BlockRef::InitializeTotalBinaryOpcodeSize()
{
    ((WT_BlockRef *) this)->m_total_binary_opcode_size =
        ComputeTotalBinaryOpcodeSize(WD_True);
}

///////////////////////////////////////////////////////////////////////////
WT_Integer32 WT_BlockRef::ComputeTotalBinaryOpcodeSize(WT_Boolean bAsPartOfList)
{
    WT_Integer32 opcode_size = ComputeBinaryOpcodeSize(get_format(), bAsPartOfList);
    return (opcode_size + sizeof(WT_Integer32) + sizeof(WT_Byte));
}

///////////////////////////////////////////////////////////////////////////
//Invoking this method with bAsPartOfList set to true will include the file_offset size.
//This usually will be the case when computing the blockref opcode size when part of
//the directory instance. For blockref (non directory instances) we ignore file offset size
//as we don't want to edit in two places (one in directory instance and the other in
//the blockref instance) if the blockref file offset gets edited/adjusted.
//With this approach we will only have to edit the directory instance of the serialized
//blockref to update information pertaining to a particular block.
WT_Integer32 WT_BlockRef::ComputeBinaryOpcodeSize(WT_BlockRef_Format, WT_Boolean bAsPartOfList)
{
    WT_Integer32 opcode_size = sizeof(WT_Unsigned_Integer16);
    if(bAsPartOfList) {
        ADDBYTELENGTH       (WT_Unsigned_Integer32         ,file_offset);
    }
    ADDBYTELENGTH       (WT_Unsigned_Integer32         ,block_size);
    ADDBYTELENGTH       (WT_Byte                       ,validity);
    ADDBYTELENGTH       (WT_Byte                       ,visibility);
    ADDBYTELENGTH       (WT_Integer32                  ,sheet_print_sequence);
    ADDBYTELENGTH       (WT_Byte                       ,flag_mini_dwf);
    ADDBYTELENGTH       (WT_Integer32                  ,zValue);
    ADDBYTELENGTH       (WT_Byte                       ,scan_flag);
    ADDBYTELENGTH       (WT_Byte                       ,mirror_flag);
    ADDBYTELENGTH       (WT_Byte                       ,inversion_flag);
    ADDBYTELENGTH       (double                        ,paper_scale);
    ADDBYTELENGTH       (WT_Integer16                  ,rotation);
    ADDARRAYBYTELENGTH  (double                        ,inked_area, 2);
    ADDBYTELENGTH       (WT_Integer16                  ,dpi_resolution);
    ADDARRAYBYTELENGTH  (double                        ,paper_offset, 2);
    ADDARRAYBYTELENGTH  (WT_Logical_Point              ,clip_rectangle, 2);
    ADDARRAYBYTELENGTH  (WT_Integer32                  ,image_representation, 3);

    if(VERIFY(block_guid))                      opcode_size += BINARYSIZEOFGUID;            //for block guid
    if(VERIFY(block_meaning))                   opcode_size += OTHERBINARYSIZES;            //for block meaning
    if(VERIFY(creation_time))                   opcode_size += BINARYSIZEOFFILETIME;        //for creation time
    if(VERIFY(modification_time))               opcode_size += BINARYSIZEOFFILETIME;        //for modification time
    if(VERIFY(parent_block_guid))               opcode_size += BINARYSIZEOFGUID;            //for parent block guid
    if(VERIFY(related_overlay_hdr_block_guid))  opcode_size += BINARYSIZEOFGUID;            //for related overlay_hdr block guid
    if(VERIFY(print_sequence_modified_time))    opcode_size += BINARYSIZEOFFILETIME;        //for print sequence modification time.
    if(VERIFY(plans_and_specs_website_guid))    opcode_size += BINARYSIZEOFGUID;            //for plans and specs website guid
    if(VERIFY(last_sync_time))                  opcode_size += BINARYSIZEOFFILETIME;        //for last sync at plans and specs site time.
    if(VERIFY(modified_block_timestamp))        opcode_size += BINARYSIZEOFFILETIME;        //for modified block timestamp.
    if(VERIFY(dwf_container_guid))              opcode_size += BINARYSIZEOFGUID;            //for dwf container block guid
    if(VERIFY(container_modified_time))         opcode_size += BINARYSIZEOFFILETIME;        //for container modification timestamp.
    if(VERIFY(dwf_discipline_guid))             opcode_size += BINARYSIZEOFGUID;            //for dwf discipline guid
    if(VERIFY(dwf_discipline_modified_time))    opcode_size += BINARYSIZEOFFILETIME;        //for dwf discipline modification timestamp.
    if(VERIFY(encryption))                      opcode_size += OTHERBINARYSIZES;            //for encryption
    if(VERIFY(orientation))                     opcode_size += OTHERBINARYSIZES;            //for orientation
    if(VERIFY(alignment))                       opcode_size += OTHERBINARYSIZES;            //for alignment
    if(VERIFY(password))                        opcode_size += BINARYSIZEOFPASSWORD;        //for password
    if(VERIFY(targeted_matrix_rep))             opcode_size += (16 * sizeof(double));       //for target matrix representation

    opcode_size += sizeof(WT_Byte);                 // The closing "}"

    return opcode_size;
}

///////////////////////////////////////////////////////////////////////////
//For regular blockref serialization instances, invoke with the later two
//parameters as WD_False. For directory blockref instance serialization,
//invoke with bAsPartOfList flag as WD_True and bForBlockSizeRewrite as
//WD_False. For updating a blockref instance, invoke with
//bForBlockSizeRewrite as WD_True and  bAsPartOfList as WD_False.
WT_Result WT_BlockRef::serialize(WT_File & file, WT_Boolean bAsPartOfList, WT_Boolean bForBlockSizeRewrite) const
{
    if (file.heuristics().target_version() >= REVISION_WHEN_PACKAGE_FORMAT_BEGINS)
        return WT_Result::Toolkit_Usage_Error;

    int zz = 0;

    WD_CHECK (file.dump_delayed_drawable());

    WT_Boolean compressor_stop_flag = WD_False;
    //by turning on the file heuristics to not allow compression
    //we will be writing this opcode in uncompressed form always

    compressor_stop_flag = file.heuristics().allow_data_compression();
    file.heuristics().set_allow_data_compression(WD_False);

    if (file.heuristics().allow_binary_data())
    {
        WT_Integer32 opcode_size; //for opcode

        opcode_size = ((WT_BlockRef *) this)->ComputeBinaryOpcodeSize(
            ((WT_BlockRef *) this)->get_format(), bAsPartOfList);

        if(!bAsPartOfList) {
            ((WT_BlockRef *) this)->InitializeTotalBinaryOpcodeSize();
        }

        WD_CHECK (file.write((WT_Byte) '{'));
        if(!bAsPartOfList && !bForBlockSizeRewrite) {
            WD_CHECK((file.stream_tell_action())(file, (unsigned long *)&m_file_offset));
             ((WT_BlockRef *) this)->m_file_offset -= sizeof(WT_Byte);
            WD_CHECK( file.set_block_size_for_tail_blockref(
                ((WT_BlockRef *) this)->m_file_offset) );
        }
        WD_CHECK (file.write((WT_Integer32) opcode_size));
        WD_CHECK (file.write((WT_Unsigned_Integer16) ((WT_BlockRef *) this)->get_format()));

        if(bAsPartOfList) {
            WRITEBINARY                 (WT_Unsigned_Integer32   ,file_offset)
        }
        WRITEBINARY                 (WT_Unsigned_Integer32   ,block_size)
        SERIALIZE                   (WT_Guid                 ,block_guid)
        SERIALIZE                   (WT_FileTime             ,creation_time)
        SERIALIZE                   (WT_FileTime             ,modification_time)
        SERIALIZE                   (WT_Encryption           ,encryption)
        SERIALIZEBINARYBOOLVALUE    (validity)
        SERIALIZEBINARYBOOLVALUE    (visibility)
        SERIALIZE                   (WT_Block_Meaning        ,block_meaning)
        SERIALIZE                   (WT_Guid                 ,parent_block_guid)
        SERIALIZE                   (WT_Guid                 ,related_overlay_hdr_block_guid)
        WRITEBINARY                 (WT_Integer32            ,sheet_print_sequence)
        SERIALIZE                   (WT_FileTime             ,print_sequence_modified_time)
        SERIALIZE                   (WT_Guid                 ,plans_and_specs_website_guid)
        SERIALIZE                   (WT_FileTime             ,last_sync_time)
        SERIALIZEBINARYBOOLVALUE    (flag_mini_dwf)
        SERIALIZE                   (WT_FileTime             ,modified_block_timestamp)
        SERIALIZE                   (WT_Guid                 ,dwf_container_guid)
        SERIALIZE                   (WT_FileTime             ,container_modified_time)
        SERIALIZE                   (WT_Guid                 ,dwf_discipline_guid)
        SERIALIZE                   (WT_FileTime             ,dwf_discipline_modified_time)
        WRITEBINARY                 (WT_Integer32            ,zValue)
        SERIALIZEBINARYBOOLVALUE    (scan_flag)
        SERIALIZEBINARYBOOLVALUE    (mirror_flag)
        SERIALIZEBINARYBOOLVALUE    (inversion_flag)
        SERIALIZEBINARYDOUBLE       (paper_scale)
        SERIALIZE                   (WT_Orientation          ,orientation)
        WRITEBINARY                 (WT_Integer16            ,rotation)
        SERIALIZE                   (WT_Alignment            ,alignment)
        SERIALIZEBINARYDOUBLEARRAY  (inked_area              ,2)
        WRITEBINARY                 (WT_Integer16            ,dpi_resolution);
        SERIALIZEBINARYDOUBLEARRAY  (paper_offset            ,2)

        if(Verify(clip_rectangle, ((WT_BlockRef *) this)->get_format()))
            WD_CHECK(file.write(2, m_clip_rectangle));

        SERIALIZE                   (WT_Password             ,password);
        WRITEARRAYBINARY            (WT_Integer32            ,image_representation, 3);

        if(Verify(targeted_matrix_rep, ((WT_BlockRef *) this)->get_format()))  {
            WD_CHECK(file.write(16 * sizeof(double), (WT_Byte *) m_targeted_matrix_rep.elements()));
        }

        WD_CHECK (file.write((WT_Byte) '}'));
    }
    else
    {
        // Extended ASCII output
        WD_CHECK (file.write_tab_level());

        if(!bAsPartOfList && !bForBlockSizeRewrite) {
            WD_CHECK((file.stream_tell_action())(file, (unsigned long *)&m_file_offset));
            ((WT_BlockRef *) this)->m_file_offset -=
                ((file.tab_level() + (int)strlen(WD_NEWLINE) ) * sizeof(WT_Byte));
            WD_CHECK( file.set_block_size_for_tail_blockref(
                ((WT_BlockRef *) this)->m_file_offset) );
        }

        WD_CHECK (file.write("(BlockRef "));

        switch (((WT_BlockRef *) this)->get_format())
        {
        case Graphics_Hdr:
                                WD_CHECK (file.write_quoted_string("Graphics_Hdr", WD_True));
                                break;
        case Overlay_Hdr:
                                WD_CHECK (file.write_quoted_string("Overlay_Hdr", WD_True));
                                break;
        case Redline_Hdr:
                                WD_CHECK (file.write_quoted_string("Redline_Hdr", WD_True));
                                break;
        case Thumbnail:
                                WD_CHECK (file.write_quoted_string("Thumbnail", WD_True));
                                break;
        case Preview:
                                WD_CHECK (file.write_quoted_string("Preview", WD_True));
                                break;
        case Overlay_Preview:
                                WD_CHECK (file.write_quoted_string("Overlay_Preview", WD_True));
                                break;
        case EmbedFont:
                                WD_CHECK (file.write_quoted_string("EmbedFont", WD_True));
                                break;
        case Graphics:
                                WD_CHECK (file.write_quoted_string("Graphics", WD_True));
                                break;
        case Overlay:
                                WD_CHECK (file.write_quoted_string("Overlay", WD_True));
                                break;
        case Redline:
                                WD_CHECK (file.write_quoted_string("Redline", WD_True));
                                break;
        case User:
                                WD_CHECK (file.write_quoted_string("User", WD_True));
                                break;
        case Null:
                                WD_CHECK (file.write_quoted_string("Null", WD_True));
                                break;
        case Global_Sheet:
                                WD_CHECK (file.write_quoted_string("Global_Sheet", WD_True));
                                break;
        case Global:
                                WD_CHECK (file.write_quoted_string("Global", WD_True));
                                break;
        case Signature:
                                WD_CHECK (file.write_quoted_string("Signature", WD_True));
                                break;
        default:
            return WT_Result::Internal_Error;
        }
        WD_CHECK                    (file.write((WT_Byte) ' '));
        if(bAsPartOfList)  {
            WRITEASCII                  (WT_Unsigned_Integer32   ,file_offset)
        }
        WRITEASCII                  (WT_Unsigned_Integer32   ,block_size)
        SERIALIZE                   (WT_Guid                 ,block_guid)
        SERIALIZE                   (WT_FileTime             ,creation_time)
        SERIALIZE                   (WT_FileTime             ,modification_time)
        SERIALIZE                   (WT_Encryption           ,encryption)
        SERIALIZEASCIIBOOLVALUE     (validity)
        SERIALIZEASCIIBOOLVALUE     (visibility)
        SERIALIZE                   (WT_Block_Meaning        ,block_meaning)
        SERIALIZE                   (WT_Guid                 ,parent_block_guid)
        SERIALIZE                   (WT_Guid                 ,related_overlay_hdr_block_guid)
        WRITEASCII                  (WT_Integer32            ,sheet_print_sequence)
        SERIALIZE                   (WT_FileTime             ,print_sequence_modified_time)
        SERIALIZE                   (WT_Guid                 ,plans_and_specs_website_guid)
        SERIALIZE                   (WT_FileTime             ,last_sync_time)
        SERIALIZEASCIIBOOLVALUE     (flag_mini_dwf)
        SERIALIZE                   (WT_FileTime             ,modified_block_timestamp)
        SERIALIZE                   (WT_Guid                 ,dwf_container_guid)
        SERIALIZE                   (WT_FileTime             ,container_modified_time)
        SERIALIZE                   (WT_Guid                 ,dwf_discipline_guid)
        SERIALIZE                   (WT_FileTime             ,dwf_discipline_modified_time)
        WRITEASCII                  (WT_Integer32            ,zValue)
        SERIALIZEASCIIBOOLVALUE     (scan_flag)
        SERIALIZEASCIIBOOLVALUE     (mirror_flag)
        SERIALIZEASCIIBOOLVALUE     (inversion_flag)
        if(Verify(paper_scale, ((WT_BlockRef *) this)->get_format()))  {
            WD_CHECK(file.write_padded_ascii(m_paper_scale));
        }
        SERIALIZE                   (WT_Orientation          ,orientation)
        WRITEASCII                  (WT_Integer16            ,rotation)
        SERIALIZE                   (WT_Alignment            ,alignment)
        SERIALIZEASCIIDOUBLEARRAY   (inked_area, 2)

        WRITEASCII                  (WT_Integer16            ,dpi_resolution);
        SERIALIZEASCIIDOUBLEARRAY   (paper_offset, 2)

        if(Verify(clip_rectangle, ((WT_BlockRef *) this)->get_format()))  {
            WD_CHECK(file.write_padded_ascii(m_clip_rectangle[0]));
            WD_CHECK(file.write((WT_Byte) ' '));
            WD_CHECK(file.write_padded_ascii(m_clip_rectangle[1]));
            WD_CHECK (file.write((WT_Byte) ' '));
        }

        SERIALIZE                   (WT_Password             ,password);
        WRITEARRAYASCII             (WT_Integer32            ,image_representation, 3);
        if(Verify(targeted_matrix_rep, ((WT_BlockRef *) this)->get_format()))
        {
            if (!file.heuristics().allow_binary_data())
                WD_CHECK (file.write((WT_Byte) ' '));
            WT_Matrix_IO oMatrix( m_targeted_matrix_rep );
            WD_CHECK(oMatrix.serialize_padded(file));
            if (!file.heuristics().allow_binary_data())
                WD_CHECK (file.write((WT_Byte) ' '));
        }

        WD_CHECK (file.write(")"));
    }

    file.heuristics().set_allow_data_compression(compressor_stop_flag);
    if(!bAsPartOfList && !bForBlockSizeRewrite) {
        WD_CHECK(file.add_blockref((WT_BlockRef &) *this));
    }
    return WT_Result::Success;
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_BlockRef::materialize(WT_Opcode const & opcode, WT_File & file)
{
    return materialize(opcode, file, WD_False);
}

///////////////////////////////////////////////////////////////////////////
//There could be two places were we could possibly materialize a blockref.
//One in the regular blockref instance and another time as part of a directory.
//Set bAsPartOfList to WD_True when materializing from directory.
WT_Result WT_BlockRef::materialize(WT_Opcode const & opcode, WT_File & file, WT_Boolean bAsPartOfList)
{
    WT_Byte a_byte = '0';
    WT_Unsigned_Integer16 a_word = 0;
    int zz = 0;
    if (opcode.type() == WT_Opcode::Extended_Binary)
    {
        switch (m_stage)
        {
        case Starting:
        if(bAsPartOfList) {
            DEFINEBINARYREADCASE        (Getting_File_Offset                        ,WT_Unsigned_Integer32&    ,file_offset)
        }
        DEFINEBINARYREADCASE        (Getting_Block_Size                         ,WT_Unsigned_Integer32&    ,block_size)
        DEFINEMATERIALIZECASE       (Getting_Block_Guid                         ,WT_Guid                   ,block_guid)
        DEFINEMATERIALIZECASE       (Getting_Creation_Time                      ,WT_FileTime               ,creation_time)
        DEFINEMATERIALIZECASE       (Getting_Modification_Time                  ,WT_FileTime               ,modification_time)
        DEFINEMATERIALIZECASE       (Getting_Encryption                         ,WT_Encryption             ,encryption)
        DEFINEBINARYBOOLREADCASE    (Getting_Block_Validity_Flag                ,validity)
        DEFINEBINARYBOOLREADCASE    (Getting_Block_Visibility_Flag              ,visibility)
        DEFINEMATERIALIZECASE       (Getting_Block_Meaning                      ,WT_Block_Meaning          ,block_meaning)
        DEFINEMATERIALIZECASE       (Getting_Parent_Block_Guid                  ,WT_Guid                   ,parent_block_guid)
        DEFINEMATERIALIZECASE       (Getting_Related_Overlay_Hdr_Block_Guid     ,WT_Guid                   ,related_overlay_hdr_block_guid)
        DEFINEBINARYREADCASE        (Getting_Sheet_Print_Sequence               ,WT_Integer32&             ,sheet_print_sequence)
        DEFINEMATERIALIZECASE       (Getting_Print_Sequence_Modification_Time   ,WT_FileTime               ,print_sequence_modified_time)
        DEFINEMATERIALIZECASE       (Getting_Plans_And_Website_Guid             ,WT_Guid                   ,plans_and_specs_website_guid)
        DEFINEMATERIALIZECASE       (Getting_Last_Sync_Time                     ,WT_FileTime               ,last_sync_time)
        DEFINEBINARYBOOLREADCASE    (Gettinig_Mini_Dwf_Flag                     ,flag_mini_dwf)
        DEFINEMATERIALIZECASE       (Getting_Block_Modified_TimeStamp           ,WT_FileTime               ,modified_block_timestamp)
        DEFINEMATERIALIZECASE       (Getting_Dwf_Container_Id                   ,WT_Guid                   ,dwf_container_guid)
        DEFINEMATERIALIZECASE       (Getting_Dwf_Container_Modification_Time    ,WT_FileTime               ,container_modified_time)
        DEFINEMATERIALIZECASE       (Getting_Dwf_Discipline_Guid                ,WT_Guid                   ,dwf_discipline_guid)
        DEFINEMATERIALIZECASE       (Getting_Dwf_Discipline_Modification_Time   ,WT_FileTime               ,dwf_discipline_modified_time)
        DEFINEBINARYREADCASE        (Getting_ZValue                             ,WT_Integer32&             ,zValue)
        DEFINEBINARYBOOLREADCASE    (Getting_Scan_Flag                          ,scan_flag)
        DEFINEBINARYBOOLREADCASE    (Getting_Mirror_Flag                        ,mirror_flag)
        DEFINEBINARYBOOLREADCASE    (Getting_Inversion_Flag                     ,inversion_flag)
        if(Verify(paper_scale, ((WT_BlockRef *) this)->get_format())) {
            WD_CHECK(file.read(sizeof(double), (WT_Byte *) &m_paper_scale));
        }
        DEFINEMATERIALIZECASE       (Getting_Orientation                        ,WT_Orientation            ,orientation)
        DEFINEBINARYREADCASE        (Getting_Paper_Rotation                     ,WT_Integer16&             ,rotation)
        DEFINEMATERIALIZECASE       (Getting_Alignment                          ,WT_Alignment              ,alignment)
        DEFINEBINARYDOUBLDREADCASE  (Getting_Inked_Area                         ,inked_area)
        DEFINEBINARYREADCASE        (Getting_Dpi_Resolution                     ,WT_Integer16&             ,dpi_resolution);
        DEFINEBINARYDOUBLDREADCASE  (Getting_Paper_Offset                       ,paper_offset)

        m_stage = Getting_Clipping_Rectangle;
        case Getting_Clipping_Rectangle:
        if(Verify(clip_rectangle, ((WT_BlockRef *) this)->get_format()))
            WD_CHECK(file.read(2, (WT_Logical_Point *)&m_clip_rectangle));

        DEFINEMATERIALIZECASE       (Getting_Password                           , WT_Password              ,password)
        DEFINEBINARYARRAYREADCASE   (Getting_Image_Size_Representation          , WT_Integer32&            ,image_representation, 3)

        case Getting_Targeted_Matrix_Representation:
            if(Verify(targeted_matrix_rep, ((WT_BlockRef *) this)->get_format())) {
                double matrix_elements[16];
                WD_CHECK(file.read(16 * sizeof(double), (WT_Byte *) &matrix_elements[0]));
                m_targeted_matrix_rep.set((double const *) &matrix_elements[0]);
            }

        m_stage = Getting_Close;

        // No break
        case Getting_Close:
            WT_Byte close_brace;

            WD_CHECK (file.read(close_brace));
            if (close_brace != '}')
                return WT_Result::Corrupt_File_Error;

            // Yeah!! We succeeded!
            break;  // Go below to clean up...
        default:
            return WT_Result::Internal_Error;  // Illegal stage.
        } // switch (m_stage) for Binary

    }
    else if (opcode.type() == WT_Opcode::Extended_ASCII) {
        switch (m_stage)
        {
        case Starting:
            m_stage = Getting_Format;
            // No break;

        case Getting_Format:

            char *  format_name;

            WD_CHECK (file.read(format_name, 40));

            if (!strcmp(format_name, "Graphics_Hdr"))
                m_format = Graphics_Hdr;
            else if (!strcmp(format_name, "Overlay_Hdr"))
                m_format = Overlay_Hdr;
            else if (!strcmp(format_name, "Redline_Hdr"))
                m_format = Redline_Hdr;
            else if (!strcmp(format_name, "Thumbnail"))
                m_format = Thumbnail;
            else if (!strcmp(format_name, "Preview"))
                m_format = Preview;
            else if (!strcmp(format_name, "Overlay_Preview"))
                m_format = Overlay_Preview;
            else if (!strcmp(format_name, "EmbedFont"))
                m_format = EmbedFont;
            else if (!strcmp(format_name, "Graphics"))
                m_format = Graphics;
            else if (!strcmp(format_name, "Overlay"))
                m_format = Overlay;
            else if (!strcmp(format_name, "Redline"))
                m_format = Redline;
            else if (!strcmp(format_name, "User"))
                m_format = User;
            else if (!strcmp(format_name, "Null"))
                m_format = Null;
            else if (!strcmp(format_name, "Global_Sheet"))
                m_format = Global_Sheet;
            else if (!strcmp(format_name, "Global"))
                m_format = Global;
            else if (!strcmp(format_name, "Signature"))
                m_format = Signature;
            else
            {
                delete []format_name;
                WD_CHECK (opcode.skip_past_matching_paren(file));
                return          WT_Result::Success; //we return success because we
                                                    //want to be able to introduce
                                                    //future blockref formats with
                                                    //the same WT_BlockRef class.
            }

            delete []format_name;

        if(bAsPartOfList) {
            DEFINEASCIIREADCASE         (Getting_File_Offset                       ,WT_Unsigned_Integer32&,file_offset)
        }
        DEFINEASCIIREADCASE         (Getting_Block_Size                        ,WT_Unsigned_Integer32& ,block_size)
        DEFINEMATERIALIZECASE       (Getting_Block_Guid                        ,WT_Guid               ,block_guid)
        DEFINEMATERIALIZECASE       (Getting_Creation_Time                     ,WT_FileTime           ,creation_time)
        DEFINEMATERIALIZECASE       (Getting_Modification_Time                 ,WT_FileTime           ,modification_time)
        DEFINEMATERIALIZECASE       (Getting_Encryption                        ,WT_Encryption         ,encryption)
        DEFINEASCIIBOOLREADCASE     (Getting_Block_Validity_Flag               ,validity)
        DEFINEASCIIBOOLREADCASE     (Getting_Block_Visibility_Flag             ,visibility)
        DEFINEMATERIALIZECASE       (Getting_Block_Meaning                     ,WT_Block_Meaning      ,block_meaning)
        DEFINEMATERIALIZECASE       (Getting_Parent_Block_Guid                 ,WT_Guid               ,parent_block_guid)
        DEFINEMATERIALIZECASE       (Getting_Related_Overlay_Hdr_Block_Guid    ,WT_Guid               ,related_overlay_hdr_block_guid)
        DEFINEASCIIREADCASE         (Getting_Sheet_Print_Sequence              ,WT_Integer32&         ,sheet_print_sequence)
        DEFINEMATERIALIZECASE       (Getting_Print_Sequence_Modification_Time  ,WT_FileTime           ,print_sequence_modified_time)
        DEFINEMATERIALIZECASE       (Getting_Plans_And_Website_Guid            ,WT_Guid               ,plans_and_specs_website_guid)
        DEFINEMATERIALIZECASE       (Getting_Last_Sync_Time                    ,WT_FileTime           ,last_sync_time)
        DEFINEASCIIBOOLREADCASE     (Gettinig_Mini_Dwf_Flag                    ,flag_mini_dwf)
        DEFINEMATERIALIZECASE       (Getting_Block_Modified_TimeStamp          ,WT_FileTime           ,modified_block_timestamp)
        DEFINEMATERIALIZECASE       (Getting_Dwf_Container_Id                  ,WT_Guid               ,dwf_container_guid)
        DEFINEMATERIALIZECASE       (Getting_Dwf_Container_Modification_Time   ,WT_FileTime           ,container_modified_time)
        DEFINEMATERIALIZECASE       (Getting_Dwf_Discipline_Guid               ,WT_Guid               ,dwf_discipline_guid)
        DEFINEMATERIALIZECASE       (Getting_Dwf_Discipline_Modification_Time  ,WT_FileTime           ,dwf_discipline_modified_time)
        DEFINEASCIIREADCASE         (Getting_ZValue                            ,WT_Integer32&         ,zValue)
        DEFINEASCIIBOOLREADCASE     (Getting_Scan_Flag                         ,scan_flag)
        DEFINEASCIIBOOLREADCASE     (Getting_Mirror_Flag                       ,mirror_flag)
        DEFINEASCIIBOOLREADCASE     (Getting_Inversion_Flag                    ,inversion_flag)
        if(Verify(paper_scale, ((WT_BlockRef *) this)->get_format()))  {
            WD_CHECK(file.read_ascii(m_paper_scale));
        }
        DEFINEMATERIALIZECASE       (Getting_Orientation                       ,WT_Orientation        ,orientation)
        DEFINEASCIIREADCASE         (Getting_Paper_Rotation                    ,WT_Integer16&         ,rotation)
        DEFINEMATERIALIZECASE       (Getting_Alignment                         ,WT_Alignment          ,alignment)
        DEFINEASCIIDOUBLEREADCASE   (Getting_Inked_Area                        ,inked_area)
        DEFINEASCIIREADCASE         (Getting_Dpi_Resolution                    ,WT_Integer16&         ,dpi_resolution);
        DEFINEASCIIDOUBLEREADCASE   (Getting_Paper_Offset                      ,paper_offset)

        m_stage = Getting_Clipping_Rectangle;
        case Getting_Clipping_Rectangle:
        if(Verify(clip_rectangle, ((WT_BlockRef *) this)->get_format()))  {
            WD_CHECK(file.read_ascii(m_clip_rectangle[0]));
            WT_Byte a_byte;
            WD_CHECK(file.read((WT_Byte&) a_byte));
            if (a_byte != ' ')
                return WT_Result::Corrupt_File_Error;
            WD_CHECK(file.read_ascii(m_clip_rectangle[1]));
        }

        DEFINEMATERIALIZECASE       (Getting_Password                               , WT_Password               ,password)
        DEFINEASCIIARRAYREADCASE    (Getting_Image_Size_Representation              , WT_Integer32&             ,image_representation, 3)

        case Getting_Targeted_Matrix_Representation:
            if(Verify(targeted_matrix_rep, ((WT_BlockRef *) this)->get_format()))
            {
                WT_Matrix_IO oMatrix;
                WD_CHECK(oMatrix.materialize(file));
                m_targeted_matrix_rep = oMatrix;
            }

        m_stage = Getting_Close;

        // No break
        case Getting_Close:
            WD_CHECK (opcode.skip_past_matching_paren(file));

            // Yeah!! We succeeded!
            break;  // Go below to clean up...
        default:
            return WT_Result::Internal_Error;  // Illegal stage.
        } // switch (m_stage) for ascii
    }
    else {
        return WT_Result::Internal_Error;  // Illegal stage.
    }

    m_stage         = Starting;  // We finished, so reset stage to start.
    m_materialized  = WD_True;

    //we neither want to add the blockref to the directory not
    //change the rendition attribute when blockrefs materializes
    //as part of the directory.
    if(!bAsPartOfList) {
        WD_CHECK(file.add_blockref((WT_BlockRef &) *this));
        file.desired_rendition().blockref() = ((WT_BlockRef &) *this);
    }

    return          WT_Result::Success;
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_BlockRef::skip_operand(WT_Opcode const & opcode, WT_File & file)
{
    // TODO: implement a "read_block" method that performs the block reading for
    // both skipping and materialization, in one spot.  In the meantime, we use
    // materialize, and ignore the results, due to the huge amount of different
    // block types, to assure that processing is done correctly, and that code
    // is not duplicated
    return      materialize(opcode, file, WD_True);
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_BlockRef::process(WT_File & file)
{
    WD_Assert   (file.blockref_action());
    return      (file.blockref_action())(*this, file);
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_BlockRef::default_process(WT_BlockRef & item, WT_File & file)
{
    file.rendition().blockref() = item;
    return WT_Result::Success;
}


///////////////////////////////////////////////////////////////////////////
#if DESIRED_CODE(WHIP_OUTPUT)
WT_Result WT_BlockRef::sync(WT_File & file) const
{
    WD_Assert( (file.file_mode() == WT_File::File_Write)   ||
               (file.file_mode() == WT_File::Block_Append) ||
               (file.file_mode() == WT_File::Block_Write));

    if ((*this) != file.rendition().blockref() )
    {
      file.rendition().blockref() = *this;
        return serialize(file);
    }
    return WT_Result::Success;
}
#else
WT_Result WT_BlockRef::sync(WT_File &) const
{
    return WT_Result::Success;
}
#endif  // DESIRED_CODE()

///////////////////////////////////////////////////////////////////////////
WT_Boolean    WT_BlockRef::operator== (WT_Attribute const & attrib) const
{
    return ((*this) == (WT_BlockRef &) attrib);
}

///////////////////////////////////////////////////////////////////////////
WT_Boolean    WT_BlockRef::operator== (WT_BlockRef const & attrib) const
{
    if(attrib.object_id() != WT_Object::BlockRef_ID)
        return WD_False;

    if(((WT_BlockRef *) this)->get_format() != ((WT_BlockRef &)attrib).get_format())
        return WD_False;

    //we leave out block_size and file_offset fields while checking
    //for equality because, those anyway gets reset during blockref
    //serialization.
    COMPAREVARVALUE(attrib, block_guid);
    COMPAREVARVALUE(attrib, creation_time);
    COMPAREVARVALUE(attrib, modification_time);
    COMPAREVARVALUE(attrib, encryption);
    COMPAREVARVALUE(attrib, validity);
    COMPAREVARVALUE(attrib, visibility);
    COMPAREVARVALUE(attrib, block_meaning);
    COMPAREVARVALUE(attrib, parent_block_guid);
    COMPAREVARVALUE(attrib, related_overlay_hdr_block_guid);
    COMPAREVARVALUE(attrib, sheet_print_sequence);
    COMPAREVARVALUE(attrib, print_sequence_modified_time);
    COMPAREVARVALUE(attrib, plans_and_specs_website_guid);
    COMPAREVARVALUE(attrib, last_sync_time);
    COMPAREVARVALUE(attrib, flag_mini_dwf);
    COMPAREVARVALUE(attrib, modified_block_timestamp);
    COMPAREVARVALUE(attrib, dwf_container_guid);
    COMPAREVARVALUE(attrib, container_modified_time);
    COMPAREVARVALUE(attrib, dwf_discipline_guid);
    COMPAREVARVALUE(attrib, dwf_discipline_modified_time);
    COMPAREVARVALUE(attrib, zValue);
    COMPAREVARVALUE(attrib, scan_flag);
    COMPAREVARVALUE(attrib, mirror_flag);
    COMPAREVARVALUE(attrib, inversion_flag);
    COMPAREVARVALUE(attrib, paper_scale);
    COMPAREVARVALUE(attrib, orientation);
    COMPAREVARVALUE(attrib, rotation);
    COMPAREVARVALUE(attrib, alignment);
    COMPAREARRAYVARVALUE(attrib  ,inked_area, 2);
    COMPAREVARVALUE(attrib, dpi_resolution);
    COMPAREARRAYVARVALUE(attrib  ,paper_offset, 2);
    if(Verify(clip_rectangle, ((WT_BlockRef *) this)->get_format()))
    {
        if(!(((WT_BlockRef *) this)->get_clip_rectangle()[0] == ((WT_BlockRef &)attrib).get_clip_rectangle()[0]))
            return WD_False;
        if(!(((WT_BlockRef *) this)->get_clip_rectangle()[1] == ((WT_BlockRef &)attrib).get_clip_rectangle()[1]))
            return WD_False;
    }

    COMPAREVARVALUE(attrib, password);
    COMPAREARRAYVARVALUE(attrib  ,image_representation, 3);
    ((WT_BlockRef *) this)->m_targeted_matrix_rep.set(((WT_BlockRef &)attrib).get_targeted_matrix_rep());
    return WD_True;
}

///////////////////////////////////////////////////////////////////////////
WT_Boolean    WT_BlockRef::operator!= (WT_BlockRef const & attrib) const
{
    return !(*this == attrib);
}

///////////////////////////////////////////////////////////////////////////
WT_BlockRef   const &   WT_BlockRef::operator=(WT_BlockRef const & blockref)
{
    ((WT_BlockRef *) this)->set_format(((WT_BlockRef &)blockref).get_format());

    //ideally we should leave out block_size and file_offset fields while
    //copying because, those anyway gets reset during blockref
    //serialization. But, still we do it because, the parent application
    //might want to just make a copy of a particular blockref object. But
    //remember that equality check always ignores these two fields.
    m_file_offset = ((WT_BlockRef &)blockref).get_file_offset();
    m_block_size = ((WT_BlockRef &)blockref).get_block_size();
    COPYVARVALUE(blockref, block_guid);
    COPYVARVALUE(blockref, creation_time);
    COPYVARVALUE(blockref, modification_time);
    COPYVARVALUE(blockref, encryption);
    COPYVARVALUE(blockref, validity);
    COPYVARVALUE(blockref, visibility);
    COPYVARVALUE(blockref, block_meaning);
    COPYVARVALUE(blockref, parent_block_guid);
    COPYVARVALUE(blockref, related_overlay_hdr_block_guid);
    COPYVARVALUE(blockref, sheet_print_sequence);
    COPYVARVALUE(blockref, print_sequence_modified_time);
    COPYVARVALUE(blockref, plans_and_specs_website_guid);
    COPYVARVALUE(blockref, last_sync_time);
    COPYVARVALUE(blockref, flag_mini_dwf);
    COPYVARVALUE(blockref, modified_block_timestamp);
    COPYVARVALUE(blockref, dwf_container_guid);
    COPYVARVALUE(blockref, container_modified_time);
    COPYVARVALUE(blockref, dwf_discipline_guid);
    COPYVARVALUE(blockref, dwf_discipline_modified_time);
    COPYVARVALUE(blockref, zValue);
    COPYVARVALUE(blockref, scan_flag);
    COPYVARVALUE(blockref, mirror_flag);
    COPYVARVALUE(blockref, inversion_flag);
    COPYVARVALUE(blockref, paper_scale);
    COPYVARVALUE(blockref, orientation);
    COPYVARVALUE(blockref, rotation);
    COPYVARVALUE(blockref, alignment);
    COPYARRAYVARVALUE(blockref  ,inked_area, 2);
    COPYVARVALUE(blockref, dpi_resolution);
    COPYARRAYVARVALUE(blockref  ,paper_offset, 2);
    COPYARRAYVARVALUE(blockref  ,clip_rectangle, 2);
    COPYVARVALUE(blockref, password);
    COPYARRAYVARVALUE(blockref  ,image_representation, 3);
    ((WT_BlockRef *) this)->m_targeted_matrix_rep.set(((WT_BlockRef &)blockref).get_targeted_matrix_rep());

    return *this;
}
