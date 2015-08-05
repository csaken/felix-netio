#include "netio/netio.hpp"
#include "libevent.hpp"
#include <iostream>

#include <event2/listener.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>


static void run(struct event_base* ev_base)
{
	event_base_loop(ev_base, 0);
}


static void timer_cb(evutil_socket_t fd, short what, void *arg)
{

}

static void
read_cb_copy(struct bufferevent *bev, void* ptr)
{
    /* This callback is invoked when there is data to read on bev. */
    struct evbuffer *input = bufferevent_get_input(bev);

   	felix::netio::DataSinkCallbacks* callbacks = (felix::netio::DataSinkCallbacks*)ptr;

	size_t len = evbuffer_get_length(input);

	char* buffer = new char[len];
	
   	evbuffer_lock(input);
	assert(evbuffer_remove(input, buffer, len) > 0);
   	evbuffer_unlock(input);

	std::vector<felix::netio::DataSinkMessage> messages;	
	messages.emplace_back(buffer, len);

	callbacks->on_data_received(messages);
}

static void
event_cb(struct bufferevent *bev, short events, void*)
{
    if (events & BEV_EVENT_ERROR)
            perror("Error from bufferevent");
    if (events & (BEV_EVENT_EOF | BEV_EVENT_ERROR)) {
            bufferevent_free(bev);
    }
}

static void
config_socket(int fd)
{
	int optval;
	setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &optval, sizeof optval);
	setsockopt(fd, IPPROTO_TCP, TCP_CORK, &optval, sizeof optval);
	setsockopt(fd, IPPROTO_TCP, TCP_QUICKACK, &optval, sizeof optval);
}


static void
connection_accepted(struct evconnlistener *listener,
		evutil_socket_t fd, struct sockaddr *addr, int len, void *ptr)
{
	struct event_base *base = evconnlistener_get_base(listener);
	struct bufferevent *bev = bufferevent_socket_new(
                base, fd, BEV_OPT_CLOSE_ON_FREE);

	config_socket(fd);

	bufferevent_setcb(bev, read_cb_copy, NULL, event_cb, ptr); 	

    bufferevent_enable(bev, EV_READ);
    struct evbuffer *input = bufferevent_get_input(bev);
	evbuffer_enable_locking(input, NULL);

	felix::netio::DataSinkCallbacks* callbacks = (felix::netio::DataSinkCallbacks*)ptr;
    callbacks->on_open(felix::netio::endpoint_from_bufferevent(bev));
}


void
felix::netio::LibEventDataSinkServer::start(unsigned nthreads)
{
	struct sockaddr_in sa;
	sa.sin_family = AF_INET;
	sa.sin_port = htons(local_endpoint.endpoint.port());
	in_addr in;
	inet_aton(local_endpoint.endpoint.address().to_string().c_str(), &in);
	sa.sin_addr.s_addr = in.s_addr;

	struct evconnlistener* listener = evconnlistener_new_bind(
		ev_base,
		connection_accepted, this->callbacks,
		LEV_OPT_CLOSE_ON_FREE | LEV_OPT_REUSEABLE | LEV_OPT_THREADSAFE,
		-1,
		(sockaddr*)&sa, sizeof(sa));

	evutil_socket_t socket = evconnlistener_get_fd(listener);
	socklen_t slen = sizeof(sa);
	getsockname(socket, (struct sockaddr *)&sa, &slen);
	this->m_port = ntohs(sa.sin_port);

   	struct timeval one_sec = { 1, 0 };
    struct event *ev = event_new(ev_base, -1, EV_PERSIST, timer_cb, NULL);
    event_add(ev, &one_sec);

	bg_thread = std::thread(run, ev_base);
}

void felix::netio::LibEventDataSinkServer::stop()
{
   	struct timeval one_sec = { 0, 500000 };
	event_base_loopexit(ev_base, &one_sec);
	bg_thread.join();
}

unsigned short felix::netio::LibEventDataSinkServer::port()
{
	return m_port;
}
