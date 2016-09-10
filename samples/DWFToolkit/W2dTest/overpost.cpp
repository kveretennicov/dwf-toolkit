#include "stdafx.h"
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

WT_Result read_overpost(WT_Overpost &, WT_File &)
{
    return WT_Result::Success;
}


WT_Result write_overpost(WT_File & whip_file)
{
    //label 1 
    WT_Logical_Point pt(10,10);
	WT_Class_Factory *classFactory = CTestFramework::Instance().GetClassFactory();
    
    WT_Text* pt1 = classFactory->Create_Text(pt,"Label 1");
    
    // label 2
    pt.m_x = 15;
    WT_Text* pt2 = classFactory->Create_Text(pt,"Label 2");

    pt.m_y = 15;
    WT_Text* pt3 = classFactory->Create_Text(pt,"Label 3");
    
    WT_Overpost* pOverpost = classFactory->Create_Overpost(WT_Overpost::AcceptAllFit, WD_True, WD_True);
    
    pOverpost->add(*pt1); //owned
    pOverpost->add(*pt2); //owned
    pOverpost->add(*pt3); //owned

    WD_CHECK (pOverpost->serialize(whip_file));

    classFactory->Destroy( pOverpost );
    
    return WT_Result::Success;
}

// End of file.

