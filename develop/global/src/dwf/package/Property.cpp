//
//  Copyright (c) 2003-2006 by Autodesk, Inc.  All rights reserved.
//
// Permission to use, copy, modify, and distribute this software in
// object code form for any purpose and without fee is hereby granted,
// provided that the above copyright notice appears in all copies and
// that both that copyright notice and the limited warranty and
// restricted rights notice below appear in all supporting
// documentation.
//
// AUTODESK PROVIDES THIS PROGRAM 'AS IS' AND WITH ALL FAULTS.
// AUTODESK SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTY OF
// MERCHANTABILITY OR FITNESS FOR A PARTICULAR USE.  AUTODESK, INC.
// DOES NOT WARRANT THAT THE OPERATION OF THE PROGRAM WILL BE
// UNINTERRUPTED OR ERROR FREE.
//
// Use, duplication, or disclosure by the U.S. Government is subject to
// restrictions set forth in FAR 52.227-19 (Commercial Computer
// Software - Restricted Rights) and DFAR 252.227-7013(c)(1)(ii)
// (Rights in Technical Data and Computer Software), as applicable.
//
// $Header: //DWF/Development/Components/Internal/DWF Toolkit/v7.7/develop/global/src/dwf/package/Property.cpp#2 $
// $DateTime: 2011/10/11 01:26:00 $
// $Author: caos $
// $Change: 229047 $
// $Revision: #2 $
//

#include "dwfcore/DWFXMLSerializer.h"

#include "dwf/package/Property.h"
#include "dwf/package/Constants.h"
#include "dwf/package/reader/PackageManifestReader.h"

using namespace DWFToolkit;

const DWFString DWFProperty::_koEmpty;

DWFStringTable* DWFProperty::_kpDataTable = &DWFStringTable::Instance();

DWFProperty::tPropertyContent::tPropertyContent() 
	: _pszName(&_koEmpty)    
	, _pszCategory(&_koEmpty)
	, _pszType(&_koEmpty)
	, _pszUnits(&_koEmpty)
	, _pszValue(&_koEmpty)
	, _pOwner(0)

#ifndef DWFTK_READ_ONLY

	, _oAttributes()

#endif

{}

DWFProperty::tPropertyContent::tPropertyContent(const DWFProperty::tPropertyContent& rhs)
	: _pszName(rhs._pszName)
	, _pszCategory(rhs._pszCategory)
	, _pszType(rhs._pszType)
	, _pszUnits(rhs._pszUnits)
	, _pszValue(rhs._pszValue)
	, _pOwner(0)

#ifndef DWFTK_READ_ONLY

	, _oAttributes(rhs._oAttributes)

#endif
{}

DWFProperty::tPropertyContent& DWFProperty::tPropertyContent::operator=(const DWFProperty::tPropertyContent& rhs) 
{
	_pszName = rhs._pszName;
	_pszCategory = rhs._pszCategory;
	_pszType = rhs._pszType;
	_pszUnits = rhs._pszUnits;
	_pszValue = rhs._pszValue;

#ifndef DWFTK_READ_ONLY

    _oAttributes = rhs._oAttributes;

#endif
	return *this;
}

_DWFTK_API
DWFProperty::DWFProperty()
throw()
{
    ;
}

_DWFTK_API
DWFProperty::DWFProperty( const DWFString& zName,
                          const DWFString& zValue,
                          const DWFString& zCategory,
                          const DWFString& zType,
                          const DWFString& zUnits )
throw()
{
    oPropertyContent->_pszName = _kpDataTable->insert(zName);
	oPropertyContent->_pszCategory = _kpDataTable->insert(zCategory);
	oPropertyContent->_pszType = _kpDataTable->insert(zType);
	oPropertyContent->_pszUnits = _kpDataTable->insert(zUnits);
	oPropertyContent->_pszValue = _kpDataTable->insert(zValue);
}

_DWFTK_API
DWFProperty::DWFProperty( const DWFProperty& rProperty )
throw()
: oPropertyContent(rProperty.oPropertyContent)
{
    ;
}

_DWFTK_API
DWFProperty&
DWFProperty::operator=( const DWFProperty& rProperty )
throw()
{
	tImpType(rProperty.oPropertyContent).swap(oPropertyContent);

    return *this;
}

_DWFTK_API
DWFProperty::~DWFProperty()
throw()
{
    //
    // requirement of the DWFOwnable contract
    //
    _notifyDelete();

    //
    // do NOT delete the data table ptr
    //
}

