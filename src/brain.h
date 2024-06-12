#include <vector>
#include <ctime>
#include <cstdlib>
#include <cmath>
#include <iostream>
#include <fstream>

using namespace std;

class Brain {
protected:
    vector<float> pMultipliers;
    vector<float> pBiases;
public:
    // tanh mereu a fost mai buna decat sigmoid
    virtual float activation(float x) {
        return tanh(x);
    }
    
    // calculul principal
    float calculus(vector<float> inputs) {
        float sum = 0;
        for (int i = 0; i < inputs.size(); i++) {
            sum += activation(inputs[i] * getMultipliers()[i] + getBiases()[i]);
        }
        return sum;
    }

    // initializarea parametrilor random
    virtual void init(unsigned int count) {
        srand(time(NULL));
        
        pMultipliers.resize(count);
        pBiases.resize(count);

        // parametrii random
        for (int i = 0; i < count; i++) {
            pMultipliers[i] = sin(rand());
            pBiases[i] = cos(rand());
        }
    }

    virtual void mutate(float mutationRate) {
        for (int i = 0; i < getMultipliers().size(); i++) {
            pMultipliers[i] += sin(rand()) * mutationRate;
            pBiases[i] += cos(rand()) * mutationRate;
        }

        // limita
        for (int i = 0; i < getMultipliers().size(); i++) {
            if (pMultipliers[i] > 1) {
                pMultipliers[i] = 1;
            }
            if (pMultipliers[i] < -1) {
                pMultipliers[i] = -1;
            }
            if (pBiases[i] > 1) {
                pBiases[i] = 1;
            }
            if (pBiases[i] < -1) {
                pBiases[i] = -1;
            }
        }
    }

    vector<float> getMultipliers() {
        return pMultipliers;
    }

    vector<float> getBiases() {
        return pBiases;
    }

    virtual ~Brain() { }
};

class TwoOutputsBrain : public Brain {
private:
    vector<float> secondMultipliers;
    vector<float> secondBiases;
public:
    // constructor fara parametrii
    TwoOutputsBrain() {}

    // constructor cu parametrii
    TwoOutputsBrain(vector<float> multipliers, vector<float> biases, vector<float> secondMultipliers, vector<float> secondBiases) {
        this->pMultipliers = multipliers;
        this->pBiases = biases;

        this->secondMultipliers = secondMultipliers;
        this->secondBiases = secondBiases;
    }

    // constructor de copiere
    TwoOutputsBrain(const TwoOutputsBrain& other) {
        pMultipliers = other.pMultipliers;
        pBiases = other.pBiases;

        secondMultipliers = other.secondMultipliers;
        secondBiases = other.secondBiases;
    }

    // initializarea parametrilor random
    void init(unsigned int count) override {
        Brain::init(count); // ca srand sa fie apelat
        
        secondMultipliers.resize(count);
        secondBiases.resize(count);

        // add random
        for (int i = 0; i < count; i++) {
            secondMultipliers[i] = sin(rand());
            secondBiases[i] = cos(rand());
        }
    }

    // schimbarea parametrilor random
    void mutate(float mutationRate) override {
        Brain::mutate(mutationRate);
        
        for (int i = 0; i < getMultipliers().size(); i++) {
            secondMultipliers[i] += sin(rand()) * mutationRate;
            secondBiases[i] += cos(rand()) * mutationRate;
        }

        // limitarea mutatiei
        for (int i = 0; i < getMultipliers().size(); i++) {
            if (secondMultipliers[i] > 1) {
                secondMultipliers[i] = 1;
            }
            if (secondMultipliers[i] < -1) {
                secondMultipliers[i] = -1;
            }
            if (secondBiases[i] > 1) {
                secondBiases[i] = 1;
            }
            if (secondBiases[i] < -1) {
                secondBiases[i] = -1;
            }
        }
    }

    // acesta nu e un override pentru ca are alt return type
    vector<float> calculus(vector<float> inputs) {
        float sumOne = 0;
        for (int i = 0; i < inputs.size(); i++) {
            sumOne += activation(inputs[i] * getMultipliers()[i] + getBiases()[i]);
        }
        
        float sumTwo = 0;
        for (int i = 0; i < inputs.size(); i++) {
            sumTwo += activation(inputs[i] * secondMultipliers[i] + secondBiases[i]);
        }

        return {sumOne, sumTwo}; 
    }

    virtual ~TwoOutputsBrain() { }
};

// x inputs cu un hidden layer de x neuroni si un output layer de 1 neuron
class Perceptron : public Brain {
private:
    vector<float> inputsMultipliers;
    vector<float> inputsBiases;

    vector<float> hiddenMultipliers;
    vector<float> hiddenBiases;

    vector<float> outputMultipliers;
    vector<float> outputBiases;

