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
//  $Header: //DWF/Development/Components/Internal/DWF Toolkit/v7.7/develop/global/src/dwf/package/X509.cpp#1 $
//  $DateTime: 2011/02/14 01:16:30 $
//  $Author: caos $
//  $Change: 197964 $
//  $Revision: #1 $
//

#include "dwfcore/DWFXMLSerializer.h"

#include "dwf/Version.h"
#include "dwf/package/XML.h"
#include "dwf/package/Constants.h"
#include "dwf/package/X509.h"
using namespace DWFToolkit;


_DWFTK_API
X509Data::~X509Data()
    throw()
{
    tX509DataItemVector::Iterator *piDataItems = dataItems();
    if(piDataItems->valid())
    {
        for(; piDataItems->valid(); piDataItems->next())
        {
            X509DataItem* pDataItem = piDataItems->get();
            DWFCORE_FREE_OBJECT(pDataItem);
        }
    }
    DWFCORE_FREE_OBJECT( piDataItems );
}


#ifndef DWFTK_READ_ONLY

_DWFTK_API
void
X509IssuerSerial::serialize( DWFXMLSerializer& rSerializer, const DWFString& zNamespace )
    throw( DWFException )
{
    rSerializer.startElement( DWFXML::kzElement_X509IssuerSerial, zNamespace );
    {
        rSerializer.startElement( DWFXML::kzElement_X509IssuerName, zNamespace );
        {
            rSerializer.addCData( issuerName() );
        }
        rSerializer.endElement(); // end of X509IssuerName

        rSerializer.startElement( DWFXML::kzElement_X509SerialNumber, zNamespace );
        {
            wchar_t     zTempBuffer[128] = {0};
            _DWFCORE_SWPRINTF( zTempBuffer, 128, /*NOXLATE*/L"%d", serialNumber() );
            rSerializer.addCData( zTempBuffer );
        }
        rSerializer.endElement(); // end of X509SerialNumber
    }
    rSerializer.endElement(); // end of X509IssuerSerial
}

_DWFTK_API
void
X509SKI::serialize( DWFXMLSerializer& rSerializer, const DWFString& zNamespace )
    throw( DWFException )
{
    rSerializer.startElement( DWFXML::kzElement_X509SKI, zNamespace );
    {
        rSerializer.addCData( subjectKeyIdentifier() );
    }
    rSerializer.endElement(); // end of X509SKI
}

_DWFTK_API
void
X509SubjectName::serialize( DWFXMLSerializer& rSerializer, const DWFString& zNamespace )
    throw( DWFException )
{
    if(_zName.chars() > 0)
    {
        rSerializer.startElement( DWFXML::kzElement_X509SubjectName, zNamespace );
        {
            rSerializer.addCData( subjectName() );
        }
        rSerializer.endElement(); // end of X509SubjectName
    }
}

_DWFTK_API
void
X509Certificate::serialize( DWFXMLSerializer& rSerializer, const DWFString& zNamespace )
    throw( DWFException )
{
    if(_zCert.chars() > 0)
    {
        rSerializer.startElement( DWFXML::kzElement_X509Certificate, zNamespace );
        {
            rSerializer.addCData( certificate() );
        }
        rSerializer.endElement(); // end of X509Certificate
    }
}

_DWFTK_API
void
X509CRL::serialize( DWFXMLSerializer& rSerializer, const DWFString& zNamespace )
    throw( DWFException )
{
    rSerializer.startElement( DWFXML::kzElement_X509CRL, zNamespace );
    {
        rSerializer.addCData( CRL() );
    }
    rSerializer.endElement(); // end of X509CRL
}


_DWFTK_API
void
X509Data::serialize( DWFXMLSerializer& rSerializer, const DWFString& zNamespace )
    throw( DWFException )
{
    tX509DataItemVector::Iterator* piDataItems = dataItems();

        //
        // The XML/DS spec insists that we don't emit an <X509Data> element unless there are elements inside it.
        //
    if(piDataItems->valid())
    {
        rSerializer.startElement( DWFXML::kzElement_X509Data, zNamespace );
        {
            for(; piDataItems->valid(); piDataItems->next())
            {
                X509DataItem* pDataItem = piDataItems->get();
                pDataItem->serialize(rSerializer, zNamespace);
            }
        }
        rSerializer.endElement(); // end of X509Data
    }

    DWFCORE_FREE_OBJECT( piDataItems );
}


#endif