_DWFTK_API
void
DWFProperty::parseAttributeList( const char** ppAttributeList )
throw( DWFException )
{
	parseAttributeList(ppAttributeList, false);
}

_DWFTK_API
void
DWFProperty::parseAttributeList( const char** ppAttributeList, bool bIgnoreCustomizeAttribute )
throw( DWFException )
{
    unsigned char nFound = 0;
    size_t iAttrib = 0;
    const char* pAttrib = NULL;

    for(; ppAttributeList[iAttrib]; iAttrib += 2)
    {
            //
            // skip over any acceptable prefixes in the element name
            //
        if (DWFCORE_COMPARE_MEMORY(DWFXML::kzNamespace_DWF, ppAttributeList[iAttrib], 4) == 0)
        {
            pAttrib = &ppAttributeList[iAttrib][4];
        } 
        else if (DWFCORE_COMPARE_MEMORY(DWFXML::kzNamespace_ECommon, ppAttributeList[iAttrib], 8) == 0)
        {
            pAttrib = &ppAttributeList[iAttrib][8];
        } 
        else if (DWFCORE_COMPARE_MEMORY(DWFXML::kzNamespace_EPlot, ppAttributeList[iAttrib], 6) == 0)
        {
            pAttrib = &ppAttributeList[iAttrib][6];
        }
        else if (DWFCORE_COMPARE_MEMORY(DWFXML::kzNamespace_EModel, ppAttributeList[iAttrib], 7) == 0)
        {
            pAttrib = &ppAttributeList[iAttrib][7];
        }
        else
        {
            pAttrib = &ppAttributeList[iAttrib][0];
        }

            //
            // set the name
            //
        if (!(nFound & 0x01) &&
             (DWFCORE_COMPARE_ASCII_STRINGS(pAttrib, DWFXML::kzAttribute_Name) == 0))
        {
            nFound |= 0x01;

			oPropertyContent->_pszName = _kpDataTable->insert(ppAttributeList[iAttrib+1]);
        }
            //
            // set the value
            //
        else if (!(nFound & 0x02) &&
                  (DWFCORE_COMPARE_ASCII_STRINGS(pAttrib, DWFXML::kzAttribute_Value) == 0))
        {
            nFound |= 0x02;

			oPropertyContent->_pszValue = _kpDataTable->insert(ppAttributeList[iAttrib+1]);
        }
            //
            // set the type
            //
        else if (!(nFound & 0x04) &&
                  (DWFCORE_COMPARE_ASCII_STRINGS(pAttrib, DWFXML::kzAttribute_Type) == 0))
        {
            nFound |= 0x04;

			oPropertyContent->_pszType = _kpDataTable->insert(ppAttributeList[iAttrib+1]);
        }
            //
            // set the units
            //
        else if (!(nFound & 0x08) &&
                  (DWFCORE_COMPARE_ASCII_STRINGS(pAttrib, DWFXML::kzAttribute_Units) == 0))
        {
            nFound |= 0x08;

			oPropertyContent->_pszUnits = _kpDataTable->insert(ppAttributeList[iAttrib+1]);
        }
            //
            // set the category
            //
        else if (!(nFound & 0x10) &&
                  (DWFCORE_COMPARE_ASCII_STRINGS(pAttrib, DWFXML::kzAttribute_Category) == 0))
        {
            nFound |= 0x10;

			oPropertyContent->_pszCategory = _kpDataTable->insert(ppAttributeList[iAttrib+1]);
        }
		//
		// Here, we handle the customer defined attributes.
		//
		else if(!bIgnoreCustomizeAttribute)
		{
			DWFString oCustomizedProperty(pAttrib);
			size_t    position;
			if ( (position = oCustomizedProperty.find(L":")) > 0)
			{
				char*	  pNameSpace = (char*)pAttrib;				
				*(pNameSpace+position) = '\0';
				DWFString oName((char*)pAttrib+position+1);
				DWFString oValue(ppAttributeList[iAttrib+1]);

				// Since the XMLNS isn't used during writing this Attribute out by reading
				// the function of AddXMLAttribute(), we can set XMLNS is the same with
				// the namespace. This is allowed by XML standard.
				DWFXMLNamespace MyNamesSpace(pNameSpace, pNameSpace);
				addXMLAttribute(MyNamesSpace, oName, oValue);
			}
			// We shouldn't get here in logic, because when customer set custormized attribute to a 
			// property, they must set a NameSpace for it and both of nameSpace and XMLNS  
			// shouldn't be empty.
			else
			{
				DWFString oName((char*)pAttrib);
				DWFString oValue(ppAttributeList[iAttrib+1]);
				//
				// if no namespace is set, we set a default one.
				//
				DWFXMLNamespace MyNamesSpace(L"unknown", L"unknown");
				addXMLAttribute(MyNamesSpace, oName, oValue);
			}
		}
    }
}

