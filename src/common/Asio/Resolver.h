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

#ifndef Resolver_h__
#define Resolver_h__

#include "IoContext.h"
#include <boost/asio/ip/tcp.hpp>
#include <optional>
#include <string>

namespace Warhead::Asio
{
    /**
     Hack to make it possible to forward declare resolver (one of its template arguments is a typedef to something super long and using nested classes)
    */
    class Resolver
    {
    public:
        explicit Resolver(IoContext& ioContext) : _impl(ioContext) { }

        std::optional<boost::asio::ip::tcp::endpoint> Resolve(boost::asio::ip::tcp const& protocol, std::string const& host, std::string const& service)
        {
            boost::system::error_code ec;
            boost::asio::ip::resolver_base::flags flagsResolver = boost::asio::ip::resolver_base::all_matching;
            boost::asio::ip::tcp::resolver::results_type results = _impl.resolve(protocol, host, service, flagsResolver, ec);

            if (results.begin() == results.end() || ec)
                return {};

            return results.begin()->endpoint();
        }

    private:
        boost::asio::ip::tcp::resolver _impl;
    };
}

#endif // Resolver_h__
