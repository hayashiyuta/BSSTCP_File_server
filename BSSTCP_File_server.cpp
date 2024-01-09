#include <iostream>
#include <vector>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include<fstream>

#pragma comment( lib, "ws2_32.lib" )

// �|�[�g�ԍ�
const unsigned short SERVERPORT = 8888;

// ����M���郁�b�Z�[�W�̍ő�l
const unsigned int MESSAGELENGTH = 1024;

// ���X���\�P�b�g ... �N���X�����ă����o�[�ϐ��ɂ������ȁ[
int listenSock;

// �N���C�A���g�Ƃ̒ʐM�p�\�P�b�gvector ... �N���X�����ă����o�[�ϐ��ɂ������ȁ[
std::vector<int> socks;	// �C�����ۂ�

bool Init();
bool Socket(int sockType);
bool Bind(unsigned short port);
bool Listen(int backlog);
int Accept();
int Recv(int sock, char* buff);
bool Exit();

int main()
{
    // ������
    if (!Init())
    {
        std::cout << "Error: Init()" << std::endl;
    }
    std::cout << "Success: Init()" << std::endl;

    // ���X���\�P�b�g�̍쐬 ( �m���u���b�L���O )
    if (!Socket(SOCK_STREAM))
    {
        std::cout << "Error:Socket()" << std::endl;
    }
    std::cout << "Success: Socket()" << std::endl;

    // �o�C���h
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

        // recv
        for (auto sock : socks)
        {
            char buff[MESSAGELENGTH];
            ret = Recv(sock, buff);
            if (ret == 0)
            {
                std::cout << "Recv message:" << buff << std::endl;
            }
            else if (ret != WSAEWOULDBLOCK)
            {
                std::cout << "Error: Recv()" << std::endl;
            }


        }
    }

    Exit();
    std::cout << "Success: Exit()" << std::endl;

    return 0;
}

// ������
bool Init()
{
    WSADATA wsaData;
    return (WSAStartup(MAKEWORD(2, 2), &wsaData) != SOCKET_ERROR);
}

// �\�P�b�g�쐬
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

// �ڑ��v��
bool Bind(unsigned short port)
{
    // �ڑ���T�[�o�̃\�P�b�g�A�h���X���i�[
    struct sockaddr_in bindAddr;	// bind�p�̃\�P�b�g�A�h���X���
    memset(&bindAddr, 0, sizeof(bindAddr));
    bindAddr.sin_family = AF_INET;
    bindAddr.sin_port = htons(port);
    bindAddr.sin_addr.s_addr = htonl(INADDR_ANY);

    return (bind(listenSock, (struct sockaddr*)&bindAddr, sizeof(bindAddr)) == 0);
}

// ���X����Ԃ�
bool Listen(int backlog)
{
    return (listen(listenSock, 1) == 0);
}

// �ڑ��v����t
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

// �����񑗐M
int Recv(int sock, char* buff)
{
    std::ofstream receivedImage(buff, std::ios::binary);
    int byteread;
    int ret = recv(sock, buff, MESSAGELENGTH, 0);

    while ((byteread = ret) > 0)
    {
        receivedImage.write(buff, byteread);
    }


    return WSAGetLastError();
}

// �I������
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