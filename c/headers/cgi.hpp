/*
 cgi.h

 CGI -- C CGI Library -- Header

 Copyright (c) 2000 Todor Prokopov
 Distributed under GPL, see COPYING for details

 Todor Prokopov <koprok@newmail.net>

 $Id$

 $Log$
*/

#ifndef CGI_HPP  /* Prevent multiple includes */
#define CGI_HPP

#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

enum {
  CGIERR_SUCCESS,
  CGIERR_UNKNOWN_REQUEST_METHOD,
  CGIERR_NULL_QUERY_STRING,
  CGIERR_MEMORY_ALLOCATION,
  CGIERR_REPEATED_INIT_ATTEMPT,
  CGIERR_UNKNOWN_CONTENT_TYPE,
  CGIERR_UNSUPPORTED_CONTENT_TYPE,
  CGIERR_INVALID_CONTENT_LENGTH,
  CGIERR_INPUT_BLOCK_READING,
  CGIERR_INVALID_URLENCODED_DATA,
  CGI_NUM_ERRS
};

extern int cgi_errno;

extern int cgi_init(void);
extern void cgi_done(void);
extern const char *cgi_param(const char *name);
extern const char *cgi_strerror(int errnum);

#endif

/* End of cgi.h */
