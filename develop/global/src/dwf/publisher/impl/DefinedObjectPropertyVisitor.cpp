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
//  $Header: //DWF/Development/Components/Internal/DWF Toolkit/v7.7/develop/global/src/dwf/publisher/impl/DefinedObjectPropertyVisitor.cpp#1 $
//  $DateTime: 2011/02/14 01:16:30 $
//  $Author: caos $
//  $Change: 197964 $
//

#ifndef DWFTK_READ_ONLY


#include "dwf/package/ObjectDefinitionResource.h"
#include "dwf/package/utility/PropertyContainer.h"
#include "dwf/publisher/impl/DefinedObjectPropertyVisitor.h"

using namespace DWFToolkit;

_DWFTK_API
DWFDefinedObjectPropertyVisitor::DWFDefinedObjectPropertyVisitor()
throw()
                               : DWFPropertyVisitor()
                               , _pObjectDefinition( NULL )
{;}

_DWFTK_API
void
DWFDefinedObjectPropertyVisitor::visitPropertyContainer( DWFPropertyContainer& rPropertyContainer )
throw( DWFException )
{
    if (_pObjectDefinition == NULL)
    {
        _DWFCORE_THROW( DWFNullPointerException, /*NOXLATE*/L"The object definition resource pointer was not initialized" );
    }

    //
    // when we visited the published object, we made sure to copy the properties
    // and container references, what has not happened though is the actual
    // definition of those property sets. 
    //

    //
    // remove the owned containers from the object but be sure
    // to leave references to them
    //
    DWFPropertyContainer::tList oContainerList;
    rPropertyContainer.removeOwnedPropertyContainers( oContainerList, true );

    //
    // now add each container to the object properties of the current section
    //
    DWFPropertyContainer::tList::const_iterator iContainer = oContainerList.begin();
    for (; iContainer != oContainerList.end(); iContainer++)
    {
        _pObjectDefinition->getObjectProperties().addPropertyContainer( *iContainer );
    }
}


#endif

