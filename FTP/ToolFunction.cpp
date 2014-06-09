
#include "stdafx.h"
#include "DataStructAndConstant.h"
#include "ToolFunction.h"

DWORD GetFileList(LPFILE_INFO lpFI,DWORD dwFIsize,char* strPath)
	{
		WIN32_FIND_DATA wfd;
		DWORD count = 0;
		char strFileName[MAX_PATH];
		memset(strFileName,0,MAX_PATH);
		strcpy(strFileName,strPath);
		FormatString(strFileName);
		if(strFileName[strlen(strFileName)-1]!='\\')
		{
			strcat(strFileName,"\\*.*");
		}
		else
		{
			strcat(strFileName,"*.*");
		}
		HANDLE hFile = FindFirstFile(strFileName, &wfd);
		if (hFile != INVALID_HANDLE_VALUE) 
		{
			strcpy(lpFI[count].cFileName, wfd.cFileName);
			lpFI[count].dwFileAttributes = wfd.dwFileAttributes;	
			lpFI[count].ftCreationTime = wfd.ftCreationTime;
			lpFI[count].ftLastAccessTime = wfd.ftLastAccessTime;
			lpFI[count].ftLastWriteTime = wfd.ftLastWriteTime;
			lpFI[count].nFileSizeHigh = wfd.nFileSizeHigh;
			lpFI[count].nFileSizeLow = wfd.nFileSizeLow;
			count++;
			while(FindNextFile(hFile,&wfd) && count<dwFIsize) 
			{
				strcpy(lpFI[count].cFileName, wfd.cFileName);
				lpFI[count].dwFileAttributes = wfd.dwFileAttributes;	
				lpFI[count].ftCreationTime = wfd.ftCreationTime;
				lpFI[count].ftLastAccessTime = wfd.ftLastAccessTime;
				lpFI[count].ftLastWriteTime = wfd.ftLastWriteTime;
				lpFI[count].nFileSizeHigh = wfd.nFileSizeHigh;
				lpFI[count].nFileSizeLow = wfd.nFileSizeLow;
				count++;
			}
			FindClose(hFile);
		}
		return count;
	}
