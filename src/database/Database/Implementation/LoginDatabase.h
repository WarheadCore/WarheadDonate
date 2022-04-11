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

#ifndef _LOGINDATABASE_H
#define _LOGINDATABASE_H

#include "MySQLConnection.h"

enum LoginDatabaseStatements : uint32
{
    /*  Naming standard for defines:
        {DB}_{SEL/INS/UPD/DEL/REP}_{Summary of data changed}
        When updating more than one field, consider looking at the calling function
        name for a suiting suffix.
    */

    LOGIN_INS_IPS_SHOP_LINK,

    MAX_LOGINDATABASE_STATEMENTS
};

class WH_DATABASE_API LoginDatabaseConnection : public MySQLConnection
{
public:
    typedef LoginDatabaseStatements Statements;

    //- Constructors for sync and async connections
    LoginDatabaseConnection(MySQLConnectionInfo& connInfo);
    LoginDatabaseConnection(ProducerConsumerQueue<SQLOperation*>* q, MySQLConnectionInfo& connInfo);
    ~LoginDatabaseConnection() override;

    //- Loads database type specific prepared statements
    void DoPrepareStatements() override;
};

#endif
