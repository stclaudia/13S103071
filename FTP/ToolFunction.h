#ifndef TOOLFUNCTION
#define TOOLFUNCTION
DWORD GetFileList(LPFILE_INFO lpFI,DWORD dwFIsize,char* strPath);
DWORD StorFileList(char* stStorBuff,DWORD dwMaxSize,char* strPath,bool bDetail);
bool SplitPortRequest(char* strRequest,DWORD& dwIpAddr,WORD& wPort);
void ControlIoReceive(LPPER_HANDLE_DATA pPerHandleData,LPPER_IO_OPERATION_DATA pPerIOData);
void FormatString(char *strStrToFormat);
DWORD WINAPI PasvAcceptFunc(LPVOID Parament);
SOCKET WINAPI PortConnectFunc(DWORD DataIpAddr,WORD DataPort);
bool DeleteVectorItem(LPPER_HANDLE_DATA pPerHandleData);
bool CloseDataSocket(LPPER_HANDLE_DATA pPerHandleData);
bool ControlSocketPureSendData(LPPER_SOCKET_DATA pPerSocketData,char *pData);
bool ControlNormalSendData(LPPER_HANDLE_DATA pPerHandleData,LPPER_IO_OPERATION_DATA pPerIOData,NEXT_TO_DO mWhatToDo,char *DataSend,...); 
#endif