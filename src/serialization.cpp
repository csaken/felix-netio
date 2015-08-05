#include "serialization.hpp"

#include <cstring>
#include <algorithm>

size_t
felix::netio::serialize_to_buffer(char* dest, size_t dest_len, const char* src, msgheader header, int* offset)
{
	size_t bytes_written = 0;

	if(*offset==0)
	{
		/* If offset==0, the header needs to be written. We need to have space
		   for the header and at least one byte - otherwise offset will be 0
		   again in the next call. */
		if(dest_len < sizeof(header)+1)	
			return 0;

		memcpy(dest, (char*)(&header), sizeof(header));
		dest += sizeof(header);
		dest_len -= sizeof(header);
		bytes_written += sizeof(header);
	}

	size_t bytes_to_write = std::min(header.len-*offset, (long long unsigned) dest_len);
	memcpy(dest, src+*offset, bytes_to_write);
	*offset += bytes_to_write;
	bytes_written += bytes_to_write;
	
	return bytes_written;
}
