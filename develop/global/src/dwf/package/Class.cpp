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
//  $Header: //DWF/Development/Components/Internal/DWF Toolkit/v7.7/develop/global/src/dwf/package/Class.cpp#1 $
//  $DateTime: 2011/02/14 01:16:30 $
//  $Author: caos $
//  $Change: 197964 $
//  $Revision: #1 $
//

#include "dwf/package/Class.h"
#include "dwf/package/Constants.h"
#include "dwf/package/writer/PackageWriter.h"

using namespace DWFToolkit;

_DWFTK_API
DWFClass::DWFClass( const DWFString& zID,
                    DWFContent* pContent )
throw()
           : DWFContentElement( zID, pContent )
           , _oBaseClasses()
{
}

_DWFTK_API
DWFClass::~DWFClass()
throw()
{
}

_DWFTK_API
void
DWFClass::parseAttributeList( const char** ppAttributeList,
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
        // get any base class references
        //
        if (DWFCORE_COMPARE_ASCII_STRINGS(pAttrib, DWFXML::kzAttribute_ClassRefs) == 0)
        {
            rUnresolved.push_back( tUnresolved(eBaseClassReferences, DWFString(ppAttributeList[iAttrib+1])) );
            break;
        }
    }
}

#ifndef DWFTK_READ_ONLY

_DWFTK_API
void
DWFClass::_serializeAttributes( DWFXMLSerializer& rSerializer, 
                                unsigned int nFlags )
throw( DWFException )
{
    //
    //  First let the base class serialize it attributes
    //
    DWFContentElement::_serializeAttributes( rSerializer, nFlags );

    //
    //  Serialize any base class references.
    //
    if (_oBaseClasses.size() > 0)
    {
        DWFClass::tIterator* piBase = _oBaseClasses.iterator();
        if (piBase)
        {
            DWFString zReferences;
            DWFClass* pReference = NULL;
            for (; piBase->valid(); piBase->next())
            {
                pReference = piBase->get();
                
                zReferences.append( pReference->id() );
                zReferences.append( /*NOXLATE*/L" " );
            }

            if (zReferences.chars() > 0)
            {
                rSerializer.addAttribute( DWFXML::kzAttribute_ClassRefs, zReferences );
            }

            DWFCORE_FREE_OBJECT( piBase );
        }
    }
}

_DWFTK_API
void 
DWFClass::_serializeXML( DWFXMLSerializer& rSerializer, 
                         unsigned int nFlags )
throw( DWFException )
{
    if (nFlags & DWFPackageWriter::eGlobalContent)
    {
        //
        //  Open the element
        //
        DWFString zNamespace( _oSerializer.namespaceXML( nFlags ) );
        rSerializer.startElement( DWFXML::kzElement_Class, zNamespace );

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
DWFClass::insertPropertyAncestors( DWFContentElement::tVector& rAncestorElements ) const
throw()
{
    for (size_t i = 0; i < _oBaseClasses.size(); ++i)
    {
        rAncestorElements.push_back( _oBaseClasses[i] );
    }
}


