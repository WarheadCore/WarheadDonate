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

#include "Donate.h"
#include "Config.h"
#include "DatabaseEnv.h"
#include "DiscordMgr.h"
#include "StringConvert.h"
#include "Log.h"
#include <nlohmann/json.hpp>

std::atomic<bool> Donate::_stopEvent = false;
uint8 Donate::_exitCode = SHUTDOWN_EXIT_CODE;
uint32 Donate::LoopCounter = 0;

constexpr auto NEXUS_QUANTITY = "quantity";
constexpr auto NEXUS_ITEM_ID = "itemID";
constexpr auto NEXUS_C_FIELDS = "cfields";

/*static*/ Donate* Donate::instance()
{
    static Donate instance;
    return &instance;
}

void Donate::Update(Milliseconds diff)
{
    ProcessQueryCallbacks();
    _scheduler.Update(diff);
}

void Donate::ScheduleTask()
{
    _queryProcessor.AddCallback(ForumDatabase.AsyncQuery(ForumDatabase.GetPreparedStatement(FORUM_SEL_NEXUS_INVOICES)).WithPreparedCallback(std::bind(&Donate::CheckDonateCallback, this, std::placeholders::_1)));
}

void Donate::Init()
{
    _repeatDelay = Seconds(sConfigMgr->GetOption<int64>("Donate.Repeat.Delay", 30));
    if (_repeatDelay < 5s)
    {
        LOG_ERROR("donate", "Incorrect repeat delay {} sec. Set 5 sec.", _repeatDelay.count());
        _repeatDelay = 5s;
    }

    _scheduler.Schedule(5s, [this](TaskContext context)
    {
        ScheduleTask();
        context.Repeat(_repeatDelay);
    });

    _scheduler.Schedule(30min, [](TaskContext context)
    {
        LOG_DEBUG("sql.driver", "Ping MySQL to keep connection alive");
        LoginDatabase.KeepAlive();
        ForumDatabase.KeepAlive();
        context.Repeat();
    });

    sDiscordMgr->LoadConfig();
}

void Donate::ProcessQueryCallbacks()
{
    _queryProcessor.ProcessReadyCallbacks();
}

void Donate::CheckDonateCallback(PreparedQueryResult result)
{
    if (!result)
        return;

    do
    {
        auto const& [id, info] = result->FetchTuple<uint32, std::string_view>();
        ParseInfo(id, info);

    } while (result->NextRow());
}

void Donate::ParseInfo(uint32 id, std::string_view info)
{
    if (info.empty())
    {
        LOG_ERROR("donate", "Empty info!");
        return;
    }

    using nlohmann::json;

    try
    {
        auto jsonInfo = json::parse(info);
        if (!jsonInfo.is_array())
        {
            LOG_ERROR("donate", "Json data is not array '{}'!", info);
            return;
        }

        for (auto const& json : jsonInfo)
        {
            if (!json.contains(NEXUS_QUANTITY))
            {
                LOG_ERROR("donate", "Not found field '{}'!", NEXUS_QUANTITY);
                return;
            }

            if (!json.contains(NEXUS_ITEM_ID))
            {
                LOG_ERROR("donate", "Not found field '{}'!", NEXUS_ITEM_ID);
                return;
            }

            if (!json.contains(NEXUS_C_FIELDS))
            {
                LOG_ERROR("donate", "Not found field '{}'!", NEXUS_C_FIELDS);
                return;
            }

            auto itemCount = json[NEXUS_QUANTITY].get<uint32>();
            auto itemId = json[NEXUS_ITEM_ID].get<uint32>();

            std::string realmName;
            std::string playerName;

            uint32 count = 1;
            for (auto const& cField : json[NEXUS_C_FIELDS])
            {
                if (count == 2)
                    realmName = cField.get<std::string_view>();

                if (count == 1)
                    playerName = cField.get<std::string_view>();

                count++;
            }

            SendDonate(id, realmName, playerName, itemId, itemCount);
        }
    }
    catch (nlohmann::detail::exception const& e)
    {
        LOG_ERROR("donate", "Error at parse: {}. Data: {}", e.what(), info);
    }
}

void Donate::SendDonate(uint32 id, std::string_view realmName, std::string_view playerName, uint32 itemId, uint32 itemCount)
{
    auto loginStmt = LoginDatabase.GetPreparedStatement(LOGIN_INS_IPS_SHOP_LINK);
    loginStmt->SetArguments(playerName, realmName, itemId, itemCount);
    LoginDatabase.Execute(loginStmt);

    auto forumStmt = ForumDatabase.GetPreparedStatement(FORUM_UPD_NEXUS_INVOICES);
    forumStmt->SetArguments(id);
    ForumDatabase.Execute(forumStmt);

    LOG_INFO("donate", "Send donate. ID {}. Realm {}. Player {}. Item {}. Count {}",
        id, realmName, playerName, itemId, itemCount);

    DiscordEmbedMsg message;
    message.SetTitle(Warhead::StringFormat("Покупка на игровом мире; `{}`", realmName));
    message.AddEmbedField("Игрок", playerName);
    message.AddEmbedField("Предмет", Warhead::ToString(itemId));
    message.AddEmbedField("Количество", Warhead::ToString(itemCount));
    sDiscordMgr->SendEmbedMessage(message, DiscordChannelType::General);
}
