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
//  $Header: //DWF/Development/Components/Internal/DWF Toolkit/v7.7/develop/global/src/dwf/package/reader/ContentResourceReader.cpp#1 $
//  $DateTime: 2011/02/14 01:16:30 $
//  $Author: caos $
//  $Change: 197964 $
//  $Revision: #1 $
//

#include "dwf/package/Instance.h"
#include "dwf/package/Constants.h"
#include "dwf/package/reader/ContentResourceReader.h"
using namespace DWFToolkit;

_DWFTK_API
DWFContentResourceReader::DWFContentResourceReader( unsigned int nProviderFlags )
throw()
                                                  : DWFXMLCallback()
                                                  , _oDefaultElementBuilder()
                                                  , _pElementBuilder( NULL )
                                                  , _nCurrentCollectionProvider( eProvideNone )
                                                  , _nProviderFlags( nProviderFlags )
                                                  , _pCurrentInstance( NULL )
                                                  , _pFilter( NULL )
                                                  , _zRenderableID( /*NOXLATE*/L"" )
                                                  , _zResourceObjectID( /*NOXLATE*/L"" )
{
    _pElementBuilder = &_oDefaultElementBuilder;
}

_DWFTK_API
DWFContentResourceReader::~DWFContentResourceReader()
throw()
{
}

_DWFTK_API
void
DWFContentResourceReader::setResourceObjectID( const DWFString& zObjectID )
throw()
{
    _zResourceObjectID = zObjectID;

    if (_pFilter)
    {
        _pFilter->setResourceObjectID( zObjectID );
    }
}

_DWFTK_API
void
DWFContentResourceReader::setFilter( DWFContentResourceReader* pFilter )
throw()
{
    _pFilter = pFilter;

    if (_pFilter)
    {
        _pFilter->setResourceObjectID( _zResourceObjectID );
    }
}

////

_DWFTK_API
void
DWFContentResourceReader::notifyStartElement( const char*   zName,
                                              const char**  ppAttributeList )
throw()
{
    //
    // skip over any "dwf:" in the element name
    //
    if (DWFCORE_COMPARE_MEMORY(DWFXML::kzNamespace_DWF, zName, 4) == 0)
    {
        zName+=4;
    }

    switch (_nElementDepth)
    {
        case 0:
        {
            if ((DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_SectionContent) == 0) ||
                (DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_ContentResource) == 0))
            {
                if (_nProviderFlags & eProvideAttributes)
                {
                    unsigned char nFound = 0;
                    size_t iAttrib = 0;
                    const char* pAttrib = NULL;

                    for(; ppAttributeList[iAttrib]; iAttrib += 2)
                    {
                        pAttrib = &ppAttributeList[iAttrib][0];

                        //
                        // provide the content resource version
                        //
                        if ((_nProviderFlags & eProvideVersion) &&
                            !(nFound & eProvideVersion)         &&
                             (DWFCORE_COMPARE_ASCII_STRINGS(pAttrib, DWFXML::kzAttribute_Version) == 0))
                        {
                            nFound |= eProvideVersion;
                            _provideContentResourceVersion( ppAttributeList[iAttrib+1] );
                        }
                    }
                }
            }
            else
            {
                //
                // we have an invalid content here, turn off all subsequent
                // processing and indicate the error somewhere...
                //
                _nProviderFlags = eProvideNone;
            }

            break;
        }

        case 1:
        {
            if (DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_Instances) == 0)
            {
                _nCurrentCollectionProvider = eProvideInstances;
            }
            break;
        }

        case 2:
        {
            if (DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_Instance) == 0)
            {
                _pCurrentInstance = _pElementBuilder->buildInstance( ppAttributeList );
                _zRenderableID = _findAttributeValue( DWFXML::kzAttribute_RenderableRef, ppAttributeList );
            }
            break;
        }
        default:
        {
            ;
        }
    }

    _nElementDepth++;
}

_DWFTK_API
void
DWFContentResourceReader::notifyEndElement( const char* zName )
throw()
{
    //
    //  Skip over any "dwf:" in the element name
    //
    if (DWFCORE_COMPARE_MEMORY(DWFXML::kzNamespace_DWF, zName, 4) == 0)
    {
        zName+=4;
    }

    //
    //  Perform the decrement before testing so the element closing depth matches the opening depth
    //
    switch (--_nElementDepth)
    {
        case 0:
        {
            //
            //  Reset
            //
            _nCurrentCollectionProvider = eProvideNone;
            _nProviderFlags = eProvideAll;
            _pCurrentInstance = NULL;
            _zRenderableID.assign( /*NOXLATE*/L"" );
            _zResourceObjectID.assign( /*NOXLATE*/L"" );

            break;
        }
        case 1:
        {
            if (DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_Instances) == 0)
            {
                resolveInstances();
            }

            _nCurrentCollectionProvider = eProvideNone;

            break;
        }
        case 2:
        {
            _provideInstance( _pCurrentInstance );
            _pCurrentInstance = NULL;

            break;
        }
        default:
        {
            ;
        }
    };
}

_DWFTK_API
const char*
DWFContentResourceReader::provideContentResourceVersion( const char* zVersion )
throw( DWFException )
{
    //
    //  These methods do real work in the derived class
    //
    return zVersion;
}

_DWFTK_API
DWFInstance*
DWFContentResourceReader::provideInstance( DWFInstance* pInstance,
                                           const DWFString& /*zRenderableID*/,
                                           const DWFString& /*zResourceObjectID*/ )
throw( DWFException )
{
    //
    //  These methods do real work in the derived class
    //
    return pInstance;
}

_DWFTK_API
void
DWFContentResourceReader::_provideContentResourceVersion( const char* zVersion )
throw( DWFException )
{
    provideContentResourceVersion( _pFilter ? _pFilter->provideContentResourceVersion( zVersion ) : zVersion );
}

_DWFTK_API
void
DWFContentResourceReader::_provideInstance( DWFInstance* pInstance )
throw( DWFException )
{
    if (_zResourceObjectID.chars() == 0)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"The object ID of the resource needs to be set on the reader before using it." );
    }

    if (_pFilter)
    {
        pInstance = _pFilter->provideInstance( pInstance, _zRenderableID, _zResourceObjectID );
    }
    provideInstance( pInstance, _zRenderableID, _zResourceObjectID );
}

_DWFTK_API
DWFString
DWFContentResourceReader::_findAttributeValue( const char*  pAttributeName,
                                               const char** ppAttributeList )
throw()
{
    size_t iAttrib = 0;
    for(; ppAttributeList[iAttrib]; iAttrib += 2)
    {
        if (DWFCORE_COMPARE_ASCII_STRINGS(pAttributeName, ppAttributeList[iAttrib]) == 0)
        {
            return DWFString( ppAttributeList[iAttrib+1] );
            break;
        }
    }

    return /*NOXLATE*/L"";
}
