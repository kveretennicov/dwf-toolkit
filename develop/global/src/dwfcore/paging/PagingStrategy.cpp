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
#include "dwfcore/paging/PagingStrategy.h"
#include <cassert>

namespace DWFCore
{
	DWFPagingStrategyBase::DWFPagingStrategyBase(DWFPagingAdapter& rPagingAdapter)
		: _pMemoryManager(0)
		, _nHighWater(10240)
		, _nLowWater(5120)
		, _pPagingAdapter(&rPagingAdapter)
	{		
	}

	DWFPagingStrategyBase::~DWFPagingStrategyBase()
	{
		assert(!isInstalled());
	}

	bool DWFPagingStrategyBase::isInstalled() const
	{
		return _pMemoryManager != 0;
	}

	
	size_t DWFPagingStrategyBase::highWater() const
	{
		return _nHighWater;
	}

	size_t DWFPagingStrategyBase::lowWater() const
	{
		return _nLowWater;
	}

	void DWFPagingStrategyBase::highWater(size_t nNewMarker)
	{
		_nHighWater = nNewMarker;

		if (_nLowWater > _nHighWater)
			_nLowWater = _nHighWater;

		if (isInstalled())
			_pMemoryManager->notify();
	}

	void DWFPagingStrategyBase::lowWater(size_t nNewMarker)
	{
		_nLowWater = nNewMarker;

		if (_nLowWater > _nHighWater)
			_nHighWater = _nLowWater;

		if (isInstalled())
			_pMemoryManager->notify();

	}

	DWFPagingAdapter& DWFPagingStrategyBase::pagingAdapter() const
	{
		return *_pPagingAdapter;
	}

	void DWFPagingStrategyBase::_install(DWFMemoryManager& rMMgr)
	{	
		assert(!isInstalled());
		_pMemoryManager = &rMMgr;
	}

	void DWFPagingStrategyBase::_uninstall()
	{	
		assert(isInstalled());
		_pMemoryManager = 0;
	}

	DWFNruStrategy::DWFNruStrategy(DWFPagingAdapter& rAdapter) 
		: DWFPagingStrategyBase(rAdapter)
	{
	}

	void DWFNruStrategy::onAcquire(DWFMemoryManager::tTableInfo& rTableInfo)
	{
		typedef DWFMemoryManager::tTimestampType tTimestampType;
		typedef DWFMemoryManager::tStubInfo tStubInfo;
		typedef DWFPagingAdapter::tHandleType tHandleType;

		assert(isInstalled());
		assert(_nHighWater == rTableInfo._nHighWater);
		assert(_nLowWater == rTableInfo._nLowWater);

		if (rTableInfo._nUsedCount > _nHighWater)
		{
			typedef std::multimap<tTimestampType, tStubInfo*> tTrackerType;
			tTrackerType oTracker;

			tStubInfo* itr = rTableInfo._pUsedHead;
			for (; itr != 0; itr = itr->_next)
			{
				assert(itr->_pObj && itr->_pOwner);
				oTracker.insert(std::make_pair(itr->_lastAccess, itr));
			}
			assert(oTracker.size() == rTableInfo._nUsedCount);

			tTrackerType::iterator first(oTracker.begin()), last(oTracker.begin());

			std::advance(last, rTableInfo._nUsedCount - rTableInfo._nLowWater);

			DWFPagingAdapter::tScopedTransaction autoTransaction(pagingAdapter());
			for (; first != last; ++first)
				first->second->_pOwner->pageOut();
			autoTransaction.setSuccess(true);
		}
	}

	void DWFNruStrategy::onRelease(DWFMemoryManager::tTableInfo& /*rTableInfo*/)
	{
		// Not implemented
	}

}