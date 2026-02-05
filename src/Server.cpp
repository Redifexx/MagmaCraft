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
	if (m_Server == nullptr) return;
	ShutdownServer();
}

void Server::ShutdownServer()
{
	for (auto& [peerID, peer] : m_Clients)
	{
		enet_peer_disconnect_now(peer, 0);
	}
	enet_host_flush(m_Server);
	enet_host_destroy(m_Server);
	m_Server = nullptr;
}
