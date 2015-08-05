#ifndef NETIO_POSIX_HPP
#define NETIO_POSIX_HPP



#include <thread>
#include <atomic>
#include <memory>

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <netinet/tcp.h>


namespace felix
{
	namespace netio
	{
		class PosixDataSinkServer : public DataSinkServer
		{
		protected:
			unsigned short m_port;
			std::thread bg_thread;
			std::atomic_bool running;
			int fd;

		public:
			PosixDataSinkServer(Endpoint local_endpoint, DataSinkCallbacks* callbacks) 
			  : DataSinkServer(local_endpoint, callbacks) 
			{
			}

			~PosixDataSinkServer()
			{
			}


			void start(unsigned nthreads=1);
			void stop();
			unsigned short port();
		};

		namespace detail
		{

			struct PosixConnection
			{
				static const unsigned POSIX_BUFSIZE = 16384;

				int fd;
				char* buf;
				size_t buflen;

				PosixConnection(Endpoint destination)
				{
					connect_socket(destination);
					buf = new char[POSIX_BUFSIZE];
					buflen = 0;
				}

				~PosixConnection()
				{
					send(fd, buf, buflen, 0);
					shutdown(fd, SHUT_WR);
					close(fd);
					delete[] buf;
				}

				void connect_socket(Endpoint destination);

			private:
				PosixConnection(PosixConnection& other) {}
			};
		}


		class PosixDataSource : public DataSource
		{
		protected:
			std::map<Endpoint, std::unique_ptr<detail::PosixConnection>> connections;

			detail::PosixConnection* connection(Endpoint destination);

		public:
			PosixDataSource(DataSourceCallbacks* callbacks) : DataSource(callbacks) 
			{
			}

			~PosixDataSource()
			{
			}

			virtual void send_messages(Endpoint destination, 
				std::vector<std::unique_ptr<Message>>& messages);
			virtual void send_message(Endpoint destination,
				Message& msg);

			void start(unsigned nthreads=1) {};
			void stop() {};
		};
	}
}




#endif
