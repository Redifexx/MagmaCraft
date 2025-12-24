#include "Client.h"

using namespace Magma;

Client::Client()
{
	//WSADATA data;
	//WORD version = MAKEWORD(2, 2);
	//int ws0k = WSAStartup(version, &data);
	//if (ws0k != 0)
	//{
	//	std::cout << "Can't start winsock! " << ws0k << std::endl;
	//	return;
	//}

	m_Bound = false;
}

Client::~Client()
{
	// close that socket
	//closesocket(m_Out);
	//WSACleanup();
}

void Client::Bind()
{
	// create hint structure for server
	//m_ServerHint.sin_family = AF_INET;
	//m_ServerHint.sin_port = htons(54000);
	//inet_pton(AF_INET, "127.0.0.1", &m_ServerHint.sin_addr);
	//
	//// socket creation
	//m_Out = socket(AF_INET, SOCK_DGRAM, 0);
	//
	m_Bound = true;
}

bool Client::SendServerMessage(std::string msg)
{
	// write out to that socket
	//int sendOk = sendto(m_Out, msg.c_str(), msg.size() + 1, 0, (sockaddr*)&m_ServerHint, sizeof(m_ServerHint));
	//
	//if (sendOk == SOCKET_ERROR)
	//{
	//	std::cout << "Client: Socket Didn't work." << WSAGetLastError() << std::endl;
	//	return false;
	//}

	return true;
}