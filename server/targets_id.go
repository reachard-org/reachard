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
	"net/http"
	"strconv"

	"reachard/database/postgresql"
)

type TargetsIDHandler struct {
	Handler
}

func (handler TargetsIDHandler) HandleDelete(writer http.ResponseWriter, request *http.Request) {
	ctx := request.Context()

	sessionInfo, authenticated := handler.AuthenticateBySessionToken(writer, request)
	if !authenticated {
		return
	}

	pathValueIDString := request.PathValue("id")
	pathValueID, err := strconv.Atoi(pathValueIDString)
	if err != nil {
		http.Error(writer, "only integer IDs are supported in the URL path", http.StatusBadRequest)
		return
	}

	targetID := postgresql.TargetID(pathValueID)
	target := postgresql.Target{ID: targetID, UserID: sessionInfo.UserID}

	err = handler.DB.PostgreSQL.DeleteTarget(ctx, target)
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
	case "DELETE":
		handler.HandleDelete(writer, request)
	case "OPTIONS":
		handler.HandleOptions(writer, request)
	default:
		http.Error(writer, "method not allowed", http.StatusMethodNotAllowed)
	}
}
