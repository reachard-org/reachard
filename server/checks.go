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

func (server Server) CheckTarget(target postgresql.Target) (clickhouse.Latency, error) {
	startTime := time.Now()

	response, err := http.Get(target.URL)
	if err != nil {
		return clickhouse.Latency{}, err
	}
	defer response.Body.Close()

	duration := time.Since(startTime)
	timestamp := startTime.Unix()
	latencyValue := duration.Milliseconds()

	latency := clickhouse.Latency{
		UserID:    target.UserID,
		TargetID:  target.ID,
		Timestamp: timestamp,
		Value:     latencyValue,
	}

	return latency, nil
}

func (server Server) CheckTargets(ctx context.Context) error {
	targets, err := server.DB.PostgreSQL.GetTargets(ctx)
	if err != nil {
		return err
	}

	latencies := make([]clickhouse.Latency, 0, len(targets))
	for _, target := range targets {
		latency, err := server.CheckTarget(target)
		if err != nil {
			return err
		}

		latencies = append(latencies, latency)
	}

	err = server.DB.ClickHouse.AddLatencies(ctx, latencies)
	if err != nil {
		return err
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