    unsigned int count;
public:
    // contructoru fara parametrii (esential)
    Perceptron() {}

    // constructor cu parametrii
    Perceptron(vector<float> inputsMultipliers, vector<float> inputsBiases, vector<float> hiddenMultipliers, vector<float> hiddenBiases, vector<float> outputMultipliers, vector<float> outputBiases) {
        this->inputsMultipliers = inputsMultipliers;
        this->inputsBiases = inputsBiases;

        this->hiddenMultipliers = hiddenMultipliers;
        this->hiddenBiases = hiddenBiases;

        this->outputMultipliers = outputMultipliers;
        this->outputBiases = outputBiases;
    }

    // constructor de copiere
    Perceptron(const Perceptron& other) {
        inputsMultipliers = other.inputsMultipliers;
        inputsBiases = other.inputsBiases;

        hiddenMultipliers = other.hiddenMultipliers;
        hiddenBiases = other.hiddenBiases;

        outputMultipliers = other.outputMultipliers;
        outputBiases = other.outputBiases;
    }

    // functia de activare
    float activation(float x) override {
        return sin(x * 3.1415); // am observat ca asta e mai buna decat tanh
    }

    // operatoru de afisare (pentru debug din pacate)
    friend ostream& operator<<(ostream& os, const Perceptron& p) {
        os << "Perceptron: \n";
        os << "Inputs multipliers: ";
        for (int i = 0; i < p.getInputsMultipliers().size(); i++) {
            os << p.getInputsMultipliers()[i] << " ";
        }
        os << "\n";

        os << "Inputs biases: ";
        for (int i = 0; i < p.getInputsBiases().size(); i++) {
            os << p.getInputsBiases()[i] << " ";
        }
        os << "\n";

        os << "Hidden multipliers: ";
        for (int i = 0; i < p.getHiddenMultipliers().size(); i++) {
            os << p.getHiddenMultipliers()[i] << " ";
        }
        os << "\n";

        os << "Hidden biases: ";
        for (int i = 0; i < p.getHiddenBiases().size(); i++) {
            os << p.getHiddenBiases()[i] << " ";
        }
        os << "\n";
    
        os << "Output multipliers: ";
        for (int i = 0; i < p.getOutputMultipliers().size(); i++) {
            os << p.getOutputMultipliers()[i] << " ";
        }
        os << "\n";

        os << "Output biases: ";
        for (int i = 0; i < p.getOutputBiases().size(); i++) {
            os << p.getOutputBiases()[i] << " ";
        }
        os << "\n";

        return os;
    }

    // initializarea parametrilor random
    void init(unsigned int count) override {
        Brain::init(count);
        this->count = count;

        inputsMultipliers.resize(count);
        inputsBiases.resize(count);

        hiddenMultipliers.resize(count);
        hiddenBiases.resize(count);

        outputMultipliers.resize(count);
        outputBiases.resize(count);

        // add random
        for (int i = 0; i < count; i++) {
            inputsMultipliers[i] = sin(rand());
            inputsBiases[i] = cos(rand());

            hiddenMultipliers[i] = sin(rand());
            hiddenBiases[i] = cos(rand());

            outputMultipliers[i] = sin(rand());
            outputBiases[i] = cos(rand());
        }
    }

    // toti parametrii devin 0
    void zero() {
        for (int i = 0; i < count; i++) {
            inputsMultipliers[i] = 0;
            inputsBiases[i] = 0;

            hiddenMultipliers[i] = 0;
            hiddenBiases[i] = 0;

            outputMultipliers[i] = 0;
            outputBiases[i] = 0;
        }
    }

    // schimbarea parametrilor random
    void mutate(float mutationRate) override {
        for (int i = 0; i < inputsMultipliers.size(); i++) {
            inputsMultipliers[i] += sin(rand()) * mutationRate;
            inputsBiases[i] += cos(rand()) * mutationRate;

            hiddenMultipliers[i] += sin(rand()) * mutationRate;
            hiddenBiases[i] += cos(rand()) * mutationRate;

            outputMultipliers[i] += sin(rand()) * mutationRate;
            outputBiases[i] += cos(rand()) * mutationRate;
        }

        // limitarea mutatiei
        for (int i = 0; i < inputsMultipliers.size(); i++) {
            if (inputsMultipliers[i] > 1) inputsMultipliers[i] = 1;
            if (inputsMultipliers[i] < -1) inputsMultipliers[i] = -1;
            if (inputsBiases[i] > 1) inputsBiases[i] = 1;
            if (inputsBiases[i] < -1) inputsBiases[i] = -1;
            if (hiddenMultipliers[i] > 1) hiddenMultipliers[i] = 1;
            if (hiddenMultipliers[i] < -1) hiddenMultipliers[i] = -1;
            if (hiddenBiases[i] > 1) hiddenBiases[i] = 1;
            if (hiddenBiases[i] < -1) hiddenBiases[i] = -1;
            if (outputMultipliers[i] > 1) outputMultipliers[i] = 1;
            if (outputMultipliers[i] < -1) outputMultipliers[i] = -1;
            if (outputBiases[i] > 1) outputBiases[i] = 1;
            if (outputBiases[i] < -1) outputBiases[i] = -1;
        }
    }

