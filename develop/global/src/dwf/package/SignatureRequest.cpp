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
//  $Header: //DWF/Development/Components/Internal/DWF Toolkit/v7.7/develop/global/src/dwf/package/SignatureRequest.cpp#1 $
//  $DateTime: 2011/02/14 01:16:30 $
//  $Author: caos $
//  $Change: 197964 $
//  $Revision: #1 $
//

#include "dwfcore/MIME.h"
#include "dwfcore/BufferInputStream.h"
#include "dwfcore/BufferOutputStream.h"
//#include "dwfcore/crypto/KeyGenerator.h"
#include "dwfcore/crypto/CryptoEngineProvider.h"
#include "dwfcore/crypto/AsymmetricAlgorithm.h"
using namespace DWFCore;


#include "dwf/Version.h"
#include "dwf/package/XML.h"
#include "dwf/package/Manifest.h"
#include "dwf/package/Constants.h"
#include "dwf/package/writer/PackageWriter.h"
#include "dwf/package/SignatureResource.h"
#include "dwf/package/SignatureRequest.h"
using namespace DWFToolkit;



_DWFTK_API
DWFSignatureRequest::DWFSignatureRequest()
    throw()
    : _nVersion( 0.1f /* TODO: _DWF_FORMAT_SIGNATURE_VERSION_CURRENT_FLOAT */ )
    , _pResource(NULL)
    , _pPrivateKey(NULL)
{
}

_DWFTK_API
DWFSignatureRequest::~DWFSignatureRequest()
throw()
{
    if(_pResource != NULL)
    {
        DWFCORE_FREE_OBJECT(_pResource);
    }

    if(_pPrivateKey != NULL)
    {
        DWFCORE_FREE_OBJECT(_pPrivateKey);
    }
}

_DWFTK_API
void DWFSignatureRequest::setPrivateKey(DWFCryptoKey* pPrivateKey)
    throw()
{
    _pPrivateKey = pPrivateKey;
}

#ifndef DWFTK_READ_ONLY

