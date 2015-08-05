#ifndef NETIO_LIBEVENT_HPP
#define NETIO_LIBEVENT_HPP


#include <thread>
#include <atomic>
#include <thread>
#include <map>
#include <array>

#include <event2/event.h>
#include <event2/listener.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include <event2/thread.h>


namespace felix
{
	namespace netio
	{
		const size_t MAX_SOCKET_BUFFER = 64*1024*1024; 


		class LibEventDataSinkServer : public DataSinkServer
		{
		protected:
			struct event_base* ev_base;
			std::thread bg_thread;
			std::atomic_bool running;
			unsigned short m_port;
			
		public:
			LibEventDataSinkServer(Endpoint local_endpoint, DataSinkCallbacks* callbacks) 
			  : DataSinkServer(local_endpoint, callbacks) 
			{
				ev_base = event_base_new();
			}

			~LibEventDataSinkServer()
			{
				event_base_free(ev_base);
			}


			void start(unsigned nthreads=1);
			void stop();
			unsigned short port();
		};


		namespace detail
		{
			class Connection
			{
				static const unsigned NUM_CONNECTIONS = 1;
				std::array<struct evbuffer*, NUM_CONNECTIONS> buffers;
				unsigned current_buffer = 0;

				struct evbuffer* connect(struct event_base* ev_base, 
					Endpoint destination, DataSourceCallbacks* callbacks);

			public:
				Connection(struct event_base* ev_base, Endpoint destination,
				 DataSourceCallbacks* callbacks)
				{
					for(int i=0; i<NUM_CONNECTIONS; i++)
					{
						buffers[i] = connect(ev_base, destination, callbacks);
					}
				}

				struct evbuffer* get()
				{
					current_buffer = (current_buffer+1)%NUM_CONNECTIONS;
					return buffers[current_buffer];
				}

			};
		}

		class LibEventDataSource : public DataSource
		{
		protected:
			struct event_base* ev_base;
			std::thread bg_thread;
			std::map<Endpoint, detail::Connection> connections;

			struct evbuffer* connection_buffer(Endpoint destination);

		public:
			LibEventDataSource(DataSourceCallbacks* callbacks) : DataSource(callbacks) 
			{
				evthread_use_pthreads();
				ev_base = event_base_new();
			}

			~LibEventDataSource()
			{
				event_base_free(ev_base);
			}

			virtual void send_messages(Endpoint destination, 
				std::vector<std::unique_ptr<Message>>& messages);
			virtual void send_message(Endpoint destination, 
				Message&);


			virtual void start(unsigned nthreads=1);
			virtual void stop();
		};

		Endpoint endpoint_from_bufferevent(struct bufferevent *bev);


	}
}

#endif
