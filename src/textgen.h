/* headerul folosit pentru generarea numelor de planete, etc... */

#include <string>
#include <vector>
#include <ctime>
#include <cstdlib>

using namespace std;

// ==============================================
// ===== FUNCTII AUJTATOARE =====================
// ==============================================

// imparte un string in functie de un delimitator
vector<string> splitString(string str, string delimiter) {
    vector<string> result;
    size_t pos = 0;
    string token;
    while ((pos = str.find(delimiter)) != string::npos) {
        token = str.substr(0, pos);
        result.push_back(token);
        str.erase(0, pos + delimiter.length());
    }
    return result;
}

// face prima litera din string mare
string capitalize(string str) {
    if (str.size() == 0) {
        return str;
    }

    str[0] = toupper(str[0]);
    return str;
}

// capitalizeaza si dupa spatiu sau cratima
string doubleCapitalize(string str) {
    if (str.size() == 0) {
        return str;
    }

    str[0] = toupper(str[0]);

    for (unsigned int i = 1; i < str.size(); i++) {
        if (str[i] == ' ' || str[i] == '-') {
            str[i + 1] = toupper(str[i + 1]);
        }
    }

    return str;
}

// generator complet aleatoriu de text (nu va fi folosit in joc)
string randGen(unsigned int length) {
    string text = "";
    for (unsigned int i = 0; i < length; i++) {
        text += (char)(rand() % 26 + 'a');
    }
    return text;

}

// nr de vocale / nr total de litere
float vowelsPerLetters(string text) {
    unsigned int vowels = 0;
    unsigned int letters = 0;

    // conversie in lowercase
    for (unsigned int i = 0; i < text.size(); i++) {
        if (text[i] >= 'A' && text[i] <= 'Z') {
            text[i] = text[i] - 'A' + 'a';
        }
    }

    for (unsigned int i = 0; i < text.size(); i++) {
        if (text[i] == 'a' || text[i] == 'e' || text[i] == 'i' || text[i] == 'o' || text[i] == 'u') {
            vowels++;
        }

        if (text[i] >= 'a' && text[i] <= 'z') {
            letters++;
        }
    }

    return (float)vowels / (float)letters;
}

// nr de consoane / nr total de litere
float consonantsPerLetters(string text) {
    unsigned int consonants = 0;
    unsigned int letters = 0;

    // conversie in lowercase
    for (unsigned int i = 0; i < text.size(); i++) {
        if (text[i] >= 'A' && text[i] <= 'Z') {
            text[i] = text[i] - 'A' + 'a';
        }
    }

    for (unsigned int i = 0; i < text.size(); i++) {
        if (text[i] >= 'a' && text[i] <= 'z' && text[i] != 'a' && text[i] != 'e' && text[i] != 'i' && text[i] != 'o' && text[i] != 'u') {
            consonants++;
        }

        if (text[i] >= 'a' && text[i] <= 'z') {
            letters++;
        }
    }

    return (float)consonants / (float)letters;
}

// nr de cratime
unsigned int countDashes(string text) {
    unsigned int dashes = 0;

    for (unsigned int i = 0; i < text.size(); i++) {
        if (text[i] == '-') {
            dashes++;
        }
    }

    return dashes;
}

// nr de cuvinte
unsigned int countWords(string text) {
    unsigned int words = 0;

    for (unsigned int i = 0; i < text.size(); i++) {
        if (text[i] == ' ' || text[i] == '-') {
            words++;
        }
    }

    return words + 1;
}

// vocala cea mai des intalnita (ascii codeu)
char mostUsedVowel(string text) {
    unsigned int vowels[5] = {0, 0, 0, 0, 0};

    for (unsigned int i = 0; i < text.size(); i++) {
        if (text[i] == 'a') {
            vowels[0]++;
        }

        if (text[i] == 'e') {
            vowels[1]++;
        }

        if (text[i] == 'i') {
            vowels[2]++;
        }

        if (text[i] == 'o') {
            vowels[3]++;
        }

        if (text[i] == 'u') {
            vowels[4]++;
        }
    }

    unsigned int max = 0;
    unsigned int maxIndex = 0;

    for (unsigned int i = 0; i < 5; i++) {
        if (vowels[i] > max) {
            max = vowels[i];
            maxIndex = i;
        }
    }

    return "aeiou"[maxIndex];
}

