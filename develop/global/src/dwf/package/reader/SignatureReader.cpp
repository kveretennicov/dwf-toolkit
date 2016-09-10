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
//  $Header: //DWF/Development/Components/Internal/DWF Toolkit/v7.7/develop/global/src/dwf/package/reader/SignatureReader.cpp#1 $
//  $DateTime: 2011/02/14 01:16:30 $
//  $Author: caos $
//  $Change: 197964 $
//  $Revision: #1 $
//

#include "dwf/package/Instance.h"
#include "dwf/package/Constants.h"
#include "dwf/package/reader/SignatureReader.h"
using namespace DWFToolkit;

_DWFTK_API
DWFSignatureReader::DWFSignatureReader( DWFPackageReader* pPackageReader,
                                        unsigned int      nProviderFlags )
throw()
                                                  : DWFXMLCallback()
                                                  , _pPackageReader( pPackageReader )
                                                  , _oDefaultElementBuilder()
                                                  , _pElementBuilder( NULL )
                                                  , _nCurrentCollectionProvider( eProvideNone )
                                                  , _nProviderFlags( nProviderFlags )
                                                  , _pFilter( NULL )
                                                  , _zResourceObjectID( L"" )
                                                  , _zCDataAccumulator( L"" )
                                                  , _oCurrentDSAKeyValueStrings()
                                                  , _oCurrentRSAKeyValueStrings()
                                                  , _pCurrentX509Data( NULL )
                                                  , _pCurrentX509IssuerSerial( NULL )
{
    _pElementBuilder = &_oDefaultElementBuilder;
}

_DWFTK_API
DWFSignatureReader::~DWFSignatureReader()
throw()
{
}

