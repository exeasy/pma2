#!/bin/bash

./sqltest pma.db "create table pma_lsdb(\
	areaid int not null,
	rid int not null,
	nrid int not null,
	ifid int not null,
	nifid int not null,
	metric int,
	status int,
	seq int,
	age text,
	primary key(areaid, rid, nrid, ifid, nifid)
);"
