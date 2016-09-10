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
//  $Header: //DWF/Development/Components/Internal/DWF Toolkit/v7.7/develop/global/src/dwf/opc/CoreProperties.cpp#1 $
//  $DateTime: 2011/02/14 01:16:30 $
//  $Author: caos $
//  $Change: 197964 $
//  $Revision: #1 $
//
//

#include "dwf/opc/CoreProperties.h"
#include "dwf/opc/Constants.h"
using namespace DWFToolkit;


//DNT_Start

#if defined(DWFTK_STATIC) || !defined(_DWFCORE_WIN32_SYSTEM)

const char* const OPCCoreProperties::kzName  = "CoreProperties.xml";

#endif


_DWFTK_API
OPCCoreProperties::OPCCoreProperties()
throw()
                 : _oOPCProperties()
                 , _oDCProperties()
                 , _oDCTermsProperties()
{
    setPath( L"/" );
    setName( OPCCoreProperties::kzName );
}


_DWFTK_API
OPCCoreProperties::~OPCCoreProperties()
throw()
{
    // As an ownable class, we need to notify observers that we are about to be deleted
    _notifyDelete();
}

_DWFTK_API
void
OPCCoreProperties::setPropertyKeywords( const DWFString& zValue )
throw()
{
    _oOPCProperties.insert( std::make_pair(OPCXML::kzCoreProperty_Keywords, zValue) );
}

_DWFTK_API
DWFString
OPCCoreProperties::keywords() const
throw()
{
    return _getProperty( _oOPCProperties, OPCXML::kzCoreProperty_Keywords );
}

_DWFTK_API
void
OPCCoreProperties::setPropertyContentType( const DWFString& zValue )
throw()
{
    _oOPCProperties.insert( std::make_pair(OPCXML::kzCoreProperty_ContentType, zValue) );
}

_DWFTK_API
DWFString
OPCCoreProperties::contentType() const
throw()
{
    return _getProperty( _oOPCProperties, OPCXML::kzCoreProperty_ContentType );
}

_DWFTK_API
void
OPCCoreProperties::setPropertyCategory( const DWFString& zValue )
throw()
{
    _oOPCProperties.insert( std::make_pair(OPCXML::kzCoreProperty_Category, zValue) );
}

_DWFTK_API
DWFString
OPCCoreProperties::category() const
throw()
{
    return _getProperty( _oOPCProperties, OPCXML::kzCoreProperty_Category );
}

_DWFTK_API
void
OPCCoreProperties::setPropertyVersion( const DWFString& zValue )
throw()
{
    _oOPCProperties.insert( std::make_pair(OPCXML::kzCoreProperty_Version, zValue) );
}

_DWFTK_API
DWFString
OPCCoreProperties::version() const
throw()
{
    return _getProperty( _oOPCProperties, OPCXML::kzCoreProperty_Version );
}

_DWFTK_API
void
OPCCoreProperties::setPropertyRevision( const DWFString& zValue )
throw()
{
    _oOPCProperties.insert( std::make_pair(OPCXML::kzCoreProperty_Revision, zValue) );
}

_DWFTK_API
DWFString
OPCCoreProperties::revision() const
throw()
{
    return _getProperty( _oOPCProperties, OPCXML::kzCoreProperty_Revision );
}

_DWFTK_API
void
OPCCoreProperties::setPropertyLastModifiedBy( const DWFString& zValue )
throw()
{
    _oOPCProperties.insert( std::make_pair(OPCXML::kzCoreProperty_LastModifiedBy, zValue) );
}

_DWFTK_API
DWFString
OPCCoreProperties::lastModifiedBy() const
throw()
{
    return _getProperty( _oOPCProperties, OPCXML::kzCoreProperty_LastModifiedBy );
}

_DWFTK_API
void
OPCCoreProperties::setPropertyLastPrinted( const DWFString& zValue )
throw()
{
    _oOPCProperties.insert( std::make_pair(OPCXML::kzCoreProperty_LastPrinted, zValue) );
}

