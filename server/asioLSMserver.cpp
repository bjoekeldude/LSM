#include <cstdlib>
#include <iostream>
#include <memory>
#include <utility>
#include <boost/asio.hpp>
#include <string>

using boost::asio::ip::tcp;

struct session
   : public std::enable_shared_from_this<session>
{
public: 
	session(tcp::socket socket)
		: socket_(std::move(socket))
	{
	}

	void start(){
		do_read();
	}

private:
	void do_read(){
		auto self(shared_from_this());
		socket_.async_read_some(boost::asio::buffer(data_, max_length),
				[this, self](boost::system::error_code ec, std::size_t length)
		
		{
			if(check1.compare(data_)){
				std::cout << "Check 1 triggered by String\n";
			}
            else if(check2.compare(data_)){
                std::cout << "Check 2 triggered by String\n";
            }
            else if(check3.compare(data_)){
                std::cout << "Check 3 triggered by String\n";
            }
			std::cout << data_ << " " << max_length << " " << length << "\n";
			if(!ec){
				acknowledge("Roger that\n");
			}
		});
	}

	void acknowledge(std::string ackmessage){
		auto self(shared_from_this());
		boost::asio::async_write(socket_, boost::asio::buffer(ackmessage),
				[this, self](boost::system::error_code ec, std::size_t /*length*/)
				{
					if(ec.value()){
						std::cerr << ec.category().name() << ':' << ec.value() << "\n";
					}
				});
	}

	void do_write(std::size_t length){
		auto self(shared_from_this());
		boost::asio::async_write(socket_, boost::asio::buffer(data_, length),
				[this, self](boost::system::error_code ec, std::size_t /*length*/)
		{
			if(!ec){
				do_read();
			}
		});
	}

	tcp::socket socket_;
	enum {max_length = 1024};
	char data_[max_length];
	std::string check1{"eintauchen"};
    std::string check2{"herausfahren"};
    std::string check3{"fahre auf position: 30%"};
};

class server{
public: 
	server(boost::asio::io_context& io_context, short port)
		: acceptor_(io_context, tcp::endpoint(tcp::v4(), port)),
		socket_(io_context)
	{
		do_accept();
	}

private:
	void do_accept(){
		acceptor_.async_accept(socket_,
			[this](boost::system::error_code ec)
		{
			if(!ec){
				std::make_shared<session>(std::move(socket_))->start();
			}

			do_accept();
		});
	}

	tcp::acceptor acceptor_;
	tcp::socket socket_;
};

int main(int argc, char* argv[]){
	try{
		if(argc != 2){
			std::cerr << "Usage: asioLSMserver <port> \n";
			return 22; //EINVAL: Invalid Arguments errno
		}
		boost::asio::io_context io_context;

		server s(io_context, std::atoi(argv[1]));

		io_context.run();

	}
	catch (std::exception& e){
		std::cerr << "Exception: " << e.what() << "\n";
	}
}
