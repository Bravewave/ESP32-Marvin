#include "Weather.h"

/* Helper function to convert floats to strings */
String Weather::fToStr(float f) {
    std::string str = std::to_string(f);
    return String(str.c_str());
}

/* Parse response from Geocoding API and store coordinates in external variables */
void Weather::parseCoordinates(const String &str, float &latitude, float &longitude) {
  const char *data = str.c_str();

  // Search response for first instance of latitude data
  const char *latPos = strstr(data, "\"lat\":");
  if (latPos == NULL) {
    Serial.println("Latitude not found.");
    return;
  }
  // Move pointer to the end of the substring
  latPos += 6;

  // Parse latitude value
  latitude = atof(latPos);

  // Repeat for longitude
  const char *lonPos = strstr(data, "\"lon\":");
  if (lonPos == NULL) {
    Serial.println("Longitude not found.");
    return;
  }

  lonPos += 6;

  longitude = atof(lonPos);
}

/* Given a response from the weather API, return the 3-digit weather code */
void Weather::getWeatherCode(const String &str, int &weatherCode) {
  const char *data = str.c_str();

  // Very similar functionality to parseCoordinates()

  const char *idPos = strstr(data, "\"id\":");
  if (idPos == NULL) {
      Serial.println("ID not found.");
      return;
  }
  
  idPos += 5;

  // Use a buffer this time to store the ID & null terminator
  char buffer[4];
  int i = 0;
  while (isdigit(*idPos) && i < 3) {
      buffer[i++] = *idPos++;
  }
  buffer[i] = '\0';

  weatherCode = atoi(buffer);
}

/* Make request to weather API, given a location name */
int Weather::makeApiRequest(const String& location) {
  float latitude, longitude;
  int weatherCode;

  HTTPClient http;
  String geoURL = "http://api.openweathermap.org/geo/1.0/direct?q=" + location + "&limit=5&appid=9dc9cc093e1273022db4eef4dc49d254";

  // Do GeoCoding request

  http.begin(geoURL.c_str());

  int geoResCode = http.GET();
  if (geoResCode > 0) {
    String geoPayload = http.getString();
    parseCoordinates(geoPayload, latitude, longitude);
  } else {
    Serial.print("Error on GEO request: ");
    Serial.println(geoResCode);
    return -1; // Return -1 on fail
  }
  http.end();

  // Do weather data request

  String weatherURL = "https://api.openweathermap.org/data/2.5/weather?lat=" + fToStr(latitude) + "&lon=" + fToStr(longitude) + "&appid=9dc9cc093e1273022db4eef4dc49d254";

  http.begin(weatherURL.c_str());

  int weatherResCode = http.GET();
  if (weatherResCode > 0) {
    String weatherPayload = http.getString();
    getWeatherCode(weatherPayload, weatherCode);
  } else {
    Serial.print("Error on weather request: ");
    Serial.println(weatherResCode);
    return -1;
  }
  http.end();

  return weatherCode;
}