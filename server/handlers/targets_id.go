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
	"errors"
	"net/http"
	"strconv"

	"reachard/database"
	"reachard/database/postgresql"
)

type TargetsIDHandler struct {
	Handler
}

func NewTargetsIDHandler(db database.Database) TargetsIDHandler {
	return TargetsIDHandler{Handler{DB: db}}
}

func (handler TargetsIDHandler) ParseTargetID(writer http.ResponseWriter, request *http.Request) postgresql.TargetID {
	pathValueIDString := request.PathValue("id")
	pathValueID, err := strconv.Atoi(pathValueIDString)
	if err != nil {
		http.Error(writer, "only integer IDs are supported in the URL path", http.StatusBadRequest)
		return -1
	}

	targetID := postgresql.TargetID(pathValueID)
	return targetID
}

func (handler TargetsIDHandler) HandleGet(writer http.ResponseWriter, request *http.Request) {
	ctx := request.Context()

	sessionInfo, authenticated := handler.AuthenticateBySessionToken(writer, request)
	if !authenticated {
		return
	}

	targetID := handler.ParseTargetID(writer, request)
	if targetID < 0 {
		return
	}

	target, err := handler.DB.PostgreSQL.GetUserTarget(ctx, sessionInfo.UserID, targetID)
	if err != nil {
		var errInternalServerError postgresql.ErrInternalServerError
		var errNotFound postgresql.ErrNotFound
		switch {
		case errors.As(err, &errInternalServerError):
			http.Error(writer, errInternalServerError.UserMsg, http.StatusInternalServerError)
		case errors.As(err, &errNotFound):
			http.Error(writer, errNotFound.UserMsg, http.StatusNotFound)
		}
		return
	}

	json, err := json.Marshal(target)
	if err != nil {
		http.Error(writer, "internal server error", http.StatusInternalServerError)
		return
	}

	writer.Header().Set("Content-Type", "application/json")
	writer.Write(json)
}

func (handler TargetsIDHandler) HandleDelete(writer http.ResponseWriter, request *http.Request) {
	ctx := request.Context()

	sessionInfo, authenticated := handler.AuthenticateBySessionToken(writer, request)
	if !authenticated {
		return
	}

	targetID := handler.ParseTargetID(writer, request)
	if targetID < 0 {
		return
	}

	err := handler.DB.PostgreSQL.DeleteUserTarget(ctx, sessionInfo.UserID, targetID)
	if err != nil {
		http.Error(writer, "failed to delete the target", http.StatusInternalServerError)
		return
	}
}

func (handler TargetsIDHandler) HandleOptions(writer http.ResponseWriter, request *http.Request) {
	writer.Header().Set("Access-Control-Allow-Headers", "Authorization, Content-Type")
	writer.Header().Set("Access-Control-Allow-Methods", "DELETE")
}

func (handler TargetsIDHandler) ServeHTTP(writer http.ResponseWriter, request *http.Request) {
	handler.HandleCORS(writer, request)

	switch request.Method {
	case "GET":
		handler.HandleGet(writer, request)
	case "DELETE":
		handler.HandleDelete(writer, request)
	case "OPTIONS":
		handler.HandleOptions(writer, request)
	default:
		http.Error(writer, "method not allowed", http.StatusMethodNotAllowed)
	}
}
