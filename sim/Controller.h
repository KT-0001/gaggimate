/**
 * @file Controller_stub.h
 * @brief Stub controller for simulator - replaces hardware controller
 */

#ifndef CONTROLLER_STUB_H
#define CONTROLLER_STUB_H

// Stub controller class for simulator
class Controller {
public:
    void setup() {}
    void loop() {}
    
    // Stub methods that UI might call
    void startBrew() {}
    void stopBrew() {}
    void setTemperature(float temp) {}
    float getTemperature() { return 93.0f; }
    float getPressure() { return 9.0f; }
};

// Global instance
extern Controller controller;

#endif // CONTROLLER_STUB_H
