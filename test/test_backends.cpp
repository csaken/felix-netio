#include "catch/catch.hpp"
#include "netio/netio.hpp"
#include "../src/posix/posix.hpp"
#include "../src/libevent/libevent.hpp"


class TestCallbacks : public felix::netio::DataSourceCallbacks
{
public:
	void on_open(felix::netio::Endpoint) {};
	void on_close(felix::netio::Endpoint) {};
	void on_data_send(const felix::netio::Message&) {};
	void on_data_send_with_error(const felix::netio::Message&) {};
};

TEST_CASE( "Create POSIX backend", "[backends]" ) {
	TestCallbacks callbacks;
	std::unique_ptr<felix::netio::DataSource> datasource = felix::netio::make_datasource("posix", &callbacks);

	REQUIRE( typeid(*datasource) == typeid(felix::netio::PosixDataSource) );
}

TEST_CASE( "Create libevent backend", "[backends]" ) {
	TestCallbacks callbacks;
	std::unique_ptr<felix::netio::DataSource> datasource = felix::netio::make_datasource("libevent", &callbacks);

	REQUIRE( typeid(*datasource) == typeid(felix::netio::LibEventDataSource) );
}

TEST_CASE( "libevent is not posix", "[backends]" ) {
	TestCallbacks callbacks;
	std::unique_ptr<felix::netio::DataSource> d1 = felix::netio::make_datasource("posix", &callbacks);
	std::unique_ptr<felix::netio::DataSource> d2 = felix::netio::make_datasource("libevent", &callbacks);

	REQUIRE( typeid(*d1) != typeid(*d2) );
}
