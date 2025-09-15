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

package server

import (
	"context"
	"log"
	"net/http"
	"time"

	"reachard/database/clickhouse"
	"reachard/database/postgresql"
)

func (server Server) CheckTarget(target postgresql.Target) (clickhouse.CheckResult, error) {
	response, err := http.Get(target.URL)
	if err != nil {
		return clickhouse.CheckResult{}, err
	}

	var status uint8 = 0
	if response.StatusCode < 200 || response.StatusCode > 299 {
		status = 1
	}

	checkResult := clickhouse.CheckResult{
		UserID:    target.UserID,
		URL:       target.URL,
		Timestamp: time.Now().UTC(),
		Status:    status,
	}

	return checkResult, nil
}

func (server Server) CheckAllTargets(ctx context.Context) error {
	targets, err := server.DB.PostgreSQL.GetAllTargets(ctx)
	if err != nil {
		return err
	}

	checkResults := make([]clickhouse.CheckResult, 0, len(targets))
	for _, target := range targets {
		checkResult, err := server.CheckTarget(target)
		if err != nil {
			return err
		}

		checkResults = append(checkResults, checkResult)
	}

	err = server.DB.ClickHouse.AddCheckResults(ctx, checkResults)
	if err != nil {
		return err
	}

	return nil
}

func (server Server) RunChecksLoop(ctx context.Context) {
	timer := time.Tick(5 * time.Second)

	for range timer {
		err := server.CheckAllTargets(ctx)
		if err != nil {
			log.Printf("Failed to check a target: %v", err)
		}
	}
}

func (server Server) StartChecksLoop(ctx context.Context) {
	go server.RunChecksLoop(ctx)
}
