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

package database

import (
	"context"
	"encoding/json"
)

func (db Database) Targets(ctx context.Context) (string, error) {
	const sql = "select json_agg(row_to_json(t)) from (select * from " + SchemaVersion + ".targets) t"
	row := db.Pool.QueryRow(ctx, sql)

	var result string
	err := row.Scan(&result)
	if err != nil {
		return "", err
	}

	return result + "\n", nil
}

type Target struct {
	URL             string `json:"url"`
	IntervalSeconds int32  `json:"interval_seconds"`
}

func (db Database) AddTarget(ctx context.Context, input []byte) error {
	var target Target
	err := json.Unmarshal(input, &target)
	if err != nil {
		return err
	}

	const sql = "INSERT INTO " + SchemaVersion + ".targets (url, interval_seconds) VALUES ($1, $2)"
	_, err = db.Pool.Exec(ctx, sql, target.URL, target.IntervalSeconds)
	if err != nil {
		return err
	}

	return nil
}

type TargetID int32

func (db Database) DeleteTarget(ctx context.Context, id TargetID) error {
	const sql = "DELETE FROM " + SchemaVersion + ".targets WHERE id = $1"
	_, err := db.Pool.Exec(ctx, sql, id)
	if err != nil {
		return err
	}

	return nil
}
