// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2024 C-E and others
 */
#include "TimeoutHelper.h"
#include <Arduino.h>

TimeoutHelper::TimeoutHelper()
{
    timeout = 0;
    startMillis = 0;
}

void TimeoutHelper::set(const uint32_t ms)
{
    timeout = ms;
    startMillis = millis();
}

void TimeoutHelper::extend(const uint32_t ms)
{
    timeout += ms;
}

void TimeoutHelper::reset()
{
    startMillis = millis();
}

bool TimeoutHelper::occured() const
{
    return millis() > (startMillis + timeout);
}