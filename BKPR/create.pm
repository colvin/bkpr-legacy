package BKPR::create;

use strict;
use Switch;
use Getopt::Long qw(:config require_order no_ignore_case );
use Data::Dumper;

sub create {
	my $self = shift;
	my $guest;
	my $force = $self->{'cfg'}->{'autocreate'};

	unless ($self->connect()) {
		return 0;
	}

	## get commandline specification
	my %cmdopts;
	GetOptions(\%cmdopts,
		"f" => sub { $force = 1 },
		"F" => sub { $force = 0 },
		"i=i",
		"n=s",
		"c=i",
		"m=i",
		"r=s",
		"d=s@",
		"N=s@",
		"o=s",
		"l=s",
		"G=s",
		"M=s",
		"D=s"
	);

	$guest->{'cpu'} = 1;
	$guest->{'mem'} = 1024;
	$guest->{'os'} = 'freebsd';
	$guest->{'loader'} = 'bhyveload';

	$guest->{'vmid'} = $cmdopts{'i'} if ($cmdopts{'i'});
	$guest->{'name'} = $cmdopts{'n'} if ($cmdopts{'n'});
	$guest->{'cpu'} = $cmdopts{'c'} if ($cmdopts{'c'});
	$guest->{'mem'} = $cmdopts{'m'} if ($cmdopts{'m'});
	$guest->{'os'} = $cmdopts{'o'} if ($cmdopts{'o'});
	$guest->{'loader'} = $cmdopts{'l'} if ($cmdopts{'l'});
	$guest->{'descr'} = $cmdopts{'D'} if ($cmdopts{'D'});

	$guest->{'grubmap'} = $cmdopts{'M'} if ($cmdopts{'M'});
	$guest->{'grubcmd'} = $cmdopts{'G'} if ($cmdopts{'G'});

	unless ($guest->{'name'}) {
		$self->err_set("guest must have a name");
		return 0;
	}

	if ($self->check_name($guest->{'name'})) {
		$self->err_set("name already in use");
		return 0;
	}

	if (defined($cmdopts{'i'})) {
		unless ($cmdopts{'i'} =~ /^\d+$/) {
			$self->err_set("invalid index value");
			return 0;
		}
		$guest->{'vmid'} = $cmdopts{'i'};
	} else {
		$guest->{'vmid'} = $self->alloc_vmid();
	}

	if ($self->check_vmid($guest->{'vmid'})) {
		$self->err_set("index already in use");
		return 0;
	}

	unless ($self->validate_loader($guest->{'loader'})) {
		$self->err_set("invalid loader: $guest->{'loader'}");
		return 0;
	}

	if ($guest->{'grubmap'}) {
		$guest->{'grubmap'} = $self->file_cannonicalize_path($guest->{'grubmap'},$guest->{'name'});
		return 0 unless ($guest->{'grubmap'});
	}
	if ($guest->{'grubcmd'}) {
		$guest->{'grubcmd'} = $self->file_cannonicalize_path($guest->{'grubcmd'},$guest->{'name'});
		return 0 unless ($guest->{'grubcmd'});
	}

	if ($cmdopts{'r'}) {
		my $r = $self->disk_parse_spec($cmdopts{'r'},1);
		return 0 unless ($r);
		$r = $self->disk_cannonicalize_path($r,$guest->{'name'});
		$guest->{'root'} = $r;
	}

	unless (defined($guest->{'root'})) {
		$self->err_set("must have a root disk");
		return 0;
	}

	my @d;
	my $diskid = 0;
	foreach my $d (@{$cmdopts{'d'}}) {
		my $disk = $self->disk_parse_spec($d);
		return 0 unless $disk;
		$disk = $self->disk_cannonicalize_path($disk,$guest->{'name'},$diskid);
		push(@d,$disk);
		$diskid++;
	}
	$guest->{'disks'} = \@d;

	my @N;
	$self->{'alloc_tapid_increment'} = 0;
	push(@{$cmdopts{'N'}},'auto') unless ($cmdopts{'N'});
	foreach my $net (@{$cmdopts{'N'}}) {
		$net = $self->net_parse_spec($net);
		return 0 unless $net;
		push(@N,$net);
	}
	$guest->{'nics'} = \@N;

	if ($self->{'noop'} or $self->{'verb'}) {
		$self->print_guest_list_header();
		$self->print_guest_list($guest);
		$self->print_guest_list_header();
	}

	if ($force) {
		## create the guest directory
		my $guestdir = $self->{'cfg'}->{'prefix'}."/".$guest->{'name'};
		unless (-d $guestdir) {
			$self->debug("creating directory $guestdir");
			unless (mkdir($guestdir)) {
				$self->err_set($@);
				return 0;
			}
		}
		## create volumes
		my @alldisks = @d;
		unshift(@alldisks,$guest->{'root'});
		foreach my $disk (@alldisks) {
			if ($disk->{'type'} eq 'zvol') {
				my $relpath = $self->disk_relative_path($disk->{'path'});
				unless ($self->zfs_check_dataset($relpath)) {
					$self->debug("creating dataset: $relpath size $disk->{'size'}");
					unless ($self->zfs_create_dataset($relpath,$disk->{'size'})) {
						return 0;
					}
				} else {
					$self->debug("dataset exists: $relpath");
				}
			} elsif ($disk->{'type'} eq 'file') {
				unless (-e $disk->{'path'}) {
					$self->debug("creating file: $disk->{'path'} size $disk->{'size'}");
					unless ($self->file_create_img($disk->{'path'},$disk->{'size'})) {
						return 0;
					}
				} else {
					$self->debug("file exists: $disk->{'path'}");
				}
			}
		}
	}

	return 1 if ($self->{'noop'});

	unless ($self->insert_guest($guest)) {
		return 0;
	}

	return 1;
}

1;

