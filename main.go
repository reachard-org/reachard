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
	"reachard/server"
)

func printInfo() {
	bw := bufio.NewWriter(os.Stdout)

	envVars := []string{
		"REACHARD_POSTGRESQL_DB",
		"REACHARD_POSTGRESQL_USER",
		"",
		"REACHARD_CLICKHOUSE_HOST",
		"REACHARD_CLICKHOUSE_PORT",
		"REACHARD_CLICKHOUSE_DB",
		"REACHARD_CLICKHOUSE_USER",
		"",
		"PGHOST",
		"PGPORT",
		"PGDATABASE",
		"PGUSER",
		"",
		"REACHARD_HOST",
		"REACHARD_PORT",
	}

	for _, envVar := range envVars {
		if envVar == "" {
			fmt.Fprint(bw, "\n")
			continue
		}
		fmt.Fprintf(bw, "%s: %s\n", envVar, os.Getenv(envVar))
	}

	bw.WriteByte('\n')
	bw.Flush()
}

func dbPingRun(command *cmd.Command, args cmd.Args) error {
	printInfo()

	ctx := context.Background()

	db, err := database.Connect(ctx, "")
	if err != nil {
		return fmt.Errorf("Couldn't connect to the database: %v", err)
	}
	defer db.Close()

	println("Successfully pinged the databases!")

	return nil
}

func dbPrepareRun(command *cmd.Command, args cmd.Args) error {
	printInfo()

	ctx := context.Background()

	db, err := database.Connect(ctx, "")
	if err != nil {
		return fmt.Errorf("Couldn't connect to a database: %v", err)
	}
	defer db.Close()

	err = db.ExecSchemas(ctx)
	if err != nil {
		return fmt.Errorf("Failed to execute a schema: %v", err)
	}

	println("Successfully prepared the databases!")

	return nil
}

func dbServeRun(command *cmd.Command, args cmd.Args) error {
	printInfo()

	port := os.Getenv("REACHARD_PORT")
	if port == "" {
		port = "7272"
	}

	addr := os.Getenv("REACHARD_HOST") + ":" + port

	server, err := server.NewServer()
	if err != nil {
		return fmt.Errorf("Failed to create a server: %v", err)
	}
	defer server.Cleanup()

	println("Serving at", addr)

	err = server.ListenAndServe(addr)
	if err != nil {
		return fmt.Errorf("Failed to serve: %v", err)
	}

	return nil
}

func run(command *cmd.Command, args cmd.Args) error {
	err := command.Parse(args)
	if err != nil {
		return err
	}

	return nil
}

func main() {
	dbPingCommand := cmd.Command{
		Name:        "ping",
		Description: "Ping the databases",
		RunFunc:     dbPingRun,
	}

	dbPrepareCommand := cmd.Command{
		Name:        "prepare",
		Description: "Prepare the database schemas",
		RunFunc:     dbPrepareRun,
	}

	dbCommand := cmd.Command{
		Name:        "db",
		Description: "Operate on the databases",
		RunFunc:     run,
	}

	dbCommand.AddSubcommand(&dbPingCommand)
	dbCommand.AddSubcommand(&dbPrepareCommand)

	serveCommand := cmd.Command{
		Name:        "serve",
		Description: "Start the server",
		RunFunc:     dbServeRun,
	}

	mainCommand := cmd.Command{
		Name:    "reachard",
		RunFunc: run,
	}

	mainCommand.AddSubcommand(&dbCommand)
	mainCommand.AddSubcommand(&serveCommand)

	mainCommand.Run(os.Args)
}
