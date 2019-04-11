#include <cstdlib>
#include <iostream>
#include <memory>
#include <utility>
#include <boost/asio.hpp>
#include <string>

constexpr int maxInputBufferLength{1024};
std::string epicsProtocolTerminator{"X"};
using boost::asio::ip::tcp;

struct session
   : public std::enable_shared_from_this<session>
{
    using command_t = std::string;


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
		socket_.async_read_some(boost::asio::buffer(data_, maxInputBufferLength),
				[this, self](boost::system::error_code ec, std::size_t length)
		
		{
		    makeCommandFromEpicsProtocol(data_);
		    std::string inputString{data_};
		    inputString.erase(inputString.find(epicsProtocolTerminator));
		    std::cout << inputString << std::endl << check1;
			if(0==check1.compare(inputString)){
			    std::cout << check1.compare(inputString) << std::endl;
				std::cout << "Check 1 triggered by String\n";
			}
            else if(0==check2.compare(inputString)){
                std::cout << "Check 2 triggered by String\n";
            }

			if(!ec){
				acknowledge("Roger that\n");
			}
		});
	}

	command_t makeCommandFromEpicsProtocol(const char* inputData){
        std::string inputString{inputData};
        return command_t{inputString.erase(inputString.find(epicsProtocolTerminator))};
	}

	void actOutCommand(command_t command){

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
/*
	class command_t{
    public:
	    command_t(const char* inputData){
            std::string inputString{data_};
            action{inputString.erase(inputString.find(epicsProtocolTerminator))};
	    }

    private:
	    std::string action;
	    int_fast8_t value{};
	};*/

	tcp::socket socket_;
	//enum {max_length = 1024};
	char data_[maxInputBufferLength];
	std::string check1{"eintauchen"};
    std::string check2{"herausfahren"};
    //std::string check3{"fahre auf position: 30%"};
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
    if(argc != 2){
        std::cerr << "Usage: asioLSMserver <port> \n";
        return 22; //EINVAL: Invalid Arguments errno
    }

    try{
		boost::asio::io_context io_context;

		server s(io_context, std::atoi(argv[1]));

		io_context.run();

	}
	catch (std::exception& e){
		std::cerr << "Exception: " << e.what() << "\n";
	}
}
