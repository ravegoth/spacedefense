// in acest fisier doar testez clasele
// nu va fi folosit in joc

#include <iostream>
#include <thread>
#include <chrono>

#include "clock.h"
#include "brain.h"
#include "sounds.h" 
#include "textgen.h"
#include "colorconvert.h"

using namespace std;

int main() {
    cout << "Testing ended.";

    // auto color = hslToRGB<int>(123, 50, 30);

    // cout << "RGB: " << color[0] << " " << color[1] << " " << color[2] << endl;

    // Language * extraterestra = new Language();
    // TextGen & gen = TextGen::getInstance();

    // // 10 nume de planete
    // for (int i = 0; i < 10; i++) {
    //     cout << gen.generatePlanetName() << endl;
    // }

    // for (int i = 0; i < 10; i++) {
    //     cout << gen+1 << endl;
    // }

    // cout << "\nText in the language: " << extraterestra << ": \n\n";

    // for (int i = 0; i < 10; i++) {
    //     string generated = gen + extraterestra;
    //     cout << generated << " ---> scor: " << *extraterestra % generated << endl;
    // }

    // cout << "\nAnti-texts in the language: " << extraterestra << ": \n\n";

    // for (int i = 0; i < 10; i++) {
    //     string generated = gen - extraterestra;
    //     cout << generated << " ---> scor: " << *extraterestra % generated << endl;
    // }

    // cout << "\nrandGens in the language: " << extraterestra << ": \n\n";

    // for (int i = 0; i < 10; i++) {
    //     string generated = randGen(3);

    //     while (extraterestra->check(generated) > 0.99) {
    //         generated = randGen(3);
    //     }

    //     cout << generated << " ---> scor: " << *extraterestra % generated << endl;
    // }

    // cout << "\nCustom test: \n\n";

    // cout << "aaa = " << *extraterestra % "aaa" << endl;
    // cout << "aab = " << *extraterestra % "aab" << endl;
    // cout << "aba = " << *extraterestra % "aba" << endl;
    // cout << "baa = " << *extraterestra % "baa" << endl;
    // cout << "abb = " << *extraterestra % "abb" << endl;
    // cout << "bab = " << *extraterestra % "bab" << endl;
    // cout << "bba = " << *extraterestra % "bba" << endl;
    // cout << "bbb = " << *extraterestra % "bbb" << endl;
    // cout << "xyz = " << *extraterestra % "xyz" << endl;
    // cout << "xx y = " << *extraterestra % "xx y" << endl;
    // cout << "yy z = " << *extraterestra % "yy z" << endl;
    // cout << "zz x = " << *extraterestra % "zz x" << endl;
    // cout << "aaaaaa aaaa = " << *extraterestra % "aaaaaa aaaa" << endl;
    // cout << "aaaaaa aaaaa = " << *extraterestra % "aaaaaa aaaaa" << endl;
    // cout << "aaaaaa aaaaaa = " << *extraterestra % "aaaaaa aaaaaa" << endl;
    // cout << "aaaaaa aaab = " << *extraterestra % "aaaaaa aaab" << endl;
    // cout << "aaaaaa baab = " << *extraterestra % "aaaaaa baab" << endl;
    // cout << "aaaaab baab = " << *extraterestra % "aaaaab baab" << endl;
    // cout << "aaaaac baab = " << *extraterestra % "aaaaac baab" << endl;
    // cout << "aaaaac caac = " << *extraterestra % "aaaaac caac" << endl;

    // cout << "Testing frmo aaaa to zzzz:\n";
    // for (char i = 'a'; i <= 'z'; i++) {
    //     for (char j = 'a'; j <= 'z'; j++) {
    //         for (char k = 'a'; k <= 'z'; k++) {
    //             for (char l = 'a'; l <= 'z'; l++) {
    //                 string generated = string(1, i) + string(1, j) + string(1, k) + string(1, l);
    //                 if ((*extraterestra % generated) > 0.9999) {
    //                     cout << generated << " ---> scor: " << *extraterestra % generated << endl;
    //                 }
    //             }
    //         }
    //     }
    // }

    // while (1) {
    //     cout << "Enter text to check score: ";

    //     string text;
    //     cin >> text;

    //     if (text == "exit") {
    //         break;
    //     }

    //     cout << "Scor: " << *extraterestra % text << endl;
    // }

    // delete extraterestra;
    // delete &gen;

    // Perceptron * brain = new Perceptron();
    // brain->init(2);
    // brain->zero();

    // cout << "Pentru {0, 0}, se afiseaza: " << brain->calculus({0, 0}) << "\n";
    // cout << "Pentru {1, 0}, se afiseaza: " << brain->calculus({1, 0}) << "\n";
    // cout << "Pentru {0, 1}, se afiseaza: " << brain->calculus({0, 1}) << "\n";

    // brain->mutate(1);
    // cout << "Mutate...\n";

    // cout << "Pentru {0, 0}, se afiseaza: " << brain->calculus({0, 0}) << "\n";
    // cout << "Pentru {1, 0}, se afiseaza: " << brain->calculus({1, 0}) << "\n";
    // cout << "Pentru {0, 1}, se afiseaza: " << brain->calculus({0, 1}) << "\n";

    // cout << "Learning...\n";
    // auto error = brain->learn({{0, 0}, {1, 0}, {0, 1}, {1, 1}}, {0, 1, 1, 0}, 0.1);
    // while (error > 0.1) {
    //     error = brain->learn({{0, 0}, {1, 0}, {0, 1}, {1, 1}}, {0, 1, 1, 0}, 0.1);
    // }
    // cout << "Done. Error: " << error << "\n";

    // cout << "Pentru {0, 0}, se afiseaza: " << brain->calculus({0, 0}) << "\n";
    // cout << "Pentru {1, 0}, se afiseaza: " << brain->calculus({1, 0}) << "\n";
    // cout << "Pentru {0, 1}, se afiseaza: " << brain->calculus({0, 1}) << "\n";
    // cout << "Pentru {1, 1}, se afiseaza: " << brain->calculus({1, 1}) << "\n";

    // cout << "Others:\n";

    // cout << "Pentru {0.5, 0.5}, se afiseaza: " << brain->calculus({0.5, 0.5}) << "\n";
    // cout << "Pentru {0.5, -0.5}, se afiseaza: " << brain->calculus({0.5, -0.5}) << "\n";
    // cout << "Pentru {-0.5, 0.5}, se afiseaza: " << brain->calculus({-0.5, 0.5}) << "\n";
    // cout << "Pentru {-0.5, -0.5}, se afiseaza: " << brain->calculus({-0.5, -0.5}) << "\n";

    // cout << "----------------\n";
    // cout << *brain;
    // cout << "----------------\n";

    // brain->init(3);
    // brain->zero();
    // brain->iie();

    // cout << "Pentru {0, 0, 0}, se afiseaza: " << brain->calculus({0, 0, 0}) << "\n";
    // cout << "Pentru {0, 0, 1}, se afiseaza: " << brain->calculus({0, 0, 1}) << "\n";
    // cout << "Pentru {0, 1, 0}, se afiseaza: " << brain->calculus({0, 1, 0}) << "\n";
    // cout << "Pentru {0, 1, 1}, se afiseaza: " << brain->calculus({0, 1, 1}) << "\n";
    // cout << "Pentru {1, 0, 0}, se afiseaza: " << brain->calculus({1, 0, 0}) << "\n";
    // cout << "Pentru {1, 0, 1}, se afiseaza: " << brain->calculus({1, 0, 1}) << "\n";
    // cout << "Pentru {1, 1, 0}, se afiseaza: " << brain->calculus({1, 1, 0}) << "\n";
    // cout << "Pentru {1, 1, 1}, se afiseaza: " << brain->calculus({1, 1, 1}) << "\n";

    // cout << "Learning...\n";
    // // 1 if the inputs have only one 1
    // error = brain->learn({{0, 0, 0}, {0, 0, 1}, {0, 1, 0}, {0, 1, 1},
    //                     {1, 0, 0}, {1, 0, 1}, {1, 1, 0}, {1, 1, 1}},
    //                     {0, 1, 1, 0, 1, 0, 0, 0}, 0.1);
    // while (error > 1.1) {
    //     error = brain->learn({{0, 0, 0}, {0, 0, 1}, {0, 1, 0}, {0, 1, 1},
    //                     {1, 0, 0}, {1, 0, 1}, {1, 1, 0}, {1, 1, 1}},
    //                     {0, 1, 1, 0, 1, 0, 0, 0}, 0.1);
    //     cout << "Error: " << error << endl;
    // }
    // cout << "Done. Error: " << error << "\n";

    // cout << "Pentru {0, 0, 0}, se afiseaza: " << brain->calculus({0, 0, 0}) << "\n";
    // cout << "Pentru {0, 0, 1}, se afiseaza: " << brain->calculus({0, 0, 1}) << "\n";
    // cout << "Pentru {0, 1, 0}, se afiseaza: " << brain->calculus({0, 1, 0}) << "\n";
    // cout << "Pentru {0, 1, 1}, se afiseaza: " << brain->calculus({0, 1, 1}) << "\n";
    // cout << "Pentru {1, 0, 0}, se afiseaza: " << brain->calculus({1, 0, 0}) << "\n";
    // cout << "Pentru {1, 0, 1}, se afiseaza: " << brain->calculus({1, 0, 1}) << "\n";
    // cout << "Pentru {1, 1, 0}, se afiseaza: " << brain->calculus({1, 1, 0}) << "\n";
    // cout << "Pentru {1, 1, 1}, se afiseaza: " << brain->calculus({1, 1, 1}) << "\n";

    // brain->exportData();

    // delete brain;

    // ClockStamp<int> *clock = new ClockStamp<int>();
    // MinuteClockStamp<int> *minClock = new MinuteClockStamp<int>();

    // cout << "Diferenta: " << clock->getDiff() << endl;
    // cout << "MDiferenta: " << minClock->getDiff() << endl;

    // // sleep 10 secunde

    // this_thread::sleep_for(70s);

    // cout << "Diferenta: " << secundeDeLa(*clock) << endl;
    // cout << "MDiferenta: " << secundeDeLa(*minClock) << endl;
    // cout << "Diferenta: " << minuteDeLa(*clock) << endl;
    // cout << "MDiferenta: " << minuteDeLa(*minClock) << endl;

    // cout << "Diferenta: " << clock->getDiff() << endl;
    // cout << "MDiferenta: " << minClock->getDiff() << endl;

    // cout << "\n\n\n\n\nTEST\n";

    // TextGen& gen = TextGen::getInstance();

    // for (int i = 0; i < 5; i++) {
    //     cout << gen.generatePlanetName() << endl;
    // }

    // cout << "\n";

    // for (int i = 0; i < 5; i++) {
    //     cout << gen.generateSpaceshipName() << endl;
    // }

    // cout << "\n";

    // string propozitie = gen.generatePhrase(1);

    // cout << propozitie << endl;

    // cout << "\nEND OF TEST\n"; // ca sa mi dau seama cand da crash

    // SoundEngine& engine = SoundEngine::getInstance();

    // engine.defineSound("enemy_spawn", "./res/enemy_spawn.wav");

    // cout << "Duratie: " << engine.getSoundDuration("enemy_spawn") << " seconds\n";

    // if (engine.playSound("enemy_spawn")) {
    //     cout << "a mers";
    // } else {
    //     cout << "nu a mers";
    // }

    // // timeout exact cat dureaza sunetu
    // // ca aparent tre sa traiasca bufferu
    // this_thread::sleep_for(engine.getSoundDuration("enemy_spawn") * 1s);

    // Perceptron *p = new Perceptron();
    // vector<float> inputs;
    // float error;

    // p->init(2);

    // // xor
    // vector<vector<float>> trainingSet = {
    //     {-1, -1},
    //     {-1, 1},
    //     {1, -1},
    //     {1, 1}
    // };

    // vector<float> targets = {-1, 1, 1, -1};

    // cout << "Pentru {-1, -1}, se afiseaza: " << p->calculus({-1, -1}) << "\n";
    // cout << "Pentru {-1, 1}, se afiseaza: " << p->calculus({-1, 1}) << "\n";
    // cout << "Pentru {1, -1}, se afiseaza: " << p->calculus({1, -1}) << "\n";
    // cout << "Pentru {1, 1}, se afiseaza: " << p->calculus({1, 1}) << "\n";

    // cout << "Se invata...\n";

    // size_t counter = 999;
    // while (counter--) {
    //     error = p->learn(trainingSet, targets);
    //     cout << "Error: " << error << endl;
    // }

    // cout << "Pentru {-1, -1}, se afiseaza: " << p->calculus({-1, -1}) << "\n";
    // cout << "Pentru {-1, 1}, se afiseaza: " << p->calculus({-1, 1}) << "\n";
    // cout << "Pentru {1, -1}, se afiseaza: " << p->calculus({1, -1}) << "\n";
    // cout << "Pentru {1, 1}, se afiseaza: " << p->calculus({1, 1}) << "\n";

    // // while (error = p->learn(trainingSet, targets) > 0.1) {
    // //     cout << "Error: " << error << endl;
    // // }

    // for (int i = 0; i < trainingSet.size(); i++) {
    //     cout<< "input: " << trainingSet[i][0] << " " << trainingSet[i][1]
    //         << " -> " << p->calculus(trainingSet[i]) << endl;
    // }

    // delete p;
}