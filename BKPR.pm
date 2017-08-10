package BKPR;

use strict;
use DBI();
use IO::Select;
use IPC::Open3;
use Switch;
use Data::Dumper;

our @ISA	= qw( );
our @EXPORT	= qw( );
our @EXPORT_OK	= qw( );

our $BKPR_VERSION = "a.12";

sub new {
	my $class = shift;
	my $self;

	$self->{'VERSION'} = $BKPR_VERSION;

	## error storage
	$self->{'err'} = {
		'code'	=> 0,
		'msg'	=> undef
	};

	## default configuration file
	$self->{'configfile'} = "/usr/local/etc/bkpr.cfg";

	## output verbosity
	##	0: quiet
	##	1: normal
	##      2: debugging
	$self->{'verb'} = 1;

	## no-operation (dry run) mode
	$self->{'noop'} = 0;

	bless $self,$class;
	return $self;
}

sub operation {
	my $self = shift;
	my $op = shift;
	switch ($op) {
		case 'info' {
			require BKPR::info;
			return BKPR::info::info($self,@_);
		}
		case 'init' {
			require BKPR::init;
			return BKPR::init::init($self,@_);
		}
		case 'list' {
			require BKPR::list;
			return BKPR::list::list($self,@_);
		}
		case 'test' {
			require BKPR::test;
			return BKPR::test::test($self,@_);
		}
		case 'create' {
			require BKPR::create;
			return BKPR::create::create($self,@_);
		}
		case 'update' {
			require BKPR::update;
			return BKPR::update::update($self,@_);
		}
		case 'destroy' {
			require BKPR::destroy;
			return BKPR::destroy::destroy($self,@_);
		}
		case 'start' {
			require BKPR::start;
			return BKPR::start::start($self,@_);
		}
		case 'status' {
			require BKPR::status;
			return BKPR::status::status($self,@_);
		}
		case 'kill' {
			require BKPR::kill;
			return BKPR::kill::kill($self,@_);
		}
		else {
			$self->err_set("invalid operation: $op");
			return 0;
		}
	}
}

###
#    storage routines
###

sub zfs_check_dataset {
	my $self = shift;
	my $dataset = shift;

	my $list = $self->shell("zfs list -Hp -t all -o name",{ 'yesop' => 1 })->{'out'};
	my @list = split(/\n/,$list);
	foreach my $e (@list) {
		if ($e eq $dataset) {
			return 1;
		}
	}
	return 0; ## dataset does not exist
}

sub zfs_create_dataset {
	my $self = shift;
	my $path = shift;
	my $size = shift;

	unless (($path) and ($size)) {
		$self->err_set("zfs_create_dataset(): must supply a path and a size");
		return 0;
	}

	my $r = $self->shell("zfs create -p -V $size $path");
	if ($r->{'return'} != 0) {
		$self->err_set("zfs_create_dataset(): failed to create $path: ".$r->{'out'});
		return 0;
	}
	return 1;
}

sub file_create_img {
	my $self = shift;
	my $path = shift;
	my $size = shift;

	unless (($path) and ($size)) {
		$self->err_set("file_create_img(): must supply a path and a size");
		return 0;
	}

	my $r = $self->shell("truncate -s $size $path");
	if ($r->{'return'}) {
		$self->err_set("file_create_img(): failed to create $path: ".$r->{'out'});
		return 0;
	}
	return 1;
}

sub disk_parse_spec {
	my $self = shift;
	my $spec = shift;
	my $root = shift;
	$root = 0 unless ($root);

	my $r;

	my ($vtype,$vpath,$vsz) = split(/::/,$spec);
	$r->{'type'} = $vtype;
	$r->{'path'} = $vpath;
	## allow explicit size suffix, default to G
	$vsz = "${vsz}G" if (($vsz) and ($vsz !~ /\d+[:alpha:]/));
	$vsz = 0 unless ($vsz);
	$r->{'size'} = $vsz;
	$r->{'root'} = $root;

	switch($vtype) {
		case 'zvol'	{}
		case 'file'	{}
		else {
			$self->err_set("invalid disk type: $vtype");
			return undef;
		}
	}

	return $r;
}

