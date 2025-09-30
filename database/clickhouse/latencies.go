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

type LatencyValue = int64

type Latency struct {
	UserID    types.UserID   `ch:"user_id" json:"-"`
	TargetID  types.TargetID `ch:"target_id" json:"-"`
	Timestamp Timestamp      `ch:"timestamp" json:"timestamp"`
	Value     LatencyValue   `ch:"value" json:"value"`
}

func (database Database) AddLatency(ctx context.Context, latency Latency) error {
	const sql = `INSERT INTO "reachard.` + SchemaVersion + `".latencies`
	batch, err := database.Conn.PrepareBatch(ctx, sql)
	if err != nil {
		return err
	}
	defer batch.Close()

	err = batch.AppendStruct(&latency)
	if err != nil {
		return err
	}

	err = batch.Send()
	if err != nil {
		return err
	}

	return nil
}

type Latencies struct {
	Timestamps []Timestamp    `ch:"timestamps" json:"timestamps"`
	Values     []LatencyValue `ch:"values" json:"values"`
}

type Step = uint64

type GetLatenciesOptions struct {
	Since Timestamp
	Step  Step
}

func (database Database) GetLatencies(
	ctx context.Context,
	userID types.UserID,
	targetID types.TargetID,
	options GetLatenciesOptions,
) (Latencies, error) {
	if options.Step == 0 {
		options.Step = 1
	}

	sql := `
SELECT
	groupArray(toUnixTimestamp(timestamp)) AS timestamps,
	groupArray(value) AS values
FROM
(
	SELECT timestamp, value
	FROM
	(
		SELECT
			timestamp,
			value,
			row_number() OVER (
                PARTITION BY user_id, target_id
                ORDER BY timestamp
            ) AS rn
		FROM "reachard.` + SchemaVersion + `".latencies
		WHERE user_id = $1 AND target_id = $2 AND timestamp >= $3
	)
	WHERE (rn - 1) % $4 = 0
	ORDER BY timestamp
)
`

	args := []any{userID, targetID, options.Since, options.Step}
	row := database.Conn.QueryRow(ctx, sql, args...)

	var latencies Latencies
	err := row.ScanStruct(&latencies)
	if err != nil {
		return Latencies{}, err
	}

	return latencies, nil
}
