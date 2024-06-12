#include <vector>
#include <cmath> // pt abs

using namespace std;

// hue e grad si saturation si lightness sunt procente
template<typename T> // template in caz ca folosesc short sau unsigned sau int normal etc...
vector<T> hslToRGB(T hue, T saturation, T lightness) {
    // verificam mai intai daca e intervalul bun
    float sat = max(0.0f, min(static_cast<float>(saturation), 100.0f));
    float lit = max(0.0f, min(static_cast<float>(lightness), 100.0f));

    float C = (1 - fabs(2 * lit / 100 - 1)) * sat / 100;
    float X = C * (1 - fabs(fmod(hue / 60.0, 2) - 1));
    float m = lit / 100 - C / 2;

    float r = 0, g = 0, b = 0;

    // sectoarele
    if (0 <= hue && hue < 60) {
        r = C; g = X; b = 0;
    } else if (60 <= hue && hue < 120) {
        r = X; g = C; b = 0;
    } else if (120 <= hue && hue < 180) {
        r = 0; g = C; b = X;
    } else if (180 <= hue && hue < 240) {
        r = 0; g = X; b = C;
    } else if (240 <= hue && hue < 300) {
        r = X; g = 0; b = C;
    } else if (300 <= hue && hue < 360) {
        r = C; g = 0; b = X;
    }

    // conversia in intervalul rgb
    T red = static_cast<T>((r + m) * 255);
    T green = static_cast<T>((g + m) * 255);
    T blue = static_cast<T>((b + m) * 255);

    return {red, green, blue};
}
