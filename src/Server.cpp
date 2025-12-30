#include "Server.h"

using namespace Magma;

// Thank you Low Level Game Dev
Server::Server()
{
	m_Host = nullptr;
	m_ServerHint.host = ENET_HOST_ANY; // takes clients from any IP address
	m_ServerHint.port = 1233; // check ports later

	m_Host = enet_host_create(&m_ServerHint /* the address to bind the server host to */,
		m_MaxClients /* allow up to m_MaxClients clients and/or outgoing connections */,
		2 /* allow up to 2 channels to be used, 0 & 1 */,
		0 /* assume any amount of incoming bandwidth */,
		0 /* assume any amount of outgoing bandwidth */
	);

	if (m_Host == NULL)
	{
		std::cout << "An error occurred while trying to create an ENet server host.\n";
		return;
	}
}

Server::~Server()
{
	delete m_Host;
}

void Server::Update()
{
	ENetEvent event;
	while (enet_host_service(m_Host, &event, 0) > 0)
	{
		switch (event.type)
		{
			case ENET_EVENT_TYPE_CONNECT:
				printf("A new client connected from %x:%u.\n",
					event.peer->address.host,
					event.peer->address.port);

				m_Clients[event.peer->incomingPeerID] = event.peer;
				break;

			case ENET_EVENT_TYPE_RECEIVE:
				printf("A packet of length %u containing %s was received on channel %u.\n",
					event.packet->dataLength,
					event.packet->data,
					event.channelID);
				/* clean up the packet after usage */
				enet_packet_destroy(event.packet);
				break;

			case ENET_EVENT_TYPE_DISCONNECT:
				printf("client disconnected.\n");
				m_Clients.erase(event.peer->incomingPeerID);
				break;
		}
	}
}
