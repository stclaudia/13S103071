
#if !defined(AFX_MYSUPERSERVER_H__EB881E00_CF44_4469_90C4_19510D07E3C7__INCLUDED_)
#define AFX_MYSUPERSERVER_H__EB881E00_CF44_4469_90C4_19510D07E3C7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "resource.h"

#include "HttpProtocol.h"


CHttpProtocol *pHttpProtocol;

UINT HttpServerThread(LPVOID param);
UINT FtpServerThread1(LPVOID param);
UINT FtpServerThread2(LPVOID param);
CWinThread* m_pServerThread[3];
#endif // !defined(AFX_MYSUPERSERVER_H__EB881E00_CF44_4469_90C4_19510D07E3C7__INCLUDED_)
