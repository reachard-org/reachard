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
	"reachard/database/postgresql"
)

type SessionHandler struct {
	DB database.Database
}

func (handler SessionHandler) HandlePost(writer http.ResponseWriter, request *http.Request) {
	rawRequestBody, err := io.ReadAll(request.Body)
	if err != nil {
		http.Error(writer, "failed to read the body", http.StatusInternalServerError)
		return
	}

	type RequestBody = postgresql.Credentials

	var requestBody RequestBody
	err = json.Unmarshal(rawRequestBody, &requestBody)
	if err != nil {
		http.Error(writer, "failed to parse the body as JSON", http.StatusBadRequest)
		return
	}

	ctx := request.Context()

	credentials := requestBody
	userID, err := handler.DB.PostgreSQL.AuthenticateByCredentials(ctx, credentials)
	if err != nil {
		switch err := err.(type) {
		case postgresql.ErrUnauthorized:
			http.Error(writer, err.UserMsg, http.StatusUnauthorized)
		case postgresql.ErrInternalServerError:
			http.Error(writer, err.UserMsg, http.StatusInternalServerError)
		}
		return
	}

	sessionToken, err := handler.DB.PostgreSQL.CreateSessionToken(ctx, userID)
	if err != nil {
		http.Error(writer, "internal server error", http.StatusInternalServerError)
		return
	}

	json, err := json.Marshal(sessionToken)
	if err != nil {
		http.Error(writer, "internal server error", http.StatusInternalServerError)
		return
	}

	writer.Header().Set("Content-Type", "application/json")
	writer.Write(json)
}

func (handler SessionHandler) ServeHTTP(writer http.ResponseWriter, request *http.Request) {
	switch request.Method {
	case "POST":
		handler.HandlePost(writer, request)
	default:
		http.Error(writer, "method not allowed", http.StatusMethodNotAllowed)
	}
}
