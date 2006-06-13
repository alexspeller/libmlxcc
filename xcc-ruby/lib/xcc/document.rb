#=============================================================================
#
# Copyright 2006 Ryan Grimm <grimm@xqdev.com> 
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
require "rexml/document"

module Xcc

	# Shared lock
	LOCK_SHARED = "shared"
	
	# Exclusive lock
	LOCK_EXCLUSIVE = "exclusive"

	# This is a simple helper class for Xcc::Document. It is used as a data structure only.
	# It holds the basic infomation about a entry in a documents property sheet.
	class Property

		# Creates a new Property object
		# * name - The name of the property to add to the document
		# * value - The value to set to the new property
		def initialize(name, value)
			@name = name
			@value = value
		end
		
		# Returns the name of the property
		def name()
			return @name
		end

		# Returns the value of the property
		def value()
			return @value
		end
	end

	# This is a simple helper class for Xcc::Document. It is used as a data structure only.
	# It holds the basic information about a permission that is granted or will be granted to a document.
	class Permission

		# Creates a new Permission object
		# * role - A role that is defined in the server.
		# * capability - The permission for this role on a document
		def initialize(role, capability)
			@role = role
			if(capability == Xcc::READ)
				@capability = "read"
			end
			if(capability == Xcc::INSERT)
				@capability = "insert"
			end
			if(capability == Xcc::UPDATE)
				@capability = "update"
			end
			if(capability == Xcc::EXECUTE)
				@capability = "execute"
			end
		end
		
		# Returns the name of the role that the permission is granted to
		def role
			return @role
		end

		# Returns the capability for this role.  Will be one of: Xcc::READ, Xcc::INSERT, Xcc::UPDATE, Xcc::EXECUTE
		def capability
			return @capability
		end
	end

	# This class is designed to help with some of the common and sometimes
	# painful things that one might want to do with documents held in the
	# server.  This includes things like fetching by uri, loading new documents
	# in one line, modifying permissions, collections, quality, properties,
	# moving and copying documents along with fetching various information
	# about the document.
	# ----
	# To use this library: <tt>require 'xcc/util'</tt>
	# ----
	# It should also be said that this class is under development.  As
	# development continues more features will be added which may break
	# backward compatibility.  You have been warned but the ride should be fun.
	class Document

		@hasFetched = false
		@quality = nil

		# Fetches a document from the database
		# * session - A Xcc::Session object
		# * uri - The uri of the document to access
		# * Returns a new Xcc::Document object
		def initialize(session, uri)
			@session = session
			@uri = uri
		end

		# fetch a number of documents based off of the specified xpath
		#def Document.xpath(session, xpath)
		#end

		# Load the given file from the filesystem into the database
		# * session - A Xcc::Session object 
		# * file - The filesystem path to the document to load
		# * uri - The uri that the document should be loaded to
		# * repair - Specifies if the server should attempt to fix a malformed XML document.  Should be one of: Xcc::CORRECTION_FULL or Xcc::CORRECTION_NONE. Default is Xcc::CORRECTION_FULL
		# * Returns a new Xcc::Document object
		def Document.load(session, file, uri, repair = Xcc::CORRECTION_FULL)
			content = session.new_content()
			content.file_name = file
			content.repair = repair
			content.uri = uri

			begin
				bytes = content.insert()
				return Document.new(session, uri)
			rescue XccError => e
				throw e.message
			end
		end

		# private method used for lazy fetching of the document because
		# most functions don't need the actual content
		def fetch()
			if(!@hasFetched)
				query = "
					define variable $uri as xs:string external
					doc($uri)
				"
				request = @session.new_adhoc_query(query)
				request.set_variable(Xcc::STRING, "", "uri", @uri)
				result = request.submit()
				@doc = result.next_item()
				@hasFetched = true
			end
		end

		# Returns the uri of the docuemnt that you have fetched
		def uri()
			return @uri
		end

		# Copies a document to a new uri.  This method also copies all of the
		# permissions, collections, properties and quality of the original
		# document.
		# * new_uri - The uri that the document should be copied to.
		# * Returns a Xcc::Document object for the new document
		def copy(new_uri)

		end

		# Moves a document to a new uri (same as a copy but also removes the original)
		# * new_uri - The new uri that the document will be moved to
		def move(new_uri)
			self.copy(new_uri)
			self.delete()
			@uri = new_uri
		end

		# Saves the document from the database out to a file on the filesystem
		# * file - The filesystem path to write the document out to
		def save(file)
			fetch()
			outfile = File.new(file, "w")
			outfile.print @doc.to_s()
			outfile.close
		end

		# Deletes the document from the database
		def delete()
			query = "
				define variable $uri as xs:string external
				xdmp:document-delete($uri)
			"
			request = @session.new_adhoc_query(query)
			request.set_variable(Xcc::STRING, "", "uri", @uri)
			request.submit()
		end

		# Returns the document as a REXML object.
		# If the document is not XML, nil is returned.
		def to_REXML()
			if(self.format() == "xml")
				return doc = REXML::Document.new(self.to_s())
			end
		end

		# Returns an array of Xcc::Property objects, one for each property that is on the document.
		def properties()
			props = Array.new()
			query = "
				declare namespace prop='http://marklogic.com/xdmp/property'

				define variable $uri as xs:string external
				for $prop in xdmp:document-properties($uri)/prop:properties/*
				return <prop><name>{ local-name($prop) }</name><value>{ string($prop) }</value></prop>
			"
			request = @session.new_adhoc_query(query)
			request.set_variable(Xcc::STRING, "", "uri", @uri)
			result = request.submit()
		
			while(result.has_next?)
				item = result.next_item()
				doc = REXML::Document.new(item.to_s())
				name = ""
				value = ""
				doc.elements.each("/prop/name") { |element| name = element.text }
				doc.elements.each("/prop/value") { |element| value = element.text }
				props.push(Property.new(name, value))
			end
			return props
		end

		# Adds a property to the document.
		# * property - The name of the property to create
		# * value - The value to set to the property
		def add_property(property, value)
			query = "
				define variable $uri as xs:string external
				define variable $property as xs:string external
				define variable $value as xs:string external
				xdmp:document-add-properties($uri, element { $property } { $value })
			"
			request = @session.new_adhoc_query(query)
			request.set_variable(Xcc::STRING, "", "uri", @uri)
			request.set_variable(Xcc::STRING, "", "property", property)
			request.set_variable(Xcc::STRING, "", "value", value.to_s)
			request.submit()
		end

		# Removes a property from the document
		# * property - The name of the property to be removed.  Note: If more then one property on the document has this name they will all be removed.
		def rm_property(property)
			query = "
				define variable $uri as xs:string external
				define variable $property as xs:string external
				xdmp:document-remove-properties($uri, expanded-QName('', $property))
			"
			request = @session.new_adhoc_query(query)
			request.set_variable(Xcc::STRING, "", "uri", @uri)
			request.set_variable(Xcc::STRING, "", "property", property)
			request.submit()
		end

		# Returns an array of the collections that the document is in.
		def collections()
			colls = Array.new()
			query = "
				define variable $uri as xs:string external
				xdmp:document-get-collections($uri)
			"
			request = @session.new_adhoc_query(query)
			request.set_variable(Xcc::STRING, "", "uri", @uri)
			result = request.submit()

			while(result.has_next?)
				item = result.next_item()
				colls.push(item.to_s().chomp())
			end
			return colls
		end

		# Adds the document to the given collection.
		# * collection - The name of the collection that the docuemnt should be added into
		def add_collection(collection)
			query = "
				define variable $uri as xs:string external
				define variable $collection as xs:string external
				xdmp:document-add-collections($uri, $collection)
			"
			request = @session.new_adhoc_query(query)
			request.set_variable(Xcc::STRING, "", "uri", @uri)
			request.set_variable(Xcc::STRING, "", "collection", collection)
			request.submit()
		end

		# Removes the document from the given collection
		# * collection - The name of the collection that the document should be removed from
		def rm_collection(collection)
			query = "
				define variable $uri as xs:string external
				define variable $collection as xs:string external
				xdmp:document-remove-collections($uri, $collection)
			"
			request = @session.new_adhoc_query(query)
			request.set_variable(Xcc::STRING, "", "uri", @uri)
			request.set_variable(Xcc::STRING, "", "collection", collection)
			request.submit()
		end

		# Returns an array of Xcc::Permission objects, one for each permission
		# that has been granted to the document.
		def permissions()
			docperms = Array.new()
			query = <<EOF
declare namespace sec="http://marklogic.com/xdmp/security"

define variable $uri as xs:string external

for $perm in xdmp:document-get-permissions($uri)
let $role := try {
	string(xdmp:eval-in(concat('
		import module "http://marklogic.com/xdmp/security" at "/MarkLogic/security.xqy"
		sec:get-role-names(xs:unsignedLong("', string($perm/sec:role-id), '"))'), xdmp:database("Security")))
	}
	catch($e) {
		"N/A"
	}
return
	<perm>
		<capability>{ string($perm/sec:capability) }</capability>
		<role>{ $role }</role>
	</perm>
EOF
			request = @session.new_adhoc_query(query)
			request.set_variable(Xcc::STRING, "", "uri", @uri)
			result = request.submit()
			while(result.has_next?)
				item = result.next_item()
				doc = REXML::Document.new(item.to_s())
				role = ""
				capability = ""
				doc.elements.each("/perm/role") { |element| role = element.text }
				doc.elements.each("/perm/capability") { |element| capability = element.text }
				if(capability == "read")
					enum_capability = Xcc::READ
				end
				if(capability == "insert")
					enum_capability = Xcc::INSERT
				end
				if(capability == "update")
					enum_capability = Xcc::UPDATE
				end
				if(capability == "execute")
					enum_capability = Xcc::EXECUTE
				end
				docperms.push(Permission.new(role, enum_capability))
			end
			return docperms
		end

		# Adds a permission to the document
		# * permission - A Xcc::Permission object specifying what role and capability should be granted
		def grant(permission)
			query = "
				define variable $uri as xs:string external
				define variable $role as xs:string external
				define variable $permission as xs:string external
				xdmp:document-add-permissions($uri, xdmp:permission($role, $permission))
			"
			request = @session.new_adhoc_query(query)
			request.set_variable(Xcc::STRING, "", "uri", @uri)
			request.set_variable(Xcc::STRING, "", "role", permission.role)
			request.set_variable(Xcc::STRING, "", "permission", permission.capability)
			request.submit()
		end

		# Removes a permission from the document
		# * permission - A Xcc::Permission object specifying what role and capability should be revoked
		def revoke(permission)
			query = "
				define variable $uri as xs:string external
				define variable $role as xs:string external
				define variable $permission as xs:string external
				xdmp:document-remove-permissions($uri, xdmp:permission($role, $permission))
			"
			request = @session.new_adhoc_query(query)
			request.set_variable(Xcc::STRING, "", "uri", @uri)
			request.set_variable(Xcc::STRING, "", "role", permission.role)
			request.set_variable(Xcc::STRING, "", "permission", permission.capability)
			request.submit()
		end

		# Locks a document
		# * scope - The scope to lock the document with.  Should be one of Xcc::LOCK_SHARED or Xcc::LOCK_EXCLUSIVE.
		# * timeout - The length in seconds before the lock expires
		def lock(scope, timeout)
			query = "
				define variable $uri as xs:string external
				define variable $scope as xs:string external
				define variable $timeout as xs:integer external
				xdmp:lock-acquire($uri, $scope, 'infinity', (), $timeout)
			"
			request = @session.new_adhoc_query(query)
			request.set_variable(Xcc::STRING, "", "uri", @uri)
			request.set_variable(Xcc::STRING, "", "scope", scope)
			request.set_variable(Xcc::INTEGER, "", "timeout", timeout.to_s())
			request.submit()
		end

		# Unlocks the document
		def unlock()
			query = "
				define variable $uri as xs:string external
				xdmp:lock-release($uri)
			"
			request = @session.new_adhoc_query(query)
			request.set_variable(Xcc::STRING, "", "uri", @uri)
			request.submit()
		end

		# Returns the quality of the document
		def quality()
			if(@quality == nil)
				query = "
					define variable $uri as xs:string external
					xdmp:document-get-quality($uri)
				"
				request = @session.new_adhoc_query(query)
				request.set_variable(Xcc::STRING, "", "uri", @uri)
				result = request.submit()
				item = result.next_item()
				@quality = item.to_i()
			end
			return @quality
		end

		# Set the quality of the document
		def quality=(quality)
			query = "
				define variable $uri as xs:string external
				define variable $quality as xs:integer external
				xdmp:document-set-quality($uri, $quality)
			"
			request = @session.new_adhoc_query(query)
			request.set_variable(Xcc::STRING, "", "uri", @uri)
			request.set_variable(Xcc::INTEGER, "", "quality", quality.to_s())
			request.submit()
			@quality = quality
		end

		# Gets the format of the document as specified in the server mimetype configuration
		def format()
			query = "
				define variable $uri as xs:string external
				xdmp:uri-format($uri)
			"
			request = @session.new_adhoc_query(query)
			request.set_variable(Xcc::STRING, "", "uri", @uri)
			result = request.submit()
			item = result.next_item()
			return item.to_s().chomp()
		end

		# Gets the mimetype of the document as specified in the server mimetype conifg
		def mimetype()
			query = "
				define variable $uri as xs:string external
				xdmp:uri-content-type($uri)
			"
			request = @session.new_adhoc_query(query)
			request.set_variable(Xcc::STRING, "", "uri", @uri)
			result = request.submit()
			item = result.next_item()
			return item.to_s().chomp()
		end

		# Outputs what forest the document is in
		def forest()
			query = "
				define variable $uri as xs:string external
				xdmp:forest-name(xdmp:document-forest($uri))
			"
			request = @session.new_adhoc_query(query)
			request.set_variable(Xcc::STRING, "", "uri", @uri)
			result = request.submit()
			item = result.next_item()
			return item.to_s().chomp()
		end

		# Output the document as a string
		def to_s()
			fetch()
			@doc.to_s()
		end

		private :fetch

	end

end