// consoana cea mai des intalnita (ascii codeu)
char mostUsedConsonant(string text) {
    unsigned int consonants[21];
    for (unsigned int i = 0; i < 21; i++) {
        consonants[i] = 0;
    }

    for (unsigned int i = 0; i < text.size(); i++) {
        if (text[i] >= 'a' && text[i] <= 'z' && text[i] != 'a' && text[i] != 'e' && text[i] != 'i' && text[i] != 'o' && text[i] != 'u') {
            consonants[text[i] - 'a']++;
        }
    }

    unsigned int max = 0;
    unsigned int maxIndex = 0;

    for (unsigned int i = 0; i < 21; i++) {
        if (consonants[i] > max) {
            max = consonants[i];
            maxIndex = i;
        }
    }

    return 'a' + maxIndex;
}

// nr de litere mediu pe cuvant
float averageLettersPerWord(string text) {
    unsigned int words = countWords(text);
    unsigned int letters = 0;

    for (unsigned int i = 0; i < text.size(); i++) {
        if (text[i] >= 'a' && text[i] <= 'z') {
            letters++;
        }
    }

    return (float)letters / (float)words;
}

// nr de litere mediu pe fraza
float averageLettersPerPhrase(string text) {
    unsigned int phrases = 0;
    unsigned int letters = 0;

    for (unsigned int i = 0; i < text.size(); i++) {
        if (text[i] == '.' || text[i] == '!' || text[i] == '?') {
            phrases++;
        }

        if (text[i] >= 'a' && text[i] <= 'z') {
            letters++;
        }
    }

    if (phrases == 0) {
        return 0;
    }

    return (float)letters / (float)phrases;
}

// cea mai folosita litera
char mostUsedLetter(string text) {
    unsigned int letters[26];
    for (unsigned int i = 0; i < 26; i++) {
        letters[i] = 0;
    }

    for (unsigned int i = 0; i < text.size(); i++) {
        if (text[i] >= 'a' && text[i] <= 'z') {
            letters[text[i] - 'a']++;
        }
    }

    unsigned int max = 0;
    unsigned int maxIndex = 0;

    for (unsigned int i = 0; i < 26; i++) {
        if (letters[i] > max) {
            max = letters[i];
            maxIndex = i;
        }
    }

    return 'a' + maxIndex;
}

// verificator realist pt nume realist
string validateName(string name) {
    // repara erorile gen doua cratime una langa alta
    for (unsigned int i = 0; i < name.size() - 1; i++) {
        if (name[i] == '-' && name[i + 1] == '-') {
            name.erase(i, 1);
        }
    
        if (name[i] == ' ' && name[i + 1] == ' ') {
            name.erase(i, 1);
        }

        if (name[i] == ' ' && name[i + 1] == '-') {
            name.erase(i, 1);
        }
    }

    // daca incepe cu cratima sau spatiu, le sterge
    // * totusi cred ca e imposibil
    if (name[0] == '-' || name[0] == ' ') {
        name.erase(0, 1);
    }

    // daca se termina cu cratima sau spatiu, le sterge
    // * si asta e imposibil
    if (name[name.size() - 1] == '-' || name[name.size() - 1] == ' ') {
        name.erase(name.size() - 1, 1);
    }

    return name;
}

// ==============================================
// ===== CLASE ==================================
// ==============================================

// clasa aceasta reprezinta un stil de a vorbi
class Language {
private:
    slowSinPerceptron * lang = new slowSinPerceptron();
    // parametrii limbii:
    // - 0: nr de vocale / nr total de litere
    // - 1: nr de consoane / nr total de litere
    // - 2: nr de cratime
    // - 3: nr de cuvinte
    // - 4: vocala cea mai des intalnita (ascii codeu)
    // - 5: consoana cea mai des intalnita (ascii codeu)
    // - 6: nr de litere mediu pe cuvant
    // - 7: nr de litere mediu pe fraza
    // - 8: cea mai folosita litera
public:
    // constructor
    Language() {
        lang->init(9);
    }

    // constr cu parametru
    Language(slowSinPerceptron * brain) {
        lang->init(9);
        lang->beLike(*brain);
    }

    // copy constructor
    Language(const Language& other) {
        lang->init(9);
        lang->beLike(*other.lang);
    }

    // operatoru =
    Language& operator=(const Language& other) {
        lang->beLike(*other.lang);
        return *this;
    }

