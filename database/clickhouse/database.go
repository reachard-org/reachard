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
	_ "embed"
	"os"
	"regexp"
	"strings"

	"github.com/ClickHouse/clickhouse-go/v2"
	"github.com/ClickHouse/clickhouse-go/v2/lib/driver"
)

type Database struct {
	Conn driver.Conn
}

func Connect(ctx context.Context) (Database, error) {
	host := os.Getenv("REACHARD_CLICKHOUSE_HOST")
	port := os.Getenv("REACHARD_CLICKHOUSE_PORT")
	db := os.Getenv("REACHARD_CLICKHOUSE_DB")
	user := os.Getenv("REACHARD_CLICKHOUSE_USER")
	password := os.Getenv("REACHARD_CLICKHOUSE_PASSWORD")

	if port == "" {
		port = "9000"
	}

	addr := host + ":" + port

	conn, err := clickhouse.Open(&clickhouse.Options{
		Addr: []string{addr},
		Auth: clickhouse.Auth{
			Database: db,
			Username: user,
			Password: password,
		},
	})
	if err != nil {
		return Database{}, err
	}

	err = conn.Ping(ctx)
	if err != nil {
		return Database{}, err
	}

	return Database{Conn: conn}, nil
}

const SchemaVersion = "v0"

//go:embed schemas/v0.sql
var Schema string

func (db Database) ExecSchema(ctx context.Context) error {
	// ClickHouse supports neither multiple statements nor transactions

	lastCommentRegex := regexp.MustCompile(`--.*\n\n`)
	lastCommentLocation := lastCommentRegex.FindIndex([]byte(Schema))

	cleanSchema := Schema[lastCommentLocation[1] : len(Schema)-2]

	statements := strings.SplitSeq(cleanSchema, ";")
	for statement := range statements {
		err := db.Conn.Exec(ctx, statement)
		if err != nil {
			return err
		}
	}

	return nil
}

func (database Database) Close() {
	_ = database.Conn.Close()
}
