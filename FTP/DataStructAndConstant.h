#ifndef DATASTRUCTANDCONSTANT
#define DATASTRUCTANDCONSTANT
#include "stdafx.h"
#define BUFFERSIZE 0x10000
#define LISTENPORT 2100
#define FIRSTPASVPORT 6000
#define MAXUSER 1000
#define MAX_FILE 1024                                       ///定义最多文件数
#define MAXUSERNAMELENGTH 32
#define MAX_ADDRESS 20
#define DEFAULTWORKPATH   "D:\\"
typedef enum{DataIoTransPacket,DataIoUpload,ControlIoSend,ControlIoRecv,IoQuit} IO_TYPE;
typedef enum{HandleControl,HandleData} HANDLE_TYPE;
typedef enum{ToSendData,ToReceiveData,/*ToCreatPasvConnect,ToCreatPortConnect,*/ToReceive,ToAbort,DoNothing} NEXT_TO_DO;
typedef struct _FILE_INFO                                   ///定义一个文件结构，用于提取文件属性
{
	TCHAR cFileName[MAX_PATH];                              ///文件名称
	DWORD dwFileAttributes;                                 ///文件属性（是否目录）
    FILETIME ftCreationTime;                                ///文件创建时间
    FILETIME ftLastAccessTime;                              ///文件的最后访问时间
    FILETIME ftLastWriteTime;                               ///文件的最后修改时间
    DWORD nFileSizeHigh;                                    ///文件大小的高32位
    DWORD nFileSizeLow;                                     ///文件大小的低32位
} FILE_INFO, *LPFILE_INFO;
struct ContolHandleInfo
{
	char strCurrentWorkPath[1024];//该控制连接目前所在的工作路径
	void * pDataLinkSocketData;//一个指向于本控制连接对应的数据连接对应的PER_SOCKET_DATA结构的指针
	void * pDataLinkSocketDataBuffer;//指向存储DataLinkSocketData的内存块 
	char strCurrentName[MAXUSERNAMELENGTH];
	bool bBePasv;// 表示该控制连接对应的数据连接类型是否为PASV
	HANDLE EventDataValid;//数据连结是否有效
	DWORD dwDataResetPosition;//对应数据连接传送文件的偏移量
	HANDLE hCurrentFileHandle;//本用户进程当前打开的文件句柄
};
struct DataHandleInfo
{
	void * pControlLinkSocketData;//一个指向于本数据连接对应的控制连接对应的PER_SOCKET_DATA结构的指针

	bool bValid;//表示当前连接是否有效
	WSAEVENT weAcceptEvent;
	bool bAbor;//用来表示当前数据连接正在被终止
	
};
typedef struct 
{
	SOCKET Socket;    //控制连接或数据连接套接字
	HANDLE_TYPE enCurrentHandleType;//标示当前连接时数据连接还是控制连接
	union 
	{
		ContolHandleInfo cCtrlInfo;
		DataHandleInfo cDataInfo;
	} uHandleInfo;
}PER_HANDLE_DATA,*LPPER_HANDLE_DATA;


struct DataIoTransPacketInfo//客户端下载文件 
{
	ULONG uTotolFileSize;
	HANDLE hTmpFile;//需要关闭的文件句柄！
};
struct DataIoUploadInfo//客户端上传文件
{
	HANDLE hFileHandle;//客户上传文件在本地存储文件的句柄
	ULONG uTotolFileSize;
	ULONG uReceivedFileSize;
};
struct ControlIoSendInfo
{
	DWORD dwSendedDataLength;//由于ＷＳＡＳｅｎｄ　存在一次发送数据一部分的可能　故使用该值储存累计发送的字节数
	NEXT_TO_DO enNextToDo;//标示下一步所需的操作
	BYTE bWhatToDo[256];//存储一些相关下一步工作的信息 具体内容由上一个参数而定
};
struct ControlIoRecvInfo
{
	int NoUse;
};
struct IoQuitInfo
{
	int NoUse;
};
typedef struct 
{
	OVERLAPPED Overlapped;
	WSABUF cBuffer;
	char stBuffer[BUFFERSIZE];
	IO_TYPE IoType;
	union 
	{
		DataIoTransPacketInfo cDataIoTransPacketInfo;
		DataIoUploadInfo cDataIoUploadInfo;
		ControlIoSendInfo cControlIoSendInfo;
		ControlIoRecvInfo cControlIoRecvInfo;
		IoQuitInfo cIoQuitInfo;
	} uIoInfo;

}PER_IO_OPERATION_DATA, *LPPER_IO_OPERATION_DATA;

typedef struct
{
	PER_HANDLE_DATA cPerHandleData;
	PER_IO_OPERATION_DATA cPerIoData;
	DWORD dwClientIP;
	WORD wPort;
	PER_IO_OPERATION_DATA cPerIoData2;
}PER_SOCKET_DATA,*LPPER_SOCKET_DATA;

extern CRITICAL_SECTION  PerSocketVectorProtection;//作为操作vector的全局互斥量
extern HANDLE CompetionPort;//作为全局的完成端口的句柄
extern CRITICAL_SECTION  AddIocpProtection;//为了添加完成端口处理socket的全局变量

typedef vector <LPPER_SOCKET_DATA> PerSocketDataVector;
extern PerSocketDataVector cPerSocketDataVector;
#endif