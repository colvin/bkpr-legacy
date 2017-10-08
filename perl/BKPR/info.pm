package BKPR::info;

use strict;
use Getopt::Long qw(:config require_order );

sub info {
	my $self = shift;
	my $m = 1;
	GetOptions(
		'q'	=> sub { $m = 0 },
		'v'	=> sub { $m = 2 }
	);
	if ($m < 1) {
		if ($self->{'cfg'}->{'dbtype'} eq 'sqlite') {
			printf("%s %s::%s\n",
				$self->{'VERSION'},
				$self->{'cfg'}->{'dbtype'},
				$self->{'cfg'}->{'dbtarget'}
			);
		} else {
			printf("%s %s::%s@%s/%s\n",
				$self->{'VERSION'},
				$self->{'cfg'}->{'dbtype'},
				$self->{'cfg'}->{'dbuser'},
				$self->{'cfg'}->{'dbtarget'},
				$self->{'cfg'}->{'dbschema'}
			);
		}
	} else {
		$self->output("version:    $self->{'VERSION'}");
		$self->output("config:     $self->{'configfile'}");
		if ($self->{'cfg'}->{'dbtype'} eq 'sqlite') {
			$self->output("database:   $self->{'cfg'}->{'dbtype'}::$self->{'cfg'}->{'dbtarget'}");
		} elsif ($self->{'cfg'}->{'dbtype'} eq 'mysql') {
			$self->output("database:   $self->{'cfg'}->{'dbtype'}::$self->{'cfg'}->{'dbuser'}\@$self->{'cfg'}->{'dbtarget'}/$self->{'cfg'}->{'dbschema'}");
		}
		if ($m > 1) {
			unless ($self->connect()) {
				$self->err_print();
				return 0;
			}
			$self->output("guests:     ".$self->count_guests());
		}
	}
	return 1;
}

1;

