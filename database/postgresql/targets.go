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

package postgresql

import (
	"context"

	"github.com/jackc/pgx/v5"
)

type TargetID = int32

type Target struct {
	ID              int32  `json:"id"`
	URL             string `json:"url"`
	IntervalSeconds int32  `json:"interval_seconds"`
}

func (db Database) GetTargets(ctx context.Context) ([]Target, error) {
	const sql = "SELECT * FROM " + SchemaVersion + ".targets"
	rows, err := db.Pool.Query(ctx, sql)
	if err != nil {
		return nil, err
	}

	targets, err := pgx.CollectRows(rows, pgx.RowToStructByName[Target])
	if err != nil {
		return nil, err
	}

	return targets, nil
}

func (db Database) AddTarget(ctx context.Context, target Target) (TargetID, error) {
	const sql = "INSERT INTO " +
		SchemaVersion + ".targets (url, interval_seconds) VALUES ($1, $2) RETURNING id"
	row := db.Pool.QueryRow(ctx, sql, target.URL, target.IntervalSeconds)

	var targetID TargetID
	err := row.Scan(&targetID)
	if err != nil {
		return 0, err
	}

	return targetID, nil
}

func (db Database) DeleteTarget(ctx context.Context, id TargetID) error {
	const sql = "DELETE FROM " + SchemaVersion + ".targets WHERE id = $1"
	_, err := db.Pool.Exec(ctx, sql, id)
	if err != nil {
		return err
	}

	return nil
}
