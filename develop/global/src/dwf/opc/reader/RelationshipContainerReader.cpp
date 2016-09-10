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
//  $Header: //DWF/Development/Components/Internal/DWF Toolkit/v7.7/develop/global/src/dwf/opc/reader/RelationshipContainerReader.cpp#1 $
//  $DateTime: 2011/02/14 01:16:30 $
//  $Author: caos $
//  $Change: 197964 $
//  $Revision: #1 $
//
//


#include "dwf/opc/reader/RelationshipContainerReader.h"
#include "dwf/opc/Constants.h"
#include "dwf/opc/Relationship.h"
using namespace DWFToolkit;


_DWFTK_API
OPCRelationshipContainerReader::OPCRelationshipContainerReader( unsigned int nProviderFlags )
throw()
                              : _oDefaultElementBuilder()
                              , _pElementBuilder( &_oDefaultElementBuilder )
                              , _nProviderFlags( nProviderFlags )
                              , _pFilter( NULL )
                              , _pCurrentRelationship( NULL )
{
    ;
}

_DWFTK_API
OPCRelationshipContainerReader::~OPCRelationshipContainerReader()
throw()
{
    ;
}

_DWFTK_API
OPCRelationshipContainerReader*
OPCRelationshipContainerReader::filter() const
throw()
{
    return _pFilter;
}

_DWFTK_API
void
OPCRelationshipContainerReader::setFilter( OPCRelationshipContainerReader* pFilter )
throw()
{
    _pFilter = pFilter;
}

_DWFTK_API
OPCRelationship*
OPCRelationshipContainerReader::provideRelationship( OPCRelationship* pRelationship )
throw()
{
    return pRelationship;
}

_DWFTK_API
void
OPCRelationshipContainerReader::notifyStartElement( const char*  zName,
                                                    const char** ppAttributeList )
throw()
{
    if ((_nProviderFlags & eProvideRelationships) && 
        (DWFCORE_COMPARE_ASCII_STRINGS(zName, OPCXML::kzElement_Relationship) == 0))
    {
        _pCurrentRelationship = NULL;
        _pElementBuilder->build( _pCurrentRelationship, ppAttributeList );
    }
}

_DWFTK_API
void
OPCRelationshipContainerReader::notifyEndElement( const char* zName )
throw()
{
    if (DWFCORE_COMPARE_ASCII_STRINGS(zName, OPCXML::kzElement_Relationship) == 0)
    {
        _provideRelationship( _pCurrentRelationship );
        _pCurrentRelationship = NULL;
    }
}

_DWFTK_API
void
OPCRelationshipContainerReader::notifyStartNamespace( const char* /*zPrefix*/,
                                                      const char* /*zURI*/ )
throw()
{
}

_DWFTK_API
void
OPCRelationshipContainerReader::notifyEndNamespace( const char* /*zPrefix*/ )
throw()
{
}

_DWFTK_API
void
OPCRelationshipContainerReader::notifyCharacterData( const char* /*zCData*/, 
                                                     int         /*nLength*/ ) 
throw()
{
}

void
OPCRelationshipContainerReader::_provideRelationship( OPCRelationship* pRelationship )
throw()
{
    provideRelationship( _pFilter ? _pFilter->provideRelationship(pRelationship) : pRelationship );
}


