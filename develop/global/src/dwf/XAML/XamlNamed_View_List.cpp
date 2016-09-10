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

WT_Result WT_XAML_Named_View_List::serialize(
    WT_File &file) const
{
    WT_XAML_File& rFile = static_cast<WT_XAML_File &>(file);
    if (rFile.serializingAsW2DContent() )
    {
        if ( !rFile.w2dContentFile() )
            return WT_Result::Toolkit_Usage_Error;
        else
            return WT_Named_View_List::serialize( *rFile.w2dContentFile() );
    }
    
    WD_CHECK( rFile.dump_delayed_drawable() );
    WD_CHECK( rFile.serializeRenditionSyncEndElement() ); 

    DWFXMLSerializer* pW2XSerializer = rFile.w2xSerializer();
	if(!pW2XSerializer)
    {
		return WT_Result::Internal_Error;
    }

    pW2XSerializer->startElement(XamlXML::kpzNamed_View_List_Element);

    WT_Named_View *pView = (WT_Named_View *) get_head();
    while (pView != NULL)
    {
        //
        // WT_XAML_Named_View does not have any specific members,
        // therefore a static cast is okay to re-use the serialize()
        // code
        //
        static_cast<WT_XAML_Named_View *>(pView)->serialize(rFile);
        pView = (WT_Named_View *)pView->next();
    }
  
    pW2XSerializer->endElement();

    return WT_Result::Success;
}
