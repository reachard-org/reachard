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
	"errors"
	"net/http"
	"strings"

	"reachard/database"
	"reachard/database/postgresql"
)

type Handler struct {
	DB database.Database
}

func (handler Handler) HandleCORS(writer http.ResponseWriter, request *http.Request) {
	origin := request.Header.Get("Origin")
	if origin != "" {
		writer.Header().Set("Access-Control-Allow-Origin", request.Header.Get("Origin"))
	}
	writer.Header().Set("Vary", "Origin")
}

type ErrHTTP struct {
	error string
	code  int
}

func (err ErrHTTP) Error() string {
	return err.error
}

func (handler Handler) AuthenticateBySessionToken(
	ctx context.Context,
	request *http.Request,
) (postgresql.SessionToken, error) {
	authorizationHeader := request.Header.Get("Authorization")
	if authorizationHeader == "" {
		return "", ErrHTTP{"missing the Authorization header", http.StatusUnauthorized}
	}

	authorizationHeaderParts := strings.Split(authorizationHeader, " ")
	if len(authorizationHeaderParts) != 2 || authorizationHeaderParts[0] != "Bearer" {
		return "", ErrHTTP{"couldn't parse the Authorization header", http.StatusUnauthorized}
	}

	sessionToken := authorizationHeaderParts[1]
	_, err := handler.DB.PostgreSQL.AuthenticateBySessionToken(ctx, sessionToken)
	if err != nil {
		var errInternalServerError postgresql.ErrInternalServerError
		var errUnauthorized postgresql.ErrUnauthorized
		switch {
		case errors.As(err, &errInternalServerError):
			return "", ErrHTTP{errInternalServerError.UserMsg, http.StatusInternalServerError}
		case errors.As(err, &errUnauthorized):
			return "", ErrHTTP{errUnauthorized.UserMsg, http.StatusUnauthorized}
		}
	}

	return sessionToken, nil
}
