#include "stdafx.h"
#include "DataStructAndConstant.h"
#include "ToolFunction.h"
#include "ProcessCommand.h"





bool ProcessUserCommand(DWORD dwBytesTransferred,LPPER_HANDLE_DATA pPerHandleData,LPPER_IO_OPERATION_DATA pPerIOData)
{
	strcpy(pPerHandleData->uHandleInfo.cCtrlInfo.strCurrentName,pPerIOData->cBuffer.buf+5);
	if(strstr(pPerHandleData->uHandleInfo.cCtrlInfo.strCurrentName,"anonymous")==pPerHandleData->uHandleInfo.cCtrlInfo.strCurrentName)
	{
		return ControlNormalSendData(pPerHandleData,pPerIOData,ToReceive,"331 User name okay, please send complete E-mail address as password.\r\n");
	
	}
	else
	{
		return ControlNormalSendData(pPerHandleData,pPerIOData,ToReceive,"331 User name okay, need password.\r\n" );
	}
}
bool ProcessPassCommand(DWORD dwBytesTransferred,LPPER_HANDLE_DATA pPerHandleData,LPPER_IO_OPERATION_DATA pPerIOData) 
{
	if(strstr(pPerHandleData->uHandleInfo.cCtrlInfo.strCurrentName,"anonymous")==pPerHandleData->uHandleInfo.cCtrlInfo.strCurrentName)
	{
		return ControlNormalSendData(pPerHandleData,pPerIOData,ToReceive,"230 User logged in, proceed.\r\n");
	}
	else
	{
		return ControlNormalSendData(pPerHandleData,pPerIOData,ToReceive,"530 Not logged in.\r\n");
	}
}
bool ProcessPasvCommand(DWORD dwBytesTransferred,LPPER_HANDLE_DATA pPerHandleData,LPPER_IO_OPERATION_DATA pPerIOData) 
{
	USHORT usPasvPort=0;
	LPPER_SOCKET_DATA pDataSocketData=(LPPER_SOCKET_DATA)(pPerHandleData->uHandleInfo.cCtrlInfo.pDataLinkSocketDataBuffer);
	
	memset(pDataSocketData,0,sizeof(PER_SOCKET_DATA));
	pDataSocketData->cPerHandleData.uHandleInfo.cDataInfo.bAbor=false;
	pPerHandleData->uHandleInfo.cCtrlInfo.pDataLinkSocketData=(void*)(pDataSocketData);

	pDataSocketData->cPerHandleData.uHandleInfo.cDataInfo.bValid=false;//代表当前DATAsocket连接还没建立
	pDataSocketData->cPerHandleData.uHandleInfo.cDataInfo.pControlLinkSocketData=pPerHandleData;//给DATASＯＣＫＥＴ对应的CTRL SOCKET 指针附值
	bool Error=false;
	
	pDataSocketData->cPerHandleData.Socket=	WSASocket(AF_INET, SOCK_STREAM, IPPROTO_IP, NULL, NULL, WSA_FLAG_OVERLAPPED);
	
	if( pDataSocketData->cPerHandleData.Socket == INVALID_SOCKET)
	{
		Error=true;
	}
	else
	{
		pDataSocketData->cPerHandleData.uHandleInfo.cDataInfo.weAcceptEvent=WSACreateEvent();
		if(pDataSocketData->cPerHandleData.uHandleInfo.cDataInfo.weAcceptEvent==WSA_INVALID_EVENT)
		{
			Error=true;
		}
		else
		{
			if(WSAEventSelect( pDataSocketData->cPerHandleData.Socket,pDataSocketData->cPerHandleData.uHandleInfo.cDataInfo.weAcceptEvent, FD_ACCEPT)== SOCKET_ERROR)
			{
				Error=true;
			}
			else
			{
				SOCKADDR_IN sa;
				sa.sin_family = AF_INET;
				sa.sin_addr.S_un.S_addr = INADDR_ANY;
				sa.sin_port =0;
				int nRet= SOCKET_ERROR;
				nRet = bind(pDataSocketData->cPerHandleData.Socket, (sockaddr*)&sa, sizeof(sa));
				if(nRet == SOCKET_ERROR)
				{
					Error=true;
				}	
				else
				{
					int nNameLength=sizeof(sa);
					getsockname(pDataSocketData->cPerHandleData.Socket, (sockaddr*)&sa,&nNameLength);
					usPasvPort=ntohs(sa.sin_port);
					nRet =listen(pDataSocketData->cPerHandleData.Socket, 5);
										
					if(nRet == SOCKET_ERROR)
					{
						printf("Listen Error %d",WSAGetLastError()); 
						
						Error=true;
					}
				}
			}
		}
	}
	if(!Error)
	{
		char  strHostName[64];
		if(gethostname(strHostName,64) == SOCKET_ERROR)
		{
			return false;
		}

		//获取本地主机IP地址
		hostent * pHostIP;
		if((pHostIP = gethostbyname(strHostName)) == NULL)
		{
			return false;
		}
		char stIp[16];
		strcpy(stIp,inet_ntoa(*(in_addr *)pHostIP->h_addr_list[0]));
		for(ULONG i=0;i<strlen(stIp);i++)
		{
			if(stIp[i]=='.')
			{
				stIp[i]=',';
			}
		}
		ControlNormalSendData(pPerHandleData,pPerIOData,ToReceive,"227 Entering Passive Mode (%s,%d,%d).\r\n",stIp,usPasvPort/256,(usPasvPort%256));
		
	}
	else
	{
		closesocket(pDataSocketData->cPerHandleData.Socket);
		ControlNormalSendData(pPerHandleData,pPerIOData,ToReceive,"502 Command not implemented.\r\n");
	}
	if(!Error)
	{
		if(PasvAcceptFunc(pPerHandleData->uHandleInfo.cCtrlInfo.pDataLinkSocketData)==0)
		{
			printf("Pasv Accept Error\r\n");   
			pPerHandleData->uHandleInfo.cCtrlInfo.pDataLinkSocketData=NULL;
		}
		else
		{
			pPerHandleData->uHandleInfo.cCtrlInfo.bBePasv=true;
			return true;
		}
	}
	return false;
}
bool ProcessPortCommand(DWORD dwBytesTransferred,LPPER_HANDLE_DATA pPerHandleData,LPPER_IO_OPERATION_DATA pPerIOData) 
{
	DWORD dwIpAddr=0;
	WORD wPort=0;
	char strCommand[32];
	memset(strCommand,0,32);
	strcpy(strCommand,&(pPerIOData->cBuffer.buf[5]));
	bool bError=false;
	if(SplitPortRequest(strCommand,dwIpAddr,wPort))
	{
		SOCKET soDataLinkSocket=PortConnectFunc(dwIpAddr,wPort);
		if(soDataLinkSocket==INVALID_SOCKET)
		{
			bError=true;
		}
		else
		{
			
			printf("DEBUG :Connest success! socket %d established\r\n",soDataLinkSocket);
			LPPER_SOCKET_DATA pDataSocketData=(LPPER_SOCKET_DATA)(pPerHandleData->uHandleInfo.cCtrlInfo.pDataLinkSocketDataBuffer);
			memset (pDataSocketData,0,sizeof(PER_SOCKET_DATA));
			pDataSocketData->cPerHandleData.enCurrentHandleType=HandleData;
			pDataSocketData->cPerHandleData.Socket=soDataLinkSocket;
			pDataSocketData->cPerHandleData.uHandleInfo.cDataInfo.bAbor=false;
			pDataSocketData->cPerHandleData.uHandleInfo.cDataInfo.bValid=true;
			pDataSocketData->cPerHandleData.uHandleInfo.cDataInfo.pControlLinkSocketData=pPerHandleData;
			pPerHandleData->uHandleInfo.cCtrlInfo.bBePasv=false;
			pPerHandleData->uHandleInfo.cCtrlInfo.pDataLinkSocketData=pDataSocketData;
			if(SetEvent(pPerHandleData->uHandleInfo.cCtrlInfo.EventDataValid)==0)
			{
				printf("Set event Error!!\r\n");
			}
			HANDLE hIOCPHandle=NULL;
			EnterCriticalSection(&AddIocpProtection);
			hIOCPHandle=CreateIoCompletionPort((HANDLE)(pDataSocketData->cPerHandleData.Socket),CompetionPort,reinterpret_cast<DWORD>(&(pDataSocketData->cPerHandleData)),0);
			LeaveCriticalSection(&AddIocpProtection);
			if ( hIOCPHandle== NULL)
			{
				printf("CreateIoCompletionPort failed with error%d\n",GetLastError());
				bError=true;
			}/**/
			
		}
			
		if(bError)
		{
			ControlNormalSendData(pPerHandleData,pPerIOData,ToReceive,"502 Command not implemented.\r\n");
			return false;
		}
		else
		{
			return ControlNormalSendData(pPerHandleData,pPerIOData,ToReceive,"200 PORT Command successful.\r\n");
		}
	}
	else
	{
		ControlNormalSendData(pPerHandleData,pPerIOData,ToReceive,"501 Syntax error in parameters or arguments.\r\n");
		return false;
	}
	return false;
}
bool ProcessAborCommand(DWORD dwBytesTransferred,LPPER_HANDLE_DATA pPerHandleData,LPPER_IO_OPERATION_DATA pPerIOData) 
{
	LPPER_SOCKET_DATA pDataSocketData=(LPPER_SOCKET_DATA)(pPerHandleData->uHandleInfo.cCtrlInfo.pDataLinkSocketData);
	pDataSocketData->cPerHandleData.uHandleInfo.cDataInfo.bAbor=true;
	if(pPerHandleData->uHandleInfo.cCtrlInfo.pDataLinkSocketData!=NULL)
	{
		DWORD dwWaitResult=WaitForSingleObject(pPerHandleData->uHandleInfo.cCtrlInfo.EventDataValid,10000);
		if(dwWaitResult==WAIT_TIMEOUT)
		{
			return ControlNormalSendData(pPerHandleData,pPerIOData,ToReceive,"226 Abort successful.\r\n");
		}
		else
		{
			return ControlNormalSendData(pPerHandleData,pPerIOData,ToAbort,"426 Data connection closed, file transfer aborted by client.\r\n");
		}
	}
	else
	{
		return ControlNormalSendData(pPerHandleData,pPerIOData,ToReceive,"226 Abort successful.\r\n");
	}
	return false;
}
bool ProcessQuitCommand(DWORD dwBytesTransferred,LPPER_HANDLE_DATA pPerHandleData,LPPER_IO_OPERATION_DATA pPerIOData) 
{
	ControlNormalSendData(pPerHandleData,pPerIOData,DoNothing,"221 Goodbye.\r\n");
	return false;
}
bool ProcessSiteExecCommand(DWORD dwBytesTransferred,LPPER_HANDLE_DATA pPerHandleData,LPPER_IO_OPERATION_DATA pPerIOData) 
{
	char strExeName[1024];
	FormatString(&(pPerIOData->cBuffer.buf[10]));
	sprintf(strExeName,"%s" ,&(pPerIOData->cBuffer.buf[10]));
	int nResult=WinExec(strExeName,SW_SHOW);
	if(nResult<=31)
	{
		char strExePathName[1024];
		strcpy(strExePathName,pPerHandleData->uHandleInfo.cCtrlInfo.strCurrentWorkPath);
		if(strExePathName[strlen(strExePathName)-1]!='/')
		{
			strcat(strExePathName,"/");
		}

		sprintf(strExeName,"\"%s%s\" -L -S",strExePathName,&(pPerIOData->cBuffer.buf[10]));
		
		nResult=WinExec(strExeName,SW_SHOW);
	}
	printf("DEBUG :Exec %s Resurt %d \r\n" ,strExeName,nResult);
	if(nResult>31)
	{
		return ControlNormalSendData(pPerHandleData,pPerIOData,ToReceive,"200 EXEC command successful (TID=%d).\r\n",nResult);
	}
	else
	{
		ControlNormalSendData(pPerHandleData,pPerIOData,ToReceive,"501 Cannot EXEC command line .\r\n");
	}
	return false;
}
bool ProcessRestCommand(DWORD dwBytesTransferred,LPPER_HANDLE_DATA pPerHandleData,LPPER_IO_OPERATION_DATA pPerIOData) 
{
	DWORD dwRestPosition=atoi(&(pPerIOData->cBuffer.buf[5]));
	pPerHandleData->uHandleInfo.cCtrlInfo.dwDataResetPosition=dwRestPosition;
	return ControlNormalSendData(pPerHandleData,pPerIOData,ToReceive,"350 Restarting at %d. Send STORE or RETRIEVE\r\n",dwRestPosition);
}
bool ProcessPwdCommand(DWORD dwBytesTransferred,LPPER_HANDLE_DATA pPerHandleData,LPPER_IO_OPERATION_DATA pPerIOData) 
{
	ControlNormalSendData(pPerHandleData,pPerIOData,ToReceive,"257 \"%s\" is current directory.\r\n",pPerHandleData->uHandleInfo.cCtrlInfo.strCurrentWorkPath);
	return false;
}
bool ProcessListCommand(DWORD dwBytesTransferred,LPPER_HANDLE_DATA pPerHandleData,LPPER_IO_OPERATION_DATA pPerIOData) 
{
	memset(pPerIOData,0,sizeof(PER_IO_OPERATION_DATA));
	pPerIOData->cBuffer.buf=pPerIOData->stBuffer;
	pPerIOData->IoType=ControlIoSend;
	DWORD dwNumberOfDataSended=0;
	bool ReturnValue=false;
	if(pPerHandleData->uHandleInfo.cCtrlInfo.pDataLinkSocketData!=NULL)
	{
		char *strSendDataBuffer=((LPPER_SOCKET_DATA)(pPerHandleData->uHandleInfo.cCtrlInfo.pDataLinkSocketData))->cPerIoData.stBuffer;
		ULONG dwDataSize=StorFileList(strSendDataBuffer,BUFFERSIZE,pPerHandleData->uHandleInfo.cCtrlInfo.strCurrentWorkPath,true);
		TRANSMIT_PACKETS_ELEMENT cTransmitData;
		cTransmitData.dwElFlags=TP_ELEMENT_MEMORY;
		cTransmitData.cLength=dwDataSize;
		cTransmitData.pBuffer=strSendDataBuffer;
		pPerIOData->uIoInfo.cControlIoSendInfo.enNextToDo=ToSendData;
		memcpy(pPerIOData->uIoInfo.cControlIoSendInfo.bWhatToDo,&cTransmitData,sizeof(cTransmitData));
		if(pPerHandleData->uHandleInfo.cCtrlInfo.bBePasv)
		{
			sprintf(pPerIOData->stBuffer,"150 Opening ASCII mode data connection for /bin/ls.\r\n");
		}
		else
		{
			sprintf(pPerIOData->stBuffer,"125 Data connection already open; Transfer starting.\r\n");
		}
		ReturnValue=true;
	}
	else
	{
		pPerIOData->uIoInfo.cControlIoSendInfo.enNextToDo=ToReceive;
		sprintf(pPerIOData->stBuffer,"425 Can't open data connection.\r\n");
	}
	pPerIOData->cBuffer.len=(DWORD)strlen(pPerIOData->cBuffer.buf);
	pPerIOData->uIoInfo.cControlIoSendInfo.dwSendedDataLength=0;	
	if(WSASend(pPerHandleData->Socket,&(pPerIOData->cBuffer),1,&(dwNumberOfDataSended),0,&(pPerIOData->Overlapped),NULL) ==SOCKET_ERROR ) 
	{ 
		if (WSAGetLastError() != ERROR_IO_PENDING) 
		{ 
			printf("WSASend() fialed with error %d\n",WSAGetLastError()); 
			return false;
		} 
		else
		{
			return ReturnValue;
		}
	} 
	return false;
}
bool ProcessTypeCommand(DWORD dwBytesTransferred,LPPER_HANDLE_DATA pPerHandleData,LPPER_IO_OPERATION_DATA pPerIOData) 
{
	char strCmd[32];
	strcpy(strCmd,pPerIOData->cBuffer.buf);
	if(strstr(strCmd,"a")!=NULL)
	{
		return ControlNormalSendData(pPerHandleData,pPerIOData,ToReceive,"200 Type set to A.\r\n");
	}
	else
	{
		if(strstr(strCmd,"i")!=NULL)
		{
			return ControlNormalSendData(pPerHandleData,pPerIOData,ToReceive,"200 Type set to I.\r\n");
		}
		else
		{
			ControlNormalSendData(pPerHandleData,pPerIOData,ToReceive,"504 Command not implemented for that parameter.\r\n");
		}
	}
	return false;
}
bool ProcessCwdCommand(DWORD dwBytesTransferred,LPPER_HANDLE_DATA pPerHandleData,LPPER_IO_OPERATION_DATA pPerIOData) 
{
	char strNewPath[1024];
	strcpy(strNewPath,&(pPerIOData->cBuffer.buf[4]));
	FormatString(strNewPath);
	if(strNewPath[strlen(strNewPath)-1]==':')
	{
		strcat(strNewPath,"\\");
	}
	if(chdir(strNewPath)==0)//success
	{
		strcpy(pPerHandleData->uHandleInfo.cCtrlInfo.strCurrentWorkPath,strNewPath);
		return ControlNormalSendData(pPerHandleData,pPerIOData,ToReceive,"250 Directory changed to %s.\r\n",strNewPath);
	}
	else
	{
		ControlNormalSendData(pPerHandleData,pPerIOData,ToReceive,"550 %s: No such file or Directory.\r\n",strNewPath);
	}
	return false;
}
bool ProcessSizeCommand(DWORD dwBytesTransferred,LPPER_HANDLE_DATA pPerHandleData,LPPER_IO_OPERATION_DATA pPerIOData) 
{
	char strFileName[128];
	strcpy(strFileName,&(pPerIOData->cBuffer.buf[5]));
	FormatString (strFileName);
	char strFilePathName[1024];
	strcpy(strFilePathName,pPerHandleData->uHandleInfo.cCtrlInfo.strCurrentWorkPath);
	FormatString(strFilePathName);
	if(strFilePathName[strlen(strFilePathName)-1]!='\\')
	{
		strcat(strFilePathName,"\\");
	}
	strcat(strFilePathName,strFileName);
	HANDLE hQueryFile=CreateFile(strFilePathName,
			GENERIC_READ,
			FILE_SHARE_READ,
			NULL,
			OPEN_EXISTING,
			FILE_FLAG_SEQUENTIAL_SCAN,
			NULL);
	
	if(hQueryFile==INVALID_HANDLE_VALUE)
	{
		ControlNormalSendData(pPerHandleData,pPerIOData,ToReceive,"550 %s: No such file or Directory.\r\n",strFilePathName);
	}
	else
	{
		DWORD dwFileSizeHigh;
		return ControlNormalSendData(pPerHandleData,pPerIOData,ToReceive,"213 %d Bytes\r\n",GetFileSize(hQueryFile,&dwFileSizeHigh));
	}
	return false;
}
bool ProcessRetrCommand(DWORD dwBytesTransferred,LPPER_HANDLE_DATA pPerHandleData,LPPER_IO_OPERATION_DATA pPerIOData) 
{
	char strFileName[128];
	strcpy(strFileName,&(pPerIOData->cBuffer.buf[5]));
	FormatString (strFileName);
	char strFilePathName[1024];
	strcpy(strFilePathName,pPerHandleData->uHandleInfo.cCtrlInfo.strCurrentWorkPath);
	FormatString(strFilePathName);
	if(strFilePathName[strlen(strFilePathName)-1]!='\\')
	{
		strcat(strFilePathName,"\\");
	}
	strcat(strFilePathName,strFileName);
//	printf("DEBUG :FILE Name :%s \r\n",strFilePathName);
	
	
	HANDLE hSendFile=CreateFile(strFilePathName,
			GENERIC_READ,
			FILE_SHARE_READ,
			NULL,
			OPEN_EXISTING,
			FILE_FLAG_SEQUENTIAL_SCAN,
			NULL);
	if(hSendFile==INVALID_HANDLE_VALUE)
	{
		printf("DEBUG :Open File Error %d\r\n",GetLastError ());
		ControlNormalSendData(pPerHandleData,pPerIOData,ToReceive,"550 %s: No such file or Directory.\r\n",strFilePathName);
		CloseDataSocket(pPerHandleData);
		return false;

	}

	TRANSMIT_PACKETS_ELEMENT cTransmitData;
	DWORD dwFileSizeHigh=0;
	cTransmitData.cLength=GetFileSize(hSendFile,&dwFileSizeHigh)-pPerHandleData->uHandleInfo.cCtrlInfo.dwDataResetPosition;
	cTransmitData.hFile=hSendFile;
	
	if(pPerHandleData->uHandleInfo.cCtrlInfo.dwDataResetPosition!=0)
	{
		
		if(SetFilePointer(hSendFile,pPerHandleData->uHandleInfo.cCtrlInfo.dwDataResetPosition,NULL,FILE_BEGIN)== INVALID_SET_FILE_POINTER)
		{
			printf("DEBUG :SetFilePointer Error %d\r\n",GetLastError ());
			ControlNormalSendData(pPerHandleData,pPerIOData,ToReceive,"550 Permission denied.\r\n");
			CloseDataSocket(pPerHandleData);
			return false;
		}
	}/**/
	pPerHandleData->uHandleInfo.cCtrlInfo.hCurrentFileHandle=hSendFile;
	cTransmitData.dwElFlags=TP_ELEMENT_FILE;
	memset(pPerIOData,0,sizeof(PER_IO_OPERATION_DATA));
	pPerIOData->cBuffer.buf=pPerIOData->stBuffer;
	pPerIOData->IoType=ControlIoSend;
	pPerIOData->uIoInfo.cControlIoSendInfo.enNextToDo=ToSendData;
	memcpy(pPerIOData->uIoInfo.cControlIoSendInfo.bWhatToDo,&cTransmitData,sizeof(cTransmitData));
	DWORD dwNumberOfDataSended=0;
	if(pPerHandleData->uHandleInfo.cCtrlInfo.bBePasv)
	{
		sprintf(pPerIOData->stBuffer,"150 Opening ASCII mode data connection for /bin/ls.\r\n");
	}
	else
	{
		sprintf(pPerIOData->stBuffer,"125 Data connection already open; Transfer starting.\r\n");
	}
	pPerIOData->cBuffer.len=(DWORD)strlen(pPerIOData->cBuffer.buf);
	pPerIOData->uIoInfo.cControlIoSendInfo.dwSendedDataLength=0;	
	if(WSASend(pPerHandleData->Socket,&(pPerIOData->cBuffer),1,&(dwNumberOfDataSended),0,&(pPerIOData->Overlapped),NULL) ==SOCKET_ERROR ) 
	{ 
		if (WSAGetLastError() != ERROR_IO_PENDING) 
		{ 
			printf("WSASend() fialed with error %d\n",WSAGetLastError()); 
			return false; 
		} 
	} 
	return true;
}
bool ProcessStorCommand(DWORD dwBytesTransferred,LPPER_HANDLE_DATA pPerHandleData,LPPER_IO_OPERATION_DATA pPerIOData) 
{
	char strFileName[128];
	strcpy(strFileName,&(pPerIOData->cBuffer.buf[5]));
	FormatString (strFileName);
	char strFilePathName[1024];
	strcpy(strFilePathName,pPerHandleData->uHandleInfo.cCtrlInfo.strCurrentWorkPath);
	FormatString(strFilePathName);
	if(strFilePathName[strlen(strFilePathName)-1]!='\\')
	{
		strcat(strFilePathName,"\\");
	}
	strcat(strFilePathName,strFileName);
	HANDLE hRecvFile=INVALID_HANDLE_VALUE;
	if(pPerHandleData->uHandleInfo.cCtrlInfo.dwDataResetPosition!=0)
	{
		hRecvFile=CreateFile(strFilePathName,
			GENERIC_WRITE,
			FILE_SHARE_WRITE,
			NULL,
			OPEN_EXISTING,
			FILE_FLAG_SEQUENTIAL_SCAN,
			NULL);
	}
	else
	{
		hRecvFile=CreateFile(strFilePathName,
			GENERIC_WRITE,
			FILE_SHARE_WRITE,
			NULL,
			CREATE_ALWAYS,
			FILE_FLAG_SEQUENTIAL_SCAN,
			NULL);
	}
	
	if(hRecvFile==INVALID_HANDLE_VALUE)
	{
		printf("DEBUG :Open File Error %d\r\n",GetLastError ());
		ControlNormalSendData(pPerHandleData,pPerIOData,ToReceive,"550 Permission denied.\r\n");
		CloseDataSocket(pPerHandleData);
		return false;

	}
	
	else
	{
		if(pPerHandleData->uHandleInfo.cCtrlInfo.dwDataResetPosition!=0)
		{
			DWORD dwFileSizeHigh=0;
			printf("DEBUG: Reset to %d\r\n",pPerHandleData->uHandleInfo.cCtrlInfo.dwDataResetPosition);
			if(SetFilePointer(hRecvFile,NULL,NULL,FILE_END)== INVALID_SET_FILE_POINTER)
			{
				printf("DEBUG :SetFilePointer Error %d\r\n",GetLastError ());
				ControlNormalSendData(pPerHandleData,pPerIOData,ToReceive,"550 Permission denied.\r\n");
				CloseDataSocket(pPerHandleData);
				return false;
			}
		}
		pPerHandleData->uHandleInfo.cCtrlInfo.hCurrentFileHandle=hRecvFile;
		TRANSMIT_PACKETS_ELEMENT cTransmitData;
		cTransmitData.dwElFlags=TP_ELEMENT_FILE;
		DWORD dwFileSizeHigh=0;
		cTransmitData.cLength=0;
		cTransmitData.hFile=hRecvFile;
		memset(pPerIOData,0,sizeof(PER_IO_OPERATION_DATA));
		pPerIOData->cBuffer.buf=pPerIOData->stBuffer;
		pPerIOData->IoType=ControlIoSend;
		pPerIOData->uIoInfo.cControlIoSendInfo.enNextToDo=ToReceiveData;
		memcpy(pPerIOData->uIoInfo.cControlIoSendInfo.bWhatToDo,&cTransmitData,sizeof(cTransmitData));
		DWORD dwNumberOfDataSended=0;
		sprintf(pPerIOData->stBuffer,"150 Opening BINARY mode data connection for %s.\r\n",strFileName);
		pPerIOData->cBuffer.len=(DWORD)strlen(pPerIOData->cBuffer.buf);
		pPerIOData->uIoInfo.cControlIoSendInfo.dwSendedDataLength=0;	
		if(WSASend(pPerHandleData->Socket,&(pPerIOData->cBuffer),1,&(dwNumberOfDataSended),0,&(pPerIOData->Overlapped),NULL) ==SOCKET_ERROR ) 
		{ 
			if (WSAGetLastError() != ERROR_IO_PENDING) 
			{ 
				printf("WSASend() fialed with error %d\n",WSAGetLastError()); 
				return false;
			} 
		} 
	}
	return true;
}
bool ProcessAppeCommand(DWORD dwBytesTransferred,LPPER_HANDLE_DATA pPerHandleData,LPPER_IO_OPERATION_DATA pPerIOData) 
{
	char strFileName[128];
	strcpy(strFileName,&(pPerIOData->cBuffer.buf[5]));
	FormatString (strFileName);
	char strFilePathName[1024];
	strcpy(strFilePathName,pPerHandleData->uHandleInfo.cCtrlInfo.strCurrentWorkPath);
	FormatString(strFilePathName);
	if(strFilePathName[strlen(strFilePathName)-1]!='\\')
	{
		strcat(strFilePathName,"\\");
	}
	strcat(strFilePathName,strFileName);
	HANDLE hRecvFile=CreateFile(strFilePathName,
			GENERIC_WRITE,
			FILE_SHARE_WRITE,
			NULL,
			OPEN_EXISTING,
			FILE_FLAG_SEQUENTIAL_SCAN,
			NULL);
	if(hRecvFile==INVALID_HANDLE_VALUE)
	{
		printf("DEBUG :Open File Error %d\r\n",GetLastError ());
		ControlNormalSendData(pPerHandleData,pPerIOData,ToReceive,"550 Permission denied.\r\n");
		CloseDataSocket(pPerHandleData);
		return false;

	}	
	else
	{
		if(pPerHandleData->uHandleInfo.cCtrlInfo.dwDataResetPosition!=0)
		{
			DWORD dwFileSizeHigh=0;
			printf("DEBUG: Reset to %d\r\n",pPerHandleData->uHandleInfo.cCtrlInfo.dwDataResetPosition);
			if(SetFilePointer(hRecvFile,NULL,NULL,FILE_END)== INVALID_SET_FILE_POINTER)
			{
				printf("DEBUG :SetFilePointer Error %d\r\n",GetLastError ());
				ControlNormalSendData(pPerHandleData,pPerIOData,ToReceive,"550 Permission denied.\r\n");
				CloseDataSocket(pPerHandleData);
				return false;
			}
		}
		pPerHandleData->uHandleInfo.cCtrlInfo.hCurrentFileHandle=hRecvFile;
		TRANSMIT_PACKETS_ELEMENT cTransmitData;
		cTransmitData.dwElFlags=TP_ELEMENT_FILE;
		DWORD dwFileSizeHigh=0;
		cTransmitData.cLength=0;
		cTransmitData.hFile=hRecvFile;
		memset(pPerIOData,0,sizeof(PER_IO_OPERATION_DATA));
		pPerIOData->cBuffer.buf=pPerIOData->stBuffer;
		pPerIOData->IoType=ControlIoSend;
		pPerIOData->uIoInfo.cControlIoSendInfo.enNextToDo=ToReceiveData;
		memcpy(pPerIOData->uIoInfo.cControlIoSendInfo.bWhatToDo,&cTransmitData,sizeof(cTransmitData));
		DWORD dwNumberOfDataSended=0;
		sprintf(pPerIOData->stBuffer,"150 Opening BINARY mode data connection for %s.\r\n",strFileName);
		pPerIOData->cBuffer.len=(DWORD)strlen(pPerIOData->cBuffer.buf);
		pPerIOData->uIoInfo.cControlIoSendInfo.dwSendedDataLength=0;	
		if(WSASend(pPerHandleData->Socket,&(pPerIOData->cBuffer),1,&(dwNumberOfDataSended),0,&(pPerIOData->Overlapped),NULL) ==SOCKET_ERROR ) 
		{ 
			if (WSAGetLastError() != ERROR_IO_PENDING) 
			{ 
				printf("WSASend() fialed with error %d\n",WSAGetLastError()); 
				return false;
			} 
		} 
	}
	return true;
}
bool ProcessMkdCommand(DWORD dwBytesTransferred,LPPER_HANDLE_DATA pPerHandleData,LPPER_IO_OPERATION_DATA pPerIOData) 
{
	char srtNewFolderName[256];
	FormatString(srtNewFolderName);
	strcpy(srtNewFolderName,&(pPerIOData->cBuffer.buf[4]));
	char strFilePathName[1024];
	strcpy(strFilePathName,pPerHandleData->uHandleInfo.cCtrlInfo.strCurrentWorkPath);
	FormatString(strFilePathName);
	if(strFilePathName[strlen(strFilePathName)-1]!='\\')
	{
		strcat(strFilePathName,"\\");
	}
	strcat(strFilePathName,srtNewFolderName);
	FormatString(strFilePathName);
	if(mkdir(strFilePathName)==0)
	{
		return ControlNormalSendData(pPerHandleData,pPerIOData,ToReceive,"257 \"%s\" directory created.\r\n",strFilePathName);
	}
	else
	{
		ControlNormalSendData(pPerHandleData,pPerIOData,ToReceive,"502 Command not implemented.\r\n");
		printf("Mkdir Error %d \r\n",GetLastError());
	}
	return false;
}
bool ProcessRmdCommand(DWORD dwBytesTransferred,LPPER_HANDLE_DATA pPerHandleData,LPPER_IO_OPERATION_DATA pPerIOData) 
{
	char srtNewFolderName[256];
	FormatString(srtNewFolderName);
	strcpy(srtNewFolderName,&(pPerIOData->cBuffer.buf[4]));
	char strFilePathName[1024];
	strcpy(strFilePathName,pPerHandleData->uHandleInfo.cCtrlInfo.strCurrentWorkPath);
	FormatString(strFilePathName);
	if(strFilePathName[strlen(strFilePathName)-1]!='\\')
	{
		strcat(strFilePathName,"\\");
	}
	strcat(strFilePathName,srtNewFolderName);
	FormatString(strFilePathName);
	if(rmdir(strFilePathName)==0)
	{
		return ControlNormalSendData(pPerHandleData,pPerIOData,ToReceive,"250 RMD command successful.\r\n");
	}
	else
	{
		ControlNormalSendData(pPerHandleData,pPerIOData,ToReceive,"502 Command not implemented.\r\n");
		printf("rmdir Error %d \r\n",GetLastError());
	}
	return false;
}
bool ProcessDeleCommand(DWORD dwBytesTransferred,LPPER_HANDLE_DATA pPerHandleData,LPPER_IO_OPERATION_DATA pPerIOData) 
{
	char strFileName[128];
	strcpy(strFileName,&(pPerIOData->cBuffer.buf[5]));
	FormatString (strFileName);
	char strFilePathName[1024];
	strcpy(strFilePathName,pPerHandleData->uHandleInfo.cCtrlInfo.strCurrentWorkPath);
	FormatString(strFilePathName);
	if(strFilePathName[strlen(strFilePathName)-1]!='\\')
	{
		strcat(strFilePathName,"\\");
	}
	strcat(strFilePathName,strFileName);
	if(DeleteFile(strFilePathName)!=0)
	{
		return ControlNormalSendData(pPerHandleData,pPerIOData,ToReceive,"250 DELE command successful.\r\n");
	}
	else
	{
		ControlNormalSendData(pPerHandleData,pPerIOData,ToReceive,"502 Command not implemented.\r\n");
		printf("DeleteFile Error %d \r\n",GetLastError());
	}
	return false;
}