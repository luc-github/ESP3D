#ifndef __POLLEDTIMING_H__
#define __POLLEDTIMING_H__

#include <Arduino.h>

class PolledTimeout
{
public:
    PolledTimeout(uint32_t timeout)
    {
        _timeOut = timeout;
        _startMS = millis();
    }
    operator bool() const
    {
        return ((millis() - _startMS) > _timeOut);
    }
    void reset()
    {
        _startMS = millis();
    }
private:
    uint32_t _startMS;
    uint32_t _timeOut;
};

#endif
