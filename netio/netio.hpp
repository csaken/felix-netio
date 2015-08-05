#ifndef NETIO_HPP
#define NETIO_HPP

#include <vector>
#include <set>
#include <string>
#include <map>
#include <thread>
#include <atomic>
#include <memory>
#include <functional>

#include <boost/asio.hpp>


namespace felix
{
    namespace netio
    {

        /**
         * A message is transferred from a `DataSource` object to a `DataSink` object. It represents
         * a FELIX chunk plus metadata.
         */
        class Message
        {
            public:
   				typedef std::function<void(void)> cleanup_fn;

                Message(const char* d, size_t size, cleanup_fn hook=NULL);
                Message(const std::vector<const char*> data, const std::vector<size_t> sizes, cleanup_fn hook=NULL);
                Message(const char* const* data, const size_t* sizes, const size_t n, cleanup_fn hook=NULL);
                Message(const std::string& str, cleanup_fn hook=NULL);
                Message(const std::vector<boost::asio::mutable_buffer>& buffers, cleanup_fn hook=NULL);
                Message(const Message& msg);
                Message(Message&& other);
                ~Message();
                Message& operator= (const Message& other);


                size_t size(void) const;
                char* data_copy(void) const;

                const char* const* fragments() const;
				const size_t* fragment_sizes() const;
				size_t num_fragments() const;

			private:
				std::vector<const char*> data;
				std::vector<size_t> sizes;

				static const size_t MAX_STATIC_FRAGMENTS = 32;
				const char* fragment_data_array[MAX_STATIC_FRAGMENTS];
				size_t fragment_sizes_array[MAX_STATIC_FRAGMENTS];

				size_t n_fragments;
				bool use_static_array;

				cleanup_fn cleanup_hook;
        };


        /**  
         * Class representing incoming messages on a DataSinkServer. The object
         * takes ownership of the incoming data, i.e., the referenced memory is
         * destroyed using delete[] when the object is deleted.
         */
        class DataSinkMessage
        {
            private:
                char* data;
                size_t len;
            public:
                DataSinkMessage(char* data, const size_t len);
                DataSinkMessage(DataSinkMessage&& other);
                ~DataSinkMessage(void);

                size_t size(void) const;
        };



		/**
		 * Describes a TCP/IP endpoint. It is stored as a combination of ip address and port.
 		*/
		struct Endpoint
		{
			boost::asio::ip::tcp::endpoint endpoint; 

			Endpoint(boost::asio::ip::tcp::endpoint ep) 
			    : endpoint(ep) {}

			Endpoint(std::string address, unsigned short port)
			    : endpoint(boost::asio::ip::address::from_string(address), port) {}

			Endpoint(unsigned long address, unsigned short port)
				: endpoint(boost::asio::ip::address_v4(address), port) {}

			bool operator<(Endpoint& other) const
			{
				return endpoint < other.endpoint;
			}

			bool operator<(const Endpoint& other) const
			{
				return endpoint < other.endpoint;	
			}
		};



		/**
		 * An abstract class with callback functions to be implemented by the user in subclasses. 
		 *
		 * Subclasses should implement the `on_open`, `on_close`, `on_data_received` and
		 * `on_data_received_with_error` callbacks. These callbacks are called by the
		 * asyncmsg framework whenever new event data from a FELIX device arrived. 
		 * *The callbacks have to be implemented in a thread-safe manner.*
		 */
		class DataSinkCallbacks
		{
		public:
			/**
			 * Callback that is called when a new FELIX connection is opened.
			 * Has to be implemented thread-safe.
			 */
			virtual void on_open(felix::netio::Endpoint endpoint) = 0;

			/**
			 * Callback that is called when a FELIX connection is closed.
			 * Has to be implemented thread-safe.
			 */
			virtual void on_close(felix::netio::Endpoint endpoint) = 0;

			/**
			 * Callback that is called when new data arrives on a FELIX connection.
			 * Has to be implemented thread-safe.
			 */
			virtual void on_data_received(const std::vector<DataSinkMessage>& messages) = 0;

