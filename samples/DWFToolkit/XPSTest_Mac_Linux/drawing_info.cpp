#include "StdAfx.h"

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
// $Header: //DWF/Development/Components/Internal/DWF Toolkit/v7.7/samples/DWFToolkit/XPSTest_Mac_Linux/drawing_info.cpp#1 $

#include "TestFramework.h"

WT_Result read_drawing_info(WT_Drawing_Info & /*polytriangle*/, WT_File &/*whip_file*/)
{
    return WT_Result::Success;
}

WT_Result write_drawing_info (WT_File & whip_file)
{
    if (CTestFramework::Instance().GetClassFactoryType() != CTestFramework::XAML)
    {
        whip_file.desired_rendition().drawing_info().author().set("Jeffrey Klug");
        whip_file.desired_rendition().drawing_info().comments().set("W2dTest - comments");
        whip_file.desired_rendition().drawing_info().copyright().set("Copyright 2002, Autodesk, Inc.");
        whip_file.desired_rendition().drawing_info().creator().set("W2dTest.exe");
        whip_file.desired_rendition().drawing_info().description().set("Drawing file used to test the Whip toolkit");
        whip_file.desired_rendition().drawing_info().keywords().set("Keywords");
        whip_file.desired_rendition().drawing_info().subject().set("Testing");
        whip_file.desired_rendition().drawing_info().title().set("DwfTest test drawing");

        WD_CHECK(whip_file.desired_rendition().drawing_info().sync(whip_file));
    }

    return WT_Result::Success;
}
