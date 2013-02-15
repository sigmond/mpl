/*
 *   Copyright 2013 ST-Ericsson SA
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 *   Author: Per Sigmond <per@sigmond.no>
 *   Author: Harald Johansen <harald.johansen@stericsson.com>
 *   Author: Emil B. Viken <emil.b.viken@gmail.com>
 *
 */

/*************************************************************************
 *
 * File name: mpl_file.c
 *
 * Description: MPL file access implementation
 *
 **************************************************************************/


/*****************************************************************************
 *
 * Include files
 *
 *****************************************************************************/
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mpl_stdint.h"
#include "mpl_file.h"
#include "mpl_param.h"
#include "mpl_dbgtrace.h"

/*****************************************************************************
 *
 * Defines & Type definitions
 *
 *****************************************************************************/

#define MPL_FILE_ALLOC_BLOCKSIZE 1024 /* Will (re)allocate blocks that is a
                                         multiple of this size */
#define MPL_FILE_READ_CHUNKSIZE (MPL_FILE_ALLOC_BLOCKSIZE - 1)

#define MPL_FILE_MAXLINE 256
#define MPL_FILE_READ_MAX_NUMLINES 512

/*****************************************************************************
 *
 * Local variables
 *
 *****************************************************************************/

/*****************************************************************************
 *
 * Private function prototypes
 *
 *****************************************************************************/
static int mpl_get_file(char **buf_pp, size_t max_filesize, FILE *fp);
static int mpl_buffer_gets(char *dest_p, size_t maxline, char **src_pp);
static int mpl_buffer_get_line(char *s,
                               size_t size,
                               char **file_pos_pp,
                               int *line);


/****************************************************************************
 *
 * Public Functions
 *
 ****************************************************************************/


int mpl_file_read_params_bl(FILE* fp,
                            mpl_list_t **param_list_pp,
                            int param_set_id,
                            mpl_blacklist_t blacklist)
{
  char *file_buf_p;
  char *file_pos_p;
  char *buf;
  int line = 0,numargs,i;
  mpl_arg_t *args_p;
  mpl_param_element_t* unpackparam = NULL;
  int res;

  if (fp == NULL)
  {
    MPL_DBG_TRACE_ERROR(E_MPL_INVALID_PARAMETER,
                        ("File descriptor is NULL, "
                         "no parameter file loaded\n"));
    mpl_set_errno(E_MPL_INVALID_PARAMETER);
    return -1;
  }

  if ((res = mpl_get_file(&file_buf_p,
                          (MPL_FILE_READ_MAX_NUMLINES * MPL_FILE_MAXLINE),
                          fp)) < 0)
  {
    return -1;
  }

  if (res == 0)
  {
    /* File was empty */
    free(file_buf_p);
    return 0;
  }

  args_p = malloc(sizeof(mpl_arg_t)*MPL_MAX_ARGS);
  if(NULL == args_p)
  {
    MPL_DBG_TRACE_ERROR(E_MPL_FAILED_ALLOCATING_MEMORY,
                        ("failed allocating memory!\n"));
    mpl_set_errno(E_MPL_FAILED_ALLOCATING_MEMORY);
    free(file_buf_p);
    return -1;
  }
  memset(args_p,0,sizeof(mpl_arg_t)*MPL_MAX_ARGS);

  buf = malloc(MPL_FILE_MAXLINE);
  if(NULL == buf)
  {
    MPL_DBG_TRACE_ERROR(E_MPL_FAILED_ALLOCATING_MEMORY,
                        ("failed allocating memory!\n"));
    mpl_set_errno(E_MPL_FAILED_ALLOCATING_MEMORY);
    free(file_buf_p);
    free(args_p);
    return -1;
  }

  file_pos_p = file_buf_p;
  while ((res = mpl_buffer_get_line(buf,
                                    MPL_FILE_MAXLINE,
                                    &file_pos_p,
                                    &line)) > 0)
  {
    numargs = mpl_get_args(args_p, MPL_MAX_ARGS, buf, '=', ';', '\\');
    for(i=0;i<numargs;i++)
    {
      if (mpl_param_unpack_param_set(args_p[i].key_p,
                                     args_p[i].value_p,
                                     &unpackparam,
                                     param_set_id) < 0)
      {
        MPL_DBG_TRACE_ERROR(E_MPL_FAILED_OPERATION,
                            ("mpl_unpack_param() failed at line %d in file\n",
                             line));
        mpl_set_errno(E_MPL_FAILED_OPERATION);
        continue;//skip the bad parameters
      }

      /* Is the parameter in the blacklist? */
      if (mpl_param_list_find(unpackparam->id, blacklist))
      {
        mpl_param_element_destroy(unpackparam);
        continue;//skip parameters in the blacklist
      }
      mpl_list_add(param_list_pp, &unpackparam->list_entry);
    }
  }

  free(file_buf_p);
  free(args_p);
  free(buf);

  if (res < 0)
  {
    return -1;
  }

  return 0;
}