    // pentru calculus
    float calculus(vector<float> inputs) {
        vector<float> inputLayersResult = {};
        /*cout << "input gol: " << inputLayersResult.size() << "\n";*/
        for (int i = 0; i < inputs.size(); i++) {
            inputLayersResult.push_back(inputs[i] * inputsMultipliers[i] + inputsBiases[i]);
        }

        vector<float> hiddenLayersResult = {};
        /*cout << "hidden gol: " << hiddenLayersResult.size() << "\n";*/
        for (int i = 0; i < inputLayersResult.size(); i++) {
            hiddenLayersResult.push_back(activation(inputLayersResult[i] * hiddenMultipliers[i] + hiddenBiases[i]));
        }

        float outputResult = 0;

        /*cout << "output gol: " << outputResult << "\n";*/
        for (int i = 0; i < hiddenLayersResult.size(); i++) {
            outputResult += hiddenLayersResult[i] * outputMultipliers[i] + outputBiases[i];
        }

        /*cout << "output final: " << activation(outputResult) << "\n";*/
        return activation(outputResult);
    }

    // copiaza parametrii unui perceptron
    void beLike(Perceptron other) {
        inputsMultipliers = other.getInputsMultipliers();
        inputsBiases = other.getInputsBiases();
        
        hiddenMultipliers = other.getHiddenMultipliers();
        hiddenBiases = other.getHiddenBiases();

        outputMultipliers = other.getOutputMultipliers();
        outputBiases = other.getOutputBiases();
    }

    // functia de invatare (cea mai importanta)
    float learn(const vector<vector<float>>& inputArray, const vector<float>& expectedOutputs, float mutationRate = 0.1) {
        Perceptron *newPerceptron = new Perceptron(*this);

        // eroarea normala
        float error = 0;
        for (int i = 0; i < inputArray.size(); i++) {
            vector<float> inputs = inputArray[i];
            float expectedOutput = expectedOutputs[i];

            error += abs(expectedOutput - newPerceptron->calculus(inputs));
        }

        // celui nou
        // cout << "Debug: cel vechi: " << inputsMultipliers[0] << "\n";
        newPerceptron->mutate(mutationRate);
        // cout << "Debug: cel nou: " << newPerceptron->getInputsMultipliers()[0] << "\n";

        float newError = 0;
        for (int i = 0; i < inputArray.size(); i++) {
            vector<float> inputs = inputArray[i];
            float expectedOutput = expectedOutputs[i];

            newError += abs(expectedOutput - newPerceptron->calculus(inputs));
        }

        // daca e mai bun, il copiem
        if (newError < error) {
            // cout << "before belike: " << inputsMultipliers[0] << "\n";
            beLike(*newPerceptron);
            // cout << "after belike: " << inputsMultipliers[0] << "\n";
        }

        delete newPerceptron;

        return error < newError ? error : newError;
    }

    // operatorul de == intre doi perceptroni va compara parametrii
    bool operator==(const Perceptron& other) {
        return inputsMultipliers == other.inputsMultipliers &&
            inputsBiases == other.inputsBiases &&
            hiddenMultipliers == other.hiddenMultipliers &&
            hiddenBiases == other.hiddenBiases &&
            outputMultipliers == other.outputMultipliers &&
            outputBiases == other.outputBiases;
    }

    // operatoru de = intre doi perceptroni va copia parametrii
    Perceptron& operator=(const Perceptron& other) {
        if (this == &other) {
            return *this;
        } else {
            this->inputsMultipliers = other.getInputsMultipliers();
            this->inputsBiases = other.getInputsBiases();
            this->hiddenMultipliers = other.getHiddenMultipliers();
            this->hiddenBiases = other.getHiddenBiases();
            this->outputMultipliers = other.getOutputMultipliers();
            this->outputBiases = other.getOutputBiases();

            return *this;
        }
    }

    // getteri
    const vector<float>& getInputsMultipliers() const { return inputsMultipliers; }
    const vector<float>& getInputsBiases() const { return inputsBiases; }
    const vector<float>& getHiddenMultipliers() const { return hiddenMultipliers; }
    const vector<float>& getHiddenBiases() const { return hiddenBiases; }
    const vector<float>& getOutputMultipliers() const { return outputMultipliers; }
    const vector<float>& getOutputBiases() const { return outputBiases; }

