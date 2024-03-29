#include <iostream>
#include <vector>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include<fstream>
#include<sys/types.h>
using namespace std;

#pragma comment( lib, "ws2_32.lib" )

// ポート番号
const unsigned short SERVERPORT = 8888;

// 送受信するメッセージの最大値
const unsigned int MESSAGELENGTH = 1024;

// リスンソケット ... クラス化してメンバー変数にしたいなー
int listenSock;
int fd;
const int text = 1;
const int file = 2;

// クライアントとの通信用ソケットvector ... クラス化してメンバー変数にしたいなー
std::vector<int> socks;	// 靴下っぽい
//std::string recevedFile;

bool Init();
bool Socket(int sockType);
bool Bind(unsigned short port);
bool Listen(int backlog);
int Accept();
int Recv(int sock, char* buff);
int Recvfile(int sock, char* buff);
bool Exit();

int main()
{
    // 初期化
    if (!Init())
    {
        std::cout << "Error: Init()" << std::endl;
    }
    std::cout << "Success: Init()" << std::endl;

    // リスンソケットの作成 ( ノンブロッキング )
    if (!Socket(SOCK_STREAM))
    {
        std::cout << "Error:Socket()" << std::endl;
    }
    std::cout << "Success: Socket()" << std::endl;

    // バインド
    if (!Bind(SERVERPORT))
    {
        std::cout << "Error:Bind()" << std::endl;
    }
    std::cout << "Success: Bind()" << std::endl;

    if (!Listen(1))
    {
        std::cout << "Error: Listen()" << std::endl;
    }
    std::cout << "Success: Listen()" << std::endl;

    
    int len = 0;
    std::cout << "---Text chat or receive files---" << std::endl;
    std::cout << "Text : 1, File : 2" << std::endl;
    std::cin >> len;
    while (true)
    {
        int ret = Accept();
        if (ret == 0)
        {
            std::cout << "--- accept new client ---" << std::endl;
        }
        else if (ret != WSAEWOULDBLOCK)
        {
            std::cout << "Error: Accept()" << std::endl;
        }
        
        if (len == text)
        {
            // recv
            for (auto sock : socks)
            {
                char buff[MESSAGELENGTH];
                ret = Recv(sock, buff);
                //memset(buff, 0, sizeof(buff));
                /*while (true)//すべてのデータを受信するまでrecvし続ける
                {

                    //buff[ret] = '\0';
                    len += ret;
                }*/


                if (ret == 0)
                {
                    std::cout << "buff = " << buff << std::endl;
                }
                else if (ret != WSAEWOULDBLOCK)
                {
                    std::cout << "Error: Recv()" << std::endl;
                }              
            }
        }
        else if (len == file)
        {
            // recvfile
            for (auto sock : socks)
            {
                char buff[MESSAGELENGTH];
                
                //memset(buff, 0, sizeof(buff));
                while (true)//すべてのデータを受信するまでrecvし続ける
                {
                    ret = Recv(sock, buff);
                    //buff[ret] = '\0';
                    len += ret;
                }


                if (ret == 0)
                {
                    ifstream ifs(buff);
                    if (!ifs.bad())
                    {
                        std::cout << "buff = " << ifs.rdbuf() << std::endl;//テキストファイルの中身を書き出す
                        ifs.close();
                    }

                }
                else if (ret != WSAEWOULDBLOCK)
                {
                    std::cout << "Error: Recv()" << std::endl;
                }
                //buff[ret] = '\0';

            }
        }
        else 
        {
            std::cout << "Error: Untargeted input" << std::endl;
            Exit();
            std::cout << "Success: Exit()" << std::endl;
            return 0;
        }
        
    }

    Exit();
    std::cout << "Success: Exit()" << std::endl;
    return 0;
}

// 初期化
bool Init()
{
    WSADATA wsaData;
    return (WSAStartup(MAKEWORD(2, 2), &wsaData) != SOCKET_ERROR);
}

// ソケット作成
bool Socket(int sockType)
{
    int tempSock = socket(AF_INET, sockType, 0);
    if (tempSock < 0)
    {
        return false;
    }

    unsigned long cmdArg = 0x01;
    if (ioctlsocket(tempSock, FIONBIO, &cmdArg) == 0)
    {
        listenSock = tempSock;
        return true;
    }
    return false;
}

// 接続要求
bool Bind(unsigned short port)
{
    // 接続先サーバのソケットアドレス情報格納
    struct sockaddr_in bindAddr;	// bind用のソケットアドレス情報
    memset(&bindAddr, 0, sizeof(bindAddr));
    bindAddr.sin_family = AF_INET;
    bindAddr.sin_port = htons(port);
    bindAddr.sin_addr.s_addr = htonl(INADDR_ANY);

    return (bind(listenSock, (struct sockaddr*)&bindAddr, sizeof(bindAddr)) == 0);
}

// リスン状態に
bool Listen(int backlog)
{
    return (listen(listenSock, 1) == 0);
}

// 接続要求受付
int Accept()
{
    struct sockaddr_in clientAddr;
    int addrlen = sizeof(clientAddr);
    int tempSock = accept(listenSock, (struct sockaddr*)&clientAddr, &addrlen);

    if (tempSock > 0)
    {
        unsigned long cmdArg = 0x01;
        if (ioctlsocket(tempSock, FIONBIO, &cmdArg) == 0)
        {
            socks.push_back(tempSock);
            return 0;
        }
    }
    return WSAGetLastError();
}

// 文字列送信
int Recv(int sock, char* buff)
{
    int ret = recv(sock, buff, MESSAGELENGTH, 0);//文字受け取るだけ
    
    if (ret == MESSAGELENGTH)
        return 0;

    return WSAGetLastError();
}

int Recvfile(int sock, char* buff)
{
    std::ofstream receivedfile(buff, std::ios::binary);
    int ret = recv(sock, buff, sizeof(buff) - 1, 0);//テキストファイル
    while (ret > 0)
    {
        receivedfile.write(buff, ret);
    }
    return WSAGetLastError();
}

// 終了処理
bool Exit()
{
    for (auto sock : socks)
    {
        shutdown(sock, 0x02);
        closesocket(sock);
    }
    socks.clear();
    closesocket(listenSock);
    return (WSACleanup() != SOCKET_ERROR);
}