    // ! ################## important ###########################
    // verifica daca un text este corect in limba aceasta
    // ! ################## important ###########################
    bool check(string text) {
        // cout << "DEBUG: score=" << lang->calculus({
        //     (float)(vowelsPerLetters(text)),
        //     (float)(consonantsPerLetters(text)),
        //     (float)(countDashes(text)),
        //     (float)(countWords(text)/3),
        //     (float)(mostUsedVowel(text)/2),
        //     (float)(mostUsedConsonant(text)/2),
        //     (float)(averageLettersPerWord(text)/4),
        //     (float)(averageLettersPerPhrase(text)/15),
        //     (float)(mostUsedLetter(text)/10) // impartit la 10 ca sa fie mai mica sensibilitatea la activarea cu sinus
        // }) << "\n";

        // cout << "vowelsPerLetters: " << (float)(vowelsPerLetters(text)) << "\n";
        // cout << "consonantsPerLetters: " << (float)(consonantsPerLetters(text)) << "\n";
        // cout << "countDashes: " << (float)(countDashes(text)) << "\n";
        // cout << "countWords: " << (float)(countWords(text)/3) << "\n";
        // cout << "mostUsedVowel: " << (float)(mostUsedVowel(text)/2) << "\n";
        // cout << "mostUsedConsonant: " << (float)(mostUsedConsonant(text)/2) << "\n";
        // cout << "averageLettersPerWord: " << (float)(averageLettersPerWord(text)/4) << "\n";
        // cout << "averageLettersPerPhrase: " << (float)(averageLettersPerPhrase(text)/15) << "\n";
        // cout << "mostUsedLetter: " << (float)(mostUsedLetter(text)/10) << "\n";

        // impartirile/inmultirile sunt pentru a scadea din sensibilitatea
        // activarii cu sinus
        return lang->calculus({
            (float)(vowelsPerLetters(text)*3),
            (float)(consonantsPerLetters(text)*3),
            (float)(countDashes(text)*2),
            (float)(countWords(text)/6),
            (float)(mostUsedVowel(text)/3),
            (float)(mostUsedConsonant(text)/3),
            (float)(averageLettersPerWord(text)/10),
            (float)(averageLettersPerPhrase(text)/15),
            (float)(mostUsedLetter(text)/4)
        }) > 0.999; // mergea si == 1 dar era prea slow
    }

    // verifica daca un text e complet diferit de limba aceasta
    bool antiCheck(string text) {
        return lang->calculus({
            (float)(vowelsPerLetters(text)*3),
            (float)(consonantsPerLetters(text)*3),
            (float)(countDashes(text)*2),
            (float)(countWords(text)/6),
            (float)(mostUsedVowel(text)/3),
            (float)(mostUsedConsonant(text)/3),
            (float)(averageLettersPerWord(text)/10),
            (float)(averageLettersPerPhrase(text)/15),
            (float)(mostUsedLetter(text)/4)
        }) < - 0.999; 
    }

    // operatorul % cu un string returneaza scoru de la calculus
    float operator%(string text) {
        return lang->calculus({
            (float)(vowelsPerLetters(text)*3),
            (float)(consonantsPerLetters(text)*3),
            (float)(countDashes(text)*2),
            (float)(countWords(text)/6),
            (float)(mostUsedVowel(text)/3),
            (float)(mostUsedConsonant(text)/3),
            (float)(averageLettersPerWord(text)/10),
            (float)(averageLettersPerPhrase(text)/15),
            (float)(mostUsedLetter(text)/4)
        });
    }

    // echivalent, lang[phrase] = true sau false (o verifica)
    // * nu cred ca se va folosi
    bool operator[](string text) {
        return check(text);
    }

    // destructor
    ~Language() {
        delete lang;
    }
};

/* clasa pt generatoru de text e singleton pt ca ar fi useless sa fie mai multe */
/* oarecum asta va previne sa fie instantiat de mai multe ori inutil */

class TextGen {
private:
    TextGen() {
        // privat ca sa nu mearga instantiat
    }

    TextGen(const TextGen&) = delete;
    TextGen& operator=(const TextGen&) = delete;
public:
    static TextGen& getInstance() {
        static TextGen instance;
        srand(time(NULL));
        return instance;
    }

