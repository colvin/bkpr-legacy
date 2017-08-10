package BKPR::list;

use strict;
use Switch;
use Getopt::Long qw(:config require_order no_ignore_case bundling );

sub list {
	my $self = shift;

	my $format = $self->{'cfg'}->{'listformat'};
	my $i;
	my $n;
	my @w;
	GetOptions(
		't=s' => \$format,
		'i=i' => \$i,
		'n=s' => \$n,
		'w=s' => \@w
	);

	unless ($self->connect()) {
		$self->err_print();
		return 0;
	}

	my @guests;

	if (@w) {
		my $where = 0;
		my $clause = "";
		foreach my $pat (@w) {
			my ($k,$o,$v) = $pat =~ /(.*)\s+(\S)\s+(.*)/;
			unless ($k and $o and $v) {
				$self->err("invalid match criteria: $pat");
				return 0;
			}
			switch($k) {
				case 'vmid'	{}
				case 'name'	{}
				case 'cpu'	{}
				case 'mem'	{}
				case 'os'	{}
				case 'loader'	{}
				case 'root'	{}
				case 'disk'	{}
				#case 'net'	{}
				else {
					$self->err_set("invalid match key: $k");
					return 0;
				}
			}
			if ($v =~ /[;'"\\]/) {
				$self->err("dangerous characters in value");
				return 0;
			}

			if ($where) {
				$clause .= " AND ";
			} else {
				$clause .= " WHERE ";
				$where++;
			}

			switch($o) {
				case '=' { $clause .= "$k = '$v'" }
				case '!' { $clause .= "$k != '$v'" }
				case '>' { $clause .= "$k > '$v'" }
				case '<' { $clause .= "$k < '$v'" }
				case '%' { $clause .= "$k LIKE '\%$v\%'" }
				case '#' { $clause .= "$k NOT LIKE '\%$v\%'" }
				else {
					$self->err("invalid match operator: $o");
					return 0;
				}
			}
		}
		$self->debug("clause: $clause");
		@guests = $self->fetch_guests_filter($clause);
		if ($self->err_check()) {
			return 0;
		}
	} else {
		if ($i or $n) {
			undef $n if ($i);
			if ($n) {
				$i = $self->name_to_vmid($n);
				unless ($i) {
					$self->err("no such vm");
					return 0;
				}
			}
			my $g = $self->fetch_guest($i);
			push(@guests,$g);
		} else {
			my $ids = $self->fetch_vmids();
			foreach my $i (@{$ids}) {
				my $g = $self->fetch_guest($i);
				push(@guests,$g);
			}
		}
	}

	if (scalar(@guests) == 0) {
		$self->warning("no guests");
		return 0;
	}

	if ($format eq 'csv') {
		foreach my $g (@guests) {
			$self->print_guest_csv($g);
		}
	} elsif ($format eq 'col') {
		$self->print_guest_col_header();
		foreach my $g (@guests) {
			$self->print_guest_col($g);
		}
		$self->print_guest_col_trailer();
	} elsif ($format eq 'scol') {
		$self->print_guest_scol_header();
		foreach (@guests) {
			$self->print_guest_scol($_);
		}
		$self->print_guest_scol_trailer();
	} elsif ($format eq 'list') {
		foreach my $g (@guests) {
			$self->print_guest_list_header();
			$self->print_guest_list($g);
			$self->print_guest_list_header();
		}
	} else {
		$self->err("invalid format type: $format");
		return 0;
	}

	return 1;
}

1;

