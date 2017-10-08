package BKPR::kill;

use strict;
use POSIX;
use Getopt::Long qw(:config require_order no_ignore_case );
use Data::Dumper;

our $bhyvectl = 'bhyvectl';

sub kill {
	my $self = shift;

	unless ($self->connect()) {
		return 0;
	}

	my %cmdopts;
	GetOptions(\%cmdopts,
		'i=i',		## index
		'n=s',		## name
		'r',		## force-reboot
		'p'		## force-poweroff
	);

	unless (($cmdopts{'n'}) or ($cmdopts{'i'})) {
		$self->err_set('invalid configuration: need either name (-n) or index (-i)');
		return 0;
	}
	my $name;
	if ($cmdopts{'i'}) {
		$name = $self->vmid_to_name($cmdopts{'i'});
	} elsif ($cmdopts{'n'}) {
		unless ($self->check_name($cmdopts{'n'})) {
			$self->err_set('no such vm');
			return 0;
		}
		$name = $cmdopts{'n'};
	}

	if (($cmdopts{'r'}) and ($cmdopts{'p'})) {
		$self->err_set('invalid configuration: only one of reboot or poweroff');
		return 0;
	}

	if ($cmdopts{'p'}) {
		my $cmd = join(' ',$bhyvectl,'--vm',$name,'--force-poweroff');
		if ($self->{'noop'}) {
			$self->output("(noop) shell% $cmd");
			return 1;
		}
		$self->output("shell% $cmd");
		my $r = system($cmd);
		if ($r != 0) {
			$r = $r >> 8;
			$self->err_set("failed to poweroff $name, returned $r");
			return 0;
		}
	} elsif ($cmdopts{'r'}) {
		my $cmd = join(' ',$bhyvectl,'--vm',$name,'--force-reset');
		if ($self->{'noop'}) {
			$self->output("(noop) shell% $cmd");
			return 1;
		}
		$self->output("shell% $cmd");
		my $r = system($cmd);
		if ($r != 0) {
			$r = $r >> 8;
			$self->err_set("failed to reset $name, returned $r");
			return 0;
		}
	} else {
		$self->err_set('must give either -r (reboot) or -p (poweroff)');
		return 0;
	}
}

1;

