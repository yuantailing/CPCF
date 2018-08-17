#include "rocksdb.h"
#include "../../shared_api/inet/tinyhttpd.h"

namespace rocksdb
{


class RocksDBHandler:public inet::HttpHandler<RocksDBHandler>
{
	friend class RocksDBServe;
	RocksDB			_DB;
	RocksDBServe*	_Httpd;

protected:
	bool OnRequest(inet::HttpResponse& resp);

public:
	~RocksDBHandler(){ Close(); }
	bool Open(LPCSTR dbpath, bool force_new);
	void Close();
};


class RocksDBServe
{
	friend class RocksDBHandler;

	ReadOptions					_ReadOpt;
	WriteOptions				_WriteOpt;
	inet::TinyHttpd				_Httpd;
	rt::Buffer<RocksDBHandler>	_DBs;

public:
	bool Start(const rt::String_Ref& dbs, int port, bool force_new = false, bool less_consistent = false);
	void Stop();
};


} // namespace rocksdb
