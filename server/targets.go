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
	"encoding/json"
	"io"
	"net/http"
	"strconv"

	"reachard/database/postgresql"
)

type TargetsHandler struct {
	Handler
}

func (handler TargetsHandler) HandleDelete(writer http.ResponseWriter, request *http.Request) {
	ctx := request.Context()

	rawRequestBody, err := io.ReadAll(request.Body)
	if err != nil {
		http.Error(writer, "failed to read the body", http.StatusInternalServerError)
		return
	}

	type RequestBody = postgresql.TargetID

	var requestBody RequestBody
	err = json.Unmarshal(rawRequestBody, &requestBody)
	if err != nil {
		http.Error(writer, "bad request", http.StatusBadRequest)
		return
	}

	targetID := requestBody
	err = handler.DB.PostgreSQL.DeleteTarget(ctx, targetID)
	if err != nil {
		http.Error(writer, "failed to delete the target", http.StatusInternalServerError)
		return
	}
}

func (handler TargetsHandler) HandleGet(writer http.ResponseWriter, request *http.Request) {
	ctx := request.Context()

	targets, err := handler.DB.PostgreSQL.GetTargets(ctx)
	if err != nil {
		http.Error(writer, "failed to get the targets", http.StatusInternalServerError)
		return
	}

	json, err := json.Marshal(targets)
	if err != nil {
		http.Error(writer, "failed to convert the targets to JSON", http.StatusInternalServerError)
		return
	}

	writer.Header().Set("Content-Type", "application/json")
	writer.Write(json)
}

func (handler TargetsHandler) HandleOptions(writer http.ResponseWriter, request *http.Request) {
	writer.Header().Set("Access-Control-Allow-Headers", "Authorization, Content-Type")
	writer.Header().Set("Access-Control-Allow-Methods", "GET, POST, DELETE")
}

func (handler TargetsHandler) HandlePost(writer http.ResponseWriter, request *http.Request) {
	ctx := request.Context()

	sessionInfo, authenticated := handler.AuthenticateBySessionToken(writer, request)
	if !authenticated {
		return
	}

	rawRequestBody, err := io.ReadAll(request.Body)
	if err != nil {
		http.Error(writer, "failed to read the body", http.StatusInternalServerError)
		return
	}

	type RequestBody = postgresql.Target

	var requestBody RequestBody
	err = json.Unmarshal(rawRequestBody, &requestBody)
	if err != nil {
		http.Error(writer, "failed to parse the body as JSON", http.StatusBadRequest)
		return
	}

	target := requestBody
	target.UserID = sessionInfo.UserID

	targetID, err := handler.DB.PostgreSQL.AddTarget(ctx, target)
	if err != nil {
		http.Error(writer, "failed to add the target", http.StatusInternalServerError)
		return
	}

	requestURLString := request.URL.String()
	targetIDString := strconv.Itoa(int(targetID))
	writer.Header().Set("Location", requestURLString+targetIDString)
	writer.WriteHeader(http.StatusCreated)
}

func (handler TargetsHandler) ServeHTTP(writer http.ResponseWriter, request *http.Request) {
	handler.HandleCORS(writer, request)

	if request.Method != "OPTIONS" && request.Method != "POST" {
		_, authenticated := handler.AuthenticateBySessionToken(writer, request)
		if !authenticated {
			return
		}
	}

	switch request.Method {
	case "DELETE":
		handler.HandleDelete(writer, request)
	case "GET":
		handler.HandleGet(writer, request)
	case "OPTIONS":
		handler.HandleOptions(writer, request)
	case "POST":
		handler.HandlePost(writer, request)
	default:
		http.Error(writer, "method not allowed", http.StatusMethodNotAllowed)
	}
}
