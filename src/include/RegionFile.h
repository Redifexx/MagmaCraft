#pragma once

#include <string>
#include <fstream>
#include <vector>
#include <mutex>
#include <map>
#include <filesystem>
#include "Chunk.h"

// instead of reading/writing a single file per chunk
// group chunks into regions then store those regions in their own file
// like minecraft anvil's format but 3D
namespace Craft
{
	const int REGION_WIDTH = 32; // 32^3 chunks per file
	const int REGION_SHIFT = 5;
	const int REGION_MASK = 31;
	const int SECTOR_SIZE = 4096; // 4KB alignment

	struct RegionLocation
	{
		// 3 bytes for the sector offset
		uint32_t offset = 24;
		// 1 byte for the sector count
		uint32_t count = 8;
	};

	class RegionFile
	{
		public:
			RegionFile(const std::string& path);
			~RegionFile();

			bool ReadChunk(int localX, int localY, int localZ, std::vector<uint8_t>& outData);
			void WriteChunk(int localX, int localY, int localZ, std::vector<uint8_t>& data);

		private:
			std::string m_FilePath;
			std::fstream m_FileStream;
			std::mutex m_FileMutex;

			// header table
			std::vector<RegionLocation> m_Header;

			// helper to find free space
			//uint32_t FindFreeSector(uint32_t sectorCount);
	};
}