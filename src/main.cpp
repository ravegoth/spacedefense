// specifice c++
#include <iostream>
#include <cmath>
#include <ctime>
#include <cstdlib> 
#include <string>
#include <vector>
#include <memory>
#include <time.h>
#include <algorithm>
#include <filesystem>
#include <windows.h>
#include <tlhelp32.h>

// librariile pentru grafica
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <SFML/System.hpp>
#include <SFML/Audio.hpp>

// header-urile faculte de mine
#include "brain.h"
#include "clock.h"
#include "sounds.h"
#include "textgen.h"
#include "colorconvert.h"

using namespace std;
using namespace sf; // sfml

// -------------------------------------------------------------------- CONSTANTE --------------------------------------------------------------------

// initial acestea trebuiau sa fie scrise
// cu #define da nu e voie cu macro-uri
const unsigned int FPS_LIMIT = 60;
const unsigned int NUMARUL_DE_STELE = 300;
const unsigned int NUMARUL_DE_PLANETE = 6;
const unsigned int NUMAR_CLONE_MAXIM = 4;

// -------------------------------------------------------------------- FUNCTII COD UTILITARE --------------------------------------------------------------------

// randomizare uniforma intre doua numere
// probabil e o idee proasta sa folosesc rand
// in loc de std::uniform_real_distribution
float rand_uniform(float a, float b) {
    return rand() / (RAND_MAX + 1.0) * (b - a) + a; // nu-s sigur daca e bine //! de editat
}

// convertor de probabilitate (din n in n secunde --> cadre)
template<typename T> 
inline bool probability_second(T n) {
    return rand_uniform(0, 60 * n) < 1;
}

// distanta dintre puncte reprezentate de vectori
template<typename T>
float distance(Vector2<T> a, Vector2<T> b) {
    return sqrt(pow(a.x - b.x, 2) + pow(a.y - b.y, 2));
}

int main(); // declarare pt a putea fi folosita inainte de definitie (mai jos chiar)

// entry point pt aplicatie windows
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd) {
    // trebuie ascunsa consola ca sa fie totu profesional
    HWND hWnd = GetConsoleWindow();
    ShowWindow(hWnd, SW_HIDE);
    return main(); 
}

// pt a ascunde cmd urile (.batu care va rula jocul)

// exceptie custom pt problemele legate de ascunderea cmd urilor (cu winapi)
//* (macar sa fie una in tot programu)
class HideCmdWindowsException : public exception {
    string message; // mesajul custom
public:
    HideCmdWindowsException(const string& msg) : message(msg) {} // constructor 

    // suprascrierea metoda what pentru a returna mesajul custom
    const char* what() const noexcept override {
        return message.c_str();
    }
};

// pt a ascunde cmd urile (batu care va rula jocul)
void hideAllCmdWindows() {
    HWND hwnd; // handle ul va fi pe rand fiecare cmd
    DWORD pid; // id-ul procesului
    PROCESSENTRY32 pe32; // stocheaza info despre proces
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0); // salveaza lista/snapshotu de procese si returneaza handle ul (ca sa le iterez)

    // daca n-a mers va arunca exceptia
    if (hSnapshot == INVALID_HANDLE_VALUE) {
        throw HideCmdWindowsException("Couldn't create process snapshot");
    }

    pe32.dwSize = sizeof(PROCESSENTRY32); // dimensiunea structurii e necesara pt fctz cu winapi altfel nu merg

    // daca nu s-a gasit nici macar PRIMUL proces din lista, arunca exceptia
    if (!Process32First(hSnapshot, &pe32)) {
        CloseHandle(hSnapshot);
        throw HideCmdWindowsException("Couldn't get first process in snapshot");
    }

    do {
        if (_stricmp(pe32.szExeFile, "cmd.exe") == 0) { // verifica daca e cmd, e doar comparare
            pid = pe32.th32ProcessID;
            hwnd = GetTopWindow(0); // ia fereastra top-level (prima)
            
            // itereaza peste toate ferestrele
            while (hwnd) {
                DWORD windowPid;
                
                GetWindowThreadProcessId(hwnd, &windowPid);
                
                // se adauga atributul de ascundere
                if (windowPid == pid) {
                    ShowWindow(hwnd, SW_HIDE);
                }
                hwnd = GetNextWindow(hwnd, GW_HWNDNEXT); // urmatoarea fereastra
            }
        }
    } while (Process32Next(hSnapshot, &pe32)); // itereaza pana la sfarsitul listei

    CloseHandle(hSnapshot); // inchide snapshotu
}

// acum inchiderea tuturor cmd-urilor (la sfarsit)
// analog ca mai sus doar ca foloseste PostMessage(hwnd, WM_CLOSE, 0, 0); pentru a inchide
void exitAllCmdWindows() {
    HWND hwnd;
    DWORD pid;
    PROCESSENTRY32 pe32;
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

    if (hSnapshot == INVALID_HANDLE_VALUE) {
        throw HideCmdWindowsException("Couldn't create process snapshot");
    }

    pe32.dwSize = sizeof(PROCESSENTRY32);

    if (!Process32First(hSnapshot, &pe32)) {
        CloseHandle(hSnapshot);
        throw HideCmdWindowsException("Couldn't get first process in snapshot");
    }

    do {
        if (_stricmp(pe32.szExeFile, "cmd.exe") == 0) {
            pid = pe32.th32ProcessID;
            hwnd = GetTopWindow(0);

            while (hwnd) {
                DWORD windowPid;

                GetWindowThreadProcessId(hwnd, &windowPid);

                if (windowPid == pid) {
                    PostMessage(hwnd, WM_CLOSE, 0, 0);
                }
                hwnd = GetNextWindow(hwnd, GW_HWNDNEXT);
            }
        }
    } while (Process32Next(hSnapshot, &pe32));

    CloseHandle(hSnapshot);
}

// -------------------------------------------------------------------- VARIABILE JOC --------------------------------------------------------------------

// elemente de baza legate de functnionalitate
bool mouseDown = false;
bool leftClick = false;
bool rightClick = false;
float mouseX, mouseY;
bool keysPressed[256]; // va fi true daca tasta cu codul i e apasata
bool inMenu = true;
bool gamePaused = false;
bool gameMusicMuted = false;
Font mainFont;
Texture tempTexture;
Sprite tempSprite;
SoundEngine& soundManager = SoundEngine::getInstance(); // singleton
SoundTrackPlayer soundTrack("bgm", "./res/bgmCropped.wav"); // asta nu e singleton ca poate vreau sa pun in viitor mai multe melodii de exemplu
MinuteClockStamp<unsigned> *gameClock = new MinuteClockStamp<unsigned>(); // pentru a sti cat a durat jocul (pt dificultate)
TextGen& textGen = TextGen::getInstance(); // singleton

// chestii de joc si game mechanics
float playerVelocityX = 0;
float playerVelocityY = 0; 
float speedLimit = 3; // limita de viteza pe care o poate avea playerul
unsigned int firePause = 0; // daca e mai mare ca 0, nu se poate trage
bool lightSpeedOn = false;
bool trailEffect = false; // pentru viteza luminii
bool playerAccelerating = false; // pentru efectul de accelerare (se schimba texturile)
unsigned int upgradeIconOrder = 0; // pentru a sti ce upgrade sa se afiseze
unsigned int money = 0; // banii vor fi folositi pentru a da cumpara si a face trading cu extraterestrii (sau pariat)
bool safeZone = false; // daca safezone e true, playerul va avea shield care se regenereaza f rapid + nu se spawneaza inamicii
unsigned int planetPlayerIsOn = 0; // planeta pe care se afla playerul
bool freshVisit = false; // semnalul ca tocmai a vizitat planeta noua
bool moonEnergy = false; // daca e true, playerului ii va creste fuelu

// -------------------------------------------------------------------- OBIECTE JOC --------------------------------------------------------------------

// vor fi in background
// si vor crea efectul de paralax
// cand jucatoru se misca gen
class Star {
private:
    float radius;
    float x, y;
    float speed;

public:
    Star() {
        // radiusu va fi aleator
        radius = rand_uniform(1, 3);

        // initial si pozitia va fi random dar se vor repeta
        // dupa ce trec de margini
        // ca lowkey sa se economiseasca resurse
        // x intre -100 si 900
        x = rand() % 1000 - 100;
        // y intre -100 si 700
        y = rand() % 800 - 100;

        // speedu va fi proportional cu radiusu
        // asta va face efectu de distanta
        speed = 1 + radius / 4;
    }

    void move() {
        // la margini se schimba xu si yu ca sa nu
        // se poata vedea un pattern al stelelor
        // deci se creaza iluzia de infinit
        // tho problema e ca daca jucatoru se misca
        // inapoi, va vedea alte stele

        // e o margine de 100px dupa edge
        if (y > 700) y = -100, x = rand() % 1000 - 100;
        if (y < -100) y = 700, x = rand() % 1000 - 100;
        if (x > 900) x = -100, y = rand() % 800 - 100;
        if (x < -100) x = 900, y = rand() % 800 - 100;

        // se misca cum se misca jucatoru
        // (adica velocitatea opusa)
        x -= playerVelocityX * speed;
        y -= playerVelocityY * speed;
    }

    void draw(RenderWindow &window) {
        CircleShape star(radius);
        star.setFillColor(Color::White);
        star.setPosition(x - radius, y - radius);
        // bugfix: transparenta e 0
        star.setFillColor(Color(255, 255, 255, 255));
        star.setOutlineColor(Color::White);
        star.setOutlineThickness(1);
        window.draw(star);
        // draw 1px at -1 -1 solid rectangle
        // to fix the bug
    }

    // getterul pentru locatie
    Vector2f location() {
        return Vector2f(x, y);
    }

    // operatorul star += vector e pentru suma
    // simultana pentru x si y
    Vector2f operator+=(Vector2f v) {
        x += v.x;
        y += v.y;
        return Vector2f(x, y);
    }

    // copy constructor
    Star(const Star& other) {
        radius = other.radius;
        x = other.x;
        y = other.y;
        speed = other.speed;
    }

    // op de atribuire
    Star& operator=(const Star& other) {
        if (this != &other) {
            radius = other.radius;
            x = other.x;
            y = other.y;
            speed = other.speed;
        }
        return *this;
    }

    // constructoru cu toti param (chiar daca nu va fi folosit)
    Star(float radius, float x, float y, float speed) : radius(radius), x(x), y(y), speed(speed) {}

    // operatoru << va arata locatia si marimea
    friend ostream& operator<<(ostream& os, const Star& star) {
        os << "Steaua:\nlocatie: x=[" << star.x << "] y=[" << star.y << "]\nmarime: [" << star.radius << "]\n";
        return os;
    }

    ~Star() {
        // nimic pentru ca nu a fost alocat nimic dinamic
    }
};

class Particle {
private:
    // proprietati de baza
    // float angleVariation;
    float sizeVariation;
    float colorVariation;

    // primul keyframe
    float initX, initY;
    float initSize;
    float initR, initG, initB, initA;

    // ultimul keyframe
    float endX, endY;
    float endSize;
    float endR, endG, endB, endA;

    // cele folosite pentru animatie
    float currentX, currentY;
    float currentSize;
    float currentR, currentG, currentB, currentA;

    // cea mai importanta este duratia
    unsigned int duration; // in cate cadre se va termina animatia
    unsigned int frame; // frame-ul curent

    // variabila pentru a sti daca trebuie stearsa 
    // din multimea principala
    bool deleteMe = false;
public:
    void nextFrame() {
        currentX += (endX - initX) / duration;
        currentY += (endY - initY) / duration;
        currentSize += (endSize - initSize) / duration;
        currentR += (endR - initR) / duration;
        currentG += (endG - initG) / duration;
        currentB += (endB - initB) / duration;
        currentA += (endA - initA) / duration;

        // mutatiile specifice culorilor
        currentR += rand_uniform(-colorVariation, colorVariation);
        currentG += rand_uniform(-colorVariation, colorVariation);
        currentB += rand_uniform(-colorVariation, colorVariation);
        currentA += rand_uniform(-colorVariation, colorVariation);

        // mutatiile specifice marimii
        currentSize += rand_uniform(-sizeVariation, sizeVariation);
        
        // se limiteaza culorile intre 0 - 255
        if (currentR > 255) currentR = 255;
        if (currentR < 0) currentR = 0;
        if (currentG > 255) currentG = 255;
        if (currentG < 0) currentG = 0;
        if (currentB > 255) currentB = 255;
        if (currentB < 0) currentB = 0;

        // daca se s-a terminat animatia se va sterge
        if (frame++ > duration) {
            deleteMe = true;
        }
    }

    // singurul constructor util deoarece prezinta animatia perfect
    Particle(
            float initX, float initY, float initSize, float initR, float initG, float initB, float initA,
            float endX, float endY, float endSize, float endR, float endG, float endB, float endA,
            unsigned int duration = 60, float sizeVariation = 0, float colorVariation = 0) {
        this->initX = initX;
        this->initY = initY;
        this->initSize = initSize;
        this->initR = initR;
        this->initG = initG;
        this->initB = initB;
        this->initA = initA;

        this->endX = endX;
        this->endY = endY;
        this->endSize = endSize;
        this->endR = endR;
        this->endG = endG;
        this->endB = endB;
        this->endA = endA;

        this->duration = duration;
        this->sizeVariation = sizeVariation;
        this->colorVariation = colorVariation;

        currentX = initX;
        currentY = initY;
        currentSize = initSize;
        currentR = initR;
        currentG = initG;
        currentB = initB;
        currentA = initA;

        frame = 0; // pentru a se tine minte cadrele spre sfarsit
    }
    
    // getterele pentru variabilele private

    bool needsDeletion() { // am inventat eu cuvantu asta probabil
        return deleteMe || currentSize < 0 || currentA < 1;
    }

    Vector2f location() {
        return Vector2f(currentX, currentY);
    }

    float size() {
        return currentSize;
    }

    float colorR() { return currentR; }
    float colorG() { return currentG; }
    float colorB() { return currentB; }
    float colorA() { return currentA; }

    // copy constr
    Particle(const Particle& other) {
        initX = other.initX;
        initY = other.initY;
        initSize = other.initSize;
        initR = other.initR;
        initG = other.initG;
        initB = other.initB;
        initA = other.initA;

        endX = other.endX;
        endY = other.endY;
        endSize = other.endSize;
        endR = other.endR;
        endG = other.endG;
        endB = other.endB;
        endA = other.endA;

        currentX = other.currentX;
        currentY = other.currentY;
        currentSize = other.currentSize;
        currentR = other.currentR;
        currentG = other.currentG;
        currentB = other.currentB;
        currentA = other.currentA;

        duration = other.duration;
        sizeVariation = other.sizeVariation;
        colorVariation = other.colorVariation;

        frame = other.frame;
        deleteMe = other.deleteMe;
    }

    // op de atribuire
    Particle& operator=(const Particle& other) {
        if (this != &other) {
            initX = other.initX;
            initY = other.initY;
            initSize = other.initSize;
            initR = other.initR;
            initG = other.initG;
            initB = other.initB;
            initA = other.initA;

            endX = other.endX;
            endY = other.endY;
            endSize = other.endSize;
            endR = other.endR;
            endG = other.endG;
            endB = other.endB;
            endA = other.endA;

            currentX = other.currentX;
            currentY = other.currentY;
            currentSize = other.currentSize;
            currentR = other.currentR;
            currentG = other.currentG;
            currentB = other.currentB;
            currentA = other.currentA;

            duration = other.duration;
            sizeVariation = other.sizeVariation;
            colorVariation = other.colorVariation;

            frame = other.frame;
            deleteMe = other.deleteMe;
        }
        return *this;
    }

    // << operator pt info despre particula
    // [initial] --> [final] gen
    friend ostream& operator<<(ostream& os, const Particle& particle) {
        os << "Particula:\nde la x=[" << particle.initX << "] --> [" << particle.endX << "]\n";
        os << "de la y=[" << particle.initY << "] --> [" << particle.endY << "]\n";
        os << "de la size=[" << particle.initSize << "] --> [" << particle.endSize << "]\n";
        os << "de la r=[" << particle.initR << "] --> [" << particle.endR << "]\n";
        os << "de la g=[" << particle.initG << "] --> [" << particle.endG << "]\n";
        os << "de la b=[" << particle.initB << "] --> [" << particle.endB << "]\n";
        os << "de la a=[" << particle.initA << "] --> [" << particle.endA << "]\n";
        os << "durata: " << particle.duration << " cadre\n";
        os << "variatiile: size=[" << particle.sizeVariation << "] color=[" << particle.colorVariation << "]\n";
        return os;
    }

    ~Particle() {
        // nimic pentru ca nu a fost alocat nimic dinamic
    }
};

class particleEngine {
    // pentru efecte vizuale de explozie
    // si alte chestii de genul
private:
    vector<Particle> particles;
public:
    particleEngine() {
        // initializarea pariiculelor e oricum goala
        // da scriu asta ashea ca sa fie good practice
        particles = vector<Particle>();
    }

    // copy constructoru
    particleEngine(const particleEngine& other) {
        particles = other.particles;
    }

    // operatorul de atribuire
    particleEngine& operator=(const particleEngine& other) {
        particles = other.particles;
        return *this;
    }

    // adaugarea unei particule se face si cu operatoru +
    // pentru ca e mai fancy
    // nu cred ca va fi folosit vreodata
    bool operator+(Particle particle) {
        try {
            particles.push_back(particle);
            return true;
        } catch (exception& e) {
            cerr << "Failed to add particle: " << e.what() << endl;
            return false; // eroare
        }
    }

    void addParticle(Particle particle) {
        particles.push_back(particle);
    }

    void draw(RenderWindow &window, string shape="circle") {
        // se deseneaza particulele
        if (shape == "circle") {
            for (auto &particle : particles) {
                CircleShape circle(particle.size());
                circle.setFillColor(Color(particle.colorR(), particle.colorG(), particle.colorB(), particle.colorA()));
                circle.setOrigin(particle.size() / 2, particle.size() / 2); // ca sa fie in centru
                circle.setPosition(particle.location());
                window.draw(circle);
            }
        } else if (shape == "rectangle") {
            for (auto &particle : particles) {
                RectangleShape rectangle(Vector2f(particle.size(), particle.size()));
                rectangle.setFillColor(Color(particle.colorR(), particle.colorG(), particle.colorB(), particle.colorA()));
                rectangle.setOrigin(particle.size() / 2, particle.size() / 2); // ca sa fie in centru
                rectangle.setPosition(particle.location());
                window.draw(rectangle);
            }
        }

        // pentru fiecare particula se face animatia corespunzatoare
        for (auto &particle : particles) {
            particle.nextFrame();
        }
    }

    void clear() {
        // se sterg particulele care trebuie sterse
        for (int i = 0; i < particles.size(); i++) {
            if (particles[i].needsDeletion()) {
                particles.erase(particles.begin() + i);
            }
        }
    }

    // generatoare de particule predefinite -------------

    // exploziile cand ... explodeaza ceva...
    void explosion(Vector2f location, float size, unsigned int duration = 60, unsigned int count = 60) {
        for (unsigned int i = 0; i < count; i++) {
            float angle = rand_uniform(0, 360) * 3.14159 / 180;
            float distance = rand_uniform(0, size * 10); 

            float colorVariation = rand_uniform(0, 3);
            float sizeVariation = rand_uniform(0, 1);

            float colorR, colorG;
            if (i%2 == 0) {
                // galben
                colorR = rand_uniform(200, 255);
                colorG = rand_uniform(200, 255);
            } else {
                // rosu
                colorR = rand_uniform(200, 255);
                colorG = rand_uniform(0, 20);
            }
            float colorB = rand_uniform(0, 0);
            float colorA = rand_uniform(200, 255);

            Particle particle(
                location.x, location.y, size, colorR, colorG, colorB, colorA,
                location.x + cos(angle) * distance, location.y + sin(angle) * distance, 0, colorR / 2, colorG / 2, colorB, 0,
                duration, sizeVariation, colorVariation
            );

            addParticle(particle);
        }
    }

    // impactul glont-inamic
    // va face o explozie mica, unghiul va fi gen
    // unghiul din spre care venea glontul += rand(-30, 30)
    void impact(Vector2f location, float size, float angle, unsigned int duration = 60, unsigned int count = 30) {
        for (unsigned int i = 0; i < count; i++) {
            float colorVariation = rand_uniform(0, 3);
            float sizeVariation = rand_uniform(0, 1);

            float colorR, colorG, colorB, colorA;
            if (i%3 == 0) { 
                // galben
                colorR = rand_uniform(100, 155);
                colorG = rand_uniform(100, 155);
                colorB = rand_uniform(0, 20);
            } else if (i%3 == 1) {
                // rosu
                colorR = rand_uniform(100, 155);
                colorG = rand_uniform(0, 20);
                colorB = rand_uniform(0, 20);
            } else {
                // gri (ca sa fie de culoarea inamicului
                colorR = rand_uniform(100, 155);
                colorG = colorR;
                colorB = colorR;
            }
            colorA = rand_uniform(200, 255);

            Particle particle(
                location.x, location.y, size, colorR, colorG, colorB, colorA,
                location.x + cos(angle + rand_uniform(-30, 30) * 3.14159 / 180) * size * 10, location.y + sin(angle + rand_uniform(-30, 30) * 3.14159 / 180) * size * 10, 0, colorR / 2, colorG / 2, colorB, 0,
                duration, sizeVariation, colorVariation
            );

            addParticle(particle);
        }
    }

    void playerImpact(Vector2f location, float size, float angle, unsigned int duration = 60, unsigned int count = 30) {
        // la fel doar ca mai mic, 50% sanse sa fie alb si 50% rosu
        for (unsigned int i = 0; i < count; i++) {
            float colorVariation = rand_uniform(0, 3);
            float sizeVariation = rand_uniform(0, 1);

            float colorR, colorG, colorB, colorA;
            if (i%2 == 0) {
                // alb
                colorR = rand_uniform(200, 255);
                colorG = rand_uniform(200, 255);
                colorB = rand_uniform(200, 255);
            } else {
                // rosu
                colorR = rand_uniform(200, 255);
                colorG = rand_uniform(0, 20);
                colorB = rand_uniform(0, 20);
            }
            colorA = rand_uniform(200, 255);

            Particle particle(
                location.x, location.y, size, colorR, colorG, colorB, colorA,
                location.x + cos(angle + rand_uniform(-30, 30) * 3.14159 / 180) * size * 10, location.y + sin(angle + rand_uniform(-30, 30) * 3.14159 / 180) * size * 10, 0, colorR / 2, colorG / 2, colorB, 0,
                duration, sizeVariation, colorVariation
            );

            addParticle(particle);
        }
    }

    void shieldBreak(unsigned int duration = 30, unsigned int count = 100) {
        // albastre, in centru, se strica shieldul
        for (unsigned int i = 0; i < count; i++) {
            float colorVariation = rand_uniform(0, 3);
            float sizeVariation = rand_uniform(0, 5);

            float colorR, colorG, colorB, colorA;
            colorR = rand_uniform(0, 20);
            colorG = rand_uniform(0, 20);
            colorB = rand_uniform(200, 255);
            colorA = rand_uniform(200, 255);

            Particle particle(
                400, 300, 5, colorR, colorG, colorB, colorA,
                400 + cos(rand_uniform(0, 360) * 3.14159 / 180) * 100, 300 + sin(rand_uniform(0, 360) * 3.14159 / 180) * 100, 0, colorR / 2, colorG / 2, colorB, 0,
                duration, sizeVariation, colorVariation
            );

            addParticle(particle);
        }
    }

    void bulletReflect(Vector2f location, float size, float angle, unsigned int duration = 60, unsigned int count = 10) {
        // verzi si mici, apar la margine cand se reflecta
        // gloantele care au boomerang
        for (unsigned int i = 0; i < count; i++) {
            float colorVariation = rand_uniform(0, 3);
            float sizeVariation = rand_uniform(0, 1);

            float colorR, colorG, colorB, colorA;
            colorR = rand_uniform(0, 20);
            colorG = rand_uniform(200, 255);
            colorB = rand_uniform(0, 20);
            colorA = rand_uniform(200, 255);

            Particle particle(
                location.x, location.y, size, colorR, colorG, colorB, colorA,
                location.x + cos(angle + rand_uniform(-15, 15) * 3.14159 / 180) * size * 10, location.y + sin(angle + rand_uniform(-15, 15) * 3.14159 / 180) * size * 10, 0, colorR / 2, colorG / 2, colorB, 0,
                duration, sizeVariation, colorVariation
            );

            addParticle(particle);
        }
    }