_DWFTK_API
void 
DWFSignatureReader::setResourceObjectID( const DWFString& zObjectID )
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
DWFSignatureReader::setFilter( DWFSignatureReader* pFilter )
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
DWFSignatureReader::notifyStartElement( const char*   zName,
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
        //
        // skip over any "signatures:" in the element name
        //
    else if (DWFCORE_COMPARE_MEMORY(DWFXML::kzNamespace_Signatures, zName, 11) == 0)
    {
        zName+=11;
    }


    switch (_nElementDepth)
    {
        case 0:
        {
            if (DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_Signature ) == 0)
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
                        // provide the ID
                        //
                        if ((_nProviderFlags & eProvideSignatureID) &&
                            !(nFound & eProvideSignatureID)         &&
                             (DWFCORE_COMPARE_ASCII_STRINGS(pAttrib, DWFXML::kzAttribute_ID) == 0))
                        {
                            nFound |= eProvideSignatureID;
                            _provideSignatureID( ppAttributeList[iAttrib+1] );
                        }
                    }
                }
            }
            else
            {
                //
                // we have an invalid signature here, turn off all subsequent
                // processing and indicate the error somewhere...
                //
                _nProviderFlags = eProvideNone;
            }

            break;
        }

        case 1:
        {
            if (DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_SignedInfo) == 0)
            {
                // ...
            }
            else if (DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_SignatureValue) == 0)
            {
                // ...
            }
            else if (DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_KeyInfo) == 0)
            {
                // ...
            }
            break;
        }

        case 2:
        {
            if (DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_X509Data) == 0)
            {
                _pCurrentX509Data = DWFCORE_ALLOC_OBJECT(X509Data);
            }
            else if (DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_KeyName) == 0)
            {
                // ...
            }
            else if (DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_KeyValue) == 0)
            {
                // ...
            }
            else if (DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_CanonicalizationMethod) == 0)
            {
                // ...
            }
            else if (DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_SignatureMethod) == 0)
            {
                const DWFString & algorithm = _findAttributeValue( DWFXML::kzAttribute_Algorithm, ppAttributeList );
                _provideSignatureMethod(algorithm);
            }
            else if (DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_Reference) == 0)
            {
                const DWFString & uri = _findAttributeValue( DWFXML::kzAttribute_URI, ppAttributeList );
                _provideReference(uri);
            }
            break;
        }

        case 3:
        {
            if (DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_X509IssuerSerial) == 0)
            {
                _pCurrentX509IssuerSerial = DWFCORE_ALLOC_OBJECT(X509IssuerSerial);
            }
            else if (DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_X509SKI) == 0)
            {
                // Handled in notifyEndElement
            }
            else if (DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_X509SubjectName) == 0)
            {
                // Handled in notifyEndElement
            }
            else if (DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_X509Certificate) == 0)
            {
                // Handled in notifyEndElement
            }
            else if (DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_X509CRL) == 0)
            {
                // Handled in notifyEndElement
            }
            else if (DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_RSAKeyValue) == 0)
            {
                // Handled in notifyEndElement
            }
            else if (DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_DSAKeyValue) == 0)
            {
                // Handled in notifyEndElement
            }
            else if (DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_DigestMethod) == 0)
            {
                const DWFString & algorithm = _findAttributeValue( DWFXML::kzAttribute_Algorithm, ppAttributeList );
                _provideDigestMethod(algorithm);
            }
            else if (DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_DigestValue) == 0)
            {
                // Handled in notifyEndElement
            }
            break;
        }

        case 4:
        {
            if (DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_X509IssuerName) == 0)
            {
                // Handled in notifyEndElement
            }
            else if (DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_X509SerialNumber) == 0)
            {
                // Handled in notifyEndElement
            }
            else if (DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_RSAKeyValueModulus) == 0)
            {
                // Handled in notifyEndElement
            }
            else if (DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_RSAKeyValueExponent) == 0)
            {
                // Handled in notifyEndElement
            }
            else if (DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_DSAKeyValueP) == 0)
            {
                // Handled in notifyEndElement
            }
            else if (DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_DSAKeyValueQ) == 0)
            {
                // Handled in notifyEndElement
            }
            else if (DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_DSAKeyValueG) == 0)
            {
                // Handled in notifyEndElement
            }
            else if (DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_DSAKeyValueY) == 0)
            {
                // Handled in notifyEndElement
            }
            else if (DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_DSAKeyValueJ) == 0)
            {
                // Handled in notifyEndElement
            }
            else if (DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_DSAKeyValueSeed) == 0)
            {
                // Handled in notifyEndElement
            }
            else if (DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_DSAKeyValuePgenCounter) == 0)
            {
                // Handled in notifyEndElement
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
DWFSignatureReader::notifyCharacterData( const char* zCData,
                                         int         nLength )
    throw()
{
    _zCDataAccumulator.append(zCData, nLength);
}


_DWFTK_API
void
DWFSignatureReader::notifyEndElement( const char* zName )
throw()
{
        //
        // skip over any "dwf:" in the element name
        //
    if (DWFCORE_COMPARE_MEMORY(DWFXML::kzNamespace_DWF, zName, 4) == 0)
    {
        zName+=4;
    }
        //
        // skip over any "signatures:" in the element name
        //
    else if (DWFCORE_COMPARE_MEMORY(DWFXML::kzNamespace_Signatures, zName, 11) == 0)
    {
        zName+=11;
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
            _zResourceObjectID.assign( L"" );

            break;
        }
        case 1:
        {
            if (DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_SignedInfo) == 0)
            {
                // ...
            }
            else if (DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_SignatureValue) == 0)
            {
                _provideSignatureValue(_zCDataAccumulator);
            }
            else if (DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_KeyInfo) == 0)
            {
                // ...
            }
            break;
        }

        case 2:
        {
            if (DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_X509Data) == 0)
            {
                _provideX509Data(_pCurrentX509Data);
                _pCurrentX509Data = NULL;
            }
            else if (DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_KeyName) == 0)
            {
                _provideKeyName(_zCDataAccumulator);
            }
            else if (DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_KeyValue) == 0)
            {
                // ...
            }
            else if (DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_CanonicalizationMethod) == 0)
            {
                // ...
            }
            else if (DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_SignatureMethod) == 0)
            {
                // ...
            }
            else if (DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_Reference) == 0)
            {
                // ...
            }
            break;
        }

        case 3:
        {
            if (DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_X509IssuerSerial) == 0)
            {
                if( (_pCurrentX509Data != NULL) && (_pCurrentX509IssuerSerial != NULL) )
                {
                    _pCurrentX509Data->addDataItem(_pCurrentX509IssuerSerial);
                }
            }
            else if (DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_X509SKI) == 0)
            {
                if(_pCurrentX509Data != NULL)
                {
                    X509SKI *pX509SKI = DWFCORE_ALLOC_OBJECT(X509SKI);
                    pX509SKI->setSubjectKeyIdentifier(_zCDataAccumulator);
                    _pCurrentX509Data->addDataItem(pX509SKI);
                }
            }
            else if (DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_X509SubjectName) == 0)
            {
                if(_pCurrentX509Data != NULL)
                {
                    X509SubjectName *pX509SubjectName = DWFCORE_ALLOC_OBJECT(X509SubjectName);
                    pX509SubjectName->setSubjectName(_zCDataAccumulator);
                    _pCurrentX509Data->addDataItem(pX509SubjectName);
                }
            }
            else if (DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_X509Certificate) == 0)
            {
                if(_pCurrentX509Data != NULL)
                {
                    X509Certificate *pX509Certificate = DWFCORE_ALLOC_OBJECT(X509Certificate);
                    pX509Certificate->setCertificate(_zCDataAccumulator);
                    _pCurrentX509Data->addDataItem(pX509Certificate);
                }
            }
            else if (DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_X509CRL) == 0)
            {
                if(_pCurrentX509Data != NULL)
                {
                    X509CRL *pX509CRL = DWFCORE_ALLOC_OBJECT(X509CRL);
                    pX509CRL->setCRL(_zCDataAccumulator);
                    _pCurrentX509Data->addDataItem(pX509CRL);
                }
            }
            else if (DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_RSAKeyValue) == 0)
            {
                _provideRSAKeyValueStrings(_oCurrentRSAKeyValueStrings);
            }
            else if (DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_DSAKeyValue) == 0)
            {
                _provideDSAKeyValueStrings(_oCurrentDSAKeyValueStrings);
            }
            else if (DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_DigestMethod) == 0)
            {
            }
            else if (DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_DigestValue) == 0)
            {
                _provideDigestValue(_zCDataAccumulator);
            }
            break;
        }

        case 4:
        {
            if (DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_X509IssuerName) == 0)
            {
                if(_pCurrentX509IssuerSerial != NULL)
                {
                    _pCurrentX509IssuerSerial->setIssuerName(_zCDataAccumulator);
                }
            }
            else if (DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_X509SerialNumber) == 0)
            {
                if(_pCurrentX509IssuerSerial != NULL)
                {
                    char* pUTF8(NULL);
                    _zCDataAccumulator.getUTF8( &pUTF8 );

                    long sn = ::atol(pUTF8);

                    DWFCORE_FREE_MEMORY( pUTF8 );

                    _pCurrentX509IssuerSerial->setSerialNumber(sn);
                }
            }
            else if (DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_RSAKeyValueModulus) == 0)
            {
                _oCurrentRSAKeyValueStrings._zModulus = _zCDataAccumulator;
            }
            else if (DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_RSAKeyValueExponent) == 0)
            {
                _oCurrentRSAKeyValueStrings._zExponent = _zCDataAccumulator;
            }
            else if (DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_DSAKeyValueP) == 0)
            {
                _oCurrentDSAKeyValueStrings._zP = _zCDataAccumulator;
            }
            else if (DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_DSAKeyValueQ) == 0)
            {
                _oCurrentDSAKeyValueStrings._zQ = _zCDataAccumulator;
            }
            else if (DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_DSAKeyValueG) == 0)
            {
                _oCurrentDSAKeyValueStrings._zG = _zCDataAccumulator;
            }
            else if (DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_DSAKeyValueY) == 0)
            {
                _oCurrentDSAKeyValueStrings._zY = _zCDataAccumulator;
            }
            else if (DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_DSAKeyValueJ) == 0)
            {
                _oCurrentDSAKeyValueStrings._zJ = _zCDataAccumulator;
            }
            else if (DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_DSAKeyValueSeed) == 0)
            {
                _oCurrentDSAKeyValueStrings._zSeed = _zCDataAccumulator;
            }
            else if (DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_DSAKeyValuePgenCounter) == 0)
            {
                _oCurrentDSAKeyValueStrings._zPgenCounter = _zCDataAccumulator;
            }
            break;
        }

        default:
        {
            ;
        }

    };

    // Every time an element ends, if we haven't grabbed any accumulated CData, blow it away.
    _zCDataAccumulator.assign(L"");
}

