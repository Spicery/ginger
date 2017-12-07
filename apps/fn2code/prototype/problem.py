class ProblemException( Exception ):

    def __init__( self, *args, problem=None ):
        super().__init__( *args )
        self._problem = problem

    def problem( self ):
        return self._problem