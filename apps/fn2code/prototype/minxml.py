#!/usr/bin/python3
################################################################################
# 	Utilities
################################################################################

import sys

class MinXMLError( Exception ):
	'''An exception class reserved for this module.'''
	def __init__( self, *args, **kwargs ):
		super().__init__( *args, **kwargs )

def _escape_char( ch ):
	if ch == '"':
		return "&quot;"
	elif ch == '\'':
		return "&apos;"
	elif ch == '<':
		return "&lt;"
	elif ch == '>':
		return "&gt;" 
	elif ch == '&':
		return "&amp;" 
	elif ' ' <= ch and ch <= '~':
		return ch
	else:
		return '&#' + str( ord( ch ) ) + ';'

# Helper function.
def _minxml_escape( text ):
	try:
		sofar = ''
		for ch in text:
			sofar += _escape_char( ch )
		return sofar
	except TypeError:
		raise MinXMLError( "Error while printing MinXML - Unexpected non-text value: {}".format( text ) )


################################################################################
# 	Implementation of MinXML (equivalent to FlexiMinXML)
################################################################################

class MinXML:
	"""An implementation of Minimal XML - a clean subset of XML"""

	def __init__( self, _typename, *kids, **kwargs ):
		self._typename = _typename
		self._children = []
		self._children.extend( kids )
		self._attributes = {}
		for k, v in kwargs.items():
			self._attributes[ k ] = str( v )

	def copy( self ):
		'''Returns a shallow copy of the MinXML element. The parts (name, children
		and attributes) are all copied.'''
		return MinXML( self._typename, *self._children, **self._attributes )

	@property
	def children( self ):
		'''An updateable property that gets or sets the children of the element.'''
		return self._children

	@children.setter
	def children( self, kids ):
		self._children = kids

	def getChildren( self ):
		'''deprecate'''
		return self._children

	@property
	def attributes( self ):
		'''An updateable property that gets or sets the attributes of the element.'''
		return self._attributes

	@attributes.setter
	def attributes( self, a ):
		self._attributes = a

	def getAttributes( self ):
		'''To be deprecated'''
		return self._attributes

	def hasAttribute( self, key ):
		'''Returns true if the element has attributes key.'''
		return key in self._attributes

	def hasAttributeValue( self, key, value ):
		'''Returns true if the element has (key,value) as an attribute.'''
		return key in self._attributes and self._attributes[key] == value

	def hasntAttribute( self, key ):
		'''Returns true if the element doesn't have an attribute with key.'''
		return not( key in self._attributes )

	def hasntAttributeValue( self, key, value ):
		'''Returns true if the element has not got (key,value) as an attribute.'''
		return not( key in self._attributes ) or self._attributes[key] != value

	def hasAnyAttributes( self ):
		'''Returns true if there is one or more pair of attributes for this element.'''
		return self._attributes and True

	def hasntAnyAttributes( self ):
		'''Returns true if there are no pairs of attributes for this element.'''
		return self._attributes and False

	def has( self, key, value=None ):
		'''True if the element has an attribute with key and optionally checks the value too.'''
		return ( key in self._attributes ) and ( value is None or self._attributes[key] == value )

	def hasnt( self, key, value=None ):
		'''True if the element does not have an attribute with key and optionally checks the value too.'''
		return ( not key in self._attributes ) or ( value != None and self._attributes[key] != value )

	def clear( self ):
		'''Discards all the child elements.'''
		self._children.clear()

	@property
	def name( self ):
		'''An updateable property that gets or sets the name of the element.'''
		return self._typename

	@name.setter
	def name( self, new_name ):
		self._typename = new_name

	def getName( self ):
		'''deprecate'''
		return self._typename

	def setName( self, _name ):
		'''deprecate'''
		self._typename = _name

	def hasName( self, name ):
		'''Returns true if the element has the given name.'''
		return self._typename == name

	# Helper method for __str__
	def start_tag( self, list ):
		list.append( '<' )
		list.append( self._typename )
		for ( k, v ) in sorted( self._attributes.items() ):
			list.append( ' ' )
			list.append( k )
			list.append( '="' )
			list.append( _minxml_escape( v ) )
			list.append( '"' )
		list.append( '>' if self._children else '/>' )

	# Helper method for __str__
	def end_tag( self, list ):
		list.append( '</' )
		list.append( self._typename )
		list.append( '>' )

	# Helper method for __str__
	def strlist( self, list ):
		if not self._children:
			self.start_tag( list )
		else:
			self.start_tag( list )
			for k in self._children:
				k.strlist( list )
			self.end_tag( list )

	def __str__( self ):
		sofar = []
		self.strlist( sofar )
		return ''.join( sofar )

	# Helper method for pretty.
	def _pretty_start_tag( self, file, is_empty ):
		print( '<', file=file, end='' )
		print( self._typename, file=file, end='' )
		for ( k, v ) in sorted( self._attributes.items() ):
			print( ' ', k, '="', _minxml_escape( v ), '"', file=file, sep='', end='' )
		print( '/>' if is_empty else '>', file=file )

	# Helper method for pretty.
	def _pretty_end_tag( self, file ):
		print( '</', self._typename, '>', file=file, sep='' )

	def _pretty_indent( self, file, indent ):
		print( '    ' * indent, end='', file=file )

	def pretty( self, file=sys.stdout, indent=0 ):
		'''Pretty prints the element to the optional file.'''
		if not self._children:
			self._pretty_indent( file, indent )
			self._pretty_start_tag( file, True )
		else:
			self._pretty_indent( file, indent )
			self._pretty_start_tag( file, False )
			for k in self._children:
				k.pretty( file=file, indent=indent+1 )
			self._pretty_indent( file, indent )
			self._pretty_end_tag( file )			

	def add( self, *minx, **kwargs ):
		'''Adds children and attributes to the element.'''
		self._children.extend( minx )
		self._attributes.update( kwargs )
		return self 	# For chaining.

	def __getitem__( self, n ):
		'''Returns the n-th child of the element.'''
		return self._children[ n ]

	def __setitem__( self, n, value ):
		'''Sets the n-th child of the element to value.'''
		self._children[ n ] = value

	def __len__( self ):
		'''Returns the number of children of the element.'''
		return len( self._children )

	def __iter__( self ):
		'''Iterator over the children of the element.'''
		return iter( self._children )

	def __not__( self ):
		'''Returns true if the element has no children.'''
		return not( self._children )

	def isEmpty( self ):
		'''Returns true if the element has no children.'''
		return not( self._children )

	@property
	def first( self ):
		return self._children[ 0 ]

	@first.setter
	def first( self, value ):
		self._children[ 0 ] = value

	def getFirst( self ):
		'''To be deprecated'''
		return self._children[ 0 ]

	def getChild( self, n ):
		'''Gets the n-th child of the element.'''
		return self._children[ n ]

	def setChild( self, n, value ):
		'''Sets the n-th child of the element to value.'''
		self._children[ n ] = value

	@property
	def last( self ):
		'''An updateable property that gets or sets the last child.'''
		return self._children[ -1 ]

	@last.setter
	def last( self, value ):
		self._children[ -1 ] = value

	def getLast( self ):
		'''To be deprecated'''
		return self._children[ -1 ]

	def get( self, key, otherwise=KeyError ):
		'''Gets the value of the attribute with the given key. If there
		is no such attribute then it returns a KeyError, unless otherwise
		is given a different value.'''
		try:
			return self._attributes[ key ]
		except KeyError:
			if otherwise == KeyError:
				raise KeyError
			return otherwise

	def put( self, *key_values, **kwargs ):
		'''The variable arguments must be a sequence of alternating keys and values
		of even length, which are used to set the attributes of the element. If
		keyword arguments are supplied, they are also used to set the key/value
		pairs of the element. The intention is to give a lot of syntactic 
		flexibility.'''
		it = iter( key_values )
		try:
			while True:
				key = it.__next__()
				value = it.__next__()
				self._attributes[ key ] = value
		except StopIteration:
			pass
		self._attributes.update( kwargs )
		return self			# For chaining.


