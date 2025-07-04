// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2024 C-E and others
 */

/*
This command is used to fetch live run time data from the inverter.

Derives from MultiDataCommand

Command structure:
* DT: this specific command uses 0x0b

00   01 02 03 04   05 06 07 08   09   10   11   12 13 14 15   16 17   18 19   20 21 22 23   24 25   26   27 28 29 30 31
-----------------------------------------------------------------------------------------------------------------------
                                      |<------------------- CRC16 --------------------->|
15   71 60 35 46   80 12 23 04   80   0b   00   65 72 06 B8   00 00   00 00   00 00 00 00   00 00   00   -- -- -- -- --
^^   ^^^^^^^^^^^   ^^^^^^^^^^^   ^^   ^^   ^^   ^^^^^^^^^^^   ^^^^^           ^^^^^^^^^^^   ^^^^^   ^^
ID   Target Addr   Source Addr   Idx  DT   ?    Time          Gap             Password      CRC16   CRC8
*/
#include "RealTimeRunDataCommand.h"
#include "Hoymiles.h"
#include "inverters/InverterAbstract.h"

RealTimeRunDataCommand::RealTimeRunDataCommand(const uint64_t target_address, const uint64_t router_address, const time_t time)
    : MultiDataCommand(target_address, router_address)
{
    setTime(time);
    setDataType(0x0b);
    setTimeout(500);
}

String RealTimeRunDataCommand::getCommandName() const
{
    return "RealTimeRunData";
}

bool RealTimeRunDataCommand::handleResponse(InverterAbstract& inverter, const fragment_t fragment[], const uint8_t max_fragment_id)
{
    // Check CRC of whole payload
    if (!MultiDataCommand::handleResponse(inverter, fragment, max_fragment_id)) {
        return false;
    }

    // Check if at least all required bytes are received
    // In case of low power in the inverter it occours that some incomplete fragments
    // with a valid CRC are received.
    const uint8_t fragmentsSize = getTotalFragmentSize(fragment, max_fragment_id);
    const uint8_t expectedSize = inverter.Statistics()->getExpectedByteCount();
    if (fragmentsSize < expectedSize) {
        Hoymiles.getMessageOutput()->printf("ERROR in %s: Received fragment size: %d, min expected size: %d\r\n",
            getCommandName().c_str(), fragmentsSize, expectedSize);

        return false;
    }

    // Move all fragments into target buffer
    uint8_t offs = 0;
    inverter.Statistics()->beginAppendFragment();
    inverter.Statistics()->clearBuffer();
    for (uint8_t i = 0; i < max_fragment_id; i++) {
        inverter.Statistics()->appendFragment(offs, fragment[i].fragment, fragment[i].len);
        offs += (fragment[i].len);
    }
    inverter.Statistics()->endAppendFragment();
    inverter.Statistics()->resetRxFailureCount();
    inverter.Statistics()->setLastUpdate(millis());
    return true;
}

void RealTimeRunDataCommand::gotTimeout(InverterAbstract& inverter)
{
    inverter.Statistics()->incrementRxFailureCount();
}