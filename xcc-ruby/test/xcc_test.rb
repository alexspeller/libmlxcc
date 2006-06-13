# Make sure we're using the right .so (not one that has been already installed)
require "#{File.dirname(__FILE__)}/../Xcc.so"
require 'xcc/util'

module Xcc
    class Test
        USER = 'user'
        PASS = 'pass'
        HOST = 'host'
        PORT = 8003
        DB   = ''
		MODULESDB = 'Modules'
    end
end
