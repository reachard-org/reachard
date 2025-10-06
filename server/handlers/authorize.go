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
	"embed"
	_ "embed"
	"io/fs"
	"net/http"

	"reachard/database"
)

type AuthorizeHandler struct {
	Handler
}

func NewAuthorizeHandler(db database.Database) AuthorizeHandler {
	return AuthorizeHandler{Handler{DB: db}}
}

//go:embed authorize/*
var AuthorizePageContent embed.FS

func (handler AuthorizeHandler) HandleGet(writer http.ResponseWriter, request *http.Request) {
	rootFS, err := fs.Sub(AuthorizePageContent, "authorize")
	if err != nil {
		http.Error(writer, "internal server error", http.StatusInternalServerError)
		return
	}

	fileServerHandler := http.StripPrefix("/v0/authorize/", http.FileServerFS(rootFS))
	fileServerHandler.ServeHTTP(writer, request)
}

func (handler AuthorizeHandler) HandleOptions(writer http.ResponseWriter, request *http.Request) {
	writer.Header().Set("Access-Control-Allow-Headers", "Authorization, Content-Type")
	writer.Header().Set("Access-Control-Allow-Methods", "GET")
}

func (handler AuthorizeHandler) ServeHTTP(writer http.ResponseWriter, request *http.Request) {
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
