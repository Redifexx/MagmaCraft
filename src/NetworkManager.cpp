#include "NetworkManager.h"

using namespace Craft;

void PacketWriter::WriteInt(int value)
{
	// write int as 4 bytes (little-endian)
	buffer.push_back(static_cast<uint8_t>(value & 0xFF));
	buffer.push_back(static_cast<uint8_t>((value >> 8) & 0xFF));
	buffer.push_back(static_cast<uint8_t>((value >> 16) & 0xFF));
	buffer.push_back(static_cast<uint8_t>((value >> 24) & 0xFF));
}

void PacketWriter::WriteData(const void* data, size_t size)
{
	const uint8_t* bytes = static_cast<const uint8_t*>(data);
	buffer.insert(buffer.end(), bytes, bytes + size);
}

bool NetworkManager::Begin()
{
	switch (m_NetworkRole)
	{
		case NetworkRole::SOLO:
			// Until I send data directly, use localhost
			m_Server = new Server();
			m_Client = new Client();
			m_WorldManager = new WorldManager();
			m_WorldRenderer = new WorldRenderer();

			return true;
		case NetworkRole::SERVER:
			m_Server = new Server();
			m_WorldManager = new WorldManager();
			return true;

		case NetworkRole::CLIENT:
			m_Client = new Client();
			m_WorldRenderer = new WorldRenderer();
			return true;

		default:
			return false;
	}
}

void NetworkManager::End()
{
	if (m_Server != nullptr) delete m_Server;
	if (m_Client != nullptr) delete m_Client;
	if (m_WorldManager != nullptr) delete m_WorldManager;
	if (m_WorldRenderer != nullptr) delete m_WorldRenderer;
	NetworkRole m_Role = NetworkRole::NONE;
}	

void NetworkManager::SendChunkData(ENetPeer* peer, int chunkX, int chunkZ)
{
	// get compressed chunk data
	std::vector<uint8_t> chunkData = m_WorldManager->GetChunkCompressed(chunkX, chunkZ);

	// write to packet
	PacketWriter writer;
	writer.WriteByte(static_cast<uint8_t>(PacketType::CHUNK_DATA));
	writer.WriteInt(chunkX);
	writer.WriteInt(chunkZ);
	writer.WriteData(chunkData.data(), compressedData.size());

	// create ENet packet
	ENetPacket* packet = enet_packet_create(
		writer.buffer.data(),
		writer.buffer.size(),
		ENET_PACKET_FLAG_RELIABLE // bc chunk data is important
	);

	// send
	enet_peer_send(peer, 0, packet);
}

void NetworkManager::Update()
{
	// Implementation for updating network state and handling incoming packets
}

void NetworkManager::HandlePacket(ENetPacket* packet, ENetPeer* peer)
{
	// Implementation for handling received packets
}