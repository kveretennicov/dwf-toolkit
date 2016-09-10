#include "StdAfx.h"
#include <math.h>

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

WT_Result read_delineate(WT_Delineate &, WT_File &)
{
    return WT_Result::Success;
}

WT_Result write_delineate(WT_File& whip_file)
{
	WT_Class_Factory *classFactory = CTestFramework::Instance().GetClassFactory();

	WT_Delineate *delineate1 = classFactory->Create_Delineate(WD_True);
    WT_Logical_Point points[5];
    
    double radian = 0.017453293; //hard coded 
    double offset = 180.0/4.0;

    for (int i = 0; i < 5 ; i++)
    {
        points[i].m_x = 10 * i ;
        points[i].m_y = 10 * i * (WT_Integer32) ceil(sin(radian * (offset* i))) ;
    }   
        
	WT_Polyline *polyline = classFactory->Create_Polyline(5,&points[0], false);

    // save out the old delineate rendition value 
    WT_Delineate *previous_delineate = classFactory->Create_Delineate();
	*previous_delineate = whip_file.desired_rendition().delineate();

    // set the new delineate value 
    whip_file.desired_rendition().delineate() = *delineate1;

    // serialize the WT_Polyline 
    polyline->serialize(whip_file);

    //reset the old delineate behavior
    whip_file.desired_rendition().delineate() = *previous_delineate;

	classFactory->Destroy(previous_delineate);
	classFactory->Destroy(polyline);
	classFactory->Destroy(delineate1);

    return WT_Result::Success;
}

// End of file.
