/******************************************************************************************
* 文件传输协议(ftp)的简单设计与实现(c语言,VC6.0)
* 使用Socket进行Client/Server程序设计的一般连接过程是这样的：Server端Listen(监听)某个端口是否有
* 连接请求，Client端向Server端发出Connect(连接)请求，Server端向Client端发回Accept（接受）消息。
* 一个连接就建立起来了。Server端和Client端都可以通过Send，Write等方法与对方通信。
*　　对于一个功能齐全的Socket，都要包含以下基本结构，其工作过程包含以下四个基本的步骤：
*　　（1） 创建Socket；
*　　（2） 打开连接到Socket的输入/出流；
*　　（3） 按照一定的协议对Socket进行读/写操作；
*　　（4） 关闭Socket.
*　　第三步是程序员用来调用Socket和实现程序功能的关键步骤，其他三步在各种程序中基本相同。
***********************************client.c***********************************************/

#include <winsock2.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>

#pragma comment(lib,"ws2_32.lib")
#define DEFAULT_PORT        2302
#define DEFAULT_BUFFER      2048
#define DEFAULT_MESSAGE     "This is a test of the emergency \ broadcasting system"

char szServerip[128],            // Server to connect to
      szMessage[1024];           // Message to send to sever
short   iPort     = DEFAULT_PORT;  // Port on server to connect to
//DWORD dwCount   = DEFAULT_COUNT; // Number of times to send message
BOOL bSendOnly = FALSE;          // Send data only; don't receive

using namespace std;


int dirfun(SOCKET sClient)
{
    int ret;
    char *MSG="dir$";char szBuffer[80];
    strcpy(szMessage, MSG);

    ret = send(sClient, szMessage, strlen(szMessage), 0);
    if (ret == 0)
        return 1;
    else if (ret == SOCKET_ERROR)
    {
        printf("send() failed: %d\n", WSAGetLastError());
        return 1;
    }


    while(!bSendOnly)
    {
        //读取流并显示
        //ret = recv(sClient, szBuffer, 80, 0);
        //printf("%s",szBuffer);
        ret = recv(sClient, szBuffer, 80, 0);
        cout<<endl<<"-----------"<<ret<<"-------------"<<endl;
        if (ret == 0)        // Graceful close
            return 1;
        else if (ret == SOCKET_ERROR)
        {
            printf("recv() failed: %d\n", WSAGetLastError());
            return 1;
        }
        szBuffer[ret] = '\0';


        if(strncmp(szBuffer,"226 Close",strlen("226 Close"))==0)
        {
            cout<<endl<<"-----------end-------------"<<endl;
             break;
        }
        printf("%s",szBuffer);
        if(strncmp(szBuffer,"500 Syntax error",strlen("500 Syntax error"))==0)
        {
             break;
        }
    }
    return 0;
}
int getfun(SOCKET sClient,char filename[40])
{
    int ret;
    FILE *fpre;
    char szBuffer[80];
    szMessage[0]='\0';
    strcat(szMessage, "get$");
    //strcat(szMessage, "\\");
    strcat(szMessage,filename);

    ret = send(sClient, szMessage, strlen(szMessage)+1, 0);
    if (ret == 0)
        return 1;
    else if (ret == SOCKET_ERROR)
    {
        printf("send() failed: %d\n", WSAGetLastError());
        return 1;
    }
    printf("Send %d bytes\n", ret);
    ret = recv(sClient, szBuffer, 80, 0);

    szBuffer[ret] = '\0';
    printf("%s\n",szBuffer);
    if(strncmp( szBuffer,"125 Transfering...",strlen("125 Transfering...") )==0)
    {
        if( (fpre=fopen(filename,"w")) == NULL )
        {
            printf("open errer");
            return 1;
        }
        printf("bSendOnly=%d\n",bSendOnly);
        while(!bSendOnly)
        {
            //读取流并显示
            ret = recv(sClient, szBuffer, 80, 0);
            cout<<"conning  "<<ret<<endl;
            szBuffer[ret] = '\0';
            fprintf(fpre,"%s",szBuffer);
             if (ret <80)
             {
                 break;
                 }     // Graceful close

             if (ret == SOCKET_ERROR)
            {
                printf("recv() failed: %d\n", WSAGetLastError());
                return 1;
            }

        }
        cout<<"Transfer completed... "<<endl;
        fclose(fpre);
    }
    return 0;
}











