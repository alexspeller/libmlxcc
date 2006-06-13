use ExtUtils::MakeMaker;

WriteMakefile(
    NAME              => 'Xcc',
    DISTNAME          => 'perl-xcc',
    VERSION_FROM      => 'lib/Xcc/Version.pm', # finds $VERSION
    LIBS              => '-lmlxcc',
    OBJECT            => 'xcc-perl.o',
    PREREQ_PM         => {}, # e.g., Module::Name => 1.1
    ($] >= 5.005 ?     ## Add these new keywords supported since 5.005
      (ABSTRACT  => 'Xcc - Mark Logic XML Content Connector',
       AUTHOR         => 'Andrew Bruno <aeb@qnot.org>') : ()),
);
