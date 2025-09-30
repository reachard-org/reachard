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
	"errors"

	"reachard/database/types"

	"github.com/jackc/pgx/v5"
)

type Target struct {
	ID              types.TargetID `json:"id"`
	UserID          types.UserID   `json:"-"`
	TimeAdded       Timestamp      `json:"time_added"`
	Name            string         `json:"name"`
	URL             string         `json:"url"`
	IntervalSeconds int32          `json:"interval_seconds"`
}

func (db Database) getTargetWith(ctx context.Context, sql string, args ...any) (Target, error) {
	rows, _ := db.Pool.Query(ctx, sql, args...)
	target, err := pgx.CollectOneRow(rows, pgx.RowToStructByName[Target])
	if err != nil {
		if errors.Is(err, pgx.ErrNoRows) {
			return Target{}, errors.Join(ErrNotFound{
				"no target found",
				"no target found",
			}, err)
		}
		return Target{}, errors.Join(ErrInternalServerError{
			"internal server error",
			"failed to look up a target",
		}, err)
	}

	return target, nil
}

func (db Database) GetUserTarget(ctx context.Context, userID types.UserID, targetID types.TargetID) (Target, error) {
	const sql = "SELECT * FROM " + SchemaVersion + ".targets WHERE id = $1 AND user_id = $2"
	return db.getTargetWith(ctx, sql, targetID, userID)
}

func (db Database) getTargetsWith(ctx context.Context, sql string, args ...any) ([]Target, error) {
	rows, _ := db.Pool.Query(ctx, sql, args...)
	targets, err := pgx.CollectRows(rows, pgx.RowToStructByName[Target])
	if err != nil {
		return nil, err
	}

	return targets, nil
}

func (db Database) GetTargets(ctx context.Context) ([]Target, error) {
	const sql = "SELECT * FROM " + SchemaVersion + ".targets"
	return db.getTargetsWith(ctx, sql)
}

func (db Database) GetUserTargets(ctx context.Context, userID types.UserID) ([]Target, error) {
	const sql = "SELECT * FROM " + SchemaVersion + ".targets WHERE user_id = $1"
	return db.getTargetsWith(ctx, sql, userID)
}

func (db Database) AddTarget(ctx context.Context, target Target) (types.TargetID, error) {
	const sql = "INSERT INTO " +
		SchemaVersion + ".targets VALUES (DEFAULT, $1, $2, $3, $4, $5) RETURNING id"
	row := db.Pool.QueryRow(
		ctx, sql,
		target.UserID,
		target.TimeAdded,
		target.Name,
		target.URL,
		target.IntervalSeconds,
	)

	var targetID types.TargetID
	err := row.Scan(&targetID)
	if err != nil {
		return 0, err
	}

	return targetID, nil
}

func (db Database) DeleteUserTarget(ctx context.Context, userID types.UserID, targetID types.TargetID) error {
	const sql = "DELETE FROM " + SchemaVersion + ".targets WHERE id = $1 AND user_id = $2"
	_, err := db.Pool.Exec(ctx, sql, targetID, userID)
	if err != nil {
		return err
	}

	return nil
}
