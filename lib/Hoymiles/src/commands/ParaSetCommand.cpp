// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2024 C-E and others
 */
#include "ParaSetCommand.h"

ParaSetCommand::ParaSetCommand(const uint64_t target_address, const uint64_t router_address)
    : CommandAbstract(target_address, router_address)
{
    _payload[0] = 0x52;
}