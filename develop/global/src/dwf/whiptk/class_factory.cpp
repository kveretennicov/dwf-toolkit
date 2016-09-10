//
//  Copyright (c) 2006 by Autodesk, Inc.
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

#include "whiptk/pch.h"
#include "whiptk/class_factory.h"

void WT_Class_Factory::Destroy(WT_Object* pObject)
{
	if(pObject)
	{
		delete pObject;
		pObject = NULL;
	}
}

void WT_Class_Factory::Destroy(WT_Attribute*	pAttribute)
{
	if(pAttribute)
	{
		delete pAttribute;
		pAttribute = NULL;
	}
}

void WT_Class_Factory::Destroy(WT_File*	pFile)
{
    if(pFile)
    {
        delete pFile;
        pFile = NULL;
    }
}

void WT_Class_Factory::Destroy(WT_Dash_Pattern*	pDashPat)
{
    if(pDashPat)
    {
        delete pDashPat;
        pDashPat = NULL;
    }
}

void WT_Class_Factory::Destroy(WT_Layer*	pLayer)
{
    if(pLayer)
    {
        delete pLayer;
        pLayer = NULL;
    }
}

void WT_Class_Factory::Destroy(WT_Named_View*	pNamedView)
{
    if(pNamedView)
    {
        delete pNamedView;
        pNamedView = NULL;
    }
}

void WT_Class_Factory::Destroy(WT_Object_Node*	pObjectNode)
{
    if(pObjectNode)
    {
        delete pObjectNode;
        pObjectNode = NULL;
    }
}
