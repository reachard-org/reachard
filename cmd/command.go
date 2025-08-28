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
	"os"
	"strconv"
	"strings"
	"text/template"
)

type Args []string
type Commands = []*Command
type Subcommands map[string]*Command

type Command struct {
	Name          string
	Description   string
	Subcommands   Subcommands
	ParentCommand *Command
	Run           func(command *Command, args Args)
}

func (parentCommand *Command) SetSubcommands(commands Commands) {
	subcommands := make(Subcommands)
	for _, command := range commands {
		command.ParentCommand = parentCommand
		subcommands[command.Name] = command
	}
	parentCommand.Subcommands = subcommands
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

func (command *Command) Parse(args Args) {
	flagSet := flag.NewFlagSet(command.Name, flag.ExitOnError)
	flagSet.Usage = command.Usage

	if len(args) < 2 {
		command.Usage()
		return
	}

	if subcommand, ok := command.Subcommands[args[1]]; ok {
		subcommand.Run(subcommand, args[1:])
	} else {
		flagSet.Parse(args[1:])
	}
}
