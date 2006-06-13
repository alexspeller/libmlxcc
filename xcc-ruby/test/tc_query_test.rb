require 'test/unit'
require 'xcc_test'

class QueryTest < Test::Unit::TestCase
    def setup
        @session = Xcc::Session.new(Xcc::Test::USER,
                                    Xcc::Test::PASS,
                                    Xcc::Test::HOST,
                                    Xcc::Test::DB,
                                    Xcc::Test::PORT)
        assert_instance_of(Xcc::Session, @session, "session is not an instance of Xcc::Session")
    end 

    def teardown
        @session = nil
    end

    def test_server_info
        info = @session.server_info
        assert_not_nil(info, "session.server_info returned nil") 
        assert(info.length > 0, "session.server_info returned 0 length string") 
    end

    def test_query
        request = @session.new_adhoc_query("for $i in ('hello', 'world', 'test') return $i")
        assert_instance_of(Xcc::Request, request, "request is not an instance of Xcc::Request")
        result = request.submit()
        assert_instance_of(Xcc::Result, result, "result is not an instance of Xcc::Result")
        while(result.has_next?())
            item = result.next_item()
            assert_instance_of(Xcc::Item, item, "item is not an instance of Xcc::Item")
            str = item.to_s
            assert_not_nil(str, "item returned nil") 
            assert(str.length > 0, "item returned 0 length string") 
        end
    end

    def test_error_query
        request = @session.new_adhoc_query("for $i in ( return $i")
        assert_instance_of(Xcc::Request, request, "request is not an instance of Xcc::Request")
        begin
            result = request.submit()
            flunk("Invalid xquery request did not throw exception")
        rescue Xcc::XQueryError => e
            xml = e.to_xml
            assert_not_nil(xml, "xml error is nil") 
            assert(xml.length > 0, "xml error string has 0 length") 
        end
    end
end
