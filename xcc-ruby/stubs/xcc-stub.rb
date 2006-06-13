#=============================================================================
#
# Copyright 2006 Andrew Bruno <aeb@qnot.org> and Ryan Grimm <grimm@xqdev.com> 
#
# Licensed under the Apache License, Version 2.0 (the "License"); you may not
# use this file except in compliance with the License. You may obtain a copy of
# the License at 
#
#  http://www.apache.org/licenses/LICENSE-2.0 
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
# WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
# License for the specific language governing permissions and limitations under
# the License.
#
#=============================================================================
#
# This file contains only stubs because the real code is found in the xcc-ruby.so
#

# This library allows Ruby to connect to a MarkLogic Database Server.  Its
# design is based off of the XCC/J Java libraries developed by MarkLogic.  This
# library allows users to fully interact with the server issuing queries and
# inserting documents.
# ----
# Creating a new connection:
#  session = Xcc::Session.new("test-user", "uber-secure", "localhost", "test", 8002)
# Inserting a new document:
#  content = session.new_content()
#  content.data = "<data>Some Test Data</data>"
#  content.uri = "/mark/logic/uri.xml"
#  content.insert
# Issuing a query:
#  query = "for $i in input() return base-uri($i)"
#  request = session.new_adhoc_query(query)
#  result = request.submit()
#  while(result.has_next?())
#      puts = result.next_item().to_s
#  end
module Xcc
	# Equivalent to xs:anyURI
	ANYURI = 1

	# Equivalent to xs:base64Binary
	BASE64BINARY = 2

	# Equivalent to binary()
	BINARY = 3

	# Equivalent to xs:boolean
	BOOLEAN = 4

	# Equivalent to xs:date
	DATE = 5

	# Equivalent to xs:dateTime
	DATETIME = 6

	# Equivalent to xs:dayTimeDuration
	DAYTIMEDURATION = 7

	# Equivalent to xs:decimal
	DECIMAL = 8

	# Equivalent to xs:double
	DOUBLE = 9

	# Equivalent to xs:duration
	DURATION = 10

	# Equivalent to xs:float
	FLOAT = 11

	# Equivalent to xs:gDay
	GDAY = 12

	# Equivalent to xs:gMonth
	GMONTH = 13

	# Equivalent to xs:gMonthDay
	GMONTHDAY = 14

	# Equivalent to xs:gYear
	GYEAR = 15

	# Equivalent to xs:gYearMonth
	GYEARMONTH = 16

	# Equivalent to xs:hexBinary
	HEXBINARY = 17

	# Equivalent to xs:integer
	INTEGER = 18

	# Equivalent to node()
	NODE = 19

	# Equivalent to xs:QName
	QNAME = 20

	# Equivalent to xs:string
	STRING = 21

	# Equivalent to xs:text
	TEXT = 22

	# Equivalent to xs:time
	TIME = 23

	# Equivalent to xs:yearMOnthDuration
	YEARMONTHDURATION = 24

	UNKNOWN = 25

	# No specified document format
	FORMAT_NONE = 0

	# Format of the document is binary
	FORMAT_BINARY = 1

	# Format of the document is text
	FORMAT_TEXT = 2

	# format of the document is XML
	FORMAT_XML = 3

	# Don't correct malformed XML
	CORRECTION_NONE = 0

	# Have the server do its best to correct malformed XML
	CORRECTION_FULL = 1

	# Insert permission
	INSERT = 2

	# Update permission
	UPDATE = 4

	# Read permission
	READ = 3

	# Execute permission
	EXECUTE = 5

	# The Content class is used for loading new documents into the MarkLogic
	# database.  You can not create new Content objects directly.  New Content
	# objects can be obtained by calling Xcc::Session.new_conent().
	# ----
	# Summary of use:
	#  session = Xcc::Session.new("test-user", "uber-secure", "localhost", "test", 8002)
	#  content = session.new_content()
	#  content.data = "<data>Some Test Data</data>"
	#  content.uri = "/mark/logic/uri.xml"
	#  content.add_collection("collection1")
	#  content.add_permission(Xcc::READ, "users")
	#  contnet.format = Xcc::FORMAT_XML
	#  contnet.repair = Xcc::CORRECTION_NONE
	#  contnet.quality = 50
	#  content.insert
	class Content
		# Add the document to the specified collection.  If called more then
		# once the document will be in each of the collections.
		# ----
		#  content = session.new_content()
		#  content.file_name = "/path/to/file.xml"
		#  content.uri = "/mark/logic/uri.xml"
		#  content.add_collection("collection1")
		#  content.add_collection("collection2")
		#  content.insert
		# ----
		# In the above example the document with a uri of
		# <tt>/mark/logic/uri.xml</tt> will be placed in two collections
		# <tt>collection1</tt> and <tt>collection2</tt>.
		def add_collection(name)
		end

		# Clears all of the collections that have been set.
		def clear_collections()
		end

		# Adds a permission to the document.
		# * permission - The permission to be granted.  Should be one of: Xcc::INSERT, Xcc::UPDATE, Xcc::READ or Xcc::EXECUTE
		# * role - The role that will be granted the specified permission.
		# ----
		#  content = session.new_content()
		#  content.file_name = "/path/to/file.xml"
		#  content.uri = "/mark/logic/uri.xml"
		#  content.add_permission(Xcc::READ, "users")
		#  content.add_permission(Xcc::INSERT, "users")
		#  content.add_permission(Xcc::UPDATE, "users")
		#  content.insert
		# ----
		# In the above example the <tt>users</tt> group will have read, insert,
		# and update permissions on the document.
		def add_permission(permisssion, role)
		end

		# Clears all of the permisssions that have been set.
		def clear_permissions()
		end

		# Sets the data for the document.
		# * data - The data that will make up the document
		# ----
		#  content = session.new_content()
		#  content.uri = "/mark/logic/uri.xml"
		#  content.data = "<data>Some Test Data</data>"
		#  content.insert
		# ----
		# The above example will create a document with a uri of
		# <tt>/mark/logic/uri.xml</tt> and the contents of that document will
		# be <tt><data>Some Test Data</data></tt>
		# ----
		# See Also: file_name()
		def data=(data)
		end

		# Sets the file to be used as the data for the new document.
		# * file_path - The path to the file to be loaded
		# ----
		#  content = session.new_content()
		#  content.file_name = "/path/to/file.xml"
		#  content.uri = "/mark/logic/uri.xml"
		#  content.insert
		# ----
		# In the above example the file located at <tt>/path/to/file.xml</tt>
		# will be loaded into the database with a uri of
		# <tt>/mark/logic/uri.xml</tt>.
		# ----
		# See Also: data=()
		def file_name(file_path)
		end

		# Sets the format of the document to be loaded.
		# * format - The format that the document should be treated as.  Should be one of:
		# * Xcc::FORMAT_XML - Treat the document as XML (default)
		# * Xcc::FORMAT_TEXT - Treat the document as text
		# * Xcc::FORMAT_BINARY - Treat the document as binary
		# * Xcc::FORMAT_NONE - Don't set a format and let the server figure out what it should be.
		# ----
		#  content = session.new_content()
		#  content.file_name = "/path/to/file.txt"
		#  content.uri = "/mark/logic/uri.txt"
		#  contnet.format = Xcc::FORMAT_TEXT
		#  content.insert
		# ----
		# In the above example, the document will be saved in the database as a text file.
		def format(format)
		end

		# Begins inserting the document into the database
		# * Returns the number of bytes inserted
		def insert()
		end

		# Sets the default namespace for the document.
		def namespace(namespace)
		end

		# Sets the quality to be assigned to the document.
		# ----
		#  content = session.new_content()
		#  content.file_name = "/path/to/file.txt"
		#  content.uri = "/mark/logic/uri.txt"
		#  contnet.quality = 50
		#  content.insert
		# ----
		# Sets the quality of the new document to <tt>50</tt>.
		def quality=(quality)
		end

		# Sets if the server should try to repair the document if it is not
		# well-formed (only applies to XML documents).
		# * type - Should be one of: Xcc::CORRECTION_NONE or Xcc::CORRECTION_FULL
		# ----
		#  content = session.new_content()
		#  content.file_name = "/path/to/file.txt"
		#  content.uri = "/mark/logic/uri.txt"
		#  contnet.repair = Xcc::CORRECTION_NONE
		#  content.insert
		# ----
		def repair=(type)
		end

		# Sets the timeout value for connecting to the server
		def timeout=(timeout)
		end

		# Sets the uri that the document should be stored to.
		# * uri - The uri for the document
		# ----
		#  content = session.new_content()
		#  content.file_name = "/path/to/file.txt"
		#  content.uri = "/mark/logic/uri.txt"
		#  content.insert
		def uri=(uri)
		end
	end

	# The Item class holds the data for a single item in a result set.
	# You can not create new Item objects directly.  New Item
	# objects are returned from Result.next_item().
	class Item
		# Converts the items content to a float
		def to_f()
		end

		# Converts the items content to an integer
		def to_i()
		end

		# Converts the items content to a string
		def to_s()
		end

		# Returns what type of content the item is.  Should be one of:
		# Xcc::INTEGER, Xcc::STRING, Xcc::TEXT, Xcc::BINARY, Xcc::BOOLEAN,
		# Xcc::DECIMAL, Xcc::DOUBLE, Xcc::FLOAT, Xcc::NODE, Xcc::QNAME,
		# Xcc::DATE, Xcc::DATETIME, Xcc::DAYTIMEDURATION, Xcc::DURATION,
		# Xcc::GDAY, Xcc::GMONTH, Xcc::GMONTHDAY, Xcc::GYEAR, Xcc::GYEARMONTH,
		# Xcc::TIME, Xcc::YEARMONTHDURATION, Xcc::ANYURI, Xcc::BASE64BINARY,
		# Xcc::HEXBINARY, Xcc::UNKNOWN
		def type()
		end
	end

	# The Request class is used mainly to set external variables on queries and
	# to actually submit a query for execution.  You can not create new Request
	# objects directly.  New Request objects are returned from
	# Session.new_adhoc_query()
	# ----
	# Summary of use:
	#  session = Xcc::Session.new("test-user", "uber-secure", "localhost", "test", 8002)
	#  query = "
	#      define variable $author as xs:string external
	#      for $i in /book[author = $author] return base-uri($i)
	#  "
	#  request = session.new_adhoc_query(query)
	#  request.set_variable(Xcc::String, "", "author", "Jason Hunter")
	#  result = request.submit()
	class Request
		# This can be used to override the database that is in the Session.
		# * database - The database name to issue this request against
		# ----
		# Summary of use:
		#  session = Xcc::Session.new("test-user", "uber-secure", "localhost", "Documents", 8002)
		#  request = session.new_adhoc_query("for $i in input() return base-uri($i)")
		#  request.dbname = "Security"
		#  result = request.submit()
		# ----
		# The above example will get back a list of files in the Security
		# databae even though the session is connected to the Documents
		# database.
		def dbname=(database)
		end

		# This can be used to override the database that is in the Session.
		# * id - The database id to issue this request against
		# ----
		# Summary of use:
		#  session = Xcc::Session.new("test-user", "uber-secure", "localhost", "Documents", 8002)
		#  request = session.new_adhoc_query("for $i in input() return base-uri($i)")
		#  request.dbid = "1646876832164729896"
		#  result = request.submit()
		# ----
		# The above example will get back a list of files in the database with
		# an id of "1646876832164729896" databae even though the session is
		# connected to the Documents database.
		def dbid=(id)
		end

		# Sets the value for a external (bind) variable.
		# * type - Should be one of: Xcc::INTEGER, Xcc::STRING, Xcc::TEXT, Xcc::BINARY, Xcc::BOOLEAN, Xcc::DECIMAL, Xcc::DOUBLE, Xcc::FLOAT, Xcc::NODE, Xcc::QNAME, Xcc::DATE, Xcc::DATETIME, Xcc::DAYTIMEDURATION, Xcc::DURATION, Xcc::GDAY, Xcc::GMONTH, Xcc::GMONTHDAY, Xcc::GYEAR, Xcc::GYEARMONTH, Xcc::TIME, Xcc::YEARMONTHDURATION, Xcc::ANYURI, Xcc::BASE64BINARY, Xcc::HEXBINARY, Xcc::UNKNOWN
		# * namespace - The namespace to place the variable in
		# * variable_name - The name of the variable to set
		# * value - The value to set the variable to. Note: this needs to be passed in as a string.
		# ----
		#  query = "
		#      define variable $author as xs:string external
		#      for $i in /book[author = $author] return base-uri($i)
		#  "
		#  request = session.new_adhoc_query(query)
		#  request.set_variable(Xcc::String, "", "author", "Jason Hunter")
		#  result = request.submit()
		# ----
		# The above example set the value of <tt>$author</tt> to <tt>Jason Hunter</tt> and then executed the query.
		def set_variable(type, namespace, variable_name, value)
		end

		# Submits the query to the server for execution.
		# * Returns a Xcc::Result object
		# ----
		#  query = "for $i in input() return base-uri($i)"
		#  request = session.new_adhoc_query(query)
		#  result = request.submit()
		def submit()
		end

		# Sets the timeout value for connecting to the server
		def timeout=()
		end
	end

	# The Result object is a product of issuing a query. You can not create new
	# Result objects directly.  New Result objects are returned from
	# Request.submit().  The Request object has methods for finding out if
	# there are any more results to be fetched from the issued query and
	# getting those getting access to those results.
	class Result
		# Returns true if there is another item in the result list that can be fetched.
		# ----
		#  query = "for $i in input() return base-uri($i)"
		#  request = session.new_adhoc_query(query)
		#  result = request.submit()
		#  while(result.has_next?())
		#      puts = result.next_item().to_s
		#  end
		def has_next?()
		end

		# Fetches the next item in the result list.
		# * Returns a Xcc::Item object
		# ----
		#  query = "for $i in input() return base-uri($i)"
		#  request = session.new_adhoc_query(query)
		#  result = request.submit()
		#  while(result.has_next?())
		#      puts = result.next_item().to_s
		#  end
		def next_item()
		end
	end

	# The Session object holds all of the information required to communicate
	# with the database and is the entry point for issuing all queries and
	# loading content into the database.
	# ----
	# To use this library: <tt>require "xcc"</tt>
	class Session
		# Creates a new session (connection) with the MarkLogic Server
		# * user - User to connect to the database as
		# * password - Password for the specified user
		# * host - Hostname of the MarkLogic server (eg: marklogic.mysite.com)
		# * database - Default database to connect to (Only applies to version 3.1 of MarkLogic server)
		# * port - Port number of the XDBC server to connect to
		# ----
		#  require "xcc"
		#  session = Xcc::Session.new("test-user", "uber-secure", "localhost", "test", 8002)
		def initialize(user, password, host, database, port)
		end

		# Creates a new query request to be executed on the server
		# * query - The query to be executed.
		# * Returns a new Xcc::Request object
		# ----
		#  require "xcc"
		#  session = Xcc::Session.new("test-user", "uber-secure", "localhost", "test", 8002)
		#  query = "for $i in input() return base-uri($i)"
		#  request = session.new_adhoc_query(query)
		#  result = request.submit()
		#  while(result.has_next?())
		#      puts = result.next_item().to_s
		#  end
		# ----
		# The above example will return the uri of every document in the database.
		def new_adhoc_query(query)
		end

		# Creates a new module request to be invoked.  The uri should be set to the uri
		# where the module can be found in the database.  External variables
		# can be set on the returned Request object (see
		# Request.set_variable())
		# * uri - The module uri to be invoked
		# * Returns a new Xcc::Request object
		# ----
		#  require "xcc"
		#  session = Xcc::Session.new("test-user", "uber-secure", "localhost", "test", 8002)
		#  request = session.new_module_invoke("http://example.com/sample.xqy")
		#  result = request.submit()
		#  while(result.has_next?())
		#      puts = result.next_item().to_s
		#  end
		# ----
		# The above example will return the result of evaluating the module
		def new_module_invoke(uri)
		end

		# Creates a new module request to be spawned.  The uri should be set to the uri
		# where the module can be found in the database.  External variables
		# can be set on the returned Request object (see
		# Request.set_variable())
		# * uri - The module uri to be spawned
		# * Returns a new Xcc::Request object
		# ----
		#  require "xcc"
		#  session = Xcc::Session.new("test-user", "uber-secure", "localhost", "test", 8002)
		#  request = session.new_module_spawn("http://example.com/sample.xqy")
		#  request.submit()
		# ----
		# The above example will add the module onto the task queue for evaluation
		def new_module_spawn(uri)
		end

		# Creates a new Xcc::Content object.  This object can be used to insert
		# new documents into the database.
		# ----
		#  require "xcc"
		#  session = Xcc::Session.new("test-user", "uber-secure", "localhost", "test", 8002)
		#  content = session.new_content()
		#  content.file_name = "/path/to/file.xml"
		#  content.uri = "/mark/logic/uri.xml"
		#  content.insert
		# ----
		# The above example will insert the file found at
		# <tt>/path/to/file.xml</tt> into the database with a uri of
		# <tt>/mark/logic/uri.xml</tt>
		# ----
		# See Also: Xcc::Document.load()
		def new_content()
		end

		# Returns basic information about the server that the session is communicating with.
		# Equilivant to issuing: <tt>xdmp:server-info()</tt>
		# ----
		#  require "xcc"
		#  session = Xcc::Session.new("test-user", "uber-secure", "localhost", "test", 8002)
		#  puts session.server_info()
		# ----
		# Outputs: <tt>MarkLogic Server Standard - 3.0-5 (linux)</tt>
		def server_info()
		end

		# Issues a query against Mark Logic and returns a result set. 
		# * query - The XQuery to execute
		# * Returns a new Xcc::Result object
		# ----
		#  require "xcc"
		#  session = Xcc::Session.new("test-user", "uber-secure", "localhost", "test", 8002)
		#  result = session.query("('Hello World')")
        #  result.each do |item|
        #      puts item
        #  end
		# ----
		def query(query)
		end

		# Invoke a module on the server and return a result set.
		# * module - The module to invoke
		# * Returns a new Xcc::Result object
		# ----
		#  require "xcc"
		#  session = Xcc::Session.new("test-user", "uber-secure", "localhost", "test", 8002)
		#  result = session.invoke("module.xqy")
        #  result.each do |item|
        #      puts item
        #  end
		# ----
		def invoke(module)
		end

		# Spawn a module on the server
		# * module - The module to spawn
		# * Returns a new Xcc::Result object
		# ----
		#  require "xcc"
		#  session = Xcc::Session.new("test-user", "uber-secure", "localhost", "test", 8002)
		#  session.spawn("module.xqy")
		# ----
		def spawn(module)
		end
	end
end
