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

#include "XAML/pch.h"

WT_Result WT_XAML_Named_View::serialize(
    WT_File &file) const
{
    WT_XAML_File& rFile = static_cast<WT_XAML_File &>(file);
    if (rFile.serializingAsW2DContent() )
    {
        if ( !rFile.w2dContentFile() )
            return WT_Result::Toolkit_Usage_Error;
        else
            return WT_Named_View::serialize( *rFile.w2dContentFile() );
    }
    
    WD_CHECK( rFile.dump_delayed_drawable() );
    WD_CHECK( rFile.serializeRenditionSyncEndElement() ); 

    DWFXMLSerializer* pW2XSerializer = rFile.w2xSerializer();
	if(!pW2XSerializer)
    {
		return WT_Result::Internal_Error;
    }

    pW2XSerializer->startElement(XamlXML::kpzNamed_View_Element);
    
    pW2XSerializer->addAttribute(
        XamlXML::kpzRefName_Attribute,
        rFile.nameIndexString());

    rFile.nameIndex()++;

    pW2XSerializer->addAttribute(
        XamlXML::kpzName_Attribute,
        name().ascii());

    wchar_t pBuf[ 128 ];

    WT_Logical_Point        adjusted_min;
    WT_Logical_Point        adjusted_max;
    WT_Logical_Point        final_min;
    WT_Logical_Point        final_max;

    if (rFile.heuristics().apply_transform())
    {
        WT_Transform const &    xform = rFile.heuristics().transform();
        WT_Logical_Point        orig_xformed_min_cnr;
        WT_Logical_Point        orig_xformed_max_cnr;

        orig_xformed_min_cnr = view()->m_min * xform;
        orig_xformed_max_cnr = view()->m_max * xform;

        switch (xform.rotation())
        {
            case 0:
                adjusted_min = orig_xformed_min_cnr;
                adjusted_max = orig_xformed_max_cnr;
                break;
            case 90:
                adjusted_min = WT_Logical_Point(orig_xformed_max_cnr.m_x, orig_xformed_min_cnr.m_y);
                adjusted_max = WT_Logical_Point(orig_xformed_min_cnr.m_x, orig_xformed_max_cnr.m_y);
                break;
            case 180:
                adjusted_min = orig_xformed_max_cnr;
                adjusted_max = orig_xformed_min_cnr;
                break;
            case 270:
                adjusted_min = WT_Logical_Point(orig_xformed_min_cnr.m_x, orig_xformed_max_cnr.m_y);
                adjusted_max = WT_Logical_Point(orig_xformed_max_cnr.m_x, orig_xformed_min_cnr.m_y);
                break;
            default:
                throw WT_Result::Internal_Error;
        }
    }
    else
    {
        adjusted_min = view()->m_min;
        adjusted_max = view()->m_max;
    }

    final_min.m_x = WD_MIN(adjusted_min.m_x, adjusted_max.m_x);
    final_min.m_y = WD_MIN(adjusted_min.m_y, adjusted_max.m_y);

    final_max.m_x = WD_MAX(adjusted_min.m_x, adjusted_max.m_x);
    final_max.m_y = WD_MAX(adjusted_min.m_y, adjusted_max.m_y);

    _DWFCORE_SWPRINTF(pBuf, 128, L"%d %d %d %d", 
        final_min.m_x,
        final_min.m_y,
        final_max.m_x,
        final_max.m_y);

    pW2XSerializer->addAttribute(
        XamlXML::kpzArea_Attribute,
        pBuf);

    pW2XSerializer->endElement();

    return WT_Result::Success;
}

WT_Result
WT_XAML_Named_View::parseAttributeList(XamlXML::tAttributeMap& rMap, WT_XAML_File& /*rFile*/)
{
	if(!rMap.size())
		return WT_Result::Internal_Error;

    const char** ppName = rMap.find( XamlXML::kpzName_Attribute );
    if (ppName!=NULL && *ppName!=NULL)
    {
        set( *ppName );
    }
    else
    {
        return WT_Result::Corrupt_File_Error;
    }

    const char** ppArea = rMap.find( XamlXML::kpzArea_Attribute );
    if (ppArea!=NULL && *ppArea!=NULL)
    {
        int minX =0,minY =0,maxX =0,maxY = 0;
        int nProcessed = sscanf( *ppArea, "%d %d %d %d", &minX,&minY,&maxX,&maxY);
        if (nProcessed == 4)
        {
            WT_Logical_Box box;
            box.m_min.m_x = minX;
            box.m_min.m_y = minY;
            box.m_max.m_x = maxX;
            box.m_max.m_y = maxY;
            set( box );
        }
        else
        {
            return WT_Result::Internal_Error;
        }
    }
    else
    {
        return WT_Result::Corrupt_File_Error;
    }
	
    materialized() = WD_True;
	return WT_Result::Success;
}

///////////////////////////////////////////////////////////////////////////
WT_Named_View* WT_XAML_Named_View::copy()
{
    WT_XAML_Named_View *pView = new WT_XAML_Named_View( *this );
    return pView;
}
