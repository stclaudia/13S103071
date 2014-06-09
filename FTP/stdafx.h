// stdafx.h : 标准系统包含文件的包含文件，
// 或是常用但不常更改的项目特定的包含文件
//
#define _WIN32_WINNT    0x0500
#pragma once


#include <iostream>
#include <tchar.h>
#include <cstdlib>
#include <clocale>
#include <ctime>
#include <vector>
#include <algorithm>
#include <winsock2.h>
#include <mswsock.h>
#include <direct.h>
#include <iphlpapi.h>
#include <stdarg.h>
// TODO: 在此处引用程序要求的附加头文件
using namespace std;
#pragma comment(lib,"ws2_32.lib")
#pragma comment(lib,"mswsock.lib")
#pragma comment(lib,"Iphlpapi.lib")