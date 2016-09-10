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

#include "XAML/pch.h"


//
// ctor
//
XamlDictionary::XamlDictionary(
    WT_XAML_File & rFile)                   // file
: _nKeyId(0), _rFile(rFile)
{
}

//
// dtor : release the internal resource copies
//
XamlDictionary::~XamlDictionary()
{
    map<const wchar_t *, XamlResource *>::iterator i;

    for(i = _oKeyToResource.begin(); i != _oKeyToResource.end(); i++) 
    {
        wchar_t *pBuf = const_cast<wchar_t*>(i->first);
        DWFCORE_FREE_MEMORY( pBuf );
        XamlResource *pRes = i->second;
        DWFCORE_FREE_OBJECT( pRes );
    }
}

//
// resource instance to key lookup (linear)
//
const wchar_t *XamlDictionary::lookup(
    const XamlResource &what)               // resource to lookup
{
    map<const wchar_t *, XamlResource *>::iterator i;
    for(i = _oKeyToResource.begin(); i != _oKeyToResource.end(); i++) {

        if(*i->second == what) {

            return i->first;
        }
    }

    wchar_t *pBuf = DWFCORE_ALLOC_MEMORY( wchar_t, 32 );

    _DWFCORE_SWPRINTF(pBuf, 31, L"R%d", _nKeyId++);

    _oKeyToResource.insert(pair<const wchar_t*, XamlResource *>(pBuf, what.copy()));
    
    what.serializeResource(pBuf, _rFile, *_rFile.xamlDictionarySerializer());

    return pBuf;
}

//
// private assignment operator
//
XamlDictionary &XamlDictionary::operator = (
    XamlDictionary & )
{
    return *this;
}
