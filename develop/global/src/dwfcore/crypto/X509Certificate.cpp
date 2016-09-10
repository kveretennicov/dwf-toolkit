//
//  Copyright (c) 2006 by Autodesk, Inc.
//
//  By using this code, you are agreeing to the terms and conditions of
//  the License Agreement included in the documentation for this code.
//
//  AUTODESK MAKES NO WARRANTIES, EXPRESS OR IMPLIED,
//  AS TO THE CORRECTNESS OF THIS CODE OR ANY DERIVATIVE
//  WORKS WHICH INCORPORATE IT.
//
//  AUTODESK PROVIDES THE CODE ON AN "AS-IS" BASIS
//  AND EXPLICITLY DISCLAIMS ANY LIABILITY, INCLUDING
//  CONSEQUENTIAL AND INCIDENTAL DAMAGES FOR ERRORS,
//  OMISSIONS, AND OTHER PROBLEMS IN THE CODE.
//
//  Use, duplication, or disclosure by the U.S. Government is subject to
//  restrictions set forth in FAR 52.227-19 (Commercial Computer Software
//  Restricted Rights) and DFAR 252.227-7013(c)(1)(ii) (Rights in Technical
//  Data and Computer Software), as applicable.
//


#ifndef DWFCORE_DISABLE_CRYPTO


#include "dwfcore/crypto/X509Certificate.h"
using namespace DWFCore;


_DWFCORE_API
DWFX509Certificate::DWFX509Certificate()
throw()
{
    ;
}

_DWFCORE_API
DWFX509Certificate::~DWFX509Certificate()
throw()
{
}

///
///
///

_DWFCORE_API
DWFX509Certificate::IssuerSerial::IssuerSerial( unsigned int      nNumber,
                                                const DWFString&  rCN,
                                                const DWFString&  rC,
                                                const DWFString&  rOU,
                                                const DWFString&  rO,
                                                const DWFString&  rL,
                                                const DWFString&  rST,
                                                const DWFString&  rT )
throw( DWFException )
                  : _nNumber( nNumber )
{
    bool bFirst( false );

    if (rCN.chars() > 0)
    {
        bFirst = true;
        _zName.append( L"CN=" );
        _zName.append( rCN );
    }

    if (rC.chars() > 0)
    {
        if (bFirst)
        {
            _zName.append( ", " );
        }
        else
        {
            bFirst = true;
        }
        _zName.append( L"C=" );
        _zName.append( rC );
    }

    if (rOU.chars() > 0)
    {
        if (bFirst)
        {
            _zName.append( ", " );
        }
        else
        {
            bFirst = true;
        }
        _zName.append( L"OU=" );
        _zName.append( rOU );
    }

    if (rO.chars() > 0)
    {
        if (bFirst)
        {
            _zName.append( ", " );
        }
        else
        {
            bFirst = true;
        }
        _zName.append( L"O=" );
        _zName.append( rO );
    }

    if (rL.chars() > 0)
    {
        if (bFirst)
        {
            _zName.append( ", " );
        }
        else
        {
            bFirst = true;
        }
        _zName.append( L"L=" );
        _zName.append( rL );
    }

    if (rST.chars() > 0)
    {
        if (bFirst)
        {
            _zName.append( ", " );
        }
        else
        {
            bFirst = true;
        }
        _zName.append( L"ST=" );
        _zName.append( rST );
    }

    if (rT.chars() > 0)
    {
        if (bFirst)
        {
            _zName.append( ", " );
        }
        else
        {
            bFirst = true;
        }
        _zName.append( L"T=" );
        _zName.append( rT );
    }
}

#endif

