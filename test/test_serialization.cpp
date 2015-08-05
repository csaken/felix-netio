#include "catch/catch.hpp"

#include "../src/serialization.hpp"

using namespace felix::netio;

TEST_CASE( "small buffer", "[serialization]" ) {
	char buffer[1024];
	const char* src = "xxxx";
	msgheader header = {};
	header.len = 4;


	int offset = 0;
	size_t bytes_written = serialize_to_buffer(buffer, sizeof buffer, src, header, &offset);

	REQUIRE ( bytes_written == sizeof(size_t)+4 );
	REQUIRE ( offset == 4 );

	bytes_written = serialize_to_buffer(buffer, sizeof buffer, src, header, &offset);

	REQUIRE ( bytes_written == 0 );
	REQUIRE ( offset == 4 );

}


TEST_CASE( "buffer too small for header", "[serialization]" ) {
	char buffer[4];
	const char* src = "xxxx";
	msgheader header = {};
	header.len = 4;


	int offset = 0;
	size_t bytes_written = serialize_to_buffer(buffer, sizeof buffer, src, header, &offset);

	REQUIRE ( bytes_written == 0 );
	REQUIRE ( offset == 0 );

}

TEST_CASE( "large buffer", "[serialization]" ) {
	char buffer[12];
	const char* src = "xxxx";
	msgheader header = {};
	header.len = 4;


	int offset = 0;
	size_t bytes_written = serialize_to_buffer(buffer, sizeof buffer, src, header, &offset);

	REQUIRE ( bytes_written == sizeof(size_t)+4 );
	REQUIRE ( offset == 4 );

}

