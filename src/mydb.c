/*
 * Copyright (c) 2019-2021 Nikola Kolev <koue@chaosophia.net>
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

#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fslbase.h>
#include <cez_queue.h>
#include <mydb.h>

mydb *
mydb_init(void)
{
	mydb *db;

	if ((db = calloc(1, sizeof(*db))) == NULL) {
		fprintf(stderr, "[ERROR] %s: %s\n", __func__, strerror(errno));
		exit(1);
	}
	cez_queue_init(&db->config);
	if ((db->conn = mysql_init(NULL)) == NULL) {
		return (NULL);
	}
	return (db);
}

void
mydb_set_option(mydb *db, const char *name, const char *value)
{
	if (db == NULL || name == NULL || value == NULL) {
		return;
	}
	if (cqa(&db->config, name, value) == -1) {
		mydb_close(db);
		printf("%s: fail\n", __func__);
		exit(1);
	}
}

int
mydb_connect(mydb *db)
{
	const char *name;
	const char *mydb_options_list[] = { "hostname", "username",
	    "password", "database", NULL };

	if (db == NULL) {
		return (-1);
	}
	if ((name = cqc(&db->config, mydb_options_list)) != NULL) {
		snprintf(db->error, sizeof(db->error),
		    "Missing option: %s", name);
		return (-1);
	}
	if (mysql_real_connect(db->conn, cqg(&db->config, "hostname"),
	    cqg(&db->config, "username"), cqg(&db->config, "password"),
	    cqg(&db->config, "database"), 0, NULL, 0) == NULL) {
		snprintf(db->error, sizeof(db->error), "%s",
		    mysql_error(db->conn));
		return (-1);
	}
	return (0);
}

int
mydb_exec(mydb *db, const char *zSql, ...)
{
	Blob sql;
	va_list ap;
	const char *zQuery;
	int ret;

	if (db == NULL || zSql == NULL) {
		return (-1);
	}
	blob_init(&sql, 0 , 0);
	va_start(ap, zSql);
	blob_vappendf(&sql, zSql, ap);
	va_end(ap);
	zQuery = blob_str(&sql);
	if ((ret = mysql_query(db->conn, zQuery)) != 0) {
		snprintf(db->error, sizeof(db->error),
		    "%s: %s", __func__, mysql_error(db->conn));
	}
	blob_reset(&sql);
	return (ret);
}

mydb_res *
mydb_query(mydb *db, const char *zSql)
{
	mydb_res *Stmt;

	if (db == NULL || zSql == NULL) {
		return (NULL);
	}
	if ((Stmt = calloc(1, sizeof(*Stmt))) == NULL) {
		mydb_close(db);
		fprintf(stderr, "[ERROR] %s: %s\n", __func__, strerror(errno));
		exit(1);
	}
	if (mysql_query(db->conn, zSql) != 0) {
		snprintf(db->error, sizeof(db->error),
		    "%s: %s", __func__, mysql_error(db->conn));
		return (NULL);
	}
	Stmt->res = mysql_store_result(db->conn);
	if ((Stmt->res == NULL) && mysql_errno(db->conn)) {
		return (NULL);
	}
	if (Stmt->res) {
		Stmt->numrows = mysql_num_rows(Stmt->res);
		Stmt->numfields = mysql_num_fields(Stmt->res);
	} else {
		Stmt->numfields = 0;
		Stmt->numrows = 0;
	}
	Stmt->current = 0;
	return (Stmt);
}

mydb_res *
mydb_prepare(mydb *db, const char *zSql, ...)
{
	Blob sql;
	mydb_res *Stmt;
	va_list ap;
	const char *zQuery;

	if (db == NULL || zSql == NULL) {
		return (NULL);
	}
	blob_init(&sql, 0 , 0);
	va_start(ap, zSql);
	blob_vappendf(&sql, zSql, ap);
	va_end(ap);
	zQuery = blob_str(&sql);
	Stmt = mydb_query(db, zQuery);
	blob_reset(&sql);
	return(Stmt);
}

/*
 * free() the result in the calling function
 */
