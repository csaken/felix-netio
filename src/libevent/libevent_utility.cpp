#include "netio/netio.hpp"
#include "libevent.hpp"

felix::netio::Endpoint
felix::netio::endpoint_from_bufferevent(struct bufferevent *bev)
{
	int fd = bufferevent_getfd(bev);
	struct sockaddr_in addr;
	socklen_t addrlen = sizeof(addr);
	getpeername(fd, (sockaddr*)&addr, &addrlen);
	return felix::netio::Endpoint(ntohl(addr.sin_addr.s_addr), addr.sin_port);
}