_DWFTK_API
DWFString
OPCCoreProperties::lastPrinted() const
throw()
{
    return _getProperty( _oOPCProperties, OPCXML::kzCoreProperty_LastPrinted );
}

_DWFTK_API
void
OPCCoreProperties::setPropertyContentStatus( const DWFString& zValue )
throw()
{
    _oOPCProperties.insert( std::make_pair(OPCXML::kzCoreProperty_ContentStatus, zValue) );
}

_DWFTK_API
DWFString
OPCCoreProperties::contentStatus() const
throw()
{
    return _getProperty( _oOPCProperties, OPCXML::kzCoreProperty_ContentStatus );
}


_DWFTK_API
void
OPCCoreProperties::setPropertyCreator( const DWFString& zValue )
throw()
{
    _oDCProperties.insert( std::make_pair(OPCXML::kzCoreProperty_Creator, zValue) );
}

_DWFTK_API
DWFString
OPCCoreProperties::creator() const
throw()
{
    return _getProperty( _oDCProperties, OPCXML::kzCoreProperty_Creator );
}

_DWFTK_API
void
OPCCoreProperties::setPropertyIdentifier( const DWFString& zValue )
throw()
{
    _oDCProperties.insert( std::make_pair(OPCXML::kzCoreProperty_Identifier, zValue) );
}

_DWFTK_API
DWFString
OPCCoreProperties::identifier() const
throw()
{
    return _getProperty( _oDCProperties, OPCXML::kzCoreProperty_Identifier );
}

_DWFTK_API
void
OPCCoreProperties::setPropertyTitle( const DWFString& zValue )
throw()
{
    _oDCProperties.insert( std::make_pair(OPCXML::kzCoreProperty_Title, zValue) );
}

_DWFTK_API
DWFString
OPCCoreProperties::title() const
throw()
{
    return _getProperty( _oDCProperties, OPCXML::kzCoreProperty_Title );
}

_DWFTK_API
void
OPCCoreProperties::setPropertySubject( const DWFString& zValue )
throw()
{
    _oDCProperties.insert( std::make_pair(OPCXML::kzCoreProperty_Subject, zValue) );
}

_DWFTK_API
DWFString
OPCCoreProperties::subject() const
throw()
{
    return _getProperty( _oDCProperties, OPCXML::kzCoreProperty_Subject );
}

_DWFTK_API
void
OPCCoreProperties::setPropertyDescription( const DWFString& zValue )
throw()
{
    _oDCProperties.insert( std::make_pair(OPCXML::kzCoreProperty_Description, zValue) );
}

_DWFTK_API
DWFString
OPCCoreProperties::description() const
throw()
{
    return _getProperty( _oDCProperties, OPCXML::kzCoreProperty_Description );
}

_DWFTK_API
void
OPCCoreProperties::setPropertyLanguage( const DWFString& zValue )
throw()
{
    _oDCProperties.insert( std::make_pair(OPCXML::kzCoreProperty_Language, zValue) );
}

_DWFTK_API
DWFString
OPCCoreProperties::language() const
throw()
{
    return _getProperty( _oDCProperties, OPCXML::kzCoreProperty_Language );
}


_DWFTK_API
void
OPCCoreProperties::setPropertyCreated( int nYear, int nMonth, int nDay )
throw( DWFException )
{
    _oDCTermsProperties.insert( std::make_pair(OPCXML::kzCoreProperty_Created, _getDateString(nYear, nMonth, nDay)) );
}

_DWFTK_API
void
OPCCoreProperties::setPropertyCreated( const DWFString& zValue )
throw()
{
    _oDCTermsProperties.insert( std::make_pair(OPCXML::kzCoreProperty_Created, zValue) );
}

_DWFTK_API
DWFString
OPCCoreProperties::created() const
throw()
{
    return _getProperty( _oDCTermsProperties, OPCXML::kzCoreProperty_Created );
}

_DWFTK_API
void
OPCCoreProperties::setPropertyModified( int nYear, int nMonth, int nDay )
throw( DWFException )
{
    _oDCTermsProperties.insert( std::make_pair(OPCXML::kzCoreProperty_Modified, _getDateString(nYear, nMonth, nDay)) );
}

