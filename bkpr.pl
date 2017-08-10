#!/usr/bin/perl
#
## bkpr.pl -- bhyve virtual machine manager

use strict;
use BKPR;
use Getopt::Long qw(:config require_order bundling no_ignore_case );
use Switch;
use Data::Dumper;

my $bkpr = BKPR->new();

my $verb;
GetOptions(
	'f=s'	=> \$bkpr->{'configfile'},
	'n'	=> sub { $verb = 1 },
	'd'	=> sub { $verb = 2 },
	'N'	=> sub { $bkpr->{'noop'} = 1 }
);

unless ($bkpr->parse_config()) {
	$bkpr->err_print();
	exit(1);
}
$bkpr->{'verb'} = $verb if ($verb);

my $op = shift;

if (!defined($op) or ($op eq 'help')) {
	$bkpr->usage();
	exit(0);
}

unless ($bkpr->operation($op)) {
	$bkpr->err_print();
	$bkpr->disconnect();
	exit(1);
}
$bkpr->disconnect();
exit(0);

