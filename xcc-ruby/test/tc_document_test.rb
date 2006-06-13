require 'xcc_test'
require "xcc/util"

class DocumentTest < Test::Unit::TestCase
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
		assert(@session.server_info().length > 0)
	end

	def test_document_load
		doc = Xcc::Document.load(@session, "#{File.dirname(__FILE__)}/support-files/test.xml", "/test.xml")
		assert(doc)
		doc.delete
	end

	def test_document_settings
		doc = Xcc::Document.load(@session, "#{File.dirname(__FILE__)}/support-files/test.xml", "/test.xml")

		doc.add_property("prop1", 1)
		doc.add_property("prop2", 2)

		doc.quality = 10

		doc.add_collection("collection1")
		doc.add_collection("collection2")

		doc.grant(Xcc::Permission.new("security", Xcc::READ))
		doc.grant(Xcc::Permission.new("security", Xcc::INSERT))


		getdoc = Xcc::Document.new(@session, "/test.xml")
		assert_equal(10, getdoc.quality, "Incorrect quality")
		assert_equal("/test.xml", getdoc.uri, "Incorrect uri")

		assert_equal("collection1", getdoc.collections()[0], "Incorrect first collection")
		assert_equal("collection2", getdoc.collections()[1], "Incorrect second collection")

		getdoc.rm_collection("collection2")
		assert_equal("collection1", getdoc.collections()[0], "Incorrect first collection after removal")
		assert_equal(nil, getdoc.collections()[1], "Incorrect second collection after removal")

		assert_equal("xml", getdoc.format(), "Incorrect format")
		assert_equal("text/xml", getdoc.mimetype(), "Incorrect mimetype")

		assert_equal("security", getdoc.permissions()[0].role, "Incorrect first permission role")
		assert_equal("read", getdoc.permissions()[0].capability, "Incorrect first permission capability")
		assert_equal("security", getdoc.permissions()[1].role, "Incorrect second permission role")
		assert_equal("insert", getdoc.permissions()[1].capability, "Incorrect second permission capability")

		assert_equal("prop1", getdoc.properties()[0].name, "Incorrect first property name")
		assert_equal("1", getdoc.properties()[0].value, "Incorrect first property value")
		assert_equal("prop2", getdoc.properties()[1].name, "Incorrect second property name")
		assert_equal("2", getdoc.properties()[1].value, "Incorrect second property value")

		getdoc.rm_property("prop1")

		assert_equal("prop2", getdoc.properties()[0].name, "Incorrect second property name")
		assert_equal("2", getdoc.properties()[0].value, "Incorrect second property value")
		
		assert_equal("<test/>\n", getdoc.to_s, "String value of document isn't correct")
		assert(doc.forest().length > 0, "Can't get back a forest")

		doc.delete
	end
end

