require 'test/unit'
require 'xcc_test'

class BasicTest < Test::Unit::TestCase
    def test_minimal
        session = Xcc::Session.new(Xcc::Test::USER,
                                   Xcc::Test::PASS,
                                   Xcc::Test::HOST,
                                   Xcc::Test::DB,
                                   Xcc::Test::PORT)
        assert_instance_of(Xcc::Session, session, "session is not an instance of Xcc::Session")
        request = session.new_adhoc_query("('bunk')")
        assert_instance_of(Xcc::Request, request, "request is not an instance of Xcc::Request")
    end 
end
