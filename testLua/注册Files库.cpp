#ifdef _HHHHHHHHHH
extern "C" {
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}

#include <iostream>
#include <string>
#include <complex> //����
#include <windows.h>
using namespace std;


//������ʾ����Windows�²����ļ�����

//����:string·����

//���:userdata���Handle(���û�ҵ�������nil), string�ļ���

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

//����:userdata:findfirst���ص�Handle

//���:string���ļ��������û�ҵ����򷵻�nil

int findnext(lua_State *L)
{

	WIN32_FIND_DATAA FindFileData;
	if (::FindNextFileA(lua_touserdata(L, 1), &FindFileData))
		lua_pushstring(L, FindFileData.cFileName);
	else
		lua_pushnil(L);
	return 1;
}

//����:userdata:findfirst���ص�Handle

//û�����

int findclose(lua_State *L)
{
	::FindClose(lua_touserdata(L, 1));
	return 0;
}

//ע�ắ����

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

int main()
{
	char* szLua_code =
		"hFind,sFile = Files.FindFirst('c:\\\\*.*'); "
		"if hFind then "
		"    repeat "
		"        print(sFile) "
		"        sFile = Files.FindNext(hFind) "
		"    until sFile==nil; "
		"    Files.FindClose(hFind) "
		"end";
	lua_State *L = luaL_newstate();
	luaL_openlibs(L);
	luaL_requiref(L, "Files", Filess, true);
	//luaopen_Files(L);
	bool err = luaL_loadstring(L, szLua_code) || lua_pcall(L, 0, 0, 0);
	if (err)
	{
		cerr << lua_tostring(L, -1);
		lua_pop(L, 1);
	}
	lua_close(L);
	return 0;
}
#endif