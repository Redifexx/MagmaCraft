#pragma once

#include <string>
#include <map>
#include <glm/glm.hpp>
#include <vector>

struct ma_engine;
struct ma_sound;

// Uses miniaudio lib to play audio
// Can easily be swapped with another audio lib!
namespace Magma
{
	class AudioEngine
	{
	public:
		static void Init();
		static void Shutdown();

		static void UpdateListener(const glm::vec3& position, const glm::vec3& forward, const glm::vec3& up);
		static void UpdateActiveSounds();

		static void PlayAtLocation(const std::string& filepath, const glm::vec3& position, float volume = 1.0f, bool loop = false);
		static void PlayGlobal(const std::string& filepath, float volume = 1.0f, bool loop = false);

		static void StopGlobal();

	private:
		static ma_engine* s_Engine;
		static std::map<std::string, ma_sound*> s_SoundCache;
		static std::vector<ma_sound*> s_ActiveSounds;
	};
}