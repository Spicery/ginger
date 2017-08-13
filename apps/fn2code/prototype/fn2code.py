#!/usr/bin/env python3

import minxml
import sys

def doSomeTransformation( gnx ):
    # Processing goes here.
    return gnx

def main():
    while True:
        gnx = minxml.readMinXML( sys.stdin )
        if gnx == None:
            break
        gnx = doSomeTransformation( gnx )
        print( gnx )
        sys.stdout.flush()  # Required to ensure proper switch of control between processes.

if __name__ == "__main__":
    main()
