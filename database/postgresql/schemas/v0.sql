-- Copyright 2025 Pavel Sobolev
--
-- This file is part of the Reachard project, located at
--
--     https://reachard.paveloom.dev
--
-- Licensed under the Apache License, Version 2.0 (the "License");
-- you may not use this file except in compliance with the License.
-- You may obtain a copy of the License at
--
--     http://www.apache.org/licenses/LICENSE-2.0
--
-- Unless required by applicable law or agreed to in writing, software
-- distributed under the License is distributed on an "AS IS" BASIS,
-- WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
-- See the License for the specific language governing permissions and
-- limitations under the License.
--
-- SPDX-License-Identifier: Apache-2.0

BEGIN;

CREATE SCHEMA IF NOT EXISTS v0;

CREATE TABLE IF NOT EXISTS v0.users (
    id SERIAL PRIMARY KEY,
    username TEXT NOT NULL CHECK (username <> ''),
    password TEXT NOT NULL CHECK (password <> '')
);

CREATE TABLE IF NOT EXISTS v0.sessions (
    session_token TEXT CHECK (session_token <> '') PRIMARY KEY,
    user_id INTEGER NOT NULL REFERENCES v0.users (id)
);

CREATE TABLE IF NOT EXISTS v0.targets (
    id SERIAL PRIMARY KEY,
    user_id INTEGER NOT NULL REFERENCES v0.users (id),
    name TEXT NOT NULL CHECK (name <> ''),
    url TEXT NOT NULL CHECK (url <> ''),
    interval_seconds INTEGER NOT NULL
);

END;