int main(int argc, char **argv)
{
    WSADATA       wsd;
    SOCKET        sClient;
    char          szBuffer[DEFAULT_BUFFER];
    int           ret;
    //unsigned int i;
    //int j;
    struct sockaddr_in server;
    struct hostent    *host = NULL;
     char choice[5],choice2[40];
    // Parse the command line and load Winsock
     argv[1]="-s:127.0.0.1";
     strcpy(szServerip, &argv[1][3]);
    //ValidateArgs(argc, argv);
    if (WSAStartup(MAKEWORD(2,2), &wsd) != 0)
    {
        printf("Failed to load Winsock library!\n");
        return 1;
    }
    //strcpy(szMessage, DEFAULT_MESSAGE);
    //
    // Create the socket, and attempt to connect to the server
    //
    sClient = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sClient == INVALID_SOCKET)
    {
        printf("socket() failed: %d\n", WSAGetLastError());
        return 1;
    }
    server.sin_family = AF_INET;
    server.sin_port = htons(iPort);
     printf("server.sin_port=%u\n",server.sin_port);
    server.sin_addr.s_addr = inet_addr(szServerip);
   //
    // If the supplied server address wasn't in the form
    // "aaa.bbb.ccc.ddd" it's a hostname, so try to resolve it
    //
    if (server.sin_addr.s_addr == INADDR_NONE)
    {
        host = gethostbyname(szServerip);
        if (host == NULL)
        {
            printf("Unable to resolve server: %s\n", szServerip);
            return 1;
        }
        CopyMemory(&server.sin_addr, host->h_addr_list[0],
            host->h_length);
    }
    if (connect(sClient, (struct sockaddr *)&server,
        sizeof(server)) == SOCKET_ERROR)
    {
        printf("connect() failed: %d\n", WSAGetLastError());
        return 1;
    }
    //显示接通信息
    //for(j=0;j<2;j++)
    {
        ret = recv(sClient, szBuffer, DEFAULT_BUFFER, 0);
        if (ret == 0)        // Graceful close
        return 0;
        else if (ret == SOCKET_ERROR)
        {
        printf("recv() failed: %d\n", WSAGetLastError());
        return 0;
        }
        szBuffer[ret] = '\0';
        printf("%s\n",szBuffer);
        if(ret<15)
        {
           ret = recv(sClient, szBuffer, DEFAULT_BUFFER, 0);
           if (ret == 0)        // Graceful close
               return 0;
           else if (ret == SOCKET_ERROR)
           {
               //printf("recv() failed: %d\n", WSAGetLastError());
               return 0;
           }
           szBuffer[ret] = '\0';
           printf("%s\n",szBuffer);
        }
        //printf("DEFAULT_BUFFER=%d\n",DEFAULT_BUFFER);
    }
    while(1)
    {
     cout<<"------------------------------------------"<<endl;
     cout<<"dir：列出客户端文件目录"<<endl;
     cout<<"get x.txt：下载名为x.txt的文件"<<endl;
     cout<<"quit：退出并关闭程序"<<endl;
     cout<<"请输入命令：";

        scanf("%s", choice);


        if(strncmp(choice,"dir",3)==0||strncmp(choice,"DIR",2)==0)
        {
          dirfun(sClient);
          continue;
        }

        else if(strncmp(choice,"quit",4)==0||strncmp(choice,"QUIT",2)==0)
        {
           break;
        }


        scanf("%s", choice2);
        if(strncmp(choice,"get",3)==0||strncmp(choice,"GET",3)==0)
        {
           getfun(sClient,choice2);
           continue;
        }
    }

    closesocket(sClient);

    WSACleanup();
    return 0;
}
