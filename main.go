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

package main

import (
	"bufio"
	"context"
	"fmt"
	"os"

	"reachard/cmd"
	"reachard/database"
)

func dbPingRun(command *cmd.Command, args cmd.Args) {
	bw := bufio.NewWriter(os.Stdout)

	envVars := []string{
		"PGHOST", "PGPORT", "PGDATABASE", "PGUSER",
	}

	for _, envVar := range envVars {
		fmt.Fprintf(bw, "%s: %s\n", envVar, os.Getenv(envVar))
	}

	bw.WriteByte('\n')
	bw.Flush()

	db, err := database.Connect(context.Background(), "")
	if err != nil {
		fmt.Fprintf(os.Stderr, "Couldn't connect to the database: %v\n", err)
		os.Exit(1)
	}
	defer db.Close()

	println("Successfully pinged the database!")
}

func run(command *cmd.Command, args cmd.Args) {
	command.Parse(args)
}

func main() {
	dbPingCommand := cmd.Command{
		Name:        "ping",
		Description: "Ping the database",
		RunFunc:     dbPingRun,
	}

	dbCommand := cmd.Command{
		Name:        "db",
		Description: "Operate on the database",
		RunFunc:     run,
	}

	dbCommand.AddSubcommand(&dbPingCommand)

	serveCommand := cmd.Command{
		Name:        "serve",
		Description: "Start the server",
		RunFunc:     run,
	}

	mainCommand := cmd.Command{
		Name:    "reachard",
		RunFunc: run,
	}

	mainCommand.AddSubcommand(&dbCommand)
	mainCommand.AddSubcommand(&serveCommand)

	mainCommand.Run(os.Args)
}
