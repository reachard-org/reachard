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
	"io"
	"net/http"

	"reachard/database"
	"reachard/database/postgresql"
)

type SessionHandler struct {
	Handler
}

func NewSessionHandler(db database.Database) SessionHandler {
	return SessionHandler{Handler{DB: db}}
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
		var errInternalServerError postgresql.ErrInternalServerError
		var errUnauthorized postgresql.ErrUnauthorized
		switch {
		case errors.As(err, &errInternalServerError):
			http.Error(writer, errInternalServerError.UserMsg, http.StatusInternalServerError)
		case errors.As(err, &errUnauthorized):
			http.Error(writer, errUnauthorized.UserMsg, http.StatusUnauthorized)
		}
		return
	}

	sessionToken, err := handler.DB.PostgreSQL.CreateSessionToken(ctx, userID)
	if err != nil {
		http.Error(writer, "internal server error", http.StatusInternalServerError)
		return
	}

	cookie := http.Cookie{
		Name:     "session_token",
		Value:    sessionToken,
		MaxAge:   3600,
		Path:     "/v0/authorize/",
		HttpOnly: true,
		Secure:   true,
		SameSite: http.SameSiteStrictMode,
	}
	http.SetCookie(writer, &cookie)

	writer.WriteHeader(http.StatusOK)
}

func (handler SessionHandler) HandleOptions(writer http.ResponseWriter, request *http.Request) {
	writer.Header().Set("Access-Control-Allow-Headers", "Authorization, Content-Type")
	writer.Header().Set("Access-Control-Allow-Methods", "POST, DELETE")
}

func (handler SessionHandler) HandleDelete(writer http.ResponseWriter, request *http.Request) {
	ctx := request.Context()

	sessionInfo, authenticated := handler.AuthenticateBySessionToken(writer, request)
	if !authenticated {
		return
	}

	err := handler.DB.PostgreSQL.DeleteSessionToken(ctx, sessionInfo.SessionToken)
	if err != nil {
		http.Error(writer, "internal server error", http.StatusInternalServerError)
		return
	}
}

func (handler SessionHandler) ServeHTTP(writer http.ResponseWriter, request *http.Request) {
	handler.HandleCORS(writer, request)

	switch request.Method {
	case "DELETE":
		handler.HandleDelete(writer, request)
	case "OPTIONS":
		handler.HandleOptions(writer, request)
	case "POST":
		handler.HandlePost(writer, request)
	default:
		http.Error(writer, "method not allowed", http.StatusMethodNotAllowed)
	}
}
