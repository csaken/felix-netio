#include <chrono>
#include <atomic>
#include <iostream>

#include "boost/lexical_cast.hpp"

#include "netio/netio.hpp"

const char VERSION_STRING[] = "0.1";


using namespace std;
using namespace felix::netio;

class Callbacks : public DataSinkCallbacks
{
public:
	atomic_ulong messages_received;
	atomic_ulong messages_received_with_error;

	Callbacks()
	{
		messages_received.store(0);
		messages_received_with_error.store(0);
	}

	void on_open(felix::netio::Endpoint endpoint)
	{
		cout << "Connection opened from " << endpoint.endpoint << endl;
	}

	void on_close(felix::netio::Endpoint endpoint)
	{
		cout << "Connection closed from " << endpoint.endpoint << "  /  "
	         << "Messages received: " << messages_received << "  /  "
	   	     << "Messages received with error: " << messages_received_with_error << endl;
	}

	void on_data_received(const std::vector<DataSinkMessage>&)
	{
		messages_received++;
	}

	void on_data_received_with_error(const std::vector<DataSinkMessage>&)
	{
		messages_received++;
		messages_received_with_error++;
	}
};

void
display_help()
{
	cout << "usage: netio_datagen [options]" << endl;
	cout << endl << "where options are:" << endl;
	cout << "    -p  TCP port to listen on [default: 12345]" << endl;
	cout << "    -h  Display this help" << endl;
	cout << "    -v  Display version information and exit" << endl;
}


bool
parse_options(int argc, char** argv,
	unsigned short& dest_port)
{
	char c;

	while ((c = getopt (argc, argv, "p:hv")) != -1)
    switch (c)
      {
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
	unsigned short port = 12345;
	if(!parse_options(argc, argv, port))
	{
		return -1;
	}

	Callbacks callbacks;
	auto server = make_datasink("libevent", Endpoint("0.0.0.0", port), &callbacks);

	server->start();

	while(true)
	{
	    this_thread::sleep_for (chrono::seconds(1));
	//    cout << "Messages received: " << callbacks.messages_received << endl;
	//   	cout << "Messages received with error: " << callbacks.messages_received_with_error << endl;
	}


	server->stop();
}
