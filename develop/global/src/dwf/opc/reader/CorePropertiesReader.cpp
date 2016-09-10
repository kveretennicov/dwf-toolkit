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
//  $Header: //DWF/Development/Components/Internal/DWF Toolkit/v7.7/develop/global/src/dwf/opc/reader/CorePropertiesReader.cpp#1 $
//  $DateTime: 2011/02/14 01:16:30 $
//  $Author: caos $
//  $Change: 197964 $
//  $Revision: #1 $
//
//


#include "dwf/opc/reader/CorePropertiesReader.h"
#include "dwf/opc/Constants.h"
using namespace DWFToolkit;

//DNT_Start

_DWFTK_API
OPCCorePropertiesReader::OPCCorePropertiesReader( unsigned int nProviderFlags )
throw()
                              : _oDefaultElementBuilder()
                              , _pElementBuilder( &_oDefaultElementBuilder )
                              , _nProviderFlags( nProviderFlags )
                              , _pFilter( NULL )
                              , _zCDataAccumulator()
{
    ;
}

_DWFTK_API
OPCCorePropertiesReader::~OPCCorePropertiesReader()
throw()
{
    ;
}

_DWFTK_API
OPCCorePropertiesReader*
OPCCorePropertiesReader::filter() const
throw()
{
    return _pFilter;
}

_DWFTK_API
void
OPCCorePropertiesReader::setFilter( OPCCorePropertiesReader* pFilter )
throw()
{
    _pFilter = pFilter;
}

_DWFTK_API
void
OPCCorePropertiesReader::notifyStartElement( const char*  zName,
                                             const char** /*ppAttributeList*/ )
throw()
{
    if ((_nProviderFlags & eProvideProperties) && 
        (DWFCORE_COMPARE_ASCII_STRINGS(zName, OPCXML::kzCoreProperty_Keywords) == 0 ||
         DWFCORE_COMPARE_ASCII_STRINGS(zName, OPCXML::kzCoreProperty_ContentType) == 0 ||
         DWFCORE_COMPARE_ASCII_STRINGS(zName, OPCXML::kzCoreProperty_Category) == 0 ||
         DWFCORE_COMPARE_ASCII_STRINGS(zName, OPCXML::kzCoreProperty_Version) == 0 ||
         DWFCORE_COMPARE_ASCII_STRINGS(zName, OPCXML::kzCoreProperty_Revision) == 0 ||
         DWFCORE_COMPARE_ASCII_STRINGS(zName, OPCXML::kzCoreProperty_LastModifiedBy) == 0 ||
         DWFCORE_COMPARE_ASCII_STRINGS(zName, OPCXML::kzCoreProperty_LastPrinted) == 0 ||
         DWFCORE_COMPARE_ASCII_STRINGS(zName, OPCXML::kzCoreProperty_ContentStatus) == 0 ||
         DWFCORE_COMPARE_ASCII_STRINGS(zName, OPCXML::kzCoreProperty_Creator) == 0 ||
         DWFCORE_COMPARE_ASCII_STRINGS(zName, OPCXML::kzCoreProperty_Identifier) == 0 ||
         DWFCORE_COMPARE_ASCII_STRINGS(zName, OPCXML::kzCoreProperty_Title) == 0 ||
         DWFCORE_COMPARE_ASCII_STRINGS(zName, OPCXML::kzCoreProperty_Subject) == 0 ||
         DWFCORE_COMPARE_ASCII_STRINGS(zName, OPCXML::kzCoreProperty_Description) == 0 ||
         DWFCORE_COMPARE_ASCII_STRINGS(zName, OPCXML::kzCoreProperty_Language) == 0 ||
         DWFCORE_COMPARE_ASCII_STRINGS(zName, OPCXML::kzCoreProperty_Created) == 0 ||
         DWFCORE_COMPARE_ASCII_STRINGS(zName, OPCXML::kzCoreProperty_Modified) == 0))
    {
        _zCDataAccumulator.assign(L"");
    }
}