_DWFTK_API
void
DWFSignatureRequest::serializeXML( DWFXMLSerializer& rSerializer, unsigned int nFlags )
throw( DWFException )
{
        //
        // full detailed dump into the signature document
        //
    if (nFlags & DWFPackageWriter::eSignatureRequest)
    {
        DWFString zNamespace;

        if (nFlags & DWFPackageWriter::eDescriptor)
        {
            zNamespace.assign( namespaceXML(nFlags/*|DWFPackageWriter::eSignatures*/) );
        }

        rSerializer.startElement( DWFXML::kzElement_Signature, zNamespace );
        {
            //
            // other namespaces added for extension
            //

            // TODO: 
            //_tNamespaceMap::iterator iNamespace = _oAddedNamespaces.begin();
            //for (; iNamespace != _oAddedNamespaces.end(); iNamespace++)
            //{
            //    rSerializer.addAttribute( iNamespace->second.prefix(), iNamespace->second.xmlns(), /*NOXLATE*/L"xmlns:" );
            //}


            //
            // Push the computed digests into the references.
            //
            DWFSignature::tReferenceVector::Iterator* piRef = references();
            for( ; piRef->valid(); piRef->next())
            {
                Reference* pRef = dynamic_cast<Reference*>(piRef->get());
                if(pRef != NULL)
                {
                    DWFDigestOutputStream* digestStream = pRef->digestOutputStream();
                    if(digestStream != NULL)
                    {
                        DigestValue* pValue = DWFCORE_ALLOC_OBJECT(DigestValue);
                        pValue->setValueBase64(digestStream->digestBase64());
                        pRef->setDigestValue(pValue);
                    }
                }
            }
            DWFCORE_FREE_OBJECT( piRef );


            //
            // Write the signed info twice.
            //
            // The first time, it actually goes into out xml output.
            //
            // The second time, it goes to a serializer that computes
            // the digest of the stream. This is then encrypted to form
            // the signature value.
            //
            // !!!VERY IMPORTANT!!!
            // This approach will fail if anything in _serializeSignedInfo() gets
            // vended a UUID, as they won't match.
            // !!!VERY IMPORTANT!!!
            //

            // first time, into the output xml
            _serializeSignedInfo(rSerializer, zNamespace);

             // second time, compute digest
            DWFSignature::DigestValue signedInfoDigest;
            computeSignedInfoDigestValue(signedInfoDigest, zNamespace);


            // TODO: This is just temporary - until we can encrypt it to make a signature
            rSerializer.startElement( L"Debug_ExpectedDigestValue", zNamespace );
            DWFString zDigest(signedInfoDigest.valueBase64());
            if(zDigest.chars() > 0)
            {
                rSerializer.addCData( zDigest );
            }
            else
            {
                // TODO: Anything better???
                rSerializer.addCData( L"No digest value available" );
            }
            rSerializer.endElement(); // end of ExpectedDigestValue
            // End of Temporary data

            SignatureValue* pSignatureValue = DWFCORE_ALLOC_OBJECT(SignatureValue);
            if(privateKey() != NULL)
            {
                signatureMethod()->signDigest(&signedInfoDigest, digestMethod(), privateKey(), pSignatureValue);

            }

            // Update the signatureValue in the Signature object.
            setSignatureValue(pSignatureValue);

            rSerializer.startElement( DWFXML::kzElement_SignatureValue, zNamespace );
            rSerializer.addCData( pSignatureValue->valueBase64() );
            rSerializer.endElement(); // end of SignatureValue

            rSerializer.startElement( DWFXML::kzElement_KeyInfo, zNamespace );
            {
                if(keyName().chars() > 0)
                {
                    rSerializer.startElement( DWFXML::kzElement_KeyName, zNamespace );
                    {
                        rSerializer.addCData( keyName() );
                    }
                    rSerializer.endElement(); // end of KeyName
                }

                DWFCryptoKey* pPublicKey = publicKey();
                if(pPublicKey != NULL)
                {
                    rSerializer.startElement( DWFXML::kzElement_KeyValue, zNamespace );
                    {
                        DWFCryptoEngine::teType eAlgType = signatureMethod()->algorithm()->identifier();
                        if(eAlgType == DWFCryptoEngine::eDSA)
                        {
                            rSerializer.startElement( DWFXML::kzElement_DSAKeyValue, zNamespace );
                            {
                                DWFString zP(L"No P available.");
                                DWFString zQ(L"No Q available.");
                                DWFString zG(L"No G available.");
                                DWFString zY(L"No Y available.");
                                DWFString zJ(L"No J available.");
                                DWFString zSeed(L"No Seed available.");
                                DWFString zPgenCounter(L"No PgenCounter available.");
                                // TODO: Extract values from key!

                                rSerializer.startElement( DWFXML::kzElement_DSAKeyValueP, zNamespace );
                                {
                                    rSerializer.addCData( zP );
                                }
                                rSerializer.endElement(); // end of P

                                rSerializer.startElement( DWFXML::kzElement_DSAKeyValueQ, zNamespace );
                                {
                                    rSerializer.addCData( zQ );
                                }
                                rSerializer.endElement(); // end of Q
 
                                rSerializer.startElement( DWFXML::kzElement_DSAKeyValueG, zNamespace );
                                {
                                    rSerializer.addCData( zG );
                                }
                                rSerializer.endElement(); // end of G

                                rSerializer.startElement( DWFXML::kzElement_DSAKeyValueY, zNamespace );
                                {
                                    rSerializer.addCData( zY );
                                }
                                rSerializer.endElement(); // end of Y

                                rSerializer.startElement( DWFXML::kzElement_DSAKeyValueJ, zNamespace );
                                {
                                    rSerializer.addCData( zJ );
                                }
                                rSerializer.endElement(); // end of J

                                rSerializer.startElement( DWFXML::kzElement_DSAKeyValueSeed, zNamespace );
                                {
                                    rSerializer.addCData( zSeed );
                                }
                                rSerializer.endElement(); // end of Seed

                                rSerializer.startElement( DWFXML::kzElement_DSAKeyValuePgenCounter, zNamespace );
                                {
                                    rSerializer.addCData( zPgenCounter );
                                }
                                rSerializer.endElement(); // end of PgenCounter
                             }
                            rSerializer.endElement(); // end of DSAKeyValue
                        }
                        else if(eAlgType == DWFCryptoEngine::eRSA)
                        {
                            rSerializer.startElement( DWFXML::kzElement_RSAKeyValue, zNamespace );
                            {
                                DWFString zModulus(L"No Modulus available.");
                                DWFString zExponent(L"No Exponent available.");
                                // TODO: Extract Modulus & Exponent from key!

                                rSerializer.startElement( DWFXML::kzElement_RSAKeyValueModulus, zNamespace );
                                {
                                    rSerializer.addCData( zModulus );
                                }
                                rSerializer.endElement(); // end of Modulus

                                rSerializer.startElement( DWFXML::kzElement_RSAKeyValueExponent, zNamespace );
                                {
                                    rSerializer.addCData( zExponent );
                                }
                                rSerializer.endElement(); // end of Exponent
                            }
                            rSerializer.endElement(); // end of RSAKeyValue
                        }
                    }
                    rSerializer.endElement(); // end of KeyValue
                }

                //
                // Serialize all the X509 data items.
                //
                tX509DataVector::Iterator *piX509Datas = _oX509DataVector.iterator();
                for(; piX509Datas->valid(); piX509Datas->next())
                {
                    X509Data* pData = piX509Datas->get();
                    pData->serialize(rSerializer, zNamespace);
                }
                DWFCORE_FREE_OBJECT( piX509Datas );

            }
            rSerializer.endElement(); // end of KeyInfo

            // TODO: Serialize the rest...

        }
        rSerializer.endElement();
    }
    else if (nFlags & DWFPackageWriter::eDescriptor)
    {
        DWFString zNamespace;
        zNamespace.assign( namespaceXML(nFlags) );

        //
        //  Start element
        //
        if ((nFlags & DWFXMLSerializer::eElementOpen) == 0)
        {
            rSerializer.startElement( DWFXML::kzElement_SignatureResource, zNamespace );
        }

        //
        //  Let base class know not to start the element
        //
        nFlags |= DWFXMLSerializer::eElementOpen;

        // TODO: Something like: DWFContentPresentationContainer::getSerializable().serializeXML( rSerializer, nFlags );


        // TODO: Determine if there's anything in Resource that we'd need
        //
        //  Base attributes
        //
        // DWFResource::serializeXML( rSerializer, nFlags );

        //
        //  Clear this bit
        //
        nFlags &= ~DWFXMLSerializer::eElementOpen;

        //
        //  Close element if this bit is not set
        //
        if ((nFlags & DWFXMLSerializer::eElementOpen) == 0)
        {
            rSerializer.endElement();
        }
    }

        //
        // otherwise defer to the base class
        //
    else
    {
        // TODO: Determine if there's anything in Resource that we'd need
        // DWFResource::serializeXML( rSerializer, nFlags );
    }
}

#endif

DWFResource*
DWFSignatureRequest::resource()
    throw()
{
    if(_pResource==NULL)
    {
        _pResource = DWFCORE_ALLOC_OBJECT( DWFSignatureResource() );
    }

    return _pResource;
}


_DWFTK_API
DWFSignatureRequest::ConcreteReference::ConcreteReference()
    throw()
    : _pResource(NULL)
{
    ;
}

///
///         Destructor
///
///\throw   None
///
_DWFTK_API
 DWFSignatureRequest::ConcreteReference::~ConcreteReference()
    throw()
{
    ;
}

const DWFResource* DWFSignatureRequest::ConcreteReference::resource() const
    throw()
{
    return _pResource;
}

void DWFSignatureRequest::ConcreteReference::setResource(const DWFResource* pResource)
    throw()
{
    _pResource = pResource;
}

const DWFString& DWFSignatureRequest::ConcreteReference::URI() const
    throw( DWFException )
{
    // TODO: Awkward
    if(_pResource==NULL)
    {
        _DWFCORE_THROW( DWFInvalidArgumentException, /*NOXLATE*/ L"No resource specified." );
    }

    return _pResource->publishedIdentity();
}
