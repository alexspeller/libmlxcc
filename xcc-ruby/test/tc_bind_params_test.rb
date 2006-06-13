require 'test/unit'
require 'xcc_test'
require 'md5'

class BindParmsTest < Test::Unit::TestCase
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

	def test_large_number_of_bind_params

		# number of bind variables to try setting
		num_vars = 200 

		external_vars = ""
		for i in (1 ... num_vars + 1)
			external_vars = external_vars + "define variable $v" + i.to_s + " as xs:integer external \n"
		end

		sum_vars = ""
		for i in (1 ... num_vars)
			sum_vars = sum_vars + "$v" + i.to_s + ", "
		end
		
		sum_vars = sum_vars + "$v" + num_vars.to_s

        request = @session.new_adhoc_query("
			#{external_vars}
			sum((
				#{sum_vars}
			))
		")
		
		for i in (1 ... num_vars + 1)
			request.set_variable(Xcc::INTEGER, "", "v" + i.to_s, i.to_s)
		end

		result = request.submit()

		real_value = 0
		for i in (1 ... num_vars + 1)
			real_value += i
		end

        if(result.has_next?())
            item = result.next_item()
            assert_instance_of(Xcc::Item, item, "item is not an instance of Xcc::Item")
            assert_equal(real_value, item.to_i, "Some bind variables didn't get set") 
        end
	end

	def test_long_bind_param

		value = ""
		for i in (1 ... 1001)
			value = value + " " + MD5.new(i.to_s).hexdigest
		end

        request = @session.new_adhoc_query("
			define variable $long as xs:string external
			$long
		")

		request.set_variable(Xcc::STRING, "", "long", value)
		request.submit
	end
end
