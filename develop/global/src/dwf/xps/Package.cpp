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
//  $Header: //DWF/Development/Components/Internal/DWF Toolkit/v7.7/develop/global/src/dwf/xps/Package.cpp#1 $
//  $DateTime: 2011/02/14 01:16:30 $
//  $Author: caos $
//  $Change: 197964 $
//  $Revision: #1 $
//
//

#include "dwf/xps/Package.h"
#include "dwf/xps/Constants.h"
using namespace DWFToolkit;


_DWFTK_API
XPSPackage::XPSPackage()
throw( DWFException )
          : _pFixedDocumentSequence( NULL )
{;}

_DWFTK_API
XPSPackage::~XPSPackage()
throw()
{
    if (_pFixedDocumentSequence != NULL)
    {
        if (_pFixedDocumentSequence->owner() == this)
        {
            notifyOwnableDeletion( *_pFixedDocumentSequence );
            DWFCORE_FREE_OBJECT( _pFixedDocumentSequence );
        }
        else
        {
            _pFixedDocumentSequence->unobserve( *this );
        }
    }
}

_DWFTK_API
XPSFixedDocumentSequence* 
XPSPackage::setFixedDocumentSequence( XPSFixedDocumentSequence* pXPSFixedDocumentSequence, 
                                      bool bOwn )
throw()
{
    XPSFixedDocumentSequence* pOldFDS = _pFixedDocumentSequence;

    if (pOldFDS != NULL)
    {
        //
        // delete the relationship to this part
        //
        deleteRelationshipsByTarget( pOldFDS );

        if (pOldFDS->owner() == this)
        {
            pOldFDS->disown( *this, true );
        }
        else
        {
            pOldFDS->unobserve( *this );
        }
    }

    _pFixedDocumentSequence = pXPSFixedDocumentSequence;

    if (_pFixedDocumentSequence != NULL)
    {
        if (bOwn)
        {
            _pFixedDocumentSequence->own( *this );
        }
        else
        {
            _pFixedDocumentSequence->observe( *this );
        }

        addRelationship( _pFixedDocumentSequence, XPSXML::kzRelationship_FixedRepresentation );
    }

    return pOldFDS;
}

_DWFTK_API
XPSFixedDocumentSequence * const
XPSPackage::xpsFixedDocumentSequence() const
throw()
{
    return _pFixedDocumentSequence;
}