    void enemyShieldImpact(Vector2f location, float size, float angle, unsigned int duration = 60, unsigned int count = 30) {
        // la fel doar ca mai mic, 50% sanse sa fie alb si 50% rosu
        for (unsigned int i = 0; i < count; i++) {
            float colorVariation = rand_uniform(0, 3);
            float sizeVariation = rand_uniform(0, 1);

            float colorR, colorG, colorB, colorA;
            if (i%5 == 0) {
                // rosu
                colorR = rand_uniform(200, 255);
                colorG = rand_uniform(0, 0);
                colorB = rand_uniform(0, 0);
            } else {
                // albastru
                colorR = rand_uniform(0, 20);
                colorG = rand_uniform(0, 20);
                colorB = rand_uniform(0, 255);
            }
            colorA = rand_uniform(200, 255);

            Particle particle(
                location.x, location.y, size, colorR, colorG, colorB, colorA,
                location.x + cos(angle + rand_uniform(-80, 80) * 3.14159 / 180) * size * 6, location.y + sin(angle + rand_uniform(-80, 80) * 3.14159 / 180) * size * 6, 0, colorR / 2, colorG / 2, colorB, 0,
                duration, sizeVariation, colorVariation
            );

            addParticle(particle);
        }
    }

    void pickup(Vector2f location, float startSize) {
        startSize = startSize / rand_uniform(3, 6); // sa fie mai mic ca 10 too big

        // particule albe. cateva in sus, cateva in jos, cateva in stanga, cateva in dreapta
        auto angle1 = rand_uniform(-30, 30) * 3.14159 / 180 + 3.14159;
        auto angle2 = rand_uniform(-30, 30) * 3.14159 / 180 + 3.14159 / 2;
        auto angle3 = rand_uniform(-30, 30) * 3.14159 / 180;
        auto angle4 = rand_uniform(-30, 30) * 3.14159 / 180 - 3.14159 / 2;

        for (unsigned int i = 0; i < 10; i++) {
            float colorVariation = rand_uniform(0, 3);
            float sizeVariation = rand_uniform(0, 1);

            float colorR, colorG, colorB, colorA;
            colorR = rand_uniform(200, 255);
            colorG = rand_uniform(200, 255);
            colorB = rand_uniform(200, 255);
            colorA = rand_uniform(200, 255);

            Particle particle(
                location.x, location.y, startSize, colorR, colorG, colorB, colorA,
                location.x + cos(angle1) * startSize * 10, location.y + sin(angle1) * startSize * 10, 0, colorR / 2, colorG / 2, colorB, 0,
                60, sizeVariation, colorVariation
            );

            addParticle(particle);

            particle = Particle(
                location.x, location.y, startSize, colorR, colorG, colorB, colorA,
                location.x + cos(angle2) * startSize * 10, location.y + sin(angle2) * startSize * 10, 0, colorR / 2, colorG / 2, colorB, 0,
                60, sizeVariation, colorVariation
            );

            addParticle(particle);

            particle = Particle(
                location.x, location.y, startSize, colorR, colorG, colorB, colorA,
                location.x + cos(angle3) * startSize * 10, location.y + sin(angle3) * startSize * 10, 0, colorR / 2, colorG / 2, colorB, 0,
                60, sizeVariation, colorVariation
            );

            addParticle(particle);

            particle = Particle(
                location.x, location.y, startSize, colorR, colorG, colorB, colorA,
                location.x + cos(angle4) * startSize * 10, location.y + sin(angle4) * startSize * 10, 0, colorR / 2, colorG / 2, colorB, 0,
                60, sizeVariation, colorVariation
            );

            addParticle(particle);
        }
    }

    void coinPickup(Vector2f location) {
        // paticule galbene, se duc in toate directiile
        // devin albe si mici (initial au 5px)
        // se spawneaza intre 3-10 (ca s prea multi coinsi si poate cauza lag)
        for (unsigned int i = 0; i < rand() % 7 + 3; i++) {
            float colorVariation = rand_uniform(0, 3);
            float sizeVariation = rand_uniform(0, 1);

            float colorR, colorG, colorB, colorA;
            colorR = rand_uniform(200, 255);
            colorG = rand_uniform(200, 255);
            colorB = rand_uniform(0, 20);
            colorA = rand_uniform(200, 255);

            Particle particle(
                location.x, location.y, 5, colorR, colorG, colorB, colorA,
                location.x + cos(rand_uniform(0, 360) * 3.14159 / 180) * 50, location.y + sin(rand_uniform(0, 360) * 3.14159 / 180) * 50, 0, colorR / 2, colorG / 2, colorB, 0,
                30, sizeVariation, colorVariation
            );

            addParticle(particle);
        }
    }

    ~particleEngine() {
        // nimic pentru ca nu a fost alocat nimic dinamic
    }

    // si constructoru cu toti param (e doar unu oricum)
    particleEngine(vector<Particle> particles) {
        this->particles = particles;
    }
} particleEngine;

class Bullet {
private:
    // proprietati de baza ########
    float x, y;
    float angle;
    bool friendly; // daca e true, e trasa de player, daca e false, e trasa de inamici
    bool farAway = false; // pt stergere 
    bool hitSomething = false; // pt stergere
    unsigned int reflectii = 0; // contor = de cate ori a atins marginea
    unsigned int cloneCount = 0; // contor cate split-uri a facut
    Vector2f target;
    // proprietati schimbabile prin upgrade-uri ########
    float damage = rand_uniform(9, 11);
    float speed = rand_uniform(8, 12); // viteza random (intre 8 si 12)
    bool boomerang = false;
    bool freeze = false; // daca e true, va fi sticky in target
    bool zigzag = false; // daca e true, va face zigzag (nu e zigzag dar se misca 90 grade random)
    bool splitter = false; // daca e true, se va imparti randomly in mai multe gloante
    bool remoteControl = false; // daca e true, va fi controlat de mouse inclusiv cand e trasa
    bool homing = false; // daca e true, va merge catre inamici daca sunt < 200px
    bool gravitational = false; // daca e true, va atrage inamicii
    bool isAlien = false; // daca e true, va fi trasa de extraterestrii (si va fi albastra)
public:
    // constructoru va avea 5 parametrii
    // de unde se trage si spre ce se trage + daca e friendly
    // acestia sunt singurii parametrii necesari
    Bullet(float x, float y, float targetX, float targetY, bool friendly) {
        this->x = x;
        this->y = y;
        this->friendly = friendly;
        angle = atan2(targetY - y, targetX - x);
        target = Vector2f(targetX, targetY);

        // restu default
        farAway = false;
        hitSomething = false;
        reflectii = 0;
        cloneCount = 0;
        damage = rand_uniform(9, 11);
        speed = rand_uniform(8, 12);
        boomerang = false;
        freeze = false;
        zigzag = false;
        splitter = false;
        remoteControl = false;
        homing = false;
        gravitational = false;
        isAlien = false;
    }

    void move() {
        // se misca in directia unghiului
        x += cos(angle) * speed;
        y += sin(angle) * speed;

        // dar trebuie sa se miste si in functie de player
        // daca playeru se misca, gloantele trebuie sa se miste
        // in directia opusa
        x -= playerVelocityX;
        y -= playerVelocityY;
        target.x -= playerVelocityX;
        target.y -= playerVelocityY;
        // ^^^ totusi asta taie din realism deoarece gloantele
        // au inertie si nu exista aer in spatiu

        // daca iese din ecran (+ marginea de 100px, se marcheaza pt stergere)
        if (x < -100 || x > 900 || y < -100 || y > 700) {
            farAway = true;
        }

        // daca este boomerang, cand va lovi o margine
        // va fi reflectat oarecum
        if (boomerang && reflectii < 5) {
            if (x < 0 || x > 800) {
                angle = 3.14159 - angle;
                reflectii++;
                particleEngine.bulletReflect(Vector2f(x, y), 5, angle);
            }
            if (y < 0 || y > 600) {
                angle = -angle;
                reflectii++;
                particleEngine.bulletReflect(Vector2f(x, y), 5, angle);
            }
        }

        // se va misca catre target MEREU si sa va opri la un 
        // moment dat deci e mai probabil ca inamicii
        // sa se loveasca de ea
        if (freeze) {
            x += (target.x - x) / 7;
            y += (target.y - y) / 7;
        }

        // daca e zigzag
        if (zigzag) {
            if (probability_second(.3)) { // cam 3 ori pe secunda
                if (rand() % 2) {
                    // turn de -90 grade
                    angle -= 3.14159 / 2;
                } else {
                    // turn de 90 grade
                    angle += 3.14159 / 2;
                }
            }
        }
    }

    void draw(RenderWindow &window) {
        // va fi o linie
        // se va genera usor ca stim
        // unde va arata pozitia
        // si unghiul
        auto from = Vector2f(x, y);
        auto to = Vector2f(x + cos(angle) * 15, y + sin(angle) * 15);

        // daca e friendly, e verde deschis
        // daca nu, e rosie
        if (friendly) {
            // e un rectangle rotit, lungime 15, latime 5
            // se va roti in functie de unghi
            // si va avea culoarea verde (gradient)
            RectangleShape bullet(Vector2f(15, 3));
            bullet.setFillColor(Color(0, 255, 0));
            bullet.setOrigin(0, 1.5);
            bullet.setPosition(x, y);
            bullet.setRotation(angle * 180 / 3.14159);
            window.draw(bullet);
        } else if (isAlien) {
            RectangleShape bullet(Vector2f(15, 3));
            bullet.setFillColor(Color(71, 71, 255));
            bullet.setOrigin(0, 1.5);
            bullet.setPosition(x, y);
            bullet.setRotation(angle * 180 / 3.14159);
            window.draw(bullet);
        } else {
            RectangleShape bullet(Vector2f(15, 3));
            bullet.setFillColor(Color(255, 0, 0));
            bullet.setOrigin(0, 1.5);
            bullet.setPosition(x, y);
            bullet.setRotation(angle * 180 / 3.14159);
            window.draw(bullet);
        }
    }

    // verifica daca e prea departe sau a lovit ceva
    bool needsDeletion() {
        return farAway || hitSomething;
    }

    // verifica daca accepta split
    bool canBeCloned() {
        return splitter && (cloneCount < NUMAR_CLONE_MAXIM);
    }

    // self explanatory
    void setAngle(float newAngle, bool relative = false) {
        if (relative) {
            angle += newAngle;
        } else {
            angle = newAngle;
        }
    }

    // necesara pentru clone, sa nu intre intr-o bucla infinita
    void turnOffSplitter() {
        splitter = false;
    }

    // setter esential
    // pentru clonele de gloante (mai ales cand e freezer)
    void setTarget(Vector2f newTarget) {
        target = newTarget;
    }

    // operatorul ++ va creste nr de clone
    // si va returna referinta la obiect
    friend Bullet &operator++(Bullet &b) {
        b.cloneCount++;
        return b;
    }

    // bullet = vector va fi operatorul pentru a da target
    // in care se va misca bulletul
    Bullet& operator=(const Vector2f& target) {
        this->target = target;
        this->angle = atan2(target.y - y, target.x - x);
        return *this;
    }

    // bullet += va adauga un vector la target
    // si va schimba unghiul
    Bullet& operator+=(const Vector2f& target) {
        this->target += target;
        this->angle = atan2(this->target.y - y, this->target.x - x);
        return *this;
    }

    bool canControl() {
        return remoteControl;
    }

    bool isGravitational() {
        return gravitational;
    }

    Vector2f location() {
        return Vector2f(x, y);
    }

    void hit() {
        hitSomething = true;
    }

    float getAngle() {
        return angle;
    }

    float getDamage() {
        return damage;
    }

    void setDamage(float damage) {
        this->damage = damage;
    }

    bool isHoming() {
        return homing;
    }

    // PENTRU UPGRADES PLAYER

    void setHoming(bool homing) {
        this->homing = homing;
    }

    void setGravitational(bool gravitational) {
        this->gravitational = gravitational;
    }

    void setFreeze(bool freeze) {
        this->freeze = freeze;
    }

    void setBoomerang(bool boomerang) {
        this->boomerang = boomerang;
    }

    void setZigzag(bool zigzag) {
        this->zigzag = zigzag;
    }

    void setSplitter(bool splitter) {
        this->splitter = splitter;
    }

    void setRemoteControl(bool remoteControl) {
        this->remoteControl = remoteControl;
    }

    // pt EXTRATERESTRII ONLY

    void setAlien() {
        friendly = false;
        isAlien = true;
    }

    // constructorii esentiali

    // copy constructor
    Bullet(const Bullet &b) {
        x = b.x;
        y = b.y;
        angle = b.angle;
        friendly = b.friendly;
        farAway = b.farAway;
        hitSomething = b.hitSomething;
        reflectii = b.reflectii;
        cloneCount = b.cloneCount;
        target = b.target;
        damage = b.damage;
        speed = b.speed;
        boomerang = b.boomerang;
        freeze = b.freeze;
        zigzag = b.zigzag;
        splitter = b.splitter;
        remoteControl = b.remoteControl;
        homing = b.homing;
        gravitational = b.gravitational;
        isAlien = b.isAlien;
    }

    // operatorul de atribuire 
    Bullet& operator=(const Bullet &b) {
        x = b.x;
        y = b.y;
        angle = b.angle;
        friendly = b.friendly;
        farAway = b.farAway;
        hitSomething = b.hitSomething;
        reflectii = b.reflectii;
        cloneCount = b.cloneCount;
        target = b.target;
        damage = b.damage;
        speed = b.speed;
        boomerang = b.boomerang;
        freeze = b.freeze;
        zigzag = b.zigzag;
        splitter = b.splitter;
        remoteControl = b.remoteControl;
        homing = b.homing;
        gravitational = b.gravitational;
        isAlien = b.isAlien;
        return *this;
    }

    // si cel default (nu trebuie apelat vreodata)
    Bullet() {
        x = 0;
        y = 0;
        angle = 0;
        friendly = true;
        farAway = false;
        hitSomething = false;
        reflectii = 0;
        cloneCount = 0;
        target = Vector2f(0, 0);
        damage = rand_uniform(9, 11);
        speed = rand_uniform(8, 12);
        boomerang = false;
        freeze = false;
        zigzag = false;
        splitter = false;
        remoteControl = false;
        homing = false;
        gravitational = false;
        isAlien = false;
        cerr << "Default constructor shouldn't be called for Bullet\n";
    }

    // si cel cu toti param (nu va fi folosit)
    Bullet(float x, float y, float angle, bool friendly, bool farAway, bool hitSomething, unsigned int reflectii, unsigned int cloneCount, Vector2f target, float damage, float speed, bool boomerang, bool freeze, bool zigzag, bool splitter, bool remoteControl, bool homing, bool gravitational, bool isAlien) {
        this->x = x;
        this->y = y;
        this->angle = angle;
        this->friendly = friendly;
        this->farAway = farAway;
        this->hitSomething = hitSomething;
        this->reflectii = reflectii;
        this->cloneCount = cloneCount;
        this->target = target;
        this->damage = damage;
        this->speed = speed;
        this->boomerang = boomerang;
        this->freeze = freeze;
        this->zigzag = zigzag;
        this->splitter = splitter;
        this->remoteControl = remoteControl;
        this->homing = homing;
        this->gravitational = gravitational;
        this->isAlien = isAlien;
    }

    // si operatoru << pt info legat de gloante
    // si upgrade-uri
    friend ostream& operator<<(ostream &os, const Bullet &b) {
        os << "Bullet: " << endl;
        os << "Position: " << b.x << " " << b.y << endl;
        os << "Angle: " << b.angle << endl;
        os << "Friendly: " << b.friendly << endl;
        os << "Far Away: " << b.farAway << endl;
        os << "Hit Something: " << b.hitSomething << endl;
        os << "Reflectii: " << b.reflectii << endl;
        os << "Clone Count: " << b.cloneCount << endl;
        os << "Target: " << b.target.x << " " << b.target.y << endl;
        os << "Damage: " << b.damage << endl;
        os << "Speed: " << b.speed << endl;
        os << "Boomerang: " << b.boomerang << endl;
        os << "Freeze: " << b.freeze << endl;
        os << "Zigzag: " << b.zigzag << endl;
        os << "Splitter: " << b.splitter << endl;
        os << "Remote Control: " << b.remoteControl << endl;
        os << "Homing: " << b.homing << endl;
        os << "Gravitational: " << b.gravitational << endl;
        os << "Is Alien: " << b.isAlien << endl;
        return os;
    }

    // ----------------------------

    ~Bullet() {
        // nimic pentru ca nu a fost alocat nimic dinamic
    }
};

// lowkey e strict pt lizibilitate si 
// pentru a folosi mai mult OOP
// puteam sa fac tot global 
class Player {
private:
    // stringul de upgrade-uri
    // va fi un vector de string-uri
    vector<string> upgrades;

    float damage = 10; // va fi aproximat
    float health = 100;
    float shield = 50;
    float fuel = 300;
    float maxFuel = 300;
    unsigned int fireRate = 18; // how many frames between shots
public:
    // constructoru
    Player() {
        // nimic inca // TODO #######
    }

    // constructoru cu toti param (nu va fi nevoie)
    Player(float health, float shield, float fuel, float maxFuel, unsigned int fireRate) {
        this->health = health;
        this->shield = shield;
        this->fuel = fuel;
        this->maxFuel = maxFuel;
        this->fireRate = fireRate;
    }

    // add upgrade
    void upgrade(string upgrade) {
        upgrades.push_back(upgrade);
    }

    // remove upgrade
    void downgrade(string upgrade) {
        for (int i = 0; i < upgrades.size(); i++) {
            if (upgrades[i] == upgrade) {
                upgrades.erase(upgrades.begin() + i);
                break;
            }
        }
    }

    // check daca are upgrade
    bool hasUpgrade(string upgrade) {
        for (auto &u : upgrades) {
            if (u == upgrade) {
                return true;
            }
        }
        return false;
    }

    // trage catre pozitie
    void shoot(float x, float y, vector<Bullet> &bullets) {
        if (firePause > 0) {
            return; // se blocheaza tragerea cand firerate-ul e mai mare decat 0
        }
        bullets.push_back(Bullet(400, 300, x, y, true));

        // se adauga upgrade-urile la gloante
        if (this->hasUpgrade("boomerang")) {
            bullets.back().setBoomerang(true);
        }
        if (this->hasUpgrade("freeze")) {
            bullets.back().setFreeze(true);
        }
        if (this->hasUpgrade("zigzag")) {
            bullets.back().setZigzag(true);
        }
        if (this->hasUpgrade("splitter")) {
            bullets.back().setSplitter(true);
        }
        if (this->hasUpgrade("remote_control")) {
            bullets.back().setRemoteControl(true);
        }
        if (this->hasUpgrade("homing")) {
            bullets.back().setHoming(true);
        }
        if (this->hasUpgrade("gravitational")) {
            bullets.back().setGravitational(true);
        }

        // se adauga damageul la gloante (CEL AL PLAYERULUI)
        bullets.back().setDamage(damage);

        firePause = fireRate;

        // sunetul de laser
        soundManager.playSound("enemy_laser");
    }

    // getterele
    float getHealth() {
        return health;
    }

    float getShield() {
        return shield;
    }

    float getFuel() {
        return fuel;
    }

    float getMaxFuel() {
        return maxFuel;
    }

    float getDamage() {
        return damage;
    }

    unsigned int getFireRate() {
        return fireRate;
    }

    // setterele
    void setHealth(float health) {
        this->health = health;
    }

    void setShield(float shield) {
        this->shield = shield;
    }

    void setFuel(float fuel) {
        this->fuel = fuel;
    }

    void setMaxFuel(float maxFuel) {
        this->maxFuel = maxFuel;
    }

    void setDamage(float damage) {
        this->damage = damage;
    }

    void setFireRate(unsigned int fireRate) {
        this->fireRate = fireRate;
    }

    // void setX(float paramx) {
    //     x = paramx;
    // }

    // void setY(float paramy) {
    //     y = paramy;
    // }

    // operatoru << va afisa toate atributele din clasa
    friend ostream& operator<<(ostream &os, const Player &p) {
        os << "Player: " << endl;
        os << "Health: " << p.health << endl;
        os << "Shield: " << p.shield << endl;
        os << "Fuel: " << p.fuel << endl;
        os << "Max Fuel: " << p.maxFuel << endl;
        os << "Damage: " << p.damage << endl;
        os << "Fire Rate: " << p.fireRate << endl;
        os << "Upgrades: " << endl;
        for (auto &u : p.upgrades) {
            os << u << endl;
        }
        return os;
    }

    // dau clear la upgrade-uri chiar daca se face automat probabil...
    ~Player() {
        upgrades.clear();
    }
};

class Enemy {
private:
    float x, y;
    float vx = 0, vy = 0; // velocitatile
    float speed = 0.1;
    float health = 50;
    float maxHealth = 50;
    float damage = 7;
    float shield = 25; // max 100
    float size = rand_uniform(18,22);
    float distantaMinima = rand_uniform(100, 300);
    unsigned int fireRate = 30;
    unsigned int firePause = rand_uniform(0, 60);; // de data asta e unic pt fiecare element, nu e ca la player
    bool regenerates = true;
    bool shooting = false;
    bool difficult = false;

    // dupa acest id se va genera procedural aspectul va fi pus in formule random
    unsigned int uniqueID = rand_uniform(0, 666);
    bool deleteMe = false;
public:
    // constr default
    Enemy() {
        x = rand_uniform(0, 800);
        y = rand_uniform(0, 600);
        vx = 0;
        vy = 0;
        health = 50;
        maxHealth = 50;
        damage = 7;
        shield = 25;
        size = rand_uniform(18,22);
        distantaMinima = rand_uniform(100, 300);
        fireRate = 30;
        firePause = rand_uniform(0, 60);
        regenerates = true;
        shooting = false;

        uniqueID = rand_uniform(0, 666);
        deleteMe = false;
    }

    // constr esential
    Enemy(unsigned x, unsigned y) {
        this->x = x;
        this->y = y;

        // restu default
        vx = 0;
        vy = 0;
        health = 50;
        maxHealth = 50;
        damage = 7;
        shield = 25;
        size = rand_uniform(18,22);
        distantaMinima = rand_uniform(100, 300);
        fireRate = 30;
        firePause = rand_uniform(0, 60);
        regenerates = true;
        shooting = false;
        difficult = false;
        uniqueID = rand_uniform(0, 666);
        deleteMe = false;
    }

    // constr cu toti param (nu va fi folosit)
    Enemy(unsigned x, unsigned y, float health, float maxHealth, float damage, float shield, float size, float distantaMinima, unsigned int fireRate, unsigned int firePause, bool regenerates, bool shooting) {
        this->x = x;
        this->y = y;
        this->health = health;
        this->maxHealth = maxHealth;
        this->damage = damage;
        this->shield = shield;
        this->size = size;
        this->distantaMinima = distantaMinima;
        this->fireRate = fireRate;
        this->firePause = firePause;
        this->regenerates = regenerates;
        this->shooting = shooting;
    }