################################################################################
# 	MinXML builder class
################################################################################

class Builder():
	
	def __init__( self ):
		self.current_element = MinXML( "DUMMY_NODE" )
		self.element_stack = []

	def startTagOpen( self, name = None ):
		self.startTag( name or '' )

	def startTag( self, _name ):
		self.element_stack.append( self.current_element )
		self.current_element = MinXML( _name )		

	def put( self, key, value ):
		self.current_element.put( key, value )
	
	def _bindName( self, name ):
		if name:
			if self.current_element.hasName( "" ):
				self.current_element.setName( name )
			elif not self.current_element.hasName( name ):
				raise MinXMLError( "Mismatched tags", { "Expected": self.current_element.getName(), "Actual": name } )

	def startTagClose( self, name = None ):
		self._bindName( name )

	def add( self, _child ):
		self.current_element.add( _child )

	def endTag( self, name = None ):
		self._bindName( name )
		b2 = self.element_stack.pop()
		b2.add( self.current_element )
		self.current_element = b2

	def	build( self ):
		if not self.current_element:
			return None
		else:
			result = self.current_element.getLast()
			self.current_element.clear()
			return result


################################################################################
# 	Implementation of MinXML parser.
################################################################################

class Source():
	'''A helper class that supports a pushable stream of characters.'''

	def __init__( self, source ):
		self.source = source
		self.prev = None

	def fetch( self, otherwise ):
		ch = self.source.read( 1 )
		if ch:
			return ch
		if otherwise != None:
			return otherwise
		raise MinXMLError( "Unexpected end of file" ) 

	def fetch_and_cache( self, otherwise ):
		self.prev = self.fetch( otherwise )
		return self.prev

	def nextChar( self, otherwise = None ):
		if self.prev:
			ch = self.prev
			self.prev = None
			return ch
		else:
			return self.fetch( otherwise )

	def pushChar( self, ch ):
		if self.prev:
			raise MinXMLError( "No room to push another character", { 'Pushing': ch } )
		else:
			self.prev = ch

	def peekChar( self, otherwise = None ):
		if self.prev:
			return self.prev
		else:
			return self.fetch_and_cache( otherwise )

	def isNextChar( self, ch ):
		if self.prev:
			return ch == self.prev
		else:
			return ch == self.advance()

	def skipChar( self ):
		if self.prev:
			self.prev = None
		else:
			self.source.read( 1 )

	def hasNextChar( self ):
		return self.peekChar( '' )

	def eatUpTo( self, stop_char ):
		while stop_char != self.nextChar():
			pass

	def eatSpace( self ):
		if self.prev:
			if self.prev.isspace():
				self.prev = None
			else:
				return
		while True:
			ch = self.source.read( 1 )
			if not ch:
				return
			if not ch.isspace():
				self.prev = ch
				return

	def tryReadChar( self, want_ch ):
		if want_ch == self.peekChar( '' ):
			self.skipChar()
			return True
		else:
			return False

	def mustReadChar( self, ch ):
		actual = self.nextChar()
		if actual != ch:
			raise MinXMLError( "Unexpected character", { 'Wanted': ch, 'Actual': actual } )
	
