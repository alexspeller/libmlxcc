# This file was created automatically by SWIG.
# Don't modify this file, modify the SWIG interface instead.
# This file is compatible with both classic and new-style classes.

import _xcc

def _swig_setattr_nondynamic(self,class_type,name,value,static=1):
    if (name == "this"):
        if isinstance(value, class_type):
            self.__dict__[name] = value.this
            if hasattr(value,"thisown"): self.__dict__["thisown"] = value.thisown
            del value.thisown
            return
    method = class_type.__swig_setmethods__.get(name,None)
    if method: return method(self,value)
    if (not static) or hasattr(self,name) or (name == "thisown"):
        self.__dict__[name] = value
    else:
        raise AttributeError("You cannot add attributes to %s" % self)

def _swig_setattr(self,class_type,name,value):
    return _swig_setattr_nondynamic(self,class_type,name,value,0)

def _swig_getattr(self,class_type,name):
    method = class_type.__swig_getmethods__.get(name,None)
    if method: return method(self)
    raise AttributeError,name

import types
try:
    _object = types.ObjectType
    _newclass = 1
except AttributeError:
    class _object : pass
    _newclass = 0
del types


from exceptions import Exception, StandardError

INTEGER = _xcc.INTEGER
ANYURI = _xcc.ANYURI
BASE64BINARY = _xcc.BASE64BINARY
BINARY = _xcc.BINARY
BOOLEAN = _xcc.BOOLEAN
DATE = _xcc.DATE
DATETIME = _xcc.DATETIME
DAYTIMEDURATION = _xcc.DAYTIMEDURATION
DECIMAL = _xcc.DECIMAL
DOUBLE = _xcc.DOUBLE
DURATION = _xcc.DURATION
FLOAT = _xcc.FLOAT
GDAY = _xcc.GDAY
GMONTH = _xcc.GMONTH
GMONTHDAY = _xcc.GMONTHDAY
GYEAR = _xcc.GYEAR
GYEARMONTH = _xcc.GYEARMONTH
HEXBINARY = _xcc.HEXBINARY
NODE = _xcc.NODE
QNAME = _xcc.QNAME
STRING = _xcc.STRING
TEXT = _xcc.TEXT
TIME = _xcc.TIME
YEARMONTHDURATION = _xcc.YEARMONTHDURATION
UNKNOWN = _xcc.UNKNOWN
FORMAT_NONE = _xcc.FORMAT_NONE
FORMAT_BINARY = _xcc.FORMAT_BINARY
FORMAT_TEXT = _xcc.FORMAT_TEXT
FORMAT_XML = _xcc.FORMAT_XML
CORRECTION_NONE = _xcc.CORRECTION_NONE
CORRECTION_FULL = _xcc.CORRECTION_FULL
INSERT = _xcc.INSERT
UPDATE = _xcc.UPDATE
READ = _xcc.READ
EXECUTE = _xcc.EXECUTE
class Session(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, Session, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, Session, name)
    def __repr__(self):
        return "<%s.%s; proxy of C XCC_SESS instance at %s>" % (self.__class__.__module__, self.__class__.__name__, self.this,)
    def __init__(self, *args):
        _swig_setattr(self, Session, 'this', _xcc.new_Session(*args))
        _swig_setattr(self, Session, 'thisown', 1)
    def __del__(self, destroy=_xcc.delete_Session):
        try:
            if self.thisown: destroy(self)
        except: pass

    def server_info(*args): return _xcc.Session_server_info(*args)
    def new_adhoc_query(*args): return _xcc.Session_new_adhoc_query(*args)
    def new_module_invoke(*args): return _xcc.Session_new_module_invoke(*args)
    def new_module_spawn(*args): return _xcc.Session_new_module_spawn(*args)
    def new_content(*args): return _xcc.Session_new_content(*args)
    def query(*args): return _xcc.Session_query(*args)
    def invoke(*args): return _xcc.Session_invoke(*args)
    def spawn(*args): return _xcc.Session_spawn(*args)

class SessionPtr(Session):
    def __init__(self, this):
        _swig_setattr(self, Session, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, Session, 'thisown', 0)
        _swig_setattr(self, Session,self.__class__,Session)
_xcc.Session_swigregister(SessionPtr)

