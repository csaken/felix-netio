#include <iostream>

#include "netio/netio.hpp"
#include "libevent.hpp"

static void
run(struct event_base* ev_base)
{
	event_base_loop(ev_base, 0);
}

static void timer_cb(evutil_socket_t fd, short what, void *arg)
{

}

void
felix::netio::LibEventDataSource::start(unsigned nthreads)
{
	struct timeval one_sec = { 1, 0 };
    struct event *ev = event_new(ev_base, -1, EV_PERSIST, timer_cb, NULL);
    event_add(ev, &one_sec);

	bg_thread = std::thread(run, ev_base);
}


void
felix::netio::LibEventDataSource::stop()
{
	struct timeval one_sec = { 0, 500000 };
	event_base_loopexit(ev_base, &one_sec);
	bg_thread.join();
}


static void
eventcb(struct bufferevent *bev, short events, void *ptr)
{
	felix::netio::DataSourceCallbacks* callbacks = (felix::netio::DataSourceCallbacks*)ptr;
    if (events & BEV_EVENT_CONNECTED) {
         /* We're connected */
    	callbacks->on_open(felix::netio::endpoint_from_bufferevent(bev));
    } else if (events & BEV_EVENT_ERROR) {	
         /* An error occured while connection. */
    	callbacks->on_close(felix::netio::endpoint_from_bufferevent(bev));
    }
}

static void
config_socket(int fd)
{
	int optval = 1;
	//setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &optval, sizeof optval);
	setsockopt(fd, IPPROTO_TCP, TCP_CORK, &optval, sizeof optval);
	//setsockopt(fd, IPPROTO_TCP, TCP_QUICKACK, &optval, sizeof optval);
}

struct evbuffer* felix::netio::detail::Connection::connect(
	struct event_base* ev_base, Endpoint destination, DataSourceCallbacks* callbacks)
{
	struct sockaddr_in sin;
	memset(&sin, 0, sizeof(sin));
   	sin.sin_family = AF_INET;
   	sin.sin_port = htons(destination.endpoint.port());

	in_addr in;
	inet_aton(destination.endpoint.address().to_string().c_str(), &in);
   	sin.sin_addr.s_addr = in.s_addr;

   	struct bufferevent* bev = bufferevent_socket_new(ev_base, -1, BEV_OPT_CLOSE_ON_FREE);
   	bufferevent_setcb(bev, NULL, NULL, eventcb, callbacks);
   	if(bufferevent_socket_connect(bev, (struct sockaddr *)&sin, sizeof(sin)))
   	{
   		std::cerr << "Could not connect to " << destination.endpoint << std::endl;
   	}

   	evutil_socket_t fd = event_get_fd((event*)bev);
   	config_socket(fd);

	struct evbuffer* buf = bufferevent_get_output(bev);
	evbuffer_enable_locking(buf, NULL);	
	return buf;			
}


struct evbuffer* 
felix::netio::LibEventDataSource::connection_buffer(Endpoint destination)
{
	if(connections.count(destination) == 0)
	{
		connections.insert(std::map<Endpoint, detail::Connection>::value_type(
			destination, detail::Connection(ev_base, destination, callbacks)));
	}

    return connections.find(destination)->second.get();
}


void
felix::netio::LibEventDataSource::send_messages(Endpoint destination,
	std::vector<std::unique_ptr<Message>>& messages)
{
	for(auto it = std::begin(messages); it != std::end(messages); it++)
	{
		struct evbuffer* buf = connection_buffer(destination);

		Message* m = it->get();
		for(int i=0; i < m->num_fragments(); i++)
		{
			while(evbuffer_get_length(buf) >= MAX_SOCKET_BUFFER)
				std::this_thread::sleep_for (std::chrono::milliseconds(5));;

			evbuffer_lock(buf);
			evbuffer_add(buf, m->fragments()[i], m->fragment_sizes()[i]);
			/*evbuffer_add_reference(buf,
    			m->data[i], m->sizes[i],
    			NULL, NULL);*/
			evbuffer_unlock(buf);
		}
		callbacks->on_data_send(*m);
	}
}

void
felix::netio::LibEventDataSource::send_message(Endpoint destination, Message& m)
{
	Message my_msg = std::move(m);
}

