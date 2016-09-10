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
//  $Header: //DWF/Development/Components/Internal/DWF Toolkit/v7.7/develop/global/src/dwf/dwfx/FixedDocument.cpp#1 $
//  $DateTime: 2011/02/14 01:16:30 $
//  $Author: caos $
//  $Change: 197964 $
//  $Revision: #1 $
//
//

#include "dwf/dwfx/FixedDocument.h"
#include "dwf/dwfx/Constants.h"
using namespace DWFToolkit;


_DWFTK_API
DWFXFixedDocument::DWFXFixedDocument( DWFManifest* pManifest)
throw()
                 : _pManifest( pManifest )
{;}

_DWFTK_API
DWFXFixedDocument::~DWFXFixedDocument()
throw()
{

    // As an ownable class, we need to notify observers that we are about to be deleted
    _notifyDelete();
}

//_DWFTK_API
//bool
//DWFXFixedDocument::addFixedPage( XPSFixedPage* pFixedPage,
//                                 bool bOwn )
//throw()
//{
//    bool bAdded = XPSFixedDocument::addFixedPage( pFixedPage, bOwn );
//
//    if (bAdded)
//    {
//        addRelationship( pFixedPage, DWFXXML::kzRelationship_RequiredResource );
//    }
//
//    return bAdded;
//}
//
//_DWFTK_API
//bool 
//DWFXFixedDocument::insertFixedPage( XPSFixedPage* pFixedPage, 
//                                    XPSFixedPage* pAfterPage,
//                                    bool bOwn )
//throw()
//{
//    bool bInserted = XPSFixedDocument::insertFixedPage( pFixedPage, pAfterPage, bOwn );
//
//    if (bInserted)
//    {
//        addRelationship( pFixedPage, DWFXXML::kzRelationship_RequiredResource );
//    }
//
//    return bInserted;
//}
//
//_DWFTK_API
//bool
//DWFXFixedDocument::removeFixedPage( XPSFixedPage* pFixedPage )
//throw()
//{
//    bool bRemoved = XPSFixedDocument::removeFixedPage( pFixedPage );
//
//    if (bRemoved)
//    {
//        //
//        // delete the relationship to this part
//        //
//        deleteRelationshipsByTarget( pFixedPage );
//
//    }
//
//    return bRemoved;
//}