def _readAttributeValue( cucharin ):
	attr = []
	q = cucharin.nextChar()
	if q != '"' and q != '\'':
		raise MinXMLError( "Attribute value not quoted", { "Character": q } )
	while True:
		ch = cucharin.nextChar()
		if ch == q:
			break;
		if ch == '&':
			attr.append( _readEscape( cucharin ) )
		else:
			if ch == '<':
				raise MinXMLError( "Forbidden character in attribute value", { "Character": ch } )
			attr.append( ch )
	return ''.join( attr )

def _readEscapeContent( cucharin ):
	esc = []
	while True:
		ch = cucharin.nextChar()
		if ch == ';':
			break;
		esc.append( ch )
		if len( esc ) > 4:
			raise MinXMLError( "Malformed escape", { "Sequence so far": esc } )
	return ''.join( esc )

_entity_table = {
	'lt': '<',
	'gt': '>',
	'amp': '&',
	'quot': '"',
	'apos': "'"
}

def _entityLookup( symbol ):
	return _entity_table[ symbol ]

def _is_name_char( ch ):
	return ch.isalnum() or ch == '-' or ch == '.' or ch == '_'

def _readEscape( cucharin ):
	esc = _readEscapeContent( cucharin )
	if len( esc ) >= 2 and esc[0] == '#':
		try:
			n = int( esc[1:] )
			return chr( n )
		except ValueError:
			raise MinXMLError( "Unexpected numeric sequence after &#", { "Sequence": esc } )
	else:
		return _entityLookup( esc )

