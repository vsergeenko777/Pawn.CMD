/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2016-2018 urShadow
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "SDK/amx/amx.h"
#include "SDK/plugincommon.h"

#include "urmem/urmem.hpp"
#include "crtp_singleton/crtp_singleton.hpp"

#include <unordered_map>
#include <unordered_set>
#include <regex>
#include <string>
#include <vector>
#include <list>
#include <queue>

#include <boost/locale.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>

#include "Pawn.CMD.inc"

extern void *pAMXFunctions;

#include "Settings.h"
#include "Logger.h"
#include "Functions.h"
#include "Scripts.h"
#include "Hooks.h"
#include "Natives.h"

namespace Plugin {
    bool Load(void **ppData) {
        try {
            pAMXFunctions = ppData[PLUGIN_DATA_AMX_EXPORTS];

            Logger::instance()->Init(ppData[PLUGIN_DATA_LOGPRINTF]);

            Settings::Read();

            Hooks::Init(*ppData);

            std::locale::global(boost::locale::generator{}(Settings::locale_name));

            Logger::instance()->Write("%s plugin v%s by urShadow has been loaded", Settings::kName, Settings::kVersion);

            return true;
        } catch (const std::exception &e) {
            Logger::instance()->Write("[%s] %s: %s", Settings::kName, __FUNCTION__, e.what());
        }

        return false;
    }

    void Unload() {
        Logger::instance()->Write("%s plugin v%s by urShadow has been unloaded", Settings::kName, Settings::kVersion);
    }

    void AmxLoad(AMX *amx) {
        Natives::Register(amx);

        Scripts::Load(amx);
    }

    void AmxUnload(AMX *amx) {
        Scripts::Unload(amx);
    }
}

PLUGIN_EXPORT unsigned int PLUGIN_CALL Supports() {
    return SUPPORTS_VERSION | SUPPORTS_AMX_NATIVES;
}

PLUGIN_EXPORT bool PLUGIN_CALL Load(void **ppData) {
    return Plugin::Load(ppData);
}

PLUGIN_EXPORT void PLUGIN_CALL Unload() {
    Plugin::Unload();
}

PLUGIN_EXPORT void PLUGIN_CALL AmxLoad(AMX *amx) {
    Plugin::AmxLoad(amx);
}

PLUGIN_EXPORT void PLUGIN_CALL AmxUnload(AMX *amx) {
    Plugin::AmxUnload(amx);
}
