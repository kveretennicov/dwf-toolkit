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
#include "dwfcore/paging/MemoryManager.h"
#include "dwfcore/paging/PagingStrategy.h"
#include "dwfcore/paging/SqlAdapter.h"
#include <cassert>
#include "dwfcore/Synchronization.h"
#include <cstdio>

namespace DWFCore
{
	struct _scoped_lock
	{
		_scoped_lock( DWFThreadMutex& pMutex ) : _pMutex(pMutex) { _pMutex.lock(); }
		~_scoped_lock() { _pMutex.unlock(); }
		DWFThreadMutex& _pMutex;
	private:
		_scoped_lock(const _scoped_lock&);
		_scoped_lock& operator=(const _scoped_lock&);
	};

	/*	Because the size of memory page may be 4K(4096) bytes, so, a appropriate block size can help to use memory efficiently.
		here, kDefaultBlockSize = 4094, the reserved memory( 2 * sizeof(tStubInfo)) can be used store tBlock itself,
		and deal with alignment and system memory management.
	*/
	const size_t kDefaultBlockSize = 4094;	
	const size_t kDefaultHighWater = kDefaultBlockSize;
	const size_t kDefaultLowWater = kDefaultHighWater / 10;

	_DWFCORE_API DWFMemoryManager::DWFMemoryManager() : _oInfo() , _pStrategy(0)
	{
		_pMutex = DWFCORE_ALLOC_OBJECT( DWFThreadMutex );
		_pMutex->init();
		
		_oInfo._nBlockSize = kDefaultBlockSize;

		// DWFSqliteAdapter is faster than DWFFileAdapter
		static DWFCore::DWFSqliteAdapter oAdapter;
		static DWFCore::DWFNruStrategy oStrategy(oAdapter);
		oStrategy.highWater(kDefaultHighWater);
		oStrategy.lowWater(kDefaultLowWater);
		cacheStrategy(&oStrategy);
	}

	///the implementaion of singleton 
	_DWFCORE_API DWFMemoryManager& DWFMemoryManager::Instance(){
		static DWFMemoryManager kInstance;
		return kInstance;
	}

	_DWFCORE_API DWFMemoryManager::tTimestampType DWFMemoryManager::Now()
	{
		static tTimestampType kCounter = 1;
		return kCounter++;
	}

	_DWFCORE_API void DWFMemoryManager::cacheStrategy(DWFPagingStrategyBase* cs) 
	{ 
		_scoped_lock oLock( *_pMutex );

		if(_pStrategy && _pStrategy->isInstalled())
		{
			_pStrategy->_uninstall();
		}

		_pStrategy = cs;

		if (_pStrategy)
		{
			_oInfo._nHighWater = cs->highWater();
			_oInfo._nLowWater = cs->lowWater();
			_pStrategy->_install(*this);

			if (_oInfo._nUsedCount > _oInfo._nHighWater)
				_pStrategy->onAcquire(_oInfo);

			if (_oInfo._nLowWater > _oInfo._nUsedCount * 2)			
				_pStrategy->onRelease(_oInfo);
		}
	}

	_DWFCORE_API DWFPagingStrategyBase* DWFMemoryManager::cacheStrategy() const 
	{ 
		_scoped_lock oLock( *_pMutex );
		return _pStrategy;
	}

	_DWFCORE_API void DWFMemoryManager::notify()
	{
		_scoped_lock oLock( *_pMutex );
		
		if (_pStrategy)
		{
			_oInfo._nHighWater = _pStrategy->highWater();
			_oInfo._nLowWater = _pStrategy->lowWater();

			if (_oInfo._nUsedCount > _oInfo._nHighWater)
				_pStrategy->onAcquire(_oInfo);

			if (_oInfo._nLowWater > _oInfo._nUsedCount * 2)			
				_pStrategy->onRelease(_oInfo);
		}
	}

	_DWFCORE_API DWFPagingAdapter* DWFMemoryManager::pagingAdapter() const
	{
		return _pStrategy == 0 ? 0 : &_pStrategy->pagingAdapter();
	}

	_DWFCORE_API void DWFMemoryManager::_clear()
	{
		_scoped_lock oLock( *_pMutex );

		for (tBlock * itr = _oInfo._pBlockHead; itr != 0; )
		{
			tBlock * _next = itr->_next;

			char *tmp = (char*)itr;
			DWFCORE_FREE_MEMORY(tmp);

			itr = _next;
		}
	}

