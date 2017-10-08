package BKPR::init;

use strict;

sub init {
	my $self = shift;

	## connect to database and create the schema
	$self->debug("creating database schema");
	unless ($self->connect() and $self->db_init()) {
		$self->err_print();
		return 0;
	}

	my $prefix = $self->{'cfg'}->{'prefix'};
	if (! -d $prefix) {
		$self->err("prefix directory does not exist: $prefix");
		return 0;
	}
	$self->debug("creating test directory");
	my $testdir = "$prefix/bkpr_init_test";
	unless (mkdir($testdir)) {
		$self->err("failed to create test directory $testdir: $!");
		return 0;
	}
	$self->debug("removing test directory");
	unless (rmdir($testdir)) {
		$self->err("failed to remove test directory $testdir: $!");
		return 0;
	}

	if ($self->{'cfg'}->{'usezfs'}) {
		my $zprefix = $self->{'cfg'}->{'zprefix'};
		my $zprefixcheck = $self->shell("zfs list $zprefix");
		if ($zprefixcheck->{'return'} == 0) {	## dataset exists
			my $vol = "$zprefix/bkpr_init_test";
			my $ss = "$vol\@foo";
			my $clonevol = "$zprefix/bkpr_init_test_clone";
			## create a volume
			my $create = $self->shell("zfs create -V 128M $vol");
			if ($create->{'return'} != 0) {
				$self->err_set("failed to create test volume $vol: $create->{'out'}");
				return 0;
			}
			## snapshot the volume
			my $snap = $self->shell("zfs snapshot $ss");
			if ($snap->{'return'} != 0) {
				$self->err_set("failed to create snapshot of test volume $ss:  $snap->{'out'}");
				return 0;
			}
			## clone the snapshot
			my $clone = $self->shell("zfs clone $ss $clonevol");
			if ($clone->{'return'} != 0) {
				$self->err_set("failed to create clone $clonevol of $ss: $clone->{'out'}");
				return 0;
			}
			## destroy the clone
			my $destrclone = $self->shell("zfs destroy $clonevol");
			if ($destrclone->{'return'} != 0) {
				$self->err_set("failed to destroy clone $clonevol: $clone->{'out'}");
				return 0;
			}
			## destroy the snapshot
			my $destrsnap = $self->shell("zfs destroy $ss");
			if ($destrsnap->{'return'} != 0) {
				$self->err_set("failed to destroy snapshot $ss: $destrsnap->{'out'}");
				return 0;
			}
			## destroy the volume
			my $destrvol = $self->shell("zfs destroy $vol");
			if ($destrvol->{'return'} != 0) {
				$self->err_set("failed to destroy volume $vol: $destrvol->{'out'}");
				return 0;
			}
		} elsif ($zprefixcheck->{'return'} == 1) { ## dataset does not exist
			$self->err_set("zprefix does not exist -- create it and try again");
			return 0;
		}
	}

	$self->debug("good");
	return 1;
}

1;

