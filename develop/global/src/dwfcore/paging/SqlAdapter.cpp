#include "dwfcore/paging/SqlAdapter.h"
#include "dwfcore/Exception.h"
#include <cassert>

namespace DWFCore
{
	DWFPagingAdapter* default_adapter = 0;

	DWFSqliteAdapter::DWFSqliteAdapter()
		: _pBTree(0)
	{
		int rc = sqlite3BtreeOpen(0, 0, &_pBTree, BTREE_OMIT_JOURNAL | BTREE_NO_READLOCK);

		if (SQLITE_OK == rc)
		{
			tScopedTransaction oScopedTrans(*this);
			int nTableId;
			rc = sqlite3BtreeCreateTable(_pBTree, &nTableId, BTREE_INTKEY|BTREE_LEAFDATA );
			rc = sqlite3BtreeCursor(_pBTree, nTableId, 1, NULL, 0, &_pCur);
			oScopedTrans.setSuccess();
		}

		if (SQLITE_OK != rc)
		{
			if (_pBTree != 0)
				sqlite3BtreeClose(_pBTree);

			_DWFCORE_THROW( DWFIOException, /*NOXLATE*/L"Open database failed");
		}
	}

	DWFSqliteAdapter::~DWFSqliteAdapter()
	{
		sqlite3BtreeCloseCursor(_pCur);
		sqlite3BtreeClose(_pBTree);
	}

	void DWFSqliteAdapter::load(DWFSqliteAdapter::tHandleType hHandle, DWFSqliteAdapter::tBufferType& buf)
	{
		int res;
		int rc = sqlite3BtreeMoveto(_pCur, 0, (i64)hHandle, true, &res);
		if (SQLITE_OK == rc)
		{
			assert(res == 0);
			u32 size;
			sqlite3BtreeDataSize(_pCur, &size);
			buf.resize(size);
			sqlite3BtreeData(_pCur, 0, size, &buf[0]);
		}
		else
		{
			_DWFCORE_THROW( DWFIOException, /*NOXLATE*/L"Move cursor failed");
		}		
	}

	DWFSqliteAdapter::tHandleType DWFSqliteAdapter::save(const DWFSqliteAdapter::tBufferType& src, DWFSqliteAdapter::tHandleType old)
	{
		static tHandleType _nextHandle = 1;

		tHandleType  new_handle = old == 0 ? _nextHandle++ : old;

		int rc = sqlite3BtreeInsert(_pCur, 0, (i64)new_handle, &src[0], (int)src.size(), old == 0);
		if (SQLITE_OK != rc)
		{
			_DWFCORE_THROW( DWFIOException, /*NOXLATE*/L"Insertion failed");
		}
		return new_handle;
	}

	void DWFSqliteAdapter::begin()
	{
		sqlite3BtreeBeginTrans(_pBTree, 1);
	}

	void DWFSqliteAdapter::commit()
	{
		sqlite3BtreeCommit(_pBTree);
	}
	void DWFSqliteAdapter::rollback()
	{
		sqlite3BtreeRollback(_pBTree);
	}
}