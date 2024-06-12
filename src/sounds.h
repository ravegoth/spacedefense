#include <iostream>
#include <vector>
#include <string>
#include <SFML/Audio.hpp>

using namespace std;
using namespace sf;

class SoundPlayer {
private:
    string name;
    string path;
    SoundBuffer buffer;
    Sound sound;
    float duration;
public:
    // constructor cu param
    SoundPlayer(string name, string path) : name(name), path(path) {
        if (!buffer.loadFromFile(path)) {
            cerr << "Error loading sound from path: " << path << endl;
        } else {
            sound.setBuffer(buffer);
            duration = buffer.getDuration().asSeconds();
        }
    }

    // constr cu TOTI param (aproape pt ca duration se pune automat)
    SoundPlayer(string name, string path, float volume, bool loop) : name(name), path(path) {
        if (!buffer.loadFromFile(path)) {
            cerr << "Error loading sound from path: " << path << endl;
        } else {
            sound.setBuffer(buffer);
            sound.setVolume(volume);
            sound.setLoop(loop);
            duration = buffer.getDuration().asSeconds();
        }
    }

    // default constructor
    SoundPlayer() : name(""), path("") {}

    void setVolume(float volume) {
        sound.setVolume(volume);
    }

    void setLoop(bool loop) {
        sound.setLoop(loop);
    }

    void play() {
        sound.play();
    }

    string getName() const {
        return name;
    }

    string getPath() const {
        return path;
    }

    const SoundBuffer& getBuffer() const {
        return buffer;
    }

    Sound& getSound() {
        return sound;
    }

    ~SoundPlayer() {
        sound.stop();
    }

    // copy constructor
    SoundPlayer(const SoundPlayer& other) : name(other.name), path(other.path), buffer(other.buffer) {
        sound.setBuffer(buffer);
    }

    // operatoru de atribuire
    SoundPlayer& operator=(const SoundPlayer& other) {
        if (this != &other) {
            name = other.name;
            path = other.path;
            buffer = other.buffer;
            sound.setBuffer(buffer);
        }
        return *this;
    }
};

// singleton
class SoundEngine {
private:
    SoundEngine() {}
    ~SoundEngine() {
        for (auto& soundPlayer : sounds) {
            soundPlayer.getSound().stop();
        }
    }
    SoundEngine(const SoundEngine&) = delete;
    SoundEngine& operator=(const SoundEngine&) = delete;

    vector<SoundPlayer> sounds;
public:
    static SoundEngine& getInstance() {
        static SoundEngine instance;
        return instance;
    }

    void add(const SoundPlayer& sound) {
        sounds.push_back(sound);
    }

    const vector<SoundPlayer>& getSounds() const {
        return sounds;
    }

    // se adauga in vector
    void defineSound(const string& name, const string& path) {
        SoundPlayer sound(name, path);
        if (sound.getBuffer().getDuration().asSeconds() == 0) {
            cerr << "Failed to define sound: " << name << endl;
            return;
        }
        sounds.push_back(sound);
    }

    // play
    bool playSound(const string& name) {
        for (auto& sound : sounds) {
            if (sound.getName() == name) {
                sound.play();
                return true;
            }
        }
        cerr << "Sound not found: " << name << endl;
        return false;
    }

    // stop
    bool stopSound(const string& name) {
        for (auto& sound : sounds) {
            if (sound.getName() == name) {
                sound.getSound().stop();
                return true;
            }
        }
        cerr << "Sound not found: " << name << endl;
        return false;
    }

    // get duratia sunetului
    float getSoundDuration(const string& name) const {
        for (const auto& sound : sounds) {
            if (sound.getName() == name) {
                return sound.getBuffer().getDuration().asSeconds();
            }
        }
        cerr << "Sound not found: " << name << endl;
        return -1;
    }

    // schimbarea volumului
    void setSoundVolume(const string& name, float volume) {
        for (auto& sound : sounds) {
            if (sound.getName() == name) {
                sound.setVolume(volume);
                return;
            }
        }
        cerr << "Sound not found: " << name << endl;
    }
};

// va fi on loop mereu, fiind background music-u
// made by traian + sirrackz
// va folosi sf::Music in loc de sf::Sound
// ca sa fie prin streaming
// si va fii on loop
// doar play() si mute() vor fi folosite
class SoundTrackPlayer {
private:
    string name;
    string path;
    Music music;
public:
    // constructor (TREBUIE FOLOSIT NEAPARAT)
    SoundTrackPlayer(string name, string path) : name(name), path(path) {
        if (!music.openFromFile(path)) {
            cerr << "Error loading sound from path: " << path << endl;
        } else {
            music.setLoop(true);
        }
    }

    // default constructor
    SoundTrackPlayer() : name(""), path("") {}

    // play music
    void play() {
        music.play();
    }

    // mute music
    void mute() {
        music.setVolume(0);
    }

    // unmute music
    void unmute() {
        music.setVolume(100);
    }

    // get name
    string getName() const {
        return name;
    }

    // get path
    string getPath() const {
        return path;
    }

    // get music
    Music& getMusic() {
        return music;
    }

    // destructor
    ~SoundTrackPlayer() {
        music.stop();
    }

    // copy constructor
    SoundTrackPlayer(const SoundTrackPlayer& other) : name(other.name), path(other.path) {
        music.openFromFile(path);

        // trebuie verificat chiar daca e copy constructor ca sa previna orice eroare
        if (!music.openFromFile(path)) {
            cerr << "Error loading sound from path: " << path << endl;
        } else {
            music.setLoop(true);
        }
    }

    // operatoru de atribuire
    SoundTrackPlayer& operator=(const SoundTrackPlayer& other) {
        if (this != &other) {
            name = other.name;
            path = other.path;
            music.openFromFile(path);

            // trebuie verificat chiar daca e copy constructor ca sa previna orice eroare
            if (!music.openFromFile(path)) {
                cerr << "Error loading sound from path: " << path << endl;
            } else {
                music.setLoop(true);
            }
        }
        return *this;
    }
};

