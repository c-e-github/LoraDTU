// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2024 C-E and others
 */

#include "WebApi_maintenance.h"
#include "Utils.h"
#include "WebApi.h"
#include "WebApi_errors.h"
#include <AsyncJson.h>

void WebApiMaintenanceClass::init(AsyncWebServer& server)
{
    using std::placeholders::_1;

    _server = &server;

    _server->on("/api/maintenance/reboot", HTTP_POST, std::bind(&WebApiMaintenanceClass::onRebootPost, this, _1));
}

void WebApiMaintenanceClass::loop()
{
}

void WebApiMaintenanceClass::onRebootPost(AsyncWebServerRequest* request)
{
    if (!WebApi.checkCredentials(request)) {
        return;
    }

    AsyncJsonResponse* response = new AsyncJsonResponse(false, 10240);
    JsonObject retMsg = response->getRoot();
    retMsg["type"] = "warning";

    if (!request->hasParam("data", true)) {
        retMsg["message"] = "No values found!";
        retMsg["code"] = WebApiError::GenericNoValueFound;
        response->setLength();
        request->send(response);
        return;
    }

    const String json = request->getParam("data", true)->value();

    if (json.length() > 10240) {
        retMsg["message"] = "Data too large!";
        retMsg["code"] = WebApiError::GenericDataTooLarge;
        response->setLength();
        request->send(response);
        return;
    }

    DynamicJsonDocument root(10240);
    const DeserializationError error = deserializeJson(root, json);

    if (error) {
        retMsg["message"] = "Failed to parse data!";
        retMsg["code"] = WebApiError::GenericParseError;
        response->setLength();
        request->send(response);
        return;
    }

    if (!(root.containsKey("reboot"))) {
        retMsg["message"] = "Values are missing!";
        retMsg["code"] = WebApiError::GenericValueMissing;
        response->setLength();
        request->send(response);
        return;
    }

    if (root["reboot"].as<bool>()) {
        retMsg["type"] = "success";
        retMsg["message"] = "Reboot triggered!";
        retMsg["code"] = WebApiError::MaintenanceRebootTriggered;

        response->setLength();
        request->send(response);
        Utils::restartDtu();
    } else {
        retMsg["message"] = "Reboot cancled!";
        retMsg["code"] = WebApiError::MaintenanceRebootCancled;

        response->setLength();
        request->send(response);
    }
}