sub disk_cannonicalize_path {
	my $self = shift;
	my $disk = shift;
	my $name = shift;
	my $id = shift;

	my $path = $disk->{'path'};

	if ($disk->{'type'} eq 'zvol') {
		my $zprefix = $self->{'cfg'}->{'zprefix'};
		if ($path !~ /^\//) {
			if ($path eq 'auto') {
				if ($disk->{'root'}) {
					$path = "/dev/zvol/$zprefix/$name/root";
				} else {
					$path = "/dev/zvol/$zprefix/$name/disk$id";
				}
			} elsif ($path =~ /^auto/) {
				$path =~ s/^auto\///g;
				$path = "/dev/zvol/$zprefix/$name/$path";
			} elsif ($path =~ /^prefix/) {
				$path =~ s/^prefix\///g;
				$path = "/dev/zvol/$zprefix/$path";
			} else {
				$path = "/dev/zvol/$path";
			}
		}
	} elsif ($disk->{'type'} eq 'file') {
		my $prefix = $self->{'cfg'}->{'prefix'};
		if ($path !~ /^\//) {
			if ($path eq 'auto') {
				if ($disk->{'root'}) {
					$path = "$prefix/$name/root";
				} else {
					$path = "$prefix/$name/disk$id";
				}
			} elsif ($path =~ /^auto/) {
				$path =~ s/^auto\///g;
				$path = "$prefix/$name/$path";
			} elsif ($path =~ /^prefix/) {
				$path =~ s/^prefix\///;
				$path = "$prefix/$path";
			} else {
				$path = "$prefix/$name/$path";
			}
		}
	}

	$disk->{'path'} = $path;

	return $disk;
}

sub disk_relative_path {
	my $self = shift;
	my $path = shift;

	$path =~ s/^\/dev\/zvol\///g;

	return $path;
}

sub file_cannonicalize_path {
	my $self = shift;
	my $path = shift;
	my $name = shift;
	my $prefix = $self->{'cfg'}->{'prefix'};
	if ($path =~ /^prefix/) {
		$path =~ s/^prefix\///;
		$path = "$prefix/$path";
	}
	if ($path =~ /^auto/) {
		unless ($name) {
			$self->err_set("file_cannonicalize_path(): must pass the guest name if using auto");
			return undef;
		}
		$path =~ s/^auto\///;
		$path = "$prefix/$name/$path";
	}
	return $path;
}

##
#    network routines
##

sub net_parse_spec {
	my $self = shift;
	my $spec = shift;
	my $r;

	unless (defined($self->{'cfg'}->{'bridge'})) {
		$self->err_set("must have a default bridge specified in configuration");
		return undef;
	}

	if ($spec eq 'auto') {
		$r->{'bridge'} = $self->{'cfg'}->{'bridge'};
		$r->{'tap'} = $self->alloc_tapid() + $self->{'alloc_tapid_increment'};
		$self->{'alloc_tapid_increment'}++;
	} else {
		if ($spec =~ /::/) {
			my ($b,$t) = $spec =~ /(.*)::(.*)/;
			unless (defined($b) and ($b =~ /^\d+$/)) {
				$self->err_set("invalid nic spec");
				return undef;
			}
			$r->{'bridge'} = $b;
			if (defined($t) and ($t ne "") and ($t ne 'auto')) {
				if ($t =~ /^\d+$/) {
					$r->{'tap'} = $t;
				} else {
					$self->err_set("invalid nic spec");
					return 0;
				}
			} else {
				$r->{'tap'} = $self->alloc_tapid() + $self->{'alloc_tapid_increment'};
				$self->{'alloc_tapid_increment'}++;
			}
		} else {
			if ($spec =~ /^\d+$/) {
				$r->{'bridge'} = $self->{'cfg'}->{'bridge'};
				$r->{'tap'} = $spec;
			} else {
				$self->err_set("invalid nic spec");
				return undef;
			}
		}
	}
	return $r;
}

## returns an arrayref of existing bridge device indices
sub net_list_bridges {
	my ($self) = @_;
	my @r;
	my $ifconfig = $self->shell("ifconfig -l",{ 'yesop' => 1 })->{'out'};
	my @intfs = split(/\s+/,$ifconfig);
	foreach my $intf (@intfs) {
		if ($intf =~ /^bridge(\d+)/) {
			push(@r,$1);
		}
	}
	return \@r;
}

## returns an arrayref of existing tap device indices
sub net_list_taps {
	my ($self) = @_;
	my @r;
	my $ifconfig = $self->shell("ifconfig -l",{ 'yesop' => 1 })->{'out'};
	my @intfs = split(/\s+/,$ifconfig);
	foreach my $intf (@intfs) {
		if ($intf =~ /^tap(\d+)/) {
			push(@r,$1);
		}
	}
	return \@r;
}

