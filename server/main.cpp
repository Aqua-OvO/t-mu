#define _SILENCE_CXX17_ALLOCATOR_VOID_DEPRECATION_WARNING
#define _SILENCE_ALL_CXX17_DEPRECATION_WARNINGS
#include <iostream>
#include "asio.hpp"
#include <functional>
#include <thread>
#include <string>

namespace timer1
{
	int func()
	{
		asio::io_context io;

		asio::steady_timer t(io, asio::chrono::seconds(5));
		t.wait();

		std::cout << "Hello, world!" << std::endl;

		return 0;
	}
}

namespace timer2
{
	void print(const asio::error_code&)
	{
		std::cout << "Hello, world!" << std::endl;
	}

	int func()
	{
		asio::io_context io;

		asio::steady_timer t(io, asio::chrono::seconds(5));
		t.async_wait(&print);
		std::cout << "xxxxxxxxxxxxxx" << std::endl;
		io.run();
		std::cout << "ffffffffffffffffff" << std::endl;
		return 0;
	}
}

namespace timer3
{
	void print(const asio::error_code& /*e*/,
		asio::steady_timer* t, int* count)
	{
		if (*count < 5)
		{
			std::cout << *count << std::endl;
			++(*count);

			t->expires_at(t->expiry() + std::chrono::seconds(1));
			t->async_wait(std::bind(print, std::placeholders::_1, t, count));
		}
	}

	int func()
	{
		asio::io_context io;

		int count = 0;
		asio::steady_timer t(io, std::chrono::seconds(1));
		t.async_wait(std::bind(print,
			std::placeholders::_1, &t, &count));

		io.run();

		std::cout << "Final count is " << count << std::endl;

		return 0;
	}
}

namespace timer4
{
	class printer
	{
	public:
		printer(asio::io_context& io)
			: timer_(io, asio::chrono::seconds(1)),
			count_(0)
		{
			timer_.async_wait(std::bind(&printer::print, this));
		}

		~printer()
		{
			std::cout << "Final count is " << count_ << std::endl;
		}

		void print()
		{
			if (count_ < 5)
			{
				std::cout << count_ << std::endl;
				++count_;

				timer_.expires_at(timer_.expiry() + asio::chrono::seconds(1));
				timer_.async_wait(std::bind(&printer::print, this));
			}
		}

	private:
		asio::steady_timer timer_;
		int count_;
	};

	int func()
	{
		asio::io_context io;
		printer p(io);
		io.run();

		return 0;
	}
}

namespace timer5
{
	class printer
	{
	public:
		printer(asio::io_context& io)
			: strand_(asio::make_strand(io)),
			timer1_(io, asio::chrono::seconds(1)),
			timer2_(io, asio::chrono::seconds(1)),
			count_(0)
		{
			timer1_.async_wait(asio::bind_executor(strand_,
				std::bind(&printer::print1, this)));

			timer2_.async_wait(asio::bind_executor(strand_,
				std::bind(&printer::print2, this)));
		}

		~printer()
		{
			std::cout << "Final count is " << count_ << std::endl;
		}

		void print1()
		{
			if (count_ < 10)
			{
				std::cout << "Timer 1: " << count_ << std::endl;
				++count_;

				timer1_.expires_at(timer1_.expiry() + asio::chrono::seconds(1));

				timer1_.async_wait(asio::bind_executor(strand_,
					std::bind(&printer::print1, this)));
			}
		}

		void print2()
		{
			if (count_ < 10)
			{
				std::cout << "Timer 2: " << count_ << std::endl;
				++count_;

				timer2_.expires_at(timer2_.expiry() + asio::chrono::seconds(1));

				timer2_.async_wait(asio::bind_executor(strand_,
					std::bind(&printer::print2, this)));
			}
		}

	private:
		asio::strand<asio::io_context::executor_type> strand_;
		asio::steady_timer timer1_;
		asio::steady_timer timer2_;
		int count_;
	};

	int func()
	{
		asio::io_context io;
		printer p(io);
		std::thread t([&io] {io.run(); });
		//std::thread t(std::bind(static_cast<std::size_t(asio::io_service::*)()>(&asio::io_service::run), &io));
		printf("xxxxxxxxxxxxxxxxxxxxxx");
		io.run();
		printf("YYYYYYYYYYYYYYYYYYYYYY");
		t.join();

		return 0;
	}
}
 
namespace server1
{
	using asio::ip::tcp;

	std::string make_daytime_string()
	{
		using namespace std; // For time_t, time and ctime;
		time_t now = time(0);
		return ctime(&now);
	}

	int func()
	{
		try
		{
			asio::io_context io_context;

			tcp::acceptor acceptor(io_context, tcp::endpoint(tcp::v4(), 13));

			for (;;)
			{
				tcp::socket socket(io_context);
				acceptor.accept(socket);

				std::string message = make_daytime_string();

				asio::error_code ignored_error;
				asio::write(socket, asio::buffer(message), ignored_error);
			}
		}
		catch (std::exception& e)
		{
			std::cerr << e.what() << std::endl;
		}

		return 0;
	}
}

namespace server2
{
	using asio::ip::tcp;

	std::string make_daytime_string()
	{
		using namespace std; // For time_t, time and ctime;
		time_t now = time(0);
		return ctime(&now);
	}

	class tcp_connection
		: public std::enable_shared_from_this<tcp_connection>
	{
	public:
		typedef std::shared_ptr<tcp_connection> pointer;

		static pointer create(asio::io_context& io_context)
		{
			return pointer(new tcp_connection(io_context));
		}

		tcp::socket& socket()
		{
			return socket_;
		}

		void start()
		{
			message_ = make_daytime_string();

			asio::async_write(socket_, asio::buffer(message_),
				std::bind(&tcp_connection::handle_write, shared_from_this(),
					std::placeholders::_1,
					std::placeholders::_2));
		}

	private:
		tcp_connection(asio::io_context& io_context)
			: socket_(io_context)
		{
		}

		void handle_write(const asio::error_code& /*error*/,
			size_t /*bytes_transferred*/)
		{
		}

		tcp::socket socket_;
		std::string message_;
	};

	class tcp_server
	{
	public:
		tcp_server(asio::io_context& io_context)
			: io_context_(io_context),
			acceptor_(io_context, tcp::endpoint(tcp::v4(), 13))
		{
			start_accept();
		}

	private:
		void start_accept()
		{
			tcp_connection::pointer new_connection =
				tcp_connection::create(io_context_);

			acceptor_.async_accept(new_connection->socket(),
				std::bind(&tcp_server::handle_accept, this, new_connection,
					std::placeholders::_1));
		}

		void handle_accept(tcp_connection::pointer new_connection,
			const asio::error_code& error)
		{
			if (!error)
			{
				new_connection->start();
			}

			start_accept();
		}

		asio::io_context& io_context_;
		tcp::acceptor acceptor_;
	};

	int func()
	{
		try
		{
			asio::io_context io_context;
			tcp_server server(io_context);
			io_context.run();
		}
		catch (std::exception& e)
		{
			std::cerr << e.what() << std::endl;
		}

		return 0;
	}
}

//int main()
//{
//	server2::func();
//	return 0;
//}
