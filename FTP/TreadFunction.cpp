#include "stdafx.h"
#include "DataStructAndConstant.h"

#include "ToolFunction.h"
#include "TreadFunction.h"
#include "ProcessCommand.h"
DWORD WINAPI ServerWorkerThread(LPVOID ComlpetionPortID) 
{ 
	HANDLE ComplectionPort = (HANDLE) ComlpetionPortID; 
	DWORD dwBytesTransferred; 
	LPPER_HANDLE_DATA pPerHandleData; 
	LPPER_IO_OPERATION_DATA pPerIOData; 
	while (TRUE) 
	{ 
		if (GetQueuedCompletionStatus(ComplectionPort,&dwBytesTransferred,(LPDWORD)&pPerHandleData,(LPOVERLAPPED*)&pPerIOData,INFINITE) == 0) 
		{ 
			DWORD dwErrorCode=GetLastError();
			
			if(pPerHandleData->enCurrentHandleType==HandleData&&pPerIOData->IoType==DataIoTransPacket&&pPerHandleData->uHandleInfo.cDataInfo.bAbor==false)
			{//这是专门为了多线程下载文件使用的分支
				LPPER_SOCKET_DATA pCtrlSockData=(LPPER_SOCKET_DATA)(pPerHandleData->uHandleInfo.cDataInfo.pControlLinkSocketData);
				ControlSocketPureSendData(pCtrlSockData,"226 Transfer complete !\r\n");
				closesocket(pPerHandleData->Socket);
				if(pPerIOData->uIoInfo.cDataIoTransPacketInfo.hTmpFile!=INVALID_HANDLE_VALUE)	
				{
					CloseHandle(pPerIOData->uIoInfo.cDataIoTransPacketInfo.hTmpFile);
				}
				
			}
			else
			{
				printf("Socket %d 's GetQueuedCompletionStatus failed with error %d\n",pPerHandleData->Socket,dwErrorCode); 
			}/**/
			continue ; 
		}  
 
		if(dwBytesTransferred==0&&pPerIOData->IoType!=IoQuit&&pPerHandleData->enCurrentHandleType==HandleControl)//客户control端关闭连接
		{
			printf("Close Socket %d \r\n",pPerHandleData->Socket);
			closesocket(pPerHandleData->Socket);
			if(!DeleteVectorItem(pPerHandleData))
			{
				printf("Delete Error!\r\n");
			}

			continue;
		}
		if(pPerHandleData->enCurrentHandleType==HandleControl)//控制连接
		{
			LPPER_SOCKET_DATA pPerSocketData=(LPPER_SOCKET_DATA)pPerHandleData;
			switch(pPerIOData->IoType)
			{
			case ControlIoSend:
				{
					pPerIOData->uIoInfo.cControlIoSendInfo.dwSendedDataLength+=dwBytesTransferred;
					if(	pPerIOData->uIoInfo.cControlIoSendInfo.dwSendedDataLength==pPerIOData->cBuffer.len)
					{//全部发送完成
						switch (pPerIOData->uIoInfo.cControlIoSendInfo.enNextToDo)
						{
						case ToSendData:
							{
								LPPER_SOCKET_DATA pDataLinkPerSocketData=(LPPER_SOCKET_DATA)(pPerHandleData->uHandleInfo.cCtrlInfo.pDataLinkSocketData);
								DWORD dwWaitResult=WaitForSingleObject(pPerHandleData->uHandleInfo.cCtrlInfo.EventDataValid,10000);
								if(dwWaitResult==WAIT_TIMEOUT||pDataLinkPerSocketData==NULL)
								{	
									printf("Data Link is invalid!\r\n");
									ControlSocketPureSendData((LPPER_SOCKET_DATA)pPerHandleData,"425 Can't open data connection.\r\n");
									ControlIoReceive(pPerHandleData,pPerIOData);		
									break;
								}
								TRANSMIT_PACKETS_ELEMENT cTransmitData;
								memcpy(&cTransmitData,pPerIOData->uIoInfo.cControlIoSendInfo.bWhatToDo,sizeof(TRANSMIT_PACKETS_ELEMENT));

								if(	cTransmitData.dwElFlags==TP_ELEMENT_MEMORY)//发送内存中的数据
								{
									TRANSMIT_FILE_BUFFERS strTransmitData;
									strTransmitData.Head=cTransmitData.pBuffer;
									strTransmitData.HeadLength=cTransmitData.cLength;
									strTransmitData.Tail=NULL;
									strTransmitData.TailLength=0;
									pDataLinkPerSocketData->cPerIoData.IoType=DataIoTransPacket;
									pDataLinkPerSocketData->cPerIoData.uIoInfo.cDataIoTransPacketInfo.hTmpFile=INVALID_HANDLE_VALUE;
									pDataLinkPerSocketData->cPerIoData.uIoInfo.cDataIoTransPacketInfo.uTotolFileSize=cTransmitData.cLength;
									if(pDataLinkPerSocketData->cPerHandleData.uHandleInfo.cDataInfo.bValid)
									{
										TransmitFile(pDataLinkPerSocketData->cPerHandleData.Socket,
													NULL,
													0,
													0,
													reinterpret_cast<LPOVERLAPPED>(&(pDataLinkPerSocketData->cPerIoData.Overlapped)),
													&strTransmitData,
													TF_USE_SYSTEM_THREAD);
										if (WSAGetLastError() != ERROR_IO_PENDING) 
										{ 
											printf("Socket %d TransmitFile failed with error %d\n",pPerHandleData->Socket,WSAGetLastError()); 
											ControlIoReceive(pPerHandleData,pPerIOData);
											break; 
										} 
									}
									else
									{
										printf("Data Link is invalid!\r\n");
										ControlSocketPureSendData((LPPER_SOCKET_DATA)pPerHandleData,"425 Can't open data connection.\r\n");
										ControlIoReceive(pPerHandleData,pPerIOData);
										
										break;
									}
								
								}
								else//通过句柄发送文件中的数据！
								{
									pDataLinkPerSocketData->cPerIoData.uIoInfo.cDataIoTransPacketInfo.hTmpFile=cTransmitData.hFile;
									pDataLinkPerSocketData->cPerIoData.IoType=DataIoTransPacket;
									pDataLinkPerSocketData->cPerIoData.uIoInfo.cDataIoTransPacketInfo.uTotolFileSize=cTransmitData.cLength;
									if(pDataLinkPerSocketData->cPerHandleData.uHandleInfo.cDataInfo.bValid)
									{
										TransmitFile(pDataLinkPerSocketData->cPerHandleData.Socket,
													cTransmitData.hFile,
													cTransmitData.cLength,
													0,
													reinterpret_cast<LPOVERLAPPED>(&(pDataLinkPerSocketData->cPerIoData.Overlapped)),
													NULL,
													TF_USE_SYSTEM_THREAD);
										if (WSAGetLastError() != ERROR_IO_PENDING) 
										{ 
											printf("Socket %d TransmitFile failed with error %d\n",pPerHandleData->Socket,WSAGetLastError()); 
											ControlIoReceive(pPerHandleData,pPerIOData);
											break; 
										} 
									}
									else
									{
										printf("Data Link is invalid!\r\n");
										ControlSocketPureSendData((LPPER_SOCKET_DATA)pPerHandleData,"425 Can't open data connection.\r\n");
										ControlIoReceive(pPerHandleData,pPerIOData);
										break;
									}
								}
								ControlIoReceive(pPerHandleData,pPerIOData);
								break;
							}
						case ToReceiveData:
							{
								TRANSMIT_PACKETS_ELEMENT cTransmitData;
								memcpy(&cTransmitData,pPerIOData->uIoInfo.cControlIoSendInfo.bWhatToDo,sizeof(TRANSMIT_PACKETS_ELEMENT));
								LPPER_SOCKET_DATA pDataLinkPerSocketData=(LPPER_SOCKET_DATA)(pPerHandleData->uHandleInfo.cCtrlInfo.pDataLinkSocketData);
								DWORD dwWaitResult=WaitForSingleObject(pPerHandleData->uHandleInfo.cCtrlInfo.EventDataValid,10000);
								if(dwWaitResult==WAIT_TIMEOUT||pDataLinkPerSocketData==NULL||pDataLinkPerSocketData->cPerHandleData.uHandleInfo.cDataInfo.bValid==false)
								{	
									printf("Data Link is invalid!\r\n");
									ControlSocketPureSendData((LPPER_SOCKET_DATA)pPerHandleData,"425 Can't open data connection.\r\n");
									ControlIoReceive(pPerHandleData,pPerIOData);		
									break;
								}
								memset(&(pDataLinkPerSocketData->cPerIoData),0,sizeof(PER_IO_OPERATION_DATA));
								pDataLinkPerSocketData->cPerIoData.IoType=DataIoUpload;
								pDataLinkPerSocketData->cPerIoData.uIoInfo.cDataIoUploadInfo.hFileHandle=cTransmitData.hFile;
								pDataLinkPerSocketData->cPerIoData.uIoInfo.cDataIoUploadInfo.uReceivedFileSize=0;
								pDataLinkPerSocketData->cPerIoData.cBuffer.buf =pDataLinkPerSocketData->cPerIoData.stBuffer;
								pDataLinkPerSocketData->cPerIoData.cBuffer.len=BUFFERSIZE;
								DWORD RecvBytes=0,Flags=0;
								if (WSARecv(pDataLinkPerSocketData->cPerHandleData.Socket,&(pDataLinkPerSocketData->cPerIoData.cBuffer),1,&RecvBytes,&Flags,&(pDataLinkPerSocketData->cPerIoData.Overlapped),NULL) == SOCKET_ERROR) 
								{ 
									if (WSAGetLastError() != ERROR_IO_PENDING) 
									{ 
										printf("Socket %d WSARecv() failed with error %d\n",pPerHandleData->Socket,WSAGetLastError()); 
										break; 
									} 
								}
								ControlIoReceive(pPerHandleData,pPerIOData);
								break;
							}
						case ToReceive:
							{

								ControlIoReceive(pPerHandleData,pPerIOData);
								break;
							}
						case ToAbort:
							{
								LPPER_SOCKET_DATA pDataSocketData=(LPPER_SOCKET_DATA)(pPerHandleData->uHandleInfo.cCtrlInfo.pDataLinkSocketData);
								ResetEvent(pPerHandleData->uHandleInfo.cCtrlInfo.EventDataValid);
								
								if(pDataSocketData!=NULL)
								{
									closesocket(pDataSocketData->cPerHandleData.Socket) ;
									CloseHandle(pPerHandleData->uHandleInfo.cCtrlInfo.hCurrentFileHandle);
									ControlNormalSendData(pPerHandleData,pPerIOData,ToReceive,"226 ABOR command successful.\r\n");
									pPerHandleData->uHandleInfo.cCtrlInfo.pDataLinkSocketData=NULL;
								}
								else
								{
									ControlNormalSendData(pPerHandleData,pPerIOData,ToReceive,"504 Command not implemented .\r\n");
								}
								break;
							}
						case DoNothing:
							{
								memset(pPerIOData,0,sizeof(PER_IO_OPERATION_DATA));
								break;
							}
						default:
							{
								printf("Error NEXT TO Do VALUE");
								break;
							}
						}

					}
					else
					{//本次发送没能全部完成　需要继续发送
						if(	pPerIOData->uIoInfo.cControlIoSendInfo.dwSendedDataLength<pPerIOData->cBuffer.len)
						{
							pPerIOData->cBuffer.len-=pPerIOData->uIoInfo.cControlIoSendInfo.dwSendedDataLength;
							pPerIOData->cBuffer.buf+=pPerIOData->uIoInfo.cControlIoSendInfo.dwSendedDataLength;
							DWORD SendBytes=0;
							if(WSASend(pPerHandleData->Socket,&(pPerIOData->cBuffer),1,&SendBytes,0,&(pPerIOData->Overlapped),NULL) ==SOCKET_ERROR ) 
							{ 
								if (WSAGetLastError() != ERROR_IO_PENDING) 
								{ 
									printf(" Socket %d WSASend() fialed with error %d\n",pPerHandleData->Socket,WSAGetLastError()); 
									break;
								} 
							} 
						}
						else
						{
							printf("Socket %d Sended Data length ERROR!\r\n",pPerHandleData->Socket); 
							break;
						}
					}
					break;
				}
			case ControlIoRecv:
				{
					pPerIOData->cBuffer.buf[dwBytesTransferred]=0;
					printf("DEBUG :Command from Socket %d:%s \r\n",pPerHandleData->Socket,pPerIOData->cBuffer.buf);
					
					_strlwr(pPerIOData->cBuffer.buf);
					
					if(pPerIOData->cBuffer.buf[0]<='p')
					{
						if(pPerIOData->cBuffer.buf[0]<='l')
						{
							if(strstr(pPerIOData->cBuffer.buf,"abor")==pPerIOData->cBuffer.buf)
							{
								ProcessAborCommand(dwBytesTransferred,pPerHandleData,pPerIOData);
								break;
							}
							if(strstr(pPerIOData->cBuffer.buf,"appe")==pPerIOData->cBuffer.buf)
							{
								ProcessAppeCommand(dwBytesTransferred,pPerHandleData,pPerIOData);
								break;

							}
							if(strstr(pPerIOData->cBuffer.buf,"cwd")==pPerIOData->cBuffer.buf)
							{
								ProcessCwdCommand(dwBytesTransferred,pPerHandleData,pPerIOData);
								break;
							}

							if(strstr(pPerIOData->cBuffer.buf,"dele")==pPerIOData->cBuffer.buf)
							{
								ProcessDeleCommand(dwBytesTransferred,pPerHandleData,pPerIOData);
								break;
							}
							if(strstr(pPerIOData->cBuffer.buf,"list")==pPerIOData->cBuffer.buf)
							{
								ProcessListCommand(dwBytesTransferred,pPerHandleData,pPerIOData);
								break;
							}
						}
						else
						{
							if(strstr(pPerIOData->cBuffer.buf,"mkd")==pPerIOData->cBuffer.buf)
							{
								ProcessMkdCommand(dwBytesTransferred,pPerHandleData,pPerIOData);
								break;
							}
							if(strstr(pPerIOData->cBuffer.buf,"pass")==pPerIOData->cBuffer.buf)//处理Ｐａｓｓ命令
							{
								ProcessPassCommand(dwBytesTransferred,pPerHandleData,pPerIOData);
								break;
							}
							if(strstr(pPerIOData->cBuffer.buf,"pasv")==pPerIOData->cBuffer.buf)
							{
								ProcessPasvCommand(dwBytesTransferred,pPerHandleData,pPerIOData);
								break;
							}

							if(strstr(pPerIOData->cBuffer.buf,"port")==pPerIOData->cBuffer.buf)
							{
								ProcessPortCommand(dwBytesTransferred,pPerHandleData,pPerIOData);
								break;
							}
							if(strstr(pPerIOData->cBuffer.buf,"pwd")==pPerIOData->cBuffer.buf)//处理ｐｗｄ
							{
								ProcessPwdCommand(dwBytesTransferred,pPerHandleData,pPerIOData);
								break;
							}
						}
					}
					else//if(pPerIOData->cBuffer.buf[0]<='p')
					{
						if(pPerIOData->cBuffer.buf[0]<='r')
						{
							if(strstr(pPerIOData->cBuffer.buf,"quit")==pPerIOData->cBuffer.buf)
							{
								ProcessQuitCommand(dwBytesTransferred,pPerHandleData,pPerIOData);
								break;
							}
							if(strstr(pPerIOData->cBuffer.buf,"rmd")==pPerIOData->cBuffer.buf)
							{
								ProcessRmdCommand(dwBytesTransferred,pPerHandleData,pPerIOData);
								break;
							}
							if(strstr(pPerIOData->cBuffer.buf,"rest")==pPerIOData->cBuffer.buf)
							{
								ProcessRestCommand(dwBytesTransferred,pPerHandleData,pPerIOData);
								break;
							}
							if(strstr(pPerIOData->cBuffer.buf,"retr")==pPerIOData->cBuffer.buf)
							{
								ProcessRetrCommand(dwBytesTransferred,pPerHandleData,pPerIOData);
								break;
							}
						}
						else
						{
							if(strstr(pPerIOData->cBuffer.buf,"site exec ")==pPerIOData->cBuffer.buf)
							{
								ProcessSiteExecCommand(dwBytesTransferred,pPerHandleData,pPerIOData);
								break;
							}
							if(strstr(pPerIOData->cBuffer.buf,"size")==pPerIOData->cBuffer.buf)
							{
								ProcessSizeCommand(dwBytesTransferred,pPerHandleData,pPerIOData);
								break;
							}
							if(strstr(pPerIOData->cBuffer.buf,"stor")==pPerIOData->cBuffer.buf)
							{
								ProcessStorCommand(dwBytesTransferred,pPerHandleData,pPerIOData);
								break;

							}
							if(strstr(pPerIOData->cBuffer.buf,"type")==pPerIOData->cBuffer.buf)
							{
								ProcessTypeCommand(dwBytesTransferred,pPerHandleData,pPerIOData);
								break;
							}
							if(strstr(pPerIOData->cBuffer.buf,"user")==pPerIOData->cBuffer.buf)//处理ＵＳＥＲ命令
							{
								ProcessUserCommand(dwBytesTransferred,pPerHandleData,pPerIOData);
								break;
							}

						}
					}	
					//默认处理　－－－无法识别的语句
					char Command[128];
					strcpy(Command,pPerIOData->cBuffer.buf);
					FormatString(Command); 
					ControlNormalSendData(pPerHandleData,pPerIOData,ToReceive,"500 \"%s\" : Syntax error, command unrecognized.\r\n",Command);
					break;
				}
			case IoQuit:
				{
					return 0;
				}
			default :
				{
					printf("error Io Type \r\n");
					break;
				}
			}
		}
		else
		{
			if(pPerHandleData->enCurrentHandleType==HandleData)//数据连接
			{
				switch(pPerIOData->IoType)
				{
				case DataIoTransPacket:
					{
						if(pPerIOData->uIoInfo.cDataIoTransPacketInfo.hTmpFile!=INVALID_HANDLE_VALUE)	
						{
							CloseHandle(pPerIOData->uIoInfo.cDataIoTransPacketInfo.hTmpFile);
						}
						LPPER_SOCKET_DATA pControlSocketData=(LPPER_SOCKET_DATA)(pPerHandleData->uHandleInfo.cDataInfo.pControlLinkSocketData);
						ControlSocketPureSendData(pControlSocketData,"226 Transfer complete .\r\n");
						closesocket(pPerHandleData->Socket);
						break;
					}
				case DataIoUpload:
					{
						if(pPerHandleData->uHandleInfo.cDataInfo.bAbor==true)
						{
							break;
						}
						if(dwBytesTransferred==0)
						{
							if(pPerIOData->uIoInfo.cDataIoUploadInfo.hFileHandle!=INVALID_HANDLE_VALUE)	
							{
								CloseHandle(pPerIOData->uIoInfo.cDataIoUploadInfo.hFileHandle);
							}
							LPPER_SOCKET_DATA pControlSocketData=(LPPER_SOCKET_DATA)(pPerHandleData->uHandleInfo.cDataInfo.pControlLinkSocketData);
							ControlSocketPureSendData(pControlSocketData,"226 Transfer Complete.\r\n");
							closesocket(pPerHandleData->Socket);

							break;
						}
						else
						{
							DWORD dwByteWritten=0;
							HANDLE hFile=pPerIOData->uIoInfo.cDataIoUploadInfo.hFileHandle;
							DWORD dwByteTotolWritten=pPerIOData->uIoInfo.cDataIoUploadInfo.uReceivedFileSize;
							if(WriteFile(hFile,pPerIOData->cBuffer.buf,dwBytesTransferred,&dwByteWritten,NULL)==0)
							{
								printf("Write File Error %d\r\n",GetLastError());
							}
							memset(pPerIOData,0,sizeof(PER_IO_OPERATION_DATA));
							pPerIOData->cBuffer.buf=pPerIOData->stBuffer;
							pPerIOData->cBuffer.len=BUFFERSIZE;
							pPerIOData->IoType=DataIoUpload;
							pPerIOData->uIoInfo.cDataIoUploadInfo.hFileHandle=hFile;
							pPerIOData->uIoInfo.cDataIoUploadInfo.uReceivedFileSize=dwByteTotolWritten+dwBytesTransferred;
							DWORD dwNumberOfDataRecv=0,Flags=0;;
							if(WSARecv(pPerHandleData->Socket,&(pPerIOData->cBuffer),1,&(dwNumberOfDataRecv),&Flags,&(pPerIOData->Overlapped),NULL) ==SOCKET_ERROR ) 
							{ 
								if (WSAGetLastError() != ERROR_IO_PENDING) 
								{ 
									printf("WSASend() fialed with error %d\n",WSAGetLastError()); 
									break; 
								} 
							} 
						}
						break;
					}
				default:
					{
						printf("error Io Type \r\n");
						break;
					}
				}
			}
			else
			{
				printf("enCurrentHandleType Error!\r\n");
			}

		}
	}
}