_DWFTK_API
const char*
DWFSignatureReader::provideSignatureID( const char* zId )
throw( DWFException )
{
    //
    //  These methods do real work in the derived class
    //
    return zId;
}

_DWFTK_API
const DWFString&
DWFSignatureReader::provideReference( const DWFString& zReferenceURI )
throw( DWFException )
{
    //
    //  These methods do real work in the derived class
    //
    return zReferenceURI;
}

_DWFTK_API
const DWFString&
DWFSignatureReader::provideSignatureMethod( const DWFString& zSigMethod )
throw( DWFException )
{
    //
    //  These methods do real work in the derived class
    //
    return zSigMethod;
}

_DWFTK_API
const DWFString&
DWFSignatureReader::provideSignatureValue( const DWFString& zSigValue )
throw( DWFException )
{
    //
    //  These methods do real work in the derived class
    //
    return zSigValue;
}

_DWFTK_API
X509Data *
DWFSignatureReader::provideX509Data( X509Data *pX509Data )
throw( DWFException )
{
    //
    //  These methods do real work in the derived class
    //
    return pX509Data;
}

_DWFTK_API
const DWFString&
DWFSignatureReader::provideDigestMethod( const DWFString& zDigestMethod )
throw( DWFException )
{
    //
    //  These methods do real work in the derived class
    //
    return zDigestMethod;
}