char *
mydb_text(mydb *db, const char *zDefault, const char *zSql, ...)
{
	Blob sql;
	mydb_res *Stmt;
	va_list ap;
	const char *zQuery;
	char *zResult;

	if (db == NULL || zDefault == NULL || zSql == NULL) {
		return (NULL);
	}
	blob_init(&sql, 0 , 0);
	va_start(ap, zSql);
	blob_vappendf(&sql, zSql, ap);
	va_end(ap);
	zQuery = blob_str(&sql);
	Stmt = mydb_query(db, zQuery);
	blob_reset(&sql);
	if ((Stmt != NULL) && (Stmt->numrows == 1) && (Stmt->numfields == 1)
	    && (mydb_step(Stmt) == 1)) {
		zResult = strdup(mydb_column_text(Stmt, 0));
	} else {
		zResult = strdup(zDefault);
	}
	mydb_finalize(Stmt);
	return (zResult);
}

int
mydb_int(mydb *db, int zDefault, const char *zSql, ...)
{
	Blob sql;
	mydb_res *Stmt;
	va_list ap;
	const char *zQuery;
	int zResult;

	if (db == NULL || zSql == NULL) {
		return (-1);
	}
	blob_init(&sql, 0 , 0);
	va_start(ap, zSql);
	blob_vappendf(&sql, zSql, ap);
	va_end(ap);
	zQuery = blob_str(&sql);
	Stmt = mydb_query(db, zQuery);
	blob_reset(&sql);
	if ((Stmt != NULL) && (Stmt->numrows == 1) && (Stmt->numfields == 1)
	    && (mydb_step(Stmt) == 1)) {
		zResult = mydb_column_int(Stmt, 0);
		mydb_finalize(Stmt);
		return (zResult);
	} else {
		mydb_finalize(Stmt);
		return (zDefault);
	}
}

int
mydb_step(mydb_res *stmt)
{
	if (stmt == NULL) {
		return (-1);
	}
	if ((stmt->res == NULL) || (stmt->current == stmt->numrows)) {
		return (0);
	}
	stmt->current++;
	stmt->row = mysql_fetch_row(stmt->res);
	return (1);
}

int
mydb_column_int(mydb_res *stmt, int field)
{
	if (stmt == NULL) {
		return (-1);
	}
	if (field >= stmt->numfields) {
		return (0);
	}
	return ((int)strtol(stmt->row[field], NULL, 10));
}

long
mydb_column_long(mydb_res *stmt, int field)
{
	if (stmt == NULL) {
		return (-1);
	}
	if (field >= stmt->numfields) {
		return (0);
	}
	return (strtol(stmt->row[field], NULL, 10));
}

float
mydb_column_float(mydb_res *stmt, int field)
{
	if (stmt == NULL) {
		return (-1);
	}
	if (field >= stmt->numfields) {
		return (0);
	}
	return (strtof(stmt->row[field], NULL));
}

double
mydb_column_double(mydb_res *stmt, int field)
{
	if (stmt == NULL) {
		return (-1);
	}
	if (field >= stmt->numfields) {
		return (0);
	}
	return (strtod(stmt->row[field], NULL));
}

const char *
mydb_column_text(mydb_res *stmt, int field)
{
	if (stmt == NULL) {
		return (NULL);
	}
	if (field >= stmt->numfields) {
		return (NULL);
	}
	return (stmt->row[field]);
}

void
mydb_finalize(mydb_res *stmt)
{
	if (stmt == NULL) {
		return;
	}
	mysql_free_result(stmt->res);
	free(stmt);
}

void
mydb_close(mydb *db)
{
	if (db == NULL) {
		return;
	}
	if (db->conn != NULL) {
		mysql_close(db->conn);
	}
	cez_queue_purge(&db->config);
	free(db);
}