    // export
    void exportData () {
        // format in genu inputMultipliers 0 value
        string data = "";
        for (int i = 0; i < count; i++) {
            data += "inputMultipliers " + to_string(i) + " " + to_string(inputsMultipliers[i]) + "\n";
            data += "inputBiases " + to_string(i) + " " + to_string(inputsBiases[i]) + "\n";
            data += "hiddenMultipliers " + to_string(i) + " " + to_string(hiddenMultipliers[i]) + "\n";
            data += "hiddenBiases " + to_string(i) + " " + to_string(hiddenBiases[i]) + "\n";
            data += "outputMultipliers " + to_string(i) + " " + to_string(outputMultipliers[i]) + "\n";
            data += "outputBiases " + to_string(i) + " " + to_string(outputBiases[i]) + "\n";
        }

        // scrie in brain_n.json
        ofstream file("brain_" + to_string(count) + ".neural");
        file << data;
        file.close();
    }

    // import
    void importData() {
        ifstream file("brain_" + to_string(count) + ".neural");
        string line;
        while (getline(file, line)) {
            vector<string> tokens;
            string token;
            // cout << "Importing line: " << line << "\n";
            for (int i = 0; i < line.size(); i++) {
                if (line[i] == ' ') {
                    tokens.push_back(token);
                    // cout << "Token: " << token << "\n";
                    token = "";
                } else {
                    token += line[i];
                }
            }
            tokens.push_back(token);
            // cout << "Token: " << token << "\n";

            if (tokens[0] == "inputMultipliers") {
                inputsMultipliers[stoi(tokens[1])] = stof(tokens[2]);
            } else if (tokens[0] == "inputBiases") {
                inputsBiases[stoi(tokens[1])] = stof(tokens[2]);
            } else if (tokens[0] == "hiddenMultipliers") {
                hiddenMultipliers[stoi(tokens[1])] = stof(tokens[2]);
            } else if (tokens[0] == "hiddenBiases") {
                hiddenBiases[stoi(tokens[1])] = stof(tokens[2]);
            } else if (tokens[0] == "outputMultipliers") {
                outputMultipliers[stoi(tokens[1])] = stof(tokens[2]);
            } else if (tokens[0] == "outputBiases") {
                outputBiases[stoi(tokens[1])] = stof(tokens[2]);
            }
            // cout << "Imported " << tokens[0] << " " << tokens[1] << " " << tokens[2] << "\n";
        }
        file.close();
    }

    // import if exists
    void iie() {
        ifstream file("brain_" + to_string(count) + ".neural");
        if (file.good()) {
            cout << "Importing brain_" + to_string(count) + ".neural\n";
            importData();
        } else {
            cerr << "File \"brain_" + to_string(count) + ".neural\" does not exist.\n";
        }
        file.close();
    }

    ~Perceptron() {}
};

// perceptroni cu alte activari ------------

// sin(x * pi/2)
class slowSinPerceptron : public Perceptron {
public:
    float activation(float x) override {
        return sin(x*3.1415/2);
    }
};

// tanh
class tanhPerceptron : public Perceptron {
public:
    float activation(float x) override {
        return tanh(x);
    }
};

// sigmoid
class sigmoidPerceptron : public Perceptron {
public:
    float activation(float x) override {
        return 1 / (1 + exp(-x));
    }
};

// relu
class reluPerceptron : public Perceptron {
public:
    float activation(float x) override {
        return x > 0 ? x : 0;
    }
};

// leaky relu
class leakyReluPerceptron : public Perceptron {
public:
    float activation(float x) override {
        return x > 0 ? x : 0.01 * x;
    }
};

// elu
class eluPerceptron : public Perceptron {
public:
    float activation(float x) override {
        return x > 0 ? x : 0.01 * (exp(x) - 1);
    }
};

// cos(x * pi)
class slowCosPerceptron : public Perceptron {
public:
    float activation(float x) override {
        return cos(x*3.1415);
    }
};

// chaotic tan(x)
class chaoticTanPerceptron : public Perceptron {
public:
    float activation(float x) override {
        return tan(x);
    }
};

// \tan\left(x\cdot\tan^{-1}\left(1\right)\right)
class tanAtanPerceptron : public Perceptron {
public:
    float activation(float x) override {
        return tan(x * atan(1));
    }
};

// mini saw fourier
// \sum_{n=1}^{20}\frac{\sin\left(-n\left(x\cdot\pi-\pi\right)\right)}{2n}
class sumSinPerceptron : public Perceptron {
public:
    float activation(float x) override {
        float sum = 0;
        for (int n = 1; n <= 20; n++) {
            sum += sin(-n*(x*3.1415-3.1415))/(2*n);
        }
        return sum;
    }
};