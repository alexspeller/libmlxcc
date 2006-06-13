use Test::More qw(no_plan);
BEGIN {     
    use_ok('Xcc');
};

ok($Xcc::INTEGER, "Test constants");

my $xcc = Xcc::Session->new("user", "pass", "host", "", 8004);
ok($xcc->server_info() eq "MarkLogic Server Standard - 3.0-6 (linux)\n", "test version");

my $req = $xcc->new_adhoc_query("('Hello World')");
ok(ref($req) eq 'Xcc::Request');

my $res = $req->submit();
ok(ref($res) eq 'Xcc::Result');

my $item = $res->next_item();
ok(ref($item) eq 'Xcc::Item');
