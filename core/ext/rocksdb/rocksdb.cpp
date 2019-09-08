#include "../../os/platform.h"
#include "rocksdb.inc"

#include "rocksdb.h"

namespace ext
{

const WriteOptions*	RocksDB::WriteOptionsFastRisky = nullptr;
const WriteOptions*	RocksDB::WriteOptionsDefault = nullptr;
const WriteOptions*	RocksDB::WriteOptionsRobust = nullptr;
const ReadOptions*	RocksDB::ReadOptionsDefault = nullptr;

namespace _details
{

struct __InitRocksDBOptions
{
	struct WriteOptionsMore: public rocksdb::WriteOptions
	{	WriteOptionsMore(int mode)
		{	switch(mode)
			{
			case 1:	sync = false;	disableWAL = true; break;
			case 2:	sync = true;	disableWAL = false; break;
			}
		}
	};

	__InitRocksDBOptions()
	{
		static const WriteOptionsMore	_WriteOptionsDefault = 0;
		static const WriteOptionsMore	_WriteOptionsFastRisky = 1;
		static const WriteOptionsMore	_WriteOptionsRobust = 2;
		
		RocksDB::WriteOptionsFastRisky = &_WriteOptionsFastRisky;
		RocksDB::WriteOptionsDefault = &_WriteOptionsDefault;
		RocksDB::WriteOptionsRobust = &_WriteOptionsRobust;

		static const ReadOptions	_ReadOptionsDefault;
		RocksDB::ReadOptionsDefault = &_ReadOptionsDefault;
	}
};

__InitRocksDBOptions	g_InitRocksDBOptions;

}} // namespace ext::_details