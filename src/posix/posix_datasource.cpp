#include "netio/netio.hpp"
#include "posix.hpp"
#include "../serialization.hpp"

#include <cmath>

void felix::netio::detail::PosixConnection::connect_socket(Endpoint destination)
{
	struct sockaddr_in serverData;
	struct hostent *host;

	struct addrinfo hints;
	struct addrinfo *result, *rp;
	int s;

	char port_buf[32];
	snprintf(port_buf, 32, "%d", destination.endpoint.port());

	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = 0;
	hints.ai_protocol = 0; 

	s = getaddrinfo(destination.endpoint.address().to_string().c_str(),
			port_buf, &hints, &result);
	if (s != 0) {
	  fprintf(stderr, "error: getaddrinfo: %s\n", gai_strerror(s));
	  exit(EXIT_FAILURE);
	}


	for (rp = result; rp != NULL; rp = rp->ai_next) {
	  fd = socket(rp->ai_family, rp->ai_socktype,
		      rp->ai_protocol);
	  if (fd == -1)
            continue;
	  
	  if (connect(fd, rp->ai_addr, rp->ai_addrlen) != -1)
            break;                  /* Success */
	  
	  close(fd);
	}

   	int state = 1;
	setsockopt(fd, IPPROTO_TCP, TCP_CORK, &state, sizeof(state));
}

felix::netio::detail::PosixConnection*
felix::netio::PosixDataSource::connection(Endpoint destination)
{
	if(connections.count(destination) == 0)
	{
		std::unique_ptr<detail::PosixConnection> c(new detail::PosixConnection(destination));
		connections.insert(
			std::map<Endpoint, std::unique_ptr<detail::PosixConnection>>::value_type(
				destination,
				std::move(c)));
	}

    return connections.find(destination)->second.get();
}


void felix::netio::PosixDataSource::send_messages(Endpoint destination, 
	std::vector<std::unique_ptr<Message>>& messages)
{
	for(auto it = std::begin(messages); it != std::end(messages); it++)
	{
		detail::PosixConnection* con = connection(destination);

		Message* m = it->get();
		for(unsigned i=0; i < m->num_fragments(); i++)
		{
			int size = m->fragment_sizes()[i];

			int n = 0;
			while(size > n)
			{ 
				msgheader header;
				header.len = size;
				size_t bytes_written = serialize_to_buffer(con->buf + con->buflen, 
				                                   detail::PosixConnection::POSIX_BUFSIZE - con->buflen,
				                                   m->fragments()[i], header, &n);
				con->buflen += bytes_written;
				if(bytes_written == 0)
				{
					send(con->fd, con->buf, con->buflen, 0);
					con->buflen = 0;
				}

			}
		}
		callbacks->on_data_send(*m);
	}
}


void felix::netio::PosixDataSource::send_message(Endpoint destination, Message& msg)
{
	detail::PosixConnection* con = connection(destination);

	for(unsigned i=0; i < msg.num_fragments(); i++)
	{
		int size = msg.fragment_sizes()[i];

		int n = 0;
		while(size > n)
		{ 
			msgheader header;
			header.len = size;
			size_t bytes_written = serialize_to_buffer(con->buf + con->buflen, 
			                                   detail::PosixConnection::POSIX_BUFSIZE - con->buflen,
			                                   msg.fragments()[i], header, &n);
			con->buflen += bytes_written;
			if(bytes_written == 0)
			{
				send(con->fd, con->buf, con->buflen, 0);
				con->buflen = 0;
			}
		}
	}
	callbacks->on_data_send(msg);
}