_DWFTK_API
void
OPCCorePropertiesReader::notifyEndElement( const char* zName )
throw()
{
    if (DWFCORE_COMPARE_ASCII_STRINGS(zName, OPCXML::kzCoreProperty_Keywords) == 0)
    {
        provideKeywords( _zCDataAccumulator );
        _zCDataAccumulator.assign(L"");
    }

    else if (DWFCORE_COMPARE_ASCII_STRINGS(zName, OPCXML::kzCoreProperty_ContentType) == 0)
    {
        provideContentType( _zCDataAccumulator );
        _zCDataAccumulator.assign(L"");
    }

    else if (DWFCORE_COMPARE_ASCII_STRINGS(zName, OPCXML::kzCoreProperty_Category) == 0)
    {
        provideCategory( _zCDataAccumulator );
        _zCDataAccumulator.assign(L"");
    }

    else if (DWFCORE_COMPARE_ASCII_STRINGS(zName, OPCXML::kzCoreProperty_Version) == 0)
    {
        provideVersion( _zCDataAccumulator );
        _zCDataAccumulator.assign(L"");
    }

    else if (DWFCORE_COMPARE_ASCII_STRINGS(zName, OPCXML::kzCoreProperty_Revision) == 0)
    {
        provideRevision( _zCDataAccumulator );
        _zCDataAccumulator.assign(L"");
    }

    else if (DWFCORE_COMPARE_ASCII_STRINGS(zName, OPCXML::kzCoreProperty_LastModifiedBy) == 0)
    {
        provideLastModifiedBy( _zCDataAccumulator );
        _zCDataAccumulator.assign(L"");
    }

    else if (DWFCORE_COMPARE_ASCII_STRINGS(zName, OPCXML::kzCoreProperty_LastPrinted) == 0)
    {
        provideLastPrinted( _zCDataAccumulator );
        _zCDataAccumulator.assign(L"");
    }

    else if (DWFCORE_COMPARE_ASCII_STRINGS(zName, OPCXML::kzCoreProperty_ContentStatus) == 0)
    {
        provideContentStatus( _zCDataAccumulator );
        _zCDataAccumulator.assign(L"");
    }

    else if (DWFCORE_COMPARE_ASCII_STRINGS(zName, OPCXML::kzCoreProperty_Creator) == 0)
    {
        provideCreator( _zCDataAccumulator );
        _zCDataAccumulator.assign(L"");
    }

    else if (DWFCORE_COMPARE_ASCII_STRINGS(zName, OPCXML::kzCoreProperty_Identifier) == 0)
    {
        provideIdentifier( _zCDataAccumulator );
        _zCDataAccumulator.assign(L"");
    }

    else if (DWFCORE_COMPARE_ASCII_STRINGS(zName, OPCXML::kzCoreProperty_Title) == 0)
    {
        provideTitle( _zCDataAccumulator );
        _zCDataAccumulator.assign(L"");
    }

    else if (DWFCORE_COMPARE_ASCII_STRINGS(zName, OPCXML::kzCoreProperty_Subject) == 0)
    {
        provideSubject( _zCDataAccumulator );
        _zCDataAccumulator.assign(L"");
    }

    else if (DWFCORE_COMPARE_ASCII_STRINGS(zName, OPCXML::kzCoreProperty_Description) == 0)
    {
        provideDescription( _zCDataAccumulator );
        _zCDataAccumulator.assign(L"");
    }

    else if (DWFCORE_COMPARE_ASCII_STRINGS(zName, OPCXML::kzCoreProperty_Language) == 0)
    {
        provideLanguage( _zCDataAccumulator );
        _zCDataAccumulator.assign(L"");
    }

    else if (DWFCORE_COMPARE_ASCII_STRINGS(zName, OPCXML::kzCoreProperty_Created) == 0)
    {
        provideCreated( _zCDataAccumulator );
        _zCDataAccumulator.assign(L"");
    }

    else if (DWFCORE_COMPARE_ASCII_STRINGS(zName, OPCXML::kzCoreProperty_Modified) == 0)
    {
        provideModified( _zCDataAccumulator );
        _zCDataAccumulator.assign(L"");
    }

}

_DWFTK_API
void
OPCCorePropertiesReader::notifyStartNamespace( const char* /*zPrefix*/,
                                               const char* /*zURI*/ )
throw()
{
}

_DWFTK_API
void
OPCCorePropertiesReader::notifyEndNamespace( const char* /*zPrefix*/ )
throw()
{
}

_DWFTK_API
void
OPCCorePropertiesReader::notifyCharacterData( const char* zCData, 
                                              int         nLength ) 
throw()
{
    _zCDataAccumulator.append(zCData, nLength);
}


_DWFTK_API
DWFString
OPCCorePropertiesReader::provideKeywords( const DWFString& zValue )
throw()
{
    return zValue;
}

_DWFTK_API
DWFString
OPCCorePropertiesReader::provideContentType( const DWFString& zValue )
throw()
{
    return zValue;
}

_DWFTK_API
DWFString
OPCCorePropertiesReader::provideCategory( const DWFString& zValue )
throw()
{
    return zValue;
}

_DWFTK_API
DWFString
OPCCorePropertiesReader::provideVersion( const DWFString& zValue )
throw()
{
    return zValue;
}

_DWFTK_API
DWFString
OPCCorePropertiesReader::provideRevision( const DWFString& zValue )
throw()
{
    return zValue;
}

_DWFTK_API
DWFString
OPCCorePropertiesReader::provideLastModifiedBy( const DWFString& zValue )
throw()
{
    return zValue;
}

_DWFTK_API
DWFString
OPCCorePropertiesReader::provideLastPrinted( const DWFString& zValue )
throw()
{
    return zValue;
}

_DWFTK_API
DWFString
OPCCorePropertiesReader::provideContentStatus( const DWFString& zValue )
throw()
{
    return zValue;
}

_DWFTK_API
DWFString
OPCCorePropertiesReader::provideCreator( const DWFString& zValue )
throw()
{
    return zValue;
}

_DWFTK_API
DWFString
OPCCorePropertiesReader::provideIdentifier( const DWFString& zValue )
throw()
{
    return zValue;
}

_DWFTK_API
DWFString
OPCCorePropertiesReader::provideTitle( const DWFString& zValue )
throw()
{
    return zValue;
}

_DWFTK_API
DWFString
OPCCorePropertiesReader::provideSubject( const DWFString& zValue )
throw()
{
    return zValue;
}

_DWFTK_API
DWFString
OPCCorePropertiesReader::provideDescription( const DWFString& zValue )
throw()
{
    return zValue;
}

_DWFTK_API
DWFString
OPCCorePropertiesReader::provideLanguage( const DWFString& zValue )
throw()
{
    return zValue;
}

_DWFTK_API
DWFString
OPCCorePropertiesReader::provideCreated( const DWFString& zValue )
throw()
{
    return zValue;
}

_DWFTK_API
DWFString
OPCCorePropertiesReader::provideModified( const DWFString& zValue )
throw()
{
    return zValue;
}

//DNT_End

