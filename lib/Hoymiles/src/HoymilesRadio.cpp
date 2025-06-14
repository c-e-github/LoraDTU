// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2024 C-E and others
 */
#include "HoymilesRadio.h"
#include "Hoymiles.h"
#include "crc.h"

serial_u HoymilesRadio::DtuSerial() const
{
    return _dtuSerial;
}

void HoymilesRadio::setDtuSerial(const uint64_t serial)
{
    _dtuSerial.u64 = serial;
}

serial_u HoymilesRadio::convertSerialToRadioId(const serial_u serial)
{
    serial_u radioId;
    radioId.u64 = 0;
    radioId.b[4] = serial.b[0];
    radioId.b[3] = serial.b[1];
    radioId.b[2] = serial.b[2];
    radioId.b[1] = serial.b[3];
    radioId.b[0] = 0x01;
    return radioId;
}

bool HoymilesRadio::checkFragmentCrc(const fragment_t& fragment) const
{
    const uint8_t crc = crc8(fragment.fragment, fragment.len - 1);
    return (crc == fragment.fragment[fragment.len - 1]);
}

void HoymilesRadio::sendRetransmitPacket(const uint8_t fragment_id)
{
    CommandAbstract* cmd = _commandQueue.front().get();

    CommandAbstract* requestCmd = cmd->getRequestFrameCommand(fragment_id);

    if (requestCmd != nullptr) {
        sendEsbPacket(*requestCmd);
    }
}

void HoymilesRadio::sendLastPacketAgain()
{
    CommandAbstract* cmd = _commandQueue.front().get();
    sendEsbPacket(*cmd);
}

void HoymilesRadio::handleReceivedPackage()
{
    if (_busyFlag && _rxTimeout.occured()) {
        std::shared_ptr<InverterAbstract> inv = Hoymiles.getInverterBySerial(_commandQueue.front().get()->getTargetAddress());

        if (nullptr != inv) {
            CommandAbstract* cmd = _commandQueue.front().get();
            uint8_t verifyResult = inv->verifyAllFragments(*cmd);
            if (verifyResult == FRAGMENT_ALL_MISSING_RESEND) {
                Hoymiles.getMessageOutput()->println("Nothing received, resend whole request");
                sendLastPacketAgain();

            } else if (verifyResult == FRAGMENT_ALL_MISSING_TIMEOUT) {
                Hoymiles.getMessageOutput()->println("Nothing received, resend count exeeded");
                _commandQueue.pop();
                _busyFlag = false;

            } else if (verifyResult == FRAGMENT_RETRANSMIT_TIMEOUT) {
                Hoymiles.getMessageOutput()->println("Retransmit timeout");
                _commandQueue.pop();
                _busyFlag = false;

            } else if (verifyResult == FRAGMENT_HANDLE_ERROR) {
                Hoymiles.getMessageOutput()->println("Packet handling error");
                _commandQueue.pop();
                _busyFlag = false;

            } else if (verifyResult > 0) {
                // Perform Retransmit
                Hoymiles.getMessageOutput()->print("Request retransmit: ");
                sendRetransmitPacket(verifyResult);

            } else {
                // Successful received all packages
                _commandQueue.pop();
                _busyFlag = false;
            }
        } else {
            // If inverter was not found, assume the command is invalid
            Hoymiles.getMessageOutput()->println("RX: Invalid inverter found");
            _commandQueue.pop();
            _busyFlag = false;
        }
    } else if (!_busyFlag) {
        // Currently in idle mode --> send packet if one is in the queue
        if (!isQueueEmpty()) {
            CommandAbstract* cmd = _commandQueue.front().get();

            auto inv = Hoymiles.getInverterBySerial(cmd->getTargetAddress());
            if (nullptr != inv) {
                inv->clearRxFragmentBuffer();
                sendEsbPacket(*cmd);
            } else {
                Hoymiles.getMessageOutput()->println("TX: Invalid inverter found");
                _commandQueue.pop();
            }
        }
    }
}

void HoymilesRadio::dumpBuf(const uint8_t buf[], const uint8_t len, const bool appendNewline)
{
    for (uint8_t i = 0; i < len; i++) {
    }
    if (appendNewline) {
        Hoymiles.getMessageOutput()->println("");
    }
}

bool HoymilesRadio::isInitialized() const
{
    return _isInitialized;
}

bool HoymilesRadio::isIdle() const
{
    return !_busyFlag;
}

bool HoymilesRadio::isQueueEmpty() const
{
    return _commandQueue.size() == 0;
}
