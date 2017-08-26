class Label:

    next_id = 0

    def __init__( self, name, is_return=False, is_continue=False, uid=None ):
        self.label_name = name
        self.is_return = is_return
        self.is_continue = is_continue
        self.uid = uid or '{}_{}'.format( self.label_name, Label.next_id )
        Label.next_id += 1

    def id( self ):
        return self.uid

Label.CONTINUE = Label( 'CONTINUE', is_continue=True, uid='CONTINUE' )
Label.RETURN = Label( 'RETURN', is_return=True, uid='RETURN' )

