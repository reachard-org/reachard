# Copyright 2025 Pavel Sobolev
#
# This file is part of the Reachard project, located at
#
#     https://reachard.paveloom.dev
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
# SPDX-License-Identifier: Apache-2.0

FROM docker.io/golang:1-alpine AS builder

WORKDIR /build/src

RUN --mount=type=cache,target=/etc/apk/cache \
    apk add --update-cache gcc musl-dev

RUN --mount=type=cache,target=/go/pkg/mod/ \
    --mount=type=cache,target="/root/.cache/go-build" \
    --mount=type=bind,target=. \
    CGO_ENABLED=1 go build -v -ldflags "-s -w" -trimpath -o /build/reachard

FROM docker.io/alpine:3

RUN set -eux; \
    addgroup -g 120 -S reachard; \
    adduser -u 120 -S -D -G reachard -h /var/lib/reachard -s /sbin/nologin reachard

COPY --from=builder --chown=reachard:reachard /build/reachard /usr/local/bin/reachard

USER reachard

WORKDIR /var/lib/reachard

CMD ["/usr/local/bin/reachard", "serve"]
