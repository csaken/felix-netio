#include "catch/catch.hpp"

#include <iostream>
#include <chrono>

#include "netio/netio.hpp"
#include "../src/libevent/libevent.hpp"
#include "utility.hpp"

using namespace felix::netio;

class TestCallbacks : public felix::netio::DataSourceCallbacks
{
public:
	void on_open(felix::netio::Endpoint) {};
	void on_close(felix::netio::Endpoint) {};
	void on_data_send(const felix::netio::Message&) {};
	void on_data_send_with_error(const felix::netio::Message&) {};
};

class TestDataSinkCallbacks : public felix::netio::DataSinkCallbacks
{
public:

	unsigned messages_received = 0;
	unsigned messages_received_with_error = 0;
	unsigned open_sessions = 0;

	virtual void on_open(felix::netio::Endpoint)
	{
		open_sessions++;
	}

	virtual void on_close(felix::netio::Endpoint)
	{
		open_sessions--;
	}

	virtual void on_data_received(const std::vector<felix::netio::DataSinkMessage>& messages)
	{
		messages_received += messages.size();
	}

	virtual void on_data_received_with_error(const std::vector<felix::netio::DataSinkMessage>& messages)
	{
		messages_received_with_error += messages.size();
	}

};

TEST_CASE( "Create libevent data source", "[datasource]" ) {
	TestCallbacks callbacks;
	LibEventDataSource source(&callbacks);
	source.start();
	source.stop();
}

TEST_CASE( "Create libevent data source and send data", "[datasource]" ) {
	
	TestDataSinkCallbacks sink_callbacks;
	LibEventDataSinkServer server(Endpoint("0.0.0.0", 0), &sink_callbacks);
	server.start();


	TestCallbacks callbacks;
	LibEventDataSource source(&callbacks);
	source.start();

	felix::netio::Endpoint destination("127.0.0.1", server.port());

	std::string message_string("Hello, world!");
	std::vector<std::unique_ptr<felix::netio::Message>> messages;
	for(int i=0; i<100000; i++)
		messages.emplace_back(new felix::netio::Message(message_string.c_str(),
			message_string.size()));
	source.send_messages(destination, messages);

	REQUIRE( wait_for_condition_timeout(sink_callbacks.open_sessions, 1) );
	REQUIRE( wait_for_condition_timeout(sink_callbacks.messages_received, 1) );

	source.stop();
	server.stop();
}

