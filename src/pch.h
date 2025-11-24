#pragma once

#include <RE/Skyrim.h>
#include "SKSE/SKSE.h"
#include <windows.h>
#include <spdlog/sinks/basic_file_sink.h>
#include "../include/QuickLootAPI.h"
#include "SimpleIni.h"
namespace logger = SKSE::log;
#include "CEActorUtils.h"
#include "CEGameEvents.h"
#include "CEGameMenuUtils.h"
#include "CEGlobals.h"
#include "CEIconUtils.h"
#include "CEMenu.h"
#include "CEThumbStick.h"

#define DLLEXPORT __declspec(dllexport)

using namespace std::literals;