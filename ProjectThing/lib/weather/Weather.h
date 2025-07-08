#ifndef WEATHER_H
#define WEATHER_H

#include <Arduino.h>
#include <HTTPClient.h>
#include <string>

class Weather {
private:
    static String fToStr(float f);

public:
    static void parseCoordinates(const String &str, float &latitude, float &longitude);
    static void getWeatherCode(const String &str, int &weatherCode);
    static int makeApiRequest(const String& location);
};

#endif