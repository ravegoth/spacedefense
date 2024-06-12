#include <typeinfo>
#include <ctime>

using namespace std;

// pentru gestionarea timpului in joc
// e nevoie pentru timer ca sa stim
// cand sa se creasca dificultatea jocului

// se pune stampila in momentul crearii obiectului
template<typename T>
class ClockStamp {
private:
    T stamp;
public:
    // constructor
    ClockStamp() {
        // se seteaza stamp la unixul curent
        time_t tempStamp = time(0);
        stamp = static_cast<T>(tempStamp);
    }

    // constructor cu parametru
    ClockStamp(T stamp) : stamp(stamp) {}

    // constructor de copiere
    ClockStamp(const ClockStamp& other) : stamp(other.stamp) {}

    // operator de atribuire
    ClockStamp& operator=(const ClockStamp& other) {
        if (this != &other) {
            stamp = other.stamp;
        }
        return *this;
    }

    // functia ce returneaza diferenta dintre
    // atunci si momentul apelului
    virtual T getDiff() const {
        time_t tempStamp = time(0);
        return static_cast<T>(tempStamp) - stamp;
    }

    void reset() {
        time_t tempStamp = time(0);
        stamp = static_cast<T>(tempStamp);
    }

    virtual ~ClockStamp() {}
};

// la fel ca ClockStamp doar ca pentru minute
template<typename T>
class MinuteClockStamp : public ClockStamp<T> {
public:
    // constructor
    MinuteClockStamp() : ClockStamp<T>() {}

    // constructor cu parametru
    MinuteClockStamp(T stamp) : ClockStamp<T>(stamp) {}

    // constructor de copiere
    MinuteClockStamp(const MinuteClockStamp& other) : ClockStamp<T>(other) {}

    // operator de atribuire
    MinuteClockStamp& operator=(const MinuteClockStamp& other) {
        if (this != &other) {
            ClockStamp<T>::operator=(other);
        }
        return *this;
    }

    T getDiff() const override {
        return ClockStamp<T>::getDiff() / 60.0;
    }

    ~MinuteClockStamp() {}
};

template<typename T>
unsigned long secondsSince(const ClockStamp<T>& clock) {
    // verifica daca clock este de tipul MinuteClockStamp
    const MinuteClockStamp<T>* minuteClockStamp = dynamic_cast<const MinuteClockStamp<T>*>(&clock);
    if (minuteClockStamp) {
        // daca este, returneaza diferenta in secunde
        return minuteClockStamp->getDiff() * 60;
    }

    // returneaza diferenta in secunde
    return clock.getDiff();
}

template<typename T>
unsigned long minutesSince(const ClockStamp<T>& clock) {
    // verifica daca clock este de tipul MinuteClockStamp
    const MinuteClockStamp<T>* minuteClockStamp = dynamic_cast<const MinuteClockStamp<T>*>(&clock);
    if (minuteClockStamp) {
        // daca este, returneaza diferenta in minute
        return minuteClockStamp->getDiff();
    }

    // returneaza diferenta in minute
    return clock.getDiff() / 60;
}