	_DWFCORE_API DWFMemoryManager::~DWFMemoryManager()
	{
		assert(_oInfo._nUsedCount == 0);
		if (_pStrategy != 0)
			_pStrategy->_uninstall();

		_clear();

		if (_pMutex)
		{
			_pMutex->destroy();
			DWFCORE_FREE_OBJECT( _pMutex );
		}
	}

	///acquire a tStubInfo node from memory management object(from DWFMemoryManager singleton)
	_DWFCORE_API DWFMemoryManager::tStubInfo* DWFMemoryManager::acquire()
	{
		_scoped_lock oLock( *_pMutex );

		if (_pStrategy && _oInfo._nUsedCount > _oInfo._nHighWater)		///if paging strategy object is installed, call the event hHandle.
			_pStrategy->onAcquire(_oInfo);

		//if no free tStubInfo node, allocate a large tBlock and initialize it
		if (_oInfo._pFreeHead == 0)		
		{
			assert(_oInfo._nFreeCount == 0);
			assert(_oInfo._nBlockSize > 0);

			///allocate a large tBlock
			tBlock * pNewBlock = (tBlock * ) DWFCORE_ALLOC_MEMORY(char, (sizeof (tBlock) + sizeof(tStubInfo) * (_oInfo._nBlockSize - 1)));

			///arrange the _Nodes of the tBlock into a unidirectional linked list
			pNewBlock->_Nodes[0]._prev = 0;

			for (size_t i = 1; i < _oInfo._nBlockSize; ++i)
			{
				pNewBlock->_Nodes[i - 1]._next = &pNewBlock->_Nodes[i];
				pNewBlock->_Nodes[i]._prev = &pNewBlock->_Nodes[i - 1];
			}

			pNewBlock->_Nodes[_oInfo._nBlockSize - 1]._next = 0;

			_oInfo._pFreeHead = pNewBlock->_Nodes; ///< _pFreeHead point to the head of the bidirectional linked list.

			pNewBlock->_next = _oInfo._pBlockHead;
			pNewBlock->_size = _oInfo._nBlockSize;

			_oInfo._pBlockHead = pNewBlock;
			_oInfo._nFreeCount += _oInfo._nBlockSize;
		}

		assert(_oInfo._pFreeHead != 0);

		///get and remove the first node from free list
		tStubInfo* result = _oInfo._pFreeHead;
		_oInfo._pFreeHead = _oInfo._pFreeHead->_next;
		if (_oInfo._pFreeHead != 0)
			_oInfo._pFreeHead->_prev = 0;

		///insert node at the front of used list
		result->_next = _oInfo._pUsedHead;
		if (_oInfo._pUsedHead != 0)
			_oInfo._pUsedHead->_prev = result;
		_oInfo._pUsedHead = result;

		///initialize the node
		tStubInfo::reset(*result);
		--_oInfo._nFreeCount;
		++_oInfo._nUsedCount;		

		return result;
	}

	_DWFCORE_API void DWFMemoryManager::release(tStubInfo* p)
	{
		_scoped_lock oLock( *_pMutex );
		assert(p);

		///remove from used linked list
		if (p->_prev == 0)
			_oInfo._pUsedHead = p->_next;
		else
			p->_prev->_next = p->_next;
		if (p->_next != 0)
			p->_next->_prev = p->_prev;


		///insert at the front of the free linked list
		p->_prev = 0;
		p->_next = _oInfo._pFreeHead;		
		if (p->_next != 0)
			p->_next->_prev = p;
		_oInfo._pFreeHead = p;

		++_oInfo._nFreeCount;
		--_oInfo._nUsedCount;

		if (_pStrategy && _oInfo._nLowWater > _oInfo._nUsedCount * 2) ///if paging strategy object is installed, call the event hHandle.
			_pStrategy->onRelease(_oInfo);
	}

	_DWFCORE_API DWFMemoryManager::tStubInfo* DWFMemoryManager::tInfoHolderBase::acquire() const
	{
		return DWFMemoryManager::Instance().acquire();
	}

	_DWFCORE_API void DWFMemoryManager::tInfoHolderBase::release(DWFMemoryManager::tStubInfo* pInfo) const
	{
		DWFMemoryManager::Instance().release(pInfo);
	}

	_DWFCORE_API DWFPagingAdapter* DWFMemoryManager::tInfoHolderBase::pagingAdapter() const
	{
		return DWFMemoryManager::Instance().pagingAdapter();
	}

	_DWFCORE_API DWFMemoryManager::tTimestampType DWFMemoryManager::tInfoHolderBase::Now()
	{
		return DWFMemoryManager::Now();
	}

}
