#include "StdAfx.h"
#include "TestFramework.h"

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
// $Header: //DWF/Development/Components/Internal/DWF Toolkit/v7.7/samples/DWFToolkit/W2dTest_Mac_Linux/userdata.cpp#1 $

#if defined(_DWFCORE_WIN32_SYSTEM)
#include <io.h>
#endif

#include <sys/types.h>
#include <sys/stat.h>

WT_Result read_userdata(WT_UserData & /*userdata*/, WT_File &/*whip_file*/)
{
    return WT_Result::Success;
}

WT_Result write_userdata(WT_File & whip_file)
{
	WT_Class_Factory *classFactory = CTestFramework::Instance().GetClassFactory();

    //test user data
    char null_area[128];
    memset(null_area, 0, 128);
	WT_UserData *userdata = classFactory->Create_UserData(WT_String("Null area"), 128, (WT_Byte*)null_area, WD_True);
    WD_CHECK(userdata->serialize(whip_file));

	classFactory->Destroy(userdata);

    return WT_Result::Success;
}

