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

#ifndef DATABASEENV_H
#define DATABASEENV_H

#include "DatabaseWorkerPool.h"
#include "Define.h"

#include "Implementation/LoginDatabase.h"
#include "Implementation/ForumDatabase.h"

#include "Field.h"
#include "PreparedStatement.h"
#include "QueryCallback.h"
#include "QueryResult.h"
#include "Transaction.h"

/// Accessor to the realm/login database
WH_DATABASE_API extern DatabaseWorkerPool<LoginDatabaseConnection> LoginDatabase;

/// Accessor to the forum database
WH_DATABASE_API extern DatabaseWorkerPool<ForumDatabaseConnection> ForumDatabase;

#endif
