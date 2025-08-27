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

package main

import (
	"os"

	"reachard/cmd/internal"
	"reachard/cmd/serve"
)

var Command = internal.Command{
	Name: "reachard",
	Commands: internal.Commands{
		"serve": &serve.Command,
	},
	Run: run,
}

func run(command *internal.Command, args internal.Args) {
	command.Parse(args)
}

func main() {
	run(&Command, os.Args)
}