_DWFTK_API
void
OPCCoreProperties::setPropertyModified( const DWFString& zValue )
throw()
{
    _oDCTermsProperties.insert( std::make_pair(OPCXML::kzCoreProperty_Modified, zValue) );
}

_DWFTK_API
DWFString
OPCCoreProperties::modified() const
throw()
{
    return _getProperty( _oDCTermsProperties, OPCXML::kzCoreProperty_Modified );
}

DWFString
OPCCoreProperties::_getProperty( const _tPropertyMap& rMap,
                                 const DWFString& zKey ) const
throw()
{
    _tPropertyMap::const_iterator it = rMap.find( zKey );
    if (it != rMap.end())
    {
        return it->second;
    }
    else
    {
        return DWFString();
    }
}

_DWFTK_API
OPCCoreProperties::tPropertyIterator*
OPCCoreProperties::properties() const
throw()
{
    tPropertyIterator* piIter = NULL;

    if (_oOPCProperties.size() > 0)
    {
        piIter = DWFCORE_ALLOC_OBJECT( tPropertyIterator );
        
        _tPropertyMap::const_iterator it = _oOPCProperties.begin();
        for (; it != _oOPCProperties.end(); ++it)
        {
            piIter->add( *it );
        }
    }

    if (_oDCProperties.size() > 0)
    {
        if (piIter == NULL)
        {
            piIter = DWFCORE_ALLOC_OBJECT( tPropertyIterator );
        }
        
        _tPropertyMap::const_iterator it = _oDCProperties.begin();
        for (; it != _oDCProperties.end(); ++it)
        {
            piIter->add( *it );
        }
    }

    if (_oDCTermsProperties.size() > 0)
    {
        if (piIter == NULL)
        {
            piIter = DWFCORE_ALLOC_OBJECT( tPropertyIterator );
        }

        _tPropertyMap::const_iterator it = _oDCTermsProperties.begin();
        for (; it != _oDCTermsProperties.end(); ++it)
        {
            piIter->add( *it );
        }
    }

    return piIter;
}

_DWFTK_API
OPCCoreProperties::tPropertyIterator*
OPCCoreProperties::propertiesInOPCNamespace() const
throw()
{
    if (_oOPCProperties.size() > 0)
    {
        tPropertyIterator* piIter = DWFCORE_ALLOC_OBJECT( tPropertyIterator );
        
        _tPropertyMap::const_iterator it = _oOPCProperties.begin();
        for (; it != _oOPCProperties.end(); ++it)
        {
            piIter->add( *it );
        }

        return piIter;
    }
    else
    {
        return NULL;
    }
}

_DWFTK_API
OPCCoreProperties::tPropertyIterator*
OPCCoreProperties::propertiesInDCNamespace() const
throw()
{
    if (_oDCProperties.size() > 0)
    {
        tPropertyIterator* piIter = DWFCORE_ALLOC_OBJECT( tPropertyIterator );
        
        _tPropertyMap::const_iterator it = _oDCProperties.begin();
        for (; it != _oDCProperties.end(); ++it)
        {
            piIter->add( *it );
        }

        return piIter;
    }
    else
    {
        return NULL;
    }
}

_DWFTK_API
OPCCoreProperties::tPropertyIterator*
OPCCoreProperties::propertiesInDCTermsNamespace() const
throw()
{
    if (_oDCTermsProperties.size() > 0)
    {
        tPropertyIterator* piIter = DWFCORE_ALLOC_OBJECT( tPropertyIterator );
        
        _tPropertyMap::const_iterator it = _oDCTermsProperties.begin();
        for (; it != _oDCTermsProperties.end(); ++it)
        {
            piIter->add( *it );
        }

        return piIter;
    }
    else
    {
        return NULL;
    }
}


_DWFTK_API
DWFString
OPCCoreProperties::provideKeywords( const DWFString& zValue )
throw()
{
    _oOPCProperties.insert( std::make_pair(OPCXML::kzCoreProperty_Keywords, zValue) );
    return zValue;
}

