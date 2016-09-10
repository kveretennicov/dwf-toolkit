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
// $Header: /NewRoot/DWF Toolkit/v6/develop/global/src/dwf/whiptk/viewport_options.cpp 1     9/12/04 8:57p Evansg $

#include "whiptk/pch.h"

///////////////////////////////////////////////////////////////////////////
int WT_Viewport_Optioncode::option_id_from_optioncode()
{
    if (!strcmp((char const *)token(),"(Units"))
        m_option_id = Viewport_Units_Option;
    else
        m_option_id = Viewport_Unknown_Option;

    return m_option_id;
}

///////////////////////////////////////////////////////////////////////////
WT_Object::WT_ID WT_Viewport_Option_Viewport_Units::object_id() const
{
    return WT_Object::Viewport_Option_Viewport_Units_ID;
}

///////////////////////////////////////////////////////////////////////////
#if DESIRED_CODE(WHIP_OUTPUT)
WT_Result WT_Viewport_Option_Viewport_Units::serialize(WT_Object const &, WT_File & file) const
{
    return WT_Units::serialize(file);
}
#else
WT_Result WT_Viewport_Option_Viewport_Units::serialize(WT_Object const &, WT_File &) const
{
    return WT_Result::Success;
}
#endif  // DESIRED_CODE()

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Viewport_Option_Viewport_Units::materialize(WT_Object &, WT_Optioncode const & optioncode, WT_File & file)
{
    return WT_Units::materialize((WT_Opcode const &)optioncode,file);
}

///////////////////////////////////////////////////////////////////////////
WT_Boolean WT_Viewport_Option_Viewport_Units::operator==(WT_Viewport_Option_Viewport_Units const & option ) const
{
    return ((WT_Units const &)*this) == ((WT_Units const &)option);
}

///////////////////////////////////////////////////////////////////////////
WT_Boolean WT_Viewport_Option_Viewport_Units::operator!=(WT_Viewport_Option_Viewport_Units const & option ) const
{
    return !(*this == option);
}
