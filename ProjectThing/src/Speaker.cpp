#include "Speaker.h"
#include "I2SOutput.h"
#include "WAVFileReader.h"

// Find the first digit 
int firstDigit(int n) 
{ 
    // Find total number of digits - 1 
    int digits = (int)log10(n); 

    // Find first digit 
    n = (int)(n / pow(10, digits)); 

    // Return first digit 
    return n; 
}

Speaker::Speaker(I2SOutput *i2s_output)
{
    m_i2s_output = i2s_output;
    m_ok = new WAVFileReader("/ok.wav");
    m_ready_ping = new WAVFileReader("/ready_ping.wav");
    m_cantdo = new WAVFileReader("/cantdo.wav");
    m_life = new WAVFileReader("/life.wav");
    m_joke = new WAVFileReader("/joke0.wav");
    m_clear = new WAVFileReader("/clear.wav");
    m_cloudy = new WAVFileReader("/cloudy.wav");
    m_drizzling = new WAVFileReader("/drizzling.wav");
    m_foggy = new WAVFileReader("/foggy.wav");
    m_rainy = new WAVFileReader("/rainy.wav");
    m_snowy = new WAVFileReader("/snowy.wav");
    m_stormy = new WAVFileReader("/stormy.wav");
    m_sunny = new WAVFileReader("/sunny.wav");
}

Speaker::~Speaker()
{
    delete m_ok;
    delete m_ready_ping;
    delete m_cantdo;
    delete m_life;
    delete m_joke;
    delete m_clear;
    delete m_cloudy;
    delete m_drizzling;
    delete m_foggy;
    delete m_rainy;
    delete m_snowy;
    delete m_stormy;
    delete m_sunny;
}

void Speaker::playOK()
{
    m_ok->reset();
    m_i2s_output->setSampleGenerator(m_ok);
}

void Speaker::playReady()
{
    m_ready_ping->reset();
    m_i2s_output->setSampleGenerator(m_ready_ping);
}

void Speaker::playCantDo()
{
    m_cantdo->reset();
    m_i2s_output->setSampleGenerator(m_cantdo);
}

void Speaker::playJoke()
{
    m_i2s_output->setSampleGenerator(m_joke);
}

void Speaker::playLife()
{
    m_life->reset();
    m_i2s_output->setSampleGenerator(m_life);
}

void Speaker::playWeather(int weatherCode)
{
    // Logic to determine which weather response file to play
    switch (firstDigit(weatherCode))
    {
    // The type of weather can be determined by the first digit of response code
    // More information: https://openweathermap.org/weather-conditions
    case 2:
        Serial.println("Stormy");
        m_stormy->reset();
        m_i2s_output->setSampleGenerator(m_stormy);
        break;
    case 3:
        Serial.println("Drizzling");
        m_drizzling->reset();
        m_i2s_output->setSampleGenerator(m_drizzling);
        break;
    case 5:
        Serial.println("Rainy");
        m_rainy->reset();
        m_i2s_output->setSampleGenerator(m_rainy);
        break;
    case 6:
        Serial.println("Snowy");
        m_snowy->reset();
        m_i2s_output->setSampleGenerator(m_snowy);
        break;
    case 7:
        // We are generalising all these responses to "foggy" due to ESP32 file storage constriants
        Serial.println("Foggy");
        m_foggy->reset();
        m_i2s_output->setSampleGenerator(m_foggy);
        break;
    case 8:
        // In the case of codes beginning with 8, 800 is clear and anything else is cloudy
        if (weatherCode == 800) {
            Serial.println("Clear");
            m_clear->reset();
            m_i2s_output->setSampleGenerator(m_clear);
        } else {
            Serial.println("Cloudy");
            m_cloudy->reset();
            m_i2s_output->setSampleGenerator(m_cloudy);
        }

        break;
    default:
        break;
    }
}