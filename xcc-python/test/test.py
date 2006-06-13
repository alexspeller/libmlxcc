from xcc import Session, XQueryError, Error

sess = Session("user", "pass", "hostname", "", 8002)
req = sess.new_adhoc_query("('hello world')")

try: 
    res = req.submit()
    while res.has_next():
        item = res.next_item()
        print item
except XQueryError, x:
    print "XQuery Error: ", x.code, " - ", x.message
except Error, x:
    print "XCC System error: ", x
except Exception, x:
    print "Unexpected error!", x
