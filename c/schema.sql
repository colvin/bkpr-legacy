-- bkpr schema

DROP TABLE IF EXISTS loaders;
CREATE TABLE loaders (
	loader_id	INTEGER PRIMARY KEY,
	loader_name	VARCHAR(255) NOT NULL
);

INSERT INTO loaders (loader_id,loader_name) VALUES
	(1,'bhyveload'),
	(2,'grub-bhyve'),
	(3,'uefi'),
	(4,'uefi-csm')
;

DROP TABLE IF EXISTS operatingsystem;
CREATE TABLE operatingsystem (
	os_id		INTEGER PRIMARY KEY,
	os_name		VARCHAR(255) NOT NULL
);

INSERT INTO  operatingsystem (os_id,os_name) VALUES
	(1,'freebsd'),
	(2,'openbsd'),
	(3,'netbsd'),
	(4,'linux'),
	(5,'sun'),
	(6,'windows')
;

CREATE TABLE IF NOT EXISTS guest (
	guest_id	INTEGER PRIMARY KEY,
	name		VARCHAR(255) NOT NULL,
	cpu		INTEGER NOT NULL,
	mem		INTEGER NOT NULL,
	os		INTEGER NOT NULL,
	loader		INTEGER NOT NULL,
	descr		VARCHAR(1024),
	grub_map	VARCHAR(1024),
	grub_cmd	VARCHAR(1024),
	FOREIGN KEY(os)
		REFERENCES operatingsystem(os_id),
	FOREIGN KEY(loader)
		REFERENCES loaders(loader_id)
);