## return an arrayref of tap indices that are members of the given bridge interface
## bridge should be a full bridge name, ie bridge0
sub net_list_bridge_members {
	my ($self,$bridge) = @_;
	my @r;
	my $ifconfig = $self->shell("ifconfig $bridge",{ 'yesop' => 1 })->{'out'};
	my @ifconfig = split(/\n/,$ifconfig);
	foreach my $line (@ifconfig) {
		if ($line =~ /member: tap(\d+)/) {
			push(@r,$1);
		}
	}
	return \@r;
}

sub net_create_bridge {
	my ($self,$bridgeid) = @_;
	my $s = $self->shell("ifconfig bridge${bridgeid} create");
	if ($s->{'code'}) {
		$self->set_err("failed to create bridge${bridgeid}: ".$s->{'err'});
		return 0;
	}
	return 1;
}

sub net_destroy_bridge {
	my ($self,$bridgeid) = @_;
	my $s = $self->shell("ifconfig bridge${bridgeid} destroy");
	if ($s->{'code'}) {
		$self->set_err("failed to remove bridge${bridgeid}: ".$s->{'err'});
		return 0;
	}
	return 1;
}

sub net_create_tap {
	my ($self,$tapid) = @_;
	my $s = $self->shell("ifconfig tap${tapid} create");
	if ($s->{'code'}) {
		$self->set_err("failed to create tap${tapid}: ".$s->{'err'});
		return 0;
	}
	return 1;
}

sub net_destroy_tap {
	my ($self,$tapid) = @_;
	my $s = $self->shell("ifconfig tap${tapid} destroy");
	if ($s->{'code'}) {
		$self->set_err("failed to remove tap${tapid}: ".$s->{'err'});
		return 0;
	}
	return 1;
}

sub net_add_tap_to_bridge {
	my ($self,$bridgeid,$tapid) = @_;
	my $s = $self->shell("ifconfig bridge${bridgeid} addm tap${tapid}");
	if ($s->{'code'}) {
		$self->set_err("failed to add tap${tapid} to bridge${bridgeid}: ".$s->{'err'});
		return 0;
	}
	return 1;
}


###
#    database routines
###

sub connect {
	my $self = shift;
	my $cfg = $self->{'cfg'};
	unless (defined($self->{'cfg'}->{'dbtype'})) {
		$self->err_set("connect(): no dbtype in configuration");
		return 0;
	}
	my %dbiopts = (
		'RaiseError' => 0,
		'PrintError' => 1
	);
	if ($cfg->{'dbtype'} eq 'mysql') {
		my $mysql;
		unless ($mysql = DBI->connect("DBI:mysql:host=".$cfg->{'dbtarget'}.";database=".$cfg->{'dbschema'},$cfg->{'dbuser'},$cfg->{'dbpass'},\%dbiopts)) {
			$self->err_set("failed database connection: $DBI::errstr");
			return 0;
		}
		$mysql->{'mysql_auto_reconnect'} = 1;
		$self->{'db'} = $mysql;
	} elsif ($cfg->{'dbtype'} eq 'sqlite') {
		my $sqlite;
		unless ($sqlite = DBI->connect("DBI:SQLite:dbname=".$cfg->{'dbtarget'},undef,undef,\%dbiopts)) {
			$self->err_set("failed database connection: $DBI::errstr");
			return 0;
		}
		$self->{'db'} = $sqlite;
	} else {
		$self->err_set("invalid dbtype parameter: $cfg->{'dbtype'}");
		return 0;
	}
	return 1;
}

sub disconnect {
	my $self = shift;
	$self->{'db'}->disconnect() if (defined($self->{'db'}));
}

sub using_sqlite {
	my $self = shift;
	if ($self->{'cfg'}->{'dbtype'} eq 'sqlite') {
		return 1;
	} else {
		return 0;
	}
}

sub using_mysql {
	my $self = shift;
	if ($self->{'cfg'}->{'dbtype'} eq 'mysql') {
		return 1;
	} else {
		return 0;
	}
}

