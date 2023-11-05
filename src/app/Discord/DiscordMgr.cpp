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

#include "DiscordMgr.h"
#include "Config.h"
#include "Log.h"
#include "StopWatch.h"
#include "Util.h"
#include <dpp/cluster.h>
#include <dpp/message.h>

namespace
{
    constexpr auto DEFAULT_CATEGORY_NAME = "donate-logs";

    // General
    constexpr auto CHANNEL_NAME_GENERAL = "general";

    // Owner
    constexpr auto OWNER_ID = 365169287926906883; // Winfidonarleyan | <@365169287926906883>
    constexpr auto OWNER_MENTION = "<@365169287926906883>";

    constexpr DiscordChannelType GetDiscordChannelType(std::string_view channelName)
    {
        if (channelName == CHANNEL_NAME_GENERAL)
            return DiscordChannelType::General;

        return DiscordChannelType::MaxType;
    }

    std::string GetChannelName(DiscordChannelType type)
    {
        switch (type)
        {
            case DiscordChannelType::General:
                return CHANNEL_NAME_GENERAL;
            default:
                return "";
        }
    }
}

DiscordMgr* DiscordMgr::instance()
{
    static DiscordMgr instance;
    return &instance;
}

DiscordMgr::~DiscordMgr()
{
    if (_bot)
        _bot->shutdown();
}

void DiscordMgr::LoadConfig()
{
    _isEnable = sConfigMgr->GetOption<bool>("Discord.Bot.Enable", false);

    if (!_isEnable)
        return;

    // Load config options
    {
        _botToken = sConfigMgr->GetOption<std::string>("Discord.Bot.Token", "");
        if (_botToken.empty())
        {
            LOG_CRIT("discord", "> Empty bot token for discord. Disable system");
            _isEnable = false;
            return;
        }

        _guildID = sConfigMgr->GetOption<int64>("Discord.Guild.ID", 0);
        if (!_guildID)
        {
            LOG_CRIT("discord", "> Empty guild id for discord. Disable system");
            _isEnable = false;
            return;
        }
    }

    Start();
}

void DiscordMgr::Start()
{
    if (!_isEnable)
        return;

    LOG_INFO("server.loading", "Loading discord bot...");

    StopWatch sw;

    _bot = std::make_unique<dpp::cluster>(_botToken, dpp::i_unverified_default_intents);

    // Prepare logs
    ConfigureLogs();

    _bot->start(dpp::st_return);

    // Check bot in guild, category and text channels
    CheckGuild();

    LOG_INFO("server.loading", ">> Discord bot is initialized in {}", sw);
    LOG_INFO("server.loading", "");
}

void DiscordMgr::Stop()
{
    if (_bot)
        _bot->shutdown();

    _bot.reset();
}

void DiscordMgr::SendDefaultMessage(std::string_view message, DiscordChannelType channelType /*= DiscordChannelType::MaxType*/)
{
    if (!_isEnable || !_bot)
        return;

    dpp::message discordMessage;
    discordMessage.channel_id = GetChannelID(channelType == DiscordChannelType::MaxType ? DiscordChannelType::General : channelType);
    discordMessage.content = std::string(message);

    _bot->message_create(discordMessage);
}

void DiscordMgr::SendEmbedMessage(DiscordEmbedMsg const& embed, DiscordChannelType channelType /*= DiscordChannelType::MaxType*/)
{
    if (!_isEnable || !_bot)
        return;

    _bot->message_create(dpp::message(GetChannelID(channelType == DiscordChannelType::MaxType ? DiscordChannelType::General : channelType), *embed.GetMessage()));
}

void DiscordMgr::ConfigureLogs()
{
    if (!_isEnable)
        return;

    _bot->on_ready([this](const auto&)
    {
        LOG_INFO("discord.bot", "DiscordBot: Logged in as {}", _bot->me.username);
    });

    _bot->on_log([](const dpp::log_t& event)
    {
        switch (event.severity)
        {
            case dpp::ll_trace:
                LOG_TRACE("discord.bot", "DiscordBot: {}", event.message);
                break;
            case dpp::ll_debug:
                LOG_DEBUG("discord.bot", "DiscordBot: {}", event.message);
                break;
            case dpp::ll_info:
                LOG_INFO("discord.bot", "DiscordBot: {}", event.message);
                break;
            case dpp::ll_warning:
                LOG_WARN("discord.bot", "DiscordBot: {}", event.message);
                break;
            case dpp::ll_error:
                LOG_ERROR("discord.bot", "DiscordBot: {}", event.message);
                break;
            case dpp::ll_critical:
                LOG_CRIT("discord.bot", "DiscordBot: {}", event.message);
                break;
            default:
                break;
        }
    });
}

