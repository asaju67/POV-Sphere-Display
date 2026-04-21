#ifndef WEB_UI_H
#define WEB_UI_H

#include <Arduino.h>

extern const char INDEX_HTML[] PROGMEM;

void setupWebServer();
void handleRoot();
void handleSubmitPatterns();
void handleStatus();
void handleNotFound();

#endif
