/*
 cgi.c

 CGI -- C CGI Library -- source

 Copyright (c) 2000 Todor Prokopov
 Distributed under GPL, see COPYING for details

 Todor Prokopov <koprok@newmail.net>

 $Id$

 $Log$
*/

#include "cgi.hpp"

static const char *errmsgs[CGI_NUM_ERRS] = {
  "Success",
  "Unknown request method",
  "Null query string",
  "Memory allocation error",
  "Repeated initialization attempt",
  "Unknown content type",
  "Unsupported content type",
  "Invalid content length",
  "Input block reading error",
  "Invalid urlencoded data"
};

struct pair {
  char *name;
  char *value;
};

int cgi_errno = CGIERR_SUCCESS;
static size_t content_length;
static char *buf;
static int init_called = 0;
static int init_complete = 0;
static unsigned int param_count;
static struct pair *params;
static unsigned int p = 0;
static const char *lastname = NULL;

static int urldecode(char *s)
{
  char *p = s;

  while (*s != '\0')
  {
    if (*s == '%')
    {
      s++;
      if (!isxdigit(*s))
        return 0;
      *p = (isalpha(*s) ? (*s & 0xdf) - 'A' + 10 : *s - '0') << 4;
      s++;
      if (!isxdigit(*s))
        return 0;
      *p += isalpha(*s) ? (*s & 0xdf) - 'A' + 10 : *s - '0';
    }
    else if (*s == '+')
      *p = ' ';
    else
      *p = *s;
    s++;
    p++;
  }
  *p = '\0';
  return 1;
}

static int parse_urlenc(void)
{
  unsigned int i;
  unsigned int p;

  if (content_length != 0)
  {
    param_count = 1;
    for (i = 0; i < content_length; i++)
      if (buf[i] == '&')
        param_count++;
    params = (struct pair *)malloc(param_count * sizeof(struct pair));
    if (params == NULL)
    {
      cgi_errno = CGIERR_MEMORY_ALLOCATION;
      return 0;
    }
    i = 0;
    p = 0;
    while (i < content_length)
    {
      params[p].name = buf + i;
      while (i < content_length && buf[i] != '=' && buf[i] != '&')
        i++;
      if (i >= content_length || buf[i] != '=')
      {
        cgi_errno = CGIERR_INVALID_URLENCODED_DATA;
        return 0;
      }
      buf[i] = '\0';
      i++;
      params[p].value = buf + i;
      while (i < content_length && buf[i] != '=' && buf[i] != '&')
        i++;
      if (i < content_length)
      {
        if (buf[i] != '&')
        {
          cgi_errno = CGIERR_INVALID_URLENCODED_DATA;
          return 0;
        }
        buf[i] = '\0';
        i++;
      }
      if (!urldecode(params[p].name) || !urldecode(params[p].value))
      {
        cgi_errno = CGIERR_INVALID_URLENCODED_DATA;
        return 0;
      }
      p++;
    }
  }
  return 1;
}

static int parse_uint(const char *str, unsigned int *value)
{
  char *tail;

  errno = 0;
  *value = (unsigned int)strtoul(str, &tail, 0);
  if (*tail != '\0' || errno != 0)
    return 0;
  return 1;
}

static int parse_size_t(const char *str, size_t *value)
{
  char *tail;

  errno = 0;
  *value = (size_t)strtoul(str, &tail, 0);
  if (*tail != '\0' || errno != 0)
    return 0;
  return 1;
}

static int init_post_urlenc(void)
{
  char *str;

  str = getenv("CONTENT_LENGTH");
  if (str == NULL)
  {
    cgi_errno = CGIERR_INVALID_CONTENT_LENGTH;
    return 0;
  }
  if (!parse_size_t(str, &content_length))
  {
    cgi_errno = CGIERR_INVALID_CONTENT_LENGTH;
    return 0;
  }
  buf = (char *)malloc(content_length+1);
  if (buf == NULL)
  {
    cgi_errno = CGIERR_MEMORY_ALLOCATION;
    return 0;
  }
  if (content_length != 0)
  {
    if (fread(buf, content_length, 1, stdin) != 1)
    {
      cgi_errno = CGIERR_INPUT_BLOCK_READING;
      return 0;
    }
  }
  buf[content_length] = '\0';
  if (!parse_urlenc())
    return 0;
  return 1;
}

static int init_multipart(void)
{
  cgi_errno = CGIERR_UNSUPPORTED_CONTENT_TYPE;
  return 0;
}

static int init_post(void)
{
  char *str;

  str = getenv("CONTENT_TYPE");
  if (str == NULL)
  {
    cgi_errno = CGIERR_UNKNOWN_CONTENT_TYPE;
    return 0;
  }
  if (strcmp(str, "application/x-www-form-urlencoded") == 0)
  {
    if (!init_post_urlenc())
      return 0;
  }
  else if (strstr(str, "multipart/form-data") == str)
  {
    if (!init_multipart())
      return 0;
  }
  else
  {
    cgi_errno = CGIERR_UNKNOWN_CONTENT_TYPE;
    return 0;
  }
  return 1;
}

static int init_get(void)
{
  char *str;

  str = getenv("QUERY_STRING");
  if (str == NULL)
  {
    cgi_errno = CGIERR_NULL_QUERY_STRING;
    return 0;
  }
  content_length = strlen(str);
  buf = (char *)malloc(content_length+1);
  if (buf == NULL)
  {
    cgi_errno = CGIERR_MEMORY_ALLOCATION;
    return 0;
  }
  memcpy(buf, str, content_length+1);
  if (!parse_urlenc())
    return 0;
  return 1;
}

int cgi_init(void)
{
  char *str;

  if (init_called)
  {
    cgi_errno = CGIERR_REPEATED_INIT_ATTEMPT;
    return 0;
  }
  init_called = 1;
  str = getenv("REQUEST_METHOD");
  if (str == NULL)
  {
    cgi_errno = CGIERR_UNKNOWN_REQUEST_METHOD;
    return 0;
  }
  if (strcmp(str, "POST") == 0)
  {
    if (!init_post())
      return 0;
  }
  else if (strcmp(str, "GET") == 0)
  {
    if (!init_get())
      return 0;
  }
  else
  {
    cgi_errno = CGIERR_UNKNOWN_REQUEST_METHOD;
    return 0;
  }
  init_complete = 1;
  return 1;
}

void cgi_done(void)
{
  if (init_called)
  {
    if (buf != NULL)
      free(buf);
    if (params != NULL)
      free(params);
  }
}

const char *cgi_param(const char *name)
{
  unsigned int q;

  if (!init_complete)
    if (!cgi_init())
      return NULL;
  if (name == NULL)
  {
    p = 0;
    lastname = NULL;
  }
  else
  {
    if (lastname != NULL)
      if (strcmp(name, lastname) != 0)
        p = 0;
    lastname = name;
    while (p < param_count)
    {
      if (strcmp(name, params[p].name) == 0)
      {
        q = p;
        p++;
        return params[q].value;
      }
      p++;
    }
  }
  return NULL;
}

const char *cgi_strerror(int errnum)
{
  if (errnum < 0 || errnum >= CGI_NUM_ERRS)
    return "Unknown error";
  else
    return errmsgs[errnum];
}

/* End of cgi.c */
