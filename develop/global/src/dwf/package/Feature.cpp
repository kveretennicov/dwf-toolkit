//
//  Copyright (c) 2005-2006 by Autodesk, Inc.
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
//  $Header: //DWF/Development/Components/Internal/DWF Toolkit/v7.7/develop/global/src/dwf/package/Feature.cpp#1 $
//  $DateTime: 2011/02/14 01:16:30 $
//  $Author: caos $
//  $Change: 197964 $
//  $Revision: #1 $
//

#include "dwf/package/Feature.h"
#include "dwf/package/Constants.h" 
#include "dwf/package/writer/PackageWriter.h"
using namespace DWFToolkit;


_DWFTK_API
DWFFeature::DWFFeature( const DWFString& zID,
                        DWFContent* pContent )
throw()
          : DWFRenderable( zID, pContent )
          , _oClasses()
{
}

_DWFTK_API
DWFFeature::~DWFFeature()
throw()
{
}

_DWFTK_API
void
DWFFeature::parseAttributeList( const char** ppAttributeList,
                                tUnresolvedList& rUnresolved )
throw( DWFException )
{
    if (ppAttributeList == NULL)
    {
        _DWFCORE_THROW( DWFInvalidArgumentException, /*NOXLATE*/L"No attributes provided" );
    }

    DWFPropertySet::parseAttributeList( ppAttributeList, rUnresolved );

    size_t iAttrib = 0;
    const char* pAttrib = NULL;

    for(; ppAttributeList[iAttrib]; iAttrib += 2)
    {
        pAttrib = &ppAttributeList[iAttrib][0];

        //
        // get any class references
        //
        if (DWFCORE_COMPARE_ASCII_STRINGS(pAttrib, DWFXML::kzAttribute_ClassRefs) == 0)
        {
            rUnresolved.push_back( tUnresolved(eClassReferences, DWFString(ppAttributeList[iAttrib+1])) );
            break;
        }
    }
}

#ifndef DWFTK_READ_ONLY

_DWFTK_API
void
DWFFeature::_serializeAttributes( DWFXMLSerializer& rSerializer, 
                                  unsigned int nFlags )
throw( DWFException )
{
    //
    //  First let the base class serialize it attributes
    //
    DWFContentElement::_serializeAttributes( rSerializer, nFlags );

    //
    //  Serialize any class references.
    //
    if (_oClasses.size() > 0)
    {
        DWFClass::tIterator* piClass = _oClasses.iterator();
        if (piClass)
        {
            //
            //  String together all the references 
            //
            DWFString zReferences;
            DWFClass* pReference = NULL;
            for (; piClass->valid(); piClass->next())
            {
                pReference = piClass->get();
                
                zReferences.append( pReference->id() );
                zReferences.append( /*NOXLATE*/L" " );
            }

            if (zReferences.chars() > 0)
            {
                rSerializer.addAttribute( DWFXML::kzAttribute_ClassRefs, zReferences );
            }

            DWFCORE_FREE_OBJECT( piClass );
        }
    }
}

_DWFTK_API
void 
DWFFeature::_serializeXML( DWFXMLSerializer& rSerializer, 
                           unsigned int nFlags )
throw( DWFException )
{
    if (nFlags & DWFPackageWriter::eGlobalContent)
    {
        //
        //  Open the element
        //
        DWFString zNamespace( _oSerializer.namespaceXML( nFlags ) );
        rSerializer.startElement( DWFXML::kzElement_Feature, zNamespace );

        //
        //  Serialize attributes
        //
        _serializeAttributes( rSerializer, nFlags );

        //
        //  Serialize child elements
        //
        {
            //
            //  Let baseclass know not to start a new XML element
            //
            bool bElementOpenFlag = false;
            if (nFlags & DWFXMLSerializer::eElementOpen)
            {
                bElementOpenFlag = true;
            }
            else
            {
                nFlags |= DWFXMLSerializer::eElementOpen;
            }

            DWFContentElement::_serializeXML( rSerializer, nFlags );

            if (bElementOpenFlag == false)
            {
                //
                //  Unset the element open bit to allow proper ending
                //
                nFlags &= ~DWFXMLSerializer::eElementOpen;
            }
        }

        rSerializer.endElement();
    }
}

#endif

_DWFTK_API
void
DWFFeature::insertPropertyAncestors( DWFContentElement::tVector& rAncestorElements ) const
throw()
{
    for (size_t i = 0; i < _oClasses.size(); ++i)
    {
        rAncestorElements.push_back( _oClasses[i] );
    }
}



