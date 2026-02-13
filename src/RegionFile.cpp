#include "RegionFile.h"
#include <iostream>

using namespace Craft;

RegionFile::RegionFile(const std::string& path) : m_FilePath(path)
{
	// open r/w binary
	m_FileStream.open(path, std::ios::in | std::ios::out | std::ios::binary);

	// if it doesnt exist
	if (!m_FileStream.is_open())
	{
		// create file
		m_FileStream.open(path, std::ios::out | std::ios::binary);
		m_FileStream.close();

		// reopen in r/w mode
		m_FileStream.open(path, std::ios::in | std::ios::out | std::ios::binary);
	}

	// setup header
	m_Header.resize(REGION_WIDTH * REGION_WIDTH * REGION_WIDTH, { 0, 0 });

	// check for header in file
	m_FileStream.seekg(0, std::ios::end);
	size_t fileSize = m_FileStream.tellg();
	size_t headerSize = m_Header.size() * sizeof(uint32_t); // 4 bytes per entry

	if (fileSize < headerSize)
	{
		// new file!
		m_FileStream.seekp(0);

		// fill header with 0s
		std::vector<uint32_t> blankHeader(m_Header.size(), 0);
		m_FileStream.write(reinterpret_cast<char*>(blankHeader.data()), headerSize);
	}
	else
	{
		// read header
		m_FileStream.seekg(0);
		m_FileStream.read(reinterpret_cast<char*>(m_Header.data()), headerSize);
	}
}

RegionFile::~RegionFile()
{
	if (m_FileStream.is_open()) { m_FileStream.close(); }
}

// helper to get index from 3D coords
inline int GetRegionIndex(int x, int y, int z)
{
	return (x & REGION_MASK) + ((z & REGION_MASK) * REGION_WIDTH) + ((y & REGION_MASK) * REGION_WIDTH * REGION_WIDTH);
}

bool RegionFile::ReadChunk(int localX, int localY, int localZ, std::vector<uint8_t>& outData)
{
	std::lock_guard<std::mutex> lock(m_FileMutex);

	int index = GetRegionIndex(localX, localY, localZ);
	RegionLocation location = m_Header[index];

	if (location.offset == 0 || location.count == 0) { return false; }

	// seek to sector
	m_FileStream.seekg(location.offset * SECTOR_SIZE);

	uint32_t dataLength;
	m_FileStream.read(reinterpret_cast<char*>(&dataLength), 4);

	// safety
	if (dataLength > (location.count * SECTOR_SIZE)) { return false; }

	// read compressed data
	outData.resize(dataLength);
	m_FileStream.read(reinterpret_cast<char*>(outData.data()), dataLength);

	return true;
}

void RegionFile::WriteChunk(int localX, int localY, int localZ, std::vector<uint8_t>& data)
{
	std::lock_guard<std::mutex> lock(m_FileMutex);

	int index = GetRegionIndex(localX, localY, localZ);

	// calc needed sectors
	uint32_t requiredSectors = (data.size() + 4 + SECTOR_SIZE - 1) / SECTOR_SIZE;

	RegionLocation& location = m_Header[index];

	// for now, gonna write only to the end of files to avoid overwrites
	m_FileStream.seekg(0, std::ios::end);
	uint32_t fileEnd = m_FileStream.tellg();
	uint32_t newOffset = fileEnd / SECTOR_SIZE;

	// check if old spot reuseable
	if (location.offset != 0 && location.count == requiredSectors)
	{
		newOffset = location.offset;
	}

	// write length
	m_FileStream.seekp(newOffset * SECTOR_SIZE);

	uint32_t dataLength = data.size();
	m_FileStream.write(reinterpret_cast<char*>(&dataLength), 4);

	// write data
	m_FileStream.write(reinterpret_cast<const char*>(data.data()), dataLength);

	// update header in memory
	location.offset = newOffset;
	location.count = requiredSectors;

	// update header in disk
	m_FileStream.seekp(index * sizeof(uint32_t));
	m_FileStream.write(reinterpret_cast<char*>(&location), sizeof(uint32_t));
}

