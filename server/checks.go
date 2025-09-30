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

func (server Server) CheckTarget(ctx context.Context, target postgresql.Target) error {
	startTime := time.Now()

	response, err := http.Get(target.URL)
	if err != nil {
		return err
	}
	defer response.Body.Close()

	timestamp := startTime.Unix()

	if response.StatusCode >= 400 && response.StatusCode < 600 {
		incident := clickhouse.Incident{
			UserID:    target.UserID,
			TargetID:  target.ID,
			Timestamp: timestamp,
		}

		err := server.DB.ClickHouse.AddIncident(ctx, incident)
		if err != nil {
			return err
		}

		return nil
	}

	duration := time.Since(startTime)
	latencyValue := duration.Milliseconds()

	latency := clickhouse.Latency{
		UserID:    target.UserID,
		TargetID:  target.ID,
		Timestamp: timestamp,
		Value:     latencyValue,
	}

	err = server.DB.ClickHouse.AddLatency(ctx, latency)
	if err != nil {
		return err
	}

	return nil
}

func (server Server) CheckTargets(ctx context.Context) error {
	targets, err := server.DB.PostgreSQL.GetTargets(ctx)
	if err != nil {
		return err
	}

	for _, target := range targets {
		err := server.CheckTarget(ctx, target)
		if err != nil {
			return err
		}
	}

	return nil
}

func (server Server) RunChecksLoop(ctx context.Context) {
	timer := time.Tick(5 * time.Second)

	for range timer {
		err := server.CheckTargets(ctx)
		if err != nil {
			log.Printf("Failed to check a target: %v", err)
		}
	}
}

func (server Server) StartChecksLoop(ctx context.Context) {
	go server.RunChecksLoop(ctx)
}