int mpl_file_write_params_internal(FILE *fp,
                                   mpl_list_t *param_list_p,
                                   bool no_prefix)
{
  int buflen;
  char *buf_p;
  size_t res;
  mpl_pack_options_t options = MPL_PACK_OPTIONS_DEFAULT;
  options.no_prefix = no_prefix;
  options.message_delimiter = '\n';

  if (fp == NULL)
  {
    MPL_DBG_TRACE_ERROR(E_MPL_INVALID_PARAMETER,
                        ("File descriptor is NULL, "
                         "no parameter file loaded\n"));
    mpl_set_errno(E_MPL_INVALID_PARAMETER);
    return -1;
  }

  /* Calculate buffer space */
  buflen = mpl_param_list_pack_extended(param_list_p,
                                        NULL,
                                        0,
                                        &options
                                       );
  if (buflen <= 0)
    return -1;

  buf_p = malloc((size_t)(buflen + 1));
  if (buf_p == NULL)
  {
    MPL_DBG_TRACE_ERROR(E_MPL_FAILED_ALLOCATING_MEMORY,("buf_p\n"));
    mpl_set_errno(E_MPL_FAILED_ALLOCATING_MEMORY);
    return -1;
  }

  /* Pack list in to the buffer */
  buflen = mpl_param_list_pack_extended(param_list_p,
                                        buf_p,
                                        buflen + 1,
                                        &options
                                       );
  if (buflen <= 0)
  {
    free(buf_p);
    return -1;
  }

  /* Write to file */
  res = fwrite(buf_p, sizeof(char), (size_t)buflen, fp);
  if (res != (size_t)buflen)
  {
    MPL_DBG_TRACE_ERROR(E_MPL_FAILED_OPERATION,
                        ("fwrite returned %zu (trying to write %d bytes)\n",
                         res,
                         buflen));/*lint !e557 zu is C99 */
    mpl_set_errno(E_MPL_FAILED_OPERATION);
    free(buf_p);
    return -1;
  }

  free(buf_p);
  return 0;
}

/****************************************************************************
 *
 * Private Functions
 *
 ****************************************************************************/

/**
 * mpl_get_file()
 **/