void DiscordMgr::CheckGuild()
{
    StopWatch sw;

    auto const& guilds = _bot->current_user_get_guilds_sync();
    if (guilds.empty())
    {
        LOG_ERROR("discord", "DiscordBot: Not found guilds. Disable bot");
        return;
    }

    bool isExistGuild{};

    for (auto const& [guildID, guild] : guilds)
    {
        if (guildID == _guildID)
        {
            isExistGuild = true;
            break;
        }
    }

    if (!isExistGuild)
    {
        LOG_ERROR("discord", "DiscordBot: Not found config guild: {}. Disable bot", _guildID);
        _isEnable = false;
        Stop();
        return;
    }

    LOG_DEBUG("discord", "DiscordBot: Found config guild: {}", _guildID);
    LOG_DEBUG("discord", "> Start check channels for guild id: {}", _guildID);

    auto GetCategory = [this](dpp::channel_map const& channels) -> uint64
    {
        for (auto const& [channelID, channel] : channels)
        {
            if (!channel.is_category())
                continue;

            if (channel.name == DEFAULT_CATEGORY_NAME)
            {
                LOG_DEBUG("discord", "> Category with name '{}' exist. ID {}", DEFAULT_CATEGORY_NAME, uint64(channelID));
                return channelID;
            }
        }

        return 0;
    };

    auto CreateCategory = [this]() -> uint64
    {
        try
        {
            dpp::channel categoryToCreate;
            categoryToCreate.set_guild_id(_guildID);
            categoryToCreate.set_name(DEFAULT_CATEGORY_NAME);
            categoryToCreate.set_flags(dpp::CHANNEL_CATEGORY);

            auto createdCategory = _bot->channel_create_sync(categoryToCreate);
            return createdCategory.id;
        }
        catch (dpp::rest_exception const& error)
        {
            LOG_ERROR("discord", "DiscordBot::CheckChannels: Error at create category: {}", error.what());
        }

        return 0;
    };

    auto GetTextChannels = [this](dpp::channel_map const& channels, uint64 findCategoryID)
    {
        for (auto const& [channelID, channel] : channels)
        {
            if (!channel.is_text_channel() || channel.parent_id != findCategoryID)
                continue;

            auto channelType = GetDiscordChannelType(channel.name);
            if (channelType == DiscordChannelType::MaxType)
                continue;

            _channels[static_cast<std::size_t>(channelType)] = channelID;
        }
    };

    auto CreateTextChannels = [this](uint64 findCategoryID)
    {
        for (size_t i = 0; i < DEFAULT_CHANNELS_COUNT; i++)
        {
            auto& channelID = _channels[i];
            if (!channelID)
            {
                auto channelType{ static_cast<DiscordChannelType>(i) };
                auto channelName = GetChannelName(channelType);
                if (channelName.empty())
                {
                    LOG_ERROR("discord", "> Empty get channel name for type {}", i);
                    continue;
                }

                try
                {
                    dpp::channel channelToCreate;
                    channelToCreate.set_guild_id(_guildID);
                    channelToCreate.set_name(channelName);
                    channelToCreate.set_flags(dpp::CHANNEL_TEXT);
                    channelToCreate.set_parent_id(findCategoryID);
                    channelToCreate.add_permission_overwrite(_guildID, dpp::overwrite_type::ot_role, 0, dpp::permissions::p_view_channel);

                    auto createdChannel = _bot->channel_create_sync(channelToCreate);
                    channelID = createdChannel.id;
                    LOG_INFO("discord", "> Created channel {}. ID {}", channelName, uint64(createdChannel.id));
                }
                catch (dpp::rest_exception const& error)
                {
                    LOG_ERROR("discord", "DiscordBot::CheckChannels: Error at create text channel: {}", error.what());
                    continue;
                }
            }
        }
    };

    // Clear channels
    _channels.fill(0);

    try
    {
        uint64 findCategoryID{};

        auto const& channels = _bot->channels_get_sync(_guildID);
        if (channels.empty())
        {
            LOG_CRIT("discord", "> Empty channels in guild. Guild is new?");
            findCategoryID = CreateCategory();
        }

        // Exist any channel
        if (!findCategoryID)
            findCategoryID = GetCategory(channels);

        // Not found DEFAULT_CATEGORY_NAME
        if (!findCategoryID)
        {
            LOG_ERROR("discord", "> Category with name '{}' not found. Start creating", DEFAULT_CATEGORY_NAME);
            findCategoryID = CreateCategory();

            if (!findCategoryID)
            {
                LOG_INFO("discord", "> Error after create category with name '{}'", DEFAULT_CATEGORY_NAME);
                return;
            }

            LOG_INFO("discord", "> Category with name '{}' created. ID: {}", DEFAULT_CATEGORY_NAME, findCategoryID);
        }

        // Exist DEFAULT_CATEGORY_NAME
        GetTextChannels(channels, findCategoryID);
        CreateTextChannels(findCategoryID);
    }
    catch (dpp::rest_exception const& error)
    {
        LOG_ERROR("discord", "DiscordBot::CheckChannels: Error at check channels: {}", error.what());
        return;
    }

    LOG_DEBUG("discord", "> End check text channels guild. Elapsed: {}", sw);
}

uint64 DiscordMgr::GetChannelID(DiscordChannelType channelType)
{
    if (channelType >= DiscordChannelType::MaxType)
    {
        LOG_ERROR("discord", "> Incorrect channel type {}", static_cast<std::size_t>(channelType));
        return 0;
    }

    auto channelID{ _channels.at(static_cast<std::size_t>(channelType)) };
    if (!channelID)
        LOG_ERROR("discord", "> Empty channel id for type {}", static_cast<std::size_t>(channelType));

    return channelID;
}