def _readName( cucharin ):
	name = []
	while cucharin.hasNextChar():
		ch = cucharin.nextChar()
		if _is_name_char( ch ):
			name.append( ch )
		else:
			cucharin.pushChar( ch )
			break
	return ''.join( name )

def _eatComment( cucharin, ch ):
	if ch == '!':
		# <!-- .... -->
		if cucharin.tryReadChar( '-' ):
			cucharin.mustReadChar( '-' )
			count_minuses = 0
			while True:
				nch = cucharin.nextChar()
				if nch == '-':
					count_minuses += 1
				elif nch == '>' and count_minuses >= 2:
					break
				else:
					if count_minuses >= 2:
						raise MinXMLError( "Invalid XML comment (while in body of comment)", { "Character following --": nch } )
					count_minuses = 0
		else:
			# <! ...... >
			while True:
				nch = cucharin.nextChar()
				if nch == '>':
					break
				if nch == '<':
					_eatComment( cucharin, cucharin.nextChar() )
	else:
		# <? .... >
		cucharin.eatUpTo( '>' )

def _read1Attribute( cucharin ):
	cucharin.eatSpace()
	c = cucharin.peekChar();
	if c == '/' or c == '>':
		return None
	key = _readName( cucharin )
	cucharin.eatSpace()
	cucharin.mustReadChar( '=' )
	cucharin.eatSpace()
	value = _readAttributeValue( cucharin )
	return ( key, value )

class Parser():
	'''A helper class for reading MinXML from a file object.'''

	def __init__( self, source ):
		self.level = 0
		self.cucharin = Source( source )
		self.pending_end_tag = False
		self.parent = Builder()
		self.tag_name = None

	def _processAttributes( self ):
		while True:
			kv = _read1Attribute( self.cucharin )
			if not kv:
				break
			( k, v ) = kv
			self.parent.put( k, v )

	def readSingleTag( self ):
		
		if self.pending_end_tag:
			self.parent.endTag( self.tag_name )
			self.pending_end_tag = False
			self.level -= 1
			return True
			
		self.cucharin.eatSpace()
		
		if not self.cucharin.hasNextChar():
			return False
		
		self.cucharin.mustReadChar( '<' )
			
		ch = self.cucharin.nextChar()
		if ch == '/':
			end_tag = _readName( self.cucharin )
			self.cucharin.eatSpace();
			self.cucharin.mustReadChar( '>' )
			self.parent.endTag( end_tag )
			self.level -= 1
			return True
		elif ch == '!' or ch == '?':
			_eatComment( self.cucharin, ch )
			return self.readSingleTag()
		else:
			self.cucharin.pushChar( ch )
		
		self.tag_name = _readName( self.cucharin )
		self.parent.startTagOpen( self.tag_name );
		self._processAttributes();
		self.parent.startTagClose( self.tag_name );
		
		self.cucharin.eatSpace()
				
		ch = self.cucharin.nextChar()
		if ch == '/':
			self.cucharin.mustReadChar( '>' )
			self.pending_end_tag = True
			self.level += 1
			return True
		elif ch == '>':
			self.level += 1
			return True
		else:
			raise MinXMLError( "Invalid continuation" )

	def readElement( self ):
		'''Read an element off the input stream or None if the stream is exhausted.
		Returns the next element'''
		while self.readSingleTag() and self.level != 0:
			pass
		if self.level != 0:
			raise MinXMLError( "Unmatched tags due to encountering end of input" );
		return self.parent.build()

	def __iter__( self ):
		while True:
			e = self.readElement()
			if None == e:
				raise StopIteration
			yield e


def readMinXML( fileobj ):
	'''Reads an MinXML element from a file-object.'''
	return Parser( fileobj ).readElement()

################################################################################

# if __name__ == "__main__":
# 	xxx = MinXML( "foo" )
# 	yyy = MinXML( "bar" )
# 	zzz = MinXML( "gort" )
# 	xxx.add( yyy )
# 	xxx.add( zzz )
# 	xxx.put( "alpha", "A" )
# 	xxx.put( "beta", "B" )
# 	xxx.put( "beta", "<>&" )
# 	bbb = Builder()
