#include "Watchdog.h"

void Watchdog::Init(uint16_t sensorCoreTimeoutMs, uint16_t mainCoreTimeoutMs)
{
    _sensorCoreTimeoutMs = sensorCoreTimeoutMs;
    _mainCoreTimeoutMs = mainCoreTimeoutMs;
    _mainCoreTimer = 0;    
    _kill = false;

}

bool Watchdog::Activate()
{
    bool wasLastResetByWatchdog = watchdog_caused_reboot();
    watchdog_enable(_sensorCoreTimeoutMs, 1);
    
    return wasLastResetByWatchdog;
}

void Watchdog::Poll_SensorCore()
{
    if(_kill == false)
    {
        watchdog_update();
    }

    if(_mainCoreTimer > _mainCoreTimeoutMs)
    {
        OnTimeout();
    }

    _mainCoreTimer += SENSIFY_POLL_RATE;
}

void Watchdog::Poll_MainCore()
{
    _mainCoreTimer = 0;
}

void Watchdog::OnTimeout()
{
    PRINT_HEADING("Core0 Watchdog triggered");
    PRINT_WARN(DM_WATCHDOG_NAME, "Main core timer %d", _mainCoreTimer);
    _kill = true;
    HealthAlert::Submit("Watchdog Event");
    watchdog_update();
    HealthAlert::SaveMessagesToFlash();
    PRINT_SUCCESS(DM_WATCHDOG_NAME, "Health message saved. Letting Watchdog reset");
}