import arity

def test_ArityConstruction():
	'''Can we create an Arity from the expected strings?'''
	a0 = arity.Arity( '0+' )
	assert a0.count() == 0 and not a0.isExact() and a0.isntExact()
	a1 = arity.Arity( '3+' )
	assert a1.count() == 3 and not a1.isExact() and a1.isntExact()
	a2 = arity.Arity( '2' )
	assert a2.count() == 2 and a2.isExact() and not a2.isntExact()
