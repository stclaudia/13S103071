#ifndef PROCESSCOMMAND
#define PROCESSCOMMAND
#include "DataStructAndConstant.h"
bool ProcessUserCommand(DWORD dwBytesTransferred,LPPER_HANDLE_DATA pPerHandleData,LPPER_IO_OPERATION_DATA pPerIOData); 
bool ProcessPassCommand(DWORD dwBytesTransferred,LPPER_HANDLE_DATA pPerHandleData,LPPER_IO_OPERATION_DATA pPerIOData); 
bool ProcessPasvCommand(DWORD dwBytesTransferred,LPPER_HANDLE_DATA pPerHandleData,LPPER_IO_OPERATION_DATA pPerIOData); 
bool ProcessPortCommand(DWORD dwBytesTransferred,LPPER_HANDLE_DATA pPerHandleData,LPPER_IO_OPERATION_DATA pPerIOData); 
bool ProcessAborCommand(DWORD dwBytesTransferred,LPPER_HANDLE_DATA pPerHandleData,LPPER_IO_OPERATION_DATA pPerIOData); 
bool ProcessQuitCommand(DWORD dwBytesTransferred,LPPER_HANDLE_DATA pPerHandleData,LPPER_IO_OPERATION_DATA pPerIOData); 
bool ProcessSiteExecCommand(DWORD dwBytesTransferred,LPPER_HANDLE_DATA pPerHandleData,LPPER_IO_OPERATION_DATA pPerIOData); 
bool ProcessRestCommand(DWORD dwBytesTransferred,LPPER_HANDLE_DATA pPerHandleData,LPPER_IO_OPERATION_DATA pPerIOData); 
bool ProcessPwdCommand(DWORD dwBytesTransferred,LPPER_HANDLE_DATA pPerHandleData,LPPER_IO_OPERATION_DATA pPerIOData); 
bool ProcessListCommand(DWORD dwBytesTransferred,LPPER_HANDLE_DATA pPerHandleData,LPPER_IO_OPERATION_DATA pPerIOData); 
bool ProcessTypeCommand(DWORD dwBytesTransferred,LPPER_HANDLE_DATA pPerHandleData,LPPER_IO_OPERATION_DATA pPerIOData); 
bool ProcessCwdCommand(DWORD dwBytesTransferred,LPPER_HANDLE_DATA pPerHandleData,LPPER_IO_OPERATION_DATA pPerIOData); 
bool ProcessSizeCommand(DWORD dwBytesTransferred,LPPER_HANDLE_DATA pPerHandleData,LPPER_IO_OPERATION_DATA pPerIOData); 
bool ProcessRetrCommand(DWORD dwBytesTransferred,LPPER_HANDLE_DATA pPerHandleData,LPPER_IO_OPERATION_DATA pPerIOData); 
bool ProcessStorCommand(DWORD dwBytesTransferred,LPPER_HANDLE_DATA pPerHandleData,LPPER_IO_OPERATION_DATA pPerIOData); 
bool ProcessAppeCommand(DWORD dwBytesTransferred,LPPER_HANDLE_DATA pPerHandleData,LPPER_IO_OPERATION_DATA pPerIOData); 
bool ProcessMkdCommand(DWORD dwBytesTransferred,LPPER_HANDLE_DATA pPerHandleData,LPPER_IO_OPERATION_DATA pPerIOData); 
bool ProcessRmdCommand(DWORD dwBytesTransferred,LPPER_HANDLE_DATA pPerHandleData,LPPER_IO_OPERATION_DATA pPerIOData); 
bool ProcessDeleCommand(DWORD dwBytesTransferred,LPPER_HANDLE_DATA pPerHandleData,LPPER_IO_OPERATION_DATA pPerIOData); 
#endif









typedef struct _TRANSMIT_PACKETS_ELEMENT {
  ULONG dwElFlags;
  ULONG cLength;
  union {
    struct {
      LARGE_INTEGER nFileOffset;
      HANDLE        hFile;
    };
    PVOID  pBuffer;
  };
} TRANSMIT_PACKETS_ELEMENT;


#define TP_ELEMENT_MEMORY 0
#define TP_ELEMENT_FILE 1