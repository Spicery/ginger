#!/usr/bin/python3
################################################################################
# 	Utilities
################################################################################

import sys

class MinXMLError( Exception ):
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

	def __init__( self, typename, *kids, **kwargs ):
		self.typename = typename
		self.children = []
		self.children.extend( kids )
		self.attributes = {}
		for k, v in kwargs.items():
			self.attributes[ k ] = str( v )

	def copy( self ):
		return MinXML( self.typename, *self.children, **self.attributes )

	def getChildren( self ):
		return self.children

	def getAttributes( self ):
		return self.attributes

	def hasAttribute( self, attname ):
		return attname in self.attributes

	def hasntAttribute( self, attname ):
		return not( attname in self.attributes )

	def hasAttributes( self ):
		return self.attributes and True

	def hasntAttributes( self ):
		return self.attributes and False

	def clear( self ):
		self.children.clear()

	def getName( self ):
		return self.typename

	def setName( self, _name ):
		self.typename = _name

	def hasName( self, name ):
		return self.typename == name

	# Helper method for __str__
	def start_tag( self, list ):
		list.append( '<' )
		list.append( self.typename )
		for ( k, v ) in sorted( self.attributes.items() ):
			list.append( ' ' )
			list.append( k )
			list.append( '="' )
			list.append( _minxml_escape( v ) )
			list.append( '"' )
		list.append( '>' if self.children else '/>' )

	# Helper method for __str__
	def end_tag( self, list ):
		list.append( '</' )
		list.append( self.typename )
		list.append( '>' )

	# Helper method for __str__
	def strlist( self, list ):
		if not self.children:
			self.start_tag( list )
		else:
			self.start_tag( list )
			for k in self.children:
				k.strlist( list )
			self.end_tag( list )

	def __str__( self ):
		sofar = []
		self.strlist( sofar )
		return ''.join( sofar )

	# Helper method for pretty.
	def _pretty_start_tag( self, file, is_empty ):
		print( '<', file=file, end='' )
		print( self.typename, file=file, end='' )
		for ( k, v ) in sorted( self.attributes.items() ):
			print( ' ', k, '="', _minxml_escape( v ), '"', file=file, sep='', end='' )
		print( '/>' if is_empty else '>', file=file )

	# Helper method for pretty.
	def _pretty_end_tag( self, file ):
		print( '</', self.typename, '>', file=file, sep='' )

	def _pretty_indent( self, file, indent ):
		print( '    ' * indent, end='', file=file )

	def pretty( self, file=sys.stdout, indent=0 ):
		if not self.children:
			self._pretty_indent( file, indent )
			self._pretty_start_tag( file, True )
		else:
			self._pretty_indent( file, indent )
			self._pretty_start_tag( file, False )
			for k in self.children:
				k.pretty( file=file, indent=indent+1 )
			self._pretty_indent( file, indent )
			self._pretty_end_tag( file )			

	def add( self, *minx, **kwargs ):
		self.children.extend( minx )
		self.attributes.update( kwargs )
		return self 	# For chaining.

	def __getitem__( self, n ):
		return self.children[ n ]

	def __setitem__( self, n, value ):
		self.children[ n ] = value

	def __len__( self ):
		return len( self.children )

	def __iter__( self ):
		return iter( self.children )

	def __not__( self ):
		return not( self.children )

	def isEmpty( self ):
		return not( self.children )

	def getFirst( self ):
		return self.children[ 0 ]

	def getChild( self, n ):
		return self.children[ n ]

	def setChild( self, n, value ):
		self.children[ n ] = value

	def getLast( self ):
		return self.children[ -1 ]

	def get( self, key, otherwise=KeyError ):
		try:
			return self.attributes[ key ]
		except KeyError:
			if otherwise == KeyError:
				raise KeyError
			return otherwise

	def put( self, *key_values, **kwargs ):
		it = iter( key_values )
		try:
			while True:
				key = it.__next__()
				value = it.__next__()
				self.attributes[ key ] = value
		except StopIteration:
			pass
		self.attributes.update( kwargs )
		return self			# For chaining.

	def has( self, key, value=None ):
		return ( key in self.attributes ) and ( value is None or self.attributes[key] == value )

	def hasnt( self, key, value=None ):
		return ( not key in self.attributes ) or ( value != None and self.attributes[key] != value )

	def clear( self ):
		self.children = []


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
