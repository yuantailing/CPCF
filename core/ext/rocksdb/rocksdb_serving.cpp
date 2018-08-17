#include "rocksdb_serving.h"


namespace rocksdb
{


bool RocksDBHandler::Open(LPCSTR dbpath, bool force_new)
{
	if(force_new)
		os::File::RemovePath(dbpath);

	return _DB.Open(dbpath);
}


bool RocksDBHandler::OnRequest(inet::HttpResponse& resp)
{
	static const rt::CharacterSet keycheck("\" {}':");

	if(!_DB.IsOpen())
	{	resp.SendHttpError(503);
		return true;
	}

	thread_local std::string  val;

	rt::String_Ref q = resp.GetLnPath(this);

	if(resp.Body.IsEmpty())
	{
		if(q == rt::SS("/get"))
		{
			rt::String_Ref k = resp.GetQueryParam(rt::SS("key"));
			if(k.IsEmpty())return false;
			if(k.FindCharacter(keycheck)>=0)return false;

			if(_DB.Get(k, val, &_Httpd->_ReadOpt))
			{	resp.Send(val.c_str(), (int)val.size(), inet::TinyHttpd::MIME_STRING_TEXT);
			}
			else
			{	resp.SendHttpError(404);
			}

			return true;
		}
		else if(q == rt::SS("/set"))
		{
			rt::String_Ref k = resp.GetQueryParam(rt::SS("key"));
			rt::String_Ref v = resp.GetQueryParam(rt::SS("val")).TrimSpace();
			if(k.IsEmpty())return false;
			if(k.FindCharacter(keycheck)>=0)return false;

			if(_DB.Set(k, v, &_Httpd->_WriteOpt))
			{	resp.Send("{\"ret\":true}", 12, inet::TinyHttpd::MIME_STRING_TEXT);
			}
			else
			{	resp.SendHttpError(500);
			}
			
			return true;
		}
		else if(q == rt::SS("/list"))
		{
			rt::String_Ref k = resp.GetQueryParam(rt::SS("key"));
			int n = resp.GetQueryParam<int>(rt::SS("num"), rt::TypeTraits<int>::MaxVal());

			resp.SendChuncked_Begin("application/json");
			resp.SendChuncked("[\n");
			rocksdb::RocksDB::Cursor c = _DB.Scan(SliceValue(k), &_Httpd->_ReadOpt);
			for(int i=0; i<n && c.IsValid(); i++)
			{
				resp.SendChuncked(i==0?"{ \"":",\n{ \"");
				resp.SendChuncked(c.Key().ToString());
				resp.SendChuncked("\" : ");

				rt::String_Ref v = c.Value().ToString();

				if(v[0] == '{')
				{
					resp.SendChuncked(v);
					resp.SendChuncked(" }");
				}
				else
				{	resp.SendChuncked("\"");
					resp.SendChuncked(rt::JsonEscapeString(v));
					resp.SendChuncked("\" }");
				}

				c.Next();
			}
			resp.SendChuncked("\n]");
			resp.SendChuncked_End();

			return true;
		}
		else if(q.GetLength() < 2)
		{
			resp.Send("{\"db_online\":true}", 18, inet::TinyHttpd::MIME_STRING_TEXT);
			return true;
		}
		else return false;
	}
	else
	{
		if(q == rt::SS("/set"))
		{
			rt::String_Ref k = resp.GetQueryParam(rt::SS("key"));
			if(k.IsEmpty())return false;

			if(_DB.Set(k, resp.Body, &_Httpd->_WriteOpt))
			{	resp.Send("{\"ret\":true}", 12, inet::TinyHttpd::MIME_STRING_TEXT);
			}
			else
			{	resp.SendHttpError(500);
			}
			
			return true;
		}

		return false;
	}
}

void RocksDBHandler::Close()
{
	_DB.Close();
}

bool RocksDBServe::Start(const rt::String_Ref& dbs, int port, bool force_new, bool less_consistent)
{
	_WriteOpt.disableWAL = less_consistent;

	rt::String_Ref dblist[128];

	UINT co = dbs.Split(dblist, sizeofArray(dblist), ";,");
	if(co == 0)return false;
	if(!_DBs.SetSize(co))return false;

	inet::LPHTTPENDPOINT * eps = (inet::LPHTTPENDPOINT *)_alloca(sizeof(inet::LPHTTPENDPOINT)*co);

	for(UINT i=0; i<co; i++)
	{
		rt::String dbn(dblist[i]);

		if(_DBs[i].Open(dbn, force_new))
		{
			_LOG("/"<<dblist[i].GetFilename()<<" = "<<dblist[i]);
		}
		else
		{	_LOG_WARNING("ERR: failed to create "<<dblist[i]);
		}

		_DBs[i].SetEndPoint(rt::SS("/") + dbn.GetFilename());
		_DBs[i]._Httpd = this;
		eps[i] = &_DBs[i];
	}

	if(!_Httpd.SetEndpoints(eps, (UINT)_DBs.GetSize()))
	{	
		_DBs.SetSize();
		return false;
	}

	inet::InetAddr local;
	local.SetAsAny();
	local.SetPort(port);

	return _Httpd.Start(local);
}

void RocksDBServe::Stop()
{
	_Httpd.Stop();
}


} // namespace rocksdb
