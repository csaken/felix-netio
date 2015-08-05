#include "netio/netio.hpp"

#include <algorithm>

namespace felix
{
    namespace netio
    {
        Message::Message(const char* d, size_t size, cleanup_fn hook) 
            : cleanup_hook(hook)
        {
                    fragment_data_array[0] = d;
                    fragment_sizes_array[0] = size;
                    n_fragments = 1;
                    use_static_array = true;
        }

        Message::Message(const std::vector<const char*> data, 
                const std::vector<size_t> sizes, cleanup_fn hook) 
            : cleanup_hook(hook)
        {
                    this->data = data;
                    this->sizes = sizes;
                    n_fragments = data.size();
                    use_static_array = false;
        }

        Message::Message(const char* const* data, 
                const size_t* sizes, const size_t n, cleanup_fn hook)
            : data(n), sizes(n), cleanup_hook(hook) 
        {
                    n_fragments = n;
                    if(n > MAX_STATIC_FRAGMENTS)
                    {
                        this->data.reserve(n);
                        this->sizes.reserve(n);
                        use_static_array = false;
                        for(int i=0; i<n; i++)
                        {
                            this->data.push_back(data[i]);
                            this->sizes.push_back(sizes[i]);
                        }
                    } else {
                        use_static_array = true;
                        for(int i=0; i<n; i++)
                        {
                            this->fragment_data_array[i] = data[i];
                            this->fragment_sizes_array[i] = sizes[i];
                        }
                    }
        }

        Message::Message(const std::string& str, cleanup_fn hook) 
            : cleanup_hook(hook)
        {
                    use_static_array = true;
                    fragment_data_array[0] = str.c_str();
                    fragment_sizes_array[0] = str.length();
                    n_fragments = 1;
        }

        Message::Message(
                const std::vector<boost::asio::mutable_buffer>& buffers, cleanup_fn hook) 
            : cleanup_hook(hook) 
        {
                    // TODO: do we still need this constructor?
                    use_static_array = false;
                    data.reserve(buffers.size());
                    sizes.reserve(buffers.size());

                    for(unsigned i=0; i<buffers.size(); i++)
                    {
                      sizes.push_back(boost::asio::buffer_size(buffers[i]));
                      data.push_back(boost::asio::buffer_cast<char*>(buffers[i]));
                    }   
        }

        Message::Message(const Message& msg)
        {
                    use_static_array = msg.use_static_array;
                    n_fragments = msg.n_fragments;
                    cleanup_hook = msg.cleanup_hook;

                    if(use_static_array)
                    {
                        for(unsigned i=0; i<n_fragments; i++)
                        {
                            fragment_data_array[i] = msg.fragment_data_array[i];
                            fragment_sizes_array[i] = msg.fragment_sizes_array[i];
                        }
                    } else {
                        this->sizes = msg.sizes;
                        this->data = msg.data;
                    };
        }

        Message::Message(Message&& other) 
        {
                    use_static_array = other.use_static_array;
                    n_fragments = other.n_fragments;
                    cleanup_hook = other.cleanup_hook;
                    other.cleanup_hook = NULL;

                    if(use_static_array)
                    {
                        for(unsigned i=0; i<n_fragments; i++)
                        {
                            fragment_data_array[i] = other.fragment_data_array[i];
                            fragment_sizes_array[i] = other.fragment_sizes_array[i];
                        }
                    } else {
                        data.swap(other.data);
                        sizes.swap(other.sizes);
                    }

                    other.n_fragments = 0;
        }

        Message::~Message()
        {
            if(cleanup_hook)
                cleanup_hook();

        }

        Message& Message::operator= (const Message& other)
        {
                    if(this != &other)
                    {
                        n_fragments = other.n_fragments;
                        use_static_array = other.use_static_array;
                        cleanup_hook = other.cleanup_hook;

                        if(use_static_array)
                        {
                            for(unsigned i=0; i<n_fragments; i++)
                            {
                                fragment_data_array[i] = other.fragment_data_array[i];
                                fragment_sizes_array[i] = other.fragment_sizes_array[i];
                            }
                        } else {
                            this->sizes = other.sizes;
                            this->data = other.data;
                        }
                    }

                    return *this;
        }

        const char* const* Message::fragments() const
        {
            if(use_static_array)
                return fragment_data_array;
            else
                return data.data();
        }

        const size_t* Message::fragment_sizes() const
        {
            if(use_static_array)
                return fragment_sizes_array;
            else
                return sizes.data();
        }
        
        size_t Message::num_fragments() const
        {
            return n_fragments;
        }

        size_t Message::size(void) const
        {
                    const size_t* fsizes = this->fragment_sizes();
                    size_t sum = 0;
                    for(unsigned i=0; i<n_fragments; i++)
                        sum += fsizes[i];
                        return sum;
        }

        char* Message::data_copy(void) const
        {
                    const char* const* fdata = this->fragments();
                    const size_t* fsizes = this->fragment_sizes();

                    char* d = new char[this->size()];
                    char* p = d;
                    for(unsigned i=0; i < num_fragments(); i++)
                    {
                        memcpy(p, fdata[i], fsizes[i]);
                        p += fsizes[i];
                    }

                    return d;
        }


        DataSinkMessage::DataSinkMessage(char* data, const size_t len) 
            : data(data), len(len) 
        {
        }

        DataSinkMessage::DataSinkMessage(DataSinkMessage&& other)
        {
            this->data = other.data;
            this->len = other.len;
            other.data = NULL;
            other.len = 0;
        }

        DataSinkMessage::~DataSinkMessage(void)
        {
            if(data)
                delete[] data;
            data = NULL;
        }

        size_t DataSinkMessage::size(void) const 
        {
            return len;
        }
    }
}

