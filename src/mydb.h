/*
 * Copyright (c) 2019-2022 Nikola Kolev <koue@chaosophia.net>
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

#ifndef _MYDB_H
#define _MYDB_H

#include <cez_queue.h>
#include <mysql/mysql.h>

struct mydb {
	MYSQL	*conn;
	struct	cez_queue config;
	char	error[1024];
	char	querystr[2048];
};

typedef struct mydb mydb;

struct mydb_res {
	MYSQL_RES	*res;
	MYSQL_ROW	row;
	long		numrows;
	long		numfields;
	long		current;
};

typedef struct mydb_res mydb_res;

mydb *mydb_init(void);
void mydb_set_option(mydb *db, const char *name, const char *value);
int mydb_connect(mydb *db);
int mydb_exec(mydb *db, const char *zSql, ...);
mydb_res *mydb_query(mydb *db, const char *zSql);
mydb_res *mydb_prepare(mydb *db, const char *zSql, ...);
char *mydb_text(mydb *db, const char *zDefault, const char *zSql, ...);
int mydb_int(mydb *db, int zDefault, const char *zSql, ...);
int mydb_step(mydb_res *stmt);
int mydb_column_int(mydb_res *stmt, int field);
long mydb_column_long(mydb_res *stmt, int field);
float mydb_column_float(mydb_res *stmt, int field);
double mydb_column_double(mydb_res *stmt, int field);
const char *mydb_column_text(mydb_res *stmt, int field);
void mydb_finalize(mydb_res *stmt);
void mydb_close(mydb *stmt);

#endif

