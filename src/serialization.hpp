#ifndef FELIX_NETIO_SERIALIZATION_HPP
#define FELIX_NETIO_SERIALIZATION_HPP

#include <unistd.h>

namespace felix
{
	namespace netio
	{

		struct msgheader
		{
			unsigned long long len;
		};

		/**
		 * Serializes an input buffer (given in src) to an output buffer. This
		 * will write a header to dest followed by the actual content. The
		 * size of the destination buffer is given in dest_len. 
		 *
		 * The function returns the number of bytes of the source buffer that
		 * have been serialized. If the serialized version of the source
		 * buffer is larger than the size of the destination buffer this
		 * number can be smaller than len and subsequent calls are necessary.
		 *
		 * The function may have to be called sevaral times with different
		 * offsets when the destination buffer cannot fit the entire message.
		 * The header is only written when offset==0. In the case offset==0,
		 * the destination buffer should at least fit the header (sizeof len)
		 * plus 1 byte.
		 */
		size_t serialize_to_buffer(char* dest, size_t dest_len, const char* src, msgheader header, int* offset);
 
	}
}

#endif
