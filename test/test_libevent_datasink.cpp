#include "catch/catch.hpp"

#include <iostream>
#include <chrono>

#include "netio/netio.hpp"
#include "../src/libevent/libevent.hpp"
#include "utility.hpp"

using namespace felix::netio;


class TestDataSinkCallbacks : public DataSinkCallbacks
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

TEST_CASE( "Create libevent server", "[datasink]" ) {

	TestDataSinkCallbacks callbacks;
	LibEventDataSinkServer server(Endpoint("0.0.0.0", 0), &callbacks);
	server.start();

	server.stop();
}

TEST_CASE( "Create libevent server and connect", "[datasink]" ) {

	TestDataSinkCallbacks callbacks;
	LibEventDataSinkServer server(Endpoint("0.0.0.0", 0), &callbacks);
	server.start();

	struct sockaddr_in serverData;
	serverData.sin_family = AF_INET;
	struct hostent *host = gethostbyname( "localhost" );
	bcopy(host->h_addr, &(serverData.sin_addr.s_addr), host->h_length);
	serverData.sin_port = htons( server.port() );

	int serverSocket = socket( PF_INET, SOCK_STREAM, IPPROTO_TCP );

	REQUIRE( connect( serverSocket,
             (struct sockaddr *)&serverData,
             sizeof(serverData)) == 0);

	char buf[] = "Hello, world!";
	REQUIRE( send(serverSocket, buf, strlen(buf), 0) == strlen(buf) );

	server.stop();
}
