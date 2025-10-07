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

class View {
  constructor(refName, title) {
    this.ref = document.getElementById(`${refName}-ref`);
    this.title = `${title} | Reachard`;
  }

  async init() {}

  set() {
    this.ref.checked = true;
    document.title = this.title;
  }
}

class AuthenticationView extends View {
  constructor() {
    super("authentication", "Authentication");
  }

  async init() {
    const authenticationForm = document.getElementById("authentication-form");
    authenticationForm.addEventListener("submit", (event) => this.logIn(event));
  }

  async logIn(event) {
    event.preventDefault();

    const form = event.target;

    const object = {
      username: form.username.value,
      password: form.password.value,
    };
    const json = JSON.stringify(object);

    await fetch("/v0/session/", {
      method: "POST",
      headers: {
        "Content-Type": "application/json",
      },
      body: json,
    });
  }
}

class AuthorizationView extends View {
  constructor() {
    super("authorization", "Authorization");
  }
}

class ViewManager {
  views = {
    authenticate: new AuthenticationView(),
    authorization: new AuthorizationView(),
  };

  async init() {
    for (const view of Object.values(this.views)) {
      view.init();
    }

    this.views.authenticate.set();
  }
}

new ViewManager().init();
