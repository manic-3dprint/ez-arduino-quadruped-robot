/* 
 * File:   Oscillator.hpp
 * Author: user
 *
 * Created on June 16, 2016, 3:35 PM
 */

#ifndef OSCILLATOR_HPP
#define OSCILLATOR_HPP
#include <Arduino.h>
#include <Servo.h>

class Oscillator {
public:

    Oscillator();
    void attach(int pin, bool rev = false);
    void detach();

    void SetA(int A) {
        _A = A;
    };

    void SetO(int O) {
        _O = O;
    };

    void SetPh(double Ph) {
        _phase0 = Ph;
    };
    void SetT(int T);

    void SetTrim(int trim) {
        _trim = trim;
    };

    int getTrim() {
        return _trim;
    };
    void SetPosition(int position);

    void Stop() {
        _stop = true;
    };

    void Play() {
        _stop = false;
    };

    void Reset() {
        _phase = 0;
    };
    int refresh();
    virtual ~Oscillator();

private:
    bool next_sample();


    //-- Oscillators parameters
    int _A; //-- Amplitude (degrees)
    int _O; //-- Offset (degrees)
    int _T; //-- Period (miliseconds)
    double _phase0; //-- Phase (radians)

    //-- Internal variables
    int _pos; //-- Current servo pos
    int _trim; //-- Calibration offset
    double _phase; //-- Current phase
    double _inc; //-- Increment of phase
    double _N; //-- Number of samples
    unsigned int _TS; //-- sampling period (ms)

    unsigned long _previousMillis;
    unsigned long _currentMillis;

    //-- Oscillation mode. If true, the servo is stopped
    bool _stop;

    //-- Reverse mode
    bool _rev;
    Servo _servo;
};

#endif /* OSCILLATOR_HPP */