    void draw(RenderWindow &window) {
        // uniqueID va fi folosit pentru determinism
        float cornerOneAngle, cornerTwoAngle, cornerThreeAngle, cornerFourAngle;
        Vector2f cornerOne, cornerTwo, cornerThree, cornerFour;
        for (int i = 10; i--;) {
            // sunt 10 componente ale inamiclui

            // se va folosi uniqueID pentru a genera aspectul
            // inamicului in sensu ca la fiecare iteratie
            // se va desena un poligon cu o culoare random
            // in functie de uniqueID (gri).

            // colturile la rectangle
            auto color = sin(uniqueID * 4 - i * uniqueID) * (255 - 20) + 20;
            cornerOneAngle = sin(uniqueID * 8 + i) * 3.14159 * 7;
            cornerTwoAngle = cos(uniqueID * 8 + i) * 3.14159 * 7;
            cornerOne = Vector2f(x + cos(cornerOneAngle) * size, y + sin(cornerOneAngle) * size);
            cornerTwo = Vector2f(x + cos(cornerTwoAngle) * size, y + sin(cornerTwoAngle) * size);

            // draw rectangle from cornerOne to cornerTwo
            RectangleShape rectangle(Vector2f(cornerTwo.x - cornerOne.x, cornerTwo.y - cornerOne.y));
            rectangle.setFillColor(Color(color, color, color));
            rectangle.setPosition(cornerOne);
            // daca e modu difficult, se face outlineul rosu
            if (difficult) {
                rectangle.setOutlineColor(Color(255, 0, 0));
                rectangle.setOutlineThickness(1);
            } else {
                rectangle.setOutlineColor(Color(0, 0, 0));
                rectangle.setOutlineThickness(1);
            }
            window.draw(rectangle);
        }

        // desenarea health barului sub el
        // marimea va fi rosie mereu dar dimensiuinea
        // va fi in functie de health
        RectangleShape healthBar(Vector2f(size * 2, 5));
        healthBar.setFillColor(Color::Red);
        healthBar.setPosition(x - size, y + size);
        healthBar.setSize(Vector2f(size * health / maxHealth * 2, 5));
        window.draw(healthBar);
        // + outlineu lui
        healthBar.setFillColor(Color::Transparent);
        healthBar.setOutlineColor(Color(100,0,0));
        healthBar.setOutlineThickness(1);
        healthBar.setSize(Vector2f(size * 2, 5));
        window.draw(healthBar);

        // shieldul va fi un cerc in jurul inamicului
        // opacitatea lui (alpha) va fi in functie de shield
        // e in functie de shield, exact ca la player
        CircleShape shieldCircle(size + 10);
        shieldCircle.setFillColor(Color(0, 0, 255, 5));
        shieldCircle.setOrigin(size + 10, size + 10);
        shieldCircle.setPosition(x, y);
        shieldCircle.setOutlineColor(Color(0, 0, 255, shield*2));
        shieldCircle.setOutlineThickness(2);
        window.draw(shieldCircle);
    }

    void move() { // lowkey e si update() 
        // se va misca catre player
        // dar si in functie de player
        // daca playeru se misca, inamicul se va misca
        // in directia opusa
        x -= playerVelocityX;
        y -= playerVelocityY;

        // PROTOTIP
        x += cos(atan2(300 - y, 400 - x)) / 10;
        y += sin(atan2(300 - y, 400 - x)) / 10;

        x += vx;
        y += vy;

        // miscarea catre player nu va avea aceeasi
        // regula ca la gravitatia folosita pana acum
        // (cea cu atan2) deoarece modul asta
        // va face miscarea inamicului mai naturala
        
        // daca e prea aproape de player
        // se da inapoi
        if (distance(Vector2f(x, y), Vector2f(400, 300)) < distantaMinima) {
            auto angle = atan2(y - 300, x - 400);
            vx += cos(angle) / 10;
            vy += sin(angle) / 10;
        } else {
            if (x < 400) vx += speed;
            if (x > 400) vx -= speed;
            if (y < 300) vy += speed;
            if (y > 300) vy -= speed;
        }

        // daca ii vine lui la intamplare, se duce intr-o
        // pozitie random, (dodge)
        if (probability_second(2)) { // adica la fiecare 3 secunde aprox
            auto tox = rand_uniform(0, 800);
            auto toy = rand_uniform(0, 600);

            auto angle = atan2(toy - y, tox - x);

            vx += cos(angle) / 3;
            vy += sin(angle) / 3;
        }

        // daca e prea departe, va veni in viteza inapoi
        if (distance(Vector2f(x, y), Vector2f(400, 300)) > 450) {
            auto angle = atan2(y - 300, x - 400); 

            vx -= cos(angle) * 2;
            vy -= sin(angle) * 2;
        }

        // tragerea gloantelor
        if (firePause > 0) {
            firePause--;
        } else {
            if (probability_second(0.5)) { // adica la fiecare secunda
                // se trage
                firePause = fireRate;
                shoot();
            }
        }

        // regenerarea lui
        if (regenerates) {
            health += 0.02;
            if (health > maxHealth) health = maxHealth;
        }

        // daca moare explodeaza
        if (health < 1) {
            particleEngine.explosion(Vector2f(x, y), size);
            deleteMe = true;
        }
    }

    void shoot() {
        // se adauga un glont UNFRIENDLY in vectorul de gloante de data asta
        firePause = fireRate;
        shooting = true;
    }

    bool isShooting() {
        return shooting;
    }

    void stopShooting() {
        shooting = false;
    }

    Vector2f location() {
        return Vector2f(x, y);
    }

    // se misca fortat spre target 
    void forcedMoveTowards(Vector2f target) {
        auto angle = atan2(target.y - y, target.x - x);
        x += cos(angle);
        y += sin(angle);
    }

    void moveTowards(Vector2f target) {
        auto angle = atan2(target.y - y, target.x - x);
        vx += cos(angle) / 5;
        vy += sin(angle) / 5;
    }

    // functia prin care ia damage
    void getHit(float damage) {
        if (shield > 0) {
            shield -= damage;
            if (shield < 0) {
                health += shield;
                shield = 0;
            }
        } else {
            health -= damage;
        }
    }

    float getHealth() {
        return health;
    }

    float getShield() {
        return shield;
    }

    float getDamage() {
        return damage;
    }

    float getSize() {
        return size;
    }

    float getX() {
        return x;
    }

    float getY() {
        return y;
    }

    bool getDifficult() {
        return difficult;
    }

    float getMaxHealth() {
        return maxHealth;
    }

    bool needsDeletion() {
        return deleteMe;
    }

    void setHealth(float health) {
        this->health = health;
    }

    void setShield(float shield) {
        this->shield = shield;
    }

    void setDamage(float damage) {
        this->damage = damage;
    }

    void setDifficult(bool difficult) {
        this->difficult = difficult;
    }

    void setMaxHealth(float maxHealth) {
        this->maxHealth = maxHealth;
    }

    // operatorul += pentru a adauga un vector la pozitia inamicului
    Vector2f operator+=(Vector2f v) {
        x += v.x;
        y += v.y;
        return Vector2f(x, y);
    }

    // operatorul *= nu e pentru inmultire, ci pentru a adauga la velocitatea lui
    Vector2f operator*=(Vector2f v) {
        vx += v.x;
        vy += v.y;
        return Vector2f(vx, vy);
    }

    // operatorul << afiseaza tot infou despre inamic
    friend ostream& operator<<(ostream &os, const Enemy &e) {
        os << "Enemy at " << e.x << ", " << e.y << " with " << e.health << " health and " << e.shield << " shield\n";
        os << "Damage: " << e.damage << " Size: " << e.size << " Distance: " << e.distantaMinima << " FireRate: " << e.fireRate << " FirePause: " << e.firePause << "\n";
        os << "Regenerates: " << e.regenerates << " Shooting: " << e.shooting << " Difficult: " << e.difficult << "\n";
        return os;
    }

    ~Enemy() {
        // nimic pt ca nu e nimic dinamic
    }
};

// tipuri de pickup:
// - small shield - ofera +25 de shield
// - big shield - ofera +70 de shield
// - small health - ofera +25 de health
// - big health - ofera +50 de health
// - medkit - ofera +100 de health 
// - fuel - ofera +100 de fuel
// - better_gun - ofera +3 la damage
// - faster_shooting - ofera -3 la fireRate
// - boomerang - ofera boomerang la gloante
// - freeze - ofera freeze la gloante
// - zigzag - ofera zigzag la gloante
// - splitter - ofera splitter la gloante
// - remote_control - ofera remote control la gloante
// - homing - ofera homing la gloante
// - gravitational - ofera gravitational la gloante
// clasa Pickup nu va fi decat pentru a fi mostenita
class Pickup {
private:
    float x, y;
    float size = 10;
    string type;
    bool deleteMe = false;
    float speed = 1;
public:
    Vector2f location() {
        return Vector2f(x, y);
    }

    Pickup(float x, float y, string type) {
        this->x = x;
        this->y = y;
        this->type = type;
        this->deleteMe = false;
        this->size = 10;
    }

    // all params constructor
    Pickup(float x, float y, string type, float size, bool deleteMe, float speed) {
        this->x = x;
        this->y = y;
        this->type = type;
        this->deleteMe = deleteMe;
        this->size = size;
        this->speed = speed;
    }

    // copy constructoru esential
    Pickup(const Pickup &p) {
        x = p.x;
        y = p.y;
        size = p.size;
        type = p.type;
        deleteMe = p.deleteMe;
    }

    // default constructoru
    Pickup() {
        x = 0;
        y = 0;
        size = 10;
        type = "unknown";
        deleteMe = false;
    }

    // operatorul de atribuire esential
    Pickup& operator=(const Pickup &p) {
        x = p.x;
        y = p.y;
        size = p.size;
        type = p.type;
        deleteMe = p.deleteMe;
        return *this;
    }

    // desenarea va fi different pt clasele mostenite
    virtual void draw(RenderWindow &w) {
        // cerc gol, de marimea size
        // cu culoarea alba
        CircleShape circle(size);
        circle.setFillColor(Color::Transparent);
        circle.setOutlineColor(Color::White);
        circle.setOutlineThickness(1);
        circle.setOrigin(size, size);
        circle.setPosition(x, y);
        w.draw(circle);
    }

    void move() {
        // misca pickup (inertia)
        setY(getY() - playerVelocityY);
        setX(getX() - playerVelocityX);

        // e atras subtil de player
        float angle = atan2(300 - y, 400 - x);
        setX(getX() + cos(angle) / 2 * speed);
        setY(getY() + sin(angle) / 2 * speed);
    }

    // getterele

    string getType() {
        return type;
    }

    bool needsDeletion() {
        return deleteMe || (getX() < -100 || getX() > 900 || getY() < -100 || getY() > 700); // daca e prea departe de centru (optimizare)
    }

    float getX() {
        return x;
    }

    float getY() {
        return y;
    }

    // setterele

    void setDeletion(bool deletion) {
        deleteMe = deletion;
    }

    void setX(float x) {
        this->x = x;
    }

    void setY(float y) {
        this->y = y;
    }

    void setSize(float size) {
        this->size = size; // ma indoiesc ca va fi folosita
    }

    void setType(string type) {
        this->type = type;
    }

    void setSpeed(float speed) {
        this->speed = speed;
    }

    float getSize() {
        return size;
    }

    // << va afisa tipul si pozitia
    friend ostream& operator<<(ostream &os, const Pickup &p) {
        os << "Pickup: " << p.type << " at " << p.x << ", " << p.y;
        return os;
    }

    // si functia esentiala:

    virtual void onPickup(Player &player) {
        // daca e mostenita, se va face ceva
        // daca nu, nimic
    }

    virtual ~Pickup() {
        // nimic pt ca nu e nimic dinamic
    }
};

// resource: res/small_shield.png
class pickupSmallShield : public Pickup {
public:
    pickupSmallShield(float x, float y) : Pickup(x, y, "small_shield") {
        // nimic
    }

    void draw(RenderWindow &w) override {
        // se va desena un sprite
        // cu textura small_shield.png
        // la pozitia x, y
        // si cu marimea size
        Texture texture;
        texture.loadFromFile("res/small_shield.png");
        Sprite sprite(texture);
        sprite.setPosition(getX(), getY());
        // are 32x32 px
        sprite.setOrigin(16, 16);
        w.draw(sprite);
    }

    void onPickup(Player &player) override {
        player.setShield(player.getShield() + 25);

        // limit
        if (player.getShield() > 100) {
            player.setShield(100);
        }
    }

    pickupSmallShield() : Pickup() {
        // nimic
    }

    ~pickupSmallShield() {
        // nimic
    }
};

// resource: res/big_shield.png
class pickupBigShield : public Pickup {
public:
    pickupBigShield(float x, float y) : Pickup(x, y, "big_shield") {
        // nimic
    }

    void draw(RenderWindow &w) override {
        // ANALOG
        // se va desena un sprite
        // cu textura big_shield.png
        // la pozitia x, y
        // si cu marimea size
        Texture texture;
        texture.loadFromFile("res/big_shield.png");
        Sprite sprite(texture);
        sprite.setPosition(getX(), getY());
        // are 32x32 px
        sprite.setOrigin(16, 16);
        w.draw(sprite);
    }

    void onPickup(Player &player) override {
        player.setShield(player.getShield() + 80);

        // limit
        if (player.getShield() > 100) {
            player.setShield(100);
        }
    }

    pickupBigShield() : Pickup() {
        // nimic
    }

    ~pickupBigShield() {
        // nimic
    }
};

// resource: res/small_health.png
class pickupSmallHealth : public Pickup {
public:
    pickupSmallHealth(float x, float y) : Pickup(x, y, "small_health") {
        // nimic
    }

    void draw(RenderWindow &w) override {
        // ANALOG
        // se va desena un sprite
        // cu textura small_health.png
        // la pozitia x, y
        // si cu marimea size
        Texture texture;
        texture.loadFromFile("res/small_health.png");
        Sprite sprite(texture);
        sprite.setPosition(getX(), getY());
        // are 32x32 px
        sprite.setOrigin(16, 16);
        w.draw(sprite);
    }

    void onPickup(Player &player) override {
        player.setHealth(player.getHealth() + 25);

        // limit
        if (player.getHealth() > 100) {
            player.setHealth(100);
        }
    }

    pickupSmallHealth() : Pickup() {
        // nimic
    }

    ~pickupSmallHealth() {
        // nimic
    }
};

// resource: res/big_health.png
class pickupBigHealth : public Pickup {
public:
    pickupBigHealth(float x, float y) : Pickup(x, y, "big_health") {
        // nimic
    }

    void draw(RenderWindow &w) override {
        // ANALOG
        // se va desena un sprite
        // cu textura big_health.png
        // la pozitia x, y
        // si cu marimea size
        Texture texture;
        texture.loadFromFile("res/big_health.png");
        Sprite sprite(texture);
        sprite.setPosition(getX(), getY());
        // are 32x32 px
        sprite.setOrigin(16, 16);
        w.draw(sprite);
    }

    void onPickup(Player &player) override {
        player.setHealth(player.getHealth() + 75);

        // limit
        if (player.getHealth() > 100) {
            player.setHealth(100);
        }
    }

    pickupBigHealth() : Pickup() {
        // nimic
    }

    ~pickupBigHealth() {
        // nimic
    }
};

// resource: res/medkit.png
class pickupMedkit : public Pickup {
public:
    pickupMedkit(float x, float y) : Pickup(x, y, "medkit") {
        // nimic
    }

    void draw(RenderWindow &w) override {
        // ANALOG
        // se va desena un sprite
        // cu textura medkit.png
        // la pozitia x, y
        // si cu marimea size
        Texture texture;
        texture.loadFromFile("res/medkit.png");
        Sprite sprite(texture);
        sprite.setPosition(getX(), getY());
        // are 32x32 px
        sprite.setOrigin(16, 16);
        w.draw(sprite);
    }

    void onPickup(Player &player) override {
        player.setHealth(player.getHealth() + 100);

        // limit
        if (player.getHealth() > 100) {
            player.setHealth(100);
        }
    }

    pickupMedkit() : Pickup() {
        // nimic
    }

    ~pickupMedkit() {
        // nimic
    }
};

// resource: res/fuel.png
class pickupFuel : public Pickup {
public:
    pickupFuel(float x, float y) : Pickup(x, y, "fuel") {
        // nimic
    }

    void draw(RenderWindow &w) override {
        // ANALOG
        // se va desena un sprite
        // cu textura fuel.png
        // la pozitia x, y
        // si cu marimea size
        Texture texture;
        texture.loadFromFile("res/fuel.png");
        Sprite sprite(texture);
        sprite.setPosition(getX(), getY());
        // are 32x32 px
        sprite.setOrigin(16, 16);
        w.draw(sprite);
    }

    void onPickup(Player &player) override {
        player.setFuel(player.getFuel() + 100);

        // limit
        if (player.getFuel() > player.getMaxFuel()) {
            player.setFuel(player.getMaxFuel());
        }
    }

    pickupFuel() : Pickup() {
        // nimic
    }

    ~pickupFuel() {
        // nimic
    }
};

// resource: res/better_gun.png
class pickupBetterGun : public Pickup {
public:
    pickupBetterGun(float x, float y) : Pickup(x, y, "better_gun") {
        // nimic
    }

    void draw(RenderWindow &w) override {
        // ANALOG
        // se va desena un sprite
        // cu textura better_gun.png
        // la pozitia x, y
        // si cu marimea size
        Texture texture;
        texture.loadFromFile("res/better_gun.png");
        Sprite sprite(texture);
        sprite.setPosition(getX(), getY());
        // are 32x32 px
        sprite.setOrigin(16, 16);
        w.draw(sprite);
    }

    void onPickup(Player &player) override {
        // creste damageu playerului cu 2-3
        player.setDamage(player.getDamage() + rand_uniform(2, 3));
    }

    pickupBetterGun() : Pickup() {
        // nimic
    }

    ~pickupBetterGun() {
        // nimic
    }
};

// resource: res/faster_shooting.png
class pickupFasterShooting : public Pickup {
public:
    pickupFasterShooting(float x, float y) : Pickup(x, y, "faster_shooting") {
        // nimic
    }

    void draw(RenderWindow &w) override {
        // ANALOG
        // se va desena un sprite
        // cu textura faster_shooting.png
        // la pozitia x, y
        // si cu marimea size
        Texture texture;
        texture.loadFromFile("res/faster_shooting.png");
        Sprite sprite(texture);
        sprite.setPosition(getX(), getY());
        // are 32x32 px
        sprite.setOrigin(16, 16);
        w.draw(sprite);
    }

    void onPickup(Player &player) override {
        if (player.getFireRate() < 3) {
            return;
        }

        // scade firerateul cu o valoare random intre 2 si 3
        player.setFireRate(player.getFireRate() - rand_uniform(2, 3));
    }

    pickupFasterShooting() : Pickup() {
        // nimic
    }

    ~pickupFasterShooting() {
        // nimic
    }
};

// resource: res/boomerang.png
class pickupBoomerang : public Pickup {
public:
    pickupBoomerang(float x, float y) : Pickup(x, y, "boomerang") {
        // nimic
    }

    void draw(RenderWindow &w) override {
        // ANALOG
        // se va desena un sprite
        // cu textura boomerang.png
        // la pozitia x, y
        // si cu marimea size
        Texture texture;
        texture.loadFromFile("res/boomerang.png");
        Sprite sprite(texture);
        sprite.setPosition(getX(), getY());
        // are 32x32 px
        sprite.setOrigin(16, 16);
        w.draw(sprite);
    }

    void onPickup(Player &player) override {
        // se va adauga un upgrade la player (daca-l are)
        if (!player.hasUpgrade("boomerang")) {
            player.upgrade("boomerang");
        }
    }

    pickupBoomerang() : Pickup() {
        // nimic
    }

    ~pickupBoomerang() {
        // nimic
    }
};

// resource: res/freeze.png
class pickupFreeze : public Pickup {
public:
    pickupFreeze(float x, float y) : Pickup(x, y, "freeze") {
        // nimic
    }

    void draw(RenderWindow &w) override {
        // ANALOG
        // se va desena un sprite
        // cu textura freeze.png
        // la pozitia x, y
        // si cu marimea size
        Texture texture;
        texture.loadFromFile("res/freeze.png");
        Sprite sprite(texture);
        sprite.setPosition(getX(), getY());
        // are 32x32 px
        sprite.setOrigin(16, 16);
        w.draw(sprite);
    }

    void onPickup(Player &player) override {
        // se va adauga un upgrade la player (daca-l are)
        if (!player.hasUpgrade("freeze")) {
            player.upgrade("freeze");
        }
    }

    pickupFreeze() : Pickup() {
        // nimic
    }

    ~pickupFreeze() {
        // nimic
    }
};

// resource: res/zigzag.png
class pickupZigzag : public Pickup {
public:
    pickupZigzag(float x, float y) : Pickup(x, y, "zigzag") {
        // nimic
    }

    void draw(RenderWindow &w) override {
        // ANALOG
        // se va desena un sprite
        // cu textura zigzag.png
        // la pozitia x, y
        // si cu marimea size
        Texture texture;
        texture.loadFromFile("res/zigzag.png");
        Sprite sprite(texture);
        sprite.setPosition(getX(), getY());
        // are 32x32 px
        sprite.setOrigin(16, 16);
        w.draw(sprite);
    }

    void onPickup(Player &player) override {
        // se va adauga un upgrade la player (daca-l are)
        if (!player.hasUpgrade("zigzag")) {
            player.upgrade("zigzag");
        }
    }

    pickupZigzag() : Pickup() {
        // nimic
    }

    ~pickupZigzag() {
        // nimic
    }
};

// resource: res/splitter.png
class pickupSplitter : public Pickup {
public:
    pickupSplitter(float x, float y) : Pickup(x, y, "splitter") {
        // nimic
    }

    void draw(RenderWindow &w) override {
        // ANALOG
        // se va desena un sprite
        // cu textura splitter.png
        // la pozitia x, y
        // si cu marimea size
        Texture texture;
        texture.loadFromFile("res/splitter.png");
        Sprite sprite(texture);
        sprite.setPosition(getX(), getY());
        // are 32x32 px
        sprite.setOrigin(16, 16);
        w.draw(sprite);
    }

    void onPickup(Player &player) override {
        // se va adauga un upgrade la player (daca-l are)
        if (!player.hasUpgrade("splitter")) {
            player.upgrade("splitter");
        }
    }

    pickupSplitter() : Pickup() {
        // nimic
    }

    ~pickupSplitter() {
        // nimic
    }
};

// resource: res/remote_control.png
class pickupRemoteControl : public Pickup {
public:
    pickupRemoteControl(float x, float y) : Pickup(x, y, "remote_control") {
        // nimic
    }

    void draw(RenderWindow &w) override {
        // ANALOG
        // se va desena un sprite
        // cu textura remote_control.png
        // la pozitia x, y
        // si cu marimea size
        Texture texture;
        texture.loadFromFile("res/remote_control.png");
        Sprite sprite(texture);
        sprite.setPosition(getX(), getY());
        // are 32x32 px
        sprite.setOrigin(16, 16);
        w.draw(sprite);
    }

    void onPickup(Player &player) override {
        // se va adauga un upgrade la player (daca-l are)
        if (!player.hasUpgrade("remote_control")) {
            player.upgrade("remote_control");
        }
    }

    pickupRemoteControl() : Pickup() {
        // nimic
    }

    ~pickupRemoteControl() {
        // nimic
    }
};

// resource: res/homing.png
class pickupHoming : public Pickup {
public:
    pickupHoming(float x, float y) : Pickup(x, y, "homing") {
        // nimic
    }

    void draw(RenderWindow &w) override {
        // ANALOG
        // se va desena un sprite
        // cu textura homing.png
        // la pozitia x, y
        // si cu marimea size
        Texture texture;
        texture.loadFromFile("res/homing.png");
        Sprite sprite(texture);
        sprite.setPosition(getX(), getY());
        // are 32x32 px
        sprite.setOrigin(16, 16);
        w.draw(sprite);
    }

    void onPickup(Player &player) override {
        // se va adauga un upgrade la player (daca-l are)
        if (!player.hasUpgrade("homing")) {
            player.upgrade("homing");
        }
    }

    pickupHoming() : Pickup() {
        // nimic
    }

    ~pickupHoming() {
        // nimic
    }
};

// resource: res/gravitational.png
class pickupGravitational : public Pickup {
public:
    pickupGravitational(float x, float y) : Pickup(x, y, "gravitational") {
        // nimic
    }

    void draw(RenderWindow &w) override {
        // ANALOG
        // se va desena un sprite
        // cu textura gravitational.png
        // la pozitia x, y
        // si cu marimea size
        Texture texture;
        texture.loadFromFile("res/gravitational.png");
        Sprite sprite(texture);
        sprite.setPosition(getX(), getY());
        // are 32x32 px
        sprite.setOrigin(16, 16);
        w.draw(sprite);
    }

    void onPickup(Player &player) override {
        // se va adauga un upgrade la player (daca-l are)
        if (!player.hasUpgrade("gravitational")) {
            player.upgrade("gravitational");
        }
    }

    pickupGravitational() : Pickup() {
        // nimic
    }

    ~pickupGravitational() {
        // nimic
    }
};