class Request(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, Request, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, Request, name)
    def __repr__(self):
        return "<%s.%s; proxy of C XCC_REQ instance at %s>" % (self.__class__.__module__, self.__class__.__name__, self.this,)
    def __init__(self, *args):
        _swig_setattr(self, Request, 'this', _xcc.new_Request(*args))
        _swig_setattr(self, Request, 'thisown', 1)
    def __del__(self, destroy=_xcc.delete_Request):
        try:
            if self.thisown: destroy(self)
        except: pass

    def submit(*args): return _xcc.Request_submit(*args)
    def set_bufsize(*args): return _xcc.Request_set_bufsize(*args)
    def set_timeout(*args): return _xcc.Request_set_timeout(*args)
    def set_dbname(*args): return _xcc.Request_set_dbname(*args)
    def set_dbid(*args): return _xcc.Request_set_dbid(*args)
    def set_timestamp(*args): return _xcc.Request_set_timestamp(*args)
    def set_name(*args): return _xcc.Request_set_name(*args)
    def set_variable(*args): return _xcc.Request_set_variable(*args)

class RequestPtr(Request):
    def __init__(self, this):
        _swig_setattr(self, Request, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, Request, 'thisown', 0)
        _swig_setattr(self, Request,self.__class__,Request)
_xcc.Request_swigregister(RequestPtr)

class Result(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, Result, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, Result, name)
    def __repr__(self):
        return "<%s.%s; proxy of C XCC_RES instance at %s>" % (self.__class__.__module__, self.__class__.__name__, self.this,)
    def __init__(self, *args):
        _swig_setattr(self, Result, 'this', _xcc.new_Result(*args))
        _swig_setattr(self, Result, 'thisown', 1)
    def __del__(self, destroy=_xcc.delete_Result):
        try:
            if self.thisown: destroy(self)
        except: pass

    def has_next(*args): return _xcc.Result_has_next(*args)
    def next_item(*args): return _xcc.Result_next_item(*args)

class ResultPtr(Result):
    def __init__(self, this):
        _swig_setattr(self, Result, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, Result, 'thisown', 0)
        _swig_setattr(self, Result,self.__class__,Result)
_xcc.Result_swigregister(ResultPtr)

class Item(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, Item, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, Item, name)
    def __repr__(self):
        return "<%s.%s; proxy of C XCC_ITEM instance at %s>" % (self.__class__.__module__, self.__class__.__name__, self.this,)
    def __init__(self, *args):
        _swig_setattr(self, Item, 'this', _xcc.new_Item(*args))
        _swig_setattr(self, Item, 'thisown', 1)
    def __del__(self, destroy=_xcc.delete_Item):
        try:
            if self.thisown: destroy(self)
        except: pass

    def __str__(*args): return _xcc.Item___str__(*args)
    def to_double(*args): return _xcc.Item_to_double(*args)
    def to_int(*args): return _xcc.Item_to_int(*args)
    def type(*args): return _xcc.Item_type(*args)

class ItemPtr(Item):
    def __init__(self, this):
        _swig_setattr(self, Item, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, Item, 'thisown', 0)
        _swig_setattr(self, Item,self.__class__,Item)
_xcc.Item_swigregister(ItemPtr)

class Content(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, Content, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, Content, name)
    def __repr__(self):
        return "<%s.%s; proxy of C XCC_CONTENT instance at %s>" % (self.__class__.__module__, self.__class__.__name__, self.this,)
    def __init__(self, *args):
        _swig_setattr(self, Content, 'this', _xcc.new_Content(*args))
        _swig_setattr(self, Content, 'thisown', 1)
    def __del__(self, destroy=_xcc.delete_Content):
        try:
            if self.thisown: destroy(self)
        except: pass

    def clear_collections(*args): return _xcc.Content_clear_collections(*args)
    def clear_permissions(*args): return _xcc.Content_clear_permissions(*args)
    def add_collection(*args): return _xcc.Content_add_collection(*args)
    def add_permission(*args): return _xcc.Content_add_permission(*args)
    def insert(*args): return _xcc.Content_insert(*args)
    def set_file_name(*args): return _xcc.Content_set_file_name(*args)
    def set_data(*args): return _xcc.Content_set_data(*args)
    def set_namespace(*args): return _xcc.Content_set_namespace(*args)
    def set_uri(*args): return _xcc.Content_set_uri(*args)
    def set_timeout(*args): return _xcc.Content_set_timeout(*args)
    def set_format(*args): return _xcc.Content_set_format(*args)
    def set_repair(*args): return _xcc.Content_set_repair(*args)
    def set_quality(*args): return _xcc.Content_set_quality(*args)

class ContentPtr(Content):
    def __init__(self, this):
        _swig_setattr(self, Content, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, Content, 'thisown', 0)
        _swig_setattr(self, Content,self.__class__,Content)
_xcc.Content_swigregister(ContentPtr)

class XccError(StandardError):

    """Base exception class"""

    code = ""
    message = ""

    def __str__(self):
        return "%d - %s" % (self.code, self.message)

class Error(XccError):

    """Xcc Exception"""

class XQueryError(XccError):

    """Exception thrown when problem with issuing XQuery"""

del Exception, StandardError