_DWFTK_API
const DWFString&
DWFSignatureReader::provideDigestValue( const DWFString& zDigestValue )
throw( DWFException )
{
    //
    //  These methods do real work in the derived class
    //
    return zDigestValue;
}

_DWFTK_API
const DWFSignatureReader::RSAKeyValueStrings&
DWFSignatureReader::provideRSAKeyValueStrings(  const DWFSignatureReader::RSAKeyValueStrings& rRSAKeyValueStrings )
throw( DWFException )
{
    //
    //  These methods do real work in the derived class
    //
    return rRSAKeyValueStrings;
}

_DWFTK_API
const DWFSignatureReader::DSAKeyValueStrings&
DWFSignatureReader::provideDSAKeyValueStrings(  const DWFSignatureReader::DSAKeyValueStrings& rDSAKeyValueStrings )
throw( DWFException )
{
    //
    //  These methods do real work in the derived class
    //
    return rDSAKeyValueStrings;
}

_DWFTK_API
const DWFString&
DWFSignatureReader::provideKeyName( const DWFString& zKeyName )
throw( DWFException )
{
    //
    //  These methods do real work in the derived class
    //
    return zKeyName;
}



_DWFTK_API
void
DWFSignatureReader::_provideSignatureID( const char* zId )
throw( DWFException )
{
    provideSignatureID( _pFilter ? _pFilter->provideSignatureID( zId ) : zId );
}

_DWFTK_API
void
DWFSignatureReader::_provideReference( const DWFString& zReferenceURI )
throw( DWFException )
{
    provideReference( _pFilter ? _pFilter->provideReference( zReferenceURI ) : zReferenceURI );
}

_DWFTK_API
void
DWFSignatureReader::_provideSignatureMethod( const DWFString& zSigMethod )
throw( DWFException )
{
    provideSignatureMethod( _pFilter ? _pFilter->provideSignatureMethod( zSigMethod ) : zSigMethod );
}

_DWFTK_API
void
DWFSignatureReader::_provideSignatureValue( const DWFString& zSigValue )
throw( DWFException )
{
    provideSignatureValue( _pFilter ? _pFilter->provideSignatureValue( zSigValue ) : zSigValue );
}

_DWFTK_API
void
DWFSignatureReader::_provideX509Data( X509Data *pX509Data )
throw( DWFException )
{
    provideX509Data( _pFilter ? _pFilter->provideX509Data( pX509Data ) : pX509Data );
}

_DWFTK_API
void
DWFSignatureReader::_provideDigestMethod( const DWFString& zDigestMethod )
throw( DWFException )
{
    provideDigestMethod( _pFilter ? _pFilter->provideDigestMethod( zDigestMethod ) : zDigestMethod );
}

_DWFTK_API
void
DWFSignatureReader::_provideDigestValue( const DWFString& zDigestValue )
throw( DWFException )
{
    provideDigestValue( _pFilter ? _pFilter->provideDigestValue( zDigestValue ) : zDigestValue );
}

_DWFTK_API
void
DWFSignatureReader::_provideRSAKeyValueStrings( const RSAKeyValueStrings& rRSAKeyValueStrings )
throw( DWFException )
{
    provideRSAKeyValueStrings( _pFilter ? _pFilter->provideRSAKeyValueStrings( rRSAKeyValueStrings ) : rRSAKeyValueStrings );
}

_DWFTK_API
void
DWFSignatureReader::_provideDSAKeyValueStrings( const DSAKeyValueStrings& rDSAKeyValueStrings )
throw( DWFException )
{
    provideDSAKeyValueStrings( _pFilter ? _pFilter->provideDSAKeyValueStrings( rDSAKeyValueStrings ) : rDSAKeyValueStrings );
}

_DWFTK_API
void
DWFSignatureReader::_provideKeyName( const DWFString& zKeyName )
throw( DWFException )
{
    provideKeyName( _pFilter ? _pFilter->provideKeyName( zKeyName ) : zKeyName );
}


_DWFTK_API
DWFString
DWFSignatureReader::_findAttributeValue( const char*  pAttributeName,
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

    return L"";
}