class Planet {
protected: // vom avea nevoie de aceste date in clasele mostenite
    // static int id;
    unsigned id;
    float x, y; // pozitia (coordonate mai mult...)
    float size; // marimea (1024px --> 30.000px)
    string name, texturePath;
    Language * lang; // limba in care vor vorbi extraterestrii de pe planeta
    Texture texture;
    Sprite sprite;
    bool visited = false;
public:
    // constr fara param
    Planet() {
        x = rand_uniform(-10000, 10000);
        y = rand_uniform(-10000, 10000);
        size = rand_uniform(1, 1.1);
        name = "unknown";
        texturePath = "res/unknown.png";
        lang = new Language();
        texture.loadFromFile(texturePath);
        sprite.setTexture(texture);
        visited = false;
        id = -1;
    }

    // constr cu param
    Planet(float x, float y, float size, string name, string texturePath) {
        this->x = x;
        this->y = y;
        this->size = size;
        this->name = name;
        this->texturePath = texturePath;
        lang = new Language();
        texture.loadFromFile(texturePath);
        sprite.setTexture(texture);
        visited = false;
        id = -1; // trebuie setat neaparat dupa, degeaba e in constructor
    }

    // constr cu toti param (NU VA FI FOLOSIT)
    Planet(float x, float y, float size, string name, string texturePath, Language * lang, bool visited, unsigned id) {
        this->x = x;
        this->y = y;
        this->size = size;
        this->name = name;
        this->texturePath = texturePath;
        this->lang = lang;
        this->visited = visited;
        this->id = id;
    }

    void setId(unsigned id) {
        this->id = id;
    }

    // copy constructor
    Planet(const Planet &p) {
        x = p.x;
        y = p.y;
        size = p.size;
        name = p.name;
        texturePath = p.texturePath;
        this->lang = p.lang;
        visited = p.visited;
        id = p.id;
    }

    // operatorul de atribuire
    Planet& operator=(const Planet &p) {
        x = p.x;
        y = p.y;
        size = p.size;
        name = p.name;
        texturePath = p.texturePath;
        this->lang = p.lang;
        visited = p.visited;
        id = p.id;
        return *this;
    }

    void setVisited(bool visited) {
        this->visited = visited;
    }

    unsigned getId() {
        return id;
    }

    virtual void draw(RenderWindow &w) {
        // se va desena spriteul
        // la pozitia x, y
        // si cu marimea size
        // mareste de size ori si centreaza
        sprite.setScale(size, size);
        sprite.setOrigin(texture.getSize().x / 2, texture.getSize().y / 2);
        sprite.setPosition(x, y);

        w.draw(sprite);

        // pt a da fix la bugu unde nu se afiseaza spriteurile,
        // se va adauga si un outline de 1px, opacitate f f mica
        // in forma de cerc de marimea planetei
        CircleShape circle(size / 2);
        circle.setFillColor(Color::Transparent);
        circle.setOutlineColor(Color(255, 255, 255, 5));
        circle.setOutlineThickness(1);
        circle.setPosition(x, y);
        w.draw(circle);

        // daca playeru e deasupra planetei, se va seta visited=true
        if (distance(Vector2f(x, y), Vector2f(400, 300)) < size * 1024 / 2 && !visited) {
            visited = true;
            // se da si semnalul pentru a primi banutzi pt vizita proaspata
            freshVisit = true;
        }

        // cand e deasupra ei, safeZone = true
        if (distance(Vector2f(x, y), Vector2f(400, 300)) < size * 1024 / 2) {
            safeZone = true;
            planetPlayerIsOn = this->id;
        }
    }

    virtual void move() {
        // inertia
        x -= playerVelocityX;
        y -= playerVelocityY;
    }

    // getteri
    virtual Vector2f location() {
        return Vector2f(x, y);
    }

    float getSize() {
        return size;
    }

    string getName() {
        return name;
    }

    string getTexturePath() {
        return texturePath;
    }

    float getRadius() {
        return size * 1024;
    }

    bool isVisited() {
        return visited;
    }

    Language * getLanguage() {
        return lang;
    }

    string phrase() {
        // cout << "DEBUG: generating phrase for planet \"" << name << "\"." << endl;
        // cout << textGen + 1 << endl;
        // cout << lang << " - ";
        // cout << textGen.generatePhrase(1, lang) << endl;
        return textGen + lang;
    }

    // op de afisarae va arata tot despre planeta
    friend ostream& operator<<(ostream &out, const Planet &p) {
        out << "Planet " << p.name << " at " << p.x << ", " << p.y << " with size " << p.size << " and texture " << p.texturePath << " and visited = " << p.visited << " and id = " << p.id << endl;
        return out;
    }

    // destructor
    ~Planet() {
        delete lang;
    }
};

// lunile vor fi obiectele ce se invart in jurul planetelor
// mosteneste Planet pt ca sunt f f similare, ... nu de alta
class Moon : public Planet { // trebuia lowkey sa fac o clasa pt CorpCeresc da nu prea mai e timp...
private:
    float parentX, parentY; // pozitia planetei in juru careia se invarte
    float orbitAngle; // unghiul la care se afla fatza de planeta
    float orbitRadius; // distanta de la planeta
    float rotationAngle; // unghiul de rotatie locala
    float rotationVelocity; // viteza de rotatie locala
    float orbitVelocity; // viteza de rotatie in jurul planetei
public:
    // constructor cu parametri
    Moon(string name, float size, string texturePath, float parentX, float parentY, float orbitRadius) :
    Planet(parentX + cos(orbitAngle) * orbitRadius, parentY + sin(orbitAngle) * orbitRadius, size, name, texturePath) {
        // ^^ in lista de initializere am pus Planet(...) deoarece constructorul default incurca (nu avea texturi)
        this->name = name;
        this->texturePath = texturePath;
        texture.loadFromFile(texturePath);
        // cout << "DEBUG: Trying to load \"" << texturePath << "\" in constructor." << endl;
        sprite.setTexture(texture);
        // cout << "DEBUG: Loaded \"" << texturePath << "\" in sprite." << endl;
        
        this->size = size;

        this->parentX = parentX;
        this->parentY = parentY;
        
        this->orbitRadius = orbitRadius;
        
        orbitAngle = rand_uniform(0, 2 * 3.141);
        rotationAngle = rand_uniform(0, 2 * 3.141);
        rotationVelocity = rand_uniform(-0.1, 0.1) / 180 * 3.141;
        orbitVelocity = rand_uniform(-0.2, 0.2) / 180 * 3.141;
        
        this->x = parentX + cos(orbitAngle) * orbitRadius;
        this->y = parentY + sin(orbitAngle) * orbitRadius;

        // cout << "DEBUG: Moon created: \n";
        // cout << "x = " << x << ", y = " << y << endl;
        // cout << "parentX = " << parentX << ", parentY = " << parentY << endl;
        // cout << "orbitRadius = " << orbitRadius << endl;
        // cout << "orbitAngle = " << orbitAngle << endl;
        // cout << "rotationAngle = " << rotationAngle << endl;
        // cout << "rotationVelocity = " << rotationVelocity << endl;
        // cout << "orbitVelocity = " << orbitVelocity << endl;
    }

    // settari pt a evita bugu de concurenta pe thread la rotatii si velocitati
    void setOrbitAngle(float orbitAngle) { this->orbitAngle = orbitAngle; }
    void setRotationAngle(float rotationAngle) { this->rotationAngle = rotationAngle; }
    void setRotationVelocity(float rotationVelocity) { this->rotationVelocity = rotationVelocity; }
    void setOrbitVelocity(float orbitVelocity) { this->orbitVelocity = orbitVelocity; }

    // si cel cu TOTI parametrii (nu va fi folosit)
    Moon(string name, float size, string texturePath, float parentX, float parentY, float orbitRadius, float orbitAngle, float rotationAngle, float rotationVelocity, float orbitVelocity) :
    Planet(parentX + cos(orbitAngle) * orbitRadius, parentY + sin(orbitAngle) * orbitRadius, size, name, texturePath) {
        this->name = name;
        this->texturePath = texturePath;
        texture.loadFromFile(texturePath);
        sprite.setTexture(texture);
        
        this->size = size;

        this->parentX = parentX;
        this->parentY = parentY;
        
        this->orbitRadius = orbitRadius;
        
        this->orbitAngle = orbitAngle;
        this->rotationAngle = rotationAngle;
        this->rotationVelocity = rotationVelocity;
        this->orbitVelocity = orbitVelocity;
        
        this->x = parentX + cos(orbitAngle) * orbitRadius;
        this->y = parentY + sin(orbitAngle) * orbitRadius;
    }

    // constructor fara parametri
    // * NU VREAU SA FIE FOLOSIT VREODATA
    // * DACA E FOLOSIT, E O EROARE
    Moon() : Planet() {
        parentX = 0;
        parentY = 0;
        orbitRadius = 0;
        orbitAngle = 0;
        rotationAngle = 0;
        rotationVelocity = 0;
        orbitVelocity = 0;
        cerr << "DEBUG: Moon() constructor should not be used!";
    }

    // copy constructor
    Moon(const Moon &m) : Planet(m) {
        parentX = m.parentX;
        parentY = m.parentY;
        orbitRadius = m.orbitRadius;
        orbitAngle = m.orbitAngle;
        rotationAngle = m.rotationAngle;
        rotationVelocity = m.rotationVelocity;
        orbitVelocity = m.orbitVelocity;
    }

    // operatorul de atribuire
    Moon& operator=(const Moon &m) {
        parentX = m.parentX;
        parentY = m.parentY;
        orbitRadius = m.orbitRadius;
        orbitAngle = m.orbitAngle;
        rotationAngle = m.rotationAngle;
        rotationVelocity = m.rotationVelocity;
        orbitVelocity = m.orbitVelocity;
        return *this;
    }

    void draw(RenderWindow &w) override {
        // se va desena spriteul
        // la pozitia x, y
        // mareste de size ori si centreaza
        sprite.setScale(size, size);
        sprite.setOrigin(texture.getSize().x / 2, texture.getSize().y / 2);
        sprite.setPosition(x, y);
        // se roteste in fct de rotationAngle
        sprite.setRotation(rotationAngle * 180 / 3.141);
        w.draw(sprite);

        // pt a da fix la bugu unde nu se afiseaza spriteurile,
        // se va adauga si un outline de 1px, opacitate f f mica
        // in forma de cerc de marimea planetei
        CircleShape circle(size * 1024 / 2);
        circle.setFillColor(Color::Transparent);
        circle.setOutlineColor(Color(255, 255, 255, 5));
        circle.setOutlineThickness(1);
        circle.setPosition(x, y);
        w.draw(circle);

        // se va desena si orbita
        // simt ca ar fi interesant
        // #fff4 dar lineWidth va fi 2px pt prima data
        CircleShape orbit(orbitRadius);
        orbit.setFillColor(Color::Transparent);
        orbit.setOutlineColor(Color(255, 255, 255, 20));
        orbit.setOutlineThickness(2);
        orbit.setOrigin(orbitRadius, orbitRadius);
        orbit.setPosition(parentX, parentY);
        w.draw(orbit);

        // daca playeru e deasupra planetei, se va seta visited=true
        if (distance(Vector2f(x, y), Vector2f(400, 300)) < size * 1024 / 2 && !visited) {
            visited = true;
            freshVisit = true;
            // safeZone = true; // nu vreau ca pe luna sa fie safezone de fapt
        }

        // incarca fuel daca e deasupra
        if (distance(Vector2f(x, y), Vector2f(400, 300)) < size * 1024 / 2) {
            moonEnergy = true;
        }
    }

    void move() override {
        // orbitarea
        this->x = parentX + cos(orbitAngle) * orbitRadius;
        this->y = parentY + sin(orbitAngle) * orbitRadius;

        // inertia
        x -= playerVelocityX;
        y -= playerVelocityY;
        parentX -= playerVelocityX;
        parentY -= playerVelocityY;

        // rotatia naturala
        this->orbitAngle += orbitVelocity;
        this->rotationAngle += rotationVelocity;
    }

    // << afiseaza info despre luna (pozitia si ambele locatii)
    friend ostream& operator<<(ostream &out, const Moon &m) {
        out << "Moon at " << m.x << ", " << m.y << " orbiting at " << m.parentX << ", " << m.parentY << endl;
        return out;
    }

    // destructor
    ~Moon() {
        // nmk ca se apeleaza destructorul de la Planet 
        // ala sterge automat lang
    }
};

class Coin {
private:
    float x, y;
    float vx, vy;
    bool deleteMe = false;
public:
    // cel normal
    Coin(float x, float y) {
        this->x = x;
        this->y = y;
        float randomAngle = rand_uniform(0, 2 * 3.141);
        this->vx = cos(randomAngle) * 2;
        this->vy = sin(randomAngle) * 2;
    }

    // copy constructor
    Coin(const Coin &c) {
        x = c.x;
        y = c.y;
        vx = c.vx;
        vy = c.vy;
        deleteMe = c.deleteMe;
    }

    // acesta nu va fi folosit vreodata
    Coin() {
        x = 0;
        y = 0;
        vx = 0;
        vy = 0;
        deleteMe = false;
    }

    // operatorul de atribuire
    Coin& operator=(const Coin &c) {
        x = c.x;
        y = c.y;
        vx = c.vx;
        vy = c.vy;
        deleteMe = c.deleteMe;
        return *this;
    }

    // desenarea banutzului
    void draw(RenderWindow &w) {
        // se va desena un cerc
        // cu raza de 5px
        // la pozitia x, y
        CircleShape circle(5);
        circle.setFillColor(Color::Yellow);
        circle.setOrigin(5, 5);
        circle.setPosition(x, y);
        w.draw(circle);
    }

    // mai mult functia de update
    void move() {
        // inertia
        x -= playerVelocityX;
        y -= playerVelocityY;

        // velocitatea banutzului creste spre player
        float angle = atan2(300 - y, 400 - x);
        float atractia = 900/(distance(Vector2f(x, y), Vector2f(400, 300))-16) + 1;
        vx += cos(angle) / 15 * atractia;
        vy += sin(angle) / 15 * atractia;

        // se va misca in directia
        // in care a fost lansat
        x += vx;
        y += vy;

        // da si fortat spre player
        x += cos(angle) / 2;
        y += sin(angle) / 2;

        // daca e prea departe de player
        // se va sterge
        if (distance(Vector2f(x, y), Vector2f(400, 300)) > 600) {
            deleteMe = true;
        }

        // dar si cand e atinsa de player la fel (ca ii da pickup)
        if (distance(Vector2f(x, y), Vector2f(400, 300)) < 20) {
            deleteMe = true;
            money += 1;

            // sunetul de colectare
            soundManager.playSound("coin_pickup");

            // se adauga un efect de particule
            particleEngine.coinPickup(Vector2f(x, y));
        }
    }

    bool needsDeletion() {
        return deleteMe;
    }

    Vector2f location() {
        return Vector2f(x, y);
    }

    ~Coin() {
        // nimic
    }

    // operatoru -- il va sterge (si in fatza si in spate)
    void operator--(int) {
        deleteMe = true;
    }

    // operatoru de afisare va arata locatia
    friend ostream& operator<<(ostream &out, const Coin &c) {
        out << "Coin at " << c.x << ", " << c.y << endl;
        return out;
    }

    // constructoru cu toti param (NU VA FI NEVOIE)
    Coin(float x, float y, float vx, float vy, bool deleteMe) {
        this->x = x;
        this->y = y;
        this->vx = vx;
        this->vy = vy;
        this->deleteMe = deleteMe;
    }
};

// creaturile alien vor fi friendly
// ba chiar vor trage si ei in inamici
// cu gloante homing pt ca-s mai avansati
class Alien {
private:
    float x, y;
    float originX = 0, originY = 0; // pozitia de origine (planeta unde tre sa stea)
    float vx, vy; // viteza de deplasare
    float angle;
    float fireRate;
    float fireCooldown;
    // bool deleteMe = false; <--- //* nu va fi nevoie sa fie stersi, extraterestrii exista mereu
    string name; // va fi atribuit folosind textgenu
    float race; // va fi folosit pentru a alege culoarea lor
    TwoOutputsBrain brain; // va fi folosit pentru a alege actiunile lor
    // parametrii creier:
    // - dunghiul fatza de player
    // - distanta de la player
    // - distanta de la cel mai apropiat alien
    // - rasa lui doar ca (x-180)/180 (ca sa fie intre -1 si 1)
    Texture texture;
    Sprite sprite;
    // Planet planet; // planeta pe care se afla (de aici va extrage limba in care vorbeste)
    string talks; // ce vorbeste (se afiseaza deasupra lui)
public:
    // constructor cu parametri
    Alien(float x, float y) {
        this->x = x;
        this->y = y;
        this->vx = 0;
        this->vy = 0;
        this->angle = 0;
        this->fireRate = 8;
        this->fireCooldown = 0;
        this->name = textGen.generateSpaceshipName();
        this->race = rand_uniform(0, 360);
        brain.init(3);
        texture.loadFromFile("./res/ufo.png");
        sprite.setTexture(texture);
        // cout << "DEBUG: Alien named " << name << " created at " << x << ", " << y << endl;
    }

    // constructor cu toti parametrii (da' nu va fi folosit vreodata)
    Alien(float x, float y, float vx, float vy, float angle, float fireRate, float fireCooldown, string name, float race, TwoOutputsBrain brain, string talks) {
        this->x = x;
        this->y = y;
        this->vx = vx;
        this->vy = vy;
        this->angle = angle;
        this->fireRate = fireRate;
        this->fireCooldown = fireCooldown;
        this->name = name;
        this->race = race;
        this->brain = brain;
        this->talks = talks;
        texture.loadFromFile("./res/ufo.png");
        sprite.setTexture(texture);
    }

    // aceasta functie vreau sa fie singura care ii adauga originea
    // pt ca daca nu e folosita, extraterestrii sa se intoarca la 0, 0
    void setOrigin(Vector2f origin) {
        originX = origin.x;
        originY = origin.y;
    }
    // o supraincarcr si cu x si y pt ca nu se stie daca am nevoie
    void setOrigin(float x, float y) {
        originX = x;
        originY = y;
    }

    // si asta la fel 
    void setRace(float x) {
        race = x;
    }

    // la name e doar pt reparararea
    // bugului de concurenta in thread
    // de au aceealsi nume
    // cand sunt adaugati
    void setName(string name) {
        this->name = name;
    }

    // copy constructor
    Alien(const Alien &a) {
        x = a.x;
        y = a.y;
        vx = a.vx;
        vy = a.vy;
        angle = a.angle;
        fireRate = a.fireRate;
        fireCooldown = a.fireCooldown;
        name = a.name;
        race = a.race;
        brain = a.brain;
        // planet = a.planet;
        talks = a.talks;
    }

    // acesta nu va fi folosit vreodata (sper)
    Alien() {
        x = 0;
        y = 0;
        vx = 0;
        vy = 0;
        angle = 0;
        fireRate = 0;
        fireCooldown = 0;
        name = "unknown";
        race = 0;
        brain.init(3);
        talks = "unknown";
        // planet = Planet();
    }

    // operatorul de atribuire
    Alien& operator=(const Alien &a) {
        x = a.x;
        y = a.y;
        vx = a.vx;
        vy = a.vy;
        angle = a.angle;
        fireRate = a.fireRate;
        fireCooldown = a.fireCooldown;
        name = a.name;
        race = a.race;
        brain = a.brain;
        // planet = a.planet;
        talks = a.talks;
        return *this;
    }

    void draw(RenderWindow &w) {
        // se va desena un sprite
        // cu ozn-ul iar extraterestrul
        // se va desena procedural
        // folosind cercuri si rectangle-uri
        // aleatorii peste care 
        // pun doi ochi de culoarea
        // opusa rasei
        // si un nume deasupra
        // care va fi centrat ofc (altfel nu ar arata bn)

        // HSL method
        auto hue = static_cast<int>(sin(this->race * 3.1415 / 180) * 180 + 180);
        auto raceColor = hslToRGB(hue, 50, 50);

        // intai se deseneaza ozn-ul pe care e
        // (adica doar sprite-ul)
        sprite.setPosition(x, y);
        sprite.setOrigin(texture.getSize().x / 2, texture.getSize().y / 2);
        // sprite.setScale(1,1);
        w.draw(sprite);

        // apoi se deseneaza extraterestrul
        // corpul lui e format din 5 cercuri
        // si un rectangle
        // cercurile sunt random
        // iar ochii sunt de culoarea opusa rasei

        // corpul e facut din 5 cercuri
        // puse simetric si pe dreapta si pe stanga

        // corp principal
        CircleShape body1(15 + sin(race*2) * 5);
        body1.setFillColor(Color(raceColor[0], raceColor[1], raceColor[2]));
        body1.setOutlineColor(Color::White);
        body1.setOutlineThickness(2);
        body1.setOrigin(body1.getRadius(), body1.getRadius());
        body1.setPosition(x, y);
        w.draw(body1);

        // partea nr 1
        CircleShape body2(10 + sin(race*3) * 4);
        body2.setFillColor(Color(raceColor[0], raceColor[1], raceColor[2]));
        body2.setOutlineColor(Color::Black);
        body2.setOutlineThickness(1);
        body2.setOrigin(body2.getRadius(), body2.getRadius());
        body2.setPosition(x - 10*sin(race*3 + race/2), y + 10*sin(race*3 + race/3));
        // si mirrorul
        CircleShape body2mirror(10 + sin(race*3) * 4);
        body2mirror.setFillColor(Color(raceColor[0], raceColor[1], raceColor[2]));
        body2mirror.setOutlineColor(Color::Black);
        body2mirror.setOutlineThickness(1);
        body2mirror.setOrigin(body2mirror.getRadius(), body2mirror.getRadius());
        body2mirror.setPosition(x + 10*sin(race*3 + race/2), y + 10*sin(race*3 + race/3));
        w.draw(body2);
        w.draw(body2mirror);

        // partea nr 2
        CircleShape body3(10 + sin(race*4) * 4);
        body3.setFillColor(Color(raceColor[0], raceColor[1], raceColor[2]));
        body3.setOutlineColor(Color::Black);
        body3.setOutlineThickness(1);
        body3.setOrigin(body3.getRadius(), body3.getRadius());
        body3.setPosition(x - 10*sin(race*4 + race/2), y + 10*sin(race*4 + race/3));
        // si mirrorul
        CircleShape body3mirror(10 + sin(race*4) * 4);
        body3mirror.setFillColor(Color(raceColor[0], raceColor[1], raceColor[2]));
        body3mirror.setOutlineColor(Color::Black);
        body3mirror.setOutlineThickness(1);
        body3mirror.setOrigin(body3mirror.getRadius(), body3mirror.getRadius());
        body3mirror.setPosition(x + 10*sin(race*4 + race/2), y + 10*sin(race*4 + race/3));
        w.draw(body3);
        w.draw(body3mirror);

        // partea nr 3 va fi rectangle
        RectangleShape body4(Vector2f(20, 10));
        body4.setFillColor(Color(raceColor[0], raceColor[1], raceColor[2]));
        body4.setOutlineColor(Color::Black);
        body4.setOutlineThickness(2);
        body4.setOrigin(body4.getSize().x/2, body4.getSize().y/2);
        body4.setPosition(x + 15*sin(race*5 + race/2), y + 15*sin(race*5 + race/3));
        body4.setRotation(sin(race*7) * 2);
        // si mirrorul
        RectangleShape body4mirror(Vector2f(20, 10));
        body4mirror.setFillColor(Color(raceColor[0], raceColor[1], raceColor[2]));
        body4mirror.setOutlineColor(Color::Black);
        body4mirror.setOutlineThickness(2);
        body4mirror.setOrigin(body4mirror.getSize().x/2, body4mirror.getSize().y/2);
        body4mirror.setPosition(x - 15*sin(race*5 + race/2), y + 15*sin(race*5 + race/3));
        body4mirror.setRotation(sin(race*7) * 2);
        w.draw(body4);
        w.draw(body4mirror);

        // ochii
        CircleShape eye1(3);
        eye1.setFillColor(Color(255 - raceColor[0], 255 - raceColor[1], 255 - raceColor[2]));
        eye1.setOutlineColor(Color::White);
        eye1.setOrigin(eye1.getRadius(), eye1.getRadius());
        eye1.setPosition(x - 5, y - 5);
        w.draw(eye1);

        CircleShape eye2(3);
        eye2.setFillColor(Color(255 - raceColor[0], 255 - raceColor[1], 255 - raceColor[2]));
        eye1.setOutlineColor(Color::White);
        eye2.setOrigin(eye2.getRadius(), eye2.getRadius());
        eye2.setPosition(x + 5, y - 5);
        w.draw(eye2);

        // in interiorul ochilor va pupila, cu negru
        // si se va uita in directia in care merge
        // (adica se calculeaza unghiul cu vx si vy)
        auto unghiOchi = atan2(vy, vx);
        CircleShape pupil1(2);
        pupil1.setFillColor(Color::Black);
        pupil1.setOrigin(pupil1.getRadius(), pupil1.getRadius());
        pupil1.setPosition(x - 5 + cos(unghiOchi) * 2, y - 5 + sin(unghiOchi) * 2);
        w.draw(pupil1);

        CircleShape pupil2(2);
        pupil2.setFillColor(Color::Black);
        pupil2.setOrigin(pupil2.getRadius(), pupil2.getRadius());
        pupil2.setPosition(x + 5 + cos(unghiOchi) * 2, y - 5 + sin(unghiOchi) * 2);
        w.draw(pupil2);

        // numele (va fi centered inclusiv cand textul e prea mare)
        Text text(name, mainFont, 12);
        text.setFillColor(Color::White);
        text.setPosition(x - text.getLocalBounds().width / 2, y - 30);
        w.draw(text);

        // ce zice
        // codul urmator e comentat pt ca randu e prea mare.
        // Text text2(talks, mainFont, 12);
        // text2.setFillColor(Color::White);
        // text2.setOutlineColor(Color::Black);
        // text2.setOutlineThickness(1);
        // text2.setPosition(x - text2.getLocalBounds().width / 2, y + 20);
        // w.draw(text2);
        // codul corect pentru a desparte textul in mai multe linii
        // maxim 3 cuvinte pe rand
        Text text2("", mainFont, 12);
        text2.setFillColor(Color::White);
        text2.setOutlineColor(Color::Black);
        text2.setOutlineThickness(1);
        text2.setPosition(x - text2.getLocalBounds().width / 2, y + 20);
        string textToDraw = talks;
        string word = "";
        int line = 0;
        int wordCount = 0;
        for (int i = 0; i < textToDraw.size(); i++) {
            if (textToDraw[i] == ' ') {
            if (text2.getLocalBounds().width > 200 || wordCount == 2) {
                text2.setString(word);
                text2.setPosition(x - text2.getLocalBounds().width / 2, y + 20 + 15 * line);
                w.draw(text2);
                line++;
                word = "";
                wordCount = 0;
            } else {
                word += ' ';
                wordCount++;
            }
            } else {
            word += textToDraw[i];
            }
        }
        text2.setString(word);
        text2.setPosition(x - text2.getLocalBounds().width / 2, y + 20 + 15 * line);
        w.draw(text2);
    }

