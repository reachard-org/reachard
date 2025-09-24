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
	"time"
)

type UserID = int32
type TargetID = int32
type Timestamp = time.Time
type Latency = int64

type CheckResult struct {
	UserID    UserID    `ch:"user_id" json:"-"`
	TargetID  TargetID  `ch:"target_id" json:"-"`
	Timestamp Timestamp `ch:"timestamp" json:"timestamp"`
	Latency   Latency   `ch:"latency" json:"latency"`
}

func (database Database) AddCheckResults(ctx context.Context, checkResults []CheckResult) error {
	const sql = `INSERT INTO "reachard.` + SchemaVersion + `".check_results`
	batch, err := database.Conn.PrepareBatch(ctx, sql)
	if err != nil {
		return err
	}
	defer batch.Close()

	for _, checkResult := range checkResults {
		err := batch.AppendStruct(&checkResult)
		if err != nil {
			return err
		}
	}

	err = batch.Send()
	if err != nil {
		return err
	}

	return nil
}

type CheckResults struct {
	Timestamps []Timestamp `ch:"timestamps" json:"timestamps"`
	Latencies  []Latency   `ch:"latencies" json:"latencies"`
}

func (database Database) GetCheckResults(
	ctx context.Context,
	userID UserID,
	targetID TargetID,
) (CheckResults, error) {
	const sql = "SELECT groupArray(timestamp) AS timestamps, groupArray(latency) AS latencies " +
		`FROM (SELECT timestamp, latency FROM "reachard.` + SchemaVersion + `".check_results ` +
		"WHERE user_id = $1 and target_id = $2 ORDER BY timestamp)"
	row := database.Conn.QueryRow(ctx, sql, userID, targetID)

	var checkResults CheckResults
	err := row.ScanStruct(&checkResults)
	if err != nil {
		return CheckResults{}, err
	}

	return checkResults, nil
}
