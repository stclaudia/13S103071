#ifndef DATASTRUCTANDCONSTANT
#define DATASTRUCTANDCONSTANT
#include "stdafx.h"
#define BUFFERSIZE 0x10000
#define LISTENPORT 2100
#define FIRSTPASVPORT 6000
#define MAXUSER 1000
#define MAX_FILE 1024                                       ///��������ļ���
#define MAXUSERNAMELENGTH 32
#define MAX_ADDRESS 20
#define DEFAULTWORKPATH   "D:\\"
typedef enum{DataIoTransPacket,DataIoUpload,ControlIoSend,ControlIoRecv,IoQuit} IO_TYPE;
typedef enum{HandleControl,HandleData} HANDLE_TYPE;
typedef enum{ToSendData,ToReceiveData,/*ToCreatPasvConnect,ToCreatPortConnect,*/ToReceive,ToAbort,DoNothing} NEXT_TO_DO;
typedef struct _FILE_INFO                                   ///����һ���ļ��ṹ��������ȡ�ļ�����
{
	TCHAR cFileName[MAX_PATH];                              ///�ļ�����
	DWORD dwFileAttributes;                                 ///�ļ����ԣ��Ƿ�Ŀ¼��
    FILETIME ftCreationTime;                                ///�ļ�����ʱ��
    FILETIME ftLastAccessTime;                              ///�ļ���������ʱ��
    FILETIME ftLastWriteTime;                               ///�ļ�������޸�ʱ��
    DWORD nFileSizeHigh;                                    ///�ļ���С�ĸ�32λ
    DWORD nFileSizeLow;                                     ///�ļ���С�ĵ�32λ
} FILE_INFO, *LPFILE_INFO;
struct ContolHandleInfo
{
	char strCurrentWorkPath[1024];//�ÿ�������Ŀǰ���ڵĹ���·��
	void * pDataLinkSocketData;//һ��ָ���ڱ��������Ӷ�Ӧ���������Ӷ�Ӧ��PER_SOCKET_DATA�ṹ��ָ��
	void * pDataLinkSocketDataBuffer;//ָ��洢DataLinkSocketData���ڴ�� 
	char strCurrentName[MAXUSERNAMELENGTH];
	bool bBePasv;// ��ʾ�ÿ������Ӷ�Ӧ���������������Ƿ�ΪPASV
	HANDLE EventDataValid;//���������Ƿ���Ч
	DWORD dwDataResetPosition;//��Ӧ�������Ӵ����ļ���ƫ����
	HANDLE hCurrentFileHandle;//���û����̵�ǰ�򿪵��ļ����
};
struct DataHandleInfo
{
	void * pControlLinkSocketData;//һ��ָ���ڱ��������Ӷ�Ӧ�Ŀ������Ӷ�Ӧ��PER_SOCKET_DATA�ṹ��ָ��

	bool bValid;//��ʾ��ǰ�����Ƿ���Ч
	WSAEVENT weAcceptEvent;
	bool bAbor;//������ʾ��ǰ�����������ڱ���ֹ
	
};
typedef struct 
{
	SOCKET Socket;    //�������ӻ����������׽���
	HANDLE_TYPE enCurrentHandleType;//��ʾ��ǰ����ʱ�������ӻ��ǿ�������
	union 
	{
		ContolHandleInfo cCtrlInfo;
		DataHandleInfo cDataInfo;
	} uHandleInfo;
}PER_HANDLE_DATA,*LPPER_HANDLE_DATA;


struct DataIoTransPacketInfo//�ͻ��������ļ� 
{
	ULONG uTotolFileSize;
	HANDLE hTmpFile;//��Ҫ�رյ��ļ������
};
struct DataIoUploadInfo//�ͻ����ϴ��ļ�
{
	HANDLE hFileHandle;//�ͻ��ϴ��ļ��ڱ��ش洢�ļ��ľ��
	ULONG uTotolFileSize;
	ULONG uReceivedFileSize;
};
struct ControlIoSendInfo
{
	DWORD dwSendedDataLength;//���ڣףӣ��ӣ��䡡����һ�η�������һ���ֵĿ��ܡ���ʹ�ø�ֵ�����ۼƷ��͵��ֽ���
	NEXT_TO_DO enNextToDo;//��ʾ��һ������Ĳ���
	BYTE bWhatToDo[256];//�洢һЩ�����һ����������Ϣ ������������һ����������
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

extern CRITICAL_SECTION  PerSocketVectorProtection;//��Ϊ����vector��ȫ�ֻ�����
extern HANDLE CompetionPort;//��Ϊȫ�ֵ���ɶ˿ڵľ��
extern CRITICAL_SECTION  AddIocpProtection;//Ϊ�������ɶ˿ڴ���socket��ȫ�ֱ���

typedef vector <LPPER_SOCKET_DATA> PerSocketDataVector;
extern PerSocketDataVector cPerSocketDataVector;
#endif