_DWFTK_API
DWFString
OPCCoreProperties::provideContentType( const DWFString& zValue )
throw()
{
    _oOPCProperties.insert( std::make_pair(OPCXML::kzCoreProperty_ContentType, zValue) );
    return zValue;
}

_DWFTK_API
DWFString
OPCCoreProperties::provideCategory( const DWFString& zValue )
throw()
{
    _oOPCProperties.insert( std::make_pair(OPCXML::kzCoreProperty_Category, zValue) );
    return zValue;
}

_DWFTK_API
DWFString
OPCCoreProperties::provideVersion( const DWFString& zValue )
throw()
{
    _oOPCProperties.insert( std::make_pair(OPCXML::kzCoreProperty_Version, zValue) );
    return zValue;
}

_DWFTK_API
DWFString
OPCCoreProperties::provideRevision( const DWFString& zValue )
throw()
{
    _oOPCProperties.insert( std::make_pair(OPCXML::kzCoreProperty_Revision, zValue) );
    return zValue;
}

_DWFTK_API
DWFString
OPCCoreProperties::provideLastModifiedBy( const DWFString& zValue )
throw()
{
    _oOPCProperties.insert( std::make_pair(OPCXML::kzCoreProperty_LastModifiedBy, zValue) );
    return zValue;
}

_DWFTK_API
DWFString
OPCCoreProperties::provideLastPrinted( const DWFString& zValue )
throw()
{
    _oOPCProperties.insert( std::make_pair(OPCXML::kzCoreProperty_LastPrinted, zValue) );
    return zValue;
}

_DWFTK_API
DWFString
OPCCoreProperties::provideContentStatus( const DWFString& zValue )
throw()
{
    _oOPCProperties.insert( std::make_pair(OPCXML::kzCoreProperty_ContentStatus, zValue) );
    return zValue;
}

_DWFTK_API
DWFString
OPCCoreProperties::provideCreator( const DWFString& zValue )
throw()
{
    _oDCProperties.insert( std::make_pair(OPCXML::kzCoreProperty_Creator, zValue) );
    return zValue;
}

_DWFTK_API
DWFString
OPCCoreProperties::provideIdentifier( const DWFString& zValue )
throw()
{
    _oDCProperties.insert( std::make_pair(OPCXML::kzCoreProperty_Identifier, zValue) );
    return zValue;
}

_DWFTK_API
DWFString
OPCCoreProperties::provideTitle( const DWFString& zValue )
throw()
{
    _oDCProperties.insert( std::make_pair(OPCXML::kzCoreProperty_Title, zValue) );
    return zValue;
}

_DWFTK_API
DWFString
OPCCoreProperties::provideSubject( const DWFString& zValue )
throw()
{
    _oDCProperties.insert( std::make_pair(OPCXML::kzCoreProperty_Subject, zValue) );
    return zValue;
}

_DWFTK_API
DWFString
OPCCoreProperties::provideDescription( const DWFString& zValue )
throw()
{
    _oDCProperties.insert( std::make_pair(OPCXML::kzCoreProperty_Description, zValue) );
    return zValue;
}

_DWFTK_API
DWFString
OPCCoreProperties::provideLanguage( const DWFString& zValue )
throw()
{
    _oDCProperties.insert( std::make_pair(OPCXML::kzCoreProperty_Language, zValue) );
    return zValue;
}

_DWFTK_API
DWFString
OPCCoreProperties::provideCreated( const DWFString& zValue )
throw()
{
    _oDCTermsProperties.insert( std::make_pair(OPCXML::kzCoreProperty_Created, zValue) );
    return zValue;
}

_DWFTK_API
DWFString
OPCCoreProperties::provideModified( const DWFString& zValue )
throw()
{
    _oDCTermsProperties.insert( std::make_pair(OPCXML::kzCoreProperty_Modified, zValue) );
    return zValue;
}


#ifndef DWFTK_READ_ONLY