			/**
			 * Callback that is called when an error occurs while accepting new data.
			 * Has to be implemented thread-safe.
			 */
			virtual void on_data_received_with_error(const std::vector<DataSinkMessage>& messages) = 0;
		};



		/**
		 * A server that uses asyncmsg to wait for incoming connections from FELIX devices. 
		 *
		 * The server will start accepting connections when `start` is called. This will spawn
		 * one or multiple threads that handle connections and incoming data. 
		 */
		class DataSinkServer
		{
		protected:
			Endpoint local_endpoint;
			DataSinkCallbacks* callbacks;

		public:
			DataSinkServer(Endpoint local_endpoint, DataSinkCallbacks* callbacks) 
			  : local_endpoint(local_endpoint), callbacks(callbacks) {}

			/**
			 * Starts accepting incoming connections and data processing. This will spawn
			 * or more new threads that run in background.
			 */
			virtual void start(unsigned nthreads=1) = 0;

 			/**
			 * Stops background threads and closes networks connections.
			 */
			virtual void stop() = 0;

			/**
			 * Returns the port that the server is listening on, or 0 if not listening.
			 */
			virtual unsigned short port() = 0;
		};


		/**
		 * An abstract class with callback functions to be implemented by the user in subclasses. 
		 *
		 * Subclasses should implement the `on_open`, `on_close`, `on_data_send` and
		 * `on_data_send_with_error` callbacks. These callbacks are called by the
		 * asyncmsg framework whenever new event data from a FELIX device is sent. 
		 * *The callbacks have to be implemented in a thread-safe manner.*
		 */
		class DataSourceCallbacks
		{
		public:
			/**
			 * Callback that is called when a new FELIX connection is opened.
			 * Has to be implemented thread-safe.
			 */
			virtual void on_open(felix::netio::Endpoint endpoint) = 0;

			/**
			 * Callback that is called when a FELIX connection is closed.
			 * Has to be implemented thread-safe.
			 */
			virtual void on_close(felix::netio::Endpoint endpoint) = 0;

			/**
			 * Callback that is called when data was successfully sent on a FELIX connection.
			 * Has to be implemented thread-safe.
			 */
			virtual void on_data_send(const felix::netio::Message& messages) = 0;

			/**
			 * Callback that is called when an error occurs while sending data.
			 * Has to be implemented thread-safe.
			 */
			virtual void on_data_send_with_error(const felix::netio::Message& messages) = 0;
		};

		/**
		 * An abstract class based on asyncmsg to be used to send data to DataSink servers. 
		 * 
		 * Data messages can be send with `send_messages`. When the operation has been
		 * completed, the `on_sent_data` or `on_sent_data_with_error` callbacks are called.
		 * The callbacks have to be implemented thread-safe.
		 *
		 * One or more threads are used for communication in the background. They can be
		 * started and stopped with the `start()` and `stop()` methods.
		 */
		class DataSource
		{
		protected:
			DataSourceCallbacks* callbacks;

		public:
			DataSource(DataSourceCallbacks* callbacks) : callbacks(callbacks) { }

			/**
			 * Send messages to the endpoint specified by `address` and `port`. The method
			 * is asynchronous and does not block. On successful completion `on_sent_data`
			 * is called; `on_sent_data_with_error` is called when an error occurs.
			 */
			virtual void send_messages(
				Endpoint destination, std::vector<std::unique_ptr<Message>>& messages) = 0;

			/**
			 * Send message to the specified endpoint. The method may or may not block
			 * depending on the backend that is used. On successful completion `on_sent_data`
			 * is called; `on_sent_data_with_error` is called when an error occurs.
			 */
			virtual void send_message(Endpoint destination, Message& msg) = 0;


			/**
			 * Start background threads and networking.
			 */
			virtual void start(unsigned nthreads=1) = 0;

			/**
			 * Stop background threads.
			 */
			virtual void stop() = 0;
		};

		std::unique_ptr<DataSinkServer> make_datasink(std::string backend, Endpoint endpoint, DataSinkCallbacks* cb);
		std::unique_ptr<DataSource> make_datasource(std::string backend, DataSourceCallbacks* cb);
	}
}


#endif
