package BKPR::update;

use strict;
use Switch;
use Getopt::Long qw(:config require_order no_ignore_case );
use Data::Dumper;

sub update {
	my $self = shift;
	my $guest;

	unless ($self->connect()) {
		return 0;
	}

	my %cmdopts;
	GetOptions(\%cmdopts,
		"i=i",
		"n=s",
		"c=i",
		"m=i",
		"r=s",
		"d=s@",
		"N=s@",
		"o=s",
		"l=s",
		"G:s",
		"M:s",
		"D=s"
	);

	## if we get -i, we operate on that index
	## if we get -n, we operate on that name
	## if we get both -i and -n, then:
	##   if <index> is held by a guest, then that guest is renamed <name>
	##   if <index> is not held by a guest but <name> is, we reindex <name> to <index>
	##   if neither <index> nor <name> are held by a guest, then error
	my $index;
	if ($cmdopts{'i'} and $cmdopts{'n'}) {
		if ($self->check_vmid($cmdopts{'i'})) {		##XXX need to validate these inputs
			$index = $cmdopts{'i'};
		} elsif ($self->check_name($cmdopts{'n'})) {	## especially this one
			$index = $self->name_to_vmid($cmdopts{'n'});
		} else {
			$self->err_set("no such vm");
			return 0;
		}
	} elsif ($cmdopts{'i'}) {
		$index = $cmdopts{'i'};
	} elsif ($cmdopts{'n'}) {
		$index = $self->name_to_vmid($cmdopts{'n'});
	}
	$self->debug("operating on vm with id: $index");

	$guest = $self->fetch_guest($index);
	return 0 unless ($guest);

	##XXX
	$self->debug("PRE:");
	$self->print_guest_list_header();
	$self->print_guest_list($guest);
	$self->print_guest_list_header();
	$self->output("warning: this operation is experimental!");
	##XXX

	$guest->{'name'}	= $cmdopts{'n'} if ($cmdopts{'n'});
	$guest->{'vmid'}	= $cmdopts{'i'} if ($cmdopts{'i'});
	$guest->{'cpu'}		= $cmdopts{'c'} if ($cmdopts{'c'});
	$guest->{'mem'}		= $cmdopts{'m'} if ($cmdopts{'m'});
	$guest->{'os'}		= $cmdopts{'o'} if ($cmdopts{'o'});
	$guest->{'loader'}	= $cmdopts{'l'} if ($cmdopts{'l'});
	$guest->{'descr'}	= $cmdopts{'D'} if ($cmdopts{'D'});

	if (defined($cmdopts{'M'})) {
		if ($cmdopts{'M'} eq '') {
			$guest->{'grubmap'} = undef;
		} else {
			$guest->{'grubmap'} = $cmdopts{'M'}
		}
	}
	if (defined($cmdopts{'G'})) {
		if ($cmdopts{'G'} eq '') {
			$guest->{'grubcmd'} = undef;
		} else {
			$guest->{'grubcmd'} = $cmdopts{'G'}
		}
	}

	my @rmdisks;
	my $diskid = scalar @{$guest->{'disks'}};
	foreach my $dreq (@{$cmdopts{'d'}}) {
		my $mode = 1;	## we add disks that have no modifier
		if ($dreq =~ /^-/) {
			$mode = 0;
			$dreq =~ s/^.//;
		}
		my $disk = $self->disk_parse_spec($dreq);
		return 0 unless ($disk);
		$disk = $self->disk_cannonicalize_path($disk,$guest->{'name'},++$diskid);
		if ($mode) {
			push(@{$guest->{'disks'}},$disk);
		} else {
			push(@rmdisks,$disk);
		}
	}

	## process disk removals
	my @finaldisks;
	foreach my $disk (@{$guest->{'disks'}}) {
		my $keep = 1;
		foreach my $rmdisk (@rmdisks) {
			if ($disk->{'path'} eq $rmdisk->{'path'}) {
				$keep = 0;
			}
		}
		push(@finaldisks,$disk) if ($keep);
	}
	$guest->{'disks'} = \@finaldisks;

	## TODO add/remove nics
	if ($cmdopts{'N'}) {
		$self->warning("adding/removing nics not yet supported");
	}

	##XXX
	$self->debug("POST:");
	$self->print_guest_list_header();
	$self->print_guest_list($guest);
	$self->print_guest_list_header();
	##XXX

	unless ($self->{'noop'}) {
		return $self->update_guest($index,$guest);
	}

	return 1;
}

1;

