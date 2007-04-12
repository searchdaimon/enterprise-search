package Common::Validate;
use strict;
use warnings;
use Exporter;
use Carp;
our @ISA = qw(Exporter);
our @EXPORT = qw(valid_numeric);

##
# Croaks on not numeric, or missing variables.
#
# Example usage:
# validate_numeric({'id' => 4}) - OK
# validate_numeric({'id' => "hello!"}) - Croaks
sub valid_numeric {
	my $variables_ref = shift;
	my %variables = %{$variables_ref};

	for my $var (keys %variables) {		
		unless (defined $variables{$var}) {
			croak "Error: variable '$var' is not defined.";
		}

		unless ($variables{$var} =~ m{\d+}sx)  {
			croak "Error: variable '$var' is not numeric (value: ", $variables{$var}, ")";
		}
	}
	1;
}

1;