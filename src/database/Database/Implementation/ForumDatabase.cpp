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

#include "ForumDatabase.h"
#include "MySQLPreparedStatement.h"

void ForumDatabaseConnection::DoPrepareStatements()
{
    if (!m_reconnecting)
        m_stmts.resize(MAX_FORUM_DATABASE_STATEMENTS);

    PrepareStatement(FORUM_SEL_NEXUS_INVOICES, "SELECT `i_id`, `i_items` FROM `nexus_invoices` WHERE i_status = 'paid' AND flag = 0", CONNECTION_ASYNC);
    PrepareStatement(FORUM_UPD_NEXUS_INVOICES, "UPDATE `nexus_invoices` SET `flag` = 1 WHERE `i_id` = ?", CONNECTION_ASYNC);
}
