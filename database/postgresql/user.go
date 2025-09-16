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
	"fmt"

	"golang.org/x/crypto/bcrypt"
)

type UserID = int32

func (database Database) GeneratePlainPassword() string {
	buffer := make([]byte, 40)
	rand.Read(buffer)
	return base64.URLEncoding.EncodeToString(buffer)
}

func (database Database) HashPassword(password string) (string, error) {
	hashedPassword, err := bcrypt.GenerateFromPassword([]byte(password), bcrypt.DefaultCost)
	if err != nil {
		return "", err
	}
	return string(hashedPassword), nil
}

func (database Database) AddNewUser(ctx context.Context, username string, password string) error {
	fmt.Printf("username: %s\n", username)

	if password == "" {
		password = database.GeneratePlainPassword()
	}
	fmt.Printf("password: %s\n", password)

	hashedPassword, err := database.HashPassword(password)
	if err != nil {
		return err
	}
	fmt.Printf("hashed password: %s\n", hashedPassword)

	const sql = "INSERT INTO " + SchemaVersion + ".users (username, hashed_password) VALUES ($1, $2)"
	_, err = database.Pool.Exec(ctx, sql, username, hashedPassword)
	if err != nil {
		return err
	}

	return nil
}
