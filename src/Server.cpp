#include "Server.h"

using namespace Magma;

Server::Server()
{
	/*
	WSADATA data;
	WORD version = MAKEWORD(2, 2);
	int wsOk = WSAStartup(version, &data);
	if (wsOk != 0)
	{
		std::cout << "Can't start winsock! " << wsOk << std::endl;
		return;
	}
	*/
	m_Bound = false;
}

Server::~Server()
{
	//closesocket(m_In);
	//WSACleanup();
}

void Server::Bind()
{
	//m_In = socket(AF_INET, SOCK_DGRAM, 0);
	//m_ServerHint.sin_addr.S_un.S_addr = ADDR_ANY;
	//m_ServerHint.sin_family = AF_INET;
	//m_ServerHint.sin_port = htons(54000);

	//if (bind(m_In, (sockaddr*)&m_ServerHint, sizeof(m_ServerHint)) == SOCKET_ERROR)
	//{
	//	std::cout << "Can't bind socket! " << WSAGetLastError() << std::endl;
	//	return;
	//}

	m_Bound = true;
}

void Server::Update()
{
	//sockaddr_in client;
	//int clientLength = sizeof(client);
	//ZeroMemory(&client, clientLength);
	//
	//char buf[1024];
	//
	//while (true)
	//{
	//	ZeroMemory(buf, 1024);
	//
	//	// wait for message
	//	int bytesIn = recvfrom(m_In, buf, 1024, 0, (sockaddr*)&client, &clientLength);
	//	if (bytesIn == SOCKET_ERROR)
	//	{
	//		std::cout << "Error receiving from client " << WSAGetLastError() << std::endl;
	//		return;
	//	}
	//
	//	// display message and client info
	//	char clientIp[256];
	//	ZeroMemory(clientIp, 256);
	//
	//	inet_ntop(AF_INET, &client.sin_addr, clientIp, 256);
	//
	//	std::cout << "Message Received from  " << clientIp << " : " << buf << std::endl;
	//}
}
