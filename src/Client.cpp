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

void Client::Update()
{
	ENetEvent event;
	while (enet_host_service(m_Client, &event, 0) > 0)
	{
		switch (event.type)
		{
			case ENET_EVENT_TYPE_CONNECT:
				std::cout << "Connection to server succeeded.\n";
				break;
			case ENET_EVENT_TYPE_RECEIVE:
				std::cout << (char*)event.packet->data << std::endl;
				enet_packet_destroy(event.packet);
				break;
			case ENET_EVENT_TYPE_DISCONNECT:
				std::cout << "Disconnected from server.\n";
				break;
		}
	}
}

void Client::SendPacket(const char* data, bool isReliable)
{
	ENetPacket* packet = enet_packet_create(data, strlen(data) + 1, (isReliable ? ENET_PACKET_FLAG_RELIABLE : 0));
	enet_peer_send(m_Server, 0, packet);
	std::cout << "[" << data << "] sent to " << m_Server->address.host << ":" << m_Server->incomingPeerID << std::endl;
	enet_host_flush(m_Client);
}