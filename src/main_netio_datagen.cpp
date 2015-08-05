#include <iostream>
#include <chrono>
#include <atomic>

#include "boost/lexical_cast.hpp"

#include "netio/netio.hpp"

const char VERSION_STRING[] = "0.1";


using namespace std;
using namespace felix::netio;

class DatagenCallbacks : public DataSourceCallbacks
{
public:
	DatagenCallbacks(unsigned long max_messages)
	{
		data_sent.store(0);
		nr_messages.store(0);
		this->max_messages = max_messages;
	}

	atomic_ulong data_sent;
	atomic_ulong nr_messages;
	unsigned long max_messages;
	chrono::time_point<std::chrono::high_resolution_clock> t1;
	chrono::time_point<std::chrono::high_resolution_clock> t2;

	void on_open(Endpoint endpoint)
	{
		cout << "-- Opened connection to " << endpoint.endpoint << endl;
	}

	void on_close(Endpoint endpoint)
	{
		cout << "-- Closed connection to " << endpoint.endpoint << endl;
	}

	void on_data_send(const Message&)
	{
		count_message();
	}

	void on_data_send_with_error(const Message&)
	{
		count_message();
	}

	double duration_in_seconds()
	{
		return std::chrono::duration_cast<std::chrono::nanoseconds>(t2-t1).count() * 1e-9;
	}

private:
	void count_message()
	{
		if(nr_messages.load() == 0)
		{
			t1 = std::chrono::high_resolution_clock::now(); 
		}

		nr_messages++;

		if(nr_messages.load() == max_messages)
		{
			t2 = std::chrono::high_resolution_clock::now(); 
		}

	}

};

void
display_help()
{
	cout << "usage: netio_datagen [options]" << endl;
	cout << endl << "where options are:" << endl;
	cout << "    -n  Number of messages to send [default: 1,000,000]" << endl;
	cout << "    -s  Message size in Byte [default: 1024 Byte]" << endl;
	cout << "    -d  IP address of the host to connect to [default: 127.0.0.1]" << endl;
	cout << "    -p  TCP port of the destination host [default: 12345]" << endl;
	cout << "    -h  Display this help" << endl;
	cout << "    -v  Display version information and exit" << endl;
}


bool
parse_options(int argc, char** argv,
	unsigned& n_messages,
	unsigned& message_size,
	std::string& dest_host,
	unsigned short& dest_port)
{
	char c;

	while ((c = getopt (argc, argv, "n:s:d:p:hv")) != -1)
    switch (c)
      {
      case 'n':
        n_messages = boost::lexical_cast<unsigned>(optarg);
        break;
      case 's':
        message_size = boost::lexical_cast<unsigned>(optarg);
        break;
      case 'd':
        dest_host = optarg;
        break;
      case 'p':
        dest_port = boost::lexical_cast<unsigned short>(optarg);
        break;
      case 'v':
        cout << "netio_datagen v" << VERSION_STRING << endl;
        return false;
      case '?':
        if (optopt == 'h')
        	cerr << "Option -" << optopt << " requires an argument.\n";
        else if (isprint (optopt))
        	cerr << "Unknown option `" << optopt << "'.\n";
        else
        	cerr << "Unknown option charactor\n";
      case 'h':
      default:
        display_help();
		return false;
      }

      return true;
}

int
main(int argc, char** argv)
{
	unsigned n_messages = 1000000;
	unsigned message_size = 1024;
	std::string hostname("127.0.0.1");
	unsigned short port = 12345;
	if(!parse_options(argc, argv,
		n_messages, message_size, hostname, port))
	{
        return -1;
	}

	DatagenCallbacks callbacks(n_messages);
	auto datasource = make_datasource("posix", &callbacks);

	datasource->start();

	std::string msg_str(message_size, 'x');
	Message msg(msg_str.c_str(), msg_str.length());
	std::vector<std::unique_ptr<Message>> messages;
	for(unsigned i=0; i < n_messages; i++)
		messages.emplace_back(new Message(msg_str.c_str(), msg_str.length()));


	Endpoint destination(hostname, port);
	datasource->send_messages(destination, messages);

	while (callbacks.nr_messages.load() != n_messages)
	{
	    this_thread::sleep_for (chrono::milliseconds(1000));
	}

	double seconds =  callbacks.duration_in_seconds();
	cout << "Duration: " << seconds << " seconds" << endl;
	cout << "Overall: " << msg_str.length()*n_messages / seconds / (1024.*1024.) << " MB/s" << endl;

	datasource->stop();
}