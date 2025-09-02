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

	"reachard/database"
)

type TargetsHandler struct {
	DB database.Database
}

func (handler TargetsHandler) handleCORS(writer http.ResponseWriter, request *http.Request) {
	origin := request.Header.Get("Origin")
	if origin != "" {
		writer.Header().Set("Access-Control-Allow-Origin", request.Header.Get("Origin"))
	}
}

func (handler TargetsHandler) handleGet(writer http.ResponseWriter, request *http.Request) {
	handler.handleCORS(writer, request)

	targets, err := handler.DB.Targets(request.Context())
	if err != nil {
		http.Error(writer, "failed to get the targets", http.StatusInternalServerError)
		return
	}

	writer.Header().Set("Content-Type", "application/json")
	writer.Write([]byte(targets))
}

func (handler TargetsHandler) handleOptions(writer http.ResponseWriter, request *http.Request) {
	handler.handleCORS(writer, request)

	writer.Header().Set("Access-Control-Allow-Headers", "Content-Type")
	writer.Header().Set("Access-Control-Allow-Methods", "GET, POST, DELETE")
}

func (handler TargetsHandler) handlePost(writer http.ResponseWriter, request *http.Request) {
	handler.handleCORS(writer, request)

	rawRequestBody, err := io.ReadAll(request.Body)
	if err != nil {
		http.Error(writer, "failed to read the body", http.StatusInternalServerError)
		return
	}

	type RequestBody = database.Target

	var requestBody RequestBody
	err = json.Unmarshal(rawRequestBody, &requestBody)
	if err != nil {
		http.Error(writer, "bad request", http.StatusBadRequest)
		return
	}

	target := requestBody
	err = handler.DB.AddTarget(request.Context(), target)
	if err != nil {
		http.Error(writer, "failed to add the target", http.StatusInternalServerError)
		return
	}
}

func (handler TargetsHandler) handleDelete(writer http.ResponseWriter, request *http.Request) {
	handler.handleCORS(writer, request)

	type RequestBody struct {
		ID database.TargetID `json:"id"`
	}

	rawRequestBody, err := io.ReadAll(request.Body)
	if err != nil {
		http.Error(writer, "failed to read the body", http.StatusInternalServerError)
		return
	}

	var requestBody RequestBody
	err = json.Unmarshal(rawRequestBody, &requestBody)
	if err != nil {
		http.Error(writer, "bad request", http.StatusBadRequest)
		return
	}

	err = handler.DB.DeleteTarget(request.Context(), requestBody.ID)
	if err != nil {
		http.Error(writer, "failed to delete the target", http.StatusInternalServerError)
		return
	}
}

func (handler TargetsHandler) ServeHTTP(writer http.ResponseWriter, request *http.Request) {
	switch request.Method {
	case "GET":
		handler.handleGet(writer, request)
	case "OPTIONS":
		handler.handleOptions(writer, request)
	case "POST":
		handler.handlePost(writer, request)
	case "DELETE":
		handler.handleDelete(writer, request)
	default:
		http.Error(writer, "method not allowed", http.StatusMethodNotAllowed)
	}
}
