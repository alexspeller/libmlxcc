require "xcc/util"
require 'xcc_test'

class CmpCharsTest < Test::Unit::TestCase
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
		system("rm test/support-files/temp.xml")
	end

	def test_cmp_chars
		input = "#{File.dirname(__FILE__)}/support-files/chars-input.xml"
		temp = "#{File.dirname(__FILE__)}/support-files/temp.xml"
		out = "#{File.dirname(__FILE__)}/support-files/chars-output.xml"

		doc = Xcc::Document.load(@session, input, "/chars.xml")

		doc.save(temp)

		system("xmllint #{temp} > #{out}")

		assert(system("cmp #{input} #{out}"), "Was not able to round trip the xml document")
	end
end