//
// The attributes can only be visited but can't be modified
//
_DWFTK_API
const DWFProperty::tAttributeMap& DWFProperty::getCustomizedAttributeList()
throw()
{
	return oPropertyContent->_oAttributes;
}

_DWFTK_API
void DWFProperty::setName( const DWFString& zName )
throw()
{
	oPropertyContent->_pszName = _kpDataTable->insert(zName);
}

_DWFTK_API
void DWFProperty::setValue( const DWFString& zValue )
throw()
{
	oPropertyContent->_pszValue = _kpDataTable->insert(zValue);
}

_DWFTK_API
void DWFProperty::setCategory( const DWFString& zCategory )
throw()
{
	oPropertyContent->_pszCategory = _kpDataTable->insert(zCategory);
}

_DWFTK_API
void DWFProperty::setType( const DWFString& zType )
throw()
{
	oPropertyContent->_pszType = _kpDataTable->insert(zType);
}

_DWFTK_API
void DWFProperty::setUnits( const DWFString& zUnits )
throw()
{
	oPropertyContent->_pszUnits = _kpDataTable->insert(zUnits);
}

_DWFTK_API
void
DWFProperty::own( DWFOwner& rOwner )
throw( DWFException )
{
    if (oPropertyContent->_pOwner == &rOwner)
    {
        return;
    }

    if (oPropertyContent->_pOwner)
    {
		oPropertyContent->_pOwner->notifyOwnerChanged( this->_toOwnable() );
    }

    oPropertyContent->_pOwner = &rOwner;
    oPropertyContent->_oOwnerObservers.insert( &rOwner );
}

_DWFTK_API
bool
DWFProperty::disown( DWFOwner& rOwner, bool bForget )
throw( DWFException )
{
    if (oPropertyContent->_pOwner)
    {
            //
            // can only disown what we own
            //
        if (oPropertyContent->_pOwner == &rOwner)
        {
                //
                // remove owner from observers
                //
            if (bForget)
            {
                oPropertyContent->_oOwnerObservers.erase( oPropertyContent->_pOwner );
            }

            //
            //
            //
			oPropertyContent->_pOwner->notifyOwnerChanged( this->_toOwnable() );
            oPropertyContent->_pOwner = NULL;

            return true;
        }
    }

    return false;
}

_DWFTK_API
DWFOwner*
DWFProperty::owner()
throw( DWFException )
{
    return oPropertyContent->_pOwner;
}

_DWFTK_API
void
DWFProperty::observe( DWFOwner& rOwner )
throw( DWFException )
{
    if (oPropertyContent->_pOwner == &rOwner)
    {
        // can't observe something that you already own.
        return;
    }

    oPropertyContent->_oOwnerObservers.insert( &rOwner );
}

_DWFTK_API
bool
DWFProperty::unobserve( DWFOwner& rOwner )
throw( DWFException )
{
        //
        // Ensure that observe and own aren't being confused.
        // We'd better not be the owner if we're just observing.
        //
    if (oPropertyContent->_pOwner == &rOwner)
    {
        return false;
    }

    //
    // remove observer from observers
    //
    oPropertyContent->_oOwnerObservers.erase( &rOwner );

    return true;
}

_DWFTK_API
void
DWFProperty::_notifyDelete()
throw()
{
    if (oPropertyContent->_oOwnerObservers.size() > 0)
    {
        DWFOwner* pOwner = NULL;
        DWFSortedList<DWFOwner*>::Iterator* piOwner = oPropertyContent->_oOwnerObservers.iterator();

        for (; piOwner->valid(); )
        {
            pOwner = piOwner->get();

            //
            // Increment the iterator here, before calling notifyOwnableDeletion,
            // as that might cause pOwner to disown/unobserve us, which messes
            // up the iterator. (Which causes a crash when calling piOwner->next().)
            //
            piOwner->next();

                //
                // owner should be the only one deleting this object
                // so there is no need to notify him
                //
            if (pOwner != oPropertyContent->_pOwner)
            {
				pOwner->notifyOwnableDeletion( this->_toOwnable() );
            }
        }

        DWFCORE_FREE_OBJECT( piOwner );
    }
}

