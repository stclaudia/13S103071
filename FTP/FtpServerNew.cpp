// FtpServerNew.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
#include "DataStructAndConstant.h"
#include "TreadFunction.h"
CRITICAL_SECTION  PerSocketVectorProtection;//��Ϊ����vector��ȫ�ֻ�����
CRITICAL_SECTION  AddIocpProtection;//Ϊ�������ɶ˿ڴ���socket��ȫ�ֱ���
HANDLE CompetionPort;//��Ϊȫ�ֵ���ɶ˿ڵľ��
extern bool DeleteVectorItem(LPPER_HANDLE_DATA pPerHandleData);


PerSocketDataVector cPerSocketDataVector;
int _tmain(int argc, char* argv[])
{
	SOCKADDR_IN InternetAddr;
	SOCKET Listen,Accept;
	SYSTEM_INFO SystenInfo;
	DWORD ThreadID;
	WSADATA wsadata;
	DWORD Ret;
	if (Ret = WSAStartup(0x2020,&wsadata) != 0)
	{
		printf("WSAStartup failed with error %d\n",Ret);
		return 0;
	}
   //��һ���յ���ɶ˿�


	if ((CompetionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE,NULL,0,0)) == NULL)
	{
		printf("CreateIoCompletionPort failed with error %d\n",GetLastError());
		return 0;
	}
	else
	{
		printf("CreateIoCompletionPort Success����\r\n");
	}
	GetSystemInfo(&SystenInfo);
	// ����cpu������2�������߳� 
	for (ULONG i=0; i < 6+SystenInfo.dwNumberOfProcessors*2; i++)
	{
		HANDLE ThreadHandle;
		//���������������̣߳��������̴߳�����ɶ˿�
		if ((ThreadHandle = CreateThread(NULL,0,ServerWorkerThread,CompetionPort,0,&ThreadID)) == NULL)
		{
			printf("CreateThread failed with error %d\n" ,GetLastError());
			return 0;
		}
		CloseHandle(ThreadHandle);
	}
	printf("Start %d Threats����\r\n",6+SystenInfo.dwNumberOfProcessors*2);
	//��һ��������socket
	if ((Listen = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED)) == INVALID_SOCKET)
	{
		printf("WSASocket() failed with error %d\n", WSAGetLastError());
		return 0;
	}
	InternetAddr.sin_family = AF_INET;
	InternetAddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	InternetAddr.sin_port = htons(LISTENPORT);
	if (bind(Listen,(LPSOCKADDR)&InternetAddr,sizeof(InternetAddr)) == SOCKET_ERROR)
	{
		printf("bind failed with error %d\n",WSAGetLastError());
		return 0;
	}
    if (listen(Listen,5) == SOCKET_ERROR)
	{
		printf("listen failed with error %d\n",WSAGetLastError());
		return 0;
	}
	printf("Ftp Server Start������ \r\n");
	InitializeCriticalSection(&PerSocketVectorProtection);
	InitializeCriticalSection(&AddIocpProtection);
     //�������Ӳ��ҷַ�����ɶ˿�
	while (TRUE)
	{
		if ((Accept = WSAAccept(Listen,NULL,NULL,NULL,0)) == SOCKET_ERROR)
		{
			printf("WSAAccept failed with error %d\n",WSAGetLastError());
			continue;
		}
         //�������׽�����ص��׽�����Ϣ�ṹ
		LPPER_SOCKET_DATA pAcceptSocketData=new PER_SOCKET_DATA;
		EnterCriticalSection(&PerSocketVectorProtection);
			cPerSocketDataVector.push_back(pAcceptSocketData);
		LeaveCriticalSection(&PerSocketVectorProtection);
		memset((void*)pAcceptSocketData,0,sizeof(PER_SOCKET_DATA));
		pAcceptSocketData->cPerHandleData.Socket=Accept;
		pAcceptSocketData->cPerHandleData.uHandleInfo.cCtrlInfo.EventDataValid=CreateEvent(NULL,TRUE,FALSE,NULL);
		pAcceptSocketData->cPerHandleData.enCurrentHandleType=HandleControl;
		pAcceptSocketData->cPerHandleData.uHandleInfo.cCtrlInfo.pDataLinkSocketDataBuffer=(void *)new PER_SOCKET_DATA;
		strcpy(pAcceptSocketData->cPerHandleData.uHandleInfo .cCtrlInfo.strCurrentWorkPath,DEFAULTWORKPATH);
		//�����ǵĴ������Ǹ���ɶ˿ڹ�������,���ؼ���Ҳ��ָ����һ����ɶ˿ڹ���
		EnterCriticalSection(&AddIocpProtection);
		if ((CreateIoCompletionPort((HANDLE)Accept,CompetionPort,reinterpret_cast<DWORD>(&(pAcceptSocketData->cPerHandleData)),0)) == NULL)
		{
			printf("CreateIoCompletionPort failed with error%d\n",GetLastError());
			LeaveCriticalSection(&AddIocpProtection);
			closesocket(Accept);
			continue;
		}
		LeaveCriticalSection(&AddIocpProtection);
		int zero = 0; 
		if ( setsockopt(pAcceptSocketData->cPerHandleData.Socket, SOL_SOCKET, SO_SNDBUF, (char *)&zero, sizeof(zero) ) == SOCKET_ERROR )
		{
			printf("Set socket option Error %d",WSAGetLastError());
		}
		zero = 0; 
		if ( setsockopt(pAcceptSocketData->cPerHandleData.Socket, SOL_SOCKET, SO_RCVBUF, (char *)&zero, sizeof(zero) ) == SOCKET_ERROR )
		{
			printf("Set socket option Error %d",WSAGetLastError());
		}
 // ����ͬ�����WSARecv������ص�IO�׽�����Ϣ�ṹ��
		pAcceptSocketData->cPerIoData.cBuffer.buf=pAcceptSocketData->cPerIoData.stBuffer;
		strcpy(pAcceptSocketData->cPerIoData.cBuffer.buf,"220 Welcome to Access BlackBeast FTP Server...\r\n");
		pAcceptSocketData->cPerIoData.cBuffer.len=(DWORD)strlen(pAcceptSocketData->cPerIoData.cBuffer.buf);
		pAcceptSocketData->cPerIoData.IoType=ControlIoSend;
		pAcceptSocketData->cPerIoData.uIoInfo.cControlIoSendInfo.enNextToDo=ToReceive;
	
									
		DWORD SendBytes=0;
		if (WSASend(pAcceptSocketData->cPerHandleData.Socket,&(pAcceptSocketData->cPerIoData.cBuffer),1,&SendBytes,0,&(pAcceptSocketData->cPerIoData.Overlapped),NULL) ==SOCKET_ERROR ) 
		{ 
			if (WSAGetLastError() != ERROR_IO_PENDING) 
			{ 
				printf("WSASend() fialed with error %d\n",WSAGetLastError()); 
				DeleteVectorItem(&(pAcceptSocketData->cPerHandleData));
				closesocket(Accept);
				continue; 
			} 
		} 
		
	
	}
	return 0;
}
