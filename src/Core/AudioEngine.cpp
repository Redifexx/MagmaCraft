#include "Core/AudioEngine.h"
#include <iostream>

#define MINIAUDIO_IMPLEMENTATION
#include <miniaudio.h>
#include <filesystem>

namespace Magma
{
	ma_engine* AudioEngine::s_Engine = nullptr;
	std::map<std::string, ma_sound*> AudioEngine::s_SoundCache;
	std::vector<ma_sound*> AudioEngine::s_ActiveSounds;

	void AudioEngine::Init()
	{
		s_Engine = new ma_engine();
		if (ma_engine_init(NULL, s_Engine) != MA_SUCCESS)
		{
			delete s_Engine;
			s_Engine = nullptr;
			throw std::runtime_error("Failed to initialize audio engine");
		}
	}

	void AudioEngine::Shutdown()
	{
		for (auto& [path, sound] : s_SoundCache)
		{
			ma_sound_uninit(sound);
			delete sound;
		}
		s_SoundCache.clear();

		ma_engine_uninit(s_Engine);
		delete s_Engine;
	}

	void AudioEngine::UpdateListener(const glm::vec3& position, const glm::vec3& forward, const glm::vec3& up)
	{
		if (!s_Engine) return;

		// Update listener position and orientation
		// only one listener because single player
		ma_engine_listener_set_position(s_Engine, 0, position.x, position.y, position.z);
		ma_engine_listener_set_direction(s_Engine, 0, forward.x, forward.y, forward.z);
		ma_engine_listener_set_world_up(s_Engine, 0, up.x, up.y, up.z);
	}

	void AudioEngine::UpdateActiveSounds()
	{
		if (!s_Engine) return;

		// Clean up finished sounds to avoid mem leaks
		auto it = s_ActiveSounds.begin();
		while (it != s_ActiveSounds.end())
		{
			ma_sound* sound = *it;
			ma_bool32 isPlaying = ma_sound_is_playing(sound);
			if (!isPlaying)
			{
				ma_sound_uninit(sound);
				delete sound;
				it = s_ActiveSounds.erase(it);
			}
			else
			{
				++it;
			}
		}
	}

	void AudioEngine::PlayAtLocation(const std::string& filepath, const glm::vec3& position, float volume, bool loop)
	{
		if (!s_Engine) return;
		
		ma_sound* sound = new ma_sound();

		ma_result result = ma_sound_init_from_file(s_Engine, filepath.c_str(), 0, NULL, NULL, sound);

		if (result != MA_SUCCESS)
		{
			delete sound;
			std::cerr << "Failed to load sound: " << filepath << " | Error Code: " << result << std::endl;
			return;
		}

		// Spatialization Settings
		ma_sound_set_position(sound, position.x, position.y, position.z);
		ma_sound_set_volume(sound, volume);

		ma_sound_set_spatialization_enabled(sound, MA_TRUE);

		ma_sound_set_attenuation_model(sound, ma_attenuation_model_inverse); // inverse distance attenuation
		ma_sound_set_min_distance(sound, 1.0f); // ideally would be based on world and object emitter
		ma_sound_set_max_distance(sound, 100.0f);

		if (loop)
		{
			ma_sound_set_looping(sound, MA_TRUE);
		}

		ma_sound_start(sound);
		s_ActiveSounds.push_back(sound);
	}

	void AudioEngine::PlayGlobal(const std::string& filepath, float volume, bool loop)
	{
		if (!s_Engine) return;

		ma_sound* sound = new ma_sound();

		ma_result result = ma_sound_init_from_file(s_Engine, filepath.c_str(), 0, NULL, NULL, sound);
		if (result != MA_SUCCESS)
		{
			delete sound;
			std::printf("Failed to load sound.\n");
			return;
		}

		ma_sound_set_volume(sound, volume);
		ma_sound_set_spatialization_enabled(sound, MA_FALSE);

		if (loop)
		{
			ma_sound_set_looping(sound, MA_TRUE);
		}

		ma_sound_start(sound);
		s_ActiveSounds.push_back(sound);
	}

	void AudioEngine::StopGlobal()
	{
		if (!s_Engine) return;
		for (auto& sound : s_ActiveSounds)
		{
			ma_sound_stop(sound);
		}
	}
}