DWFOwnable& DWFProperty::_toOwnable()
{
	IDWFOwnable* ownable = this;
	DWFOwnable* oa = (DWFOwnable*)ownable;
	return *oa;
}

_DWFTK_API
void
DWFProperty::addXMLAttribute( const DWFXMLNamespace&    rNamespace,
                              const DWFString&          zName,
                              const DWFString&          zValue )
throw( DWFException )
{
        //
        // all parameters are required
        //
    if ((zName.chars() == 0) || (zValue.chars() == 0))
    {
        _DWFCORE_THROW( DWFInvalidArgumentException, /*NOXLATE*/L"Attribute name and value parameters must be provided." );
    }

    tPropertyContent::_tAttributeMap::iterator iAttributeList = oPropertyContent->_oAttributes.find( rNamespace.prefix() );
    if (iAttributeList != oPropertyContent->_oAttributes.end())
    {
		iAttributeList->second.push_back( tPropertyContent::_tAttribute(zName, zValue) );
    }
    else
    {
		tPropertyContent::_tAttributeList oNewList;
		oNewList.push_back( tPropertyContent::_tAttribute(zName, zValue) );
        oPropertyContent->_oAttributes[rNamespace.prefix()] = oNewList;
    }
}

#ifndef DWFTK_READ_ONLY

_DWFTK_API
void
DWFProperty::serializeXML( DWFXMLSerializer& rSerializer, unsigned int nFlags )
throw( DWFException )
{
    DWFString zNamespace;

        //
        // namespace dictated by document and section type 
        //
    if (nFlags & DWFPackageWriter::eDescriptor ||
        nFlags & DWFPackageWriter::eGlobalContent ||
        nFlags & DWFPackageWriter::eSectionContent)
    {
        zNamespace.assign( namespaceXML(nFlags) );
    }
        //
        // the dwf namespace is currently only used in the manifest
        //
    else if (nFlags & DWFPackageWriter::eManifest)
    {
        zNamespace.assign( DWFXML::kzNamespace_DWF );
    }


    rSerializer.startElement( DWFXML::kzElement_Property, zNamespace );

    rSerializer.addAttribute( DWFXML::kzAttribute_Name, name() );
    rSerializer.addAttribute( DWFXML::kzAttribute_Value, value() );
    rSerializer.addAttribute( DWFXML::kzAttribute_Category, category() );
    rSerializer.addAttribute( DWFXML::kzAttribute_Type, type() );
    rSerializer.addAttribute( DWFXML::kzAttribute_Units, units() );

    zNamespace.destroy();

    tPropertyContent::_tAttributeMap::iterator iAttributeList = oPropertyContent->_oAttributes.begin();
    for (; iAttributeList != oPropertyContent->_oAttributes.end(); iAttributeList++)
    {
        zNamespace.assign( iAttributeList->first );
        zNamespace.append( /*NOXLATE*/L":" );

		tPropertyContent::_tAttributeList& rAttributeList = iAttributeList->second;
		tPropertyContent::_tAttributeList::iterator iAttribute = rAttributeList.begin();
        for (; iAttribute != rAttributeList.end(); iAttribute++)
        {
            rSerializer.addAttribute( iAttribute->first, iAttribute->second, zNamespace );
        }
    }

    rSerializer.endElement();
}

#endif

void DWFProperty::tPropertyArchive::load(DWFInputAdapter& rInputAdapter
		 , DWFProperty::tPropertyArchive::tHandleType hHandle
		 , DWFProperty::tPropertyArchive::tPointer pObject)
{			
	assert(hHandle != DWFInputAdapter::kInvalidHandle);
	assert(pObject);

	DWFInputAdapter::tBufferType buf;
	rInputAdapter.load(hHandle, buf);

	if (buf.empty())
	{
		_DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"The Adapter returned invalid data, data source may be damaged" );
	}
	
	const DWFString** ppString = (const DWFString**)&buf[0];

	pObject->_pszName		= *ppString++;
	pObject->_pszCategory	= *ppString++;
	pObject->_pszType		= *ppString++;
	pObject->_pszUnits		= *ppString++;
	pObject->_pszValue		= *ppString++;

	DWFOwner** owner_ptr	= (DWFOwner**)ppString;
	pObject->_pOwner		= *owner_ptr++;

	size_t* size_ptr		= (size_t*)owner_ptr;
	size_t count	= *size_ptr++;

	owner_ptr		= (DWFOwner**)size_ptr;
	for (size_t i =0; i < count; ++i)
		pObject->_oOwnerObservers.insert(*owner_ptr++);

