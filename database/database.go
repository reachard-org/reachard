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
	_ "embed"
	"fmt"

	"reachard/database/clickhouse"
	"reachard/database/postgresql"
)

type Database struct {
	PostgreSQL postgresql.Database
	ClickHouse clickhouse.Database
}

func Connect(ctx context.Context, connString string) (Database, error) {
	PostgreSQL, err := postgresql.Connect(ctx, connString)
	if err != nil {
		return Database{}, fmt.Errorf("couldn't connect to the PostgreSQL database: %v", err)
	}

	ClickHouse, err := clickhouse.Connect(ctx)
	if err != nil {
		return Database{}, fmt.Errorf("couldn't connect to the ClickHouse database: %v", err)
	}

	return Database{PostgreSQL, ClickHouse}, nil
}

func (db Database) ExecSchemas(ctx context.Context) error {
	err := db.PostgreSQL.ExecSchema(ctx)
	if err != nil {
		return fmt.Errorf("failed to execute the PostgreSQL schema: %v", err)
	}

	err = db.ClickHouse.ExecSchema(ctx)
	if err != nil {
		return fmt.Errorf("failed to execute the ClickHouse schema: %v", err)
	}

	return nil
}

func (db Database) Close() {
	db.PostgreSQL.Close()
	db.ClickHouse.Close()
}
