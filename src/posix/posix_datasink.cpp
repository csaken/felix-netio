#include "netio/netio.hpp"
#include "posix.hpp"
#include "../serialization.hpp"

static void
run(std::atomic_bool& running, int fd, felix::netio::DataSinkCallbacks* callbacks)
{
	while(running.load())
	{
	  /*		felix::netio::msgheader header;
		ssize_t count = read(fd, &header, sizeof(header));
		assert(count == sizeof(header));

		char* data = new char[header.len];

		ssize_t bytes_read = 0;
		while(bytes_read < header.len)
		{
			count = read(fd, data+bytes_read, header.len-bytes_read);
			if (count == 0)
			{
				std::vector<felix::netio::DataSinkMessage> messages;
				messages.emplace_back(data, bytes_read);
				callbacks->on_data_received_with_error(messages);
				return;
			}
			bytes_read += count;
		}

	  */

	  char* data = new char[1024];
	  ssize_t count = read(fd, data, 1024);

		std::vector<felix::netio::DataSinkMessage> messages;
		messages.emplace_back(data, count);
		callbacks->on_data_received(messages);
	}
}

void 
felix::netio::PosixDataSinkServer::start(unsigned nthreads)
{
	running.store(true);
	bg_thread = std::thread(run, std::ref(running), fd, callbacks);
}

void 
felix::netio::PosixDataSinkServer::stop()
{
	running.store(false);
	bg_thread.join();
	bg_thread = std::thread();
}

unsigned short
felix::netio::PosixDataSinkServer::port()
{
	return m_port;
}
