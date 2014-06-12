/****************server.c****************/

#include <stdio.h>
#include <winsock.h>
#include <stdlib.h>

#pragma comment(lib,"ws2_32.lib")

WSADATA wsd;
char sbuffer[80],rbuffer[80]; //send and receive buffers
int n,bytes;                  //counters
SOCKET newsocket,ns_data;
struct sockaddr_in remoteaddr; //remoteaddr_data;
int port_connect=0;            //port connect flag
char path[80]="";
char order[100]="";

//SOCKET s_data_port;

int sy_error=1; // use for indicate Syntax error

//server functions

int sdirfun(SOCKET newsocket);
int sgetfun(SOCKET newsocket);
void HandleError(char *func);

//server functions end

//MAIN

int main(int argc, char *argv[])
{
    struct sockaddr_in localaddr;//local address structure
    SOCKET s;//s_data;//welcome socket and welcome socket for data connection,and port connection for connect to client
    int addr_inlen;//address lenght variable

    if(WSAStartup(MAKEWORD(2,2), &wsd) != 0)
    {
        WSACleanup();
        printf("WSAStartup failed\n");
    }
    memset(&localaddr,0,sizeof(localaddr));//clear localaddr
    s = socket(PF_INET, SOCK_STREAM, 0);

    if (s <0)
    {
        printf("socket failed\n");
    }

    localaddr.sin_family = AF_INET;
    if(argc == 2)
        localaddr.sin_port = htons((u_short)atoi(argv[1]));
    else
        localaddr.sin_port = htons(2302);
    localaddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(s,(struct sockaddr *)(&localaddr),sizeof(localaddr)) < 0)
    {
        printf("Bind failed!\n");
    }
    //INFINITE LOOP

    while (1) // while loop 1
    {
        //LISTEN
        listen(s,3);
        addr_inlen = sizeof(remoteaddr);

        //ACCEPT main connection (control connection)
        newsocket = accept(s,(struct sockaddr *)(&remoteaddr),&addr_inlen);
        if (newsocket == INVALID_SOCKET) break;
            printf("connected to      %s at port %d \n",inet_ntoa(remoteaddr.sin_addr),ntohs(localaddr.sin_port));

        //Respond with welcome message, FTP client requires those
        sprintf(sbuffer,"200 Welcome \r\n");
        bytes = send(newsocket, sbuffer, strlen(sbuffer), 0);
        sprintf(sbuffer,"530 Log in \r\n");
        bytes = send(newsocket, sbuffer, strlen(sbuffer), 0);

        //INFINITE LOOP
        while (1) // while loop 2
        {
            n = 0;
            sy_error=1;
            while (1) // while loop 3
            {
                //RECEIVE
                bytes = recv(newsocket, &rbuffer[n], 1, 0);
                printf("rbuffer[%d]=%c\n",n,rbuffer[n]);
                if ((bytes < 0) || (bytes == 0))
                    break;
                if (rbuffer[n] == '$')
                {
                    rbuffer[n] = '\0';
                    break;
                }
                if (rbuffer[n] != '\r')
                    n++;
            } // end of while loop 3

            if ((bytes < 0) || (bytes == 0))
                break;
            printf("#The Server receives:# '%s' from client \n", rbuffer);

            //THE FTP COMMANDS HERE
            //LIST
            if(strncmp(rbuffer,"dir",3)==0)
            {
                sdirfun(newsocket);
            }


            //GET
            if (strncmp(rbuffer,"get",3)==0)
            {
                sgetfun(newsocket);
            }



            //Syntax error
            if (sy_error==1)
            {
                printf("command unrecognized, non-implemented!\n");
                sprintf(sbuffer, "500 Syntax error. \n");
                bytes = send(newsocket, sbuffer, strlen(sbuffer), 0);
            }
        } // end of while loop 2

        //CLOSE CONTROL SOCKET
        closesocket(newsocket);
        printf("disconnected from %s at port %d, close control socket.\n",inet_ntoa(remoteaddr.sin_addr),ntohs(localaddr.sin_port));
    } // end of while loop 1

    //CLOSE WELCOME SOCKET
    closesocket(s);
    printf("Welcome sockets close");
    return 0;
}


int sdirfun(SOCKET newsocket)
{
    char temp_buffer[80];
    FILE *fin;

    printf("Equivalent to dir \n");
    order[0]='\0';
    strcat(order,"dir ");
    strcat(order,path);
    strcat(order," >tmp.txt");
    system(order);

    fin=fopen("tmp.txt","r");
    sprintf(sbuffer, "125 Transfering... \r\n");
    bytes = send(newsocket, sbuffer, strlen(sbuffer), 0);
    while (fgets(temp_buffer,80,fin)!=NULL)
    {
        sprintf(sbuffer,"%s",temp_buffer);
        if(port_connect==0)
            send(newsocket, sbuffer, strlen(sbuffer), 0);
    }

    fclose(fin);
    sprintf(sbuffer, "226 Transfer completed... \r\n");
    bytes = send(newsocket, sbuffer, strlen(sbuffer), 0);
    system("del tmp.txt");

    //CLOSE the ns_data SOCKET or data port SOCKET
    if(port_connect==0)
    {
        closesocket(ns_data);
        sprintf(sbuffer,"226 Close the data socket... \r\n");
        bytes = send(newsocket, sbuffer, strlen(sbuffer), 0);
        ns_data = socket(AF_INET, SOCK_STREAM, 0);
    }

    sy_error=0;
    return 0;
}


int sgetfun(SOCKET newsocket)
{
    int i=4,k=0;
    char filename[20],temp_buffer[80];
    char *p_filename=order;
    FILE *fp;

    printf("RETR mode.\r\n");
    // identify the filename from rbuffer after the word "RETR "
    while (1) // while loop 4
    {
        //RECEIVE
        bytes = recv(newsocket, &rbuffer[i], 1, 0);
        printf("rbuffer[i]=%c\n",rbuffer[i]);

        if ((bytes < 0) || (bytes == 0))
            break;

        filename[k]=rbuffer[i];
        if (rbuffer[i] == '\0')
        {   /*end on LF*/
            filename[k] = '\0';
            break;
        }

        if (rbuffer[i] != '\r')
        {
            i++;
            k++;/*ignore CR's*/
        }
    } // end of while loop 4

    order[0]='\0';
    strcat(order,path);
    if(strlen(path)>0)
        strcat(order,"\\");
    strcat(order,filename);


    if( (fp=fopen(p_filename,"r")) == NULL )
    {
        sprintf(sbuffer, "Sorry, cannot open %s. Please try again.\r\n",filename);
        bytes = send(newsocket, sbuffer, strlen(sbuffer), 0);
        sprintf(sbuffer, "226 Transfer completed... \r");
        bytes = send(newsocket, sbuffer, strlen(sbuffer), 0);
        return 1;
    }
    else
    {
        printf("The file %s found,ready to transfer.\n",filename);
        sprintf(sbuffer, "125 Transfering... \r\n");
        bytes = send(newsocket, sbuffer, strlen(sbuffer), 0);
        while (fgets(temp_buffer,80,fp)!=NULL)
        {
            sprintf(sbuffer,"%s",temp_buffer); //
            if(port_connect==0)
                send(newsocket, sbuffer, strlen(sbuffer), 0);
        }//end of while

        fclose(fp);

    }

    sy_error=0;
    return 0;
}





void HandleError(char *func)
{
    char info[65]= {0};
    _snprintf(info, 64, "%s: %d\n", func, WSAGetLastError());
    printf(info);
}
