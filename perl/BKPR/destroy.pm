package BKPR::destroy;

use strict;
use Switch;
use Getopt::Long qw(:config require_order no_ignore_case );
use Data::Dumper;

sub destroy {
	my $self = shift;
	my $force = $self->{'cfg'}->{'autocreate'};

	my %cmdopts;
	GetOptions(\%cmdopts,
		"i=i",
		"n=s",
		"F"	=> sub { $force = 0 },
		"f"	=> sub { $force = 1 }
	);

	return 0 unless ($self->connect());

	my $vmid;
	if ($cmdopts{'i'}) {
		$vmid = $cmdopts{'i'};
	} else {
		if ($cmdopts{'n'}) {
			$vmid = $self->name_to_vmid($cmdopts{'n'});
			unless (defined($vmid)) {
				$self->err_set("no such guest") unless ($self->err_check());
				return 0;
			}
		} else {
			$self->err_set("must supply a guest by name or id");
			return 0;
		}
	}

	my $guest = $self->fetch_guest($vmid);

	if ($force) {
		$self->warning("resource removal not implemented");
		$self->output("would remove the following disks:");
		$self->output("\t$guest->{'root'}->{'type'}::".$self->disk_relative_path($guest->{'root'}->{'path'}));
		if (defined($guest->{'disks'})) {
			foreach my $disk (@{$guest->{'disks'}}) {
				$self->output("\t$disk->{'type'}::".$self->disk_relative_path($disk->{'path'}));
			}
		}
	} else {
		$self->output("not removing the following disks:");
		$self->output("\t$guest->{'root'}->{'type'}::".$self->disk_relative_path($guest->{'root'}->{'path'}));
		if (defined($guest->{'disks'})) {
			foreach my $disk (@{$guest->{'disks'}}) {
				$self->output("\t$disk->{'type'}::".$self->disk_relative_path($disk->{'path'}));
			}
		}
	}

	return 0 unless ($self->destroy_guest($vmid));
	return 1;
}

1;

