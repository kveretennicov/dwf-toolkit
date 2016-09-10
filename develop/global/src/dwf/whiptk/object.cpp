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
// $Header: /NewRoot/DWF Toolkit/v6/develop/global/src/dwf/whiptk/object.cpp 1     9/12/04 8:55p Evansg $

#include "whiptk/pch.h"

// =======================
// WT_Object
// =======================

///////////////////////////////////////////////////////////////////////////
WT_Result    WT_Object::skip_operand(WT_Opcode const & opcode, WT_File & file)
{
    return materialize(opcode, file);
}

// =======================
// WT_Option
// =======================

///////////////////////////////////////////////////////////////////////////
WT_Result    WT_Option::skip_operand(WT_Optioncode const & optioncode, WT_File & file)
{
    if (optioncode.token()[0] == '(')
        return (optioncode.skip_past_matching_paren(file));
    else
        return WT_Result::Success;
}

///////////////////////////////////////////////////////////////////////////
WT_Result    WT_Option::process(WT_File &)
{
    return WT_Result::Success;
}
