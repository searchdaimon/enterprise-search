package X::Verify;

use Exception::Class (
    X::Validate => {
	fields => ['reason', 'vars'],
    }
);


1;