_DWFTK_API
void OPCCoreProperties::serializeXML( DWFXMLSerializer& rSerializer )
throw( DWFException )
{
    rSerializer.emitXMLHeader();

    rSerializer.startElement( OPCXML::kzElement_CoreProperties );

    rSerializer.addAttribute( L"xmlns", OPCXML::kzNamespaceURI_CoreProperties );

    if (_oDCProperties.size()>0)
    {
        DWFString zTmp( OPCXML::kzNamespace_DublinCore );
        if (zTmp.substring( zTmp.chars()-1 ) == L":")
        {
            zTmp = zTmp.substring( 0, zTmp.chars()-1 );
        }
        rSerializer.addAttribute( zTmp, OPCXML::kzNamespaceURI_DublinCore, L"xmlns:" );
    }

    if (_oDCTermsProperties.size()>0)
    {
        DWFString zTmp( OPCXML::kzNamespace_DublinCoreTerms );
        if (zTmp.substring( zTmp.chars()-1 ) == L":")
        {
            zTmp = zTmp.substring( 0, zTmp.chars()-1 );
        }
        rSerializer.addAttribute( zTmp, OPCXML::kzNamespaceURI_DublinCoreTerms, L"xmlns:" );

        zTmp.assign( OPCXML::kzNamespace_XSI );
        if (zTmp.substring( zTmp.chars()-1 ) == L":")
        {
            zTmp = zTmp.substring( 0, zTmp.chars()-1 );
        }
        rSerializer.addAttribute( zTmp, OPCXML::kzNamespaceURI_XSI, L"xmlns:" );
    }


    _tPropertyMap::iterator it = _oOPCProperties.begin();
    for (; it != _oOPCProperties.end(); ++it)
    {
        rSerializer.startElement( it->first );
        rSerializer.addCData( it->second );
        rSerializer.endElement();
    }

    it = _oDCProperties.begin();
    for (; it != _oDCProperties.end(); ++it)
    {
        rSerializer.startElement( it->first, OPCXML::kzNamespace_DublinCore );
        rSerializer.addCData( it->second );
        rSerializer.endElement();
    }

    it = _oDCTermsProperties.begin();
    for (; it != _oDCTermsProperties.end(); ++it)
    {
        rSerializer.startElement( it->first, OPCXML::kzNamespace_DublinCoreTerms );
        rSerializer.addAttribute( L"type", L"dcterms:W3CDTF", OPCXML::kzNamespace_XSI );
        rSerializer.addCData( it->second );
        rSerializer.endElement();
    }

    rSerializer.endElement();
}

#endif

DWFString
OPCCoreProperties::_getDateString( int nYear, int nMonth, int nDay ) const
throw( DWFException )
{
    if ((nMonth < 1 || nMonth > 12) ||
        (nDay < 1 || nDay > 31))
    {
        _DWFCORE_THROW( DWFInvalidArgumentException, L"Invalid value for month or day." );
    }

    if (nMonth == 2 && nDay > 29)
    {
        _DWFCORE_THROW( DWFInvalidArgumentException, L"Invalid value for day in February." );
    }

    if ((nDay>30) &&
        (nMonth == 4 ||
         nMonth == 6 ||
         nMonth == 9 ||
         nMonth == 11)
       )
    {
        _DWFCORE_THROW( DWFInvalidArgumentException, L"Invalid value for day." );
    }

    int nBufferSize = 16;
    DWFPointer<wchar_t> zBuffer( DWFCORE_ALLOC_MEMORY(wchar_t, nBufferSize), true );

    int nBytes = sizeof(wchar_t) * _DWFCORE_SWPRINTF( zBuffer, nBufferSize, L"%d", nYear );
    DWFString zTmp( zBuffer, nBytes );
    zTmp.append( L"-" );
    nBytes = sizeof(wchar_t) * _DWFCORE_SWPRINTF( zBuffer, nBufferSize, L"%d", nMonth );
    zTmp.append( zBuffer, nBytes );
    zTmp.append( L"-" );
    nBytes = sizeof(wchar_t) * _DWFCORE_SWPRINTF( zBuffer, nBufferSize, L"%d", nDay );
    zTmp.append( zBuffer, nBytes );
    
//<TODO>
// Remove this hack once MS fixes their bug with the Date format validation.
//
    zTmp.append( L"T00:00:00" );

    return zTmp;
}


//DNT_End

