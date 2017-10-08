package BKPR::start;

use strict;
use Switch;
use POSIX;
use Getopt::Long qw(:config require_order no_ignore_case );
use Data::Dumper;

our $bhyve_default_flags = "-A -H -I -w";

our $bhyve_uefi_prefix = "/usr/local/share/uefi-firmware";
our $bhyve_uefi_standard = join('/',$bhyve_uefi_prefix,'BHYVE_UEFI.fd');
our $bhyve_uefi_csm = join('/',$bhyve_uefi_prefix,'BHYVE_UEFI_CSM.fd');

sub start {
	my $self = shift;

	unless ($self->connect()) {
		return 0;
	}

	my %cmdopts;
	GetOptions(\%cmdopts,
		"i=i",		## index
		"n=s",		## name
		"c=i",		## cpus
		"m=i",		## memory
		"C=s",		## console
		"d=s@",		## disk
		"N=s@",		## netif
		"l=s",		## loader
		"G=s",		## grub command file
		"M=s",		## grub device map
		"I=s",		## install disc
		"B=s",		## bhyve option flags
		"V:s",		## VNC specification
		"u:s",		## standard UEFI specification
		"U:s"		## custom UEFI specification
	);

	my $vmid;
	unless ($cmdopts{'i'} or $cmdopts{'n'}) {
		$self->err_set("must supply one of -i <index>, -n <name>");
		return 0;
	}
	## prefer the index if both index and name are given
	if ($cmdopts{'i'}) {
		$vmid = $cmdopts{'i'};
	} else {
		$vmid = $self->name_to_vmid($cmdopts{'n'});
	}
	unless ($vmid and $self->check_vmid($vmid)) {
		$self->err_set("no such vm");
		return 0;
	}

	my $guest = $self->fetch_guest($vmid);
	unless ($guest) {
		return 0;
	}

	$guest->{'cpu'} = $cmdopts{'c'} if ($cmdopts{'c'});
	$guest->{'mem'} = $cmdopts{'m'} if ($cmdopts{'m'});

	my $numdisks = @{$guest->{'disks'}};
	foreach my $d (@{$cmdopts{'d'}}) {
		my $disk = $self->disk_parse_spec($d);
		return 0 unless $disk;
		$disk = $self->disk_cannonicalize_path($disk,$guest->{'name'},++$numdisks);
		push(@{$guest->{'disks'}},$disk);
	}

	foreach my $n (@{$cmdopts{'N'}}) {
		$n = $self->net_parse_spec($n);
		return 0 unless $n;
		push(@{$guest->{'nics'}},$n);
	}

	if ($cmdopts{'C'}) {
		if ($cmdopts{'C'} =~ /^nmdm/) {
			$guest->{'com'} = "/dev/$cmdopts{'C'}";
		} else {
			$guest->{'com'} = $cmdopts{'C'};
		}
	} else {
		$guest->{'com'} = 'stdio';
	}


	$guest->{'grubmap'} = $cmdopts{'M'} if ($cmdopts{'M'});
	$guest->{'grubcmd'} = $cmdopts{'G'} if ($cmdopts{'G'});

	$guest->{'install'} = $cmdopts{'I'} if ($cmdopts{'I'});

	my $loadcmd;
	if ($cmdopts{'l'}) {
		$guest->{'loader'} = $cmdopts{'l'};
		unless ($self->validate_loader($guest->{'loader'})) {
			$self->err_set("invalid loader: $guest->{'loader'}");
			return 0;
		}
	}

	## no -V disables vnc
	## -V enables vnc with default options
	## -V <string> enables vnc with options <string>
	if (defined($cmdopts{'V'})) {
		$guest->{'vnc'} = "tcp=127.0.0.1:5900,w=1024,h=768,wait";
		$guest->{'vnc'} = $cmdopts{'V'} if ($cmdopts{'V'} ne '');
	}

	if ($guest->{'loader'} eq 'bhyveload') {
		$loadcmd = loader_bhyveload($self,$guest);
	} elsif ($guest->{'loader'} eq 'grub') {
		$loadcmd = loader_grub($self,$guest);
	} elsif ($guest->{'loader'} =~ /^uefi/i) {
		if ($guest->{'loader'} =~ /csm/i) {
			$guest->{'uefi'} = $bhyve_uefi_csm;
		} else {
			$guest->{'uefi'} = $bhyve_uefi_standard;
		}
		if ($cmdopts{'U'}) {
			$guest->{'uefi'} = $cmdopts{'U'};
		}
	}
	unless ($guest->{'loader'} =~ /uefi/i) {
		unless ($loadcmd) {
			return 0;
		}
	}

	if ($cmdopts{'B'}) {
		$bhyve_default_flags = $cmdopts{'B'};
	}

	my $bhyvecmd = bhyvecmd($self,$guest);
	unless ($bhyvecmd) {
		return 0;
	}

	my $destroyvmm = "bhyvectl --vm $guest->{'name'} --destroy 2>/dev/null";

	## setup network
	my @auto_bridges;	## cache bridges we create
	my @auto_taps;		## cache taps we create
	if ($guest->{'nics'}) {
		## create any bridge devices that don't already exist
		my $ex_bridges = $self->net_list_bridges();
		foreach my $n (@{$guest->{'nics'}}) {
			my $bridge_exists = 0;
			my $bridgeid = $n->{'bridge'};
			foreach my $b (@{$ex_bridges}) {
				if ($bridgeid == $b) {
					$bridge_exists = 1;
					last;
				}
			}
			unless ($bridge_exists) {
				push(@auto_bridges,$bridgeid);
				return 0 unless ($self->net_create_bridge($bridgeid));
			} else {
				$self->debug("bridge${bridgeid} exists");
			}
		}
		## create any taps that don't already exist and add them as members to their bridge
		## we don't want to trash any existing stuff, so if
		##   a tap already exists, we don't attempt to resolve
		##   it's bridge membership
		my $ex_taps = $self->net_list_taps();
		foreach my $n (@{$guest->{'nics'}}) {
			my $tapid = $n->{'tap'};
			my $tap_exists = 0;
			foreach my $t (@{$ex_taps}) {
				if ($tapid == $t) {
					$tap_exists = 1;
					last;
				}
			}
			unless ($tap_exists) {
				push(@auto_taps,$tapid);
				return 0 unless ($self->net_create_tap($tapid));
				## add it to the bridge
				my $bridgeid = $n->{'bridge'};
				return 0 unless ($self->net_add_tap_to_bridge($bridgeid,$tapid));
			} else {
				$self->debug("tap${tapid} exists");
			}
		}
	} else {
		$self->debug("guest has no nics");
	}

	if ($self->{'noop'}) {
		$self->output("(noop) shell%  $destroyvmm");
		$self->output("(noop) shell%  $loadcmd") if ($guest->{'loader'} !~ /uefi/i);
		$self->output("(noop) shell%  $bhyvecmd");
		$self->output("(noop) shell%  $destroyvmm");
	} else {
		my $e = 0;
		RUNLOOP: while ($e == 0) {
			$self->debug($destroyvmm);
			system($destroyvmm);
			if ($guest->{'loader'} !~ /uefi/i) {
				$self->debug($loadcmd);
				my $loadres = system($loadcmd);
				if ($loadres) {
					$loadres = WEXITSTATUS($loadres);
					$self->debug("loader returned: $loadres");
					last RUNLOOP;
				}
			}
			$self->debug($bhyvecmd);
			$e = system($bhyvecmd);
			$self->debug($destroyvmm);
			system($destroyvmm);

			$e = WEXITSTATUS($e);

			switch($e) {
				case 0		{ $self->output("guest rebooted")			}
				case 1		{ $self->output("guest powered off")			}
				case 2		{ $self->output("guest halted")				}
				case 3		{ $self->output("guest triple faulted")			}
				else {
					$self->output("!!! guest returned unknown exit status: $e");
					last RUNLOOP;
				}
			}
		}
	}

	## tear down network
	foreach my $t (@auto_taps) {
		$self->warning("failed to remove tap${t}") unless ($self->net_destroy_tap($t));
	}
	foreach my $b (@auto_bridges) {
		$self->warning("failed to remove bridge${b}") unless ($self->net_destroy_bridge($b));
	}

	return 1;
}

