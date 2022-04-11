/*
 * This file is part of the WarheadCore Project. See AUTHORS file for Copyright information
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Affero General Public License as published by the
 * Free Software Foundation; either version 3 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _DONATE_H_
#define _DONATE_H_

#include "Define.h"
#include "AsyncCallbackProcessor.h"
#include "DatabaseEnvFwd.h"
#include "TaskScheduler.h"
#include "Duration.h"
#include <memory>
#include <atomic>

enum ShutdownExitCode
{
    SHUTDOWN_EXIT_CODE,
    ERROR_EXIT_CODE
};

class Donate
{
public:
    Donate() = default;
    ~Donate() = default;

    static Donate* instance();

    inline static uint8 GetExitCode() { return _exitCode; }
    inline static void StopNow(uint8 exitcode) { _stopEvent = true; _exitCode = exitcode; }
    inline static bool IsStopped() { return _stopEvent; }

    void Init();
    void Update(Milliseconds diff);

    static uint32 LoopCounter;

private:
    void ProcessQueryCallbacks();
    void CheckDonateCallback(PreparedQueryResult result);
    void ParseInfo(uint32 id, std::string_view info);
    void SendDonate(uint32 id, std::string_view realmName, std::string_view playerName, uint32 itemId, uint32 itemCount);
    void ScheduleTask();

    QueryCallbackProcessor _queryProcessor;
    TaskScheduler _scheduler;

    static std::atomic<bool> _stopEvent;
    static uint8 _exitCode;
};

#define sDonate Donate::instance()

#endif
