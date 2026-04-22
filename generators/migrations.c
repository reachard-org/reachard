/*
 * Reachard
 * Copyright 2026 Pavel Sobolev
 *
 * This file is part of the Reachard project, located at
 * <https://reachard.paveloom.dev/>.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#define _DEFAULT_SOURCE

#include <dirent.h>
#include <regex.h>
#include <stdio.h>
#include <stdlib.h>

/// Outputs C code for an array of migrations
///
/// The code is put into the `migrations.h` file in the current working
/// directory. The generated code looks as follows:
///
/// ```c
/// static const char migration_1[] = {
///     #embed "migrations/1.sql" suffix(, 0)
/// };
///
/// static const char migration_2[] = {
///     #embed "migrations/2.sql" suffix(, 0)
/// };
///
/// static const char *migrations[] = {
///     [1] = migration_1,
///     [2] = migration_2
/// };
/// ```
int
main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <migrations_dir>\n", argv[0]);
        return 1;
    }

    int result = 1;

    regex_t regex;
    regmatch_t pmatch[1];

    const char *dir = argv[1];
    DIR *dirp = opendir(dir);

    FILE *filep = fopen("migrations.h", "w");

    const char *pattern = "^([0-9]*)\\.sql$";
    if (regcomp(&regex, pattern, REG_EXTENDED)) {
        fprintf(stderr, "failed to compile regex\n");
        goto cleanup;
    }

    int n_migrations = 0;
    struct dirent *de;
    while ((de = readdir(dirp))) {
        if (de->d_type != DT_REG) {
            continue;
        }

        const char *name = de->d_name;
        if (regexec(&regex, name, 1, pmatch, 0)) {
            continue;
        };

        const int number = strtol(name, 0, 10);

        const char *migration_pattern =
            "static const char migration_%d[] = {\n"
            "#embed \"%s/%s\" suffix(, 0)\n"
            "};\n\n";
        fprintf(filep, migration_pattern, number, dir, name);

        n_migrations++;
    }

    fprintf(filep, "static const char *migrations[] = {\n");
    for (int i = 1; i <= n_migrations; i++) {
        fprintf(filep, "    [%d] = migration_%d,\n", i, i);
    }
    fprintf(filep, "};\n\n");

    fprintf(filep, "const int n_migrations = %d;", n_migrations);

    result = 0;

cleanup:
    regfree(&regex);
    fclose(filep);
    closedir(dirp);
    return result;
}
