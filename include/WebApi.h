// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once

#include "WebApi_config.h"
#include "WebApi_device.h"
#include "WebApi_devinfo.h"
#include "WebApi_dtu.h"
#include "WebApi_eventlog.h"
#include "WebApi_firmware.h"
#include "WebApi_gridprofile.h"
#include "WebApi_inverter.h"
#include "WebApi_limit.h"
#include "WebApi_maintenance.h"
#include "WebApi_network.h"
#include "WebApi_ntp.h"
#include "WebApi_power.h"
#include "WebApi_prometheus.h"
#include "WebApi_security.h"
#include "WebApi_sysstatus.h"
#include "WebApi_webapp.h"
#include "WebApi_ws_console.h"
#include "WebApi_ws_live.h"
#include <ESPAsyncWebServer.h>
#include <TaskSchedulerDeclarations.h>

class WebApiClass {
public:
    WebApiClass();
    void init(Scheduler& scheduler);

    static bool checkCredentials(AsyncWebServerRequest* request);
    static bool checkCredentialsReadonly(AsyncWebServerRequest* request);

    static void sendTooManyRequests(AsyncWebServerRequest* request);

private:
    void loop();

    Task _loopTask;

    AsyncWebServer _server;

    WebApiConfigClass _webApiConfig;
    WebApiDeviceClass _webApiDevice;
    WebApiDevInfoClass _webApiDevInfo;
    WebApiDtuClass _webApiDtu;
    WebApiEventlogClass _webApiEventlog;
    WebApiFirmwareClass _webApiFirmware;
    WebApiGridProfileClass _webApiGridprofile;
    WebApiInverterClass _webApiInverter;
    WebApiLimitClass _webApiLimit;
    WebApiMaintenanceClass _webApiMaintenance;
    WebApiNetworkClass _webApiNetwork;
    WebApiNtpClass _webApiNtp;
    WebApiPowerClass _webApiPower;
    WebApiPrometheusClass _webApiPrometheus;
    WebApiSecurityClass _webApiSecurity;
    WebApiSysstatusClass _webApiSysstatus;
    WebApiWebappClass _webApiWebapp;
    WebApiWsConsoleClass _webApiWsConsole;
    WebApiWsLiveClass _webApiWsLive;
};

extern WebApiClass WebApi;