#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include "Server.h"
#include "Client.h"

namespace Craft
{
	enum class NetworkRole
	{
		NONE,
		SOLO,
		SERVER,
		CLIENT
	};

	enum class PacketType
	{
		HANDSHAKE,
		MESSAGE,
		CHUNK_REQUEST,
		CHUNK_DATA,
		BLOCK_UPDATE
	};

	struct PacketWriter
	{
		std::vector<uint8_t> buffer;

		void WriteByte(uint8_t value) { buffer.push_back(value); }
		void WriteInt(int value);
		void WriteData(const void* data, size_t size);

	};


	class NetworkManager
	{
		public:
			void Begin();
			void End();
			void GetNetworkRole() const { return m_Role; }
			void SetNetworkRole(NetworkRole role) { m_Role = role; }
		private:
			NetworkRole m_Role = NetworkRole::NONE;
	};
}