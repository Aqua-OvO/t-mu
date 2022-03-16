#define _SILENCE_CXX17_ALLOCATOR_VOID_DEPRECATION_WARNING
#define _SILENCE_ALL_CXX17_DEPRECATION_WARNINGS
extern "C" {
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}

#include <iostream>
#include <filesystem>
#include <string>
#include <complex> //复数
//#include <windows.h>
#include <thread>
#include <string_view>
#include <cstdarg>

#include "asio.hpp"
using namespace std;


//函数库示例，Windows下查找文件功能

//输入:string路径名

//输出:userdata存放Handle(如果没找到，则是nil), string文件名

int findfirst(lua_State *L)
{

	WIN32_FIND_DATAA FindFileData;
	HANDLE hFind = ::FindFirstFileA(luaL_checkstring(L, 1), &FindFileData);
	if (INVALID_HANDLE_VALUE == hFind)
		lua_pushnil(L);
	else
		lua_pushlightuserdata(L, hFind);
	lua_pushstring(L, FindFileData.cFileName);
	return 2;
}

//输入:userdata:findfirst返回的Handle

//输出:string：文件名，如果没找到，则返回nil

int findnext(lua_State *L)
{

	WIN32_FIND_DATAA FindFileData;
	if (::FindNextFileA(lua_touserdata(L, 1), &FindFileData))
		lua_pushstring(L, FindFileData.cFileName);
	else
		lua_pushnil(L);
	return 1;
}

//输入:userdata:findfirst返回的Handle

//没有输出

int findclose(lua_State *L)
{
	::FindClose(lua_touserdata(L, 1));
	return 0;
}

//注册函数库

static const struct luaL_Reg lrFiles[] = {
	{"FindFirst", findfirst},
	{"FindNext", findnext},
	{"FindClose", findclose},
	{NULL, NULL}    /* sentinel */
};

LUALIB_API int Filess(lua_State *L)
{
	luaL_newlib(L, lrFiles);
	return 1;
}


//int luaopen_Files(lua_State *L) {
//	luaL_register(L, "Files", lrFiles);
//	return 1;
//}

void test() {
	std::cout << "test" << endl;
}

template<typename T>
void foo(const T &t)
{
	T t1 = t;
	t1();
}

struct state_deleter {
	void operator()(lua_State* L) const {
		lua_close(L);
	}
};

class lua_service
{
public:
	lua_service();

	~lua_service();

	lua_State* get_lua_server();

private:

	static void* lalloc(void * ud, void *ptr, size_t osize, size_t nsize);
public:
	size_t mem = 0;
	size_t mem_limit = 0;
	size_t mem_report = 8 * 1024 * 1024;
private:
	std::unique_ptr<lua_State, state_deleter> lua_;
};

void *lua_service::lalloc(void *ud, void *ptr, size_t osize, size_t nsize)
{
	lua_service *l = reinterpret_cast<lua_service *>(ud);
	size_t mem = l->mem;

	l->mem += nsize;

	if (ptr)
		l->mem -= osize;
	//std::cout << "===>lalloc" << " " << l->mem << " " << ptr << " " << osize << " " << nsize << std::endl;
	if (l->mem_limit != 0 && l->mem > l->mem_limit)
	{
		if (ptr == nullptr || nsize > osize)
		{
			l->mem = mem;
			return nullptr;
		}
	}

	if (l->mem > l->mem_report)
	{
		l->mem_report *= 2;
	}

	if (nsize == 0)
	{
		free(ptr);
		return nullptr;
	}
	else
	{
		return realloc(ptr, nsize);
	}
}

lua_service::lua_service()
	: lua_(lua_newstate(lalloc, this))
{

}

lua_State* lua_service::get_lua_server()
{
	return lua_.get();
}

inline std::string format(const char* fmt, ...)
{
	if (!fmt) return std::string("");

	size_t fmt_buffer_size = 1024;

	std::string res;
	res.resize(fmt_buffer_size);

	va_list ap;
	va_start(ap, fmt);
	int len = vsnprintf(res.data(), res.size(), fmt, ap);
	va_end(ap);

	cout << "===>format" << res.data() << endl;

	return res;
}

std::string_view STR_LF = "\n"sv;

template<int T>
void func(const char (&str)[T])
{
	std::cout << str << std::endl;
}


class match_char
{
public:
	explicit match_char(char c) : c_(c) {}

	template <typename Iterator>
	std::pair<Iterator, bool> operator()(
		Iterator begin, Iterator end) const
	{
		Iterator i = begin;
		while (i != end)
			if (c_ == *i++)
				return std::make_pair(i, true);
		return std::make_pair(i, false);
	}

private:
	char c_;
};





int main()
{
	asio::io_service my_io_service;
	asio::ip::tcp::resolver resolver(my_io_service);
	asio::ip::tcp::resolver::query query("www.boost.org", "http");
	asio::ip::tcp::resolver::iterator iter = resolver.resolve(query);
	asio::ip::tcp::resolver::iterator end; // End marker.
	while (iter != end)
	{
		asio::ip::tcp::endpoint endpoint = *iter++;
		std::cout << endpoint << std::endl;
	}

	asio::ip::tcp::socket socket(my_io_service);
	asio::connect(socket, resolver.resolve(query));


	lua_service * ls = new lua_service();
	lua_State *L = ls->get_lua_server();
	luaL_openlibs(L);
	lua_pushboolean(L, true);
	lua_setglobal(L, "__init__");

	uint32_t thread_count = std::thread::hardware_concurrency();
	bool enable_console = true;
	std::string logfile;
	std::string loglevel;
	std::string bootstrap = "../main.lua";
	std::string arg = "return {'hello'}";

	int r = luaL_loadfile(L, bootstrap.data());
	std::cout << "luaL_loadfile is " << r << std::endl;
	r = luaL_dostring(L, arg.data());
	std::cout << "luaL_dostring is " << r << std::endl;
	r = lua_pcall(L, 1, 1, 1);
	std::cout << "lua_pcall is " << r << std::endl;
	lua_pushnil(L);
	while (lua_next(L, -2))
	{
		std::string key = lua_tostring(L, -2);
		if (key == "thread")
			thread_count = (uint32_t)luaL_checkinteger(L, -1);
		else if (key == "logfile")
		{
			size_t size;
			const char* sz = luaL_checklstring(L, -1, &size);
		}
		else if (key == "enable_console")
			enable_console = lua_toboolean(L, -1);
		else if (key == "loglevel")
		{
			size_t size;
			const char* sz = luaL_checklstring(L, -1, &size);
		}
		lua_pop(L, 1);
	}

	std::cout << "   " << std::filesystem::current_path().string() << endl;
	format("package.path='%s/?.lua;'..package.path", std::filesystem::current_path().string().data());
	const char aa[4] = "aab";
	func("ccc");
	//func1(2);
	lua_close(L);

	return 0;
}