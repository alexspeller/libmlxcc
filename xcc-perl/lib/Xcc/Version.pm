package Xcc;

use strict;

our $VERSION = '0.5.3';

1;
__END__

=head1 NAME

Xcc - Mark Logic XML Content Connector 

=head1 SYNOPSIS

  use Xcc;

  my $xcc = Xcc::Session->new("user", "pass", "host", "db", 8004);
  print $xcc->server_info();
  
  my $req = $xcc->new_adhoc_query("('Hello World')");
  my $res = $req->submit();
  
  while($res->has_next()) {
      my $item = $res->next_item();
      print $item->as_string();
  }

=head1 DESCRIPTION

Xcc is a module for communicating with Mark Logic. 

=head1 SEE ALSO

libmlxcc - Mark Logic C API

=head1 AUTHOR

Andrew Bruno, E<lt>aeb@qnot.orgE<gt>

=head1 COPYRIGHT AND LICENSE
 
Copyright 2005 Andrew Bruno <aeb@qnot.org> 

Licensed under the Apache License, Version 2.0 (the "License"); you may not
use this file except in compliance with the License. You may obtain a copy of
the License at 

http://www.apache.org/licenses/LICENSE-2.0 

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
License for the specific language governing permissions and limitations under
the License.

=cut
