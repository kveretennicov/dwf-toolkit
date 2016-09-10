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
// $Header: /Components/Internal/DWF Toolkit/v7.1/develop/global/src/dwf/whiptk/informational.cpp 3     5/09/05 12:42a Evansg $


#include "whiptk/pch.h"

IMPLEMENT_INFORMATIONAL_CLASS_FUNCTIONS (Author,          author,          Informational, "Author")
IMPLEMENT_INFORMATIONAL_CLASS_FUNCTIONS (Comments,        comments,        Informational, "Comments")
IMPLEMENT_INFORMATIONAL_CLASS_FUNCTIONS (Copyright,       copyright,       Informational, "Copyright")
IMPLEMENT_INFORMATIONAL_CLASS_FUNCTIONS (Creator,         creator,         Informational, "Creator")
IMPLEMENT_INFORMATIONAL_CLASS_FUNCTIONS (Description,     description,     Informational, "Description")
IMPLEMENT_INFORMATIONAL_CLASS_FUNCTIONS (Keywords,        keywords,        Informational, "Keywords")
IMPLEMENT_INFORMATIONAL_CLASS_FUNCTIONS (Title,           title,           Informational, "Title")
IMPLEMENT_INFORMATIONAL_CLASS_FUNCTIONS (Subject,         subject,         Informational, "Subject")
IMPLEMENT_INFORMATIONAL_CLASS_FUNCTIONS (Source_Filename, source_filename, Informational, "SourceFilename")


///////////////////////////////////////////////////////////////////////////
#if DESIRED_CODE(WHIP_OUTPUT)
WT_Result WT_Informational::serialize(WT_File & file, char const * command) const
{
    if (!m_string.length())
        return WT_Result::Success;

    WD_CHECK (file.dump_delayed_drawable());

    file.desired_rendition().blockref();
    WD_CHECK(file.desired_rendition().sync(file, WT_Rendition::BlockRef_Bit));

    WD_CHECK (file.write_tab_level());
    WD_CHECK (file.write ("("));
    WD_CHECK (file.write (command));
    WD_CHECK (file.write ((WT_Byte) ' '));
    WD_CHECK (m_string.serialize(file));
    return    file.write ((WT_Byte) ')');
}
#else
WT_Result WT_Informational::serialize(WT_File &, char const *) const
{
    return WT_Result::Success;
}
#endif  // DESIRED_CODE()

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Informational::materialize(WT_Opcode const & opcode, WT_File & file)
{

    if (opcode.type() != WT_Opcode::Extended_ASCII)
        return WT_Result::Opcode_Not_Valid_For_This_Object;

    switch (m_stage)
    {
    case Eating_Initial_Whitespace:
        WD_CHECK (file.eat_whitespace());
        m_stage = Gathering_String;
        // No Break Here

    case Gathering_String:
        WD_CHECK (m_string.materialize(file));
        m_stage = Eating_End_Whitespace;
        // No Break here

    case Eating_End_Whitespace:
        WD_CHECK (opcode.skip_past_matching_paren(file));
        m_stage = Eating_Initial_Whitespace;
    }

    m_materialized = WD_True;

    return WT_Result::Success;
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Informational::preprocess_workarounds(WT_Object & item, WT_File & file)
{
    if (item.object_id() == WT_Object::Creator_ID)
    {
        WT_Creator *creator = (WT_Creator *) &item;
        if (creator->string().length()==0)
            return WT_Result::Success;

        // Determine if we need to perform a "broken plot settings" fix (when we process the plot_info object)
        char* pNarrow = new char[creator->string().length()*2 + 1];
        if (pNarrow == NULL)
            return WT_Result::Out_Of_Memory_Error;

        // FIXME:  Can't we just look for the target strings in "short" form, without transcoding to MBCS first?
        #ifdef WD_WIN32_SYSTEM
            int result = ::WideCharToMultiByte(CP_ACP, 0, (WCHAR*)creator->string().unicode(), -1, pNarrow, (int) creator->string().length()*2 + 1, NULL, NULL);
            WD_Assert(result != 0);
            result = 0; //C4189
        #else
            WD_Complain ("WT_Informational::preprocess_workarounds is not implemented on this system");
        #endif

        if( (strstr(pNarrow, "Genuine AutoCAD 2000i (15.05") != NULL)   ||  // Banff release
            (strstr(pNarrow, "Genuine AutoCAD 2000 (15.0")   != NULL)   ||  // Tahoe release
            (strstr(pNarrow, "Genuine AutoCAD 2000i (U")     != NULL)   ||  // Banff pre-release
            (strstr(pNarrow, "Genuine AutoCAD 2000 (T")      != NULL) )     // Tahoe pre-release
        {
            file.heuristics().set_broken_plotinfo(WD_True);
        }
        delete [] pNarrow;
    }
    return WT_Result::Success;
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Informational::skip_operand(WT_Opcode const & opcode, WT_File & file)
{
    return opcode.skip_past_matching_paren(file);
}

///////////////////////////////////////////////////////////////////////////
void WT_Informational::set(char const * string)
{
    m_string = (unsigned char const*) string;
}

///////////////////////////////////////////////////////////////////////////
void WT_Informational::set(WT_Unsigned_Integer16 const * string)
{
    m_string.set(WT_String::wcslen(string),string);
}

///////////////////////////////////////////////////////////////////////////
void WT_Informational::set(int length, WT_Unsigned_Integer16 const * string)
{
    m_string.set(length,string);
}

///////////////////////////////////////////////////////////////////////////
void WT_Informational::set(WT_String const & string)
{
    m_string = string;
}
