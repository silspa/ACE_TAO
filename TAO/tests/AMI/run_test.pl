eval '(exit $?0)' && eval 'exec perl -S $0 ${1+"$@"}'
    & eval 'exec perl -S $0 $argv:q'
    if 0;

# -*- perl -*-

use lib "$ENV{ACE_ROOT}/bin";
use PerlACE::TestTarget;

my $server = PerlACE::TestTarget::create_target (1) || die "Create target 1 failed\n";
my $client = PerlACE::TestTarget::create_target (2) || die "Create target 2 failed\n";


$server_debug_level = '0';
$client_debug_level = '0';
$iterations = '1';

$conf_file = "muxed$PerlACE::svcconf_ext";

foreach $i (@ARGV) {
    if ($i eq '-mux') {
        $conf_file = "muxed$PerlACE::svcconf_ext";
    }
    elsif ($i eq '-debug') {
        $server_debug_level = '1';
        $client_debug_level = '1';
    }
    elsif ($i eq '-exclusive') {
        $conf_file = "exclusive$PerlACE::svcconf_ext";
    }
}

$client_conf = $client->LocalFile ($conf_file);

my $iorbase = "server.ior";
my $server_iorfile = $server->LocalFile ($iorbase);
my $client_iorfile = $client->LocalFile ($iorbase);
$server->DeleteFile($iorbase);
$client->DeleteFile($iorbase);

$SV = $server->CreateProcess ("server", "-ORBdebuglevel $server_debug_level -o $server_iorfile");

$server_status = $SV->Spawn ();

if ($server_status != 0) {
    print STDERR "ERROR: server returned $server_status\n";
    exit 1;
}

if ($server->WaitForFileTimed ($iorbase,
                               $server->ProcessStartWaitInterval()) == -1) {
    print STDERR "ERROR: cannot find file <$server_iorfile>\n";
    $SV->Kill (); $SV->TimedWait (1);
    exit 1;
}
if ($server->GetFile ($iorbase) == -1) {
    print STDERR "ERROR: cannot get file <$server_iorfile>\n";
    $SV->Kill (); $SV->TimedWait (1);
    exit 1;
}
if ($client->PutFile ($iorbase) == -1) {
    print STDERR "ERROR: cannot set file <$client_iorfile>\n";
    $SV->Kill (); $SV->TimedWait (1);
    exit 1;
}
# copy the configruation file.
if ($client->PutFile ($conf_file) == -1) {
    print STDERR "ERROR: cannot set file <$client_conf>\n";
    $SV->Kill (); $SV->TimedWait (1);
    exit 1;
}

$CL = $client->CreateProcess ("simple_client",
                              "-ORBsvcconf $client_conf "
                              . "-ORBdebuglevel $client_debug_level"
                              . " -k file://$client_iorfile "
                              . " -i $iterations -d");

$client_status = $CL->SpawnWaitKill ($client->ProcessStartWaitInterval());

if ($client_status != 0) {
    print STDERR "ERROR: client returned $client_status\n";
    $status = 1;
}


$CL2 = $client->CreateProcess ("simple_client",
                               "-ORBsvcconf $client_conf"
                               . " -ORBCollocation no"
                               . " -ORBdebuglevel $client_debug_level"
                               . " -k file://$client_iorfile "
                               . " -i $iterations -x -d");

$client_status = $CL2->SpawnWaitKill ($client->ProcessStartWaitInterval());

if ($client_status != 0) {
    print STDERR "ERROR: client returned $client_status\n";
    $status = 1;
}

$server_status = $SV->WaitKill ($server->ProcessStopWaitInterval());

if ($server_status != 0) {
    print STDERR "ERROR: server returned $server_status\n";
    $status = 1;
}

$server->DeleteFile($iorbase);
$client->DeleteFile($iorbase);

exit $status;
