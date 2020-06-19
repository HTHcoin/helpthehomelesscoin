--
-- File generated with SQLiteStudio v3.0.7 on Sun Mar 20 17:59:39 2016
--
-- Text encoding used: windows-1252
--
PRAGMA foreign_keys = off;
BEGIN TRANSACTION;

-- Table: sys_users
CREATE TABLE sys_users (id INTEGER PRIMARY KEY AUTOINCREMENT, username TEXT NOT NULL UNIQUE, passwd TEXT NOT NULL, fname TEXT NOT NULL, mname TEXT, lname TEXT NOT NULL, rank INTEGER DEFAULT (1), email TEXT NOT NULL UNIQUE, UNIQUE (username))
INSERT INTO sys_users (id, username, passwd, fname, mname, lname, rank, email) VALUES (1, 'admin', 'pass', 'Root', NULL, 'Administrator', -1, 'admin@root.org');

COMMIT TRANSACTION;
PRAGMA foreign_keys = on;