## to support uefi, some slot limitations are enforced
## ahci devices must be in slots 3,4,5, so we start normal enumeration at 6
## slot 3 is for the CD, if booting from one
## slot 4 is for the root volume, if booting uefi
## slot 5 is for the secondary volume, if booting uefi
## only two disks are currently supported for uefi
sub bhyvecmd {
	my ($self,$guest) = @_;
	my $bhyvecmd;
	my $pci = 6;

	$bhyvecmd = "bhyve $bhyve_default_flags -s 0,amd_hostbridge";

	if (($guest->{'loader'} =~ /uefi/i) and ($guest->{'os'} ne 'freebsd')) {
		my $diskc = scalar @{$guest->{'disks'}};
		if ($diskc > 1) {	## root counts as one
			$self->err_set("uefi booting is currently limited to 2 ahci disks, one of which is the root");
			return undef;
		}
		my $disktype = 'ahci-hd';
		$bhyvecmd .= " -s 3,$disktype,".$guest->{'root'}->{'path'};
		$bhyvecmd .= " -s 4,$disktype,".$guest->{'disks'}[0]->{'path'} if ($diskc);
	} else {
		$bhyvecmd .= " -s ".$pci++.",virtio-blk,".$guest->{'root'}->{'path'};
		foreach my $d (@{$guest->{'disks'}}) {
			$bhyvecmd .= " -s ".$pci++.",virtio-blk,".$d->{'path'};
		}
	}

	if ($guest->{'install'}) {
		$bhyvecmd .= " -s 5,ahci-cd,$guest->{'install'}";
	}

	foreach my $n (@{$guest->{'nics'}}) {
		$bhyvecmd .= " -s ".$pci++.",virtio-net,tap".$n->{'tap'};
	}

	if ($guest->{'vnc'}) {
		$bhyvecmd .= " -s ".$pci++.",fbuf,".$guest->{'vnc'};
		if ($guest->{'os'} =~ /windows|linux/) {
			$bhyvecmd .= " -s ".$pci++.",xhci,tablet";
		}
	}

	if ($pci >= 31) {
		$self->err_set("too many devices, lpc must be in slot 31");
		return undef;
	}

	$bhyvecmd .= " -s 31,lpc";
	$bhyvecmd .= " -l com1,$guest->{'com'}";

	if ($guest->{'loader'} =~ /uefi/i) {
		$bhyvecmd .= " -l bootrom," . $guest->{'uefi'};
	}

	$bhyvecmd .= " -c $guest->{'cpu'} -m $guest->{'mem'}M $guest->{'name'}";

	return $bhyvecmd;
}

