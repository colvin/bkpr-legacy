# bkpr
perl frontend for bhyve

## Dependencies
`p5-DBI`

`p5-DBD-mysql`

`p5-DBD-SQLite`

`p5-Switch`

## About
`bkpr` is a frontend for `bhyve` I started putting together before there were other good frontends out there.

It's not finished, and in some cases functionality is not even there. For instance, the `update` subcommand
does not actually do anything. Work in progress. There are almost certainly bugs.

There aren't significant advantages to `bkpr` over other tools like `iohyve`, but I use it for all my personal
use, and find it pretty effective. Your mileage may vary.

## SYNOPSIS

`bkpr [<global-options>] <subcommand> [<command-options>]`

## Description

`bkpr` stores configuration data for `bhyve` guests in a database and streamlines the process of managing and running guests.

It currently supports the following database backends:
- MySQL
- SQLite

See the man page in `bkpr.1` for more detailed information.

