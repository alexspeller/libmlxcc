require 'test/unit'
require 'xcc_test'

class LoadTest < Test::Unit::TestCase
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

    def test_load_content_from_file
        content = @session.new_content
        assert_instance_of(Xcc::Content, content, "content is not an instance of Xcc::Content")
        content.file_name = "#{File.dirname(__FILE__)}/support-files/test.xml"
        content.repair = Xcc::CORRECTION_FULL
        content.uri = "http://test.xcc-ruby.org/files/test.xml"
        bytes = content.insert
        assert_equal(8, bytes, "Failed to insert 8 bytes from test.xml")
    end

    def test_load_content_from_string
        content = @session.new_content
        assert_instance_of(Xcc::Content, content, "content is not an instance of Xcc::Content")
        content.data = "<test>string</test>"
        content.repair = Xcc::CORRECTION_FULL
        content.uri = "/test.xml"
        bytes = content.insert
        assert_equal(19, bytes, "Failed to insert 19 bytes from string")
    end
end