sub loader_bhyveload {
	my ($self,$guest) = @_;
	my $loadcmd;

	my $root;
	if ($guest->{'install'}) {
		$root = $guest->{'install'};
	} else {
		$root = $guest->{'root'}->{'path'};
	}

	$loadcmd = join(" ",
			"bhyveload",
			"-c $guest->{'com'}",
			"-d $root",
			"-m $guest->{'mem'}M",
			$guest->{'name'}
	);

	return $loadcmd;
}

sub loader_grub {
	my ($self,$guest) = @_;
	my $loadcmd;

	unless ($guest->{'grubmap'}) {
		$self->err_set("must supply a grub devicemap file to boot using grub");
		return undef;
	}
	$guest->{'grubmap'} = $self->file_cannonicalize_path($guest->{'grubmap'},$guest->{'name'});
	if ($self->err_check()) {
		return undef;
	}

	if ($guest->{'grubcmd'}) {
		$guest->{'grubcmd'} = $self->file_cannonicalize_path($guest->{'grubcmd'},$guest->{'name'});
		if ($self->err_check()) {
			return undef;
		}
	}

	my $root = "hd0";
	if ($guest->{'install'}) {
		$root = "cd0";
	}

	$loadcmd = join(" ",
			"grub-bhyve",
			"-m $guest->{'grubmap'}",
			"-r $root",
			"-M $guest->{'mem'}M",
			$guest->{'name'}
	);

	$loadcmd = join(" ",$loadcmd,"<",$guest->{'grubcmd'}) if ($guest->{'grubcmd'});

	return $loadcmd;
}

1;