    void talkSomething(unique_ptr<Planet> &planet) {
        // cout << "DEBUG: planetname = " << planet->getName() << endl;
        // se va seta ce vorbeste
        talks = planet->phrase();
    }

    Vector2f location() {
        return Vector2f(x, y);
    }

    void move(vector<Alien> &aliens) {
        // inertia
        x -= playerVelocityX;
        y -= playerVelocityY;

        // si originea lor are intertie
        originX -= playerVelocityX;
        originY -= playerVelocityY;

        // se va misca in directia
        // in care a fost lansat
        x += vx;
        y += vy;

        // find closest alien
        Vector2f closestAlien = Vector2f(0, 0);
        float closestDistance = 999999;
        for (auto &alien : aliens) {
            if (distance(Vector2f(x, y), alien.location()) < closestDistance) {
                closestDistance = distance(Vector2f(x, y), alien.location());
                closestAlien = alien.location();
            }
        }

        // se va calcula miscarea folosind inteligenta lui artificiala
        auto calculatedMovement = this->brain.calculus({
            (float)atan2(300 - y, 400 - x), // unghiul fata de player
            (float)sin(distance(Vector2f(x, y), Vector2f(400, 300)) / 30 * 3.14/2), // conversie in sinusoidala pt pattern natural de mers
            (float)sin(distance(Vector2f(x, y), closestAlien) / 30 * 3.14/2), // iar impartirile /300 pt sensibilitate
            (this->race - 180) / 180 // pt -1 la 1 range
        });

        // * MODUL VECHI DE MISCARE
        // this->vx += calculatedMovement[0] / 10;
        // this->vy += calculatedMovement[1] / 10;
        // * MODUL NOU. PRIMUL PARAMETRU E UNGHIUL (0-2pi) SI AL DOILEA E VITEZA
        this->vx += cos(calculatedMovement[0] * 2 * 3.141) * calculatedMovement[1] / 10;
        this->vy += sin(calculatedMovement[0] * 2 * 3.141) * calculatedMovement[1] / 10;

        // limit to 4 and -4 (velocitatile ca dupa e prea rapid)
        if (this->vx > 2) {
            this->vx = 2;
        }
        if (this->vx < -2) {
            this->vx = -2;
        }
        if (this->vy > 2) {
            this->vy = 2;
        }
        if (this->vy < -2) {
            this->vy = -2;
        }

        // daca depaseste distanta de 2000 de la origine, se intoarce
        if (distance(Vector2f(x, y), Vector2f(originX, originY)) > 1500) {
            auto angleTowardsOrigin = atan2(originY - y, originX - x);
            this->vx += cos(angleTowardsOrigin);
            this->vy += sin(angleTowardsOrigin);
            // cout << "DISTANTA TOO FAR! distance = " << distance(Vector2f(x, y), Vector2f(originX, originY)) << endl;
        }

        // probabilitatea de 1 la 5 secunde sa taca
        if (probability_second(5)) {
            talks = "";
        }
    }

    void shoot(vector<Enemy> &enemies, vector<Bullet> &playerBullets) {
        // va trage in cel mai aproape
        // inamic, cu gloante homing (k e mai avansat unpik fiind extraterestru)
        if (fireCooldown <= 0) {
            float closestDistance = 999999;
            Vector2f closestEnemy = Vector2f(0, 0);
            for (auto &enemy : enemies) {
                if (distance(Vector2f(x, y), enemy.location()) < closestDistance) {
                    closestDistance = distance(Vector2f(x, y), enemy.location());
                    closestEnemy = enemy.location();
                }
            }
            // se va trage in directia inamicului
            auto angle = atan2(closestEnemy.y - y, closestEnemy.x - x);

            // daca n-a gasit, nu mai trage
            if (closestDistance == 999999) {
                return;
            }

            // daca nu e la 450px maxim departare de mine (PLAYERU (400, 300)), nu mai trage
            if (distance(Vector2f(x, y), Vector2f(400, 300)) > 450) {
                return;
            }

            playerBullets.push_back(Bullet(x, y, closestEnemy.x, closestEnemy.y, true)); // e true ca e friendly
            soundManager.playSound("alien_laser");
            playerBullets[playerBullets.size() - 1].setHoming(true);
            // daca rasa e superioara, gloantele vor avea si splitter
            if (race > 180) { // sper ca aceasta linie de cod nu este rasista 💀
                playerBullets[playerBullets.size() - 1].setSplitter(true);
            }
            playerBullets[playerBullets.size() - 1].setAlien();
            fireCooldown = fireRate;
        } else {
            fireCooldown--;
        }
    }

    ~Alien() {
        // delete brain;
    }

    // operatoru de output va afisa locatia
    friend ostream& operator<<(ostream &os, const Alien &a) {
        os << "Alien named " << a.name << " at " << a.x << ", " << a.y << "\n";
        return os;
    }
};

// -------------------------------------------------------------------- DEFINIREA OBIECTELOR --------------------------------------------------------------------

Star stars[NUMARUL_DE_STELE];
vector<unique_ptr<Planet>> planets;
vector<unique_ptr<Moon>> moons;

Player player;
vector<Enemy> enemies;
vector<Alien> aliens;

vector<Bullet> playerBullets;
vector<Bullet> enemyBullets;
// pickupuri de player
vector<unique_ptr<pickupSmallShield>> collectionSmallShields;
vector<unique_ptr<pickupBigShield>> collectionBigShields;
vector<unique_ptr<pickupSmallHealth>> collectionSmallHealths;
vector<unique_ptr<pickupBigHealth>> collectionBigHealths;
vector<unique_ptr<pickupMedkit>> collectionMedkits;
vector<unique_ptr<pickupFuel>> collectionFuels;
// pickuupuri de arme
vector<unique_ptr<pickupBetterGun>> collectionBetterGuns;
vector<unique_ptr<pickupFasterShooting>> collectionFasterShootings;
vector<unique_ptr<pickupBoomerang>> collectionBoomerangs;
vector<unique_ptr<pickupFreeze>> collectionFreezes;
vector<unique_ptr<pickupZigzag>> collectionZigzags;
vector<unique_ptr<pickupSplitter>> collectionSplitters;
vector<unique_ptr<pickupRemoteControl>> collectionRemoteControls;
vector<unique_ptr<pickupHoming>> collectionHomings;
vector<unique_ptr<pickupGravitational>> collectionGravitationals;

vector<unique_ptr<Coin>> coins;

// -------------------------------------------------------------------- FUNCTII JOC --------------------------------------------------------------------

// definirea sunetelor folosite de joc
void loadSounds() {
    // pentru fiecare fisier .wav in /res se va definit automat in
    // soundmanager ca sa nu fie nevoie sa se faca asta manual
    // ca sigur schimb sunetele la un moment dat
    for (const auto &entry : filesystem::directory_iterator("res")) {
        string path = entry.path().string();
        if (path.substr(path.size() - 4) == ".wav") {
            // daca incepe cu bgm, skip ca e pt celalalt soundloader
            if (path.substr(4, 3) == "bgm") {
                continue;
            }

            // altfel dap, e un sunet bun
            soundManager.defineSound(path.substr(4, path.size() - 8), path);
        }
    }
}

// definirea planetelor
void loadPlanets() {
    // fiecare fisier care incepe cu "planet" din /res si se termina in png
    // se adauga in lista de planete
    int x = rand_uniform(-5000, 5000);
    int y = rand_uniform(-5000, 5000);

    vector<string> files;

    if (filesystem::exists("res")) {
        for (const auto &entry : filesystem::directory_iterator("res")) {
            string path = entry.path().string();
            
            // le adaug in files daca sunt planete ca sa iterez dupa mai usor
            if (path.substr(path.size() - 4) == ".png" && path.substr(4, 6) == "planet") {
                files.push_back(path);
            } else {
                // cout << "tried to load " << path << " but it's not a planet\n";
                // cout << "path.substr(path.size() - 4) = " << path.substr(path.size() - 4) << endl;
                // cout << "path.substr(4, 5) = " << path.substr(4, 5) << endl;
            }
        }
    }

    vector<string> names;

    for (int i = 0; i < files.size(); i++) {
        names.push_back(textGen.generatePlanetName());
    }

    vector<float> scales;

    for (int i = 0; i < files.size(); i++) {
        scales.push_back(rand_uniform(1.5, 4));
    }

    vector<float> angles;

    for (int i = 0; i < files.size(); i++) {
        angles.push_back(rand_uniform(0, 2 * 3.141));
    }

    vector<float> distances;

    for (int i = 0; i < files.size(); i++) {
        distances.push_back(rand_uniform(4000, 12000));
    }

    // se amesteca
    random_shuffle(files.begin(), files.end());

    // cout << "DEBUG: files loaded: \n";
    // for (auto &file : files) {
    //     cout << file << endl;
    // }
    
    // (intai se goleste (cu delete ca sa nu fie memory leak))
    planets.clear();
    for (int i = 0; i < files.size(); i++) {
        // se adauga la x si y dar urmatoarea planeta 
        // va fi la x+cos(angle)*dist, y+sin(angle)*dist
        string generatedName = names[i];
        float scale = scales[i];
        planets.push_back(make_unique<Planet>(x, y, scale, generatedName, files[i]));
        auto angle = angles[i];
        x += cos(angle) * distances[i];
        y += sin(angle) * distances[i];
        // add their id
        planets[planets.size() - 1]->setId(i);
    }

    // cout << "DEBUG: planete create: \n";
    // for (auto &planet : planets) {
    //     cout << planet->location().x << " " << planet->location().y << " <- " << planet->getName() << endl;
    // }

    // cout<<"DEBUG: size = " << planets.size() << endl; 
}

// definirea lunilor
void loadMoons() {
    // fiecare fisier care incepe cu "moon" din /res si se termina in png
    // se adauga in lista de luni
    int x = rand_uniform(-1000, 1000);
    int y = rand_uniform(-1000, 1000);

    vector<string> files;

    if (filesystem::exists("res")) {
        for (const auto &entry : filesystem::directory_iterator("res")) {
            string path = entry.path().string();
            
            // le adaug in files daca sunt luni ca sa iterez dupa mai usor
            if (path.substr(path.size() - 4) == ".png" && path.substr(4, 4) == "moon") {
                files.push_back(path);
            } else {
                // cout << "tried to load " << path << " but it's not a moon\n";
                // cout << "path.substr(path.size() - 4) = " << path.substr(path.size() - 4) << endl;
                // cout << "path.substr(4, 4) = " << path.substr(4, 4) << endl;
            }
        }
    }

    vector<string> names;

    for (int i = 0; i < files.size(); i++) {
        names.push_back(textGen.generatePlanetName());
    }

    vector<float> scales;

    for (int i = 0; i < files.size(); i++) {
        scales.push_back(rand_uniform(0.5, 1.5));
    }

    // vector<float> orbitAngles;

    // for (int i = 0; i < files.size(); i++) {
    //     orbitAngles.push_back(rand_uniform(0, 2 * 3.141));
    // }

    vector<float> distances;

    for (int i = 0; i < files.size(); i++) {
        distances.push_back(rand_uniform(1000, 3000));
    }

    // acu e nevoie de 
    // threadsafe alternative to:
    // orbitAngle = rand_uniform(0, 2 * 3.141);
    // rotationAngle = rand_uniform(0, 2 * 3.141);
    // rotationVelocity = rand_uniform(-0.1, 0.1) / 180 * 3.141;
    // orbitVelocity = rand_uniform(-0.2, 0.2) / 180 * 3.141;

    vector<float> orbitAngles;
    vector<float> rotationAngles;
    vector<float> rotationVelocities;
    vector<float> orbitVelocities;

    for (int i = 0; i < files.size(); i++) {
        orbitAngles.push_back(rand_uniform(0, 2 * 3.141));
        rotationAngles.push_back(rand_uniform(0, 2 * 3.141));
        rotationVelocities.push_back(rand_uniform(-0.1, 0.1) / 180 * 3.141);
        orbitVelocities.push_back(rand_uniform(-0.2, 0.2) / 180 * 3.141);
    }

    // se amesteca
    random_shuffle(files.begin(), files.end());

    // cout << "DEBUG: files loaded: \n";
    // for (auto &file : files) {
    //     cout << file << endl;
    // }
    
    // (intai se goleste (cu delete ca sa nu fie memory leak))
    moons.clear();
    for (int i = 0; i < files.size(); i++) {
        // se decide un parinte random
        int parent = rand() % planets.size();
        // se adauga la x si y dar urmatoarea luna
        moons.push_back(make_unique<Moon>(names[i], scales[i], files[i], planets[parent]->location().x, planets[parent]->location().y, distances[i]));

        // se adauga cu setterele pt ca mai sus constructoru e pt restu
        moons[moons.size() - 1]->setOrbitAngle(orbitAngles[i]);
        moons[moons.size() - 1]->setRotationAngle(rotationAngles[i]);
        moons[moons.size() - 1]->setRotationVelocity(rotationVelocities[i]);
        moons[moons.size() - 1]->setOrbitVelocity(orbitVelocities[i]);
    }
}

// functia de initializare a jocului
void init() {
    // // testing AI
    // TwoOutputsBrain brain;
    // brain.init(2);
    // cout << brain.calculus({0, 0})[0] << " " << brain.calculus({0, 0})[1] << endl;
    // cout << brain.calculus({1, 0})[0] << " " << brain.calculus({1, 0})[1] << endl;
    // cout << brain.calculus({0, 1})[0] << " " << brain.calculus({0, 1})[1] << endl;
    // cout << brain.calculus({1, 1})[0] << " " << brain.calculus({1, 1})[1] << endl;

    // elementele de baza -------------------------

    // nici o tasta nu e apasata
    for (int i = 0; i < 256; i++) {
        keysPressed[i] = false;
    }

    // nici mouseul
    // chiar daca acestea sunt false by default, le setez doar ca sa fie
    mouseDown = false;
    leftClick = false;
    rightClick = false;

    // incepem cu meniul
    inMenu = true;

    // jocul nu e pus pe pauza by default
    gamePaused = false;

    // chestii de joc -------------------------

    // initializam vectoru de stele
    for (int i = 0; i < NUMARUL_DE_STELE; i++) {
        stars[i] = Star();
    }

    // playeru nu se misca
    playerVelocityX = 0;
    playerVelocityY = 0;

    // playeru nu mai are upgradeuri
    player.downgrade("boomerang");
    player.downgrade("freeze");
    player.downgrade("zigzag");
    player.downgrade("splitter");
    player.downgrade("remote_control");
    player.downgrade("homing");
    player.downgrade("gravitational");

    // proprietatile sunt la fel
    player.setDamage(10);
    player.setFireRate(18);

    // ! auto start game (DEBUG ONLY)
    // inMenu = false;

    // ! inamic langa sa-l testez (DEBUG ONLY)
    // enemies.push_back(Enemy(100, 100));
    // enemies[0].setHealth(1);
    // enemies[0].setDamage(1);
    // enemies[0].setShield(1);

    // ! pickup boomerang langa (DEBUG ONLY)
    // collectionBoomerangs.push_back(make_unique<pickupBoomerang>(100, 100));
    // collectionFreezes.push_back(make_unique<pickupFreeze>(200, 100));
    // collectionZigzags.push_back(make_unique<pickupZigzag>(300, 100));
    // collectionSplitters.push_back(make_unique<pickupSplitter>(400, 100));
    // collectionRemoteControls.push_back(make_unique<pickupRemoteControl>(500, 100));
    // collectionHomings.push_back(make_unique<pickupHoming>(600, 100));
    // collectionGravitationals.push_back(make_unique<pickupGravitational>(700, 100));

    // sunetele
    loadSounds();

    // planetele
    loadPlanets();

    // lunile
    loadMoons();

    // fontul principal
    mainFont.loadFromFile("res/spacemadness.ttf");

    // timeru de joc devine (iar) 0
    gameClock->reset();
    // cout << "stamp: " << secondsSince(*gameClock) << endl;

    // // ! doi extraterestrii langa spawn (DEBUG ONLY)
    // aliens.push_back(Alien(100, 100));
    // aliens.push_back(Alien(100, 200));
    // aliens.push_back(Alien(100, 300));
    // aliens.push_back(Alien(100, 400));
    // aliens[0].setOrigin(0, 0); aliens[0].setRace(1);
    // aliens[1].setOrigin(0, 0); aliens[1].setRace(1);
    // aliens[2].setOrigin(0, 0); aliens[2].setRace(1);
    // aliens[3].setOrigin(0, 0); aliens[3].setRace(1);
    // cout << "DEBUG: sizeof planets = " << planets.size() << endl;
    // cout << planets[0]->getName() << endl;
    // aliens[3].talkSomething(planets[0]);
    // delete lang;

    // pentru fiecare planeta
    // se adauga 100 extraterestrii
    vector<int> planetIds; // unde adaugam extraterestrii
    vector<string> alienNames; // numele extraterestrilor
    vector<float> races; // rasele extraterestrilor
    vector<float> alienRaces; // ce rase au extraterestrii
    // 5 rase
    for (int i = 0; i < 5; i++) {
        races.emplace_back(rand_uniform(0, 360));
    }
    for (int i = 0; i < 100; i++) {
        planetIds.emplace_back(rand() % planets.size());
        alienNames.emplace_back(textGen.generateSpaceshipName());
        alienRaces.emplace_back(races[rand() % 5]);
    }
    // se adauga extraterestrii
    for (int i = 0; i < 100; i++) {
        auto location = planets[planetIds[i]]->location();
        aliens.emplace_back(Alien(location.x + rand_uniform(-100, 100), location.y + rand_uniform(-100, 100))); // acest uniform e esential pt ca reteaua neuronala e deterministica si ar merge in aceelasi sens cu totii
        aliens[i].setOrigin(location);
        aliens[i].setRace(alienRaces[i]);
        aliens[i].setName(alienNames[i]);

        // cout << "Added Alien named " << alienNames[i] << " at " << location.x << ", " << location.y << endl;
        // cout << "origin: " << location.x << ", " << location.y << endl;
        // cout << "race: " << alienRaces[i] << "\n";
    }

    soundTrack.play();
}

// meniul de start
void drawMenu(RenderWindow &window) {
    // menubg.png e fundalul
    Texture texture;
    texture.loadFromFile("res/menubg.png");
    Sprite sprite(texture);
    window.draw(sprite);

    // doua butoane
    
    // butonul de start
    // pozitia 300 200, marimea 200 50, textura playbutton.png
    Texture startTexture;
    startTexture.loadFromFile("res/playbutton.png"); // 146x60

    // butonul de exit
    // pozitia 300 300, marimea 200 50, textura exitbutton.png
    Texture exitTexture;
    exitTexture.loadFromFile("res/exitbutton.png"); // 146x60

    // daca mouseul e pe butonul de start, se transforma in textura playbuttonhover.png
    if (mouseX >= 400 - 146/2.0 && mouseX <= 400 + 146/2.0 && mouseY >= 200 && mouseY <= 200 + 60) {
        startTexture.loadFromFile("res/playbuttonhover.png");

        // daca e apasat, start game
        if (leftClick) {
            inMenu = false;
            // + sunetu de start
            soundManager.playSound("button_click");
        }
    }

    // daca mouseul e pe butonul de exit, se transforma in textura exitbuttonhover.png
    if (mouseX >= 400 - 146/2.0 && mouseX <= 400 + 146/2.0 && mouseY >= 280 && mouseY <= 280 + 60) {
        exitTexture.loadFromFile("res/exitbuttonhover.png");

        // daca e apasat, iesim din joc
        if (leftClick) {
            window.close();
        }
    }

    // desenam butoanele
    Sprite startSprite(startTexture);
    startSprite.setPosition(400 - 146/2.0, 200);
    Sprite exitSprite(exitTexture);
    exitSprite.setPosition(400 - 146/2.0, 280);

    window.draw(startSprite);
    window.draw(exitSprite);
}