DWORD StorFileList(char* stStorBuff,DWORD dwMaxSize,char* strPath,bool bDetail)
	{
		FILE_INFO Files [MAX_FILE];
		DWORD dwFiles = GetFileList(Files,MAX_FILE,strPath);
		memset(stStorBuff,0,dwMaxSize);
		sprintf(stStorBuff,"%s","");
		if(!bDetail)
		{
			for(DWORD i=0;i<dwFiles;i++)
			{
				if(strlen(stStorBuff)+strlen(Files[i].cFileName)+2 >= dwMaxSize)
				{
					break;
				}
				strcat(stStorBuff,Files[i].cFileName);
				strcat(stStorBuff,"\r\n");
			}
		}
		else
		{
			for(DWORD i=0;i<dwFiles;i++)
			{
				if(strlen(stStorBuff)+strlen(Files[i].cFileName)+48 >= dwMaxSize)
				{
					break;
				}
				if(strcmp(Files[i].cFileName,".") == 0)
				{
					continue;
				}
				if(strcmp(Files[i].cFileName,"..") == 0)
				{
					continue;
				}

				bool bFolder = false;
				DWORD dwFilein;
				char tmpBuff[30];
				
				if(Files[i].dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)
				{
					bFolder = true;
				}

				//写入文件的权限
				if(bFolder)
				{
					strcat(stStorBuff,"drw-rw-rw- ");
				}
				else 
				{
					strcat(stStorBuff,"-rw-rw-rw- ");
				}

				//写入文件的数目
				memset(tmpBuff,0,30);
				if(bFolder)
				{

					char strFilePath[1024];
					strcpy(strFilePath,strPath);
					if(strFilePath[strlen(strFilePath)-1]!='\\')
					{
						strcat(strFilePath,"\\");
					}
					strcat(strFilePath,Files[i].cFileName);
					
					FILE_INFO Filein[MAX_FILE];
					dwFilein = GetFileList(Filein,MAX_FILE,strFilePath)-2;

					if(dwFilein == 0)
					{
						dwFilein = 1;
					}
				}
				else dwFilein = 1;
				sprintf(tmpBuff,"%4d ",dwFilein);
				strcat(stStorBuff,tmpBuff);

				//写入文件的类型
				if(bFolder)
				{
					strcat(stStorBuff,"<DIR> ");
				}
				else
				{
					strcat(stStorBuff,"<FIL> ");
				}

				//写入文件的大小
				memset(tmpBuff,0,30);
				sprintf(tmpBuff,"%10d ",Files[i].nFileSizeLow);
				strcat(stStorBuff,tmpBuff);
				
				//写入文件的修改时间
				SYSTEMTIME *Stime,Tmp;
				Stime=&Tmp;
				FileTimeToSystemTime(&(Files[i].ftLastWriteTime), Stime);
				char* Month[] = {"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};
				memset(tmpBuff,0,30);
				if(bFolder)
				{	
					sprintf(tmpBuff,"%s %2d %5d ",Month[--Stime->wMonth],Stime->wDay,Stime->wYear);
				}
				else
				{
					sprintf(tmpBuff,"%s %2d %02d:%02d ",Month[--Stime->wMonth],Stime->wDay,Stime->wHour,Stime->wMinute);
				}
				strcat(stStorBuff,tmpBuff);
				

				//写入文件名称
				strcat(stStorBuff,Files[i].cFileName);
				strcat(stStorBuff,"\r\n");
			}
		}
		
		if(!strlen(stStorBuff))
		{
			strcat(stStorBuff,"\r\n");
		}
		return (DWORD)strlen(stStorBuff);
	}
bool SplitPortRequest(char* strRequest,DWORD& dwIpAddr,WORD& wPort)
{
	char strIpAddr[20];
	char strPort[15];
	memset(strIpAddr,0,20);
	memset(strPort,0,5);
	ULONG i=0;
	for(i=0;i<strlen(	strRequest);i++)
	{
		if(strRequest[i]==',')
		{
			strRequest[i]='.';
		}
	}
	unsigned int count = 0,mark1=0,mark2=0;
	if(strlen(strRequest)>30)
	{
		return false;
	}	
	for(i=0;i<strlen(strRequest);i++)
	{
		if(strRequest[i] == '.')
		{
			count++;
		
			if(count == 4)
			{
				mark1 = i;
			}
			if(count == 5)
			{
				mark2 = i;
			}
		}
	}
	if(mark1==0)
	{
		return false;
	}
	char T[32];
	for(i=0;i<mark1;i++)
	{
		T[i]=(strRequest[i]);
	}
	T[i]=0;
	dwIpAddr = inet_addr(T);
	memset(strPort,0,5);
	memcpy(strPort,strRequest+mark1+1,mark2-mark1-1);
	wPort = (WORD)(256*atoi(strPort));
	memset(strPort,0,5);
	sprintf(strPort,"%s",strRequest+mark2+1);
	wPort += (WORD)atoi(strPort);
	return true;
}
	
void ControlIoReceive(LPPER_HANDLE_DATA pPerHandleData,LPPER_IO_OPERATION_DATA pPerIOData)
{
	LPPER_SOCKET_DATA pPerSocketData=(LPPER_SOCKET_DATA)pPerHandleData;
	if(pPerIOData!=&(pPerSocketData->cPerIoData))
	{
		printf("DEBUG :Some thing gose worng!\r\n");
	}
	memset (pPerIOData,0,sizeof(PER_IO_OPERATION_DATA));
	pPerIOData->cBuffer.buf=pPerIOData->stBuffer;
	pPerIOData->cBuffer.len=BUFFERSIZE;
	pPerIOData->IoType=ControlIoRecv;
	pPerIOData->uIoInfo.cControlIoRecvInfo.NoUse=0;
	DWORD RecvBytes=0,Flags=0;
	if (WSARecv(pPerHandleData->Socket,&(pPerIOData->cBuffer),1,&RecvBytes,&Flags,&(pPerIOData->Overlapped),NULL) == SOCKET_ERROR) 
	{ 
		if (WSAGetLastError() != ERROR_IO_PENDING) 
		{ 
			printf("Socket %d WSARecv() failed with error %d\n",pPerHandleData->Socket,WSAGetLastError()); 
			return; 
		} 
	}
}
void FormatString(char *strStrToFormat)
{
	for(ULONG i=0;i<strlen(strStrToFormat);i++)
	{
		if(strStrToFormat[i]==13||strStrToFormat[i]==10)
		{
			strStrToFormat[i]=0;
		}
	}
}
DWORD WINAPI PasvAcceptFunc(LPVOID Parament)
{
	LPPER_SOCKET_DATA pDataSocketData=(LPPER_SOCKET_DATA)Parament;
	LPPER_SOCKET_DATA pCtrlSocketData=(LPPER_SOCKET_DATA)(pDataSocketData->cPerHandleData.uHandleInfo.cDataInfo.pControlLinkSocketData);
	//printf("DEBUG Soclet %d to accept\r\n",pDataSocketData->cPerHandleData.Socket);
	SOCKET soDataAcceptSocket=SOCKET_ERROR;
	
	volatile int nErrorNo=0,nNumber=0;
	do
	{
		if(WaitForSingleObject(pDataSocketData->cPerHandleData.uHandleInfo.cDataInfo.weAcceptEvent,1000)!=WSA_WAIT_TIMEOUT)
		{
			WSANETWORKEVENTS events;
			if(WSAEnumNetworkEvents(pDataSocketData->cPerHandleData.Socket, pDataSocketData->cPerHandleData.uHandleInfo.cDataInfo.weAcceptEvent, &events)!= SOCKET_ERROR)
			{
				if(events.lNetworkEvents & FD_ACCEPT)
				{
					if(events.iErrorCode[FD_ACCEPT_BIT] == 0)
					{
					//	printf("DEBUG Socket %d is Accepting\r\n",pDataSocketData->cPerHandleData.Socket);
						soDataAcceptSocket = WSAAccept(pDataSocketData->cPerHandleData.Socket,NULL,NULL,NULL,0);
					}
				}
			}
		}
		nNumber++;
		nErrorNo=WSAGetLastError();
	}while(soDataAcceptSocket==SOCKET_ERROR&&(nErrorNo==10035||nErrorNo==997)&&nNumber<=6);
	if (soDataAcceptSocket == SOCKET_ERROR)
	{
		printf("WSAAccept failed with error %d on socket %d\n",WSAGetLastError(),pDataSocketData->cPerHandleData.Socket);
		WSACloseEvent(pDataSocketData->cPerHandleData.uHandleInfo.cDataInfo.weAcceptEvent);
		closesocket(pDataSocketData->cPerHandleData.Socket);
	}
	else
	{
	//	printf("DEBUG Soclet %d  accept Success!\r\n",pDataSocketData->cPerHandleData.Socket);
		WSACloseEvent(pDataSocketData->cPerHandleData.uHandleInfo.cDataInfo.weAcceptEvent);
		closesocket(pDataSocketData->cPerHandleData.Socket);
		pDataSocketData->cPerHandleData.Socket=soDataAcceptSocket;
		pDataSocketData->cPerHandleData.enCurrentHandleType=HandleData;
		pDataSocketData->cPerHandleData.uHandleInfo.cDataInfo.bValid=true;
		int zero=0;
		if ( setsockopt( soDataAcceptSocket, SOL_SOCKET, SO_SNDBUF, (char *)&zero, sizeof(zero) ) == SOCKET_ERROR )
		{
			return 0;
		}
		zero=0;  
		if ( setsockopt( soDataAcceptSocket, SOL_SOCKET, SO_RCVBUF, (char *)&zero, sizeof(zero) ) == SOCKET_ERROR )
		{
			return 0;
		}
		EnterCriticalSection(&AddIocpProtection);
		if ((CreateIoCompletionPort((HANDLE)soDataAcceptSocket,CompetionPort,reinterpret_cast<DWORD>(&(pDataSocketData->cPerHandleData)),0)) == NULL)
		{
			printf("CreateIoCompletionPort failed with error%d\n",GetLastError());
			LeaveCriticalSection(&AddIocpProtection);
			return 0;
		}
		LeaveCriticalSection(&AddIocpProtection);
		if(SetEvent(pCtrlSocketData->cPerHandleData.uHandleInfo.cCtrlInfo.EventDataValid)==0)
		{
			printf(" Set Event error!!\r\n");
		}
		return 1;

	}
	
	return 0;
	
}
SOCKET WINAPI PortConnectFunc(DWORD DataIpAddr,WORD DataPort)
{
	struct sockaddr_in ClientSocketAddr;
	memset(&ClientSocketAddr,0,sizeof(ClientSocketAddr));
	ClientSocketAddr.sin_family = AF_INET;
	ClientSocketAddr.sin_port = htons(DataPort);
	ClientSocketAddr.sin_addr.S_un.S_addr = DataIpAddr;
	SOCKET DataLinkSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_IP, NULL, NULL, WSA_FLAG_OVERLAPPED);
	if(DataLinkSocket==INVALID_SOCKET)
	{
		printf("WSASocket failed.Error:%d",WSAGetLastError());
		return INVALID_SOCKET ;
	}
	int nResult=WSAConnect(DataLinkSocket,(struct sockaddr *)&ClientSocketAddr,sizeof(ClientSocketAddr),NULL,NULL,NULL,NULL);
	int nTimes=0;
	while(nResult==SOCKET_ERROR&&nTimes<3)
	{
		nResult=WSAConnect(DataLinkSocket,(struct sockaddr *)&ClientSocketAddr,sizeof(ClientSocketAddr),NULL,NULL,NULL,NULL);
	}
	if(nResult==SOCKET_ERROR)
	{
		printf("Connect failed.Error:%d",WSAGetLastError());
		if (closesocket(DataLinkSocket) == SOCKET_ERROR) 
		{
			printf("closesocket failed with error %d\n",WSAGetLastError()); 
		} 
		return INVALID_SOCKET ;
	}
	else
	{
		int zero = 0;  
		if ( setsockopt( DataLinkSocket, SOL_SOCKET, SO_SNDBUF, (char *)&zero, sizeof(zero) ) == SOCKET_ERROR )
		{
			printf("Socket %d Accept Data link setsockopt Error %d",DataLinkSocket,WSAGetLastError());
			if (closesocket(DataLinkSocket) == SOCKET_ERROR) 
			{
				printf("closesocket failed with error %d\n",WSAGetLastError()); 
			} 

			return INVALID_SOCKET ;
		} 
		zero = 0;  

		if ( setsockopt( DataLinkSocket, SOL_SOCKET, SO_RCVBUF, (char *)&zero, sizeof(zero) ) == SOCKET_ERROR )
		{
			printf("Socket %d Accept Data link setsockopt Error %d",DataLinkSocket,WSAGetLastError());
			if (closesocket(DataLinkSocket) == SOCKET_ERROR) 
			{
				printf("closesocket failed with error %d\n",WSAGetLastError()); 
			}
			return INVALID_SOCKET ;
		} 
	}
	return DataLinkSocket;
}
bool DeleteVectorItem(LPPER_HANDLE_DATA pPerHandleData)
{
	int nItemDeletedNumber=0;
	if(pPerHandleData->enCurrentHandleType==HandleControl)
	{
		EnterCriticalSection(&PerSocketVectorProtection);
		PerSocketDataVector::iterator ItemDelete=cPerSocketDataVector.begin();
		for(ULONG i=0;i<cPerSocketDataVector.size();i++)
		{
			if(&((cPerSocketDataVector[i])->cPerHandleData)==pPerHandleData)
			{
				if(pPerHandleData->enCurrentHandleType==HandleControl)
				{
					CloseHandle(pPerHandleData->uHandleInfo.cCtrlInfo.EventDataValid);
				}
				delete (LPPER_SOCKET_DATA)(cPerSocketDataVector[i]->cPerHandleData.uHandleInfo.cCtrlInfo.pDataLinkSocketDataBuffer);
				delete cPerSocketDataVector[i];
				cPerSocketDataVector.erase(cPerSocketDataVector.begin()+i);
				nItemDeletedNumber++;
			}
		}
		LeaveCriticalSection(&PerSocketVectorProtection);
	

		if(nItemDeletedNumber>0)
		{
			if(nItemDeletedNumber!=1)
			{
				printf("Something goes worng when delete!!!!!!!!!!!!!!\r\n");
			}
			return true;
		}
	}
	return false;
}
bool CloseDataSocket(LPPER_HANDLE_DATA pPerHandleData)
{
	WaitForSingleObject(pPerHandleData->uHandleInfo.cCtrlInfo.EventDataValid,10000);
	ResetEvent(pPerHandleData->uHandleInfo.cCtrlInfo.EventDataValid);
	LPPER_SOCKET_DATA pPerSocketData=(LPPER_SOCKET_DATA)(pPerHandleData->uHandleInfo.cCtrlInfo.pDataLinkSocketData);
	if(pPerSocketData!=NULL)
	{
		closesocket(pPerSocketData->cPerHandleData.Socket);
		pPerHandleData->uHandleInfo.cCtrlInfo.pDataLinkSocketData=NULL;
	}
	return true;
}
bool ControlSocketPureSendData(LPPER_SOCKET_DATA pPerSocketData,char *pData)
{//这个函数顺便把指向数据连结信息的指针置为null，因为只有数据连接发送数据结束才有可能调用这个函数
	LPPER_SOCKET_DATA pControlSocketData=pPerSocketData;
	pControlSocketData->cPerHandleData.uHandleInfo.cCtrlInfo.pDataLinkSocketData=NULL;
	if(ResetEvent(pControlSocketData->cPerHandleData.uHandleInfo.cCtrlInfo.EventDataValid)==0)
	{
		printf(" Reset Event error!!\r\n");
	}
	memset(&(pControlSocketData->cPerIoData2),0,sizeof(PER_IO_OPERATION_DATA));
	pControlSocketData->cPerIoData2.cBuffer.buf=pControlSocketData->cPerIoData.stBuffer;
	pControlSocketData->cPerIoData2.IoType=ControlIoSend;
	pControlSocketData->cPerIoData2.uIoInfo.cControlIoSendInfo.enNextToDo=DoNothing;
	DWORD dwNumberOfDataSended=0;
	//printf("DEBUG : Socket %d Transmit data finish \r\n",pPerHandleData->Socket);
	
	strcpy(pControlSocketData->cPerIoData2.cBuffer.buf,pData);
	pControlSocketData->cPerIoData2.cBuffer.len=(DWORD)strlen(pControlSocketData->cPerIoData2.cBuffer.buf);
	pControlSocketData->cPerIoData2.uIoInfo.cControlIoSendInfo.dwSendedDataLength=0;	
	if(WSASend(pControlSocketData->cPerHandleData.Socket,&(pControlSocketData->cPerIoData2.cBuffer),1,&(dwNumberOfDataSended),0,&(pControlSocketData->cPerIoData2.Overlapped),NULL) ==SOCKET_ERROR ) 
	{ 
		if (WSAGetLastError() != ERROR_IO_PENDING) 
		{ 
			printf("WSASend() fialed with error %d\n",WSAGetLastError()); 
			return false; 
		} 
	} 
	return true;
}


bool ControlNormalSendData(LPPER_HANDLE_DATA pPerHandleData,LPPER_IO_OPERATION_DATA pPerIOData,NEXT_TO_DO mWhatToDo,char *DataSend,...)
{
	va_list ArgList;
	va_start(ArgList,DataSend);
	memset(pPerIOData,0,sizeof(PER_IO_OPERATION_DATA));
	pPerIOData->cBuffer.buf=pPerIOData->stBuffer;
	pPerIOData->IoType=ControlIoSend;
	pPerIOData->uIoInfo.cControlIoSendInfo.enNextToDo=mWhatToDo;
	DWORD dwNumberOfDataSended=0;
	
	vsprintf(pPerIOData->cBuffer.buf,DataSend,ArgList);
	va_end(ArgList);
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