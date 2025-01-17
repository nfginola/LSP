#include "pch.h"
#include "Audio.h"
#include <ctime>
Audio::Audio() 
{
	m_soundLoop = false;
	m_volume = 0.0f;
	m_volume2 = 0.8f;
	m_pitch = -0.3f;
	m_mute = true;
}

Audio::~Audio()
{
}

void Audio::start(int i)
{
	std::vector<std::string> v1;
	v1.push_back("Sounds/boing2.wav");
	v1.push_back("Sounds/hook.wav");
	v1.push_back("Sounds/landing.wav");
	v1.push_back("Sounds/landing2.wav");
	v1.push_back("Sounds/checkpoint.wav");
	v1.push_back("Sounds/dehook.wav");
	v1.push_back("Sounds/jump1a.wav");
	v1.push_back("Sounds/jump2a.wav");
	v1.push_back("Sounds/jump3a.wav");
	v1.push_back("Sounds/wallrunBongo.wav");
	v1.push_back("Sounds/swingT.wav");
	v1.push_back("Sounds/woosh.wav");
	v1.push_back("Sounds/wilhem.wav");
	m_sound.start(v1);
	m_sound.setVolume(0.8f);
	std::vector<std::string> v;
	v.push_back("Sounds/heartbeat.wav");
	m_sound2.start(v);
	m_sound2.playLoop("Sounds/heartbeat.wav");
	std::vector<std::string> v4;
	v4.push_back("Sounds/footstep1.wav");
	m_sound3.start(v4);
	m_sound3.setVolume(0.7f);
	std::vector<std::string> v3;
	v3.push_back("Sounds/Spring2.wav");
	v3.push_back("Sounds/fall2.wav");
	m_music.start(v3);
	if(i == 0)
		m_music.playLoop("Sounds/Spring2.wav");
	else
		m_music.playLoop("Sounds/fall2.wav");
}

void Audio::update(bool ground, bool hook, bool wall, DirectX::SimpleMath::Vector3 velocity, float delta)
{
	if (m_mute) {
		if (velocity.Length() < 0.1 || (ground == false && wall == false && hook == false) && m_soundLoop == true)
		{
			m_soundLoop = false;
			m_sound.stopLoop();
			m_sound3.stopLoop();
		}
		else if (velocity.Length() != 0 && ground == true && m_soundLoop == false) {
			m_soundLoop = true;
			m_sound3.playLoop("Sounds/footstep1.wav");
		}
		else if (velocity.Length() != 0 && wall == true && m_soundLoop == false)
		{
			m_soundLoop = true;
			m_sound.playLoop("Sounds/wallrunBongo.wav");
		}
		else if (velocity.Length() != 0 && hook == true && ground == false && wall == false && m_soundLoop == false)
		{
			m_soundLoop = true;
			if (random() < 2)
				m_sound.playLoop("Sounds/swingT.wav");
		}

		if (50 < velocity.Length())
		{
			if (m_volume < 0.9f)
				m_volume += 0.05f * delta;
			if (m_pitch <= 0)
				m_pitch += 0.11f * delta;
		}
		else if (85 > velocity.Length() && ground == true)
		{
			if (m_volume > 0.1f)
				m_volume -= 0.02f * delta;
			if (m_pitch >= -0.7f)
				m_pitch -= 0.1f * delta;
		}
		//HerthBeat
		if (velocity.Length() < 10)
		{
			m_volume2 += 0.1f * delta;
		}
		else
		{
			if (m_volume2 > 0)
				m_volume2 -= 1 * delta;
		}
		m_sound2.setVolume(m_volume2);
		m_music.setVolume(m_volume);
		m_music.setPitch(m_pitch);
		m_sound3.setVolume(0.7f);
		m_sound3.setPitch(m_pitch);
	}
}

void Audio::playSound1(const std::string& soundName)
{
	m_sound.play(soundName);
}

void Audio::mute()
{
	m_mute = false;
	m_sound.setVolume(0);
	m_sound2.setVolume(0);
	m_sound3.setVolume(0);
	m_music.setVolume(0);
}

void Audio::unmute()
{
	m_sound.setVolume(1);
	m_mute = true;
}

int Audio::random()
{
	std::srand(static_cast<unsigned int>(std::time(nullptr))); // use current time as seed for random generator
	int random_variable = std::rand();

	// roll 6-sided dice 20 times
	int x = 7;
	while (x > 6)
		x = 1 + std::rand() / ((RAND_MAX + 1u) / 6);  // Note: 1+rand()%6 is biased
	return x;
		

}

void Audio::stopWalking()
{
	m_soundLoop = false;
	m_sound3.stopLoop();
}