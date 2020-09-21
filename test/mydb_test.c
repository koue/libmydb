/*
 * Copyright (c) 2019-2020 Nikola Kolev <koue@chaosophia.net>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *    - Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *    - Redistributions in binary form must reproduce the above
 *      copyright notice, this list of conditions and the following
 *      disclaimer in the documentation and/or other materials provided
 *      with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mydb.h"
#include "cez_test.h"

int
main(void)
{
	mydb *db;
	mydb_res *stmt;
	char *result;

	cez_test_start();
	assert((db = mydb_init()) != NULL);
	mydb_set_option(db, "hostname", "127.0.0.1");
	mydb_set_option(db, "username", "dbuser");
	mydb_set_option(db, "password", "dbpass");
	mydb_set_option(db, "database", "dbname");
	assert(mydb_connect(db) != -1);
	assert(mydb_exec(db, "INSERT INTO table (name) VALUES ('koue')") == 0);
	assert((stmt = mydb_query(db, "SELECT id FROM table")) != NULL);
	assert(mydb_step(stmt) == 1);
	assert(mydb_column_int(stmt, 0) == 1);
	mydb_finalize(stmt);
	assert((stmt = mydb_query(db, "SELECT id, name FROM table")) != NULL);
	assert(mydb_step(stmt) == 1);
	assert(strcmp(mydb_column_text(stmt, 1), "koue") == 0);
	mydb_finalize(stmt);
	assert(result = mydb_text(db, "unknown", "SELECT name FROM table WHERE id = 1"));
	assert(strcmp(result, "koue") == 0);
	free(result);
	assert(result = mydb_text(db, "unknown", "SELECT id, name FROM table WHERE id = 1"));
	assert(strcmp(result, "unknown") == 0);
	free(result);
	assert(result = mydb_text(db, "unknown", "SELECT name FROM table WHERE id = 123"));
	assert(strcmp(result, "unknown") == 0);
	free(result);
	assert(mydb_int(db, 0, "SELECT id FROM table WHERE name = 'koue'") == 1);
	assert(mydb_int(db, 0, "SELECT id FROM table WHERE name = 'unknown'") == 0);
	mydb_close(db);

	return (0);
}