sub db_init {
	my $self = shift;

	eval {
		$self->{'db'}->do("
			CREATE TABLE IF NOT EXISTS `vm` (
				`vmid` INT UNSIGNED NOT NULL PRIMARY KEY,
				`name` VARCHAR(255) NOT NULL,
				`cpu` INT NOT NULL DEFAULT 1,
				`mem` INT UNSIGNED NOT NULL DEFAULT 512,
				`os` VARCHAR(15) NOT NULL DEFAULT 'freebsd',
				`loader` VARCHAR(15) NOT NULL DEFAULT 'bhyveload',
				`grubmap` VARCHAR(1023) DEFAULT NULL,
				`grubcmd` VARCHAR(1023) DEFAULT NULL,
				`descr` VARCHAR(255) DEFAULT NULL
			)");

		if ($self->using_mysql()) {
			$self->{'db'}->do("DROP PROCEDURE IF EXISTS `alloc_index`");
			$self->{'db'}->do("
				CREATE PROCEDURE `alloc_index` ()
					BEGIN
						SELECT min(t1.vmid) AS nextindex FROM (
							SELECT 1  AS vmid
							UNION ALL
							SELECT vmid + 1 FROM vm
						) t1
						LEFT OUTER JOIN vm t2
						ON t1.vmid = t2.vmid
						WHERE t2.vmid IS NULL;
					END
				");
		}

		if ($self->using_mysql()) {
			$self->{'db'}->do("
				CREATE TABLE IF NOT EXISTS `disk` (
					`diskid` INT UNSIGNED AUTO_INCREMENT NOT NULL PRIMARY KEY,
					`vmid` INT UNSIGNED NOT NULL,
					`type` VARCHAR(15) NOT NULL,
					`path` VARCHAR(1023) NOT NULL,
					`root` TINYINT UNSIGNED DEFAULT 0,
					`cloned` TINYINT UNSIGNED DEFAULT 0,
					`cloneof` VARCHAR(1023) DEFAULT NULL
				)");
		} else {
			$self->{'db'}->do("
				CREATE TABLE IF NOT EXISTS `disk` (
					`diskid` INTEGER PRIMARY KEY,
					`vmid` INT UNSIGNED NOT NULL,
					`type` VARCHAR(15) NOT NULL,
					`path` VARCHAR(1023) NOT NULL,
					`root` TINYINT UNSIGNED DEFAULT 0,
					`cloned` TINYINT UNSIGNED DEFAULT 0,
					`cloneof` VARCHAR(1023) DEFAULT NULL
				)");
		}

		if ($self->using_mysql()) {
			$self->{'db'}->do("
				CREATE TABLE IF NOT EXISTS `net` (
					`netid` INT UNSIGNED AUTO_INCREMENT NOT NULL PRIMARY KEY,
					`vmid` INT UNSIGNED NOT NULL,
					`tap` INT UNSIGNED NOT NULL,
					`bridge` SMALLINT UNSIGNED NOT NULL
				)");
		} else {
			$self->{'db'}->do("
				CREATE TABLE IF NOT EXISTS `net` (
					`netid` INTEGER PRIMARY KEY,
					`vmid` INT UNSIGNED NOT NULL,
					`tap` INT UNSIGNED NOT NULL,
					`bridge` SMALLINT UNSIGNED NOT NULL
				)");
		}

		if ($self->using_mysql()) {
			$self->{'db'}->do("DROP PROCEDURE IF EXISTS `alloc_tap`");
			$self->{'db'}->do("
				CREATE PROCEDURE `alloc_tap` ()
					BEGIN
						SELECT min(t1.tap) AS nextindex FROM (
							SELECT 1  AS tap
							UNION ALL
							SELECT tap + 1 FROM net
						) t1
						LEFT OUTER JOIN net t2
						ON t1.tap = t2.tap
						WHERE t2.tap IS NULL;
					END
				");
		}
	}; if ($@) {
		$self->err_set("failed initializing database: $@");
		return 0;
	}
	return 1;
}

sub alloc_vmid {
	my $self = shift;

	if ($self->using_sqlite()) {
		my $id;
		my $c = 1;
		my $vmids = $self->fetch_vmids();
		foreach my $i (@{$vmids}) {
			if ($c < $i) {
				$id = $c;
				last
			}
			$c++;
		}
		$id = $c unless ($id);
		return $id;
	} else {
		return $self->{'db'}->selectrow_hashref("call alloc_index()")->{'nextindex'};
	}
}

sub alloc_tapid {
	my $self = shift;

	if ($self->using_sqlite()) {
		my $id;
		my $c = 0;
		my $tapids = $self->fetch_tapids();
		foreach my $i (@{$tapids}) {
			if ($c < $i) {
				$id = $c;
				last;
			}
			$c++;
		}
		$id = $c unless ($id);
		return $id;
	} else {
		return $self->{'db'}->selectrow_hashref("call alloc_tap()")->{'nextindex'};
	}
}

sub fetch_vmids {
	my $self = shift;

	unless (defined($self->{'db'})) {
		$self->err_set("fetch_vmids(): database not connected");
		return undef;
	}

	my @r;

	my $query = $self->{'db'}->prepare("SELECT vmid FROM vm ORDER BY vmid ASC");
	$query->execute();
	while (my $row = $query->fetchrow_hashref()) {
		push(@r,$row->{'vmid'});
	}
	$query->finish();

	return \@r;
}

sub fetch_tapids {
	my $self = shift;

	unless (defined($self->{'db'})) {
		$self->err_set("fetch_tapids(): database not connected");
		return 0;
	}

	my @N;

	my $query = $self->{'db'}->prepare("SELECT tap FROM net ORDER BY tap ASC");
	$query->execute();
	while (my $row = $query->fetchrow_hashref()) {
		push(@N,$row->{'tap'});
	}
	$query->finish();

	return \@N;
}

sub check_name {
	my $self = shift;
	my $name = shift;
	my $query = $self->{'db'}->prepare("SELECT COUNT(*) AS num FROM vm WHERE name = ?");
	$query->execute($name);
	my $r = $query->fetchrow_hashref();
	return $r->{'num'};
}

sub check_vmid {
	my $self = shift;
	my $vmid = shift;
	return $self->{'db'}->selectrow_hashref("SELECT COUNT(*) AS num FROM vm WHERE vmid = $vmid")->{'num'};
}

sub check_tapid {
	my $self = shift;
	my $tapid = shift;
	return $self->{'db'}->selectrow_hashref("SELECT COUNT(*) AS num FRON net WHERE tapid = $tapid")->{'num'};
}

sub count_guests {
	my $self = shift;

	unless (defined($self->{'db'})) {
		$self->err_set("fetch_guest(): database not connected");
		return undef;
	}

	my $c =  $self->{'db'}->selectrow_hashref("SELECT COUNT(*) AS num FROM vm");
	return $c->{'num'}
}

sub fetch_guest {
	my $self = shift;
	my $vmid = shift;

	unless (defined($self->{'db'})) {
		$self->err_set("fetch_guest(): database not connected");
		return undef;
	}

	my $guest = $self->{'db'}->selectrow_hashref("SELECT * FROM vm WHERE vmid = $vmid");

	my @disks;
	my $diskquery = $self->{'db'}->prepare("SELECT * FROM disk WHERE vmid = $vmid");
	$diskquery->execute();
	while (my $disk = $diskquery->fetchrow_hashref()) {
		if ($disk->{'root'} == 1) {
			$guest->{'root'} = $disk;
		} else {
			push(@disks,$disk);
		}
	}
	$guest->{'disks'} = \@disks;

	my @nics;
	my $nicquery = $self->{'db'}->prepare("SELECT * FROM net WHERE vmid = $vmid");
	$nicquery->execute();
	while (my $nic = $nicquery->fetchrow_hashref()) {
		push(@nics,$nic)
	}
	$guest->{'nics'} = \@nics;

	return $guest;
}

sub fetch_guests_filter {
	my $self = shift;
	my $clause = shift;
	my @guests;

	unless ($clause) {
		$self->err_set("no query clause given");
		return undef;
	}
	unless (defined($self->{'db'})) {
		$self->err_set("fetch_guest(): database not connected");
		return undef;
	}

	my @vmids;
	eval {
		$self->{'db'}->{'RaiseError'} = 1;

		my $id_query = "SELECT DISTINCT(vmid) FROM vm JOIN disk USING (vmid) JOIN net USING (vmid) $clause";

		my $idstmt = $self->{'db'}->prepare($id_query);
		$idstmt->execute();
		while (my $i = $idstmt->fetchrow_hashref()) {
			push(@vmids,$i->{'vmid'});
		}
		$idstmt->finish();

		$self->{'db'}->{'RaiseError'} = 0;
	}; if ($@) {
		$self->{'db'}->{'RaiseError'} = 0;
		chomp($@);
		$self->err_set($@);
	}

	foreach my $vmid (@vmids) {
		my $guest = $self->fetch_guest($vmid);
		push(@guests,$guest);
	}

	return @guests;
}

sub insert_guest {
	my $self = shift;
	my $guest = shift;

	unless (defined($self->{'db'})) {
		$self->set_error("database not connected");
		return 0;
	}

	eval {
		$self->{'db'}->{'RaiseError'} = 1;
		my $vmquery = $self->{'db'}->prepare("
			INSERT INTO vm (vmid,name,cpu,mem,os,loader,grubmap,grubcmd,descr) VALUES
			(?,?,?,?,?,?,?,?,?)");

		$vmquery->execute(
			$guest->{'vmid'},
			$guest->{'name'},
			$guest->{'cpu'},
			$guest->{'mem'},
			$guest->{'os'},
			$guest->{'loader'},
			$guest->{'grubmap'},
			$guest->{'grubcmd'},
			$guest->{'descr'}
		);
		$vmquery->finish();

		my $diskquery = $self->{'db'}->prepare("
			INSERT INTO disk (vmid,type,path,root,cloned,cloneof) VALUES 
			(?,?,?,?,?,?)");

		$diskquery->execute(
				$guest->{'vmid'},
				$guest->{'root'}->{'type'},
				$guest->{'root'}->{'path'},
				$guest->{'root'}->{'root'},
				$guest->{'root'}->{'cloned'},
				$guest->{'root'}->{'cloneof'}
			);

		foreach my $disk (@{$guest->{'disks'}}) {
			$diskquery->execute(
				$guest->{'vmid'},
				$disk->{'type'},
				$disk->{'path'},
				$disk->{'root'},
				$disk->{'cloned'},
				$disk->{'cloneof'}
			);
		}
		$diskquery->finish();

		my $netquery = $self->{'db'}->prepare("
			INSERT INTO net (vmid,tap,bridge) VALUES
			(?,?,?)");

		foreach my $nic (@{$guest->{'nics'}}) {
			$netquery->execute(
				$guest->{'vmid'},
				$nic->{'tap'},
				$nic->{'bridge'}
			);
		}
		$netquery->finish();
	}; if ($@) {
		$self->err_set("failed inserting guest: $@");
		return 0;
	}

	return 1;
}

sub destroy_guest {
	my $self = shift;
	my $vmid = shift;

	unless ($vmid) {
		$self->err_set("destroy_guest(): not passed a vmid");
		return 0;
	}

	eval {
		$self->{'db'}->do("DELETE FROM vm WHERE vmid = $vmid");
		$self->{'db'}->do("DELETE FROM disk WHERE vmid = $vmid");
		$self->{'db'}->do("DELETE FROM net WHERE vmid = $vmid");
	}; if ($@) {
		$self->err_set("failed deleting guest: $@");
		return 0;
	}

	return 1;
}

sub vmid_to_name {
	my $self = shift;
	my $vmid = shift;
	my $r;

	unless (defined($self->{'db'})) {
		$self->err_set("vmid_to_name(): database not connected");
		return undef;
	}

	my $foo = $self->{'db'}->selectrow_hashref("SELECT name FROM vm WHERE vmid = $vmid");

	if (defined($foo->{'name'})) {
		$r = $foo->{'name'};
	}

	return $r;
}

sub name_to_vmid {
	my $self = shift;
	my $vmname = shift;
	my $r;

	unless (defined($self->{'db'})) {
		$self->err_set("name_to_vmid(): database not connected");
		return undef;
	}

	my $foo = $self->{'db'}->selectrow_hashref("SELECT vmid FROM vm WHERE name = '$vmname'");

	if (defined($foo->{'vmid'})) {
		$r = $foo->{'vmid'};
	}

	return $r;
}

###
#    guest output routines
###

sub print_guest_csv {
	my $self = shift;
	my $g = shift;

	$g->{'root'}->{'path'} = $self->disk_relative_path($g->{'root'}->{'path'});
	print "$g->{'vmid'},$g->{'name'},$g->{'cpu'},$g->{'mem'},$g->{'os'},$g->{'loader'},$g->{'grubmap'},$g->{'grubcmd'},root::$g->{'root'}->{'type'}::$g->{'root'}->{'path'}";
	foreach my $disk (@{$g->{'disks'}}) {
		$disk->{'path'} = $self->disk_relative_path($disk->{'path'});
		print ",disk::$disk->{'type'}::$disk->{'path'}";
	}
	foreach my $nic (@{$g->{'nics'}}) {
		print ",net::bridge$nic->{'bridge'}::tap$nic->{'tap'}";
	}
	print "\n";
}

sub print_guest_col {
	my $self = shift;
	my $g = shift;

	printf("| %4d | %-16.16s | %3d | %-5d | %-7.7s | %-10.10s | %4d | %-48.48s | %5d |\n",
					$g->{'vmid'},
					$g->{'name'},
					$g->{'cpu'},
					$g->{'mem'},
					$g->{'os'},
					$g->{'loader'},
					scalar(@{$g->{'nics'}}),
					"$g->{'root'}->{'type'}::$g->{'root'}->{'path'}",
					scalar(@{$g->{'disks'}})
		);
}

sub print_guest_col_header {
	my $self = shift;
	print "+======+==================+=====+=======+=========+============+======+==================================================+=======+\n";
	print "| vmid : name             : cpu : mem   : os      : loader     : nics : root                                             : disks |\n";
	print "+======+==================+=====+=======+=========+============+======+==================================================+=======+\n";
}

sub print_guest_col_trailer {
	my $self = shift;
	print "+------+------------------+-----+-------+---------+------------+------+--------------------------------------------------+-------+\n";
}

sub print_guest_scol_header {
	my $self = shift;
	print "+================+=====+=======+=========+=====+=============================+\n";
	print "| name           : cpu : mem   : os      : vol : descr                       |\n";
	print "+================+=====+=======+=========+=====+=============================+\n";
}

sub print_guest_scol_trailer {
	my $self = shift;
	print "+----------------------------------------------------------------------------+\n";
}

sub print_guest_scol {
	my $self = shift;
	my $g = shift;
	printf("| %-14.14s | %3d | %5d | %-7.7s | %3d | %-27.27s |\n",
			$g->{'name'},
			$g->{'cpu'},
			$g->{'mem'},
			$g->{'os'},
			(scalar(@{$g->{'disks'}}) + 1),
			$g->{'descr'});
}

sub print_guest_list {
	my $self = shift;
	my $guest = shift;

	my $root = $self->disk_relative_path($guest->{'root'}->{'path'});
	$self->output("      id: $guest->{'vmid'}");
	$self->output("    name: $guest->{'name'}");
	$self->output("    cpus: $guest->{'cpu'}");
	$self->output("     mem: $guest->{'mem'}");
	$self->output("      os: $guest->{'os'}");
	$self->output("  loader: $guest->{'loader'}");
	$self->output("   disks: {");
	$self->output("             $guest->{'root'}->{'type'}::$root");
	if (defined($guest->{'disks'})) {
		foreach my $disk (@{$guest->{'disks'}}) {
			my $p = $self->disk_relative_path($disk->{'path'});
			$self->output("             $disk->{'type'}::$p");
		}
	}
	$self->output("          }");
	$self->output("    nics: {");
	foreach my $nic (@{$guest->{'nics'}}) {
		$self->output("             bridge$nic->{'bridge'}::tap$nic->{'tap'}");
	}
	$self->output("          }");
	$self->output(" grubmap: $guest->{'grubmap'}") if (defined($guest->{'grubmap'}));
	$self->output(" grubcmd: $guest->{'grubcmd'}") if (defined($guest->{'grubcmd'}));
	$self->output("   descr: $guest->{'descr'}") if (defined($guest->{'descr'}));
}

sub print_guest_list_header {
	my $self = shift;
	$self->output("+----------------------------------------------+");
}

###
#    utility routines
###

## attempt to convert the user-supplied os type into the internal representation
## returns the acceptable name as a string, or undef on failure
sub normalize_os {
	my $self = shift;
	my $os = shift;
	if ($os =~ /^f/i) {
		return 'freebsd';
	} elsif ($os =~ /^l/i) {
		return 'linux';
	} elsif ($os =~ /^n/i) {
		return 'netbsd';
	} elsif ($os =~ /^o/i) {
		return 'openbsd';
	} elsif ($os =~ /^w/i) {
		return 'windows';
	} elsif ($os =~ /^s/i) {
		return 'sun';
	} elsif ($os =~ /^(p|9)/i) {
		return 'plan9';
	} else {
		$self->err_set("unknown operating system: $os");
		return undef;
	}
}

## ensure that the loader we are storing or using is a valid, supported loader
sub validate_loader {
	my $self = shift;
	my $loader = shift;
	switch($loader) {
		case 'bhyveload'	{ return 1 }
		case 'grub'		{ return 1 }
		case 'uefi'		{ return 1 }
	}
	return 0;
}

## parse the configuration file and pack the config into the object
## boolean method: returns 1 on error, 0 on success
sub parse_config {
	my $self = shift;
	my $fh;
	unless (open ($fh,"<",$self->{'configfile'})) {
		$self->err_set("failed to open configfile: ".$self->{'configfile'});
		return 0;
	}


	while (my $rawline = <$fh>) {
		my $line = $rawline;
		$line =~ s/^\s//g;	## strip leading whitespace
		$line =~ s/#.*$//g;	## strip comments
		$line =~ s/\s+$//g;	## strip trailing whitespace
		next if ($line eq '');	## move on if we reduced the line to nothing
		if ($line =~ /^(\S+)\s+(\S+)$/) {	## key-value pairs
			$self->{'cfg'}->{$1} = $2;
		} elsif ($line =~ /^(\S+)$/) {		## key only
			$self->{'cfg'}->{$1} = 1;
		} else {
			$self->warning("unable to process configuration line: $line");
		}
	}
	return 1;
}

## issues a command on the shell
## captures stdout,stderr, and return code
sub shell {
	my $self = shift;
	my ($cmd,$opts) = @_;
	my $return;
	if (($self->{'noop'}) and (!$opts->{'yesop'})) {
		$self->output("(noop) shell\%  $cmd");
		$return->{'return'} = 0;
		return $return;
	}
	$self->debug("shell\%  $cmd");
	my $pid = open3(my $in, my $out, my $err, $cmd);
	my $sel = new IO::Select;
	$sel->add($out, $err);
	while (my @fhs = $sel->can_read) {
		foreach my $fh (@fhs) {
			my $line = <$fh>;
			unless (defined $line) {
				$sel->remove($fh);
				next;
			}
			if ($fh == $out) {
				$return->{'out'} .= $line;
			} elsif ($fh == $err) {
				$return->{'err'} .= $line;
			} else {
				die "[ERROR]: This should never execute!";
			}
		}
	}
	waitpid($pid, 0);
	my $status = $?;
	if ($status > 0) {
		$status = $status / 256;
	}
	$return->{return} = $status;

	return $return;
}

## produces formatted output on stdout
## verbosity level determines if output is actually printed
sub output {
	my $self = shift;
	my $msg = shift;
	my $header = "[ bkpr ]:";
	if ($self->{'verb'} > 0) {
		print STDOUT "$header $msg\n";
	}
}

## produces formatted debugging output on stderr
## verbosity level determines if output is actually printed
sub debug {
	my $self = shift;
	my $msg = shift;
	#my $header = "( bkpr ):";
	my $header =  "-------->";
	if ($self->{'verb'} > 1) {
		print STDERR "$header $msg\n";
	}
}

## produce formatted warning output on stderr
## not controlled by verbosity setting (you probably want these)
sub warning {
	my $self = shift;
	my $msg = shift;
	print STDERR "[!bkpr!]: $msg\n";
}

## print formatted error message on stderr
## also populates the error structure
## combo print/set
sub err {
	my $self = shift;
	my $msg = shift;
	$self->err_set($msg);
	$self->err_print();
}

## populate the error hash
sub err_set {
	my $self = shift;
	$self->{'err'}->{'code'} = 1;
	$self->{'err'}->{'msg'} = shift;
}

## clear the error hash
sub err_clear {
	my $self = shift;
	$self->{'err'}->{'code'} = 0;
	$self->{'err'}->{'msg'} = undef;
}

## return true if an error is set
sub err_check {
	my $self = shift;
	return 1 if ($self->{'err'}->{'code'});
	return 0;
}

## print the set error on stderr if an error has been set
sub err_print {
	my $self = shift;
	my $header = "[*bkpr*]: ";
	if ($self->{'err'}->{'code'}) {
		print STDERR $header.$self->{'err'}->{'msg'}."\n";
	}
}

## print basic usage message
sub usage {
	my $self = shift;
	$self->output("bkpr -- perl bhyve virtual machine manager");
	$self->output("version $BKPR_VERSION");
	$self->output("usage: bkpr [globalflags] <operation> [...]");
	$self->output("       man bkpr");
}

1;