// handle la controale
void controls(RenderWindow &window) {
    // cand w e apasat, playeru se misca in spre mouse
    if (keysPressed[22]) { // (w = 22)
        // daca nu are fuel, opreste W din a avea vreun efect
        if (player.getFuel() < 1) {
            playerAccelerating = false;
            return;
        }
        // altfel continua

        // unghiul centru-mouse 
        float angle = atan2(mouseY - 300, mouseX - 400);
        playerVelocityX += cos(angle) / 20;
        playerVelocityY += sin(angle) / 20;
        playerAccelerating = true;

        // limitam viteza
        if (playerVelocityX > speedLimit) playerVelocityX = speedLimit;
        if (playerVelocityX < -speedLimit) playerVelocityX = -speedLimit;
        if (playerVelocityY > speedLimit) playerVelocityY = speedLimit;
        if (playerVelocityY < -speedLimit) playerVelocityY = -speedLimit;
        
        // // frictiune (stiu ca nu exista in spatiu)
        // playerVelocityX *= 0.99;
        // playerVelocityY *= 0.99;

        // deseneaza linia intre centru si mouse
        // ca debug
        Vertex line[] = {
            Vertex(Vector2f(400, 300)),
            Vertex(Vector2f(mouseX, mouseY))
        };
        // opacitatea 10% -> 25%
        line[0].color = Color(255, 255, 255, 25);
        line[1].color = Color(255, 255, 255, 50);
        window.draw(line, 2, Lines);

        // se scade din fuel
        player.setFuel(player.getFuel() - .3);
    } else {
        playerAccelerating = false;
    }

    // cand mouseul e apasat, se trage
    if (leftClick) {
        player.shoot(mouseX, mouseY, playerBullets);
    }

    // cand p e apasat, se pune pe pauza jocu
    if (keysPressed[15]) {
        gamePaused = !gamePaused;
        keysPressed[15] = false; // ca sa nu se dea unpause imd
    }

    // cand r e apasat, viata playerului devine -1 (restart)
    if (keysPressed[17]) {
        player.setHealth(-1);
        keysPressed[17] = false; // ridica tasta
    }

    // cand m e apasat, se pune mute/unmute la soundtrack
    if (keysPressed[12]) {
        if (gameMusicMuted) { // daca e pe mute, se da unmute
            soundTrack.unmute();
            keysPressed[12] = false;
            gameMusicMuted = false;
        } else { // altfel se da mute
            soundTrack.mute();
            keysPressed[12] = false;
            gameMusicMuted = true;
        }
    }

    // // daca si shift si w sunt apasate in aceelasi timp // am renuntat la asta 
    // se va merge cu viteza luminii
    // if (keysPressed[22] && keysPressed[42]) { ----- NU MERGE IDK WHY
    if (keysPressed[11]) { // L de la light speed
        // daca nu e fuelu peste jumate (50%), renunta
        if (player.getFuel() < 150) {
            return;
        }
        // altfel:
        lightSpeedOn = true;
        playerVelocityX += cos(atan2(mouseY - 300, mouseX - 400)) / 2;
        playerVelocityY += sin(atan2(mouseY - 300, mouseX - 400)) / 2;
        trailEffect = true;
        // limit to 5 and -5 (velocitatile ca dupa e prea rapid)
        if (playerVelocityX > 12) {
            playerVelocityX = 12;
        }
        if (playerVelocityX < -12) {
            playerVelocityX = -12;
        }
        if (playerVelocityY > 12) {
            playerVelocityY = 12;
        }
        if (playerVelocityY < -12) {
            playerVelocityY = -12;
        }
    } else {
        lightSpeedOn = false;
        trailEffect = false;
    }

    // ADMIN ONLY
    // cand click dreapta si K sunt apasate in aceelasi timp
    // mor toti inamicii
    if (rightClick && keysPressed[10]) {
        for (int i = 0; i < enemies.size(); i++) {
            enemies[i].setHealth(-1);
        }
    }

    // ADMIN ONLY
    // cand spatiu e apasat, velocitatea playerului e 0
    if (keysPressed[57]) {
        playerVelocityX = 0;
        playerVelocityY = 0;
    }

    // ADMIN ONLY
    // cand click dreapta si Q sunt apasate in acelasi timp
    // se spawneaza un pickup random pe harta
    if (rightClick && keysPressed[16]) {
        int random = rand_uniform(0, 8);
        switch (random) {
            case 0:
                collectionSmallShields.push_back(make_unique<pickupSmallShield>(rand_uniform(0, 800), rand_uniform(0, 600)));
                break;
            case 1:
                collectionBigShields.push_back(make_unique<pickupBigShield>(rand_uniform(0, 800), rand_uniform(0, 600)));
                break;
            case 2:
                collectionSmallHealths.push_back(make_unique<pickupSmallHealth>(rand_uniform(0, 800), rand_uniform(0, 600)));
                break;
            case 3:
                collectionBigHealths.push_back(make_unique<pickupBigHealth>(rand_uniform(0, 800), rand_uniform(0, 600)));
                break;
            case 4:
                collectionMedkits.push_back(make_unique<pickupMedkit>(rand_uniform(0, 800), rand_uniform(0, 600)));
                break;
            case 5:
                collectionFuels.push_back(make_unique<pickupFuel>(rand_uniform(0, 800), rand_uniform(0, 600)));
                break;
            case 6:
                collectionBetterGuns.push_back(make_unique<pickupBetterGun>(rand_uniform(0, 800), rand_uniform(0, 600)));
                break;
            case 7:
                collectionFasterShootings.push_back(make_unique<pickupFasterShooting>(rand_uniform(0, 800), rand_uniform(0, 600)));
                break;
            case 8:
                collectionBoomerangs.push_back(make_unique<pickupBoomerang>(rand_uniform(0, 800), rand_uniform(0, 600)));
                break;
            case 9:
                collectionFreezes.push_back(make_unique<pickupFreeze>(rand_uniform(0, 800), rand_uniform(0, 600)));
                break;
            case 10:
                collectionZigzags.push_back(make_unique<pickupZigzag>(rand_uniform(0, 800), rand_uniform(0, 600)));
                break;
            case 11:
                collectionSplitters.push_back(make_unique<pickupSplitter>(rand_uniform(0, 800), rand_uniform(0, 600)));
                break;
            case 12:
                collectionRemoteControls.push_back(make_unique<pickupRemoteControl>(rand_uniform(0, 800), rand_uniform(0, 600)));
                break;
            case 13:
                collectionHomings.push_back(make_unique<pickupHoming>(rand_uniform(0, 800), rand_uniform(0, 600)));
                break;
            case 14:
                collectionGravitationals.push_back(make_unique<pickupGravitational>(rand_uniform(0, 800), rand_uniform(0, 600)));
                break;
        }
    }

    // am renuntat la codu asta ca n-are rost
    // // cand click dreapta si T sunt apasate in aceelasi timp
    // // ma teleporteaza la un corp ceresc random
    // // ori luna ori planeta
    // // teleportarea se face totusi schimband 
    // // pozitia tuturor obiectelor existente 
    // // cu pozitia relativa de player
    // // a planetei
    // if (rightClick && keysPressed[20]) {
    //     // se alege un corp ceresc random
    //     bool planeta = rand_uniform(0, 1) < 0.5;
    //     int random = rand_uniform(0, planeta ? planets.size() - 1 : moons.size() - 1);
    //     float x = planeta ? planets[random]->location().x : moons[random]->location().x;

    //     // miscarea tuturor corpurilor
    //     for (int i = 0; i < planets.size(); i++) {
    //         planets[i]->
    //     }

    // ADMIN ONLY
    // cand click dreapta si E sunt apasate in acelasi timp
    // se activeaza AI-ul care trage in inamici
    if (rightClick && keysPressed[18]) {
        // se trage catre inamicul cel mai aproape
        // dintre toti vizibili
        float minDist = 1000000;
        int minIndex = -1;
        for (int i = 0; i < enemies.size(); i++) {
            if (enemies[i].getHealth() > 0) {
                float dist = distance(Vector2f(enemies[i].getX(), enemies[i].getY()), Vector2f(400, 300));
                if (dist < minDist) {
                    minDist = dist;
                    minIndex = i;
                }
            }
        }

        if (minIndex != -1) {
            player.shoot(enemies[minIndex].getX(), enemies[minIndex].getY(), playerBullets);
        }

        // merge in directia opusa inamicului cel mai aproape
        if (minIndex != -1) {
            float angle = atan2(enemies[minIndex].getY() - 300, enemies[minIndex].getX() - 400);
            playerVelocityX -= cos(angle) / 20;
            playerVelocityY -= sin(angle) / 20;
        }
    }
}

// info despre player adica viata, shieldu, combustibilu
void drawPlayerInfo(RenderWindow &window) {
    // health baru se schimba din verde in rosu in functie de health
    // locatia: 700, 680
    RectangleShape healthBar(Vector2f(player.getHealth()/100 * 90, 10));
    healthBar.setFillColor(Color(255 * (100 - player.getHealth()) / 100, 255 * player.getHealth() / 100, 0));
    healthBar.setPosition(700, 580);
    window.draw(healthBar);

    // shieldu e albastru (cliche)
    // locatia: 700, 660
    RectangleShape shieldBar(Vector2f(player.getShield()/100 * 90, 10));
    shieldBar.setFillColor(Color(9, 9, 200 + 55 * player.getShield() / 100));
    shieldBar.setPosition(700, 560);
    window.draw(shieldBar);

    // combustibilu e galben (dar devine rosu cand e < 10%)
    // locatia: 700, 640
    RectangleShape fuelBar(Vector2f(player.getFuel()/player.getMaxFuel() * 90, 10));
    if (player.getFuel() < player.getMaxFuel() / 10) {
        fuelBar.setFillColor(Color(255, 0, 0));
    } else {
        fuelBar.setFillColor(Color(255, 255, 0));
    }
    fuelBar.setPosition(700, 540);
    window.draw(fuelBar);

    // se deseneaza un outline la fiecare, pentru ca 
    // daca ar fi goala, ar arata urat invizibile
    RectangleShape outline(Vector2f(90, 10));
    outline.setFillColor(Color::Transparent);
    outline.setOutlineColor(Color::White);
    outline.setOutlineThickness(1);
    
    // cel pentru health
    outline.setPosition(700, 580);
    window.draw(outline);

    // cel pentru shield
    outline.setPosition(700, 560);
    window.draw(outline);

    // cel pentru fuel
    outline.setPosition(700, 540);
    window.draw(outline);

    // se deseneaza si upgrade-urile in coltul din stanga jos
    // voi folosi upgradeIconOrder pentru a le desena mereu una langa alta
    upgradeIconOrder = 0;
    Texture tempTexture;
    Sprite tempSprite;

    // upgrade-ul boomerang
    if (player.hasUpgrade("boomerang")) {
        tempTexture.loadFromFile("res/boomerang.png");
        tempSprite.setTexture(tempTexture);
        tempSprite.setPosition(32 * upgradeIconOrder, 600 - 32);
        window.draw(tempSprite);

        // daca e mouse-ul deasupra, se va desena un outline (#fffa)
        if (mouseX >= 0 && mouseX <= 32 && mouseY >= 600 - 32 && mouseY <= 600) {
            RectangleShape hoverOutline(Vector2f(32, 32));
            hoverOutline.setFillColor(Color::Transparent);
            hoverOutline.setOutlineColor(Color(255, 255, 250, 255));
            hoverOutline.setOutlineThickness(1);
            hoverOutline.setPosition(0, 600 - 32);
            window.draw(hoverOutline);

            // daca e apasat, se va scoate upgrade-ul
            if (leftClick && mouseDown) {
                if (player.hasUpgrade("boomerang")) {
                    player.downgrade("boomerang");
                    mouseDown = false;
                }
            }
        }

        // deseneaza un outline oricum doar cu opacitatea 20%
        RectangleShape outline(Vector2f(32, 32));
        outline.setFillColor(Color::Transparent);
        outline.setOutlineColor(Color(255, 255, 250, 20));
        outline.setOutlineThickness(1);
        outline.setPosition(32 * upgradeIconOrder, 600 - 32);
        window.draw(outline);

        upgradeIconOrder++;
    }

    // upgrade-ul splitter
    if (player.hasUpgrade("splitter")) {
        tempTexture.loadFromFile("res/splitter.png");
        tempSprite.setTexture(tempTexture);
        tempSprite.setPosition(32 * upgradeIconOrder, 600 - 32);
        window.draw(tempSprite);

        // daca e mouse-ul deasupra, se va desena un outline (#fffa)
        if (mouseX >= 32 * upgradeIconOrder && mouseX <= 32 * upgradeIconOrder + 32 && mouseY >= 600 - 32 && mouseY <= 600) {
            RectangleShape hoverOutline(Vector2f(32, 32));
            hoverOutline.setFillColor(Color::Transparent);
            hoverOutline.setOutlineColor(Color(255, 255, 250, 255));
            hoverOutline.setOutlineThickness(1);
            hoverOutline.setPosition(32 * upgradeIconOrder, 600 - 32);
            window.draw(hoverOutline);

            // daca e apasat, se va scoate upgrade-ul
            if (leftClick && mouseDown) {
                if (player.hasUpgrade("splitter")) {
                    player.downgrade("splitter");
                    mouseDown = false;
                }
            }
        }

        // deseneaza un outline oricum doar cu opacitatea 20%
        RectangleShape outline(Vector2f(32, 32));
        outline.setFillColor(Color::Transparent);
        outline.setOutlineColor(Color(255, 255, 250, 20));
        outline.setOutlineThickness(1);
        outline.setPosition(32 * upgradeIconOrder, 600 - 32);
        window.draw(outline);

        upgradeIconOrder++;
    }

    // upgrade-ul remote control
    if (player.hasUpgrade("remote_control")) {
        tempTexture.loadFromFile("res/remote_control.png");
        tempSprite.setTexture(tempTexture);
        tempSprite.setPosition(32 * upgradeIconOrder, 600 - 32);
        window.draw(tempSprite);

        // daca e mouse-ul deasupra, se va desena un outline (#fffa)
        if (mouseX >= 32 * upgradeIconOrder && mouseX <= 32 * upgradeIconOrder + 32 && mouseY >= 600 - 32 && mouseY <= 600) {
            RectangleShape hoverOutline(Vector2f(32, 32));
            hoverOutline.setFillColor(Color::Transparent);
            hoverOutline.setOutlineColor(Color(255, 255, 250, 255));
            hoverOutline.setOutlineThickness(1);
            hoverOutline.setPosition(32 * upgradeIconOrder, 600 - 32);
            window.draw(hoverOutline);

            // daca e apasat, se va scoate upgrade-ul
            if (leftClick && mouseDown) {
                if (player.hasUpgrade("remote_control")) {
                    player.downgrade("remote_control");
                    mouseDown = false;
                }
            }
        }

        // deseneaza un outline oricum doar cu opacitatea 20%
        RectangleShape outline(Vector2f(32, 32));
        outline.setFillColor(Color::Transparent);
        outline.setOutlineColor(Color(255, 255, 250, 20));
        outline.setOutlineThickness(1);
        outline.setPosition(32 * upgradeIconOrder, 600 - 32);
        window.draw(outline);

        upgradeIconOrder++;
    }

    // upgrade-ul homing
    if (player.hasUpgrade("homing")) {
        tempTexture.loadFromFile("res/homing.png");
        tempSprite.setTexture(tempTexture);
        tempSprite.setPosition(32 * upgradeIconOrder, 600 - 32);
        window.draw(tempSprite);

        // daca e mouse-ul deasupra, se va desena un outline (#fffa)
        if (mouseX >= 32 * upgradeIconOrder && mouseX <= 32 * upgradeIconOrder + 32 && mouseY >= 600 - 32 && mouseY <= 600) {
            RectangleShape hoverOutline(Vector2f(32, 32));
            hoverOutline.setFillColor(Color::Transparent);
            hoverOutline.setOutlineColor(Color(255, 255, 250, 255));
            hoverOutline.setOutlineThickness(1);
            hoverOutline.setPosition(32 * upgradeIconOrder, 600 - 32);
            window.draw(hoverOutline);

            // daca e apasat, se va scoate upgrade-ul
            if (leftClick && mouseDown) {
                if (player.hasUpgrade("homing")) {
                    player.downgrade("homing");
                    mouseDown = false;
                }
            }
        }

        // deseneaza un outline oricum doar cu opacitatea 20%
        RectangleShape outline(Vector2f(32, 32));
        outline.setFillColor(Color::Transparent);
        outline.setOutlineColor(Color(255, 255, 250, 20));
        outline.setOutlineThickness(1);
        outline.setPosition(32 * upgradeIconOrder, 600 - 32);
        window.draw(outline);

        upgradeIconOrder++;
    }

    // upgrade-ul gravitational
    if (player.hasUpgrade("gravitational")) {
        tempTexture.loadFromFile("res/gravitational.png");
        tempSprite.setTexture(tempTexture);
        tempSprite.setPosition(32 * upgradeIconOrder, 600 - 32);
        window.draw(tempSprite);

        // daca e mouse-ul deasupra, se va desena un outline (#fffa)
        if (mouseX >= 32 * upgradeIconOrder && mouseX <= 32 * upgradeIconOrder + 32 && mouseY >= 600 - 32 && mouseY <= 600) {
            RectangleShape hoverOutline(Vector2f(32, 32));
            hoverOutline.setFillColor(Color::Transparent);
            hoverOutline.setOutlineColor(Color(255, 255, 250, 255));
            hoverOutline.setOutlineThickness(1);
            hoverOutline.setPosition(32 * upgradeIconOrder, 600 - 32);
            window.draw(hoverOutline);

            // daca e apasat, se va scoate upgrade-ul
            if (leftClick && mouseDown) {
                if (player.hasUpgrade("gravitational")) {
                    player.downgrade("gravitational");
                    mouseDown = false;
                }
            }
        }

        // deseneaza un outline oricum doar cu opacitatea 20%
        RectangleShape outline(Vector2f(32, 32));
        outline.setFillColor(Color::Transparent);
        outline.setOutlineColor(Color(255, 255, 250, 20));
        outline.setOutlineThickness(1);
        outline.setPosition(32 * upgradeIconOrder, 600 - 32);
        window.draw(outline);

        upgradeIconOrder++;
    }

    // upgrade-ul zigzag
    if (player.hasUpgrade("zigzag")) {
        tempTexture.loadFromFile("res/zigzag.png");
        tempSprite.setTexture(tempTexture);
        tempSprite.setPosition(32 * upgradeIconOrder, 600 - 32);
        window.draw(tempSprite);

        // daca e mouse-ul deasupra, se va desena un outline (#fffa)
        if (mouseX >= 32 * upgradeIconOrder && mouseX <= 32 * upgradeIconOrder + 32 && mouseY >= 600 - 32 && mouseY <= 600) {
            RectangleShape hoverOutline(Vector2f(32, 32));
            hoverOutline.setFillColor(Color::Transparent);
            hoverOutline.setOutlineColor(Color(255, 255, 250, 255));
            hoverOutline.setOutlineThickness(1);
            hoverOutline.setPosition(32 * upgradeIconOrder, 600 - 32);
            window.draw(hoverOutline);

            // daca e apasat, se va scoate upgrade-ul
            if (leftClick && mouseDown) {
                if (player.hasUpgrade("zigzag")) {
                    player.downgrade("zigzag");
                    mouseDown = false;
                }
            }
        }

        // deseneaza un outline oricum doar cu opacitatea 20%
        RectangleShape outline(Vector2f(32, 32));
        outline.setFillColor(Color::Transparent);
        outline.setOutlineColor(Color(255, 255, 250, 20));
        outline.setOutlineThickness(1);
        outline.setPosition(32 * upgradeIconOrder, 600 - 32);
        window.draw(outline);

        upgradeIconOrder++;
    }

    // upgrade-ul freeze
    if (player.hasUpgrade("freeze")) {
        tempTexture.loadFromFile("res/freeze.png");
        tempSprite.setTexture(tempTexture);
        tempSprite.setPosition(32 * upgradeIconOrder, 600 - 32);
        window.draw(tempSprite);

        // daca e mouse-ul deasupra, se va desena un outline (#fffa)
        if (mouseX >= 32 * upgradeIconOrder && mouseX <= 32 * upgradeIconOrder + 32 && mouseY >= 600 - 32 && mouseY <= 600) {
            RectangleShape hoverOutline(Vector2f(32, 32));
            hoverOutline.setFillColor(Color::Transparent);
            hoverOutline.setOutlineColor(Color(255, 255, 250, 255));
            hoverOutline.setOutlineThickness(1);
            hoverOutline.setPosition(32 * upgradeIconOrder, 600 - 32);
            window.draw(hoverOutline);

            // daca e apasat, se va scoate upgrade-ul
            if (leftClick && mouseDown) {
                if (player.hasUpgrade("freeze")) {
                    player.downgrade("freeze");
                    mouseDown = false;
                }
            }
        }

        // deseneaza un outline oricum doar cu opacitatea 20%
        RectangleShape outline(Vector2f(32, 32));
        outline.setFillColor(Color::Transparent);
        outline.setOutlineColor(Color(255, 255, 250, 20));
        outline.setOutlineThickness(1);
        outline.setPosition(32 * upgradeIconOrder, 600 - 32);
        window.draw(outline);

        upgradeIconOrder++;
    }
}

// la marginea ecranului se deseneaza 
// semnele specifice pt obiecte si directiile lor
void drawArrowsTowardsPlanets(RenderWindow &window) {
    // mic triunghi la marginea ecranului 
    // care arata directia pentru fiecare planeta
    // metoda: se calculeaza unghiul si se deseneaza
    // la locatia cos(angle)*1000, sin(angle)*1000
    // apoi se limiteaza la ecran (10px->790px, 10px->590px)
    for (auto &planet : planets) {
        // daca distanta e mai mica decat 300px, nu se deseneaza sageata
        // ci un cerc langa care scrie numele planetei
        // exact in centrul locatiei
        if (distance(Vector2f(400, 300), planet->location()) < 500) {
            CircleShape circle(10);
            circle.setFillColor(Color::White);
            circle.setOutlineColor(Color::White);
            circle.setOutlineThickness(1);
            circle.setPosition(planet->location().x - 5, planet->location().y - 5);
            circle.setOrigin(5, 5);
            window.draw(circle);

            // textul din dreapta punctului
            // va avea outline negru si fontul principal (mainFont)
            Text text;
            text.setFont(mainFont);
            text.setString(planet->getName());
            text.setCharacterSize(20);
            text.setFillColor(Color::White);
            text.setOutlineColor(Color::Black);
            text.setOutlineThickness(1);
            text.setPosition(planet->location().x + 10, planet->location().y - 10);
            window.draw(text);
            // cout << "DEBUG: Printing {" << planet->getName() << "} at " << planet->location().x << " " << planet->location().y << endl;

            continue;
        }

        // altfel:
        float angle = atan2(planet->location().y - 300, planet->location().x - 400);
        float x = cos(angle) * 1000 + 400;
        float y = sin(angle) * 1000 + 300;

        if (x < 10) x = 10;
        if (x > 790) x = 790;
        if (y < 10) y = 10;
        if (y > 590) y = 590;

        // se deseneaza triunghiul
        // fill = alb
        // outline = negru
        ConvexShape arrow(3);
        arrow.setPoint(0, Vector2f(0, 0));
        arrow.setPoint(1, Vector2f(10, 5));
        arrow.setPoint(2, Vector2f(0, 10));
        // daca e vizitata, se pune cu verde de fapt
        if (planet->isVisited()) {
            arrow.setFillColor(Color(0, 255, 0));
        } else {
            arrow.setFillColor(Color::White);
        }
        arrow.setOutlineColor(Color::Black);
        arrow.setOutlineThickness(1);
        arrow.setPosition(x, y);
        arrow.setRotation(angle * 180 / 3.14159); // rotit spre planeta
        arrow.setScale(1, 1);
        window.draw(arrow);
    }

    // hai si pentru luni la fel dar mai mici cu 1px
    for (auto &moon : moons) {
        // analog ca mai sus doar ca bulina va fi mai mica cu 1px si fontu cu 3px
        if (distance(Vector2f(400, 300), moon->location()) < 500) {
            CircleShape circle(9);
            circle.setFillColor(Color::White);
            circle.setOutlineColor(Color::White);
            circle.setOutlineThickness(1);
            circle.setPosition(moon->location().x - 4, moon->location().y - 4);
            circle.setOrigin(4, 4);
            window.draw(circle);

            Text text;
            text.setFont(mainFont);
            text.setString(moon->getName());
            text.setCharacterSize(17);
            text.setFillColor(Color::White);
            text.setOutlineColor(Color::Black);
            text.setOutlineThickness(1);
            text.setPosition(moon->location().x + 10, moon->location().y - 10);
            window.draw(text);
            // cout << "DEBUG: Printing {" << moon->getName() << "} at " << moon->location().x << " " << moon->location().y << endl;

            continue;
        }

        // altfel:
        float angle = atan2(moon->location().y - 300, moon->location().x - 400);
        float x = cos(angle) * 1000 + 400;
        float y = sin(angle) * 1000 + 300;

        if (x < 10) x = 10;
        if (x > 790) x = 790;
        if (y < 10) y = 10;
        if (y > 590) y = 590;

        ConvexShape arrow(3);
        arrow.setPoint(0, Vector2f(0, 0));
        arrow.setPoint(1, Vector2f(9, 4));
        arrow.setPoint(2, Vector2f(0, 9));
        if (moon->isVisited()) {
            arrow.setFillColor(Color(0, 255, 0));
        } else {
            arrow.setFillColor(Color::White);
        }
        arrow.setOutlineColor(Color::Black);
        arrow.setOutlineThickness(1);
        arrow.setPosition(x, y);
        arrow.setRotation(angle * 180 / 3.14159);
        arrow.setScale(0.5, 0.5);
        window.draw(arrow);
    }

    // iar catre extraterestrii se deseneaza
    // doar 1 px mic cyan
    for (auto &alien : aliens) {
        // analog ca mai sus doar ca bulina va fi mai mica cu 1px si fontu cu 3px
        if (distance(Vector2f(400, 300), alien.location()) < 500) {
            // CircleShape circle(9);
            // circle.setFillColor(Color::Cyan);
            // circle.setOutlineColor(Color::Cyan);
            // circle.setOutlineThickness(1);
            // circle.setPosition(alien.location().x - 4, alien.location().y - 4);
            // circle.setOrigin(4, 4);
            // window.draw(circle);

            continue;
        }

        // altfel:
        float angle = atan2(alien.location().y - 300, alien.location().x - 400);
        float x = cos(angle) * 1000 + 400;
        float y = sin(angle) * 1000 + 300;

        if (x < 10) x = 5;
        if (x > 790) x = 795;
        if (y < 10) y = 5;
        if (y > 590) y = 595;

        RectangleShape arrow(Vector2f(2, 2));
        arrow.setFillColor(Color::Cyan);
        arrow.setPosition(x, y);
        window.draw(arrow);
    }
}

