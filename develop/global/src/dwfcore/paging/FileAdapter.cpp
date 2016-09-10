//
//  Copyright (c) 2003-2006 by Autodesk, Inc.
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
#include "dwfcore/paging/FileAdapter.h"
#include "dwfcore/Exception.h"
#include "dwfcore/File.h"
#include <cassert>

namespace DWFCore
{
	DWFFileAdapter::DWFFileAdapter(const DWFFile& rFile)
		: _oStream(rFile, /*NOXLATE*/L"wb+")
		, _bTransacting(false), _oNextHandle(kInvalidHandle)
	{		
		init();
	}	

	DWFFileAdapter::~DWFFileAdapter()
	{
		_oStream.close();
	}

	void DWFFileAdapter::init()
	{
		_oStream.open();
		const char str[] = /*NOXLATE*/"DBFILE.";
		_oStream.write(str, sizeof(str));
		_oStream.seek(SEEK_END, 0);
		_oNextHandle = (tHandleType)sizeof(str);
	}

	void DWFFileAdapter::load(DWFFileAdapter::tHandleType hHandle, DWFFileAdapter::tBufferType& buf)
	{
		assert(!_bTransacting);

		if (hHandle != kInvalidHandle)
		{
			_oStream.seek(SEEK_SET, (off_t)hHandle);	

			size_t size;
			_oStream.read((void*)&size, (off_t)sizeof(size));
			if (size <= 0)
			{
				_DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Invalid data, data source may be damaged" );
			}

			tBufferType buffer(size);
			_oStream.read((void*)&buffer[0], size);

			buf.swap(buffer);
		}
		else
			buf.clear();

	}

	
	DWFFileAdapter::tHandleType DWFFileAdapter::save(const DWFFileAdapter::tBufferType& src, DWFFileAdapter::tHandleType/* old*/)
	{
		tHandleType newHandle = kInvalidHandle;

		if (!src.empty())
		{
			assert(_oBuffer.empty() || _bTransacting);

			newHandle = _oNextHandle;
			size_t size = src.size();
			copy((unsigned char*)&size, (unsigned char*)&size + sizeof(size), back_inserter(_oBuffer));
			copy(src.begin(), src.end(), back_inserter(_oBuffer));
			_oNextHandle += (tHandleType)(src.size() + sizeof(size));

			if (!_bTransacting)
				flush();
		}

		return newHandle;
	}

	void DWFFileAdapter::flush()
	{
		if (!_oBuffer.empty())
		{
			_oStream.seek(SEEK_END, 0);
			_oStream.write((const void*)&_oBuffer[0], _oBuffer.size());

			_oBuffer.clear();
		}
	}

	void DWFFileAdapter::begin()
	{
		assert(_oBuffer.empty() && !_bTransacting);
		_bTransacting = true;
	}

	void DWFFileAdapter::commit()
	{
		assert(_bTransacting);

		flush();
		_bTransacting = false;
	}

	void DWFFileAdapter::rollback()
	{
		assert(_bTransacting);
		_bTransacting = false;
		_oNextHandle -= (tHandleType)_oBuffer.size();
		_oBuffer.clear();
	}
}