static int mpl_get_file(char **buf_pp, size_t max_filesize, FILE *fp)
{
  char *buf_p;
  char *buf_pos_p;
  size_t bytes_read;
  size_t total_bytes_read = 0;
  size_t buffer_size;

  buffer_size = MPL_FILE_READ_CHUNKSIZE;

  buf_p = malloc(buffer_size + 1); /* Allocate an extra byte for zero
                                      termination */
  if (buf_p == NULL)
  {
    MPL_DBG_TRACE_ERROR(E_MPL_FAILED_ALLOCATING_MEMORY,("malloc buf_p\n"));
    mpl_set_errno(E_MPL_FAILED_ALLOCATING_MEMORY);
    return -1;
  }
  buf_pos_p = buf_p;

  do
  {
    size_t max_bytes_to_read;

    max_bytes_to_read = MPL_FILE_READ_CHUNKSIZE;
    if ((total_bytes_read + MPL_FILE_READ_CHUNKSIZE) > max_filesize)
      max_bytes_to_read = max_filesize - total_bytes_read;

    bytes_read = fread(buf_pos_p, sizeof(char), max_bytes_to_read, fp);
    if (bytes_read > 0)
      total_bytes_read += bytes_read;

    if (feof(fp) || ferror(fp))
      break;

    if (total_bytes_read >= max_filesize)
    {
      MPL_DBG_TRACE_ERROR(E_MPL_FAILED_OPERATION,
                          ("Maximum allowed file size reached (%zu bytes)\n",
                           max_filesize - 1));/*lint !e557 zu is C99 */
      mpl_set_errno(E_MPL_FAILED_OPERATION);
      free(buf_p);
      return -1;
    }

    if ((buffer_size - total_bytes_read) < MPL_FILE_READ_CHUNKSIZE)
    {
      buffer_size += MPL_FILE_READ_CHUNKSIZE;
      buf_p = realloc(buf_p, buffer_size + 1); /* Allocate an extra byte
                                                  for zero termination */
      if (buf_p == NULL)
      {
        MPL_DBG_TRACE_ERROR(E_MPL_FAILED_ALLOCATING_MEMORY,
                            ("realloc buf_p\n"));
        mpl_set_errno(E_MPL_FAILED_ALLOCATING_MEMORY);
        return -1;
      }
      buf_pos_p = buf_p + total_bytes_read;
    }
    else
    {
      buf_pos_p += bytes_read;
    }
  }
  while (bytes_read > 0);

  if (ferror(fp))
  {
    MPL_DBG_TRACE_ERROR(E_MPL_FAILED_OPERATION,
                        ("fread returned %zu (trying to read %d bytes)\n",
                         bytes_read,
                         MPL_FILE_READ_CHUNKSIZE));/*lint !e557 zu is C99 */
    mpl_set_errno(E_MPL_FAILED_OPERATION);
    free(buf_p);
    return -1;
  }

  buf_p[total_bytes_read] = '\0';
  *buf_pp = buf_p;

  return (int)total_bytes_read;
}

/**
 * mpl_buffer_gets()
 **/
static int mpl_buffer_gets(char *dest_p, size_t maxline, char **src_pp)
{
  char *p;
  size_t len;
  size_t copylen;

  for (p = *src_pp, len = 0; (*p != '\n') && (*p != '\0'); p++, len++)
  {
    /* Nothing */
  }

  if (*p == '\n')
  {
    p++;
    len++;
  }

  copylen = len < (maxline-1) ? len : (maxline-1);
  strncpy(dest_p, *src_pp, copylen);
  dest_p[copylen] = '\0';

  *src_pp = p;

  if (copylen == len)
  {
    return (int)copylen;
  }
  else
  {
    MPL_DBG_TRACE_ERROR(E_MPL_FAILED_OPERATION,
                        ("Maximum allowed line length exceeded (%zu bytes)\n",
                         maxline - 1));/*lint !e557 zu is C99 */
    mpl_set_errno(E_MPL_FAILED_OPERATION);
    return -1;
  }
}


/**
 * mpl_buffer_get_line()
 **/
static int mpl_buffer_get_line(char *s,
                               size_t size,
                               char **file_pos_pp,
                               int *line)
{
  char *pos, *end;
  int res;

  while ((res = mpl_buffer_gets(s, size, file_pos_pp)) > 0) {
    (*line)++;
    s[size - 1] = '\0';
    pos = s;
    // Skip white space from the beginning of line.
    while(isspace(*pos))
      pos++;
    // Skip comment lines and empty lines
    if(*pos == '#')
      continue;

    // Remove # comments in the end of line
    end = strchr(pos,'#');
    if(end)
      *end-- = '\0';
    else
      end = pos + strlen(pos) - 1;

    // Remove trailing white space.
    while (end > pos && isspace(*end))
      *end-- = '\0';

    if (*pos == '\0')
      continue;

    return (int)strlen(s);
  }

  return res;
}
