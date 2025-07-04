// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2024 C-E and others
 */
#include "MessageOutput.h"

#include <Arduino.h>

MessageOutputClass MessageOutput;

void MessageOutputClass::init(Scheduler& scheduler)
{
    scheduler.addTask(_loopTask);
    _loopTask.setCallback(std::bind(&MessageOutputClass::loop, this));
    _loopTask.setIterations(TASK_FOREVER);
    //_loopTask.setInterval(200);
    _loopTask.enable();
}

void MessageOutputClass::register_ws_output(AsyncWebSocket* output)
{
    _ws = output;
}

size_t MessageOutputClass::write(uint8_t c)
{
    if (_buff_pos < BUFFER_SIZE) {
        std::lock_guard<std::mutex> lock(_msgLock);
        _buffer[_buff_pos] = c;
        _buff_pos++;
    } else {
        _forceSend = true;
    }

    return Serial.write(c);
}

size_t MessageOutputClass::write(const uint8_t* buffer, size_t size)
{
    std::lock_guard<std::mutex> lock(_msgLock);
    if (_buff_pos + size < BUFFER_SIZE) {
        memcpy(&_buffer[_buff_pos], buffer, size);
        _buff_pos += size;
    }
    _forceSend = true;

    return Serial.write(buffer, size);
}

void MessageOutputClass::loop()
{
    // Send data via websocket if either time is over or buffer is full
    if (_forceSend || (millis() - _lastSend > 500)) {
        std::lock_guard<std::mutex> lock(_msgLock);
        if (_ws && _buff_pos > 0) {
            _ws->textAll(_buffer, _buff_pos);
            _buff_pos = 0;
        }
        if (_forceSend) {
            _buff_pos = 0;
        }
        _forceSend = false;
        _lastSend = millis();

    }
}