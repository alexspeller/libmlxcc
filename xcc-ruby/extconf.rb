require 'mkmf'

dir_config('mlxcc')

unless(have_library("mlxcc"))
    puts "libmlxcc not found. Please specify the path to your libmlxcc installation."
    exit(1)
end

create_makefile("Xcc")