    // genereaza un nume de planeta
    string generatePlanetName() {
        vector<string> consoane = splitString("b,c,d,f,g,h,j,k,l,m,n,p,q,r,s,t,v,w,x,z", ",");
        vector<string> vocale = splitString("a,e,i,o,u", ",");

        unsigned int nrSilabe = rand() % 6 + 2;
        string name = "";

        for (unsigned int i = 0; i < nrSilabe; i++) {
            name += consoane[rand() % consoane.size()];
            name += vocale[rand() % vocale.size()];
        }

        // 50% sanse sa se puna o consoana la sfarsit
        if (rand() % 2 == 0) {
            name += consoane[rand() % consoane.size()];
        }

        // 50% sanse sa adauge o consoana pe undeva
        if (rand() % 2 == 0) {
            unsigned int pos = rand() % name.size();
            name.insert(pos, consoane[rand() % consoane.size()]);
        }

        // 75% sanse sa mai puna o vocala undeva
        if (rand() % 4 != 0) {
            unsigned int pos = rand() % name.size();
            name.insert(pos, vocale[rand() % vocale.size()]);
        }

        // 25% sanse sa adauge vocala la inceput
        if (rand() % 4 == 0) {
            name.insert(0, vocale[rand() % vocale.size()]);
        }

        // daca numele planetei e mai lung de 8 caractere
        // 50% sanse sa se adauge o cratima/spatiu oriunde
        // prin nume prin mijloc (nu la inceput sau sfarsit)
        if (name.size() > 8 && rand() % 10 > 3) {
            unsigned int pos = rand() % (name.size() - 2) + 1;
            name.insert(pos, rand() % 2 == 0 ? "-" : " ");
        }

        // daca nr de caractere > 10, 50% sanse sa mai puna o
        // cratima sau nu spatiu pt ca simt ca-i mai realist
        if (name.size() > 10 && rand() % 2 == 0) {
            unsigned int pos = rand() % (name.size() - 2) + 1;
            name.insert(pos, rand() % 2 == 0 ? "-" : " ");
        }

        return validateName(string(doubleCapitalize(name)));
    }

    // pt gemerarea mumelor de nave
    string generateSpaceshipName() {

        vector<string> consoane = splitString("b,c,d,f,g,h,j,k,l,m,n,p,q,r,s,t,v,w,x,z", ",");
        vector<string> vocale = splitString("a,e,i,o,u", ",");
        vector<string> numere = splitString("0,1,2,3,4,5,6,7,8,9", ",");

        unsigned int nrSilabe = rand() % 2 + 2;

        string name = "";

        for (unsigned int i = 0; i < nrSilabe; i++) {
            name += consoane[rand() % consoane.size()];
            name += vocale[rand() % vocale.size()];
        }

        // 50% sanse sa adauge o consoana pe undeva
        if (rand() % 2 == 0) {
            unsigned int pos = rand() % name.size();
            name.insert(pos, consoane[rand() % consoane.size()]);
        }

        // 75% sanse sa mai puna o vocala undeva

        if (rand() % 4 != 0) {
            unsigned int pos = rand() % name.size();
            name.insert(pos, vocale[rand() % vocale.size()]);
        }

        // inca 50% sanse sa adauge o consoana la inceput
        if (rand() % 2 == 0) {
            name.insert(0, consoane[rand() % consoane.size()]);
        }

        // 60% sanse sa se puna un numar la sfarsit
        if (rand() % 10 < 6) {
            int num = rand() % 3 + 1;

            name += "-";

            for (int i = 0; i < num; i++) {
                name += numere[rand() % numere.size()];
            }
        }

        return string(capitalize(name));
    }

