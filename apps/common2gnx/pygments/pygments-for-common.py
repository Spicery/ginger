#!/usr/bin/python
from pygments.lexer import *
from pygments.formatters import *
from pygments import highlight
import sys


from pygments.lexer import RegexLexer
from pygments.token import *

class CommonLexer(RegexLexer):
	name = 'Common'
	aliases = ['common']
	filenames = ['*.common']

	tokens = {
		'root': [
			# There are a couple of other non-coding constructs, including long
			# comments that I will want to sort out later.
			(r'#[- #].*$', Token.Comment),							# End of line comments.
			
			# Legitimate whitespace - nothing special.
			(r'\s+', Token.Text),									# Whitespace.
			
			# This is a complete list of the reserved words of Common.
			(r'also|case|catch|define|do|else|elseif|elseunless|enddefine|endfn|endfor|endif|endpackage|endswitch|endtransaction|endtry|endunless|fn|for|from|if|import|in|package|return|switch|then|throw|to|transaction|try|unless|until|val|var|while|with', Token.Keyword.Reserved ),

			# This is an incomplete definition of names, mainly because the
			# ASCII escape syntax is not properly defined.
			(r'[a-zA-Z_]\w*', Token.Name),									# Keywords.
			
			# This is not right either, since we support floating point (in principle).
			(r'[-+]?\d+', Token.Number),									# Numbers.
			
			# This is correct. Brackets may be decorated by repeated %s.
			(r'[\(\[\{]%*', Token.Punctuation),						# Punctuation.
			(r'%*[\)\]\}]', Token.Punctuation),						# Punctuation.
			# Special case. Needs more work (as does the syntax).
			(r'\$\{', Token.Punctuation),
			
			# Commas and semicolons have very limited glueing power. Similarly 
			# the . and @ operator to prevent catastrophic typos.
			(r',+', Token.Text),									# Self-glue.
			(r';+', Token.Text),									# Self-glue.
			(r'\.+', Token.Operator),									# Self-glue.
			(r'\@+', Token.Operator),									# Self-glue.
			
			# This is a very poor definition of strings and symbols. Really should
			# work on capturing interpolations with Token.Literal.String.Interp
			(r'"[^"]*"', Token.Literal.String.Double ),					# String
			(r"'[^']*'", Token.Literal.String.Single ),					# Symbol
			
			# There are three special constants. Arguably "present" should be added
			# but I am not keen on that name. Even "absent" bothers me to be honest.
			(r'absent|true|false', Keyword.Constant),				# Reserved Word.
			
			# The boolean operators are special - and this is because they are 
			# likely to become incorporated into the sophisticated Prolog-like
			# extension of the language. In this extension the 'if' syntax is 
			# interpreted as 'if QUERY then' and it succeeds if there is a solution
			# to the QUERY.
			(r'and|or|not', Token.Operator.Word ),					# Operator.
			
			# Sign characters all glue with one exception, a > may not be followed 
			# by <. This restriction is necessary for writing expression like this:
			#	<li> "Some text" </li><li> "More text" </li>
			# In this definition we use negative lookbehind (?<! to capture that.
			(r'[-+*/|!?@$%^:=><](?:[-+|/!?@$%^:=>]|(?<!>)<)*', Token.Operator )
		]
	}

def fileToString(infile):
	"""
	Convenience method to read a file into a String.
	"""
	infile = open(filename, 'rb')
	contents = infile.read()
	infile.close()
	return contents

code = sys.stdin.read()
sys.stdout.write( highlight(code, CommonLexer(), TerminalFormatter()))
#sys.stdout.write( highlight(code, CommonLexer(), HtmlFormatter()))
