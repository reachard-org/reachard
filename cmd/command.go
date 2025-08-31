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

package cmd

import (
	"bufio"
	"flag"
	"fmt"
	"os"
	"strconv"
	"strings"
	"text/template"
)

type Args []string
type Subcommands map[string]*Command

type Command struct {
	Name          string
	Description   string
	Subcommands   Subcommands
	ParentCommand *Command
	RunFunc       func(command *Command, args Args) error
}

func (command *Command) AddSubcommand(subcommand *Command) {
	if command.Subcommands == nil {
		command.Subcommands = make(Subcommands)
	}

	subcommand.ParentCommand = command
	command.Subcommands[subcommand.Name] = subcommand
}

func (command *Command) Run(args Args) {
	err := command.RunFunc(command, args)
	if err != nil {
		fmt.Fprintf(os.Stderr, "%v\n", err)
		os.Exit(1)
	}
}

type commandFormat struct {
	*Command
	FullName             string
	Padding              string
	CommandsFormatString string
}

func (command *Command) Format() commandFormat {
	const paddingInt = 4

	fullName := command.Name
	for currentCommand := command.ParentCommand; currentCommand != nil; {
		fullName = currentCommand.Name + " " + fullName
		currentCommand = currentCommand.ParentCommand
	}

	maxCommandNameLength := 0
	for subcommand, _ := range command.Subcommands {
		maxCommandNameLength = max(maxCommandNameLength, len(subcommand))
	}

	paddingString := strings.Repeat(" ", paddingInt)
	commandsFormatString := "%-" + strconv.Itoa(maxCommandNameLength+paddingInt) + "s"

	return commandFormat{
		FullName:             fullName,
		Command:              command,
		Padding:              paddingString,
		CommandsFormatString: commandsFormatString,
	}
}

func (command *Command) Usage() {
	const usageTemplate = `{{if .Description}}{{.Description}}

{{end}}Usage: {{.FullName}} [OPTIONS]{{if .Subcommands}} <COMMAND>{{end}}{{if .Subcommands}}

Commands:{{range .Subcommands}}
{{$.Padding}}{{printf $.CommandsFormatString .Name}}{{.Description}}{{end}}{{end}}
`

	bw := bufio.NewWriter(os.Stderr)

	template, err := template.New("usage").Parse(usageTemplate)
	if err != nil {
		panic(err)
	}

	err = template.Execute(bw, command.Format())
	if err != nil {
		panic(err)
	}

	bw.Flush()
}

func (command *Command) Parse(args Args) error {
	flagSet := flag.NewFlagSet(command.Name, flag.ContinueOnError)
	flagSet.Usage = command.Usage

	if len(args) < 2 {
		command.Usage()
		return nil
	}

	if subcommand, ok := command.Subcommands[args[1]]; ok {
		subcommand.Run(args[1:])
	} else {
		err := flagSet.Parse(args[1:])
		if err != nil {
			return err
		}
	}

	return nil
}
