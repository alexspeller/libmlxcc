require "xcc/util"
require 'xcc_test'

class InvokeTest < Test::Unit::TestCase
	
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
		begin
			@doc.grant(Xcc::Permission.new("admin", Xcc::EXECUTE)) 
		rescue Xcc::XQueryError => e
			puts e.to_xml
		end
		@doc.grant(Xcc::Permission.new("admin", Xcc::READ)) 
    end 

    def teardown
		@doc.delete();
        @session = nil
        @mod_session = nil
    end

    def test_invoke_module
		request = @session.new_module_invoke("/test.xqy")
		result = request.submit()
		while(result.has_next?())
			assert_equal("Hello World\n", result.next_item().to_s, "Couldn't invoke module")
		end
    end
end
