#include "netio/netio.hpp"

#include "libevent/libevent.hpp"
#include "posix/posix.hpp"


using namespace felix::netio;

#define DECLARE_DATASINK(NAME, CLS)                       \
  if(backend == NAME) {                                   \
  	return std::unique_ptr<felix::netio::DataSinkServer>( \
 		new CLS(endpoint, cb));  }

#define DECLARE_DATASOURCE(NAME, CLS)                     \
  if(backend == NAME) {                                   \
  	return std::unique_ptr<felix::netio::DataSource>(     \
 		new CLS(cb));  }


std::unique_ptr<DataSinkServer>
felix::netio::make_datasink(std::string backend, Endpoint endpoint, DataSinkCallbacks* cb)
{
	DECLARE_DATASINK("posix", felix::netio::PosixDataSinkServer)
	DECLARE_DATASINK("libevent", felix::netio::LibEventDataSinkServer) 

	return nullptr;
}

std::unique_ptr<DataSource>
felix::netio::make_datasource(std::string backend, DataSourceCallbacks* cb)
{
	DECLARE_DATASOURCE("posix", felix::netio::PosixDataSource)
	DECLARE_DATASOURCE("libevent", felix::netio::LibEventDataSource)

	return nullptr;
}
