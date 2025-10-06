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
	"fmt"
	"net/http"

	"reachard/database"
	"reachard/server/handlers"
)

type Server struct {
	DB      database.Database
	Handler http.Handler
}

func NewServer() (Server, error) {
	db, err := database.Connect(context.Background())
	if err != nil {
		return Server{}, fmt.Errorf("failed to connect to a database: %v", err)
	}

	mux := http.NewServeMux()
	mux.Handle("/v0/authorize/", handlers.NewAuthorizeHandler(db))
	mux.Handle("/v0/session/{$}", handlers.NewSessionHandler(db))
	mux.Handle("/v0/targets/{$}", handlers.NewTargetsHandler(db))
	mux.Handle("/v0/targets/{id}/{$}", handlers.NewTargetsIDHandler(db))
	mux.Handle("/v0/targets/{id}/incidents/{$}", handlers.NewTargetsIDIncidentsHandler(db))
	mux.Handle("/v0/targets/{id}/latencies/{$}", handlers.NewTargetsIDLatenciesHandler(db))

	return Server{DB: db, Handler: mux}, nil
}

func (server Server) ListenAndServe(addr string) error {
	ctx := context.Background()

	server.StartChecksLoop(ctx)

	err := http.ListenAndServe(addr, server.Handler)
	if err != nil {
		return err
	}

	return nil
}

func (server Server) Cleanup() {
	server.DB.Close()
}
