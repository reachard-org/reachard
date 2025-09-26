// Copyright 2025 Pavel Sobolev
//
// This file is part of the Reachard project, located at
//
//     https://reachard.paveloom.dev
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// SPDX-License-Identifier: Apache-2.0

package clickhouse

import (
	"context"

	"reachard/database/types"
)

func (database Database) DeleteUserTarget(ctx context.Context, userID types.UserID, targetID types.TargetID) error {
	const sql = `DELETE FROM "reachard.` + SchemaVersion + `".latencies WHERE user_id = $1 AND target_id = $2`
	err := database.Conn.Exec(ctx, sql, userID, targetID)
	if err != nil {
		return err
	}

	return nil
}
