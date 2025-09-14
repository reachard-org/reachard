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

package postgresql

import (
	"context"
	"crypto/rand"
	"encoding/base64"
	"errors"

	"github.com/jackc/pgx/v5"
	"golang.org/x/crypto/bcrypt"
)

type UserID = int32

type Credentials struct {
	Username string `json:"username"`
	Password string `json:"password"`
}

func (database Database) AuthenticateByCredentials(
	ctx context.Context,
	credentials Credentials,
) (UserID, error) {
	const sql = "SELECT id, password FROM " + SchemaVersion + ".users WHERE username = $1"
	row := database.Pool.QueryRow(ctx, sql, credentials.Username)

	var userID UserID
	var hashedPassword string
	err := row.Scan(&userID, &hashedPassword)
	if err != nil {
		if errors.Is(err, pgx.ErrNoRows) {
			return -1, errors.Join(ErrUnauthorized{
				"there is no such user",
				"there is no such user",
			}, err)
		}
		return -1, errors.Join(ErrInternalServerError{
			"internal server error",
			"failed to scan the (id, password) pair",
		}, err)
	}

	err = bcrypt.CompareHashAndPassword([]byte(hashedPassword), []byte(credentials.Password))
	if err != nil {
		return -1, errors.Join(ErrUnauthorized{
			"wrong password",
			"wrong password",
		}, err)
	}

	return userID, nil
}

type SessionToken = string

func (database Database) CreateSessionToken(ctx context.Context, userID UserID) (SessionToken, error) {
	byteSessionToken := make([]byte, 40)
	_, _ = rand.Read(byteSessionToken)

	sessionToken := base64.URLEncoding.EncodeToString(byteSessionToken)

	const sql = "INSERT INTO " + SchemaVersion + ".sessions VALUES ($1, $2)"
	_, err := database.Pool.Exec(ctx, sql, sessionToken, userID)
	if err != nil {
		return "", err
	}

	return sessionToken, nil
}
