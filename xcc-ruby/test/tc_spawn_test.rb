require "xcc/util"
require 'test/unit'
require 'xcc_test'

class SpawnTest < Test::Unit::TestCase
	
	@doc = nil

    def setup
        @session = Xcc::Session.new(Xcc::Test::USER,
                                    Xcc::Test::PASS,
                                    Xcc::Test::HOST,
                                    Xcc::Test::DB,
                                    Xcc::Test::PORT)

        @mod_session = Xcc::Session.new(Xcc::Test::USER,
                                    Xcc::Test::PASS,
                                    Xcc::Test::HOST,
                                    Xcc::Test::MODULESDB,
                                    Xcc::Test::PORT)

        assert_instance_of(Xcc::Session, @mod_session, "modules session is not an instance of Xcc::Session")
        assert_instance_of(Xcc::Session, @session, "session is not an instance of Xcc::Session")

		@doc = Xcc::Document.load(@mod_session, "#{File.dirname(__FILE__)}/support-files/test.xqy", "/test.xqy")
		@doc.grant(Xcc::Permission.new("admin", Xcc::EXECUTE)) 
		@doc.grant(Xcc::Permission.new("admin", Xcc::READ)) 
    end 

    def teardown
		@doc.delete();
        @session = nil
        @mod_session = nil
    end

    def test_spawn_test
		request = @session.new_module_spawn("/test.xqy")
		request.dbname = "Modules"
		result = request.submit()
    end
end