// adauga in afara ecranului cateva monede
// sa vina spre player (drept cadou pt vizita)
void addFreshVisitCoins() {
    float amount = (abs(sin(secondsSince(*gameClock)) * 20) + 30);

    // cout << "DEBUG: Se adauga " << amount << " coins" << endl;
    for (unsigned int i = 0; i < amount; i++) {
        // unghi random 
        auto angle = rand_uniform(0, 2 * 3.14159);
        auto distance = rand_uniform(500, 600);
        auto x = cos(angle) * distance + 400;
        auto y = sin(angle) * distance + 300;
        coins.push_back(make_unique<Coin>(x, y));
    }
}

// ########################################################################
// ########################################################################
// ################### canvasu principal pt gameplay ######################
// ##################### cea mai importanta metoda ########################
// ########################################################################
// ########################################################################
void drawGame(RenderWindow &window) {
    // desenam stelele
    for (auto &star : stars) {
        star.move();
        star.draw(window);
    }

    safeZone = false;

    // desenam planetele
    for (auto &planet : planets) {
        // daca locatia de player e < 1024px de planeta
        if (distance(Vector2f(400, 300), planet->location()) < 1600 + planet->getRadius()) {
            // se deseneaza
            planet->draw(window); // aici s-ar putea sa devina safeZone-ul true
        }
        planet->move();
    }

    // desenam lunile
    for (auto &moon : moons) {
        // daca locatia de player e < 1024px de luna
        if (distance(Vector2f(400, 300), moon->location()) < 1600 + moon->getRadius()) {
            // se deseneaza
            moon->draw(window);
        }
        moon->move();
    }

    // CircleShape mouse(5);
    // mouse.setFillColor(Color::Red);
    // mouse.setPosition(mouseX - 5, mouseY - 5);
    // desenam mouseul (//! DEBUG ONLY)
    // window.draw(mouse);

    // desenarea playerului (sprite-ul cu textura res/player.png)
    // se va roti in functie de unghiul dintre mouse si player
    Sprite playerSprite;
    Texture playerTexture;
    if (!playerAccelerating) {
        playerTexture.loadFromFile("res/player.png");
    } else {
        // efectul de fuel
        if (rand_uniform(0, 1) > 0.5) {
            playerTexture.loadFromFile("res/playeraccelerate1.png");
        } else {
            playerTexture.loadFromFile("res/playeraccelerate2.png");
        }
    }
    playerSprite.setTexture(playerTexture); // are 200x200 deci se va scala la 50x50
    playerSprite.setOrigin(100, 100);
    playerSprite.setPosition(400, 300);
    playerSprite.setRotation(atan2(mouseY - 300, mouseX - 400) * 180 / 3.14159);
    playerSprite.setScale(0.33, 0.33);
    // efectul 3d se creaza prin desenarea mai multor sprite-uri una peste alta
    for (int i = 0; i < 5; i++) {
        playerSprite.setPosition(400, 300 - i * 1);
        playerSprite.setColor(Color(255, 255, 255, i * 50));
        window.draw(playerSprite);
    }

    // scut in jurul playerului
    // opacitatea lui va fi in functie de shield
    CircleShape shield(35);
    shield.setFillColor(Color(9, 9, 255, 100 * player.getShield() / 100));
    shield.setOrigin(35, 35);
    shield.setPosition(400, 300);
    window.draw(shield);
    // outlineul va fi mai puternic
    shield.setFillColor(Color::Transparent);
    shield.setOutlineColor(Color(9, 9, 255 * player.getShield() / 100));
    shield.setOutlineThickness(1);
    window.draw(shield);

    // INAMICII ################
    for (auto &enemy : enemies) {
        enemy.move(); // se misca
        enemy.draw(window); // se deseneaza

        // daca inamicul trage, se adauga un glont
        if (enemy.isShooting()) {
            enemy.shoot(); // se trage si se va seta isShooting pe false
            enemyBullets.push_back(Bullet(enemy.location().x, enemy.location().y, 400, 300, false));
            enemyBullets.back().setDamage(enemy.getDamage());
            enemy.stopShooting(); // ca sa traga doar 1 glontz

            // + sunetu de laser
            soundManager.playSound("enemy_laser");

            // daca inamicul e pe difficult, gloantele lui vor fi mai puternice
            if (enemy.getDifficult()) {
                // creste damage-ul
                enemyBullets.back().setDamage(enemy.getDamage() * 2);
                // la damage se adauga nr de minute jucate * 3
                enemyBullets.back().setDamage(enemyBullets.back().getDamage() + minutesSince(*gameClock) * 3);
                // au splitter
                // enemyBullets.back().setSplitter(true); // todo
                if (rand()%10 < 5) {
                    enemyBullets.back().setZigzag(true);
                }
                // shield-ul creste la fiecare tragere de glontz
                // cu nr de minute jucate
                enemy.setShield(enemy.getShield() + minutesSince(*gameClock));
            }
        }

        // daca-s prea departe, se limiteaza
        if (distance(enemy.location(), Vector2f(400, 300)) > 450) {
            enemy.forcedMoveTowards(Vector2f(400, 300));
        }
    }

    // GLOANTELE ######################
    // loop important
    for (unsigned int bulletIndex = 0; bulletIndex < playerBullets.size(); bulletIndex++) {
        playerBullets[bulletIndex].move();
        playerBullets[bulletIndex].draw(window);

        // aici se sterg cele marcate pentru stergere
        // (adica cele care au lovit sau sunt prea departe)
        if (playerBullets[bulletIndex].needsDeletion()) {
            playerBullets.erase(playerBullets.begin() + bulletIndex);
        }

        // daca e bullet splitter, se cloneaza de doua ori
        if (playerBullets[bulletIndex].canBeCloned()) {
            if (probability_second(.2)) { // adica 5 ori pe secunda
                Bullet cloneOne = playerBullets[bulletIndex];
                ++playerBullets[bulletIndex]; // se incrementeaza contorul de clone
                Bullet cloneTwo = playerBullets[bulletIndex];
                ++playerBullets[bulletIndex]; // se incrementeaza contorul de clone
                cloneOne.setAngle(+3.14159 / 2, true); // se roteste la stanga
                cloneTwo.setAngle(-3.14159 / 2, true); // se roteste la dreapta
                cloneOne.turnOffSplitter();
                cloneTwo.turnOffSplitter();

                // daca sunt freezer bullets, targetul va fi mouseul
                cloneOne.setTarget(Vector2f(mouseX, mouseY));
                cloneTwo.setTarget(Vector2f(mouseX, mouseY));

                playerBullets.push_back(cloneOne);
                playerBullets.push_back(cloneTwo);

                // sunetul de split
                soundManager.playSound("splitter");
            }
        }

        // daca e remote control, se va misca catre mouse
        if (playerBullets[bulletIndex].canControl() && leftClick) {
            playerBullets[bulletIndex] = Vector2f(mouseX, mouseY);
        }

        // daca e homing, se va misca catre inamici
        // adica se schimba targetul la cel mai apropiat inamic
        if (playerBullets[bulletIndex].isHoming()) {
            float minDistance = 200;
            Vector2f target;
            bool found = false;
            for (auto &enemy : enemies) {
                if (distance(playerBullets[bulletIndex].location(), enemy.location()) < minDistance) {
                    minDistance = distance(playerBullets[bulletIndex].location(), enemy.location());
                    target = enemy.location();
                    found = true;
                }
            }
            if (found) {
                playerBullets[bulletIndex]=target;
            }
        }

        // daca e gravitational, va atrage inamicii
        // si stelele (asta e doar vizual tho)
        if (playerBullets[bulletIndex].isGravitational()) {
            for (auto &enemy : enemies) {
                Vector2 bulletLocation = playerBullets[bulletIndex].location();
                Vector2 enemyLocation = enemy.location();
                if (distance(bulletLocation, enemyLocation) < 100) {
                    enemy.forcedMoveTowards(bulletLocation); // si fortat
                    enemy.moveTowards(bulletLocation); // si natural
                }
            }

            for (auto &star : stars) {
                Vector2 bulletLocation = playerBullets[bulletIndex].location();
                Vector2 starLocation = star.location();
                if (distance(bulletLocation, starLocation) < 50) {
                    star += Vector2f((bulletLocation.x - starLocation.x) / 20, (bulletLocation.y - starLocation.y) / 20);
                }
            }

            // also attract enemy bullets
            for (auto &enemyBullet : enemyBullets) {
                Vector2 bulletLocation = playerBullets[bulletIndex].location();
                Vector2 enemyBulletLocation = enemyBullet.location();
                if (distance(bulletLocation, enemyBulletLocation) < 100) {
                    enemyBullet += Vector2f((bulletLocation.x - enemyBulletLocation.x) / 20, (bulletLocation.y - enemyBulletLocation.y) / 20);
                }
            }
        }

        // daca atinge un inamic, se va sterge, va face particulele de impact,
        // si va scadea viata inamicului o valuarea egala cu damage-ul gloantelor
        for (auto &enemy : enemies) {
            if (distance(playerBullets[bulletIndex].location(), enemy.location()) < enemy.getSize()) {
                particleEngine.impact(playerBullets[bulletIndex].location(), 10, playerBullets[bulletIndex].getAngle());
                // daca are shield, va face si particule cyan peste tot
                if (enemy.getShield() > 0) {
                    particleEngine.enemyShieldImpact(enemy.location(), 10, playerBullets[bulletIndex].getAngle());
                }
                enemy.getHit(player.getDamage());
                playerBullets[bulletIndex].hit();

                // knockback
                auto angle = atan2(enemy.location().y - 300, enemy.location().x - 400);
                enemy += Vector2f(cos(angle) * 5, sin(angle) * 5); // knockback-ul subtil
                enemy *= Vector2f(cos(angle) * 3, sin(angle) * 3); // knockback-ul puternic

                // sunetul de impact
                soundManager.playSound("enemy_hit");
            }
        }
    }

    // gloantele inamiciilor ###########
    for (unsigned int bulletIndex = 0; bulletIndex < enemyBullets.size(); bulletIndex++) {
        enemyBullets[bulletIndex].move();
        enemyBullets[bulletIndex].draw(window);

        // aici se sterg cele marcate pentru stergere
        // (adica cele care au lovit sau sunt prea departe)
        if (enemyBullets[bulletIndex].needsDeletion()) {
            enemyBullets.erase(enemyBullets.begin() + bulletIndex);
        }

        // daca atinge playeru, se va sterge, va face particulele de impact,
        // si va scadea viata playerului o valuarea egala cu damage-ul gloantelor
        if (distance(enemyBullets[bulletIndex].location(), Vector2f(400, 300)) < 20) {
            particleEngine.playerImpact(enemyBullets[bulletIndex].location(), 5, enemyBullets[bulletIndex].getAngle(), 30, 20);

            // daca are shield, se va scadea din shield
            if (player.getShield() > 0) {
                player.setShield(player.getShield() - enemyBullets[bulletIndex].getDamage());
                
                // dar tre sa scada si din viata daca shieldu e < 0
                if (player.getShield() < 0) {
                    player.setHealth(player.getHealth() + player.getShield());
                    player.setShield(0);
                    particleEngine.shieldBreak(); // particulele pentru spargerea scutului
                }
            } else {
                // daca nu are shield, se va scadea din viata
                player.setHealth(player.getHealth() - enemyBullets[bulletIndex].getDamage());
            }

            enemyBullets[bulletIndex].hit();
            // + sunetu de player hit
            soundManager.playSound("player_hit");

            // + un knockback unpic
            auto angle = enemyBullets[bulletIndex].getAngle(); /* chiar unghiul glontului */ 
            playerVelocityX += cos(angle) / 2;
            playerVelocityY += sin(angle) / 2;
        }
    }

    // gloantele extraterestrilor ###########
    // pt fiecare extra-terestru
    for (auto &alien : aliens) {
        if (probability_second(2)) { // cam o data la 2 secunde
            alien.shoot(enemies, playerBullets); // (alea-s referinte deci le modifica)
        }
    }

    // desenarea particulelor
    particleEngine.draw(window);
    particleEngine.clear();

    // spawnarea inamicilor
    // va avea loc in partea in care merge playeru
    float catDeDes = 10.0 - secondsSince(*gameClock) / 30.0;
    // asta inseamna ca la 30 secunde de joc, va fi 9 secunde
    // la 1 minut de joc, va fi 8 secunde
    // la 2 minute de joc, va fi 6 secunde
    // la 3 minute de joc, va fi 3 secunde
    // la 4 minute de joc, va fi 2 secunde
    if (catDeDes < 2) catDeDes = 2; // minim 2 secunde ca altfel e prea greu
    
    // daca e safe zone, probabilitatea sa se 
    // spawneze inamicii e 100x mai mica
    if (safeZone) {
        catDeDes *= 100;
    }

    // ! SPAWN INAMICI ! SPAWN INAMICI ! SPAWN INAMICI ! SPAWN INAMICI !
    if (probability_second(catDeDes)) { // adica la fiecare 10 secunde aproape tinde spre 2 secunde

        // inamicul se va spawna in 
        // partea in care merge cel mai rapid
        if (abs(playerVelocityX) > abs(playerVelocityY)) {
            if (playerVelocityX > 0) {
                enemies.push_back(Enemy(800, rand_uniform(0, 600)));
            } else {
                enemies.push_back(Enemy(0, rand_uniform(0, 600)));
            }

        } else {
            if (playerVelocityY > 0) {
                enemies.push_back(Enemy(rand_uniform(0, 800), 600));
            } else {
                enemies.push_back(Enemy(rand_uniform(0, 800), 0));
            }
        }

        // daca a trecut 1 minut de la joc, vor avea cu 50% mai multa viata
        if (secondsSince(*gameClock) > 60) {
            enemies.back().setMaxHealth(enemies.back().getMaxHealth() * 1.5);
            enemies.back().setDifficult(true);
        }

        // sunetul de spawn
        soundManager.playSound("enemy_spawn");
    }

    // stergera inamicilor + spawnarea pickupurilor
    // ! PICKUPS ! PICKUPS ! PICKUPS // ! PICKUPS ! PICKUPS ! PICKUPS // ! PICKUPS ! PICKUPS ! PICKUPS
    // ! PICKUPS ! PICKUPS ! PICKUPS // ! PICKUPS ! PICKUPS ! PICKUPS // ! PICKUPS ! PICKUPS ! PICKUPS
    // ! PICKUPS ! PICKUPS ! PICKUPS // ! PICKUPS ! PICKUPS ! PICKUPS // ! PICKUPS ! PICKUPS ! PICKUPS
    for (int i = 0; i < enemies.size(); i++) {
        if (enemies[i].needsDeletion()) {
            // daca inamicul a murit, se va adauga un pickup random

            // 1 din 4 sanse sa dea fuel
            if (rand_uniform(0, 4) < 1) {
                collectionFuels.push_back(make_unique<pickupFuel>(enemies[i].location().x + rand_uniform(-20, 20), enemies[i].location().y + rand_uniform(-20, 20)));
                collectionFuels.back()->setSpeed(rand_uniform(3,4));
            }

            // 1 din 4 sanse sa dea small shield
            if (rand_uniform(0, 4) < 1) {
                collectionSmallShields.push_back(make_unique<pickupSmallShield>(enemies[i].location().x + rand_uniform(-20, 20), enemies[i].location().y + rand_uniform(-20, 20)));
                collectionSmallShields.back()->setSpeed(rand_uniform(3,4));
            }

            // 1 din 8 sanse sa dea big shield
            if (rand_uniform(0, 8) < 1) {
                collectionBigShields.push_back(make_unique<pickupBigShield>(enemies[i].location().x + rand_uniform(-20, 20), enemies[i].location().y + rand_uniform(-20, 20)));
                collectionBigShields.back()->setSpeed(rand_uniform(3,4));
            }

            // 1 din 4 sanse sa dea small health
            if (rand_uniform(0, 4) < 1) {
                collectionSmallHealths.push_back(make_unique<pickupSmallHealth>(enemies[i].location().x + rand_uniform(-20, 20), enemies[i].location().y + rand_uniform(-20, 20)));
                collectionSmallHealths.back()->setSpeed(rand_uniform(3,4));
            }

            // 1 din 8 sanse sa dea big health
            if (rand_uniform(0, 8) < 1) {
                collectionBigHealths.push_back(make_unique<pickupBigHealth>(enemies[i].location().x + rand_uniform(-20, 20), enemies[i].location().y + rand_uniform(-20, 20)));
                collectionBigHealths.back()->setSpeed(rand_uniform(3,4));
            }

            // 1 din 10 sanse sa dea medkit
            if (rand_uniform(0, 10) < 1) {
                collectionMedkits.push_back(make_unique<pickupMedkit>(enemies[i].location().x + rand_uniform(-20, 20), enemies[i].location().y + rand_uniform(-20, 20)));
                collectionMedkits.back()->setSpeed(rand_uniform(3,4));
            }

            // 1 din 5 sanse sa dea better gun
            if (rand_uniform(0, 5) < 1) {
                collectionBetterGuns.push_back(make_unique<pickupBetterGun>(enemies[i].location().x + rand_uniform(-20, 20), enemies[i].location().y + rand_uniform(-20, 20)));
                collectionBetterGuns.back()->setSpeed(rand_uniform(3,4));
            }

            // 1 din 5 sanse sa dea faster shooting
            if (rand_uniform(0, 5) < 1) {
                collectionFasterShootings.push_back(make_unique<pickupFasterShooting>(enemies[i].location().x + rand_uniform(-20, 20), enemies[i].location().y + rand_uniform(-20, 20)));
                collectionFasterShootings.back()->setSpeed(rand_uniform(3,4));
            }

            // 1 din 7 sanse sa dea boomerang
            if (rand_uniform(0, 7) < 1) {
                collectionBoomerangs.push_back(make_unique<pickupBoomerang>(enemies[i].location().x + rand_uniform(-20, 20), enemies[i].location().y + rand_uniform(-20, 20)));
            }

            // 1 din 7 sanse sa dea freeze
            if (rand_uniform(0, 7) < 1) {
                collectionFreezes.push_back(make_unique<pickupFreeze>(enemies[i].location().x + rand_uniform(-20, 20), enemies[i].location().y + rand_uniform(-20, 20)));
            }

            // 1 din 7 sanse sa dea zigzag
            if (rand_uniform(0, 7) < 1) {
                collectionZigzags.push_back(make_unique<pickupZigzag>(enemies[i].location().x + rand_uniform(-20, 20), enemies[i].location().y + rand_uniform(-20, 20)));
            }

            // 1 din 7 sanse sa dea splitter
            if (rand_uniform(0, 7) < 1) {
                collectionSplitters.push_back(make_unique<pickupSplitter>(enemies[i].location().x + rand_uniform(-20, 20), enemies[i].location().y + rand_uniform(-20, 20)));
            }

            // 1 din 8 sanse sa dea remote control
            if (rand_uniform(0, 8) < 1) {
                collectionRemoteControls.push_back(make_unique<pickupRemoteControl>(enemies[i].location().x + rand_uniform(-20, 20), enemies[i].location().y + rand_uniform(-20, 20)));
            }

            // 1 din 8 sanse sa dea homing
            if (rand_uniform(0, 8) < 1) {
                collectionHomings.push_back(make_unique<pickupHoming>(enemies[i].location().x + rand_uniform(-20, 20), enemies[i].location().y + rand_uniform(-20, 20)));
            }

            // 1 din 9 sanse sa dea gravitational
            if (rand_uniform(0, 9) < 1) {
                collectionGravitationals.push_back(make_unique<pickupGravitational>(enemies[i].location().x + rand_uniform(-20, 20), enemies[i].location().y + rand_uniform(-20, 20)));
            }

            // se adauga si banutzi random intre 5 si 15 da mai probabil spre 5
            float amount = (abs(sin(secondsSince(*gameClock)) * 10) + 5);

            // cout << "DEBUG: Se adauga " << amount << " coins" << endl;
            auto locatieInamic = enemies[i].location();
            for (unsigned int i = 0; i < amount; i++) {
                coins.push_back(make_unique<Coin>(locatieInamic.x + rand_uniform(-5, 5), locatieInamic.y + rand_uniform(-5, 5)));
                // cout << "DEBUG: Se adauga coin la " << locatieInamic.x + rand_uniform(-5, 5) << " " << locatieInamic.y + rand_uniform(-5, 5) << endl;
            }

            // abia apoi se sterge ca sa nu ii afecteze pozitia mai sus
            enemies.erase(enemies.begin() + i);

            // sunetul de moarte
            soundManager.playSound("enemy_dead");

            // seteaza toate planetele drept nevizitate
            for (auto &planet : planets) {
                planet->setVisited(false);
            }
        }
    }

    // cand playerul e aproape de pickup, se executa
    // functia specifica tipului de pickup

    // desenarea si miscarea - small shields ###########
    for (auto &pickup : collectionSmallShields) {
        pickup->draw(window);
        // se deseneaza si un cerc de raza 20 in jurul pickup-ului
        // outlineu e #fff2, vreau doar sa arate hitboxu
        CircleShape outline(20);
        outline.setFillColor(Color::Transparent);
        outline.setOutlineColor(Color(255, 255, 250, 20));
        outline.setOutlineThickness(1);
        outline.setPosition(pickup->location().x - 20, pickup->location().y - 20);
        window.draw(outline);

        // daca playerul e aproape de pickup, se va executa onPickup
        if (distance(pickup->location(), Vector2f(400, 300)) < 20) {
            pickup->onPickup(player); pickup->setDeletion(true); soundManager.playSound("pickup");
            particleEngine.pickup(pickup->location(), 10);
        } pickup->move();
    }
    // stergerea pickup-urilor - small shields ###########
    for (int i = 0; i < collectionSmallShields.size(); i++) {
        if (collectionSmallShields[i]->needsDeletion()) {
            collectionSmallShields.erase(collectionSmallShields.begin() + i);
        }
    }
    // desenarea si miscarea - big shields ###########
    for (auto &pickup : collectionBigShields) {
        pickup->draw(window);
        // se deseneaza si un cerc de raza 20 in jurul pickup-ului
        // outlineu e #fff2, vreau doar sa arate hitboxu
        CircleShape outline(20);
        outline.setFillColor(Color::Transparent);
        outline.setOutlineColor(Color(255, 255, 250, 20));
        outline.setOutlineThickness(1);
        outline.setPosition(pickup->location().x - 20, pickup->location().y - 20);
        window.draw(outline);

        // daca playerul e aproape de pickup, se va executa onPickup
        if (distance(pickup->location(), Vector2f(400, 300)) < 20) {
            pickup->onPickup(player); pickup->setDeletion(true); soundManager.playSound("pickup");
            particleEngine.pickup(pickup->location(), 10);
        } pickup->move();
    }
    // stergerea pickup-urilor - big shields ###########
    for (int i = 0; i < collectionBigShields.size(); i++) {
        if (collectionBigShields[i]->needsDeletion()) {
            collectionBigShields.erase(collectionBigShields.begin() + i);
        }
    }
    // desenarea si miscarea - small healths ###########
    for (auto &pickup : collectionSmallHealths) {
        pickup->draw(window);
        // se deseneaza si un cerc de raza 20 in jurul pickup-ului
        // outlineu e #fff2, vreau doar sa arate hitboxu
        CircleShape outline(20);
        outline.setFillColor(Color::Transparent);
        outline.setOutlineColor(Color(255, 255, 250, 20));
        outline.setOutlineThickness(1);
        outline.setPosition(pickup->location().x - 20, pickup->location().y - 20);
        window.draw(outline);

        // daca playerul e aproape de pickup, se va executa onPickup
        if (distance(pickup->location(), Vector2f(400, 300)) < 20) {
            pickup->onPickup(player); pickup->setDeletion(true); soundManager.playSound("pickup");
            particleEngine.pickup(pickup->location(), 10);
        } pickup->move();
    }
    // stergerea pickup-urilor - small healths ###########
    for (int i = 0; i < collectionSmallHealths.size(); i++) {
        if (collectionSmallHealths[i]->needsDeletion()) {
            collectionSmallHealths.erase(collectionSmallHealths.begin() + i);
        }
    }
    // desenarea si miscarea - big healths ###########
    for (auto &pickup : collectionBigHealths) {
        pickup->draw(window);
        // se deseneaza si un cerc de raza 20 in jurul pickup-ului
        // outlineu e #fff2, vreau doar sa arate hitboxu
        CircleShape outline(20);
        outline.setFillColor(Color::Transparent);
        outline.setOutlineColor(Color(255, 255, 250, 20));
        outline.setOutlineThickness(1);
        outline.setPosition(pickup->location().x - 20, pickup->location().y - 20);
        window.draw(outline);

        // daca playerul e aproape de pickup, se va executa onPickup
        if (distance(pickup->location(), Vector2f(400, 300)) < 20) {
            pickup->onPickup(player); pickup->setDeletion(true); soundManager.playSound("pickup");
            particleEngine.pickup(pickup->location(), 10);
        } pickup->move();
    }
    // stergerea pickup-urilor - big healths ###########
    for (int i = 0; i < collectionBigHealths.size(); i++) {
        if (collectionBigHealths[i]->needsDeletion()) {
            collectionBigHealths.erase(collectionBigHealths.begin() + i);
        }
    }
    // desenarea si miscarea - medkits ###########
    for (auto &pickup : collectionMedkits) {
        pickup->draw(window);
        // se deseneaza si un cerc de raza 20 in jurul pickup-ului
        // outlineu e #fff2, vreau doar sa arate hitboxu
        CircleShape outline(20);
        outline.setFillColor(Color::Transparent);
        outline.setOutlineColor(Color(255, 255, 250, 20));
        outline.setOutlineThickness(1);
        outline.setPosition(pickup->location().x - 20, pickup->location().y - 20);
        window.draw(outline);

        // daca playerul e aproape de pickup, se va executa onPickup
        if (distance(pickup->location(), Vector2f(400, 300)) < 20) {
            pickup->onPickup(player); pickup->setDeletion(true); soundManager.playSound("pickup");
            particleEngine.pickup(pickup->location(), 10);
        } pickup->move();
    }
    // stergerea pickup-urilor - medkits ###########
    for (int i = 0; i < collectionMedkits.size(); i++) {
        if (collectionMedkits[i]->needsDeletion()) {
            collectionMedkits.erase(collectionMedkits.begin() + i);
        }
    }
    // desenarea si miscarea - fuels ###########
    for (auto &pickup : collectionFuels) {
        pickup->draw(window);
        // se deseneaza si un cerc de raza 20 in jurul pickup-ului
        // outlineu e #fff2, vreau doar sa arate hitboxu
        CircleShape outline(20);
        outline.setFillColor(Color::Transparent);
        outline.setOutlineColor(Color(255, 255, 250, 20));
        outline.setOutlineThickness(1);
        outline.setPosition(pickup->location().x - 20, pickup->location().y - 20);
        window.draw(outline);

        // daca playerul e aproape de pickup, se va executa onPickup
        if (distance(pickup->location(), Vector2f(400, 300)) < 20) {
            pickup->onPickup(player); pickup->setDeletion(true); soundManager.playSound("pickup");
            particleEngine.pickup(pickup->location(), 10);
        } pickup->move();
    }
    // stergerea pickup-urilor - fuels ###########
    for (int i = 0; i < collectionFuels.size(); i++) {
        if (collectionFuels[i]->needsDeletion()) {
            collectionFuels.erase(collectionFuels.begin() + i);
        }
    }    
    // desenarea si miscarea - better guns ###########
    for (auto &pickup : collectionBetterGuns) {
        pickup->draw(window);
        // se deseneaza si un cerc de raza 20 in jurul pickup-ului
        // outlineu e #fff2, vreau doar sa arate hitboxu
        CircleShape outline(20);
        outline.setFillColor(Color::Transparent);
        outline.setOutlineColor(Color(255, 255, 250, 20));
        outline.setOutlineThickness(1);
        outline.setPosition(pickup->location().x - 20, pickup->location().y - 20);
        window.draw(outline);

        // daca playerul e aproape de pickup, se va executa onPickup
        if (distance(pickup->location(), Vector2f(400, 300)) < 20) {
            pickup->onPickup(player); pickup->setDeletion(true); soundManager.playSound("pickup");
            particleEngine.pickup(pickup->location(), 10);
        } pickup->move();
    }
    // stergerea pickup-urilor - better guns ###########
    for (int i = 0; i < collectionBetterGuns.size(); i++) {
        if (collectionBetterGuns[i]->needsDeletion()) {
            collectionBetterGuns.erase(collectionBetterGuns.begin() + i);
        }
    }
    // desenarea si miscarea - faster shootings ###########
    for (auto &pickup : collectionFasterShootings) {
        pickup->draw(window);
        // se deseneaza si un cerc de raza 20 in jurul pickup-ului
        // outlineu e #fff2, vreau doar sa arate hitboxu
        CircleShape outline(20);
        outline.setFillColor(Color::Transparent);
        outline.setOutlineColor(Color(255, 255, 250, 20));
        outline.setOutlineThickness(1);
        outline.setPosition(pickup->location().x - 20, pickup->location().y - 20);
        window.draw(outline);

        // daca playerul e aproape de pickup, se va executa onPickup
        if (distance(pickup->location(), Vector2f(400, 300)) < 20) {
            pickup->onPickup(player); pickup->setDeletion(true); soundManager.playSound("pickup");
            particleEngine.pickup(pickup->location(), 10);
        } pickup->move();
    }
    // stergerea pickup-urilor - faster shootings ###########
    for (int i = 0; i < collectionFasterShootings.size(); i++) {
        if (collectionFasterShootings[i]->needsDeletion()) {
            collectionFasterShootings.erase(collectionFasterShootings.begin() + i);
        }
    }
    // desenarea si miscarea - boomerangs ###########
    for (auto &pickup : collectionBoomerangs) {
        pickup->draw(window);
        // se deseneaza si un cerc de raza 20 in jurul pickup-ului
        // outlineu e #fff2, vreau doar sa arate hitboxu
        CircleShape outline(20);
        outline.setFillColor(Color::Transparent);
        outline.setOutlineColor(Color(255, 255, 250, 20));
        outline.setOutlineThickness(1);
        outline.setPosition(pickup->location().x - 20, pickup->location().y - 20);
        window.draw(outline);

        // daca playerul e aproape de pickup, se va executa onPickup
        if (distance(pickup->location(), Vector2f(400, 300)) < 20) {
            pickup->onPickup(player); pickup->setDeletion(true); soundManager.playSound("pickup");
            particleEngine.pickup(pickup->location(), 10);
        } pickup->move();
    }
    // stergerea pickup-urilor - boomerangs ###########
    for (int i = 0; i < collectionBoomerangs.size(); i++) {
        if (collectionBoomerangs[i]->needsDeletion()) {
            collectionBoomerangs.erase(collectionBoomerangs.begin() + i);
        }
    }
    // desenarea si miscarea - freezes ###########
    for (auto &pickup : collectionFreezes) {
        pickup->draw(window);
        // se deseneaza si un cerc de raza 20 in jurul pickup-ului
        // outlineu e #fff2, vreau doar sa arate hitboxu
        CircleShape outline(20);
        outline.setFillColor(Color::Transparent);
        outline.setOutlineColor(Color(255, 255, 250, 20));
        outline.setOutlineThickness(1);
        outline.setPosition(pickup->location().x - 20, pickup->location().y - 20);
        window.draw(outline);

        // daca playerul e aproape de pickup, se va executa onPickup
        if (distance(pickup->location(), Vector2f(400, 300)) < 20) {
            pickup->onPickup(player); pickup->setDeletion(true); soundManager.playSound("pickup");
            particleEngine.pickup(pickup->location(), 10);
        } pickup->move();
    }
    // stergerea pickup-urilor - freezes ###########
    for (int i = 0; i < collectionFreezes.size(); i++) {
        if (collectionFreezes[i]->needsDeletion()) {
            collectionFreezes.erase(collectionFreezes.begin() + i);
        }
    }
    // desenarea si miscarea - zigzags ###########
    for (auto &pickup : collectionZigzags) {
        pickup->draw(window);
        // se deseneaza si un cerc de raza 20 in jurul pickup-ului
        // outlineu e #fff2, vreau doar sa arate hitboxu
        CircleShape outline(20);
        outline.setFillColor(Color::Transparent);
        outline.setOutlineColor(Color(255, 255, 250, 20));
        outline.setOutlineThickness(1);
        outline.setPosition(pickup->location().x - 20, pickup->location().y - 20);
        window.draw(outline);

        // daca playerul e aproape de pickup, se va executa onPickup
        if (distance(pickup->location(), Vector2f(400, 300)) < 20) {
            pickup->onPickup(player); pickup->setDeletion(true); soundManager.playSound("pickup");
            particleEngine.pickup(pickup->location(), 10);
        } pickup->move();
    }
    // stergerea pickup-urilor - zigzags ###########
    for (int i = 0; i < collectionZigzags.size(); i++) {
        if (collectionZigzags[i]->needsDeletion()) {
            collectionZigzags.erase(collectionZigzags.begin() + i);
        }
    }
    // desenarea si miscarea - splitters ###########
    for (auto &pickup : collectionSplitters) {
        pickup->draw(window);
        // se deseneaza si un cerc de raza 20 in jurul pickup-ului
        // outlineu e #fff2, vreau doar sa arate hitboxu
        CircleShape outline(20);
        outline.setFillColor(Color::Transparent);
        outline.setOutlineColor(Color(255, 255, 250, 20));
        outline.setOutlineThickness(1);
        outline.setPosition(pickup->location().x - 20, pickup->location().y - 20);
        window.draw(outline);

        // daca playerul e aproape de pickup, se va executa onPickup
        if (distance(pickup->location(), Vector2f(400, 300)) < 20) {
            pickup->onPickup(player); pickup->setDeletion(true); soundManager.playSound("pickup");
            particleEngine.pickup(pickup->location(), 10);
        } pickup->move();
    }
    // stergerea pickup-urilor - splitters ###########
    for (int i = 0; i < collectionSplitters.size(); i++) {
        if (collectionSplitters[i]->needsDeletion()) {
            collectionSplitters.erase(collectionSplitters.begin() + i);
        }
    }
    // desenarea si miscarea - remote controls ###########
    for (auto &pickup : collectionRemoteControls) {
        pickup->draw(window);
        // se deseneaza si un cerc de raza 20 in jurul pickup-ului
        // outlineu e #fff2, vreau doar sa arate hitboxu
        CircleShape outline(20);
        outline.setFillColor(Color::Transparent);
        outline.setOutlineColor(Color(255, 255, 250, 20));
        outline.setOutlineThickness(1);
        outline.setPosition(pickup->location().x - 20, pickup->location().y - 20);
        window.draw(outline);

        // daca playerul e aproape de pickup, se va executa onPickup
        if (distance(pickup->location(), Vector2f(400, 300)) < 20) {
            pickup->onPickup(player); pickup->setDeletion(true); soundManager.playSound("pickup");
            particleEngine.pickup(pickup->location(), 10);
        } pickup->move();
    }
    // stergerea pickup-urilor - remote controls ###########
    for (int i = 0; i < collectionRemoteControls.size(); i++) {
        if (collectionRemoteControls[i]->needsDeletion()) {
            collectionRemoteControls.erase(collectionRemoteControls.begin() + i);
        }
    }
    // desenarea si miscarea - homings ###########
    for (auto &pickup : collectionHomings) {
        pickup->draw(window);
        // se deseneaza si un cerc de raza 20 in jurul pickup-ului
        // outlineu e #fff2, vreau doar sa arate hitboxu
        CircleShape outline(20);
        outline.setFillColor(Color::Transparent);
        outline.setOutlineColor(Color(255, 255, 250, 20));
        outline.setOutlineThickness(1);
        outline.setPosition(pickup->location().x - 20, pickup->location().y - 20);
        window.draw(outline);

        // daca playerul e aproape de pickup, se va executa onPickup
        if (distance(pickup->location(), Vector2f(400, 300)) < 20) {
            pickup->onPickup(player); pickup->setDeletion(true); soundManager.playSound("pickup");
            particleEngine.pickup(pickup->location(), 10);
        } pickup->move();
    }
    // stergerea pickup-urilor - homings ###########
    for (int i = 0; i < collectionHomings.size(); i++) {
        if (collectionHomings[i]->needsDeletion()) {
            collectionHomings.erase(collectionHomings.begin() + i);
        }
    }
    // desenarea si miscarea - gravitationals ###########
    for (auto &pickup : collectionGravitationals) {
        pickup->draw(window);
        // se deseneaza si un cerc de raza 20 in jurul pickup-ului
        // outlineu e #fff2, vreau doar sa arate hitboxu
        CircleShape outline(20);
        outline.setFillColor(Color::Transparent);
        outline.setOutlineColor(Color(255, 255, 250, 20));
        outline.setOutlineThickness(1);
        outline.setPosition(pickup->location().x - 20, pickup->location().y - 20);
        window.draw(outline);

        // daca playerul e aproape de pickup, se va executa onPickup
        if (distance(pickup->location(), Vector2f(400, 300)) < 20) {
            pickup->onPickup(player); pickup->setDeletion(true); soundManager.playSound("pickup");
            particleEngine.pickup(pickup->location(), 10);
        } pickup->move();
    }
    // stergerea pickup-urilor - gravitationals ###########
    for (int i = 0; i < collectionGravitationals.size(); i++) {
        if (collectionGravitationals[i]->needsDeletion()) {
            collectionGravitationals.erase(collectionGravitationals.begin() + i);
        }
    }

    // desenarea si miscarea banilor
    for (auto &coin : coins) {
        coin->draw(window);
        // se deseneaza si un cerc de raza 10 in jurul banului
        // outlineu e #fff2, vreau doar sa arate hitboxu
        CircleShape outline(10);
        outline.setFillColor(Color::Transparent);
        outline.setOutlineColor(Color(255, 255, 250, 20));
        outline.setOutlineThickness(1);
        outline.setPosition(coin->location().x - 10, coin->location().y - 10);
        window.draw(outline);

        coin->move();
    }
    // stergerea banilor
    for (int i = 0; i < coins.size(); i++) {
        if (coins[i]->needsDeletion()) {
            coins.erase(coins.begin() + i);
        }
    }

    // desenarea extraterestrilor
    for (auto &alien : aliens) {
        alien.move(aliens); // <--- e transmis prin parametru ca sa stie unde s restu si cum se misca

        // daca distanta de player e peste 666 (easter egg), skip
        if (distance(alien.location(), Vector2f(400, 300)) > 666) {
            continue; // optimizare practic
        }

        alien.draw(window);
    }

    drawPlayerInfo(window); // desenarea proprietatiilor playerului

    drawArrowsTowardsPlanets(window); // desenarea sagetilor care arata catre planete

    if (firePause>0) firePause--; // se scade firepause-ul (al playerului) pentru fireRate regulat

    controls(window); // controalele si actiunile lor

    // moartea playerului
    if (player.getHealth() < 1) {
        // se va face un efect de explozie
        particleEngine.explosion(Vector2f(400, 300), 50, 15);
        // si se va reseta jocu
        inMenu = true;
        // se sterg gloantele, inamicii, etc
        playerBullets.clear();
        enemyBullets.clear();
        enemies.clear();
        // se reseta playeru
        player = Player();
        // sunetu de moarte
        soundManager.playSound("end_game");

        // se sterg toate pickupurile
        collectionSmallShields.clear();
        collectionBigShields.clear();
        collectionSmallHealths.clear();
        collectionBigHealths.clear();

        collectionMedkits.clear();
        collectionFuels.clear();
        collectionBetterGuns.clear();
        collectionFasterShootings.clear();
        
        collectionBoomerangs.clear();
        collectionFreezes.clear();
        collectionZigzags.clear();
        collectionSplitters.clear();
        collectionRemoteControls.clear();
        collectionHomings.clear();
        collectionGravitationals.clear();

        gameClock->reset();
        // cout << "Game clock resetat: stamp=" << secondsSince(*gameClock) << endl;

        // // coordonatele memorate
        // player.setX(0);
        // player.setY(0);

        // se sterg banii
        coins.clear();


        // loadPlanets();

        // messagebox cu scorul (money)
        string messageBoxMessage = "You died! Your score was: " + to_string(money);
        MessageBox(NULL, messageBoxMessage.c_str(), "GAME OVER", MB_OK | MB_ICONINFORMATION);

        // acum intreaba daca pariaza jumate din ei: buton yes si no
        int result = IDNO;

        if (money > 10) {
            result = MessageBox(NULL, "Do you want to bet half of your money?", "Gamble", MB_YESNO | MB_ICONQUESTION);
        }

        // daca alege yes
        if (result == IDYES) {
            auto won = rand_uniform(0, 1) < 0.45 ? true : false; // 45% sansa de castig
            
            if (won) {
                money += money / 2;
                string winText = "You won! Your final score is: " + to_string(money);
                MessageBox(NULL, winText.c_str(), "You won!", MB_OK | MB_ICONINFORMATION);
            } else {
                money -= money / 2;
                string loseText = "You lost! Your final score is: " + to_string(money);
                MessageBox(NULL, loseText.c_str(), "You lost!", MB_OK | MB_ICONINFORMATION);
            }
            // se appenduieste la scores.txt
            // [data] - [ora]:[minutu]:[secunda] - [money]
            // daca nu exista scores.txt, se creeaza
            ofstream scoresFile("scores.txt", ios::app);
            time_t now = time(0);
            tm *ltm = localtime(&now);
            scoresFile << 1900 + ltm->tm_year << "-" << 1 + ltm->tm_mon << "-" << ltm->tm_mday << " - " << ltm->tm_hour << ":" << ltm->tm_min << ":" << ltm->tm_sec << " - " << money << endl;
            scoresFile.close();
        }

        // se sterg toate proiectilele
        money = 0;

        // light speed off
        lightSpeedOn = false;
        trailEffect = false;
    }

    // playerul se regenereaza, dar shieldul nu
    if (player.getHealth() < 100) {
        player.setHealth(player.getHealth() + rand_uniform(0, 0.01));
    }

    // update: se regenereaza si shieldul cand e in safezone
    if (safeZone && player.getShield() < 100) {
        player.setShield(player.getShield() + rand_uniform(0, 0.07));

        // si se deseneaza un cerc verde in jurul lui
        // fill #0f01, outline #0f0
        CircleShape outline(20);
        outline.setFillColor(Color(0, 15, 1, 1));
        outline.setOutlineColor(Color(0, 15, 0, 100));
        outline.setOutlineThickness(1);
        outline.setOrigin(20, 20);
        outline.setPosition(400, 300);
        window.draw(outline);

        // si fuel-ul creste putin
        player.setFuel(player.getFuel() + rand_uniform(0, 0.1));
    }

    // daca playeru e deasupra unei planete
    if (safeZone) {
        // iar extraterestrii se afla pe aceeasi planeta
        for (auto &alien : aliens) {
            if (probability_second(8)) { // cam la fiecare 10 secunde. are multe calcule de facut
                if (distance(alien.location(), Vector2f(400, 300)) > 800) {
                    continue;
                }

                // daca playerul e aproape (distanta < 800)
                std::unique_ptr<Planet>* planetSearch = nullptr;
                // search by id
                for (auto& planet : planets) {
                    if (planet->getId() == planetPlayerIsOn) {
                        planetSearch = &planet; // ii luam referinta (din unique_ptr)
                        break;
                    }
                }
                if (planetSearch != nullptr) {
                    alien.talkSomething(*planetSearch); // ii dam pass la ref
                }
            }
        }
    }

    // daca playeru visitase ceva recent, adauga banii
    if (freshVisit) {
        addFreshVisitCoins();
        freshVisit = false;
    }

    // daca playeru e pe o luna, se adauga fuel
    if (moonEnergy) {
        if (player.getFuel() < player.getMaxFuel()) { // daca nu e full (300 e full)
            player.setFuel(player.getFuel() + 1);
        }
        moonEnergy = false;
    }

    // daca viteza luminii e folosita, se scade fuel
    if (lightSpeedOn && player.getFuel() > 0) {
        player.setFuel(player.getFuel() - 1);
    }

    // limit fuel to 0
    if (player.getFuel() < 0) {
        player.setFuel(0);
    }

    // daca fuelu < 50%, stop lightspeed
    if (player.getFuel() < player.getMaxFuel() / 2) {
        lightSpeedOn = false;
        trailEffect = false;
    }
}

