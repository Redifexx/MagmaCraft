#include "Server.h"
#include <string>

using namespace Magma;

// Thank you Low Level Game Dev
Server::Server()
{
	m_Server = nullptr;
	m_ServerHint.host = ENET_HOST_ANY; // takes clients from any IP address
	m_ServerHint.port = 1233; // check ports later

	m_Server = enet_host_create(&m_ServerHint /* the address to bind the server host to */,
		m_MaxClients /* allow up to m_MaxClients clients and/or outgoing connections */,
		2 /* allow up to 2 channels to be used, 0 & 1 */,
		0 /* assume any amount of incoming bandwidth */,
		0 /* assume any amount of outgoing bandwidth */
	);

	if (m_Server == NULL)
	{
		std::cout << "An error occurred while trying to create an ENet server host.\n";
		return;
	}
}

Server::~Server()
{
	delete m_Server;
}

void Server::Update()
{
	if (m_Server == nullptr) {
		printf("CRITICAL ERROR: ENetHost is NULL! Server failed to start.\n");
		return;
	}
	ENetEvent event;
	while (enet_host_service(m_Server, &event, 0) > 0)
	{
		switch (event.type)
		{
			case ENET_EVENT_TYPE_CONNECT:
				printf("A new client connected from %x:%u.\n",
					event.peer->address.host,
					event.peer->address.port);
				std::cout << "Welcome Player " << event.peer->address.host << "!" << std::endl;
				m_Clients[event.peer->incomingPeerID] = event.peer;
				std::cout << "Client Count : [" << m_Clients.size() << "/" << m_MaxClients << "]" << std::endl;
				break;

			case ENET_EVENT_TYPE_RECEIVE:
			{
				std::string msg = "Player " + std::to_string(event.peer->address.host)
					+ ":" + std::to_string(event.peer->incomingPeerID) + " wrote: " + reinterpret_cast<const char*>(event.packet->data);
				SendPacket(msg.c_str(), true);

				m_MessageBuffer.push_back(std::string((char*)event.packet->data));
				if (m_MessageBuffer.size() > 128)
				{
					m_MessageBuffer.erase(m_MessageBuffer.begin());
				}

				//event.packet->dataLength
				//event.packet->data
				//event.channelID
				/* clean up the packet after usage */
				enet_packet_destroy(event.packet);
			}
				break;

			case ENET_EVENT_TYPE_DISCONNECT:
				std::cout << "Player " << event.peer->address.host << " disconnected." << std::endl;
				m_Clients.erase(event.peer->incomingPeerID);
				break;
		}
	}
}

void Server::SendPacket(const char* data, bool isReliable)
{
	ENetPacket* packet = enet_packet_create(data, strlen(data) + 1, (isReliable ? ENET_PACKET_FLAG_RELIABLE : 0));

	for (const auto& [id, peer] : m_Clients)
	{
		enet_peer_send(peer, 0, packet);
		std::cout << "[" << data << "] sent to " << peer->address.host << ":" << peer->incomingPeerID << std::endl;
	}
	enet_host_flush(m_Server);
}
