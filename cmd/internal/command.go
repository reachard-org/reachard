// Copyright 2025 Pavel Sobolev
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

package internal

import (
	"bufio"
	"flag"
	"os"
	"text/template"
)

type Args []string
type Commands map[string]*Command

type Command struct {
	Name        string
	Description string
	Commands    Commands
	Run         func(command *Command, args Args)
}

const usageTemplate = `{{if .Description}}{{.Description}}

{{end}}Usage: {{.Name}}{{if .Commands}} <COMMAND>{{end}} [OPTIONS]{{if .Commands}}

Commands:
    {{range .Commands}}{{.Name}}    {{.Description}}{{end}}{{end}}
`

func (command *Command) Usage() {
	bw := bufio.NewWriter(os.Stderr)

	template, err := template.New("usage").Parse(usageTemplate)
	if err != nil {
		panic(err)
	}

	err = template.Execute(bw, command)
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

	if subcommand, ok := command.Commands[args[1]]; ok {
		subcommand.Run(subcommand, args[1:])
	} else {
		flagSet.Parse(args[1:])
	}
}
