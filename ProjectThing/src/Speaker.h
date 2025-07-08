#ifndef _speaker_h_
#define _speaker_h_

class I2SOutput;
class WAVFileReader;

class Speaker
{
private:
    WAVFileReader *m_ok;
    WAVFileReader *m_cantdo;
    WAVFileReader *m_ready_ping;
    WAVFileReader *m_life;
    WAVFileReader *m_joke;
    WAVFileReader *m_clear;
    WAVFileReader *m_cloudy;
    WAVFileReader *m_drizzling;
    WAVFileReader *m_foggy;
    WAVFileReader *m_rainy;
    WAVFileReader *m_snowy;
    WAVFileReader *m_stormy;
    WAVFileReader *m_sunny;

    I2SOutput *m_i2s_output;

public:
    Speaker(I2SOutput *i2s_output);
    ~Speaker();
    void playOK();
    void playReady();
    void playCantDo();
    void playJoke();
    void playLife();
    void playWeather(int weatherCode);
};

#endif