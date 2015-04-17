use strict;
use warnings;

my ($file, $cc, $ccopts) = @ARGV;
$ccopts =~ s/;/ /g;
my $deps = `$cc $ccopts -M $file`;

$deps =~ s/\r?\n/ /g;
$deps =~ s/ \\/ /g;
$deps =~ s/^[\w\.]+:\s+//;
$deps =~ s/\s+/;/g;
$deps =~ s/;+$//g;

print $deps, "\n";

1;
