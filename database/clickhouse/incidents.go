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

type Incident struct {
	UserID    types.UserID   `ch:"user_id" json:"-"`
	TargetID  types.TargetID `ch:"target_id" json:"-"`
	Timestamp Timestamp      `ch:"timestamp" json:"timestamp"`
}

func (database Database) AddIncident(ctx context.Context, incident Incident) error {
	const sql = `INSERT INTO "reachard.` + SchemaVersion + `".incidents`
	batch, err := database.Conn.PrepareBatch(ctx, sql)
	if err != nil {
		return err
	}

	err = batch.AppendStruct(&incident)
	if err != nil {
		return err
	}

	err = batch.Send()
	if err != nil {
		return err
	}

	return nil
}

type Incidents struct {
	Timestamps []Timestamp `ch:"timestamps" json:"timestamps"`
}

type GetIncidentsOptions struct {
	Since Timestamp
}

func (database Database) GetIncidents(
	ctx context.Context,
	userID types.UserID,
	targetID types.TargetID,
	options GetIncidentsOptions,
) (Incidents, error) {
	sql := `
SELECT
	groupArray(toUnixTimestamp(timestamp)) AS timestamps
FROM
(
	SELECT timestamp
	FROM "reachard.` + SchemaVersion + `".incidents
	WHERE user_id = $1 AND target_id = $2 AND timestamp >= $3
)
`

	args := []any{userID, targetID, options.Since}
	row := database.Conn.QueryRow(ctx, sql, args...)

	var incidents Incidents
	err := row.ScanStruct(&incidents)
	if err != nil {
		return Incidents{}, err
	}

	return incidents, nil
}
