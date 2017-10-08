package BKPR::status;

use strict;
use POSIX;
use Getopt::Long qw(:config require_order no_ignore_case );
use Data::Dumper;

sub status {
	my $self = shift;

	unless ($self->connect()) {
		return 0;
	}

#	my %cmdopts;
#	GetOptions(\%cmdopts,
#		'i=i',		## index
#		'n=s'		## name
#	);

	if (-d '/dev/vmm') {
		eval {
			opendir(my $dir,'/dev/vmm')
				or die "cannot open /dev/vmm: $!\n";
			while (my $d = readdir($dir)) {
				next if ($d =~ /^\./);

				my $vmid = $self->name_to_vmid($d);

				my ($bkpr,$bhyve,@cmd);

				## get process information on the bhyve process
				my $ps = `ps ax -o ppid,pid,etime,args | grep bhyve | grep $d | grep -v grep`;
				$ps =~ s/^\s+//g;
				($bhyve->{'ppid'},$bhyve->{'pid'},$bhyve->{'etime'},@cmd) = split(/\s+/,$ps);
				$bhyve->{'cmd'} = join(' ',@cmd);

				## get process information for bhyve's parent, likely a bkpr process
				my $pps = `ps -p $bhyve->{'ppid'} -o ppid,pid,etime,args | tail -n +2`;
				$pps =~ s/^\s+//g;
				($bkpr->{'ppid'},$bkpr->{'pid'},$bkpr->{'etime'},@cmd) = split(/\s+/,$pps);
				$bkpr->{'cmd'} = join(' ',@cmd);

				$self->output("running: $d  vmid: $vmid");
				$self->output("\t($bkpr->{'etime'}) ($bkpr->{'pid'}) $bkpr->{'cmd'}");
				$self->output("\t($bhyve->{'etime'}) ($bhyve->{'pid'}) $bhyve->{'cmd'}");
			}
			closedir($dir);
		}; if ($@) {
			chomp($@);
			$self->err_set($@);
			return 0;
		}
	} else {
		$self->output('no running guests');
	}
}

1;

