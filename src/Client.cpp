#include "Client.h"

using namespace Magma;

Client::Client()
{
	m_Client = nullptr;
	m_Server = nullptr;

	m_Client = enet_host_create(NULL /* CREATE A CLIENT HOST */,
		1 /* only allow outgoing connection */,
		2 /* allow up to 2 channels to be used, 0 & 1 */,
		0 /* assume any amount of incoming bandwidth */,
		0 /* assume any amount of outgoing bandwidth */
	);

	if (m_Client == NULL)
	{
		std::cout << "An error occurred while trying to create an ENetclient host.\n";
		return;
	}
}

Client::~Client()
{
	enet_host_destroy(m_Client);
	delete m_Client;
	delete m_Server;
}

void Client::SetServerHint(const char* hostName, enet_uint16 port)
{
	enet_address_set_host(&m_ServerHint, hostName);
	m_ServerHint.port = port;
}

bool Client::ConnectToServer()
{
	m_Server = enet_host_connect(m_Client, &m_ServerHint, 2, 0);

	if (m_Server == nullptr)
	{
		std::cout << "Wasn't able to initialize connection\n";

		// don't forget
		enet_host_destroy(m_Client);
		return false;
	}
	std::cout << "Successful connection at: " << m_ServerHint.host << "::" << m_ServerHint.port << std::endl;
	return true;
}