    // pt generarea de fraze (random intre 1 si 3 fraze sau cu parametru)
    string generatePhrase(int phraseCountPararmeter = -1) {
        // implementation for generating phrases
        vector<string> consoane = splitString("b,c,d,f,g,h,j,k,l,m,n,p,q,r,s,t,v,w,x,z", ",");
        vector<string> vocale = splitString("a,e,i,o,u", ",");
        vector<string> punctuations = splitString(".,!,?,.,!,?,.,!,?,.,!?,??", ",");
        vector<string> commonLetters = splitString("e,t,a,o,i,n,s,r", ",");
        vector<string> rareLetters = splitString("q,z,y,k,j,x", ",");

        unsigned int wordCount = rand() % 12 + 3;
        unsigned int phraseCount = phraseCountPararmeter < 0 ? rand() % 3 + 1 : phraseCountPararmeter;

        string phrases = "";

        // generare fraze
        for (unsigned int i = 0; i < phraseCount; i++) {
            string phrase = "";

            // generare cuvinte
            for (unsigned int j = 0; j < wordCount; j++) {
                unsigned int wordSize = rand() % 8 + 2;
                string word = "";

                // generare litere
                for (unsigned int k = 0; k < wordSize; k++) {
                    if (k % 2 == 0) {
                        word += consoane[rand() % consoane.size()];
                    } else {
                        word += vocale[rand() % vocale.size()];
                    }
                }

                // 40% sanse sa scoata o litera la intamplare din cuvant
                if (rand() % 10 < 4) {
                    word.erase(rand() % word.size(), 1);
                }

                // 50% sanse sa adauge o consoana la sfarsit
                if (rand() % 2 == 0) {
                    word += consoane[rand() % consoane.size()];
                }

                // 60% sanse sa adauge o vocala oriunde
                if (rand() % 10 < 6) {
                    word.insert(rand() % word.size(), vocale[rand() % vocale.size()]);
                }

                // 10% sanse sa adauge o cratima oriunde (da nu la inceput sau sfarsit)
                if (rand() % 10 == 0 && word.size() > 2) {
                    word.insert(rand() % (word.size() - 2) + 1, "-");
                }

                // 50% sanse stearga o litera rara la intamplare
                // si sa fie inlocuita cu una comuna
                if (rand() % 2 == 0) {
                    string letterToReplace = rareLetters[rand() % rareLetters.size()];
                    string letterToInsert = commonLetters[rand() % commonLetters.size()];

                    for (unsigned int k = 0; k < word.size(); k++) {
                        if (word[k] == letterToReplace[0]) {
                            word[k] = letterToInsert[0];
                            break;
                        }
                    }
                }

                // daca exista 3 consoane una langa alta
                // se sterge cea din mijloc
                // cout << "Pt: " << word << "\n"; /*DEBUG*/
                if (word.size() > 2) {
                    for (unsigned int k = 0; k < word.size() - 2; k++) {
                        if (consoane[0].find(word[k]) != string::npos &&
                            consoane[0].find(word[k + 1]) != string::npos &&
                            consoane[0].find(word[k + 2]) != string::npos) {
                            word.erase(k + 1, 1);
                        }
                    }
                }

                phrase += word + " ";
            }

            phrase += punctuations[rand() % punctuations.size()] + " ";

            phrases += phrase;
        }

        // se sterg spatiile de la sfarsit
        phrases.erase(phrases.size() - 1, 1);

        // si spatiile de dinainte de punctuatii
        for (unsigned int i = 0; i < phrases.size(); i++) {
            if (phrases[i] == ' ' && (phrases[i + 1] == '.' || phrases[i + 1] == '!' || phrases[i + 1] == '?')) {
                phrases.erase(i, 1);
            }
        }

        // si se capitalizeaza prima litera dupa punctuatie (+ spatiu)
        for (unsigned int i = 0; i < phrases.size(); i++) {
            if (phrases[i] == '.' || phrases[i] == '!' || phrases[i] == '?') {
                phrases[i + 2] = toupper(phrases[i + 2]);
            }
        }

        return capitalize(phrases);
    }

    // genereaza o fraza care sa fie corecta in limba data
    string generatePhrase(int phraseCount, Language * lang) {
        string phrase = generatePhrase(phraseCount);

        while (lang->check(phrase) == false) {
            phrase = generatePhrase(phraseCount);
        }

        return phrase;
    }

    // genereaza o fraza care sa fie complet diferita de limba data
    string generateAntiPhrase(int phraseCount, Language * lang) {
        string phrase = generatePhrase(phraseCount);

        while (lang->antiCheck(phrase) == false) {
            phrase = generatePhrase(phraseCount);
        }

        return phrase;
    }

    // operatoru + cu un unsigned int genereaza atatea fraze
    string operator+(unsigned int phraseCount) {
        return generatePhrase(phraseCount);
    }

    // operatorul + cu o limba genereaza 1 fraza in limba aia
    string operator+(Language * lang) {
        return generatePhrase(1, lang);
    }

    // operatorul - cu o limba genereaza 1 anti fraza in limba aia
    string operator-(Language * lang) {
        return generateAntiPhrase(1, lang);
    }

    // operatorul + cu o {int, Language} genereaza atatea fraze in limba aia
    string operator+(pair<unsigned int, Language *> lang) {
        return generatePhrase(lang.first, lang.second);
    }

    // operatorul - cu o {int, Language} genereaza atatea anti fraze in limba aia
    string operator-(pair<unsigned int, Language *> lang) {
        return generateAntiPhrase(lang.first, lang.second);
    }

    // destructor
    ~TextGen() {
        // nimic aici
        // s-ar putea sa fac instance-ul null totusi
        // ca sa nu pointeze la ceva random
    }
};