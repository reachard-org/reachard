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

package handlers

import (
	"encoding/json"
	"fmt"
	"net/http"
	"strconv"

	"reachard/database"
	"reachard/database/clickhouse"
)

type TargetsIDLatenciesHandler struct {
	TargetsIDHandler
}

func NewTargetsIDLatenciesHandler(db database.Database) TargetsIDLatenciesHandler {
	return TargetsIDLatenciesHandler{TargetsIDHandler{Handler{DB: db}}}
}

func (handler TargetsIDLatenciesHandler) HandleGet(writer http.ResponseWriter, request *http.Request) {
	ctx := request.Context()

	sessionInfo, authenticated := handler.AuthenticateBySessionToken(writer, request)
	if !authenticated {
		return
	}

	targetID := handler.ParseTargetID(writer, request)
	if targetID < 0 {
		return
	}

	var since clickhouse.Timestamp
	sinceString := request.URL.Query().Get("since")
	if sinceString != "" {
		var err error
		since, err = strconv.ParseInt(sinceString, 10, 64)
		if err != nil {
			http.Error(writer, "couldn't parse the `since` query parameter", http.StatusBadRequest)
			return
		}
	}

	var step clickhouse.Step
	stepString := request.URL.Query().Get("step")
	if stepString != "" {
		var err error
		step, err = strconv.ParseUint(stepString, 10, 64)
		if err != nil {
			http.Error(writer, "couldn't parse the `step` query parameter", http.StatusBadRequest)
			return
		}
	}

	options := clickhouse.GetLatenciesOptions{Since: since, Step: step}
	latencies, err := handler.DB.ClickHouse.GetLatencies(ctx, sessionInfo.UserID, targetID, options)
	if err != nil {
		fmt.Println(err)
		http.Error(writer, "internal server error", http.StatusInternalServerError)
		return
	}

	json, err := json.Marshal(latencies)
	if err != nil {
		fmt.Println(err)
		http.Error(writer, "internal server error", http.StatusInternalServerError)
		return
	}

	writer.Header().Set("Content-Type", "application/json")
	writer.Write(json)
}

func (handler TargetsIDLatenciesHandler) HandleOptions(writer http.ResponseWriter, request *http.Request) {
	writer.Header().Set("Access-Control-Allow-Headers", "Authorization, Content-Type")
	writer.Header().Set("Access-Control-Allow-Methods", "GET")
}

func (handler TargetsIDLatenciesHandler) ServeHTTP(writer http.ResponseWriter, request *http.Request) {
	handler.HandleCORS(writer, request)

	switch request.Method {
	case "GET":
		handler.HandleGet(writer, request)
	case "OPTIONS":
		handler.HandleOptions(writer, request)
	default:
		http.Error(writer, "method not allowed", http.StatusMethodNotAllowed)
	}
}