void drawPause(RenderWindow &window) {
    // poza de fundal
    Texture pauseBg;
    pauseBg.loadFromFile("res/pausebg.png");
    Sprite pauseBgSprite(pauseBg);
    window.draw(pauseBgSprite);

    // p = se inchide pauza
    if (keysPressed[25]) {
        gamePaused = false;
    }
}

// functia principala unde se va desena jocu
void draw(RenderWindow &window) {
    if (inMenu) {
        drawMenu(window);
    } else {
        if (!gamePaused) {
            drawGame(window);
        } else {
            drawPause(window);

            if (keysPressed[15]) {
                gamePaused = false;
                keysPressed[15] = false;
            }
        }
    }
}

// -------------------------------------------------------------------- MAIN --------------------------------------------------------------------

int main() {
    // da cancel la determinism oarecum
    srand(time(NULL));

    // fereastra principala
    RenderWindow window(VideoMode(800, 600), "Space Defense", sf::Style::Resize);
    // engine-ul de sunete facut de mine (instanta lui)

    // ascunderea ferestrelor
    hideAllCmdWindows();

    // exitAllCmdWindows(); //* nvm
    
    // fps 60 e max (atat imi duce monitoru oricum)
    window.setFramerateLimit(FPS_LIMIT);
    // setare window icon
    Texture icon;
    icon.loadFromFile("res/icon.png");
    window.setIcon(525, 529, icon.copyToImage().getPixelsPtr());

    init();

    while (window.isOpen()) { // cat timp fereastra e deschisa
        Event event;
        while (window.pollEvent(event)) {
            // cred ca e good practice sa pun eventu de inchidere 
            if (event.type == Event::Closed)
                window.close();
            
            // cand mouseul e apasat
            if (event.type == Event::MouseButtonPressed) {
                mouseDown = true;

                // ce buton e apasat
                if (event.mouseButton.button == Mouse::Left) {
                    leftClick = true; // stanga e apasata
                }
                if (event.mouseButton.button == Mouse::Right) {
                    rightClick = true; // dreapta e apasata
                }
            }

            // la fiecare mouse move se schimba coordonatele mouseului
            if (event.type == Event::MouseMoved) {
                mouseX = event.mouseMove.x;
                mouseY = event.mouseMove.y;
            }

            // cand nu mai e apasat
            if (event.type == Event::MouseButtonReleased && event.mouseButton.button == Mouse::Left) {
                leftClick = false;
            }
            if (event.type == Event::MouseButtonReleased && event.mouseButton.button == Mouse::Right) {
                rightClick = false;
            }
            if (event.type == Event::MouseButtonReleased) {
                mouseDown = leftClick || rightClick;
            }

            // cand playeru apasa tasta N se adauga in vectoru de taste apasate drept true
            if (event.type == Event::KeyPressed) {
                keysPressed[event.key.code] = true;
                // cout << "Cod tasta: " << event.key.code << "\n";
            }
            if (event.type == Event::KeyReleased) {
                keysPressed[event.key.code] = false;
                // cout << "RIDICAT Cod tasta: " << event.key.code << "\n";
            }
        }

        if (!trailEffect) window.clear();
        else {
            RectangleShape trail(Vector2f(800, 600));
            trail.setFillColor(Color(0, 0, 0, 5));
            window.draw(trail);
        }
        draw(window);
        window.display();
    }

    exitAllCmdWindows(); // sa nu ramana procesele degeaba deschise
    return 0;
}
