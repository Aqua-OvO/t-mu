#define _SILENCE_CXX17_ALLOCATOR_VOID_DEPRECATION_WARNING
#define _SILENCE_ALL_CXX17_DEPRECATION_WARNINGS
#include <iostream>
#include "asio.hpp"
#include <functional>
#include <thread>
#include <array>

namespace clint1
{
	using asio::ip::tcp;

	int func(const char* arg)
	{
		try
		{
			asio::io_context io_context;

			tcp::resolver resolver(io_context);
			tcp::resolver::results_type endpoints =
				resolver.resolve(arg, "13");

			tcp::socket socket(io_context);
			asio::connect(socket, endpoints);

			for (;;)
			{
				std::array<char, 128> buf;
				asio::error_code error;

				size_t len = socket.read_some(asio::buffer(buf), error);

				if (error == asio::error::eof)
					break; // Connection closed cleanly by peer.
				else if (error)
					throw asio::system_error(error); // Some other error.

				std::cout.write(buf.data(), len);
			}
		}
		catch (std::exception& e)
		{
			std::cerr << e.what() << std::endl;
		}

		return 0;
	}
}

int main()
{
	clint1::func("127.0.0.1");
	return 0;
}