#ifndef DWFTK_READ_ONLY
	size_ptr	= (size_t*)owner_ptr;
	count		= *size_ptr++;

	DWFString key;
	for (size_t i =0; i < count; ++i)
	{						
		size_ptr = (size_t*)_loadStr(key, size_ptr);
		size_t vec_size = *size_ptr++;
		pObject->_oAttributes[key].reserve(vec_size);
        void* pos = size_ptr;
		DWFString first, second;
		for (size_t j = 0; j < vec_size; ++j)
		{
			pos = _loadStr(first, pos);
			pos = _loadStr(second, pos);
            pObject->_oAttributes[key].push_back( make_pair( first, second ) );
		}
	}
#endif
}

DWFOutputAdapter::tHandleType DWFProperty::tPropertyArchive::save(
	DWFOutputAdapter& rOutputAdapter, 
	tConstPointer p, 
	DWFOutputAdapter::tHandleType old)
{
	assert(p != 0);

	DWFInputAdapter::tBufferType buf;


	_copyByte(&p->_pszName, sizeof(size_t), back_inserter(buf));
	_copyByte(&p->_pszCategory, sizeof(size_t), back_inserter(buf));
	_copyByte(&p->_pszType, sizeof(size_t), back_inserter(buf));
	_copyByte(&p->_pszUnits, sizeof(size_t), back_inserter(buf));
	_copyByte(&p->_pszValue, sizeof(size_t), back_inserter(buf));
	_copyByte(&p->_pOwner, sizeof(size_t), back_inserter(buf));

	size_t size = p->_oOwnerObservers.size();
	_copyByte(&size, sizeof(size_t), back_inserter(buf));

	if (size > 0)
	{
		DWFSortedList<DWFOwner*>::ConstIterator* piOwner = p->_oOwnerObservers.constIterator();
		DWFOwner* pOwner = NULL;

		while(piOwner->valid())
		{
			pOwner = piOwner->get();
			_copyByte(&pOwner, sizeof(size_t), back_inserter(buf));

			piOwner->next();
		}

		DWFCORE_FREE_OBJECT( piOwner );
	}

#ifndef DWFTK_READ_ONLY
	size = p->_oAttributes.size();
	_copyByte(&size, sizeof(size_t), back_inserter(buf));

	for (DWFToolkit::DWFProperty::tPropertyContent::_tAttributeMap::const_iterator 
			itr(p->_oAttributes.begin()); 
			itr != p->_oAttributes.end(); ++itr)
	{
		size = itr->first.bytes();
		_copyByte(&size, sizeof(size_t), back_inserter(buf));
		_copyByte((const wchar_t*)itr->first, size, back_inserter(buf));

		size = itr->second.size();
		_copyByte(&size, sizeof(size_t), back_inserter(buf));
		for (DWFToolkit::DWFProperty::tPropertyContent::_tAttributeList::const_iterator 
				vitr(itr->second.begin()); vitr != itr->second.end(); ++vitr)
		{
			size = vitr->first.bytes();
			_copyByte(&size, sizeof(size_t), back_inserter(buf));
			_copyByte((const wchar_t*)vitr->first, size, back_inserter(buf));

			size = vitr->second.bytes();
			_copyByte(&size, sizeof(size_t), back_inserter(buf));
			_copyByte((const wchar_t*)vitr->second, size, back_inserter(buf));
		}
	}
#endif

	assert(!buf.empty());
	return rOutputAdapter.save(buf, old);
}

void * DWFProperty::tPropertyArchive::_loadStr(DWFString& str, void *pos)
{
	size_t* size_ptr = (size_t*)pos;
	size_t str_bytes = *size_ptr++;			//string size			
	str.assign((wchar_t*)size_ptr, str_bytes);	//string content

	wchar_t* p_wchar = (wchar_t*)size_ptr;
	p_wchar += str_bytes / sizeof(wchar_t);
	return p_wchar;
}

void DWFProperty::tPropertyArchive::_copyByte(const void* src, size_t size, tOutputIterator itr)
{
	typedef const unsigned char* UCharPtr;
	UCharPtr ptr = (UCharPtr)src;
	std::copy(ptr, ptr + size, itr);
}
