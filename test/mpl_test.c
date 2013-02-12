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
 *   Author: Emil B. Viken <emil.b.viken@stericsson.com>
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <inttypes.h>
#include <unistd.h>
#include <wchar.h>
#include <pthread.h>
#ifndef MPL_OSE_TEST
#define wait dejagnu_wait
#include <dejagnu.h>
#undef wait
#endif
#include "mpl_test_msg.h"
#include "mpl_test_old_msg.h"
#include "mpl_dbgtrace.h"
#include "mpl_param.h"
#include "mpl_list.h"
#include "mpl_config.h"
#include "mpl_file.h"

#ifndef MPL_OSE_TEST
#define CONFIG_FILE tmpnam(NULL)
#define CONFIG_FILE_ADDITIONAL tmpnam(NULL)
#define BLACKLIST_FILE tmpnam(NULL)

#else

#define CONFIG_FILE "/afs/settings/oseril/myconfig"
#define CONFIG_FILE_ADDITIONAL "/afs/settings/oseril/myconfigadditional"
#define BLACKLIST_FILE "afs/settings/oseril/myblist"

static int numberOfPass = 0;
static int numberOfFail = 0;
#define pass(format, ...) \
    do {                  \
        printf("PASS: " format "\n", ##__VA_ARGS__);     \
        numberOfPass++;                         \
    } while(0)
#define fail(format, ...) \
    do {                  \
        printf("FAIL: " format "\n", ##__VA_ARGS__);     \
        numberOfFail++;                         \
    } while(0)
#define totals()                                                        \
    do {                                                                \
        printf("Totals: Passes(%d) Failures(%d)\n", numberOfPass, numberOfFail); \
        numberOfPass = 0;                                               \
        numberOfFail = 0;                                               \
    } while(0)

#define usleep(us) delay((us)/1000)
#endif

const int mpl_test_min = 1;
const int mpl_test_max = 87;

char *buf=NULL;
int buflen=0;
mpl_list_t *packmsg_p, *upackmsg_p;
mpl_param_element_t* unpackparam_p;
mpl_param_element_t packparam;

int pack_unpack_one(mpl_param_element_t *packparam_p, bool has_value, int param_set_id);


static void debug_print_params(mpl_list_t *param_list_p)
{
  int buflen;
  char *buf_p;
  mpl_pack_options_t options = MPL_PACK_OPTIONS_DEFAULT;
  options.message_delimiter = '\n';
  options.no_prefix = true;

  /* Calculate buffer space */
  buflen = mpl_param_list_pack_extended(param_list_p, NULL, 0, &options);
  if (buflen <= 0)
    return;

  buf_p = malloc((size_t)(buflen + 1));
  if (buf_p == NULL)
  {
    printf("Error malloc\n");
    return;
  }

  /* Pack list in to the buffer */
  buflen = mpl_param_list_pack_extended(param_list_p, buf_p, buflen + 1, &options);
  if (buflen <= 0)
  {
    free(buf_p);
    return;
  }

  printf("%s\n", buf_p);

  free(buf_p);
  return;
}

static int tc_pack_unpack_req_mycommand(void)
{
  int myint = 1000;
  int res;


  buf = NULL;
  buflen=0;
  packmsg_p = NULL;

  (void) mpl_param_list_add_enum(&packmsg_p, test_paramid_msgtype, test_msgtype_req);
  (void) mpl_add_param_to_list(&packmsg_p, test_paramid_myint, &myint);

  upackmsg_p = NULL;

  /* Negative */
  if(mpl_param_list_unpack(buf) != NULL)
  {
    printf("2: mpl_param_list_unpack() unexpectedly succeeded\n");
    free(buf);
    mpl_param_list_destroy(&packmsg_p);
    return -1;
  }

  /* Positive */
  buflen = mpl_param_list_pack(packmsg_p,buf,0);
  buf = malloc(buflen + 1);

  if(mpl_param_list_pack(packmsg_p,buf,buflen + 1) != buflen)
  {
    printf("3: mpl_param_list_pack() failed\n");
    if(buf != NULL)
      free(buf);
    mpl_param_list_destroy(&packmsg_p);
    return -1;
  }

  printf("buf = '%s', buflen = %zu\n", buf, (size_t)buflen);

  upackmsg_p = mpl_param_list_unpack(buf);

  if (NULL == upackmsg_p)
  {
    printf("4: mpl_param_list_unpack() failed\n");
    free(buf);
    mpl_param_list_destroy(&packmsg_p);
    return -1;
  }

  res = mpl_compare_param_lists(packmsg_p,upackmsg_p);

  free(buf);
  mpl_param_list_destroy(&upackmsg_p);
  mpl_param_list_destroy(&packmsg_p);
  return res;
}

static int tc_pack_unpack_resp_mycommand(void)
{
  int myint = 1000;
  int res;

  buf = NULL;
  buflen=0;
  packmsg_p = NULL;
  upackmsg_p = NULL;
  (void) mpl_param_list_add_enum(&packmsg_p, test_paramid_msgtype, test_msgtype_resp);
  (void) mpl_param_list_add_enum(&packmsg_p, test_paramid_result, test_result_ok);
  (void) mpl_add_param_to_list(&packmsg_p, test_paramid_myint, &myint);

  buflen = mpl_param_list_pack(packmsg_p, buf, 0);
  buf = malloc(buflen + 1);

  if(mpl_param_list_pack(packmsg_p,buf,buflen + 1) != buflen)
  {
    printf("mpl_param_list_pack() failed\n");
    if(buf != NULL)
      free(buf);
    mpl_param_list_destroy(&packmsg_p);
    return -1;
  }

  printf("buf = '%s', buflen = %zu\n", buf, (size_t)buflen);

  upackmsg_p = mpl_param_list_unpack(buf);

  if(upackmsg_p == NULL)
  {
    printf("mpl_param_list_unpack() failed\n");
    free(buf);
    mpl_param_list_destroy(&packmsg_p);
    return -1;
  }

  res = mpl_compare_param_lists(packmsg_p,upackmsg_p);

  free(buf);
  mpl_param_list_destroy(&upackmsg_p);
  mpl_param_list_destroy(&packmsg_p);
  return res;
}


static int tc_pack_unpack_event_myevent(void)
{
  int myint = 70;
  int res;

  buf = NULL;
  buflen=0;
  packmsg_p = NULL;
  upackmsg_p = NULL;
  (void) mpl_param_list_add_enum(&packmsg_p, test_paramid_msgtype, test_msgtype_event);
  (void) mpl_add_param_to_list(&packmsg_p, test_paramid_myint, &myint);

  buflen = mpl_param_list_pack(packmsg_p, buf, 0);
  buf = malloc(buflen + 1);

  if(mpl_param_list_pack(packmsg_p,buf,buflen + 1) != buflen)  {
    printf("mpl_param_list_pack() failed\n");
    if(buf != NULL)
      free(buf);
    mpl_param_list_destroy(&packmsg_p);
    return -1;
  }

  printf("buf = '%s', buflen = %zu\n", buf, (size_t)buflen);

  upackmsg_p = mpl_param_list_unpack(buf);

  if(upackmsg_p == NULL)
  {
    printf("mpl_param_list_unpack() failed\n");
    free(buf);
    mpl_param_list_destroy(&packmsg_p);
    return -1;
  }

  res = mpl_compare_param_lists(packmsg_p,upackmsg_p);

  free(buf);
  mpl_param_list_destroy(&upackmsg_p);
  mpl_param_list_destroy(&packmsg_p);
  return res;
}


static int tc_pack_unpack_buffer_too_small(void)
{
  char mybuf[10];
  int myint = 80;

  buf = mybuf;
  buflen = 9;
  packmsg_p = NULL;
  (void) mpl_param_list_add_enum(&packmsg_p, test_paramid_msgtype, test_msgtype_event);
  (void) mpl_add_param_to_list(&packmsg_p, test_paramid_myint, &myint);
  upackmsg_p = NULL;

  if(mpl_param_list_pack(packmsg_p,buf,buflen + 1) != buflen)  {
    printf("mpl_param_list_pack() expectedly failed\n");
    mpl_param_list_destroy(&packmsg_p);
    return 0;
  }

  printf("mpl_param_list_pack() unexpectedly succeeded\n");
  return -1;

}

static int tc_pack_unsupported_param_id(void)
{
  int myint = 1000;
  mpl_param_element_t *param_elem_p;

  buf = NULL;
  buflen=0;
  packmsg_p = NULL;
  (void) mpl_param_list_add_enum(&packmsg_p, test_paramid_msgtype, test_msgtype_req);
  (void) mpl_add_param_to_list(&packmsg_p, test_paramid_myint, &myint);

  param_elem_p = mpl_param_element_create_empty(test_enum_size_paramids);
  if (param_elem_p != NULL)
  {
    printf("mpl_param_element_create_empty() successful with unsupported param id\n");
    mpl_param_element_destroy(param_elem_p);
    mpl_param_list_destroy(&packmsg_p);
    return -1;
  }
  param_elem_p = malloc(sizeof(mpl_param_element_t));
  param_elem_p->id = test_enum_size_paramids;
  param_elem_p->value_p = NULL;
  param_elem_p->list_entry.next_p = NULL;
  mpl_list_add(&packmsg_p, &param_elem_p->list_entry);

  buflen = mpl_param_list_pack(packmsg_p,buf,0);
  buf = malloc(buflen + 1);

  if(mpl_param_list_pack(packmsg_p,buf,buflen + 1) != buflen)
  {
    printf("mpl_param_list_pack() successful with unsupported param id\n");
    if(buf != NULL)
      free(buf);
    mpl_param_list_destroy(&packmsg_p);
    return -1;
  }

  mpl_param_list_destroy(&packmsg_p);
  free(buf);
  return (0);
}

static int tc_unpack_with_unknown_parameter(void)
{
  char mybuf[] = "test.message_type=req,test.message=mycommand,test.mynt=3";

  upackmsg_p = NULL;

  upackmsg_p = mpl_param_list_unpack(mybuf);

  if(upackmsg_p != NULL)
  {
    printf("mpl_param_list_unpack() successful with unknown parameter\n");
    free(buf);
    mpl_param_list_destroy(&packmsg_p);
    return -1;
  }

  mpl_param_list_destroy(&upackmsg_p);
  return (0);
}


static int tc_pack_unpack_param_int(void)
{
  int myint = -5;
  int myint_copy = 0;
  int copy_size;
  int size;
  char* value_p;
  int res;

  /* Negative */
  packparam.id = test_enum_size_paramids;
  packparam.value_p = &myint;

  size=mpl_param_pack(&packparam, NULL, 0);
  if (size>0)
  {
    printf("mpl_param_pack() unexpectedly (1) succeeded\n");
    return (-1);
  }

  /* Negative 2 */
  packparam.id = mpl_param_first_paramid(TEST_PARAM_SET_ID) - 1;
  packparam.value_p = &myint;

  size=mpl_param_pack(&packparam, NULL, 0);
  if (size>0)
  {
    printf("mpl_param_pack() unexpectedly (2) succeeded\n");
    return (-1);
  }

  /* Negative 3 */
  packparam.id = test_paramid_myint + 0x00030000;
  packparam.value_p = &myint;

  size=mpl_param_pack(&packparam, NULL, 0);
  if (size>0)
  {
    printf("mpl_param_pack() unexpectedly (3) succeeded\n");
    return (-1);
  }

  /* Find out space need */
  packparam.id = test_paramid_myint;
  packparam.value_p = &myint;

  size=mpl_param_pack(&packparam, NULL, 0);
  if (size<0)
  {
    printf("mpl_param_pack() failed\n");
    return (-1);
  }

  /* Alloc size including '\0' */
  buf = malloc(size+1);

  size=mpl_param_pack(&packparam, buf, size+1);
  if (size<0)
  {
    printf("mpl_param_pack() failed\n");
    free(buf);
    return (-1);
  }

  printf("%s\n", buf);

  value_p = strstr(buf, "=");
  if (value_p == NULL)
  {
    printf("No '=' found in packet param\n");
    free(buf);
    return (-1);
  }
  *value_p = '\0';
  value_p++;

  if (mpl_param_unpack(buf, value_p, &unpackparam_p) < 0)
  {
    printf("mpl_param_unpack() failed\n");
    free(buf);
    return (-1);
  }

  free(buf);

  res = ((unpackparam_p->id == test_paramid_myint) &&
         (*(int*)unpackparam_p->value_p == myint));

  if (res != 1)
    return -1;

  if ((copy_size = mpl_param_value_copy_out(unpackparam_p, &myint_copy, 100)) < 0)
  {
    printf("mpl_param_value_copy_out() failed\n");
    mpl_param_element_destroy(unpackparam_p);
    return -1;
  }

  if (copy_size != sizeof(myint_copy))
  {
    printf("mpl_param_value_copy_out() failed to give correct size\n");
    mpl_param_element_destroy(unpackparam_p);
    return -1;
  }

  printf("myint_copy = %d, copy_size = %d\n", myint_copy, copy_size );

  if (myint_copy != *(int*)unpackparam_p->value_p)
  {
    printf("mpl_param_value_copy_out() failed to copy correctly\n");
    mpl_param_element_destroy(unpackparam_p);
    return -1;
  }

  if ((copy_size = mpl_param_value_copy_out(unpackparam_p, &myint_copy, 0)) != sizeof(myint_copy))
  {
    printf("mpl_param_value_copy_out() failed to give correct size\n");
    mpl_param_element_destroy(unpackparam_p);
    return -1;
  }

  mpl_param_element_destroy(unpackparam_p);


  /* Negative */
  if (mpl_param_unpack(buf, value_p, NULL) >= 0)
  {
    printf("mpl_param_unpack() unexpectedly (4) succeeded\n");
    free(buf);
    return (-1);
  }

  /* Negative: Max value check */
  if (mpl_param_unpack("test.myint", "2000", &unpackparam_p) >= 0)
  {
    printf("mpl_param_unpack() unexpectedly succeeded for max value check\n");
    mpl_param_element_destroy(unpackparam_p);
    return (-1);
  }

  /* Negative: Min value check */
  if (mpl_param_unpack("test.myint", "-600", &unpackparam_p) >= 0)
  {
    printf("mpl_param_unpack() unexpectedly succeeded for min value check\n");
    mpl_param_element_destroy(unpackparam_p);
    return (-1);
  }

  /* Negative: Max value check */
  if (mpl_param_unpack("test.myint1", "-1998", &unpackparam_p) >= 0)
  {
    printf("mpl_param_unpack() unexpectedly succeeded for max value check\n");
    mpl_param_element_destroy(unpackparam_p);
    return (-1);
  }

  /* Negative: Min value check */
  if (mpl_param_unpack("test.myint1", "-3000", &unpackparam_p) >= 0)
  {
    printf("mpl_param_unpack() unexpectedly succeeded for min value check\n");
    mpl_param_element_destroy(unpackparam_p);
    return (-1);
  }

  return 0;
}

static int
tc_pack_unpack_param_uint8(void)
{
  uint8_t myuint8=0x09;
  uint8_t myuint8_copy = 0;
  int copy_size;
  int size;
  char* value_p;
  mpl_param_element_t* clone_p;
  uint8_t myuint8_2;
  mpl_param_element_t *par;

  packparam.id = test_paramid_myuint8;
  packparam.value_p = &myuint8;

  /* Find out space need */
  size=mpl_param_pack(&packparam, NULL, 0);
  if (size<0)
  {
    printf("mpl_param_pack() failed\n");
    return (-1);
  }

  /* Alloc size including '\0' */
  buf = malloc(size+1);

  size=mpl_param_pack(&packparam, buf, size+1);
  if (size<0)
  {
    printf("mpl_param_pack() failed\n");
    free(buf);
    return (-1);
  }

  value_p = strstr(buf, "=");
  if (value_p == NULL)
  {
    printf("No '=' found in packet param\n");
    free(buf);
    return (-1);
  }
  *value_p = '\0';
  value_p++;

  if (mpl_param_unpack(buf, value_p, &unpackparam_p) < 0)
  {
    printf("mpl_param_unpack() failed\n");
    free(buf);
    return (-1);
  }

  free(buf);

  if ((unpackparam_p->id != packparam.id) ||
      (*(uint8_t*)unpackparam_p->value_p != myuint8))
  {
    mpl_param_element_destroy(unpackparam_p);
    return -1;
  }


  clone_p = mpl_param_element_clone(unpackparam_p);
  if (NULL == clone_p)
  {
    printf("mpl_param_element_clone() failed\n");
    mpl_param_element_destroy(unpackparam_p);
    return (-1);
  }

  if (*(uint8_t*)clone_p->value_p != *(uint8_t*)unpackparam_p->value_p)
  {
    printf("mpl_param_element_clone() failed to clone correctly\n");
    mpl_param_element_destroy(clone_p);
    mpl_param_element_destroy(unpackparam_p);
    return (-1);
  }

  mpl_param_element_destroy(clone_p);

  if ((copy_size = mpl_param_value_copy_out(unpackparam_p, &myuint8_copy, 100)) < 0)
  {
    printf("mpl_param_value_copy_out() failed\n");
    mpl_param_element_destroy(unpackparam_p);
    return -1;
  }

  if (copy_size != sizeof(myuint8_copy))
  {
    printf("mpl_param_value_copy_out() failed to give correct size\n");
    mpl_param_element_destroy(unpackparam_p);
    return -1;
  }

  printf("myuint8_copy = %u, copy_size = %d\n", myuint8_copy, copy_size );

  if (myuint8_copy != *(uint8_t*)unpackparam_p->value_p)
  {
    printf("mpl_param_value_copy_out() failed to copy correctly\n");
    mpl_param_element_destroy(unpackparam_p);
    return -1;
  }

  if ((copy_size = mpl_param_value_copy_out(unpackparam_p, &myuint8_copy, 0)) != sizeof(myuint8_copy))
  {
    printf("mpl_param_value_copy_out() failed to give correct size\n");
    mpl_param_element_destroy(unpackparam_p);
    return -1;
  }

  mpl_param_element_destroy(unpackparam_p);

  /* Negative: Out of range */
  if (mpl_param_unpack("test.myuint8", "0x100", &unpackparam_p) >= 0)
  {
    printf("mpl_param_unpack() unexpectedly succeeded for out of range\n");
    mpl_param_element_destroy(unpackparam_p);
    return (-1);
  }

  /* Negative: Max value check */
  if (mpl_param_unpack("test.myuint8", "0x40", &unpackparam_p) >= 0)
  {
    printf("mpl_param_unpack() unexpectedly succeeded for max value check\n");
    mpl_param_element_destroy(unpackparam_p);
    return (-1);
  }

  /* Negative: Min value check */
  if (mpl_param_unpack("test.myuint8", "0x4", &unpackparam_p) >= 0)
  {
    printf("mpl_param_unpack() unexpectedly succeeded for min value check\n");
    mpl_param_element_destroy(unpackparam_p);
    return (-1);
  }

  /* Negative: Non integer */
  if (mpl_param_unpack("test.myuint8", "test", &unpackparam_p) >= 0)
  {
    printf("mpl_param_unpack() unexpectedly succeeded for Non integer\n");
    mpl_param_element_destroy(unpackparam_p);
    return (-1);
  }

  /* Out of range */
  if (mpl_param_unpack("test.myuint8", "330", &unpackparam_p) >= 0)
  {
    printf("mpl_param_unpack() unexpectedly succeeded for myuint8=%s\n", "330");
    return (-1);
  }

  /* > max */
  if (mpl_param_unpack("test.myuint8", "65", &unpackparam_p) >= 0)
  {
    printf("mpl_param_unpack() unexpectedly succeeded for myuint8=%s\n", "65");
    return (-1);
  }

  /* MIN */
  myuint8_2 = 0;
  par = mpl_param_element_create(test_paramid_myuint8_2, &myuint8_2);
  if (NULL == par)
  {
      printf("mpl_param_element_create(myuint8_2) failed\n");
      return -1;
  }
  if (pack_unpack_one(par, true, TEST_PARAM_SET_ID) < 0)
      return -1;

  mpl_param_element_destroy(par);

  /* MAX */
  myuint8_2 = UINT8_MAX;
  par = mpl_param_element_create(test_paramid_myuint8_2, &myuint8_2);
  if (NULL == par)
  {
      printf("mpl_param_element_create(myuint8_2) failed\n");
      return -1;
  }
  if (pack_unpack_one(par, true, TEST_PARAM_SET_ID) < 0)
      return -1;

  mpl_param_element_destroy(par);

  return 0;
}

static int
tc_pack_unpack_param_uint16(void)
{
  uint16_t myuint16=10000;
  uint16_t myuint16_copy = 0;
  int size;
  int copy_size;
  char* value_p;
  mpl_param_element_t* clone_p;
  uint16_t myuint16_2;
  mpl_param_element_t *par;

  packparam.id = test_paramid_myuint16;
  packparam.value_p = &myuint16;

  /* Find out space need */
  size=mpl_param_pack(&packparam, NULL, 0);
  if (size<0)
  {
    printf("mpl_param_pack() failed\n");
    return (-1);
  }

  /* Alloc size including '\0' */
  buf = malloc(size+1);

  size=mpl_param_pack(&packparam, buf, size+1);
  if (size<0)
  {
    printf("mpl_param_pack() failed\n");
    free(buf);
    return (-1);
  }

  value_p = strstr(buf, "=");
  if (value_p == NULL)
  {
    printf("No '=' found in packet param\n");
    free(buf);
    return (-1);
  }
  *value_p = '\0';
  value_p++;

  if (mpl_param_unpack(buf, value_p, &unpackparam_p) < 0)
  {
    printf("mpl_param_unpack() failed\n");
    free(buf);
    return (-1);
  }

  free(buf);

  if ((unpackparam_p->id != packparam.id) ||
      (*(uint16_t*)unpackparam_p->value_p != myuint16))
  {
    mpl_param_element_destroy(unpackparam_p);
    return -1;
  }


  clone_p = mpl_param_element_clone(unpackparam_p);
  if (NULL == clone_p)
  {
    printf("mpl_param_element_clone() failed\n");
    mpl_param_element_destroy(unpackparam_p);
    return (-1);
  }

  if (*(uint16_t*)clone_p->value_p != *(uint16_t*)unpackparam_p->value_p)
  {
    printf("mpl_param_element_clone() failed to clone correctly\n");
    mpl_param_element_destroy(clone_p);
    mpl_param_element_destroy(unpackparam_p);
    return (-1);
  }

  mpl_param_element_destroy(clone_p);

  if ((copy_size = mpl_param_value_copy_out(unpackparam_p, &myuint16_copy, 100)) < 0)
  {
    printf("mpl_param_value_copy_out() failed\n");
    mpl_param_element_destroy(unpackparam_p);
    return -1;
  }

  if (copy_size != sizeof(myuint16_copy))
  {
    printf("mpl_param_value_copy_out() failed to give correct size\n");
    mpl_param_element_destroy(unpackparam_p);
    return -1;
  }

  //printf("myuint16_copy = %u, copy_size = %u\n", myuint16_copy, copy_size );

  if (myuint16_copy != *(uint16_t*)unpackparam_p->value_p)
  {
    printf("mpl_param_value_copy_out() failed to copy correctly\n");
    mpl_param_element_destroy(unpackparam_p);
    return -1;
  }

  if ((copy_size = mpl_param_value_copy_out(unpackparam_p, &myuint16_copy, 1)) != sizeof(myuint16_copy))
  {
    printf("mpl_param_value_copy_out() failed to give correct size\n");
    mpl_param_element_destroy(unpackparam_p);
    return -1;
  }

  mpl_param_element_destroy(unpackparam_p);

  /* Negative: Out of range */
  if (mpl_param_unpack("test.myuint16", "400000", &unpackparam_p) >= 0)
  {
    printf("mpl_param_unpack() unexpectedly succeeded for out of range\n");
    mpl_param_element_destroy(unpackparam_p);
    return (-1);
  }

  /* Negative: Max value check */
  if (mpl_param_unpack("test.myuint16", "40000", &unpackparam_p) >= 0)
  {
    printf("mpl_param_unpack() unexpectedly succeeded for max value check\n");
    mpl_param_element_destroy(unpackparam_p);
    return (-1);
  }

  /* Negative: Non integer */
  if (mpl_param_unpack("test.myuint16", "test", &unpackparam_p) >= 0)
  {
    printf("mpl_param_unpack() unexpectedly succeeded for Non integer\n");
    mpl_param_element_destroy(unpackparam_p);
    return (-1);
  }

  /* MIN */
  myuint16_2 = 0;
  par = mpl_param_element_create(test_paramid_myuint16_2, &myuint16_2);
  if (NULL == par)
  {
      printf("mpl_param_element_create(myuint16_2) failed\n");
      return -1;
  }
  if (pack_unpack_one(par, true, TEST_PARAM_SET_ID) < 0)
      return -1;

  mpl_param_element_destroy(par);

  /* MAX */
  myuint16_2 = UINT16_MAX;
  par = mpl_param_element_create(test_paramid_myuint16_2, &myuint16_2);
  if (NULL == par)
  {
      printf("mpl_param_element_create(myuint16_2) failed\n");
      return -1;
  }
  if (pack_unpack_one(par, true, TEST_PARAM_SET_ID) < 0)
      return -1;

  mpl_param_element_destroy(par);

  return 0;
}

static int
tc_pack_unpack_param_uint32(void)
{
  uint32_t myuint32 = 2000;
  uint32_t myuint32_copy = 0;
  int copy_size;
  int size;
  char* value_p;
  mpl_param_element_t* clone_p;
  uint32_t myuint32_2;
  mpl_param_element_t *par;

  packparam.id = test_paramid_myuint32;
  packparam.value_p = &myuint32;

  /* Find out space need */
  size=mpl_param_pack(&packparam, NULL, 0);
  if (size<0)
  {
    printf("mpl_param_pack() failed\n");
    return (-1);
  }

  /* Alloc size including '\0' */
  buf = malloc(size+1);

  size=mpl_param_pack(&packparam, buf, size+1);
  if (size<0)
  {
    printf("mpl_param_pack() failed\n");
    free(buf);
    return (-1);
  }

  value_p = strstr(buf, "=");
  if (value_p == NULL)
  {
    printf("No '=' found in packet param\n");
    free(buf);
    return (-1);
  }
  *value_p = '\0';
  value_p++;

  if (mpl_param_unpack(buf, value_p, &unpackparam_p) < 0)
  {
    printf("mpl_param_unpack() failed\n");
    free(buf);
    return (-1);
  }

  free(buf);

  if ((unpackparam_p->id != packparam.id) ||
      (*(uint32_t*)unpackparam_p->value_p != myuint32))
  {
    mpl_param_element_destroy(unpackparam_p);
    return -1;
  }

  clone_p = mpl_param_element_clone(unpackparam_p);
  if (NULL == clone_p)
  {
    printf("mpl_param_element_clone() failed\n");
    mpl_param_element_destroy(unpackparam_p);
    return (-1);
  }

  if (*(uint32_t*)clone_p->value_p != *(uint32_t*)unpackparam_p->value_p)
  {
    printf("mpl_param_element_clone() failed to clone correctly\n");
    mpl_param_element_destroy(clone_p);
    mpl_param_element_destroy(unpackparam_p);
    return (-1);
  }

  mpl_param_element_destroy(clone_p);

  if ((copy_size = mpl_param_value_copy_out(unpackparam_p, &myuint32_copy, 100)) < 0)
  {
    printf("mpl_param_value_copy_out() failed\n");
    mpl_param_element_destroy(unpackparam_p);
    return -1;
  }

  if (copy_size != sizeof(myuint32_copy))
  {
    printf("mpl_param_value_copy_out() failed to give correct size\n");
    mpl_param_element_destroy(unpackparam_p);
    return -1;
  }

  printf("myuint32_copy = %u, copy_size = %u\n", myuint32_copy, copy_size );

  if (myuint32_copy != *(uint32_t*)unpackparam_p->value_p)
  {
    printf("mpl_param_value_copy_out() failed to copy correctly\n");
    mpl_param_element_destroy(unpackparam_p);
    return -1;
  }

  if ((copy_size = mpl_param_value_copy_out(unpackparam_p, &myuint32_copy, 0)) != sizeof(myuint32_copy))
  {
    printf("mpl_param_value_copy_out() failed to give correct size\n");
    mpl_param_element_destroy(unpackparam_p);
    return -1;
  }

  mpl_param_element_destroy(unpackparam_p);

  /* Negative: Out of range */
  if (mpl_param_unpack("test.myuint32", "0x1ffffffff", &unpackparam_p) >= 0)
  {
    printf("mpl_param_unpack() unexpectedly succeeded for out of range: %x\n",
           *(uint32_t*)unpackparam_p->value_p);
    mpl_param_element_destroy(unpackparam_p);
    return (-1);
  }

  /* > max */
  if (mpl_param_unpack("test.myuint32", "6000", &unpackparam_p) >= 0)
  {
    printf("mpl_param_unpack() unexpectedly succeeded for myuint32=%s\n", "6000");
    return (-1);
  }

  /* Negative: Non integer */
  if (mpl_param_unpack("test.myuint32", "test", &unpackparam_p) >= 0)
  {
    printf("mpl_param_unpack() unexpectedly succeeded for Non integer\n");
    mpl_param_element_destroy(unpackparam_p);
    return (-1);
  }

  /* MIN */
  myuint32_2 = 0;
  par = mpl_param_element_create(test_paramid_myuint32_2, &myuint32_2);
  if (NULL == par)
  {
      printf("mpl_param_element_create(myuint32_2) failed\n");
      return -1;
  }
  if (pack_unpack_one(par, true, TEST_PARAM_SET_ID) < 0)
      return -1;

  mpl_param_element_destroy(par);

  /* MAX */
  myuint32_2 = UINT32_MAX;
  par = mpl_param_element_create(test_paramid_myuint32_2, &myuint32_2);
  if (NULL == par)
  {
      printf("mpl_param_element_create(myuint32_2) failed\n");
      return -1;
  }
  if (pack_unpack_one(par, true, TEST_PARAM_SET_ID) < 0)
      return -1;

  mpl_param_element_destroy(par);

  return 0;
}

static int
tc_pack_unpack_param_uint64(void)
{
  uint64_t myuint64 = 2000;
  uint64_t myuint64_copy = 0;
  int copy_size;
  int size;
  char* value_p;
  mpl_param_element_t* clone_p;
  uint64_t myuint64_2;
  mpl_param_element_t *par;

  packparam.id = test_paramid_myuint64;
  packparam.value_p = &myuint64;

  /* Find out space need */
  size=mpl_param_pack(&packparam, NULL, 0);
  if (size<0)
  {
    printf("mpl_param_pack() failed\n");
    return (-1);
  }

  /* Alloc size including '\0' */
  buf = malloc(size+1);

  size=mpl_param_pack(&packparam, buf, size+1);
  if (size<0)
  {
    printf("mpl_param_pack() failed\n");
    free(buf);
    return (-1);
  }

  value_p = strstr(buf, "=");
  if (value_p == NULL)
  {
    printf("No '=' found in packet param\n");
    free(buf);
    return (-1);
  }
  *value_p = '\0';
  value_p++;

  if (mpl_param_unpack(buf, value_p, &unpackparam_p) < 0)
  {
    printf("mpl_param_unpack() failed\n");
    free(buf);
    return (-1);
  }

  free(buf);

  if ((unpackparam_p->id != packparam.id) ||
      (*(uint64_t*)unpackparam_p->value_p != myuint64))
  {
    mpl_param_element_destroy(unpackparam_p);
    return -1;
  }

  clone_p = mpl_param_element_clone(unpackparam_p);
  if (NULL == clone_p)
  {
    printf("mpl_param_element_clone() failed\n");
    mpl_param_element_destroy(unpackparam_p);
    return (-1);
  }

  if (*(uint64_t*)clone_p->value_p != *(uint64_t*)unpackparam_p->value_p)
  {
    printf("mpl_param_element_clone() failed to clone correctly\n");
    mpl_param_element_destroy(clone_p);
    mpl_param_element_destroy(unpackparam_p);
    return (-1);
  }

  mpl_param_element_destroy(clone_p);

  if ((copy_size = mpl_param_value_copy_out(unpackparam_p, &myuint64_copy, 100)) < 0)
  {
    printf("mpl_param_value_copy_out() failed\n");
    mpl_param_element_destroy(unpackparam_p);
    return -1;
  }

  if (copy_size != sizeof(myuint64_copy))
  {
    printf("mpl_param_value_copy_out() failed to give correct size\n");
    mpl_param_element_destroy(unpackparam_p);
    return -1;
  }

  printf("myuint64_copy = %" PRIu64 ", copy_size = %d\n", myuint64_copy, copy_size );

  if (myuint64_copy != *(uint64_t*)unpackparam_p->value_p)
  {
    printf("mpl_param_value_copy_out() failed to copy correctly\n");
    mpl_param_element_destroy(unpackparam_p);
    return -1;
  }

  if ((copy_size = mpl_param_value_copy_out(unpackparam_p, &myuint64_copy, 0)) != sizeof(myuint64_copy))
  {
    printf("mpl_param_value_copy_out() failed to give correct size\n");
    mpl_param_element_destroy(unpackparam_p);
    return -1;
  }

  mpl_param_element_destroy(unpackparam_p);

  /* Negative: Out of range */
  if (mpl_param_unpack("test.myuint64", "0x1ffffffff", &unpackparam_p) >= 0)
  {
    printf("mpl_param_unpack() unexpectedly succeeded for out of range: %" PRIu64 "\n",
           *(uint64_t*)unpackparam_p->value_p);
    mpl_param_element_destroy(unpackparam_p);
    return (-1);
  }

  /* > max */
  if (mpl_param_unpack("test.myuint64", "6000", &unpackparam_p) >= 0)
  {
    printf("mpl_param_unpack() unexpectedly succeeded for myuint64=%s\n", "6000");
    return (-1);
  }

  /* < min */
  if (mpl_param_unpack("test.myuint64", "4", &unpackparam_p) >= 0)
  {
    printf("mpl_param_unpack() unexpectedly succeeded for myuint64=%s\n", "4");
    return (-1);
  }

  /* Negative: Non integer */
  if (mpl_param_unpack("test.myuint64", "test", &unpackparam_p) >= 0)
  {
    printf("mpl_param_unpack() unexpectedly succeeded for Non integer\n");
    mpl_param_element_destroy(unpackparam_p);
    return (-1);
  }

  /* MIN */
  myuint64_2 = 0;
  par = mpl_param_element_create(test_paramid_myuint64_2, &myuint64_2);
  if (NULL == par)
  {
      printf("mpl_param_element_create(myuint64_2) failed\n");
      return -1;
  }
  if (pack_unpack_one(par, true, TEST_PARAM_SET_ID) < 0)
      return -1;

  mpl_param_element_destroy(par);

  /* MAX */
  myuint64_2 = UINT64_MAX;
  par = mpl_param_element_create(test_paramid_myuint64_2, &myuint64_2);
  if (NULL == par)
  {
      printf("mpl_param_element_create(myuint64_2) failed\n");
      return -1;
  }
  if (pack_unpack_one(par, true, TEST_PARAM_SET_ID) < 0)
      return -1;

  mpl_param_element_destroy(par);
  return 0;
}

static int
tc_pack_unpack_param_sint64(void)
{
  int64_t mysint64 = 2000;
  int64_t mysint64_2;
  int64_t mysint64_copy = 0;
  int copy_size;
  int size;
  char* value_p;
  mpl_param_element_t* clone_p;
  mpl_param_element_t *par;

  packparam.id = test_paramid_mysint64;
  packparam.value_p = &mysint64;

  /* Find out space need */
  size=mpl_param_pack(&packparam, NULL, 0);
  if (size<0)
  {
    printf("mpl_param_pack() failed\n");
    return (-1);
  }

  /* Alloc size including '\0' */
  buf = malloc(size+1);

  size=mpl_param_pack(&packparam, buf, size+1);
  if (size<0)
  {
    printf("mpl_param_pack() failed\n");
    free(buf);
    return (-1);
  }

  value_p = strstr(buf, "=");
  if (value_p == NULL)
  {
    printf("No '=' found in packet param\n");
    free(buf);
    return (-1);
  }
  *value_p = '\0';
  value_p++;

  if (mpl_param_unpack(buf, value_p, &unpackparam_p) < 0)
  {
    printf("mpl_param_unpack() failed\n");
    free(buf);
    return (-1);
  }

  free(buf);

  if ((unpackparam_p->id != packparam.id) ||
      (*(int64_t*)unpackparam_p->value_p != mysint64))
  {
    mpl_param_element_destroy(unpackparam_p);
    return -1;
  }

  clone_p = mpl_param_element_clone(unpackparam_p);
  if (NULL == clone_p)
  {
    printf("mpl_param_element_clone() failed\n");
    mpl_param_element_destroy(unpackparam_p);
    return (-1);
  }

  if (*(int64_t*)clone_p->value_p != *(int64_t*)unpackparam_p->value_p)
  {
    printf("mpl_param_element_clone() failed to clone correctly\n");
    mpl_param_element_destroy(clone_p);
    mpl_param_element_destroy(unpackparam_p);
    return (-1);
  }

  mpl_param_element_destroy(clone_p);

  if ((copy_size = mpl_param_value_copy_out(unpackparam_p, &mysint64_copy, 100)) < 0)
  {
    printf("mpl_param_value_copy_out() failed\n");
    mpl_param_element_destroy(unpackparam_p);
    return -1;
  }

  if (copy_size != sizeof(mysint64_copy))
  {
    printf("mpl_param_value_copy_out() failed to give correct size\n");
    mpl_param_element_destroy(unpackparam_p);
    return -1;
  }

  printf("mysint64_copy = %" PRIu64 ", copy_size = %d\n", mysint64_copy, copy_size );

  if (mysint64_copy != *(int64_t*)unpackparam_p->value_p)
  {
    printf("mpl_param_value_copy_out() failed to copy correctly\n");
    mpl_param_element_destroy(unpackparam_p);
    return -1;
  }

  if ((copy_size = mpl_param_value_copy_out(unpackparam_p, &mysint64_copy, 0)) != sizeof(mysint64_copy))
  {
    printf("mpl_param_value_copy_out() failed to give correct size\n");
    mpl_param_element_destroy(unpackparam_p);
    return -1;
  }

  mpl_param_element_destroy(unpackparam_p);

  /* Negative: Out of range */
  if (mpl_param_unpack("test.mysint64", "0x1ffffffff", &unpackparam_p) >= 0)
  {
    printf("mpl_param_unpack() unexpectedly succeeded for out of range: %" PRIu64 "\n",
           *(int64_t*)unpackparam_p->value_p);
    mpl_param_element_destroy(unpackparam_p);
    return (-1);
  }

  /* > max */
  if (mpl_param_unpack("test.mysint64", "6000", &unpackparam_p) >= 0)
  {
    printf("mpl_param_unpack() unexpectedly succeeded for mysint64=%s\n", "6000");
    return (-1);
  }

  /* < min */
  if (mpl_param_unpack("test.mysint64", "4", &unpackparam_p) >= 0)
  {
    printf("mpl_param_unpack() unexpectedly succeeded for mysint64=%s\n", "4");
    return (-1);
  }

  /* Negative: Non integer */
  if (mpl_param_unpack("test.mysint64", "test", &unpackparam_p) >= 0)
  {
    printf("mpl_param_unpack() unexpectedly succeeded for Non integer\n");
    mpl_param_element_destroy(unpackparam_p);
    return (-1);
  }

  /* Negative value */
  mysint64_2 = -33;
  par = mpl_param_element_create(test_paramid_mysint64_2, &mysint64_2);
  if (NULL == par)
  {
      printf("mpl_param_element_create(mysint64_2) failed\n");
      return -1;
  }
  if (pack_unpack_one(par, true, TEST_PARAM_SET_ID) < 0)
      return -1;

  mpl_param_element_destroy(par);

  /* MIN */
  mysint64_2 = INT64_MIN;
  par = mpl_param_element_create(test_paramid_mysint64_2, &mysint64_2);
  if (NULL == par)
  {
      printf("mpl_param_element_create(mysint64_2) failed\n");
      return -1;
  }
  if (pack_unpack_one(par, true, TEST_PARAM_SET_ID) < 0)
      return -1;

  mpl_param_element_destroy(par);

  /* MAX */
  mysint64_2 = INT64_MAX;
  par = mpl_param_element_create(test_paramid_mysint64_2, &mysint64_2);
  if (NULL == par)
  {
      printf("mpl_param_element_create(mysint64_2) failed\n");
      return -1;
  }
  if (pack_unpack_one(par, true, TEST_PARAM_SET_ID) < 0)
      return -1;

  mpl_param_element_destroy(par);

  return 0;
}

static int tc_pack_unpack_param_string(void)
{
  char mystring[] = "myapn.com";
  char mystring_copy[10] = {0};
  int copy_size;
  int size;
  char* value_p;

  packparam.id = test_paramid_mystring;
  packparam.value_p = mystring;

  /* Negative */
  size=mpl_param_pack(NULL, NULL, 0);
  if (size >= 0)
  {
    printf("mpl_param_pack() unexpectedly succeeded\n");
    return (-1);
  }

  packparam.id = test_enum_size_paramids;
  size=mpl_param_pack(&packparam, NULL, 0);
  if (size >= 0)
  {
    printf("mpl_param_pack() unexpectedly succeeded\n");
    return (-1);
  }

  packparam.id = mpl_param_first_paramid(TEST_PARAM_SET_ID) - 1;
  size=mpl_param_pack(&packparam, NULL, 0);
  if (size >= 0)
  {
    printf("mpl_param_pack() unexpectedly succeeded\n");
    return (-1);
  }

  packparam.id = test_paramid_mystring;

  /* Find out space need */
  size=mpl_param_pack(&packparam, NULL, 0);
  if (size<0)
  {
    printf("mpl_param_pack() failed\n");
    return (-1);
  }

  /* Alloc size including '\0' */
  buf = malloc(size+1);

  size=mpl_param_pack(&packparam, buf, size+1);
  if (size<0)
  {
    printf("mpl_param_pack() failed\n");
    free(buf);
    return (-1);
  }

  value_p = strstr(buf, "=");
  if (value_p == NULL)
  {
    printf("No '=' found in packet param\n");
    free(buf);
    return (-1);
  }
  *value_p = '\0';
  value_p++;

  if (mpl_param_unpack(buf, value_p, &unpackparam_p) < 0)
  {
    printf("mpl_param_unpack() failed\n");
    free(buf);
    return (-1);
  }

  free(buf);

  if ((unpackparam_p->id != test_paramid_mystring) ||
      (strcmp((char*)unpackparam_p->value_p, mystring)!=0))
  {
    mpl_param_element_destroy(unpackparam_p);
    return -1;
  }

  if ((copy_size = mpl_param_value_copy_out(unpackparam_p, &mystring_copy, 100)) < 0)
  {
    printf("mpl_param_value_copy_out() failed\n");
    mpl_param_element_destroy(unpackparam_p);
    return -1;
  }

  if (copy_size != sizeof(mystring_copy))
  {
    printf("mpl_param_value_copy_out() failed to give correct size\n");
    mpl_param_element_destroy(unpackparam_p);
    return -1;
  }

  printf("mystring_copy = %s, copy_size = %d\n", mystring_copy, copy_size );

  if (strcmp(mystring_copy, (char*) unpackparam_p->value_p))
  {
    printf("mpl_param_value_copy_out() failed to copy correctly\n");
    mpl_param_element_destroy(unpackparam_p);
    return -1;
  }

  if ((copy_size = mpl_param_value_copy_out(unpackparam_p, &mystring_copy, 3)) != sizeof(mystring_copy))
  {
    printf("mpl_param_value_copy_out() failed to give correct size\n");
    mpl_param_element_destroy(unpackparam_p);
    return -1;
  }

  mpl_param_element_destroy(unpackparam_p);

  /* Negative: Max strlen */
  if (mpl_param_unpack("test.mystring",
                           "1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890",
                           &unpackparam_p) >= 0)
  {
    printf("mpl_param_unpack() unexpectedly succeeded for max strlen check\n");
    mpl_param_element_destroy(unpackparam_p);
    return (-1);
  }

  /* Negative: Min strlen */
  if (mpl_param_unpack("test.mystring",
                           "123",
                           &unpackparam_p) >= 0)
  {
    printf("mpl_param_unpack() unexpectedly succeeded for min strlen check\n");
    mpl_param_element_destroy(unpackparam_p);
    return (-1);
  }

  return 0;
}

static int tc_pack_unpack_param_wstring(void)
{
  wchar_t mywstring[] = L"myapn.com";
  wchar_t mywstring_copy[10];
  wchar_t mywstring_long[] = L"1234567890123456789012";
  int copy_size;
  int size;
  char* value_p;
  mpl_param_element_t* cloneparam_p;
  mpl_param_element_t* elem_p;


  printf("sizeof(wchar_t) = %zu\n", sizeof(wchar_t));

  packparam.id = test_paramid_mywstring;
  packparam.value_p = mywstring;

  /* Negative */
  size=mpl_param_pack(NULL, NULL, 0);
  if (size >= 0)
  {
    printf("mpl_param_pack() unexpectedly succeeded\n");
    return (-1);
  }

  packparam.value_p = mywstring;
  packparam.id = test_enum_size_paramids;
  size=mpl_param_pack(&packparam, NULL, 0);
  if (size >= 0)
  {
    printf("mpl_param_pack() unexpectedly succeeded\n");
    return (-1);
  }

  packparam.id = mpl_param_first_paramid(TEST_PARAM_SET_ID) - 1;
  size=mpl_param_pack(&packparam, NULL, 0);
  if (size >= 0)
  {
    printf("mpl_param_pack() unexpectedly succeeded\n");
    return (-1);
  }

  packparam.id = test_paramid_mywstring;

  /* Find out space need */
  size=mpl_param_pack(&packparam, NULL, 0);
  if (size<0)
  {
    printf("mpl_param_pack() failed\n");
    return (-1);
  }

  printf("size = %d\n", size);

  /* Alloc size including '\0' */
  buf = malloc(size+1);

  size=mpl_param_pack(&packparam, buf, size+1);
  if (size<0)
  {
    printf("mpl_param_pack() failed\n");
    free(buf);
    return (-1);
  }

  printf("buf = '%s', buflen = %d\n", buf, size);

  value_p = strstr(buf, "=");
  if (value_p == NULL)
  {
    printf("No '=' found in packet param\n");
    free(buf);
    return (-1);
  }
  *value_p = '\0';
  value_p++;

  if (mpl_param_unpack(buf, value_p, &unpackparam_p) < 0)
  {
    printf("mpl_param_unpack() failed\n");
    free(buf);
    return (-1);
  }

  free(buf);

  if (unpackparam_p->id != test_paramid_mywstring)
  {
    printf("mpl_param_unpack(): param id's not equal\n");
    mpl_param_element_destroy(unpackparam_p);
    return -1;
  }

  if (wcscmp((wchar_t*)unpackparam_p->value_p, mywstring)!=0)
  {
    printf("mpl_param_unpack(): wstrings not equal\n");
    mpl_param_element_destroy(unpackparam_p);
    return -1;
  }


  mywstring[5] = L'\0';

  if (0 == mpl_param_element_compare(&packparam, unpackparam_p))
  {
    printf("unexpected param %d value match\n", packparam.id);
    free(buf);
    return -1;
  }
  printf("compare len\n");

  mywstring[5] = L'.';
  mywstring[3] = L'b';

  if (0 == mpl_param_element_compare(&packparam, unpackparam_p))
  {
    printf("unexpected param %d value match\n", packparam.id);
    free(buf);
    return -1;
  }
  printf("compare val\n");

  mywstring[3] = L'p';

  cloneparam_p = mpl_param_element_clone(&packparam);

  if (0 != mpl_param_element_compare(&packparam, cloneparam_p))
  {
    printf("param %d value mismatch\n", packparam.id);
    free(buf);
    mpl_param_element_destroy(unpackparam_p);
    mpl_param_element_destroy(cloneparam_p);
    return -1;
  }
  printf("compare clone\n");

  mpl_param_element_destroy(cloneparam_p);

  if ((copy_size = mpl_param_value_copy_out(unpackparam_p, mywstring_copy, 100)) < 0)
  {
    printf("mpl_param_value_copy_out() failed\n");
    mpl_param_element_destroy(unpackparam_p);
    return -1;
  }

  printf("copy_size = %d\n", copy_size);

  if (copy_size != 10*sizeof(L'\0'))
  {
    printf("mpl_param_value_copy_out() failed to give correct size\n");
    mpl_param_element_destroy(unpackparam_p);
    return -1;
  }

  if (wcscmp(mywstring_copy, (wchar_t*) unpackparam_p->value_p))
  {
    printf("mpl_param_value_copy_out() failed to copy correctly\n");
    mpl_param_element_destroy(unpackparam_p);
    return -1;
  }

  printf("compare copy\n");

  if ((copy_size = mpl_param_value_copy_out(unpackparam_p, mywstring_copy, 3)) != 10*sizeof(L'\0'))
  {
    printf("mpl_param_value_copy_out() failed to give correct size\n");
    mpl_param_element_destroy(unpackparam_p);
    return -1;
  }

  elem_p = mpl_param_element_create_wstringn(test_paramid_mywstring, L"Hello", 20);

  if (wcscmp(L"Hello", (wchar_t*) elem_p->value_p))
  {
    printf("mpl_param_element_create_wstringn() compare failed\n");
    mpl_param_element_destroy(unpackparam_p);
    return -1;
  }

  printf("mpl_param_element_create_wstringn()\n");

  mpl_param_element_destroy(unpackparam_p);
  mpl_param_element_destroy(elem_p);


/* Test max len */
  packparam.id = test_paramid_mywstring;
  packparam.value_p = mywstring_long;

  /* Find out space need */
  size=mpl_param_pack(&packparam, NULL, 0);
  if (size<0)
  {
    printf("mpl_param_pack() failed\n");
    return (-1);
  }

  printf("size = %d\n", size);

  /* Alloc size including '\0' */
  buf = malloc(size+1);

  size=mpl_param_pack(&packparam, buf, size+1);
  if (size<0)
  {
    printf("mpl_param_pack() failed\n");
    free(buf);
    return (-1);
  }

  printf("buf = '%s', buflen = %d\n", buf, size);

  value_p = strstr(buf, "=");
  if (value_p == NULL)
  {
    printf("No '=' found in packet param\n");
    free(buf);
    return (-1);
  }
  *value_p = '\0';
  value_p++;

  if (mpl_param_unpack(buf, value_p, &unpackparam_p) >= 0)
  {
    printf("mpl_param_unpack() suceeded\n");
    free(buf);
    return (-1);
  }

  free(buf);

  return 0;
}

static int tc_pack_unpack_param_enum(void)
{
  test_my_enum_t myenum;
  test_my_enum_t myenum_copy;
  test_my_enum5_t myenum5;
  int size;
  char* value_p;
  int res=0;
  int copy_size;
  test_my_enum_2_t my_enum_2;
  mpl_param_element_t *par;

  for (myenum=0; myenum <= test_max_value_my_enum; myenum++)
  {
    packparam.id = test_paramid_my_enum;
    packparam.value_p = &myenum;

    /* Find out space need */
    size=mpl_param_pack(&packparam, NULL, 0);
    if (size<0)
    {
      printf("mpl_param_pack() failed (1) for myenum=%d\n", myenum);
      return (-1);
    }

    /* Alloc size including '\0' */
    buf = malloc(size+1);

    size=mpl_param_pack(&packparam, buf, size+1);
    if (size<0)
    {
      printf("mpl_param_pack() failed (2) for myenum=%d\n", myenum);
      free(buf);
      return (-1);
    }

    printf("pack: %s\n", buf);

    value_p = strstr(buf, "=");
    if (value_p == NULL)
    {
      printf("No '=' found in packet param for myenum=%d\n", myenum);
      free(buf);
      return (-1);
    }
    *value_p = '\0';
    value_p++;

    if (mpl_param_unpack(buf, value_p, &unpackparam_p) < 0)
    {
      printf("mpl_param_unpack() failed (3) for myenum=%d\n", myenum);
      free(buf);
      return (-1);
    }

    free(buf);

    res = ((unpackparam_p->id == test_paramid_my_enum) &&
           (*(test_my_enum_t*)unpackparam_p->value_p == myenum));
    mpl_param_element_destroy(unpackparam_p);

    if (res != 1)
      return -1;
  }

  /* Numeric value */
  if (mpl_param_unpack("test.my_enum", "1", &unpackparam_p) < 0)
  {
    printf("mpl_param_unpack() failed (4) for myenum=%s\n", "1");
    return (-1);
  }

  res = ((unpackparam_p->id == test_paramid_my_enum) &&
         (*(test_my_enum_t*)unpackparam_p->value_p == 1));
  mpl_param_element_destroy(unpackparam_p);

  if (res != 1)
    return -1;

  /* Negative value */
  if (mpl_param_unpack("test.my_enum6", "-1", &unpackparam_p) < 0)
  {
    printf("mpl_param_unpack() failed (4) for my_enum6=%s\n", "-1");
    return (-1);
  }

  res = ((unpackparam_p->id == test_paramid_my_enum6) &&
         (*(test_my_enum6_t*)unpackparam_p->value_p == -1));
  mpl_param_element_destroy(unpackparam_p);

  if (res != 1)
    return -1;


  myenum5 = -33;
  par = mpl_param_element_create(test_paramid_my_enum5, &myenum5);
  if (NULL == par)
  {
      printf("mpl_param_element_create(my_emnum5) failed\n");
      return -1;
  }
  if (pack_unpack_one(par, true, TEST_PARAM_SET_ID) < 0)
      return -1;

  mpl_param_element_destroy(par);

  /* Copy out */
  if (mpl_param_unpack("test.my_enum", "val2", &unpackparam_p) < 0)
  {
    printf("mpl_param_unpack() failed (4) for myenum=%s\n", "val2");
    return (-1);
  }

  if ((copy_size = mpl_param_value_copy_out(unpackparam_p, &myenum_copy, 100)) < 0)
  {
    printf("mpl_param_value_copy_out() failed\n");
    mpl_param_element_destroy(unpackparam_p);
    return -1;
  }

  if (copy_size != sizeof(myenum_copy))
  {
    printf("mpl_param_value_copy_out() failed to give correct size\n");
    mpl_param_element_destroy(unpackparam_p);
    return -1;
  }

  printf("myenum_copy = %d, copy_size = %d\n", myenum_copy, copy_size );

  if (myenum_copy != *(test_my_enum_t*)unpackparam_p->value_p)
  {
    printf("mpl_param_value_copy_out() failed to copy correctly\n");
    mpl_param_element_destroy(unpackparam_p);
    return -1;
  }

  mpl_param_element_destroy(unpackparam_p);

  /* Out of range */
  if (mpl_param_unpack("test.my_enum", "10", &unpackparam_p) >= 0)
  {
    printf("mpl_param_unpack() unexpectedly succeeded for myenum=%s\n", "10");
    return (-1);
  }

  /* MIN */
  my_enum_2 = INT32_MIN;
  par = mpl_param_element_create(test_paramid_my_enum_2, &my_enum_2);
  if (NULL == par)
  {
      printf("mpl_param_element_create(my_enum_2) failed\n");
      return -1;
  }
  if (pack_unpack_one(par, true, TEST_PARAM_SET_ID) < 0)
      return -1;

  mpl_param_element_destroy(par);

  /* MAX */
  my_enum_2 = INT32_MAX;
  par = mpl_param_element_create(test_paramid_my_enum_2, &my_enum_2);
  if (NULL == par)
  {
      printf("mpl_param_element_create(my_enum_2) failed\n");
      return -1;
  }
  if (pack_unpack_one(par, true, TEST_PARAM_SET_ID) < 0)
      return -1;

  mpl_param_element_destroy(par);

  return 0;
}

static int tc_pack_unpack_param_enum8(void)
{
  test_my_enum8_t myenum8;
  int size;
  char* value_p;
  int res=0;
  test_my_enum8_t my_enum8_2;
  mpl_param_element_t *par;

  myenum8 = test_max_value_my_enum8 + 1;
  packparam.id = test_paramid_my_enum8;
  packparam.value_p = &myenum8;
  if (mpl_param_pack(&packparam, NULL, 0) != -1)
  {
      printf("pack enum8\n");
      return -1;
  }

  for (myenum8=0; myenum8 < test_max_value_my_enum8; myenum8++)
  {
    packparam.id = test_paramid_my_enum8;
    packparam.value_p = &myenum8;

    /* Find out space need */
    size=mpl_param_pack(&packparam, NULL, 0);
    if (size<0)
    {
      printf("mpl_param_pack() failed (1) for myenum8=%d\n", myenum8);
      return (-1);
    }

    /* Alloc size including '\0' */
    buf = malloc(size+1);

    size=mpl_param_pack(&packparam, buf, size+1);
    if (size<0)
    {
      printf("mpl_param_pack() failed (2) for myenum8=%d\n", myenum8);
      free(buf);
      return (-1);
    }

    printf("pack: %s\n", buf);

    value_p = strstr(buf, "=");
    if (value_p == NULL)
    {
      printf("No '=' found in packet param for myenum8=%d\n", myenum8);
      free(buf);
      return (-1);
    }
    *value_p = '\0';
    value_p++;

    if (mpl_param_unpack(buf, value_p, &unpackparam_p) < 0)
    {
      printf("mpl_param_unpack() failed (3) for myenum8=%d\n", myenum8);
      free(buf);
      return (-1);
    }

    free(buf);

    res = ((unpackparam_p->id == test_paramid_my_enum8) &&
	   (*(test_my_enum8_t*)unpackparam_p->value_p == myenum8));
    mpl_param_element_destroy(unpackparam_p);

    if (res != 1)
      return -1;
  }

  /* Numeric value */
  if (mpl_param_unpack("test.my_enum8", "1", &unpackparam_p) < 0)
  {
    printf("mpl_param_unpack() failed (4) for myenum8=%s\n", "1");
    return (-1);
  }

  res = ((unpackparam_p->id == test_paramid_my_enum8) &&
         (*(test_my_enum8_t*)unpackparam_p->value_p == 1));
  mpl_param_element_destroy(unpackparam_p);

  if (res != 1)
    return -1;

  /* Out of range */
  if (mpl_param_unpack("test.my_enum8", "10", &unpackparam_p) >= 0)
  {
    printf("mpl_param_unpack() unexpectedly succeeded for myenum8=%s\n", "10");
    return (-1);
  }

  /* MIN */
  my_enum8_2 = 0;
  par = mpl_param_element_create(test_paramid_my_enum8_2, &my_enum8_2);
  if (NULL == par)
  {
      printf("mpl_param_element_create(my_enum8_2) failed\n");
      return -1;
  }
  if (pack_unpack_one(par, true, TEST_PARAM_SET_ID) < 0)
      return -1;

  mpl_param_element_destroy(par);

  /* MAX */
  my_enum8_2 = UINT8_MAX;
  par = mpl_param_element_create(test_paramid_my_enum8_2, &my_enum8_2);
  if (NULL == par)
  {
      printf("mpl_param_element_create(my_enum8_2) failed\n");
      return -1;
  }
  if (pack_unpack_one(par, true, TEST_PARAM_SET_ID) < 0)
      return -1;

  mpl_param_element_destroy(par);

  return 0;
}

static int tc_pack_unpack_param_enum16(void)
{
  test_my_enum16_t myenum16;
  int size;
  char* value_p;
  int res=0;
  test_my_enum16_t my_enum16_2;
  mpl_param_element_t *par;

  myenum16 = test_max_value_my_enum16 + 1;
  packparam.id = test_paramid_my_enum16;
  packparam.value_p = &myenum16;
  if (mpl_param_pack(&packparam, NULL, 0) != -1)
  {
      printf("pack enum16\n");
      return -1;
  }

  for (myenum16=0; myenum16 <= test_max_value_my_enum16; myenum16++)
  {
    packparam.id = test_paramid_my_enum16;
    packparam.value_p = &myenum16;

    /* Find out space need */
    size=mpl_param_pack(&packparam, NULL, 0);
    if (size<0)
    {
      printf("mpl_param_pack() failed (1) for myenum16=%d\n", myenum16);
      return (-1);
    }

    /* Alloc size including '\0' */
    buf = malloc(size+1);

    size=mpl_param_pack(&packparam, buf, size+1);
    if (size<0)
    {
      printf("mpl_param_pack() failed (2) for myenum16=%d\n", myenum16);
      free(buf);
      return (-1);
    }

    printf("pack: %s\n", buf);

    value_p = strstr(buf, "=");
    if (value_p == NULL)
    {
      printf("No '=' found in packet param for myenum16=%d\n", myenum16);
      free(buf);
      return (-1);
    }
    *value_p = '\0';
    value_p++;

    if (mpl_param_unpack(buf, value_p, &unpackparam_p) < 0)
    {
      printf("mpl_param_unpack() failed (3) for myenum16=%d\n", myenum16);
      free(buf);
      return (-1);
    }

    free(buf);

    res = ((unpackparam_p->id == test_paramid_my_enum16) &&
	   (*(test_my_enum16_t*)unpackparam_p->value_p == myenum16));
    mpl_param_element_destroy(unpackparam_p);

    if (res != 1)
      return -1;
  }

  /* Numeric value */
  if (mpl_param_unpack("test.my_enum16", "1", &unpackparam_p) < 0)
  {
    printf("mpl_param_unpack() failed (4) for myenum16=%s\n", "1");
    return (-1);
  }

  res = ((unpackparam_p->id == test_paramid_my_enum16) &&
         (*(test_my_enum16_t*)unpackparam_p->value_p == 1));
  mpl_param_element_destroy(unpackparam_p);

  if (res != 1)
    return -1;

  /* Out of range */
  if (mpl_param_unpack("test.my_enum16", "10", &unpackparam_p) >= 0)
  {
    printf("mpl_param_unpack() unexpectedly succeeded for myenum16=%s\n", "10");
    return (-1);
  }

  /* MIN */
  my_enum16_2 = 0;
  par = mpl_param_element_create(test_paramid_my_enum16_2, &my_enum16_2);
  if (NULL == par)
  {
      printf("mpl_param_element_create(my_enum16_2) failed\n");
      return -1;
  }
  if (pack_unpack_one(par, true, TEST_PARAM_SET_ID) < 0)
      return -1;

  mpl_param_element_destroy(par);

  /* MAX */
  my_enum16_2 = UINT16_MAX;
  par = mpl_param_element_create(test_paramid_my_enum16_2, &my_enum16_2);
  if (NULL == par)
  {
      printf("mpl_param_element_create(my_enum16_2) failed\n");
      return -1;
  }
  if (pack_unpack_one(par, true, TEST_PARAM_SET_ID) < 0)
      return -1;

  mpl_param_element_destroy(par);

  return 0;
}

static int tc_pack_unpack_param_enum32(void)
{
  test_my_enum32_t myenum32;
  int size;
  char* value_p;
  int res=0;
  test_my_enum32_t my_enum32_2;
  mpl_param_element_t *par;

  myenum32 = test_max_value_my_enum32 + 1;
  packparam.id = test_paramid_my_enum32;
  packparam.value_p = &myenum32;
  if (mpl_param_pack(&packparam, NULL, 0) != -1)
  {
      printf("pack enum32\n");
      return -1;
  }

  for (myenum32=0; myenum32 <= test_max_value_my_enum32; myenum32++)
  {
    packparam.id = test_paramid_my_enum32;
    packparam.value_p = &myenum32;

    /* Find out space need */
    size=mpl_param_pack(&packparam, NULL, 0);
    if (size<0)
    {
      printf("mpl_param_pack() failed (1) for myenum32=%d\n", myenum32);
      return (-1);
    }

    /* Alloc size including '\0' */
    buf = malloc(size+1);

    size=mpl_param_pack(&packparam, buf, size+1);
    if (size<0)
    {
      printf("mpl_param_pack() failed (2) for myenum32=%d\n", myenum32);
      free(buf);
      return (-1);
    }

    printf("pack: %s\n", buf);

    value_p = strstr(buf, "=");
    if (value_p == NULL)
    {
      printf("No '=' found in packet param for myenum32=%d\n", myenum32);
      free(buf);
      return (-1);
    }
    *value_p = '\0';
    value_p++;

    if (mpl_param_unpack(buf, value_p, &unpackparam_p) < 0)
    {
      printf("mpl_param_unpack() failed (3) for myenum32=%d\n", myenum32);
      free(buf);
      return (-1);
    }

    free(buf);

    res = ((unpackparam_p->id == test_paramid_my_enum32) &&
	   (*(test_my_enum32_t*)unpackparam_p->value_p == myenum32));
    mpl_param_element_destroy(unpackparam_p);

    if (res != 1)
      return -1;
  }

  /* Numeric value */
  if (mpl_param_unpack("test.my_enum32", "1", &unpackparam_p) < 0)
  {
    printf("mpl_param_unpack() failed (4) for myenum32=%s\n", "1");
    return (-1);
  }

  res = ((unpackparam_p->id == test_paramid_my_enum32) &&
         (*(test_my_enum32_t*)unpackparam_p->value_p == 1));
  mpl_param_element_destroy(unpackparam_p);

  if (res != 1)
    return -1;

  /* Out of range */
  if (mpl_param_unpack("test.my_enum32", "10", &unpackparam_p) >= 0)
  {
    printf("mpl_param_unpack() unexpectedly succeeded for myenum32=%s\n", "10");
    return (-1);
  }

  /* MIN */
  my_enum32_2 = 0;
  par = mpl_param_element_create(test_paramid_my_enum32_2, &my_enum32_2);
  if (NULL == par)
  {
      printf("mpl_param_element_create(my_enum32_2) failed\n");
      return -1;
  }
  if (pack_unpack_one(par, true, TEST_PARAM_SET_ID) < 0)
      return -1;

  mpl_param_element_destroy(par);

  /* MAX */
  my_enum32_2 = UINT32_MAX;
  par = mpl_param_element_create(test_paramid_my_enum32_2, &my_enum32_2);
  if (NULL == par)
  {
      printf("mpl_param_element_create(my_enum32_2) failed\n");
      return -1;
  }
  if (pack_unpack_one(par, true, TEST_PARAM_SET_ID) < 0)
      return -1;

  mpl_param_element_destroy(par);

  return 0;
}

static int tc_pack_unpack_param_signed_enum8(void)
{
  test_my_senum8_t mysenum8;
  int size;
  char* value_p;
  int res=0;
  test_my_senum8_t my_senum8_2;
  mpl_param_element_t *par;

  mysenum8 = test_max_value_my_senum8 + 1;
  packparam.id = test_paramid_my_senum8;
  packparam.value_p = &mysenum8;
  if (mpl_param_pack(&packparam, NULL, 0) != -1)
  {
      printf("pack senum8\n");
      return -1;
  }

  for (mysenum8=0; mysenum8 <= test_max_value_my_senum8; mysenum8++)
  {
    packparam.id = test_paramid_my_senum8;
    packparam.value_p = &mysenum8;

    /* Find out space need */
    size=mpl_param_pack(&packparam, NULL, 0);
    if (size<0)
    {
      printf("mpl_param_pack() failed (1) for mysenum8=%d\n", mysenum8);
      return (-1);
    }

    /* Alloc size including '\0' */
    buf = malloc(size+1);

    size=mpl_param_pack(&packparam, buf, size+1);
    if (size<0)
    {
      printf("mpl_param_pack() failed (2) for mysenum8=%d\n", mysenum8);
      free(buf);
      return (-1);
    }

    printf("pack: %s\n", buf);

    value_p = strstr(buf, "=");
    if (value_p == NULL)
    {
      printf("No '=' found in packet param for mysenum8=%d\n", mysenum8);
      free(buf);
      return (-1);
    }
    *value_p = '\0';
    value_p++;

    if (mpl_param_unpack(buf, value_p, &unpackparam_p) < 0)
    {
      printf("mpl_param_unpack() failed (3) for mysenum8=%d\n", mysenum8);
      free(buf);
      return (-1);
    }

    free(buf);

    res = ((unpackparam_p->id == test_paramid_my_senum8) &&
	   (*(test_my_senum8_t*)unpackparam_p->value_p == mysenum8));
    mpl_param_element_destroy(unpackparam_p);

    if (res != 1)
      return -1;
  }

  /* Numeric value */
  if (mpl_param_unpack("test.my_senum8", "1", &unpackparam_p) < 0)
  {
    printf("mpl_param_unpack() failed (4) for mysenum8=%s\n", "1");
    return (-1);
  }

  res = ((unpackparam_p->id == test_paramid_my_senum8) &&
         (*(test_my_senum8_t*)unpackparam_p->value_p == 1));
  mpl_param_element_destroy(unpackparam_p);

  if (res != 1)
    return -1;

  /* Out of range */
  if (mpl_param_unpack("test.my_senum8", "10", &unpackparam_p) >= 0)
  {
    printf("mpl_param_unpack() unexpectedly succeeded for mysenum8=%s\n", "10");
    return (-1);
  }

  /* MIN */
  my_senum8_2 = INT8_MIN;
  par = mpl_param_element_create(test_paramid_my_senum8_2, &my_senum8_2);
  if (NULL == par)
  {
      printf("mpl_param_element_create(my_senum8_2) failed\n");
      return -1;
  }
  if (pack_unpack_one(par, true, TEST_PARAM_SET_ID) < 0)
      return -1;

  mpl_param_element_destroy(par);

  /* MAX */
  my_senum8_2 = INT8_MAX;
  par = mpl_param_element_create(test_paramid_my_senum8_2, &my_senum8_2);
  if (NULL == par)
  {
      printf("mpl_param_element_create(my_senum8_2) failed\n");
      return -1;
  }
  if (pack_unpack_one(par, true, TEST_PARAM_SET_ID) < 0)
      return -1;

  mpl_param_element_destroy(par);

  return 0;
}

static int tc_pack_unpack_param_signed_enum16(void)
{
  test_my_senum16_t mysenum16;
  int size;
  char* value_p;
  int res=0;
  test_my_senum16_t my_senum16_2;
  mpl_param_element_t *par;

  mysenum16 = test_max_value_my_senum16 + 1;
  packparam.id = test_paramid_my_senum16;
  packparam.value_p = &mysenum16;
  if (mpl_param_pack(&packparam, NULL, 0) != -1)
  {
      printf("pack senum16\n");
      return -1;
  }

  for (mysenum16=0; mysenum16 < test_max_value_my_senum16; mysenum16++)
  {
    packparam.id = test_paramid_my_senum16;
    packparam.value_p = &mysenum16;

    /* Find out space need */
    size=mpl_param_pack(&packparam, NULL, 0);
    if (size<0)
    {
      printf("mpl_param_pack() failed (1) for mysenum16=%d\n", mysenum16);
      return (-1);
    }

    /* Alloc size including '\0' */
    buf = malloc(size+1);

    size=mpl_param_pack(&packparam, buf, size+1);
    if (size<0)
    {
      printf("mpl_param_pack() failed (2) for mysenum16=%d\n", mysenum16);
      free(buf);
      return (-1);
    }

    printf("pack: %s\n", buf);

    value_p = strstr(buf, "=");
    if (value_p == NULL)
    {
      printf("No '=' found in packet param for mysenum16=%d\n", mysenum16);
      free(buf);
      return (-1);
    }
    *value_p = '\0';
    value_p++;

    if (mpl_param_unpack(buf, value_p, &unpackparam_p) < 0)
    {
      printf("mpl_param_unpack() failed (3) for mysenum16=%d\n", mysenum16);
      free(buf);
      return (-1);
    }

    free(buf);

    res = ((unpackparam_p->id == test_paramid_my_senum16) &&
	   (*(test_my_senum16_t*)unpackparam_p->value_p == mysenum16));
    mpl_param_element_destroy(unpackparam_p);

    if (res != 1)
      return -1;
  }

  /* Numeric value */
  if (mpl_param_unpack("test.my_senum16", "1", &unpackparam_p) < 0)
  {
    printf("mpl_param_unpack() failed (4) for mysenum16=%s\n", "1");
    return (-1);
  }

  res = ((unpackparam_p->id == test_paramid_my_senum16) &&
         (*(test_my_senum16_t*)unpackparam_p->value_p == 1));
  mpl_param_element_destroy(unpackparam_p);

  if (res != 1)
    return -1;

  /* Out of range */
  if (mpl_param_unpack("test.my_senum16", "10", &unpackparam_p) >= 0)
  {
    printf("mpl_param_unpack() unexpectedly succeeded for mysenum16=%s\n", "10");
    return (-1);
  }

  /* MIN */
  my_senum16_2 = INT16_MIN;
  par = mpl_param_element_create(test_paramid_my_senum16_2, &my_senum16_2);
  if (NULL == par)
  {
      printf("mpl_param_element_create(my_senum16_2) failed\n");
      return -1;
  }
  if (pack_unpack_one(par, true, TEST_PARAM_SET_ID) < 0)
      return -1;

  mpl_param_element_destroy(par);

  /* MAX */
  my_senum16_2 = INT16_MAX;
  par = mpl_param_element_create(test_paramid_my_senum16_2, &my_senum16_2);
  if (NULL == par)
  {
      printf("mpl_param_element_create(my_senum16_2) failed\n");
      return -1;
  }
  if (pack_unpack_one(par, true, TEST_PARAM_SET_ID) < 0)
      return -1;

  mpl_param_element_destroy(par);

  return 0;
}


static int tc_pack_unpack_param_signed_enum32(void)
{
  test_my_senum32_t mysenum32;
  int size;
  char* value_p;
  int res=0;
  test_my_senum32_t my_senum32_2;
  mpl_param_element_t *par;

  mysenum32 = test_max_value_my_senum32 + 1;
  packparam.id = test_paramid_my_senum32;
  packparam.value_p = &mysenum32;
  if (mpl_param_pack(&packparam, NULL, 0) != -1)
  {
      printf("pack senum32\n");
      return -1;
  }

  for (mysenum32=0; mysenum32 < test_max_value_my_senum32; mysenum32++)
  {
    packparam.id = test_paramid_my_senum32;
    packparam.value_p = &mysenum32;

    /* Find out space need */
    size=mpl_param_pack(&packparam, NULL, 0);
    if (size<0)
    {
      printf("mpl_param_pack() failed (1) for mysenum32=%d\n", mysenum32);
      return (-1);
    }

    /* Alloc size including '\0' */
    buf = malloc(size+1);

    size=mpl_param_pack(&packparam, buf, size+1);
    if (size<0)
    {
      printf("mpl_param_pack() failed (2) for mysenum32=%d\n", mysenum32);
      free(buf);
      return (-1);
    }

    printf("pack: %s\n", buf);

    value_p = strstr(buf, "=");
    if (value_p == NULL)
    {
      printf("No '=' found in packet param for mysenum32=%d\n", mysenum32);
      free(buf);
      return (-1);
    }
    *value_p = '\0';
    value_p++;

    if (mpl_param_unpack(buf, value_p, &unpackparam_p) < 0)
    {
      printf("mpl_param_unpack() failed (3) for mysenum32=%d\n", mysenum32);
      free(buf);
      return (-1);
    }

    free(buf);

    res = ((unpackparam_p->id == test_paramid_my_senum32) &&
	   (*(test_my_senum32_t*)unpackparam_p->value_p == mysenum32));
    mpl_param_element_destroy(unpackparam_p);

    if (res != 1)
      return -1;
  }

  /* Numeric value */
  if (mpl_param_unpack("test.my_senum32", "1", &unpackparam_p) < 0)
  {
    printf("mpl_param_unpack() failed (4) for mysenum32=%s\n", "1");
    return (-1);
  }

  res = ((unpackparam_p->id == test_paramid_my_senum32) &&
         (*(test_my_senum32_t*)unpackparam_p->value_p == 1));
  mpl_param_element_destroy(unpackparam_p);

  if (res != 1)
    return -1;

  /* Out of range */
  if (mpl_param_unpack("test.my_senum32", "10", &unpackparam_p) >= 0)
  {
    printf("mpl_param_unpack() unexpectedly succeeded for mysenum32=%s\n", "10");
    return (-1);
  }

  /* MIN */
  my_senum32_2 = INT32_MIN;
  par = mpl_param_element_create(test_paramid_my_senum32_2, &my_senum32_2);
  if (NULL == par)
  {
      printf("mpl_param_element_create(my_senum32_2) failed\n");
      return -1;
  }
  if (pack_unpack_one(par, true, TEST_PARAM_SET_ID) < 0)
      return -1;

  mpl_param_element_destroy(par);

  /* MAX */
  my_senum32_2 = INT32_MAX;
  par = mpl_param_element_create(test_paramid_my_senum32_2, &my_senum32_2);
  if (NULL == par)
  {
      printf("mpl_param_element_create(my_senum32_2) failed\n");
      return -1;
  }
  if (pack_unpack_one(par, true, TEST_PARAM_SET_ID) < 0)
      return -1;

  mpl_param_element_destroy(par);

  return 0;
}


static int tc_pack_unpack_param_bool(void)
{
  int mybool;
  bool mybool_copy = 0;
  int copy_size;
  int size;
  char* value_p;
  int res=0;

  packparam.id = test_paramid_mybool;
  mybool = 2;
  packparam.value_p = &mybool;
  if (mpl_param_pack(&packparam, NULL, 0) != -1)
  {
      printf("pack bool\n");
      return -1;
  }

  for (mybool=0; mybool < 2; mybool++)
  {
    packparam.id = test_paramid_mybool;
    packparam.value_p = &mybool;

    /* Find out space need */
    size=mpl_param_pack(&packparam, NULL, 0);
    if (size<0)
    {
      printf("mpl_param_pack() failed (1) for mybool=%d\n", mybool);
      return (-1);
    }

    /* Alloc size including '\0' */
    buf = malloc(size+1);

    size=mpl_param_pack(&packparam, buf, size+1);
    if (size<0)
    {
      printf("mpl_param_pack() failed (2) for mybool=%d\n", mybool);
      free(buf);
      return (-1);
    }

    // printf("pack: %s\n", buf);

    value_p = strstr(buf, "=");
    if (value_p == NULL)
    {
      printf("No '=' found in packet param for mybool=%d\n", mybool);
      free(buf);
      return (-1);
    }
    *value_p = '\0';
    value_p++;

    if (mpl_param_unpack(buf, value_p, &unpackparam_p) < 0)
    {
      printf("mpl_param_unpack() failed (3) for mybool=%d\n", mybool);
      free(buf);
      return (-1);
    }

    free(buf);

    res = ((unpackparam_p->id == test_paramid_mybool) &&
	   (*(bool*)unpackparam_p->value_p == mybool));
    mpl_param_element_destroy(unpackparam_p);

    if (res != 1)
      return -1;
  }

  /* Numeric value */
  if (mpl_param_unpack("test.mybool", "0", &unpackparam_p) < 0)
  {
    printf("mpl_param_unpack() failed (4) for mybool=%s\n", "0");
    return (-1);
  }

  res = ((unpackparam_p->id == test_paramid_mybool) &&
         (*(bool*)unpackparam_p->value_p == false));

  if (1 != res)
  {
    mpl_param_element_destroy(unpackparam_p);
    return res;
  }


  if ((copy_size = mpl_param_value_copy_out(unpackparam_p, &mybool_copy, 100)) < 0)
  {
    printf("mpl_param_value_copy_out() failed\n");
    mpl_param_element_destroy(unpackparam_p);
    return -1;
  }

  if (copy_size != sizeof(mybool_copy))
  {
    printf("mpl_param_value_copy_out() failed to give correct size\n");
    mpl_param_element_destroy(unpackparam_p);
    return -1;
  }

  printf("mybool_copy = %u, copy_size = %d\n", mybool_copy, copy_size );

  if (mybool_copy != *(bool*)unpackparam_p->value_p)
  {
    printf("mpl_param_value_copy_out() failed to copy correctly\n");
    mpl_param_element_destroy(unpackparam_p);
    return -1;
  }

  if ((copy_size = mpl_param_value_copy_out(unpackparam_p, &mybool_copy, 0)) != sizeof(bool))
  {
    printf("mpl_param_value_copy_out() failed to give correct size\n");
    mpl_param_element_destroy(unpackparam_p);
    return -1;
  }

  mpl_param_element_destroy(unpackparam_p);


  /* Numeric value */
  if (mpl_param_unpack("test.mybool", "1", &unpackparam_p) < 0)
  {
    printf("mpl_param_unpack() failed for mybool=%s\n", "1");
    return (-1);
  }

  res = ((unpackparam_p->id == test_paramid_mybool) &&
         (*(bool*)unpackparam_p->value_p == true));
  mpl_param_element_destroy(unpackparam_p);

  if (1 != res)
    return -1;

  /* Numeric value 2 */
  if (mpl_param_unpack("test.mybool", "2", &unpackparam_p) < 0)
  {
    printf("mpl_param_unpack() failed for mybool=%s\n", "2");
    return (-1);
  }

  res = ((unpackparam_p->id == test_paramid_mybool) &&
         (*(bool*)unpackparam_p->value_p == true));
  mpl_param_element_destroy(unpackparam_p);

  if (1 != res)
    return -1;

  return 0;
}

static int
tc_pack_unpack_param_bool8(void)
{
  uint8_t mybool8=1;
  uint8_t mybool8_copy = 0;
  int copy_size;
  int size;
  char* value_p;
  mpl_param_element_t* clone_p;
  int res;

  packparam.id = test_paramid_mybool8;
  mybool8 = 2;
  packparam.value_p = &mybool8;
  if (mpl_param_pack(&packparam, NULL, 0) != -1)
  {
      printf("pack bool8\n");
      return -1;
  }

  packparam.id = test_paramid_mybool8;
  mybool8 = 1;
  packparam.value_p = &mybool8;

  /* Find out space need */
  size=mpl_param_pack(&packparam, NULL, 0);
  if (size<0)
  {
    printf("mpl_param_pack() failed\n");
    return (-1);
  }

  /* Alloc size including '\0' */
  buf = malloc(size+1);

  size=mpl_param_pack(&packparam, buf, size+1);
  if (size<0)
  {
    printf("mpl_param_pack() failed\n");
    free(buf);
    return (-1);
  }

  value_p = strstr(buf, "=");
  if (value_p == NULL)
  {
    printf("No '=' found in packet param\n");
    free(buf);
    return (-1);
  }
  *value_p = '\0';
  value_p++;

  if (mpl_param_unpack(buf, value_p, &unpackparam_p) < 0)
  {
    printf("mpl_param_unpack() failed\n");
    free(buf);
    return (-1);
  }

  free(buf);

  if ((unpackparam_p->id != packparam.id) ||
      (*(uint8_t*)unpackparam_p->value_p != mybool8))
  {
    mpl_param_element_destroy(unpackparam_p);
    return -1;
  }


  clone_p = mpl_param_element_clone(unpackparam_p);
  if (NULL == clone_p)
  {
    printf("mpl_param_element_clone() failed\n");
    mpl_param_element_destroy(unpackparam_p);
    return (-1);
  }

  if (*(uint8_t*)clone_p->value_p != *(uint8_t*)unpackparam_p->value_p)
  {
    printf("mpl_param_element_clone() failed to clone correctly\n");
    mpl_param_element_destroy(clone_p);
    mpl_param_element_destroy(unpackparam_p);
    return (-1);
  }

  mpl_param_element_destroy(clone_p);

  if ((copy_size = mpl_param_value_copy_out(unpackparam_p, &mybool8_copy, 100)) < 0)
  {
    printf("mpl_param_value_copy_out() failed\n");
    mpl_param_element_destroy(unpackparam_p);
    return -1;
  }

  if (copy_size != sizeof(mybool8_copy))
  {
    printf("mpl_param_value_copy_out() failed to give correct size\n");
    mpl_param_element_destroy(unpackparam_p);
    return -1;
  }

  printf("mybool8_copy = %u, copy_size = %d\n", mybool8_copy, copy_size );

  if (mybool8_copy != *(uint8_t*)unpackparam_p->value_p)
  {
    printf("mpl_param_value_copy_out() failed to copy correctly\n");
    mpl_param_element_destroy(unpackparam_p);
    return -1;
  }

  if ((copy_size = mpl_param_value_copy_out(unpackparam_p, &mybool8_copy, 0)) != sizeof(mybool8_copy))
  {
    printf("mpl_param_value_copy_out() failed to give correct size\n");
    mpl_param_element_destroy(unpackparam_p);
    return -1;
  }

  mpl_param_element_destroy(unpackparam_p);

  /* Numeric value */
  if (mpl_param_unpack("test.mybool8", "1", &unpackparam_p) < 0)
  {
    printf("mpl_param_unpack() failed for mybool8=%s\n", "1");
    return (-1);
  }

  res = ((unpackparam_p->id == test_paramid_mybool8) &&
         (*(uint8_t*)unpackparam_p->value_p));
  mpl_param_element_destroy(unpackparam_p);

  if (1 != res)
    return -1;

  /* Numeric value 2 */
  if (mpl_param_unpack("test.mybool8", "2", &unpackparam_p) < 0)
  {
    printf("mpl_param_unpack() failed for mybool8=%s\n", "2");
    return (-1);
  }

  res = ((unpackparam_p->id == test_paramid_mybool8) &&
         (*(uint8_t*)unpackparam_p->value_p));
  mpl_param_element_destroy(unpackparam_p);

  if (1 != res)
    return -1;

  return 0;
}


static int tc_pack_param_illegal(void)
{
  int myint = 5;
  int size = 5;

  packparam.id = 555;
  packparam.value_p = &myint;
  buf = malloc(size);

  /*negative - packparam == NULL*/
  if (mpl_param_pack(NULL, NULL, 0) >= 0)
  {
    printf("mpl_param_pack() unexpected success packparam == NULL\n");
    free(buf);
    return (-1);
  }


  free(buf);
  return 0;
}

static int tc_unpack_param_illegal(void)
{
  char key1[]="test.my_enum";
  char key2[]="test.my_enum";
  char key3[]="test.myint";
  char value1[]="val1";
  char value2[]="va1";
  char value3[]="";
  char value4[]="5";
  char value5[]="0abc";

  /* Positive */
  if (mpl_param_unpack(key1, value1, &unpackparam_p) < 0)
  {
    printf("mpl_param_unpack() failed for %s=%s\n", key1, value1);
    return (-1);
  }
  mpl_param_element_destroy(unpackparam_p);

  /* Negative: not in enum */
  if (mpl_param_unpack(key1, value2, &unpackparam_p) == 0)
  {
    printf("mpl_param_unpack() unexpected success %s=%s\n", key1, value2);
    mpl_param_element_destroy(unpackparam_p);
    return (-1);
  }

  /* Negative: enum: empty string */
  if (mpl_param_unpack(key1, value3, &unpackparam_p) == 0)
  {
    printf("mpl_param_unpack() unexpected success %s=%s\n", key1, value3);
    mpl_param_element_destroy(unpackparam_p);
    return (-1);
  }

  /* Negative: integer not in enum */
  if (mpl_param_unpack(key1, value4, &unpackparam_p) == 0)
  {
    printf("mpl_param_unpack() unexpected success %s=%s\n", key1, value4);
    mpl_param_element_destroy(unpackparam_p);
    return (-1);
  }

  /* Negative: enum: integer only part of token */
  if (mpl_param_unpack(key1, value5, &unpackparam_p) == 0)
  {
    printf("mpl_param_unpack() unexpected success %s=%s\n", key1, value5);
    mpl_param_element_destroy(unpackparam_p);
    return (-1);
  }

  /* Negative: bool: empty string */
  if (mpl_param_unpack(key2, value3, &unpackparam_p) == 0)
  {
    printf("mpl_param_unpack() unexpected success %s=%s\n", key2, value3);
    mpl_param_element_destroy(unpackparam_p);
    return (-1);
  }

  /* Negative: int: empty string */
  if (mpl_param_unpack(key3, value3, &unpackparam_p) == 0)
  {
    printf("mpl_param_unpack() unexpected success %s=%s\n", key3, value3);
    mpl_param_element_destroy(unpackparam_p);
    return (-1);
  }

  /* Negative: element NULL */
  if (mpl_param_unpack(key1, value1, NULL) == 0)
  {
    printf("mpl_param_unpack() unexpected success %s=%s\n", key1, value1);
    mpl_param_element_destroy(unpackparam_p);
    return (-1);
  }
  return 0;
}

static int tc_list(void)
{
  mpl_list_t *msg_p;
  mpl_param_element_t*param_elem_p;
  mpl_param_element_t*param_elem2_p;

  /* With parameters */
  msg_p = NULL;

  param_elem_p = mpl_param_element_create_stringn(test_paramid_mystring, "tull", 4);
  if (NULL == param_elem_p)
  {
    mpl_param_list_destroy(&msg_p);
    return -1;
  }
  mpl_list_add(&msg_p,
               &param_elem_p->list_entry);


  /*Check list len*/
  if (mpl_list_len(msg_p) < 1)
  {
    printf("mpl_list_len returned unexpected len (<1) \n");
    mpl_param_list_destroy(&msg_p);
    return -1;
  }

  /*Check that the element is on the list*/
  if(!mpl_is_on_list(msg_p,&param_elem_p->list_entry))
  {
    printf("mpl_is_on_list returned false\n");
    mpl_param_list_destroy(&msg_p);
    return -1;
  }

  param_elem2_p = mpl_param_element_create_stringn(test_paramid_mystring, "toys", 4);
  if (NULL == param_elem2_p)
  {
    mpl_param_list_destroy(&msg_p);
    return -1;
  }
  mpl_list_add(&msg_p,
                   &param_elem2_p->list_entry);

  /*Negative - element is NULL*/
  if(mpl_is_on_list(msg_p,NULL))
  {
    printf("mpl_is_on_list returned unexpectedly TRUE\n");
    mpl_param_list_destroy(&msg_p);
    return -1;
  }

  /*Remove from list*/
  mpl_list_remove(&msg_p,
                  &param_elem_p->list_entry);

  /*Remove from list*/
  mpl_list_remove(&msg_p,
                  &param_elem2_p->list_entry);


  /*Negative - Check that the element is on the list*/
  if(mpl_is_on_list(msg_p,&param_elem_p->list_entry))
  {
    printf("mpl_is_on_list returned unexpectedly TRUE\n");
    mpl_param_list_destroy(&msg_p);
    mpl_param_element_destroy(param_elem_p);
    mpl_param_element_destroy(param_elem2_p);
    return -1;
  }

  /*Negative - Remove element NULL (none in list)*/
  if (mpl_list_remove(&msg_p, NULL) != NULL)
  {
    printf("mpl_list_remove returned an unexpected value \n");
    mpl_param_list_destroy(&msg_p);
    mpl_param_element_destroy(param_elem_p);
    mpl_param_element_destroy(param_elem2_p);
    return -1;
  }


  mpl_param_element_destroy(param_elem_p);
  mpl_param_element_destroy(param_elem2_p);
  mpl_param_list_destroy(&msg_p);
  msg_p = NULL;
  mpl_param_list_destroy(&msg_p); /* Does it handle NULL? */

  return 0;
}

static int
tc_param_get_default()
{
  mpl_param_element_t* elem_p;
  int res;
  mpl_blacklist_t blacklist = NULL;

  /* Normal case */
  elem_p = mpl_param_element_get_default(test_paramid_myint);
  if (NULL == elem_p)
  {
    printf("mpl_param_element_get_default() failed (1)\n");
    return (-1);
  }

  res = (elem_p->id == test_paramid_myint) &&
        (*(int*)elem_p->value_p == 99);

  if (res != 1)
  {
    printf("mpl_param_element_get_default() wrong id or value\n");
    mpl_param_element_destroy(elem_p);
    return -1;
  }

  mpl_param_element_destroy(elem_p);

  elem_p = mpl_param_element_get_default(test_paramid_my_enum);
  if (NULL == elem_p)
  {
    printf("mpl_param_element_get_default() failed (2)\n");
    return (-1);
  }

  res = (elem_p->id == test_paramid_my_enum) &&
        (*(test_my_enum_t*)elem_p->value_p == test_my_enum_val1);

  if (res != 1)
  {
    printf("mpl_param_element_get_default() wrong id or value\n");
    mpl_param_element_destroy(elem_p);
    return -1;
  }

  mpl_param_element_destroy(elem_p);

  /* Negative: wrong ID */
  elem_p = mpl_param_element_get_default(test_enum_size_paramids);
  if (NULL != elem_p)
  {
    printf("mpl_param_element_get_default() unexpectedly succeeded for wrong ID\n");
    mpl_param_element_destroy(elem_p);
    return (-1);
  }

  /* Negative: No default */
  elem_p = mpl_param_element_get_default(test_paramid_mybool);
  if (NULL != elem_p)
  {
    printf("mpl_param_element_get_default() unexpectedly succeeded (1) for no default\n");
    mpl_param_element_destroy(elem_p);
    return (-1);
  }

  /* Blacklist: */
  elem_p = mpl_param_element_create_empty(test_paramid_mywstring);
  mpl_list_add(&blacklist, &elem_p->list_entry);
  elem_p = mpl_param_element_create_empty(test_paramid_myint);
  mpl_list_add(&blacklist, &elem_p->list_entry);
  elem_p = mpl_param_element_create_empty(test_paramid_myuint16);
  mpl_list_add(&blacklist, &elem_p->list_entry);

  /* Negative */
  elem_p = mpl_param_element_get_default_bl(test_paramid_myint, blacklist);
  if (NULL != elem_p)
  {
    printf("mpl_param_element_get_default_bl() unexpectedly returned an element\n");
    mpl_param_element_destroy(elem_p);
    return (-1);
  }

  /* Positive */
  elem_p = mpl_param_element_get_default_bl(test_paramid_my_enum, blacklist);
  if (NULL == elem_p)
  {
    printf("mpl_param_element_get_default_bl() failed\n");
    return (-1);
  }

  res = (elem_p->id == test_paramid_my_enum) &&
        (*(test_my_enum_t*)elem_p->value_p == test_my_enum_val1);

  if (res != 1)
  {
    printf("mpl_param_element_get_default() wrong id or value\n");
    mpl_param_element_destroy(elem_p);
    return -1;
  }

  mpl_param_element_destroy(elem_p);
  mpl_param_list_destroy(&blacklist);

  /* String tuple */
  elem_p = mpl_param_element_get_default(test_paramid_mystring_tup2);
  if (NULL == elem_p)
  {
    printf("mpl_param_element_get_default() failed (3)\n");
    return (-1);
  }

  res = (elem_p->id == test_paramid_mystring_tup2) &&
        !strcmp(((mpl_string_tuple_t*)elem_p->value_p)->key_p,"heisann") &&
        !strcmp(((mpl_string_tuple_t*)elem_p->value_p)->value_p,"hoppsann");

  printf("key=%s, value=%s\n", ((mpl_string_tuple_t*)elem_p->value_p)->key_p, ((mpl_string_tuple_t*)elem_p->value_p)->value_p);

  if (res != 1)
  {
    printf("mpl_param_element_get_default() wrong id or value\n");
    mpl_param_element_destroy(elem_p);
    return -1;
  }

  mpl_param_element_destroy(elem_p);

  /* int tuple */
  elem_p = mpl_param_element_get_default(test_paramid_myint_tup);
  if (NULL == elem_p)
  {
    printf("mpl_param_element_get_default() failed (4)\n");
    return (-1);
  }

  res = (elem_p->id == test_paramid_myint_tup) &&
        ((mpl_int_tuple_t*)elem_p->value_p)->key == 11 &&
        ((mpl_int_tuple_t*)elem_p->value_p)->value == 22;

  printf("key=%d, value=%d\n", ((mpl_int_tuple_t*)elem_p->value_p)->key, ((mpl_int_tuple_t*)elem_p->value_p)->value);

  if (res != 1)
  {
    printf("mpl_param_element_get_default() wrong id or value\n");
    mpl_param_element_destroy(elem_p);
    return -1;
  }

  mpl_param_element_destroy(elem_p);


  /* strint tuple */
  elem_p = mpl_param_element_get_default(test_paramid_mystrint_tup);
  if (NULL == elem_p)
  {
    printf("mpl_param_element_get_default() failed (5)\n");
    return (-1);
  }

  res = (elem_p->id == test_paramid_mystrint_tup) &&
        !strcmp(((mpl_strint_tuple_t*)elem_p->value_p)->key_p,"tjohei") &&
        (((mpl_strint_tuple_t*)elem_p->value_p)->value == 33);

  printf("key=%s, value=%d\n", ((mpl_strint_tuple_t*)elem_p->value_p)->key_p, ((mpl_strint_tuple_t*)elem_p->value_p)->value);

  if (res != 1)
  {
    printf("mpl_param_element_get_default() wrong id or value\n");
    mpl_param_element_destroy(elem_p);
    return -1;
  }

  mpl_param_element_destroy(elem_p);


  return 0;
}


static int tc_param_allow_get(void)
{
  mpl_blacklist_t blacklist = NULL;
  mpl_param_element_t *elem_p;

  /* Positive */
  if (!mpl_param_allow_get(test_paramid_mystring))
  {
    printf("mpl_param_allow_get() failed\n");
    return -1;
  }

  if (!mpl_param_allow_get(test_paramid_mywstring))
  {
    printf("mpl_param_allow_get() failed\n");
    return -1;
  }

  /* Negative */
  if (mpl_param_allow_get(test_paramid_myint))
  {
    printf("mpl_param_allow_get() unexpectedly succeeded (1)\n");
    return -1;
  }

  /* Out of bounds */
  if (mpl_param_allow_get(test_enum_size_paramids))
  {
    printf("mpl_param_allow_get() unexpectedly succeeded (2)\n");
    return -1;
  }

  /* Blacklist: */
  elem_p = mpl_param_element_create_empty(test_paramid_mywstring);
  mpl_list_add(&blacklist, &elem_p->list_entry);
  elem_p = mpl_param_element_create_empty(test_paramid_myint);
  mpl_list_add(&blacklist, &elem_p->list_entry);
  elem_p = mpl_param_element_create_empty(test_paramid_my_enum);
  mpl_list_add(&blacklist, &elem_p->list_entry);

  /* Positive */
  if (!mpl_param_allow_get_bl(test_paramid_mystring, blacklist))
  {
    printf("mpl_param_allow_get_bl() failed\n");
    return -1;
  }

  /* Negative */
  if (mpl_param_allow_get_bl(test_paramid_mywstring, blacklist))
  {
    printf("mpl_param_allow_get_bl() unexpectedly succeeded (1)\n");
    return -1;
  }

  mpl_param_list_destroy(&blacklist);

  return 0;
}

static int tc_param_allow_set(void)
{
  mpl_blacklist_t blacklist = NULL;
  mpl_param_element_t *elem_p;

  /* Positive */
  if (!mpl_param_allow_set(test_paramid_mystring))
  {
    printf("mpl_param_allow_set() failed\n");
    return -1;
  }

  if (!mpl_param_allow_set(test_paramid_mywstring))
  {
    printf("mpl_param_allow_set() failed\n");
    return -1;
  }

  /* Negative */
  if (mpl_param_allow_set(test_paramid_myint))
  {
    printf("mpl_param_allow_set() unexpectedly succeeded (1)\n");
    return -1;
  }

  /* Out of bounds */
  if (mpl_param_allow_set(test_enum_size_paramids))
  {
    printf("mpl_param_allow_set() unexpectedly succeeded (2)\n");
    return -1;
  }

  /* Blacklist: */
  elem_p = mpl_param_element_create_empty(test_paramid_mywstring);
  mpl_list_add(&blacklist, &elem_p->list_entry);
  elem_p = mpl_param_element_create_empty(test_paramid_myint);
  mpl_list_add(&blacklist, &elem_p->list_entry);
  elem_p = mpl_param_element_create_empty(test_paramid_my_enum);
  mpl_list_add(&blacklist, &elem_p->list_entry);

  /* Positive */
  if (!mpl_param_allow_set_bl(test_paramid_mystring, blacklist))
  {
    printf("mpl_param_allow_set_bl() failed\n");
    return -1;
  }

  /* Negative */
  if (mpl_param_allow_set_bl(test_paramid_mywstring, blacklist))
  {
    printf("mpl_param_allow_set_bl() unexpectedly succeeded (1)\n");
    return -1;
  }

  mpl_param_list_destroy(&blacklist);

  return 0;
}

static int tc_param_allow_config(void)
{
  mpl_blacklist_t blacklist = NULL;
  mpl_param_element_t *elem_p;

  /* Positive */
  if (!mpl_param_allow_config(test_paramid_mystring))
  {
    printf("mpl_param_allow_config() failed\n");
    return -1;
  }

  if (!mpl_param_allow_config(test_paramid_mywstring))
  {
    printf("mpl_param_allow_config() failed\n");
    return -1;
  }

  /* Negative */
  if (mpl_param_allow_config(test_paramid_myuint16))
  {
    printf("mpl_param_allow_config() unexpectedly succeeded (1)\n");
    return -1;
  }

  /* Out of bounds */
  if (mpl_param_allow_config(test_enum_size_paramids))
  {
    printf("mpl_param_allow_config() unexpectedly succeeded (2)\n");
    return -1;
  }

  /* Blacklist: */
  elem_p = mpl_param_element_create_empty(test_paramid_mywstring);
  mpl_list_add(&blacklist, &elem_p->list_entry);
  elem_p = mpl_param_element_create_empty(test_paramid_myint);
  mpl_list_add(&blacklist, &elem_p->list_entry);
  elem_p = mpl_param_element_create_empty(test_paramid_my_enum);
  mpl_list_add(&blacklist, &elem_p->list_entry);

  /* Positive */
  if (!mpl_param_allow_config_bl(test_paramid_mystring, blacklist))
  {
    printf("mpl_param_allow_config_bl() failed\n");
    return -1;
  }

  /* Negative */
  if (mpl_param_allow_config_bl(test_paramid_mywstring, blacklist))
  {
    printf("mpl_param_allow_config_bl() unexpectedly succeeded (1)\n");
    return -1;
  }

  mpl_param_list_destroy(&blacklist);

  return 0;
}

static int tc_clone_param(void)
{
  mpl_param_element_t *param1_p;
  mpl_param_element_t *param2_p;
  test_my_enum_t myenum = test_my_enum_val1;
  int myint = 5;
  uint8_t myuint8 = 8;
  uint32_t myuint32 = 32;
  bool mybool = true;
  char *mystring = "hello";


  /* Positive enum */
  param1_p = mpl_param_element_create(test_paramid_my_enum, &myenum);
  if (NULL == param1_p)
  {
    printf("mpl_param_element_create() enum failed\n");
    return (-1);
  }

  param2_p = mpl_param_element_clone(param1_p);
  if (NULL == param2_p)
  {
    printf("mpl_param_element_clone() enum failed\n");
    mpl_param_element_destroy(param1_p);
    return (-1);
  }

  if (*(test_my_enum_t*)param1_p->value_p != *(test_my_enum_t*)param2_p->value_p)
  {
    printf("mpl_param_element_clone() enum failed to clone correctly\n");
    mpl_param_element_destroy(param1_p);
    mpl_param_element_destroy(param2_p);
    return (-1);
  }

  mpl_param_element_destroy(param1_p);
  mpl_param_element_destroy(param2_p);

  /* Positive int */
  param1_p = mpl_param_element_create(test_paramid_myint, &myint);
  if (NULL == param1_p)
  {
    printf("mpl_param_element_create() int failed\n");
    return (-1);
  }

  param2_p = mpl_param_element_clone(param1_p);
  if (NULL == param2_p)
  {
    printf("mpl_param_element_clone() int failed\n");
    mpl_param_element_destroy(param1_p);
    return (-1);
  }

  if (*(int*)param1_p->value_p != *(int*)param2_p->value_p)
  {
    printf("mpl_param_element_clone() int failed to clone correctly\n");
    mpl_param_element_destroy(param1_p);
    mpl_param_element_destroy(param1_p);
    return (-1);
  }


  mpl_param_element_destroy(param1_p);
  mpl_param_element_destroy(param2_p);

  /* Positive uint8 */
  param1_p = mpl_param_element_create(test_paramid_myuint8, &myuint8);
  if (NULL == param1_p)
  {
    printf("mpl_param_element_create() uint8 failed\n");
    return (-1);
  }

  param2_p = mpl_param_element_clone(param1_p);
  if (NULL == param2_p)
  {
    printf("mpl_param_element_clone() uint8 failed\n");
    mpl_param_element_destroy(param1_p);
    return (-1);
  }

  if (*(uint8_t*)param1_p->value_p != *(uint8_t*)param2_p->value_p)
  {
    printf("mpl_param_element_clone() uint8 failed to clone correctly\n");
    mpl_param_element_destroy(param1_p);
    mpl_param_element_destroy(param2_p);
    return (-1);
  }

  mpl_param_element_destroy(param1_p);
  mpl_param_element_destroy(param2_p);

  /* Positive uint32 */
  param1_p = mpl_param_element_create(test_paramid_myuint32, &myuint32);
  if (NULL == param1_p)
  {
    printf("mpl_param_element_create() uint32 failed\n");
    return (-1);
  }

  param2_p = mpl_param_element_clone(param1_p);
  if (NULL == param2_p)
  {
    printf("mpl_param_element_clone() uint32 failed\n");
    mpl_param_element_destroy(param1_p);
    return (-1);
  }

  if (*(uint32_t*)param1_p->value_p != *(uint32_t*)param2_p->value_p)
  {
    printf("mpl_param_element_clone() uint32 failed to clone correctly\n");
    mpl_param_element_destroy(param1_p);
    mpl_param_element_destroy(param2_p);
    return (-1);
  }

  mpl_param_element_destroy(param1_p);
  mpl_param_element_destroy(param2_p);

  /* Positive bool */
  param1_p = mpl_param_element_create(test_paramid_mybool, &mybool);
  if (NULL == param1_p)
  {
    printf("mpl_param_element_create() bool failed\n");
    return (-1);
  }

  param2_p = mpl_param_element_clone(param1_p);
  if (NULL == param2_p)
  {
    printf("mpl_param_element_clone() bool failed\n");
    mpl_param_element_destroy(param1_p);
    return (-1);
  }

  if (*(bool*)param1_p->value_p != *(bool*)param2_p->value_p)
  {
    printf("mpl_param_element_clone() bool failed to clone correctly\n");
    mpl_param_element_destroy(param1_p);
    mpl_param_element_destroy(param2_p);
    return (-1);
  }


  mpl_param_element_destroy(param1_p);
  mpl_param_element_destroy(param2_p);

  /* Positive string */
  param1_p = mpl_param_element_create(test_paramid_mystring, mystring);
  if (NULL == param1_p)
  {
    printf("mpl_param_element_create() string failed\n");
    return (-1);
  }

  param2_p = mpl_param_element_clone(param1_p);
  if (NULL == param2_p)
  {
    printf("mpl_param_element_clone() string failed\n");
    mpl_param_element_destroy(param1_p);
    return (-1);
  }

  if (strcmp(param1_p->value_p, param2_p->value_p))
  {
    printf("mpl_param_element_clone() string failed to clone correctly\n");
    mpl_param_element_destroy(param1_p);
    mpl_param_element_destroy(param2_p);
    return (-1);
  }

  mpl_param_element_destroy(param1_p);
  mpl_param_element_destroy(param2_p);


  /* Positive: no value */
  param1_p = mpl_param_element_create_empty(test_paramid_my_enum);
  if (NULL == param1_p)
  {
    printf("mpl_param_element_create_empty() failed\n");
    return (-1);
  }

  param1_p->value_p = NULL;
  param2_p = mpl_param_element_clone(param1_p);
  if (NULL == param2_p)
  {
    printf("mpl_param_element_clone() failed\n");
    mpl_param_element_destroy(param1_p);
    return (-1);
  }
  mpl_param_element_destroy(param2_p);

  /* Negative: nothing to clone */
  param2_p = mpl_param_element_clone(NULL);
  if (NULL != param2_p)
  {
    printf("mpl_param_element_clone() unexpectedly succeeded\n");
    mpl_param_element_destroy(param1_p);
    mpl_param_element_destroy(param2_p);
    return (-1);
  }

  /* Negative: wrong ID */
  param1_p->id = test_enum_size_paramids;
  param2_p = mpl_param_element_clone(param1_p);
  if (NULL != param2_p)
  {
    printf("mpl_param_element_clone() unexpectedly succeeded\n");
    mpl_param_element_destroy(param1_p);
    mpl_param_element_destroy(param2_p);
    return (-1);
  }

  mpl_param_element_destroy(param1_p);

  return 0;
}


static int tc_param_id_get_str(void)
{
    int index;
    test_paramid_t id;
    const char* str;

  for (index=0; index < mpl_param_num_parameters(TEST_PARAM_SET_ID); index++)
  {
    id = mpl_param_index_to_paramid(index, TEST_PARAM_SET_ID);
    str = mpl_param_id_get_string(id);
    if ((NULL == str) ||
        (0 == strcmp(str, "<unknown param>")))
    {
        printf("Failed 1: param %d = %s\n", id, str);
        return -1;
    }
    printf("param %d = %s\n", id, str);
  }

  printf("\n");
  str = mpl_param_id_get_string(mpl_param_last_paramid(TEST_PARAM_SET_ID) + 1);
  if ((NULL == str) ||
      (0 != strcmp(str, "<Error: unknown param>")))
  {
      printf("Failed 2: param %d = %s\n", mpl_param_last_paramid(TEST_PARAM_SET_ID) + 1, str);
      return -1;
  }

  str = mpl_param_id_get_string(7);
  if ((NULL == str) ||
      (0 != strcmp(str, "<Error: no parameter set>")))
  {
      printf("Failed 3: param %d = %s\n", 7, str);
      return -1;
  }

  return 0;
}


static int tc_param_value_get_str(void)
{
  test_paramid_t myenum = test_my_enum_val1;
  const char* str;

  /* Positive */
  str = mpl_param_value_get_string(test_paramid_my_enum,
                                       &myenum);
  if ((NULL == str) ||
      (0 != strcmp(str, "val1")))
  {
    return -1;
  }

  printf("%s: %d = %s\n",
         mpl_param_id_get_string(test_paramid_my_enum),
         myenum,
         str);

  /* Negative: unknown parameter */
  str = mpl_param_value_get_string(mpl_param_last_paramid(TEST_PARAM_SET_ID) + 1,
                                   &myenum);
  if ((NULL == str) ||
      (0 != strcmp(str, "<unknown param>")))
  {
    return -1;
  }

  printf("param %d = %s\n", mpl_param_last_paramid(TEST_PARAM_SET_ID) + 1, str);

  /* Negative: Ilegal value */
  myenum = test_max_value_my_enum + 1;
  str = mpl_param_value_get_string(test_paramid_my_enum,
                                   &myenum);
  if ((NULL == str) ||
      (0 != strcmp(str, "<unknown value>")))
  {
    return -1;
  }

  printf("%s: %d = %s\n",
         mpl_param_id_get_string(test_paramid_my_enum),
         myenum,
         str);

  /* Negative: unknown parameter set */
  str = mpl_param_value_get_string(3,
                                   &myenum);
  if ((NULL == str) ||
      (0 != strcmp(str, "<Error: no parameter set>")))
  {
    return -1;
  }

  return 0;
}


static int tc_get_type_string(void)
{
  test_paramid_t id;
  int index;
  const char* str;

  for (index=0; index < mpl_param_num_parameters(TEST_PARAM_SET_ID); index++)
  {
    id = mpl_param_index_to_paramid(index, TEST_PARAM_SET_ID);
    str = mpl_param_id_get_type_string(id);
    if ((NULL == str) ||
        (0 == strcmp(str, "<unknown param>")))
    {
      return -1;
    }
    printf("param %d = %s\n", id, str);
  }

  str = mpl_param_id_get_type_string(mpl_param_last_paramid(TEST_PARAM_SET_ID) + 1);
  if ((NULL == str) ||
      (0 != strcmp(str, "<unknown param>")))
  {
    return -1;
  }

  printf("param %d = %s\n", mpl_param_last_paramid(TEST_PARAM_SET_ID) + 1, str);

  str = mpl_param_id_get_type_string(test_paramid_myint);
  if ((NULL == str) ||
      (strcmp(str, "int")))
  {
    return -1;
  }

  str = mpl_param_id_get_type_string(5);
  if ((NULL == str) ||
      (strcmp(str, "<Error: no parameter set>")))
  {
    return -1;
  }

  return 0;
}

static int tc_get_type(void)
{
  test_paramid_t id;
  int index;
  mpl_type_t t;

  for (index=0; index < mpl_param_num_parameters(TEST_PARAM_SET_ID); index++)
  {
    id = mpl_param_index_to_paramid(index, TEST_PARAM_SET_ID);
    t = mpl_param_id_get_type(id);
    if (t == mpl_type_invalid)
    {
      return -1;
    }
  }

  t = mpl_param_id_get_type(mpl_param_last_paramid(TEST_PARAM_SET_ID) + 1);
  if (t != mpl_type_invalid)
  {
    return -1;
  }

  t = mpl_param_id_get_type(test_paramid_myint);
  if (t != mpl_type_int)
  {
    return -1;
  }

  t = mpl_param_id_get_type(4);
  if (t != mpl_type_invalid)
  {
    return -1;
  }

  return 0;
}


static int tc_get_args_escape_1(void)
{
  mpl_arg_t args[20]={{NULL},{NULL}};
  int numargs;
  char buf[100];
  int i;

  strcpy( buf, "mykey0=normal0 mykey1=value\\ with\\ spaces\\  mykey2=normal2");

  numargs = mpl_get_args(args, 20, buf,'=', ' ', '\\');

  for (i = 0; i < numargs; i++)
    printf("'%s' = '%s'\n", args[i].key_p, args[i].value_p);

  if (numargs != 3)
    return -1;

  if (strcmp(args[0].key_p, "mykey0"))
      return -1;
  if (strcmp(args[0].value_p, "normal0"))
      return -1;
  if (strcmp(args[1].key_p, "mykey1"))
      return -1;
  if (strcmp(args[1].value_p, "value\\ with\\ spaces\\ "))
    return -1;
  if (strcmp(args[2].key_p, "mykey2"))
    return -1;
  if (strcmp(args[2].value_p, "normal2"))
    return -1;

  return 0;
}

static int tc_get_args_escape_2(void)
{
  mpl_arg_t args[20]={{NULL},{NULL}};
  int numargs;
  char buf[100];
  int i;

  strcpy( buf, "mykey0~normal0 ,\t\r\n mykey1\r  \t~  \n  ? value\nwith? escapes?, \n\r , \t mykey2 \t \r \n~\t  \r \nnormal2   \n");

  numargs = mpl_get_args(args, 20, buf,'~', ',', '?');

  for (i = 0; i < numargs; i++)
    printf("'%s' = '%s'\n", args[i].key_p, args[i].value_p);

  if (numargs != 3)
    return -1;

  if (strcmp(args[0].key_p, "mykey0"))
      return -1;
  if (strcmp(args[0].value_p, "normal0"))
      return -1;
  if (strcmp(args[1].key_p, "mykey1"))
      return -1;
  if (strcmp(args[1].value_p, "? value\nwith? escapes?,"))
    return -1;
  if (strcmp(args[2].key_p, "mykey2"))
    return -1;
  if (strcmp(args[2].value_p, "normal2"))
    return -1;

  return 0;
}

static int tc_get_args_escape_3(void)
{
  mpl_arg_t args[20]={{NULL},{NULL}};
  int numargs;
  char buf[200];
  int i;

  strcpy( buf, "   my? key0=two? escapes??    \t\r\nmykey1??\r\t=three? escapes??? \t    mykey2????\t\r\n=\t\r\nfive?????     \n");

  numargs = mpl_get_args(args, 20, buf,'=', ' ', '?');

  printf("numargs = %d\n", numargs );

  for (i = 0; i < numargs; i++)
    printf("'%s' = '%s'\n", args[i].key_p, args[i].value_p);

  if (numargs != 3)
    return -1;

  if (strcmp(args[0].key_p, "my? key0"))
      return -1;
  if (strcmp(args[0].value_p, "two? escapes??"))
      return -1;
  if (strcmp(args[1].key_p, "mykey1??"))
      return -1;
  if (strcmp(args[1].value_p, "three? escapes??? "))
    return -1;
  if (strcmp(args[2].key_p, "mykey2????"))
    return -1;
  if (strcmp(args[2].value_p, "five????? "))
    return -1;

  return 0;
}

static int tc_byte_stuffing(void)
{
  mpl_arg_t args[20]={{NULL},{NULL}};
  int numargs;
  char buf[100];
  int i;

  strcpy( buf, "mykey0=normal0,mykey1=value/with\\,commas\\,and other stuff,mykey2=normal2");

  numargs = mpl_get_args(args, 20, buf,'=', ',', '\\');

  for (i = 0; i < numargs; i++)
    printf("'%s' = '%s'\n", args[i].key_p, args[i].value_p);

  if (numargs != 3)
    return -1;

  if (strcmp(args[0].key_p, "mykey0"))
      return -1;
  if (strcmp(args[0].value_p, "normal0"))
      return -1;
  if (strcmp(args[1].key_p, "mykey1"))
      return -1;
  if (strcmp(args[1].value_p, "value/with\\,commas\\,and other stuff"))
    return -1;
  if (strcmp(args[2].key_p, "mykey2"))
    return -1;
  if (strcmp(args[2].value_p, "normal2"))
    return -1;

  return 0;
}


static int tc_byte_stuffing2(void)
{
  char stringbuf[100];
  char *buf;
  int size;
  int res;
  mpl_arg_t args[20]={{NULL},{NULL}};
  int numargs;
  int i;

  strcpy( stringbuf, "val= /with,commas,");
  printf("Orig value: '%s'\n", stringbuf);

  /* Find out space need */
  packparam.id = test_paramid_mystring;
  packparam.value_p = stringbuf;

  size=mpl_param_pack(&packparam, NULL, 0);
  if (size<0)
  {
    printf("mpl_param_pack() failed\n");
    return (-1);
  }

  printf("size = %d\n", size);

  /* Alloc size including '\0' */
  buf = malloc(size+1);

  size=mpl_param_pack(&packparam, buf, size+1);
  if (size<0)
  {
    printf("mpl_param_pack() failed\n");
    free(buf);
    return (-1);
  }

  printf("%s\n", buf);

  numargs = mpl_get_args(args, 20, buf,'=', ',', '\\');

  for (i = 0; i < numargs; i++)
    printf("'%s' = '%s'\n", args[i].key_p, args[i].value_p);

  if (numargs != 1) {
      return -1;
  }

  if (mpl_param_unpack(args[0].key_p, args[0].value_p, &unpackparam_p) < 0)
  {
    printf("mpl_param_unpack() failed\n");
    free(buf);
    return (-1);
  }

  free(buf);

  printf("Received value: '%s'\n", (char*)unpackparam_p->value_p);
  res = ((unpackparam_p->id == test_paramid_mystring) &&
         !strcmp((char*)unpackparam_p->value_p, stringbuf));

  mpl_param_element_destroy(unpackparam_p);
  if (res != 1)
    return -1;

  return 0;
}

static int tc_byte_stuffing3(void)
{
  mpl_string_tuple_t st;
  mpl_string_tuple_t *st_p;
  char *buf;
  int size;
  int res;
  mpl_arg_t args[20]={{NULL},{NULL}};
  int numargs;
  int i;

  st.key_p = malloc(100 * sizeof(char));
  strcpy(st.key_p, "my,key");
  st.value_p = malloc(100 * sizeof(char));
  strcpy(st.value_p, "my,value");

  printf("Orig values: '%s' : '%s' \n", st.key_p, st.value_p);

  /* Find out space need */
  packparam.id = test_paramid_mystring_tup;
  packparam.value_p = &st;

  size=mpl_param_pack(&packparam, NULL, 0);
  if (size<0)
  {
    printf("mpl_param_pack() failed\n");
    free(st.key_p);
    free(st.value_p);
    return (-1);
  }

  printf("size = %d\n", size);

  /* Alloc size including '\0' */
  buf = malloc(size+1);

  size=mpl_param_pack(&packparam, buf, size+1);
  if (size<0)
  {
    printf("mpl_param_pack() failed\n");
    free(buf);
    free(st.key_p);
    free(st.value_p);
    return (-1);
  }

  printf("%s\n", buf);

  numargs = mpl_get_args(args, 20, buf,'=', ',', '\\');

  for (i = 0; i < numargs; i++)
    printf("'%s' = '%s'\n", args[i].key_p, args[i].value_p);

  if (numargs != 1) {
      free(st.key_p);
      free(st.value_p);
      return -1;
  }

  if (mpl_param_unpack(args[0].key_p, args[0].value_p, &unpackparam_p) < 0)
  {
    printf("mpl_param_unpack() failed\n");
    free(buf);
    free(st.key_p);
    free(st.value_p);
    return (-1);
  }

  free(buf);

  st_p = unpackparam_p->value_p;
  printf("Received values: '%s' : '%s' \n", st_p->key_p, st_p->value_p);
  res = ((unpackparam_p->id == test_paramid_mystring_tup) &&
         !strcmp(st_p->key_p, st.key_p) &&
         !strcmp(st_p->value_p, st.value_p));

  free(st.key_p);
  free(st.value_p);
  mpl_param_element_destroy(unpackparam_p);
  if (res != 1)
    return -1;

  return 0;
}

static int tc_byte_stuffing4(void)
{
  mpl_strint_tuple_t st;
  mpl_strint_tuple_t *st_p;
  char *buf;
  int size;
  int res;
  mpl_arg_t args[20]={{NULL},{NULL}};
  int numargs;
  int i;

  st.key_p = malloc(100 * sizeof(char));
  strcpy(st.key_p, "my,key");
  st.value = 77;

  printf("Orig values: '%s' : '%d' \n", st.key_p, st.value);

  /* Find out space need */
  packparam.id = test_paramid_mystrint_tup;
  packparam.value_p = &st;

  size=mpl_param_pack(&packparam, NULL, 0);
  if (size<0)
  {
    printf("mpl_param_pack() failed\n");
    free(st.key_p);
    return (-1);
  }

  printf("size = %d\n", size);

  /* Alloc size including '\0' */
  buf = malloc(size+1);

  size=mpl_param_pack(&packparam, buf, size+1);
  if (size<0)
  {
    printf("mpl_param_pack() failed\n");
    free(buf);
    free(st.key_p);
    return (-1);
  }

  printf("%s\n", buf);

  numargs = mpl_get_args(args, 20, buf,'=', ',', '\\');

  for (i = 0; i < numargs; i++)
    printf("'%s' = '%s'\n", args[i].key_p, args[i].value_p);

  if (numargs != 1) {
      free(st.key_p);
      return -1;
  }

  if (mpl_param_unpack(args[0].key_p, args[0].value_p, &unpackparam_p) < 0)
  {
    printf("mpl_param_unpack() failed\n");
    free(buf);
    free(st.key_p);
    return (-1);
  }

  free(buf);

  st_p = unpackparam_p->value_p;
  printf("Received values: '%s' : '%d' \n", st_p->key_p, st_p->value);
  res = ((unpackparam_p->id == test_paramid_mystrint_tup) &&
         !strcmp(st_p->key_p, st.key_p) &&
         (st_p->value == st.value));

  free(st.key_p);
  mpl_param_element_destroy(unpackparam_p);
  if (res != 1)
    return -1;

  return 0;
}


static int tc_byte_stuffing5(void)
{
  mpl_struint8_tuple_t st;
  mpl_struint8_tuple_t *st_p;
  char *buf;
  int size;
  int res;
  mpl_arg_t args[20]={{NULL},{NULL}};
  int numargs;
  int i;

  st.key_p = malloc(100 * sizeof(char));
  strcpy(st.key_p, "my,key");
  st.value = 88;

  printf("Orig values: '%s' : '%d' \n", st.key_p, st.value);

  /* Find out space need */
  packparam.id = test_paramid_mystruint8_tup;
  packparam.value_p = &st;

  size=mpl_param_pack(&packparam, NULL, 0);
  if (size<0)
  {
    printf("mpl_param_pack() failed\n");
    free(st.key_p);
    return (-1);
  }

  printf("size = %d\n", size);

  /* Alloc size including '\0' */
  buf = malloc(size+1);

  size=mpl_param_pack(&packparam, buf, size+1);
  if (size<0)
  {
    printf("mpl_param_pack() failed\n");
    free(buf);
    free(st.key_p);
    return (-1);
  }

  printf("%s\n", buf);

  numargs = mpl_get_args(args, 20, buf,'=', ',', '\\');

  for (i = 0; i < numargs; i++)
    printf("'%s' = '%s'\n", args[i].key_p, args[i].value_p);

  if (numargs != 1) {
      free(st.key_p);
      return -1;
  }

  if (mpl_param_unpack(args[0].key_p, args[0].value_p, &unpackparam_p) < 0)
  {
    printf("mpl_param_unpack() failed\n");
    free(buf);
    free(st.key_p);
    return (-1);
  }

  free(buf);

  st_p = unpackparam_p->value_p;
  printf("Received values: '%s' : '%d' \n", st_p->key_p, st_p->value);
  res = ((unpackparam_p->id == test_paramid_mystruint8_tup) &&
         !strcmp(st_p->key_p, st.key_p) &&
         (st_p->value == st.value));

  free(st.key_p);
  mpl_param_element_destroy(unpackparam_p);
  if (res != 1)
    return -1;

  return 0;
}

static int tc_byte_stuffing6(void)
{
  char stringbuf[100];
  char *buf;
  int size;
  int res;
  mpl_arg_t args[20]={{NULL},{NULL}};
  int numargs;
  int i;

  strcpy( stringbuf, "val,\nwith\\ \"fnutts\"\\");
  printf("Orig value: '%s'\n", stringbuf);

  /* Find out space need */
  packparam.id = test_paramid_mystring;
  packparam.value_p = stringbuf;

  size=mpl_param_pack(&packparam, NULL, 0);
  if (size<0)
  {
    printf("mpl_param_pack() failed\n");
    return (-1);
  }

  printf("size = %d\n", size);

  /* Alloc size including '\0' */
  buf = malloc(size+1);

  size=mpl_param_pack(&packparam, buf, size+1);
  if (size<0)
  {
    printf("mpl_param_pack() failed\n");
    free(buf);
    return (-1);
  }

  printf("%s\n", buf);

  numargs = mpl_get_args(args, 20, buf,'=', ',', '\\');

  for (i = 0; i < numargs; i++)
    printf("'%s' = '%s'\n", args[i].key_p, args[i].value_p);

  if (numargs != 1) {
      return -1;
  }

  if (mpl_param_unpack(args[0].key_p, args[0].value_p, &unpackparam_p) < 0)
  {
    printf("mpl_param_unpack() failed\n");
    free(buf);
    return (-1);
  }

  free(buf);

  printf("Received value: '%s'\n", (char*)unpackparam_p->value_p);
  res = ((unpackparam_p->id == test_paramid_mystring) &&
         !strcmp((char*)unpackparam_p->value_p, stringbuf));

  mpl_param_element_destroy(unpackparam_p);
  if (res != 1)
    return -1;

  return 0;
}


static int tc_compare_param(void)
{
  mpl_param_element_t *param1_p;
  mpl_param_element_t *param2_p;
  test_my_enum_t myenum = test_my_enum_val1;
  int myint = 5;
  uint8_t myuint8 = 8;
  uint32_t myuint32 = 32;
  bool mybool = true;
  char *mystring = "hello";


  /* Positive enum */
  param1_p = mpl_param_element_create_n(test_paramid_my_enum, &myenum, sizeof(myenum));
  if (NULL == param1_p)
  {
    printf("mpl_param_element_create() enum failed\n");
    return (-1);
  }

  param2_p = mpl_param_element_clone(param1_p);
  if (NULL == param2_p)
  {
    printf("mpl_param_element_clone() enum failed\n");
    mpl_param_element_destroy(param1_p);
    return (-1);
  }

  if (mpl_param_element_compare(param1_p, param2_p))
  {
    printf("mpl_param_element_compare() enum failed\n");
    mpl_param_element_destroy(param1_p);
    mpl_param_element_destroy(param2_p);
    return (-1);
  }

  mpl_param_element_destroy(param1_p);
  mpl_param_element_destroy(param2_p);

  /* Positive int */
  param1_p = mpl_param_element_create(test_paramid_myint, &myint);
  if (NULL == param1_p)
  {
    printf("mpl_param_element_create() int failed\n");
    return (-1);
  }

  param2_p = mpl_param_element_clone(param1_p);
  if (NULL == param2_p)
  {
    printf("mpl_param_element_clone() int failed\n");
    mpl_param_element_destroy(param1_p);
    return (-1);
  }

  if (mpl_param_element_compare(param1_p, param2_p))
  {
    printf("mpl_param_element_compare() int failed\n");
    mpl_param_element_destroy(param1_p);
    mpl_param_element_destroy(param2_p);
    return (-1);
  }


  mpl_param_element_destroy(param1_p);
  mpl_param_element_destroy(param2_p);

  /* Positive uint8 */
  param1_p = mpl_param_element_create(test_paramid_myuint8, &myuint8);
  if (NULL == param1_p)
  {
    printf("mpl_param_element_create() uint8 failed\n");
    return (-1);
  }

  param2_p = mpl_param_element_clone(param1_p);
  if (NULL == param2_p)
  {
    printf("mpl_param_element_clone() uint8 failed\n");
    mpl_param_element_destroy(param1_p);
    return (-1);
  }

  if (mpl_param_element_compare(param1_p, param2_p))
  {
    printf("mpl_param_element_compare() uint8 failed\n");
    mpl_param_element_destroy(param1_p);
    mpl_param_element_destroy(param2_p);
    return (-1);
  }


  mpl_param_element_destroy(param1_p);
  mpl_param_element_destroy(param2_p);

  /* Positive uint32 */
  param1_p = mpl_param_element_create(test_paramid_myuint32, &myuint32);
  if (NULL == param1_p)
  {
    printf("mpl_param_element_create() uint32 failed\n");
    return (-1);
  }

  param2_p = mpl_param_element_clone(param1_p);
  if (NULL == param2_p)
  {
    printf("mpl_param_element_clone() uint32 failed\n");
    mpl_param_element_destroy(param1_p);
    return (-1);
  }

  if (mpl_param_element_compare(param1_p, param2_p))
  {
    printf("mpl_param_element_compare() uint32 failed\n");
    mpl_param_element_destroy(param1_p);
    mpl_param_element_destroy(param2_p);
    return (-1);
  }

  mpl_param_element_destroy(param1_p);
  mpl_param_element_destroy(param2_p);

  /* Positive bool */
  param1_p = mpl_param_element_create(test_paramid_mybool, &mybool);
  if (NULL == param1_p)
  {
    printf("mpl_param_element_create() bool failed\n");
    return (-1);
  }

  param2_p = mpl_param_element_clone(param1_p);
  if (NULL == param2_p)
  {
    printf("mpl_param_element_clone() bool failed\n");
    mpl_param_element_destroy(param1_p);
    return (-1);
  }

  if (mpl_param_element_compare(param1_p, param2_p))
  {
    printf("mpl_param_element_compare() bool failed\n");
    mpl_param_element_destroy(param1_p);
    mpl_param_element_destroy(param2_p);
    return (-1);
  }

  mpl_param_element_destroy(param1_p);
  mpl_param_element_destroy(param2_p);

  /* Positive string */
  param1_p = mpl_param_element_create(test_paramid_mystring, mystring);
  if (NULL == param1_p)
  {
    printf("mpl_param_element_create() string failed\n");
    return (-1);
  }

  param2_p = mpl_param_element_clone(param1_p);
  if (NULL == param2_p)
  {
    printf("mpl_param_element_clone() string failed\n");
    mpl_param_element_destroy(param1_p);
    return (-1);
  }

  if (mpl_param_element_compare(param1_p, param2_p))
  {
    printf("mpl_param_element_compare() string failed\n");
    mpl_param_element_destroy(param1_p);
    mpl_param_element_destroy(param2_p);
    return (-1);
  }

  /* Negative: nothing to compare */
  if (0 == mpl_param_element_compare(NULL, param2_p))
  {
    printf("mpl_param_element_compare() unexpectedly succeeded\n");
    mpl_param_element_destroy(param1_p);
    mpl_param_element_destroy(param2_p);
    return (-1);
  }

  if (0 == mpl_param_element_compare(param1_p, NULL))
  {
    printf("mpl_param_element_compare() unexpectedly succeeded\n");
    mpl_param_element_destroy(param1_p);
    mpl_param_element_destroy(param2_p);
    return (-1);
  }

  if (0 == mpl_param_element_compare(NULL, NULL))
  {
    printf("mpl_param_element_compare() unexpectedly succeeded\n");
    mpl_param_element_destroy(param1_p);
    mpl_param_element_destroy(param2_p);
    return (-1);
  }


  mpl_param_element_destroy(param1_p);
  mpl_param_element_destroy(param2_p);

  /* Negative: wrong ID */
  param1_p = mpl_param_element_create(test_paramid_myint, &myint);
  if (NULL == param1_p)
  {
    printf("mpl_param_element_create() int failed\n");
    return (-1);
  }

  param2_p = mpl_param_element_clone(param1_p);
  if (NULL == param2_p)
  {
    printf("mpl_param_element_clone() int failed\n");
    mpl_param_element_destroy(param1_p);
    return (-1);
  }

  param2_p->id = test_paramid_mystring;
  if (0 == mpl_param_element_compare(param1_p, param2_p))
  {
    printf("mpl_param_element_compare() (wrong id) unexpectedly succeeded\n");
    mpl_param_element_destroy(param1_p);
    mpl_param_element_destroy(param2_p);
    return (-1);
  }

  mpl_param_element_destroy(param1_p);
  mpl_param_element_destroy(param2_p);


  /* Negative: wrong value */
  param1_p = mpl_param_element_create(test_paramid_myint, &myint);
  if (NULL == param1_p)
  {
    printf("mpl_param_element_create() int failed\n");
    return (-1);
  }

  param2_p = mpl_param_element_clone(param1_p);
  if (NULL == param2_p)
  {
    printf("mpl_param_element_clone() int failed\n");
    mpl_param_element_destroy(param1_p);
    return (-1);
  }

  *((int*) param2_p->value_p) = 77;
  if (0 == mpl_param_element_compare(param1_p, param2_p))
  {
    printf("mpl_param_element_compare() (wrong value) unexpectedly succeeded\n");
    mpl_param_element_destroy(param1_p);
    mpl_param_element_destroy(param2_p);
    return (-1);
  }

  mpl_param_element_destroy(param1_p);
  mpl_param_element_destroy(param2_p);

  return 0;
}

static int tc_element_create_negative(void)
{
  mpl_param_element_t *param1_p;
  int i = 123;
  char str[100];
  wchar_t *wstr;
  int len;
  mpl_uint8_array_t a8;
  mpl_uint16_array_t a16;
  mpl_uint32_array_t a32;
  mpl_string_tuple_t st;

  /* Create empty illegal id */
  param1_p = mpl_param_element_create_empty(mpl_param_first_paramid(TEST_PARAM_SET_ID) - 1);
  if (NULL != param1_p)
  {
    printf("mpl_param_element_create_empty() unexpectedly succeeded\n");
    return (-1);
  }

  printf("errno = %d\n", mpl_get_errno());

  /* Create with illegal length */
  param1_p = mpl_param_element_create_n(test_paramid_myint, &i, 2);
  if (NULL != param1_p)
  {
    printf("mpl_param_element_create_n() unexpectedly succeeded\n");
    return (-1);
  }

  printf("errno = %d\n", mpl_get_errno());

  /* Create with illegal length */
  param1_p = mpl_param_element_create_n(test_paramid_myuint8, &i, 0);
  if (NULL != param1_p)
  {
    printf("mpl_param_element_create_n() unexpectedly succeeded\n");
    return (-1);
  }

  printf("errno = %d\n", mpl_get_errno());

  /* Create with illegal length */
  param1_p = mpl_param_element_create_n(test_paramid_myuint16, &i, 1);
  if (NULL != param1_p)
  {
    printf("mpl_param_element_create_n() unexpectedly succeeded\n");
    return (-1);
  }

  printf("errno = %d\n", mpl_get_errno());

  /* Create with illegal length */
  param1_p = mpl_param_element_create_n(test_paramid_myuint32, &i, 3);
  if (NULL != param1_p)
  {
    printf("mpl_param_element_create_n() unexpectedly succeeded\n");
    return (-1);
  }

  printf("errno = %d\n", mpl_get_errno());

  /* Create with illegal length */
  param1_p = mpl_param_element_create_n(test_paramid_mybool, &i, 0);
  if (NULL != param1_p)
  {
    printf("mpl_param_element_create_n() unexpectedly succeeded\n");
    return (-1);
  }

  printf("errno = %d\n", mpl_get_errno());

  /* Create with illegal length */
  param1_p = mpl_param_element_create_n(test_paramid_mybool8, &i, 0);
  if (NULL != param1_p)
  {
    printf("mpl_param_element_create_n() unexpectedly succeeded\n");
    return (-1);
  }

  printf("errno = %d\n", mpl_get_errno());

  /* Create with illegal length */
  strcpy(str, "This is a long string, it is longer than 20 bytes");
  param1_p = mpl_param_element_create_n(test_paramid_mystring, str, strlen(str));
  if (NULL != param1_p)
  {
    printf("mpl_param_element_create_n() unexpectedly succeeded\n");
    return (-1);
  }

  printf("errno = %d\n", mpl_get_errno());

  /* Create with illegal length */
  wstr = malloc(100 * sizeof(wchar_t));
  wcscpy(wstr, L"This is a long string, it is longer than 20 bytes");
  len = wcslen(wstr);
  param1_p = mpl_param_element_create_n(test_paramid_mywstring, wstr, len);
  if (NULL != param1_p)
  {
    printf("mpl_param_element_create_n() unexpectedly succeeded\n");
    return (-1);
  }

  printf("errno = %d\n", mpl_get_errno());

  free(wstr);

  /* Create with illegal length */
  param1_p = mpl_param_element_create_n(test_paramid_my_enum, &i, 2);
  if (NULL != param1_p)
  {
    printf("mpl_param_element_create_n() unexpectedly succeeded\n");
    return (-1);
  }

  printf("errno = %d\n", mpl_get_errno());

  /* Create with illegal length */
  param1_p = mpl_param_element_create_n(test_paramid_my_enum8, &i, 0);
  if (NULL != param1_p)
  {
    printf("mpl_param_element_create_n() unexpectedly succeeded\n");
    return (-1);
  }

  printf("errno = %d\n", mpl_get_errno());

  /* Create with illegal length */
  a8.arr_p = malloc(100 * sizeof(uint8_t));

  a8.len = 30;
  for (i = 0; i < 30; i++)
  {
    a8.arr_p[i] = 255 - i;
  }

  param1_p = mpl_param_element_create(test_paramid_myuint8_arr, &a8);
  if (NULL != param1_p)
  {
    printf("mpl_param_element_create_n() unexpectedly succeeded\n");
    return (-1);
  }

  printf("errno = %d\n", mpl_get_errno());

  free(a8.arr_p);

  /* Create with illegal length */
  a16.arr_p = malloc(100 * sizeof(uint16_t));

  a16.len = 30;
  for (i = 0; i < 30; i++)
  {
    a16.arr_p[i] = 255 - i;
  }

  param1_p = mpl_param_element_create_n(test_paramid_myuint16_arr, &a16, 100);
  if (NULL != param1_p)
  {
    printf("mpl_param_element_create_n() unexpectedly succeeded\n");
    return (-1);
  }

  printf("errno = %d\n", mpl_get_errno());

  free(a16.arr_p);

  /* Create with illegal length */
  a32.arr_p = malloc(100 * sizeof(uint32_t));

  a32.len = 30;
  for (i = 0; i < 30; i++)
  {
    a32.arr_p[i] = 255 - i;
  }

  param1_p = mpl_param_element_create_n(test_paramid_myuint32_arr, &a32, 100);
  if (NULL != param1_p)
  {
    printf("mpl_param_element_create_n() unexpectedly succeeded\n");
    return (-1);
  }

  printf("errno = %d\n", mpl_get_errno());

  free(a32.arr_p);

  /* Create with illegal length */
  st.key_p = "This is a long key string, it is more than 25 bytes";
  st.value_p = "This is a long value string, it is more than 25 bytes";
  param1_p = mpl_param_element_create_n(test_paramid_mystring_tup, &st, 100);
  if (NULL != param1_p)
  {
    printf("mpl_param_element_create_n() unexpectedly succeeded\n");
    return (-1);
  }

  printf("errno = %d\n", mpl_get_errno());

  /* Create with illegal value */
  i = 10000;
  param1_p = mpl_param_element_create_n(test_paramid_myint, &i, sizeof(i));
  if (NULL != param1_p)
  {
    printf("mpl_param_element_create_n() unexpectedly succeeded\n");
    return (-1);
  }

  printf("errno = %d\n", mpl_get_errno());

  return 0;
}


static int tc_config_read(void)
{
  FILE *fp;
  char *config_path = CONFIG_FILE;
  mpl_param_element_t *param_p;

  fp = fopen(config_path, "w");
  if (NULL == fp)
  {
    printf("Could not open config file for writing\n");
    return -1;
  }

  fprintf(fp, "mystring=hallo\nmybool=true\nmyuint8=0x13\nmyuint8=0x12\nmyuint32=1234\nmystring_tup=eth0:10.2.3.4\nmystring_tup=eth*:10.10.10.10");
  fclose(fp);

  if (mpl_config_read_config(config_path, &test_config, TEST_PARAM_SET_ID))
  {
    printf("Config read failed\n");
    return -1;
  }

  /* mystring */
  param_p = mpl_config_get_para(test_paramid_mystring, &test_config);
  if (NULL == param_p)
  {
    printf("Config parameter string not found\n");
    return -1;
  }
  if (strcmp(MPL_GET_VALUE_PTR_FROM_PARAM_ELEMENT(char*, param_p), "hallo"))
  {
    printf("Parameter string value mismatch\n");
    return -1;
  }
  printf("mystring\n");

  /* mybool */
  param_p = mpl_config_get_para(test_paramid_mybool, &test_config);
  if (NULL == param_p)
  {
    printf("Config parameter bool not found\n");
    return -1;
  }
  if (MPL_GET_VALUE_FROM_PARAM_ELEMENT(bool, param_p) != true)
  {
    printf("Parameter bool value mismatch\n");
    return -1;
  }
  printf("mybool\n");

  /* myuint8 */
  param_p = mpl_config_get_para(test_paramid_myuint8, &test_config);
  if (NULL == param_p)
  {
    printf("Config parameter uint8 not found\n");
    return -1;
  }
  if (MPL_GET_VALUE_FROM_PARAM_ELEMENT(uint8_t, param_p) != 0x12)
  {
    printf("Parameter uint8 value mismatch\n");
    return -1;
  }
  printf("myuint8\n");

  param_p = mpl_param_list_find(test_paramid_myuint8, test_config);
  param_p = mpl_param_list_find_next(test_paramid_myuint8, param_p);
  if (MPL_GET_VALUE_FROM_PARAM_ELEMENT(uint8_t, param_p) != 0x13)
  {
    printf("Parameter uint8 value mismatch\n");
    return -1;
  }

  /* myuint32 */
  param_p = mpl_config_get_para(test_paramid_myuint32, &test_config);
  if (NULL == param_p)
  {
    printf("Config parameter uint32 not found\n");
    return -1;
  }
  if (MPL_GET_VALUE_FROM_PARAM_ELEMENT(uint32_t, param_p) != 1234)
  {
    printf("Parameter uint32 value mismatch\n");
    return -1;
  }
  printf("myuint32\n");


  /* int (default) */
  param_p = mpl_config_get_para(test_paramid_myint, &test_config);
  if (NULL == param_p)
  {
    printf("Config parameter int not found\n");
    return -1;
  }
  if (MPL_GET_VALUE_FROM_PARAM_ELEMENT(int, param_p) != 99)
  {
    printf("Parameter int value mismatch 1: got %d\n", MPL_GET_PARAM_VALUE_FROM_LIST(int, test_paramid_myint, test_config));
    return -1;
  }
  printf("myuint\n");

  /* myenum (default) */
  param_p = mpl_config_get_para(test_paramid_my_enum, &test_config);
  if (NULL == param_p)
  {
    printf("Config parameter enum not found\n");
    return -1;
  }
  if (MPL_GET_VALUE_FROM_PARAM_ELEMENT(test_my_enum_t, param_p) != test_my_enum_val1)
  {
    printf("Parameter enum value mismatch 1: got %d\n", MPL_GET_PARAM_VALUE_FROM_LIST(test_my_enum_t, test_paramid_my_enum, test_config));
    return -1;
  }
  printf("myenum\n");

  /* mystring_tup (default) */
  param_p = mpl_config_get_para(test_paramid_mystring_tup, &test_config);
  if (NULL == param_p)
  {
    printf("Config parameter string tup not found\n");
    return -1;
  }
  if (strcmp(MPL_GET_VALUE_PTR_FROM_PARAM_ELEMENT(mpl_string_tuple_t*, param_p)->value_p, "10.10.10.10"))
  {
    printf("Parameter value string mismatch 1, got '%s'\n", MPL_GET_VALUE_PTR_FROM_PARAM_ELEMENT(mpl_string_tuple_t*, param_p)->value_p);
    return -1;
  }
  printf("mystring_tup\n");

  param_p = mpl_config_tuple_key_get_para(test_paramid_mystring_tup, "eth0", "eth*", &test_config);
  if (NULL == param_p)
  {
    printf("Config parameter string tup not found\n");
    return -1;
  }
  if (strcmp(MPL_GET_VALUE_PTR_FROM_PARAM_ELEMENT(mpl_string_tuple_t*, param_p)->value_p, "10.2.3.4"))
  {
    printf("Parameter value string mismatch 2, got '%s'\n", MPL_GET_VALUE_PTR_FROM_PARAM_ELEMENT(mpl_string_tuple_t*, param_p)->value_p);
    return -1;
  }
  printf("mystring_tup key get\n");

  param_p = mpl_config_tuple_key_get_para(test_paramid_mystring_tup, "eth1", "eth*", &test_config);
  if (NULL == param_p)
  {
    printf("Config parameter string tup not found\n");
    return -1;
  }
  if (strcmp(MPL_GET_VALUE_PTR_FROM_PARAM_ELEMENT(mpl_string_tuple_t*, param_p)->value_p, "10.10.10.10"))
  {
    printf("Parameter value string mismatch 3, got '%s'\n", MPL_GET_VALUE_PTR_FROM_PARAM_ELEMENT(mpl_string_tuple_t*, param_p)->value_p);
    return -1;
  }
  printf("mystring_tup wildcard get\n");

  if (MPL_GET_PARAM_VALUE_FROM_LIST(test_my_enum_t, test_paramid_my_enum, test_config) != test_my_enum_val1)
  {
    printf("Parameter enum value mismatch\n");
    return -1;
  }

  mpl_config_reset(&test_config);
  if (test_config != NULL)
    return -1;

  fp = fopen(config_path, "w");
  if (NULL == fp)
  {
    printf("Could not open config file for writing\n");
    return -1;
  }

  fprintf(fp, "mystring=hallo\nmybool=true\nmyuint8=0x13\nmyuint8=0x12\nmyuint32=1234\nmyint=55\nmyenum=test_my_enum_val2\nmystring_tup=eth0:10.2.3.4\nmystring_tup=eth*:10.10.10.10");
  fclose(fp);

  if (mpl_config_read_config(config_path, &test_config, TEST_PARAM_SET_ID))
  {
    printf("Config read failed\n");
    return -1;
  }

  /* int (override default) */
  param_p = mpl_config_get_para(test_paramid_myint, &test_config);
  if (NULL == param_p)
  {
    printf("Config parameter int not found\n");
    return -1;
  }
  if (MPL_GET_VALUE_FROM_PARAM_ELEMENT(int, param_p) != 55)
  {
    printf("Parameter int value mismatch 2: got %d\n", MPL_GET_PARAM_VALUE_FROM_LIST(int, test_paramid_myint, test_config));
    //return -1;
  }
  printf("myuint\n");

  /* myenum (override default not allowed (allow config not allowed)) */
  param_p = mpl_config_get_para(test_paramid_my_enum, &test_config);
  if (NULL == param_p)
  {
    printf("Config parameter enum not found\n");
    return -1;
  }
  if (MPL_GET_VALUE_FROM_PARAM_ELEMENT(test_my_enum_t, param_p) != test_my_enum_val1)
  {
    printf("Parameter enum value mismatch 2: got %d\n", MPL_GET_PARAM_VALUE_FROM_LIST(test_my_enum_t, test_paramid_my_enum, test_config));
    //return -1;
  }
  printf("myenum\n");

  mpl_config_reset(&test_config);
  if (test_config != NULL)
    return -1;


  fp = fopen(config_path, "w");
  if (NULL == fp)
  {
    printf("Could not open config file for writing\n");
    return -1;
  }

  fprintf(fp, "mystring=hallo\nmybool=true\nmylist1={myuint8=0x13,myuint8=0x12,myuint32=1234}\nmyint=55\nmyenum=test_my_enum_val2\nmystring_tup=eth0:10.2.3.4\nmystring_tup=eth*:10.10.10.10");
  fclose(fp);

  if (mpl_config_read_config(config_path, &test_config, TEST_PARAM_SET_ID))
  {
    printf("Config read failed\n");
    return -1;
  }

  if (mpl_list_len(MPL_GET_PARAM_VALUE_PTR_FROM_LIST(mpl_list_t*,test_paramid_mylist1,test_config)) != 3)
  {
    printf("List not correct length\n");
    return -1;
  }
  
  mpl_config_reset(&test_config);
  if (test_config != NULL)
    return -1;

  return 0;
}

static void config_log_func(const char *logstring_p)
{
    printf("%s\n", logstring_p);
}
static int tc_config_merge(void)
{
  FILE *fp;
  char *config_path1;
  char *config_path2;
  mpl_param_element_t *param_p;
  mpl_config_t this_config;
  mpl_config_t this_additional_config;

  config_path1 = CONFIG_FILE;
  fp = fopen(config_path1, "w");
  if (NULL == fp)
  {
    printf("Could not open config file for writing\n");
    return -1;
  }

  fprintf(fp, "mystring=hallo\nmybool=true\nmyuint8=0x12\nmyuint32=1234\nmylist3[1]={myint=18,mystring=\"this world\",mybool=true}");
  fclose(fp);

  mpl_config_init(&this_config);
  if (mpl_config_read_config(config_path1, &this_config, TEST_PARAM_SET_ID))
  {
    printf("Config read failed\n");
    return -1;
  }

  config_path2 = CONFIG_FILE_ADDITIONAL;
  fp = fopen(config_path2, "w");
  if (NULL == fp)
  {
    printf("Could not open config file for writing\n");
    return -1;
  }

  fprintf(fp, "mysint8=-8\nmyuint32=4321\nmylist3[1]={myint=8,mystring=\"my world\",mybool=false}\n"
          "mylist3[2]={myint=1,mystring=\"your world\",mybool=true}");
  fclose(fp);

  mpl_config_init(&this_additional_config);
  if (mpl_config_read_config(config_path2, &this_additional_config, TEST_PARAM_SET_ID))
  {
    printf("Config read failed\n");
    return -1;
  }

  config_log_func("#======== this config start ========");
  (void) mpl_config_write_config_bl_func(config_log_func,
                                         this_config,
                                         TEST_PARAM_SET_ID,
                                         NULL);
  config_log_func("#======== this config end ========");
  config_log_func("#======== this additional config  start ========");
  (void) mpl_config_write_config_bl_func(config_log_func,
                                         this_additional_config,
                                         TEST_PARAM_SET_ID,
                                         NULL);
  config_log_func("#======== this additional config end ========");

  /* mystring */
  param_p = mpl_config_get_para(test_paramid_mystring, &this_config);
  if (NULL == param_p)
  {
    printf("Config parameter string not found\n");
    return -1;
  }
  if (strcmp(MPL_GET_VALUE_PTR_FROM_PARAM_ELEMENT(char*, param_p), "hallo"))
  {
    printf("Parameter string value mismatch\n");
    return -1;
  }
  printf("before: mystring\n");

  /* mybool */
  param_p = mpl_config_get_para(test_paramid_mybool, &this_config);
  if (NULL == param_p)
  {
    printf("Config parameter bool not found\n");
    return -1;
  }
  if (!MPL_GET_VALUE_FROM_PARAM_ELEMENT(bool, param_p))
  {
    printf("Parameter bool value mismatch\n");
    return -1;
  }
  printf("before: mybool\n");

  /* myuint8 */
  param_p = mpl_config_get_para(test_paramid_myuint8, &this_config);
  if (NULL == param_p)
  {
    printf("Config parameter uint8 not found\n");
    return -1;
  }
  if (MPL_GET_VALUE_FROM_PARAM_ELEMENT(uint8_t, param_p) != 0x12)
  {
    printf("Parameter uint8 value mismatch\n");
    return -1;
  }
  printf("before: myuint8\n");

  /* myuint32 */
  param_p = mpl_config_get_para(test_paramid_myuint32, &this_config);
  if (NULL == param_p)
  {
    printf("Config parameter uint32 not found\n");
    return -1;
  }
  if (MPL_GET_VALUE_FROM_PARAM_ELEMENT(uint32_t, param_p) != 1234)
  {
    printf("Parameter uint32 value mismatch\n");
    return -1;
  }
  printf("before: myuint32\n");

  /* mylist3[1] */
  param_p = mpl_config_get_para_tag(test_paramid_mylist3, 1, &this_config);
  if (NULL == param_p)
  {
    printf("Config parameter mylist3[1] not found\n");
    return -1;
  }

  if(MPL_GET_PARAM_VALUE_FROM_LIST(int,test_paramid_myint,param_p->value_p) != 18)
  {
    printf("Parameter mylist3[1].myint value mismatch\n");
    return -1;
  }
  if(!MPL_GET_PARAM_VALUE_FROM_LIST(bool,test_paramid_mybool,param_p->value_p))
  {
    printf("Parameter mylist3[1].mybool value mismatch\n");
    return -1;
  }
  if (!strcmp(MPL_GET_PARAM_VALUE_PTR_FROM_LIST(char*, test_paramid_mystring, param_p->value_p), "this world"))
  {
    printf("Parameter mylist3[1].mystring value mismatch %s\n",
           MPL_GET_PARAM_VALUE_PTR_FROM_LIST(char*, test_paramid_mystring, param_p->value_p));
    return -1;
  }
  printf("before: mylist3[1]\n");

  /* mysint8, not yet in this_config */
  param_p = mpl_config_get_para(test_paramid_mysint8, &this_config);
  if (NULL != param_p)
  {
    printf("Unexpectedly Config parameter sint8 found\n");
    return -1;
  }
  /* mylist3[2], not yet in this_config */
  param_p = mpl_config_get_para_tag(test_paramid_mylist3, 2, &this_config);
  if (NULL != param_p)
  {
    printf("Unexpectedly Config parameter mylist3[2] found\n");
    return -1;
  }

  if (mpl_config_merge(&this_config, this_additional_config) < 0) {
      printf("Config merge failed\n");
      return -1;
  }

  config_log_func("#======== this merged config  start ========");
  (void) mpl_config_write_config_bl_func(config_log_func,
                                         this_config,
                                         TEST_PARAM_SET_ID,
                                         NULL);
  config_log_func("#======== this merged config end ========");

  if (mpl_list_len(this_config) != 7)
  {
    printf("List not correct length\n");
    return -1;
  }

  /* mystring */
  param_p = mpl_config_get_para(test_paramid_mystring, &this_config);
  if (NULL == param_p)
  {
    printf("Config parameter string not found\n");
    return -1;
  }
  if (strcmp(MPL_GET_VALUE_PTR_FROM_PARAM_ELEMENT(char*, param_p), "hallo"))
  {
    printf("Parameter string value mismatch\n");
    return -1;
  }
  printf("after: mystring\n");

  /* mybool */
  param_p = mpl_config_get_para(test_paramid_mybool, &this_config);
  if (NULL == param_p)
  {
    printf("Config parameter bool not found\n");
    return -1;
  }
  if (MPL_GET_VALUE_FROM_PARAM_ELEMENT(bool, param_p) != true)
  {
    printf("Parameter bool value mismatch\n");
    return -1;
  }
  printf("after: mybool\n");

  /* myuint8 */
  param_p = mpl_config_get_para(test_paramid_myuint8, &this_config);
  if (NULL == param_p)
  {
    printf("Config parameter uint8 not found\n");
    return -1;
  }
  if (MPL_GET_VALUE_FROM_PARAM_ELEMENT(uint8_t, param_p) != 0x12)
  {
    printf("Parameter uint8 value mismatch\n");
    return -1;
  }
  printf("after: myuint8\n");

  /* myuint32 */
  param_p = mpl_config_get_para(test_paramid_myuint32, &this_config);
  if (NULL == param_p)
  {
    printf("Config parameter uint32 not found\n");
    return -1;
  }
  if (MPL_GET_VALUE_FROM_PARAM_ELEMENT(uint32_t, param_p) != 4321)
  {
    printf("Parameter uint32 value mismatch\n");
    return -1;
  }
  printf("after: myuint32\n");

  /* mysint8 */
  param_p = mpl_config_get_para(test_paramid_mysint8, &this_config);
  if (NULL == param_p)
  {
    printf("Config parameter sint8 not found\n");
    return -1;
  }
  if (MPL_GET_VALUE_FROM_PARAM_ELEMENT(sint8_t, param_p) != -8)
  {
    printf("Parameter sint8 value mismatch\n");
    return -1;
  }
  printf("after: mysint8\n");

  /* mylist3[1] */
  param_p = mpl_config_get_para_tag(test_paramid_mylist3, 1, &this_config);
  if (NULL == param_p)
  {
    printf("Config parameter mylist3[1] not found\n");
    return -1;
  }

  if(MPL_GET_PARAM_VALUE_FROM_LIST(int,test_paramid_myint,param_p->value_p) != 8)
  {
    printf("Parameter mylist3[1].myint value mismatch\n");
    return -1;
  }
  if(MPL_GET_PARAM_VALUE_FROM_LIST(bool,test_paramid_mybool,param_p->value_p))
  {
    printf("Parameter mylist3[1].mybool value mismatch\n");
    return -1;
  }
  if (!strcmp(MPL_GET_PARAM_VALUE_PTR_FROM_LIST(char*, test_paramid_mystring, param_p->value_p), "my world"))
  {
    printf("Parameter mylist3[1].mystring value mismatch %s\n",
           MPL_GET_PARAM_VALUE_PTR_FROM_LIST(char*, test_paramid_mystring, param_p->value_p));
    return -1;
  }
  printf("after: mylist3[1]\n");

  /* mylist3[2] */
  param_p = mpl_config_get_para_tag(test_paramid_mylist3, 2, &this_config);
  if (NULL == param_p)
  {
    printf("Config parameter mylist3[2] not found\n");
    return -1;
  }

  if(MPL_GET_PARAM_VALUE_FROM_LIST(int,test_paramid_myint,param_p->value_p) != 1)
  {
    printf("Parameter mylist3[2].myint value mismatch\n");
    return -1;
  }
  if(!MPL_GET_PARAM_VALUE_FROM_LIST(bool,test_paramid_mybool,param_p->value_p))
  {
    printf("Parameter mylist3[2].mybool value mismatch\n");
    return -1;
  }
  if (!strcmp(MPL_GET_PARAM_VALUE_PTR_FROM_LIST(char*, test_paramid_mystring, param_p->value_p), "your world"))
  {
    printf("Parameter mylist3[2].mystring value mismatch %s\n",
           MPL_GET_PARAM_VALUE_PTR_FROM_LIST(char*, test_paramid_mystring, param_p->value_p));
    return -1;
  }
  printf("after: mylist3[2]\n");

  mpl_config_reset(&this_config);
  mpl_config_reset(&this_additional_config);
  return 0;
}

static int tc_config_read_bl(void)
{
  FILE *fp;
  char *config_path = CONFIG_FILE;
  char *blacklist_path  = BLACKLIST_FILE;
  mpl_param_element_t *param_p;
  mpl_blacklist_t blacklist = NULL;

/* Blacklist: */
  fp = fopen(blacklist_path, "w");
  if (NULL == fp)
  {
      printf("Could not open blacklist file for writing\n");
    return -1;
  }

  fprintf(fp, "mystring\nmybool8\nmystring_tup\n");
  fclose(fp);

  if (mpl_config_read_blacklist(blacklist_path, &blacklist, TEST_PARAM_SET_ID))
  {
    printf("Config read failed\n");
    return -1;
  }

  /* Config */
  fp = fopen(config_path, "w");
  if (NULL == fp)
  {
    printf("Could not open config file for writing\n");
    return -1;
  }

  fprintf(fp, "mystring=hallo\nmybool[1]=true\nmyuint8=0x13\nmyuint8=0x12\nmyuint32=1234\nmystring_tup=eth0:10.2.3.4\nmystring_tup=eth*:10.10.10.10");
  fclose(fp);

  if (mpl_config_read_config_bl(config_path, &test_config, TEST_PARAM_SET_ID, blacklist))
  {
    printf("Config read failed\n");
    return -1;
  }

  /* mystring */
  param_p = mpl_config_get_para_bl(test_paramid_mystring, &test_config, blacklist);
  if (NULL != param_p)
  {
    printf("Config parameter string unexpectedly found\n");
    return -1;
  }
  printf("mystring\n");
  /* mybool */
  param_p = mpl_config_get_para_bl_tag(test_paramid_mybool, 1, &test_config, blacklist);
  if (NULL == param_p)
  {
    printf("Config parameter bool not found\n");
    return -1;
  }
  if (MPL_GET_VALUE_FROM_PARAM_ELEMENT(bool, param_p) != true)
  {
    printf("Parameter bool value mismatch\n");
    return -1;
  }
  printf("mybool\n");

  /* mystring_tup (default) */
  param_p = mpl_config_get_para_bl(test_paramid_mystring_tup, &test_config, blacklist);
  if (NULL != param_p)
  {
    printf("Config parameter string_tup unexpectedly found (1)\n");
    return -1;
  }
  printf("mystring_tup\n");

  param_p = mpl_config_tuple_key_get_para_bl(test_paramid_mystring_tup, "eth0", "eth*", &test_config, blacklist);
  if (NULL != param_p)
  {
    printf("Config parameter string_tup unexpectedly found (2)\n");
    return -1;
  }
  printf("mystring_tup key get\n");

  mpl_config_reset(&test_config);
  if (test_config != NULL)
    return -1;

  mpl_param_list_destroy(&blacklist);

  return 0;
}


static int tc_config_negative(void)
{
  FILE *fp;
  char *config_path = CONFIG_FILE;

  fp = fopen(config_path, "w+");
  if (NULL == fp)
  {
    printf("Could not open config file for writing\n");
    return -1;
  }

  fprintf(fp, "mystring=hallo\nmyenum=val1\n\tmybool=true;\nmystring=heider; #comment\n    mybool=true #comment\nmyuint8=0x12;myuint32=1234 # a comment\ntullball=1\n#comment\n");
  fclose(fp);

  /* No file */
  if (!mpl_config_read_config("/tmp/test_config.noexist", &test_config, TEST_PARAM_SET_ID))
  {
    printf("Unexpected success\n");
    return -1;
  }

  mpl_config_reset(&test_config);

  if (mpl_config_read_config(config_path, &test_config, TEST_PARAM_SET_ID))
  {
    printf("Config read failed\n");
    return -1;
  }

  debug_print_params(test_config);

  if (mpl_list_len(test_config) != 6)
  {
    /* myenum not configurable, tullball not existing => 5 */
    printf("Unexpected number of config parameters: %zu\n", mpl_list_len(test_config));
    return -1;
  }

  mpl_config_reset(&test_config);

  return 0;

}

static int tc_init_mpl_param(void)
{
  static mpl_param_descr_set_t my_param_descr_set;

  my_param_descr_set = *test_param_descr_set_external_p;

  if (mpl_param_init(&my_param_descr_set) < 0)
  {
    printf("mpl_param_init() failed unexpectedly\n");
    return -1;
  }

  printf("errno = %d\n", mpl_get_errno());

  if (strcmp(mpl_paramset_prefix(TEST_PARAM_SET_ID), "test") != 0)
  {
      printf("mpl_paramset_prefix() returned wrong prefix\n");
      return -1;
  }

  if (mpl_param_first_paramid(TEST_PARAM_SET_ID) != test_paramid_cmd)
  {
      printf("mpl_param_first_paramid() returned wrong id %x\n", mpl_param_first_paramid(TEST_PARAM_SET_ID));
      return -1;
  }

  if (mpl_param_last_paramid(TEST_PARAM_SET_ID) != test_paramid_mylast)
  {
      printf("mpl_param_last_paramid() returned wrong id\n");
      return -1;
  }

  if (mpl_paramset_prefix(9) != NULL)
  {
      printf("mpl_paramset_prefix() suceeded unexpectedly\n");
      return -1;
  }

  if (mpl_param_first_paramid(9) != MPL_PARAM_ID_UNDEFINED)
  {
      printf("mpl_param_first_paramid() suceeded unexpectedly\n");
      return -1;
  }

  if (mpl_param_last_paramid(9) != MPL_PARAM_ID_UNDEFINED)
  {
      printf("mpl_param_last_paramid() suceeded unexpectedly\n");
      return -1;
  }

  my_param_descr_set.param_set_id += 10;

  if (mpl_param_init(&my_param_descr_set) == 0)
  {
    printf("mpl_param_init() succeeded (2) unexpectedly\n");
    return -1;
  }

  printf("errno = %d\n", mpl_get_errno());

  my_param_descr_set.param_set_id -= 10;
  strcpy(my_param_descr_set.paramid_prefix, "dill");

  if (mpl_param_init(&my_param_descr_set) == 0)
  {
    printf("mpl_param_init() succeeded (3) unexpectedly\n");
    return -1;
  }

  return 0;

}


static int tc_pack_unpack_param_default_param_set(void)
{
  int res=0;

  /* Default set */
  if (mpl_param_unpack_param_set("mybool", "0", &unpackparam_p, TEST_PARAM_SET_ID) < 0)
  {
    printf("mpl_param_unpack_param_set() failed (1) for mybool=%s\n", "0");
    return (-1);
  }

  res = ((unpackparam_p->id == test_paramid_mybool) &&
         (*(bool*)unpackparam_p->value_p == false));
  mpl_param_element_destroy(unpackparam_p);

  if (1 != res)
    return -1;

  /* Default set explicit */
  if (mpl_param_unpack_param_set("test.mybool", "0", &unpackparam_p, TEST_PARAM_SET_ID) < 0)
  {
    printf("mpl_param_unpack_param_set() failed (2) for mybool=%s\n", "0");
    return (-1);
  }

  res = ((unpackparam_p->id == test_paramid_mybool) &&
         (*(bool*)unpackparam_p->value_p == false));
  mpl_param_element_destroy(unpackparam_p);

  if (1 != res)
    return -1;

  /* Non-default set explicit */
  if (mpl_param_unpack_param_set("tull.myint", "77", &unpackparam_p, TEST_PARAM_SET_ID) < 0)
  {
    printf("mpl_param_unpack_param_set() failed (3) for myint=%s\n", "77");
    return (-1);
  }

  res = ((unpackparam_p->id == tull_paramid_myint) &&
         (*(int*)unpackparam_p->value_p == 77));
  mpl_param_element_destroy(unpackparam_p);

  if (1 != res)
    return -1;

  /* Non-default set explicit */
  if (mpl_param_unpack_param_set("test.myint", "77", &unpackparam_p, TULL_PARAM_SET_ID) < 0)
  {
    printf("mpl_param_unpack_param_set() failed (4) for myint=%s\n", "77");
    return (-1);
  }

  res = ((unpackparam_p->id == test_paramid_myint) &&
         (*(int*)unpackparam_p->value_p == 77));
  mpl_param_element_destroy(unpackparam_p);

  if (1 != res)
    return -1;

  /* Default set negative */
  if (mpl_param_unpack("mybool", "0", &unpackparam_p) == 0)
  {
    printf("mpl_param_unpack() succeeded (1) for mybool=%s\n", "0");
    return (-1);
  }


  return 0;
}

static int tc_uint8_array(void)
{
  mpl_uint8_array_t a, a_copy;
  int i;
  int size;
  char *value_p;
  char *key_p;
  mpl_param_element_t* cloneparam_p;
  mpl_param_element_t* newparam_p;
  int copy_size;
  uint8_t staticarr[10];

  a.arr_p = malloc(100 * sizeof(uint8_t));

  a.len = 10;
  for (i = 0; i < 10; i++)
  {
    a.arr_p[i] = 255 - i;
    staticarr[i] = 255 - i;
  }

  /* Find out space need */
  packparam.id = test_paramid_myuint8_arr;
  packparam.value_p = &a;

  buflen=mpl_param_pack(&packparam, NULL, 0);
  if (buflen<0)
  {
    printf("mpl_param_pack() failed\n");
    free(a.arr_p);
    return (-1);
  }

  /* Alloc size including '\0' */
  buf = malloc(buflen+1);

  size=mpl_param_pack(&packparam, buf, buflen+1);
  if ((size<0) || (size != buflen))
  {
    printf("mpl_param_pack() failed\n");
    free(buf);
    free(a.arr_p);
    return (-1);
  }

  printf("buf = '%s', buflen = %zu\n", buf, (size_t)buflen);
  key_p = buf;
  value_p = strstr(buf, "=");
  *value_p = 0;
  value_p++;

  if (mpl_param_unpack(key_p, value_p, &unpackparam_p) < 0)
  {
    printf("mpl_param_unpack() failed\n");
    free(buf);
    free(a.arr_p);
    return (-1);
  }

  if ((newparam_p = mpl_param_element_create_uint8_array(test_paramid_myuint8_arr, staticarr, 10)) == NULL)
  {
    printf("mpl_param_element_create_uint8_array failed\n");
    free(buf);
    free(a.arr_p);
    return -1;
  }

  if (0 != mpl_param_element_compare(&packparam, newparam_p))
  {
    printf("param element mismatch\n");
    free(buf);
    free(a.arr_p);
    return -1;
  }
  mpl_param_element_destroy(newparam_p);


  if (0 != mpl_param_element_compare(&packparam, unpackparam_p))
  {
    printf("param %d value mismatch\n", packparam.id);
    free(buf);
    free(a.arr_p);
    return -1;
  }

  a.len--;

  if (0 == mpl_param_element_compare(&packparam, unpackparam_p))
  {
    printf("unexpected param %d value match\n", packparam.id);
    free(buf);
    free(a.arr_p);
    return -1;
  }
  printf("len\n");

  a.len++;
  a.arr_p[5]--;

  if (0 == mpl_param_element_compare(&packparam, unpackparam_p))
  {
    printf("unexpected param %d value match\n", packparam.id);
    free(buf);
    free(a.arr_p);
    return -1;
  }
  printf("val\n");

  a.arr_p[5]++;

  cloneparam_p = mpl_param_element_clone(&packparam);

  if (0 != mpl_param_element_compare(&packparam, cloneparam_p))
  {
    printf("param %d value mismatch\n", packparam.id);
    free(buf);
    free(a.arr_p);
    mpl_param_element_destroy(unpackparam_p);
    mpl_param_element_destroy(cloneparam_p);
    return -1;
  }
  mpl_param_element_destroy(cloneparam_p);
  printf("clone\n");

  a_copy.arr_p = malloc(100 * sizeof(uint8_t));
  size = a.len * sizeof(uint8_t);

  if ((copy_size = mpl_param_value_copy_out(&packparam, a_copy.arr_p, 100)) < 0)
  {
    printf("mpl_param_value_copy_out() failed\n");
    free(buf);
    free(a.arr_p);
    free(a_copy.arr_p);
    mpl_param_element_destroy(unpackparam_p);
    return -1;
  }

  if (copy_size != size)
  {
    printf("mpl_param_value_copy_out() failed to give correct size: %d\n", copy_size);
    free(buf);
    free(a.arr_p);
    free(a_copy.arr_p);
    mpl_param_element_destroy(unpackparam_p);
    return -1;
  }

  if (memcmp(a_copy.arr_p, a.arr_p, size) != 0)
  {
    printf("mpl_param_value_copy_out() failed to copy correctly\n");
    free(buf);
    free(a.arr_p);
    free(a_copy.arr_p);
    mpl_param_element_destroy(unpackparam_p);
    return -1;
  }

  if ((copy_size = mpl_param_value_copy_out(&packparam, a_copy.arr_p, 0)) != size)
  {
    printf("mpl_param_value_copy_out() failed\n");
    free(buf);
    free(a.arr_p);
    free(a_copy.arr_p);
    mpl_param_element_destroy(unpackparam_p);
    return -1;
  }
  printf("copy out\n");

  /* Negative: Max value check */
  if (mpl_param_unpack("test.myuint8_arr", "000000150102030405060708090a0b0c0d0e0f101112131415", &unpackparam_p) >= 0)
  {
      printf("mpl_param_unpack() unexpectedly succeeded for max value check\n");
      mpl_param_element_destroy(unpackparam_p);
      return (-1);
  }
  mpl_param_element_destroy(unpackparam_p);
  
  /* Negative: Min value check */
  if (mpl_param_unpack("test.myuint8_arr", "0000000101", &unpackparam_p) >= 0)
  {
      printf("mpl_param_unpack() unexpectedly succeeded for min value check\n");
      mpl_param_element_destroy(unpackparam_p);
      return (-1);
  }

  /* Sanity */
  if (mpl_param_unpack("test.myuint8_arr", "000000020102", &unpackparam_p) != 0)
  {
      printf("mpl_param_unpack() unexpectedly failed\n");
      return (-1);
  }

  mpl_param_element_destroy(unpackparam_p);

  free(buf);
  free(a.arr_p);
  free(a_copy.arr_p);
  return 0;
}


static int tc_uint16_array(void)
{
  mpl_uint16_array_t a, a_copy;
  int i;
  int size;
  char *value_p;
  char *key_p;
  mpl_param_element_t* cloneparam_p;
  mpl_param_element_t* newparam_p;
  uint16_t staticarr[10];
  int copy_size;

  a.arr_p = malloc(100 * sizeof(uint16_t));

  a.len = 10;
  for (i = 0; i < 10; i++)
  {
    a.arr_p[i] = 255 - i;
    staticarr[i] = 255 - i;
  }

  /* Find out space need */
  packparam.id = test_paramid_myuint16_arr;
  packparam.value_p = &a;

  buflen=mpl_param_pack(&packparam, NULL, 0);
  if (buflen<0)
  {
    printf("mpl_param_pack() failed\n");
    free(a.arr_p);
    return (-1);
  }

  /* Alloc size including '\0' */
  buf = malloc(buflen+1);

  size=mpl_param_pack(&packparam, buf, buflen+1);
  if ((size<0) || (size != buflen))
  {
    printf("mpl_param_pack() failed\n");
    free(buf);
    free(a.arr_p);
    return (-1);
  }

  printf("buf = '%s', buflen = %zu\n", buf, (size_t)buflen);
  key_p = buf;
  value_p = strstr(buf, "=");
  *value_p = 0;
  value_p++;

  if (mpl_param_unpack(key_p, value_p, &unpackparam_p) < 0)
  {
    printf("mpl_param_unpack() failed\n");
    free(buf);
    free(a.arr_p);
    return (-1);
  }

  if ((newparam_p = mpl_param_element_create_uint16_array(test_paramid_myuint16_arr, staticarr, 10)) == NULL)
  {
    printf("mpl_param_element_create_uint16_array failed\n");
    free(buf);
    free(a.arr_p);
    return -1;
  }

  if (0 != mpl_param_element_compare(&packparam, newparam_p))
  {
    printf("param element mismatch\n");
    free(buf);
    free(a.arr_p);
    return -1;
  }
  mpl_param_element_destroy(newparam_p);


  if (0 != mpl_param_element_compare(&packparam, unpackparam_p))
  {
    printf("param %d value mismatch\n", packparam.id);
    free(buf);
    free(a.arr_p);
    return -1;
  }

  a.len--;

  if (0 == mpl_param_element_compare(&packparam, unpackparam_p))
  {
    printf("unexpected param %d value match\n", packparam.id);
    free(buf);
    free(a.arr_p);
    return -1;
  }
  printf("len\n");

  a.len++;
  a.arr_p[5]--;

  if (0 == mpl_param_element_compare(&packparam, unpackparam_p))
  {
    printf("unexpected param %d value match\n", packparam.id);
    free(buf);
    free(a.arr_p);
    return -1;
  }
  printf("val\n");

  a.arr_p[5]++;

  cloneparam_p = mpl_param_element_clone(&packparam);

  if (0 != mpl_param_element_compare(&packparam, cloneparam_p))
  {
    printf("param %d value mismatch\n", packparam.id);
    free(buf);
    free(a.arr_p);
    mpl_param_element_destroy(unpackparam_p);
    mpl_param_element_destroy(cloneparam_p);
    return -1;
  }
  mpl_param_element_destroy(cloneparam_p);
  printf("clone\n");

  a_copy.arr_p = malloc(100 * sizeof(uint16_t));
  size = a.len * sizeof(uint16_t);

  if ((copy_size = mpl_param_value_copy_out(&packparam, a_copy.arr_p, 200)) < 0)
  {
    printf("mpl_param_value_copy_out() failed\n");
    free(buf);
    free(a.arr_p);
    free(a_copy.arr_p);
    mpl_param_element_destroy(unpackparam_p);
    return -1;
  }
  if (copy_size != size)
  {
    printf("mpl_param_value_copy_out() failed to give correct size: %d\n", copy_size);
    free(buf);
    free(a.arr_p);
    free(a_copy.arr_p);
    mpl_param_element_destroy(unpackparam_p);
    return -1;
  }

  if (memcmp(a_copy.arr_p, a.arr_p, size) != 0)
  {
    printf("mpl_param_value_copy_out() failed to copy correctly\n");
    free(buf);
    free(a.arr_p);
    free(a_copy.arr_p);
    mpl_param_element_destroy(unpackparam_p);
    return -1;
  }

  if ((copy_size = mpl_param_value_copy_out(&packparam, a_copy.arr_p, 0)) != size)
  {
    printf("mpl_param_value_copy_out() failed\n");
    free(buf);
    free(a.arr_p);
    free(a_copy.arr_p);
    mpl_param_element_destroy(unpackparam_p);
    return -1;
  }
  printf("copy out\n");

  mpl_param_element_destroy(unpackparam_p);

  free(buf);
  free(a.arr_p);
  free(a_copy.arr_p);
  return 0;
}


static int tc_uint32_array(void)
{
  mpl_uint32_array_t a, a_copy;
  int i;
  int size;
  char *value_p;
  char *key_p;
  mpl_param_element_t* cloneparam_p;
  mpl_param_element_t* newparam_p;
  uint32_t staticarr[10];
  int copy_size;

  a.arr_p = malloc(100 * sizeof(uint32_t));

  a.len = 10;
  for (i = 0; i < 10; i++)
  {
    a.arr_p[i] = 255 - i;
    staticarr[i] = 255 - i;
  }

  /* Find out space need */
  packparam.id = test_paramid_myuint32_arr;
  packparam.value_p = &a;

  buflen=mpl_param_pack(&packparam, NULL, 0);
  if (buflen<0)
  {
    printf("mpl_param_pack() failed\n");
    free(a.arr_p);
    return (-1);
  }

  /* Alloc size including '\0' */
  buf = malloc(buflen+1);

  size=mpl_param_pack(&packparam, buf, buflen+1);
  if ((size<0) || (size != buflen))
  {
    printf("mpl_param_pack() failed\n");
    free(buf);
    free(a.arr_p);
    return (-1);
  }

  printf("buf = '%s', buflen = %zu\n", buf, (size_t)buflen);
  key_p = buf;
  value_p = strstr(buf, "=");
  *value_p = 0;
  value_p++;

  if (mpl_param_unpack(key_p, value_p, &unpackparam_p) < 0)
  {
    printf("mpl_param_unpack() failed\n");
    free(buf);
    free(a.arr_p);
    return (-1);
  }

  if ((newparam_p = mpl_param_element_create_uint32_array(test_paramid_myuint32_arr, staticarr, 10)) == NULL)
  {
    printf("mpl_param_element_create_uint32_array failed\n");
    free(buf);
    free(a.arr_p);
    return -1;
  }

  if (0 != mpl_param_element_compare(&packparam, newparam_p))
  {
    printf("param element mismatch\n");
    free(buf);
    free(a.arr_p);
    return -1;
  }
  mpl_param_element_destroy(newparam_p);



  if (0 != mpl_param_element_compare(&packparam, unpackparam_p))
  {
    printf("param %d value mismatch\n", packparam.id);
    free(buf);
    free(a.arr_p);
    return -1;
  }

  a.len--;

  if (0 == mpl_param_element_compare(&packparam, unpackparam_p))
  {
    printf("unexpected param %d value match\n", packparam.id);
    free(buf);
    free(a.arr_p);
    return -1;
  }
  printf("len\n");

  a.len++;
  a.arr_p[5]--;

  if (0 == mpl_param_element_compare(&packparam, unpackparam_p))
  {
    printf("unexpected param %d value match\n", packparam.id);
    free(buf);
    free(a.arr_p);
    return -1;
  }
  printf("val\n");

  a.arr_p[5]++;

  cloneparam_p = mpl_param_element_clone(&packparam);

  if (0 != mpl_param_element_compare(&packparam, cloneparam_p))
  {
    printf("param %d value mismatch\n", packparam.id);
    free(buf);
    free(a.arr_p);
    mpl_param_element_destroy(unpackparam_p);
    mpl_param_element_destroy(cloneparam_p);
    return -1;
  }
  mpl_param_element_destroy(cloneparam_p);
  printf("clone\n");

  a_copy.arr_p = malloc(100 * sizeof(uint32_t));
  size = a.len * sizeof(uint32_t);

  if ((copy_size = mpl_param_value_copy_out(&packparam, a_copy.arr_p, 400)) < 0)
  {
    printf("mpl_param_value_copy_out() failed\n");
    free(buf);
    free(a.arr_p);
    free(a_copy.arr_p);
    mpl_param_element_destroy(unpackparam_p);
    return -1;
  }
  if (copy_size != size)
  {
    printf("mpl_param_value_copy_out() failed to give correct size: %d\n", copy_size);
    free(buf);
    free(a.arr_p);
    free(a_copy.arr_p);
    mpl_param_element_destroy(unpackparam_p);
    return -1;
  }

  if (memcmp(a_copy.arr_p, a.arr_p, size) != 0)
  {
    printf("mpl_param_value_copy_out() failed to copy correctly\n");
    free(buf);
    free(a.arr_p);
    free(a_copy.arr_p);
    mpl_param_element_destroy(unpackparam_p);
    return -1;
  }

  if ((copy_size = mpl_param_value_copy_out(&packparam, a_copy.arr_p, 0)) != size)
  {
    printf("mpl_param_value_copy_out() failed\n");
    free(buf);
    free(a.arr_p);
    free(a_copy.arr_p);
    mpl_param_element_destroy(unpackparam_p);
    return -1;
  }
  printf("copy out\n");

  mpl_param_element_destroy(unpackparam_p);

  free(buf);
  free(a.arr_p);
  free(a_copy.arr_p);
  return 0;
}


static int tc_string_tuple(void)
{
  mpl_string_tuple_t st, st_copy;
  int size;
  char *value_p;
  char *key_p;
  mpl_param_element_t* cloneparam_p;
  mpl_param_element_t* newparam_p;
  int copy_size;

  st.key_p = malloc(100 * sizeof(char));
  strcpy(st.key_p, "mykey");
  st.value_p = malloc(100 * sizeof(char));
  strcpy(st.value_p, "myvalue");

  /* Find out space need */
  packparam.id = test_paramid_mystring_tup;
  packparam.value_p = &st;

  buflen=mpl_param_pack(&packparam, NULL, 0);
  if (buflen<0)
  {
    printf("mpl_param_pack() failed\n");
    free(st.key_p);
    free(st.value_p);
    return (-1);
  }

  /* Alloc size including '\0' */
  buf = malloc(buflen+1);

  size=mpl_param_pack(&packparam, buf, buflen+1);
  if ((size<0) || (size != buflen))
  {
    printf("mpl_param_pack() failed\n");
    free(buf);
    free(st.key_p);
    free(st.value_p);
    return (-1);
  }

  printf("buf = '%s', buflen = %zu\n", buf, (size_t)buflen);
  key_p = buf;
  value_p = strstr(buf, "=");
  *value_p = 0;
  value_p++;

  if (mpl_param_unpack(key_p, "12345", &unpackparam_p) == 0)
  {
    printf("mpl_param_unpack() succeeded\n");
    free(buf);
    free(st.key_p);
    free(st.value_p);
    mpl_param_element_destroy(unpackparam_p);
    return (-1);
  }

  if (mpl_param_unpack(key_p, "12345:123456789012345678901", &unpackparam_p) == 0)
  {
    printf("mpl_param_unpack() succeeded\n");
    free(buf);
    free(st.key_p);
    free(st.value_p);
    mpl_param_element_destroy(unpackparam_p);
    return (-1);
  }

  if (mpl_param_unpack(key_p, "123456789012345678901:12345", &unpackparam_p) == 0)
  {
    printf("mpl_param_unpack() succeeded\n");
    free(buf);
    free(st.key_p);
    free(st.value_p);
    mpl_param_element_destroy(unpackparam_p);
    return (-1);
  }

  if (mpl_param_unpack(key_p, "12345678901234567890:12345", &unpackparam_p) != 0)
  {
    printf("mpl_param_unpack() failed\n");
    free(buf);
    free(st.key_p);
    free(st.value_p);
    return (-1);
  }
  mpl_param_element_destroy(unpackparam_p);

  if (mpl_param_unpack(key_p, value_p, &unpackparam_p) < 0)
  {
    printf("mpl_param_unpack() failed\n");
    free(buf);
    free(st.key_p);
    free(st.value_p);
    return (-1);
  }

  if ((newparam_p = mpl_param_element_create_string_tuple(test_paramid_mystring_tup, "mykey", "myvalue")) == NULL)
  {
    printf("mpl_param_element_create_string_tuple failed\n");
    free(buf);
    free(st.key_p);
    free(st.value_p);
    return -1;
  }

  if (0 != mpl_param_element_compare(&packparam, newparam_p))
  {
    printf("param value mismatch (0)\n");
    free(buf);
    free(st.key_p);
    free(st.value_p);
    mpl_param_element_destroy(newparam_p);
    return -1;
  }


  if (mpl_param_list_string_tuple_find(test_paramid_mystring_tup,
                                       "mykey",
                                       "myvalue",
                                       &newparam_p->list_entry) != newparam_p)
  {
    printf("mpl_param_list_string_tuple_find()\n");
    free(buf);
    free(st.key_p);
    free(st.value_p);
    mpl_param_element_destroy(newparam_p);
    return -1;
  }

  mpl_param_element_destroy(newparam_p);


  if (0 != mpl_param_element_compare(&packparam, unpackparam_p))
  {
    printf("param value mismatch (1)\n");
    free(buf);
    free(st.key_p);
    free(st.value_p);
    return -1;
  }

  st.key_p[2] = 'x';

  if (0 == mpl_param_element_compare(&packparam, unpackparam_p))
  {
    printf("unexpected param %d value match 1\n", packparam.id);
    free(buf);
    free(st.key_p);
    free(st.value_p);
    return -1;
  }
  printf("key\n");

  st.key_p[2] = 'k';
  st.value_p[4] = 'y';

  if (0 == mpl_param_element_compare(&packparam, unpackparam_p))
  {
    printf("unexpected param %d value match 2\n", packparam.id);
    free(buf);
    free(st.key_p);
    free(st.value_p);
    return -1;
  }

  st.key_p[2] = 'x';

  if (0 == mpl_param_element_compare(&packparam, unpackparam_p))
  {
    printf("unexpected param %d value match 3\n", packparam.id);
    free(buf);
    free(st.key_p);
    free(st.value_p);
    return -1;
  }
  printf("val\n");

  mpl_param_element_destroy(unpackparam_p);

  cloneparam_p = mpl_param_element_clone(&packparam);

  if (0 != mpl_param_element_compare(&packparam, cloneparam_p))
  {
    printf("param value mismatch (2)\n");
    free(buf);
    free(st.key_p);
    free(st.value_p);
    mpl_param_element_destroy(cloneparam_p);
    return -1;
  }
  printf("clone\n");

  mpl_param_element_destroy(cloneparam_p);

  st_copy.value_p = malloc(100 * sizeof(char));
  size = strlen(st.value_p)+1;

  if ((copy_size = mpl_param_value_copy_out(&packparam, st_copy.value_p, 100)) < 0)
  {
    printf("mpl_param_value_copy_out() failed\n");
    free(buf);
    free(st.key_p);
    free(st.value_p);
    free(st_copy.value_p);
    return -1;
  }
  if (copy_size != size)
  {
    printf("mpl_param_value_copy_out() failed to give correct size: %d\n", copy_size);
    free(buf);
    free(st.key_p);
    free(st.value_p);
    free(st_copy.value_p);
    return -1;
  }

  if (strcmp(st_copy.value_p, st.value_p) != 0)
  {
    printf("mpl_param_value_copy_out() failed to copy correctly(%s != %s)\n",
          st_copy.value_p, st.value_p);
    free(buf);
    free(st.key_p);
    free(st.value_p);
    free(st_copy.value_p);
    return -1;
  }

  if ((copy_size = mpl_param_value_copy_out(&packparam, st_copy.value_p, 0)) != size)
  {
    printf("mpl_param_value_copy_out() failed\n");
    free(buf);
    free(st.key_p);
    free(st.value_p);
    free(st_copy.value_p);
    return -1;
  }
  printf("copy out\n");

  /* Negative: Max value check */
  if (mpl_param_unpack("test.mystring_tup", "123456789012345678901:1234", &unpackparam_p) >= 0)
  {
    printf("mpl_param_unpack() unexpectedly succeeded for max value check\n");
    mpl_param_element_destroy(unpackparam_p);
    return (-1);
  }

  /* Negative: Max value check */
  if (mpl_param_unpack("test.mystring_tup", "1234:123456789012345678901", &unpackparam_p) >= 0)
  {
    printf("mpl_param_unpack() unexpectedly succeeded for max value check\n");
    mpl_param_element_destroy(unpackparam_p);
    return (-1);
  }

  /* Negative: Min value check */
  if (mpl_param_unpack("test.mystring_tup", "1:12345", &unpackparam_p) >= 0)
  {
    printf("mpl_param_unpack() unexpectedly succeeded for min value check\n");
    mpl_param_element_destroy(unpackparam_p);
    return (-1);
  }

  /* Negative: Min value check */
  if (mpl_param_unpack("test.mystring_tup", "12345:1", &unpackparam_p) >= 0)
  {
    printf("mpl_param_unpack() unexpectedly succeeded for min value check\n");
    mpl_param_element_destroy(unpackparam_p);
    return (-1);
  }

  /* Sanity check */
  if (mpl_param_unpack("test.mystring_tup", "12345:1234", &unpackparam_p) != 0)
  {
    printf("mpl_param_unpack() unexpectedly failed\n");
    return (-1);
  }
  mpl_param_element_destroy(unpackparam_p);



  free(buf);
  free(st.key_p);
  free(st.value_p);
  free(st_copy.value_p);
  return 0;
}

static int tc_param_list_tuple_key_find(void)
{
  mpl_string_tuple_t *st_p;
  mpl_param_element_t* param_p;
  mpl_list_t *list_p = NULL;

  st_p = malloc(sizeof(*st_p));
  st_p->key_p = malloc(100 * sizeof(char));
  strcpy(st_p->key_p, "mykey1");
  st_p->value_p = malloc(100 * sizeof(char));
  strcpy(st_p->value_p, "myvalue1");
  param_p = mpl_param_element_create_empty(test_paramid_mystring_tup);
  param_p->value_p = st_p;
  mpl_list_add(&list_p, &param_p->list_entry);

  st_p = malloc(sizeof(*st_p));
  st_p->key_p = malloc(100 * sizeof(char));
  strcpy(st_p->key_p, "mykey2");
  st_p->value_p = malloc(100 * sizeof(char));
  strcpy(st_p->value_p, "myvalue2");
  param_p = mpl_param_element_create_empty(test_paramid_mystring_tup);
  param_p->value_p = st_p;
  mpl_list_add(&list_p, &param_p->list_entry);

  st_p = malloc(sizeof(*st_p));
  st_p->key_p = malloc(100 * sizeof(char));
  strcpy(st_p->key_p, "mykey3");
  st_p->value_p = malloc(100 * sizeof(char));
  strcpy(st_p->value_p, "myvalue3");
  param_p = mpl_param_element_create_empty(test_paramid_mystring_tup);
  param_p->value_p = st_p;
  mpl_list_add(&list_p, &param_p->list_entry);

  st_p = malloc(sizeof(*st_p));
  st_p->key_p = malloc(100 * sizeof(char));
  strcpy(st_p->key_p, "mykey*");
  st_p->value_p = malloc(100 * sizeof(char));
  strcpy(st_p->value_p, "myvalue99");
  param_p = mpl_param_element_create_empty(test_paramid_mystring_tup);
  param_p->value_p = st_p;
  mpl_list_add(&list_p, &param_p->list_entry);

  st_p = malloc(sizeof(*st_p));
  st_p->key_p = malloc(100 * sizeof(char));
  strcpy(st_p->key_p, "mykey4");
  st_p->value_p = malloc(100 * sizeof(char));
  strcpy(st_p->value_p, "myvalue4");
  param_p = mpl_param_element_create_empty(test_paramid_mystring_tup);
  param_p->value_p = st_p;
  mpl_list_add(&list_p, &param_p->list_entry);

  param_p = mpl_param_list_tuple_key_find(test_paramid_mystring_tup, "mykey2", list_p);
  st_p = MPL_GET_VALUE_PTR_FROM_PARAM_ELEMENT(mpl_string_tuple_t*, param_p);
  printf("Search for 'mykey2', found: key_p = '%s', value_p = '%s'\n", st_p->key_p, st_p->value_p );
  if (strcmp("myvalue2", st_p->value_p))
  {
    printf("value mismatch (1)\n");
    mpl_param_list_destroy(&list_p);
    return -1;
  }

  param_p = mpl_param_list_tuple_key_find(test_paramid_mystring_tup, "mykey1", list_p);
  st_p = MPL_GET_VALUE_PTR_FROM_PARAM_ELEMENT(mpl_string_tuple_t*, param_p);
  printf("Search for 'mykey1', found: key_p = '%s', value_p = '%s'\n", st_p->key_p, st_p->value_p );
  if (strcmp("myvalue1", st_p->value_p))
  {
    printf("value mismatch (2)\n");
    mpl_param_list_destroy(&list_p);
    return -1;
  }

  param_p = mpl_param_list_tuple_key_find(test_paramid_mystring_tup, "mykey3", list_p);
  st_p = MPL_GET_VALUE_PTR_FROM_PARAM_ELEMENT(mpl_string_tuple_t*, param_p);
  printf("Search for 'mykey3', found: key_p = '%s', value_p = '%s'\n", st_p->key_p, st_p->value_p );
  if (strcmp("myvalue3", st_p->value_p))
  {
    printf("value mismatch (3)\n");
    mpl_param_list_destroy(&list_p);
    return -1;
  }

  param_p = mpl_param_list_tuple_key_find(test_paramid_mystring_tup, "mykey4", list_p);
  st_p = MPL_GET_VALUE_PTR_FROM_PARAM_ELEMENT(mpl_string_tuple_t*, param_p);
  printf("Search for 'mykey4', found: key_p = '%s', value_p = '%s'\n", st_p->key_p, st_p->value_p );
  if (strcmp("myvalue4", st_p->value_p))
  {
    printf("value mismatch (4)\n");
    mpl_param_list_destroy(&list_p);
    return -1;
  }

  param_p = mpl_param_list_tuple_key_find(test_paramid_mystring_tup, "mykey5", list_p);
  if (NULL != param_p)
  {
    printf("param_p unexpectedly not null\n");
    mpl_param_list_destroy(&list_p);
    return -1;
  }
  printf("Search for 'mykey5', not found.\n");

  param_p = mpl_param_list_tuple_key_find(test_paramid_myint_tup, "mykey5", list_p);
  if (NULL != param_p)
  {
    printf("param_p unexpectedly not null\n");
    mpl_param_list_destroy(&list_p);
    return -1;
  }

  param_p = mpl_param_list_tuple_key_find_wildcard(test_paramid_mystring_tup, "mykey3", "mykey*", list_p);
  st_p = MPL_GET_VALUE_PTR_FROM_PARAM_ELEMENT(mpl_string_tuple_t*, param_p);
  printf("Wildcard search for 'mykey3', found: key_p = '%s', value_p = '%s'\n", st_p->key_p, st_p->value_p );
  if (strcmp("myvalue3", st_p->value_p))
  {
    printf("value mismatch (5)\n");
    mpl_param_list_destroy(&list_p);
    return -1;
  }

  param_p = mpl_param_list_tuple_key_find_wildcard(test_paramid_mystring_tup, "mykey5", "mykey*", list_p);
  st_p = MPL_GET_VALUE_PTR_FROM_PARAM_ELEMENT(mpl_string_tuple_t*, param_p);
  printf("Wildcard search for 'mykey5', found: key_p = '%s', value_p = '%s'\n", st_p->key_p, st_p->value_p );
  if (strcmp("myvalue99", st_p->value_p))
  {
    printf("value mismatch (5)\n");
    mpl_param_list_destroy(&list_p);
    return -1;
  }

  if (mpl_param_list_tuple_key_find_wildcard(test_paramid_mystrint_tup, "mykey5", "mykey*", list_p) != NULL)
  {
      printf("mpl_param_list_tuple_key_find_wildcard()\n");
      return -1;
  }

  mpl_param_list_destroy(&list_p);

  return 0;
}

static int tc_pack_unpack_param_list(void)
{
  int myint = 1000;
  uint8_t myuint8 = 10;
  uint16_t myuint16 = 100;
  uint32_t myuint32 = 2000;
  char mystring[] = "hallo der";
  bool mybool = true;
  uint8_t mybool8 = 0;

  mpl_list_t *param_list_p = NULL;
  mpl_list_t *unpacked_param_list_p = NULL;

  (void) mpl_add_param_to_list(&param_list_p, test_paramid_myint, &myint);
  (void) mpl_add_param_to_list(&param_list_p, test_paramid_myuint8, &myuint8);
  (void) mpl_add_param_to_list(&param_list_p, test_paramid_myuint16, &myuint16);
  (void) mpl_add_param_to_list(&param_list_p, test_paramid_myuint32, &myuint32);
  (void) mpl_add_param_to_list(&param_list_p, test_paramid_mystring, &mystring);
  (void) mpl_add_param_to_list(&param_list_p, test_paramid_mybool, &mybool);
  (void) mpl_add_param_to_list(&param_list_p, test_paramid_mybool8, &mybool8);

  /* Negative */
  buf = NULL;
  buflen=0;
  if(mpl_param_list_pack(NULL,buf,buflen) < 0)
  {
    printf("mpl_param_list_pack() unexpectedly succeeded\n");
    if(buf != NULL)
      free(buf);
    return -1;
  }

  unpacked_param_list_p = mpl_param_list_unpack(buf);
  if (unpacked_param_list_p != NULL)
  {
    printf("mpl_param_list_unpack() unexpectedly succeeded\n");
    free(buf);
    return -1;
  }

  /* Positive */
  /* Find out space needs */
  buflen = mpl_param_list_pack(param_list_p, buf, buflen);
  if (buflen <= 0)
  {
    printf("mpl_param_list_pack() failed\n");
    if(buf != NULL)
      free(buf);
    return -1;
  }

  buf = malloc(buflen + 1);

  buflen = mpl_param_list_pack(param_list_p, buf, buflen + 1);
  if (buflen <= 0)
  {
    printf("mpl_param_list_pack() failed\n");
    if(buf != NULL)
      free(buf);
    return -1;
  }

  printf("buf = '%s', buflen = %zu\n", buf, (size_t)buflen);

  unpacked_param_list_p = mpl_param_list_unpack(buf);
  if (unpacked_param_list_p == NULL)
  {
    printf("mpl_param_list_unpack() failed\n");
    mpl_param_list_destroy(&param_list_p);
    free(buf);
    return -1;
  }

  if (mpl_compare_param_lists(unpacked_param_list_p, param_list_p))
  {
    printf("parameter list mismatch\n");
    free(buf);
    mpl_param_list_destroy(&param_list_p);
    mpl_param_list_destroy(&unpacked_param_list_p);
    return -1;
  }

  free(buf);
  mpl_param_list_destroy(&param_list_p);
  mpl_param_list_destroy(&unpacked_param_list_p);
  return 0;
}

static int tc_pack_param_list_no_prefix(void)
{
  int myint = 1000;
  uint8_t myuint8 = 10;

  mpl_list_t *param_list_p = NULL;

  (void) mpl_add_param_to_list(&param_list_p, test_paramid_myint, &myint);
  (void) mpl_add_param_to_list(&param_list_p, test_paramid_myuint8, &myuint8);

  buf = NULL;
  buflen=0;

  buflen = mpl_param_list_pack_no_prefix(param_list_p, buf, buflen);
  if (buflen <= 0)
  {
    printf("mpl_param_list_pack_no_prefix() failed\n");
    if(buf != NULL)
      free(buf);
    return -1;
  }

  buf = malloc(buflen + 1);

  if (mpl_param_list_pack_no_prefix(param_list_p, buf, buflen + 1) != buflen)
  {
    printf("mpl_param_list_pack_no_prefix() failed\n");
    if(buf != NULL)
      free(buf);
    return -1;
  }

  printf("buf = '%s', buflen = %zu\n", buf, (size_t)buflen);

  if ((buflen != 22) || strcmp("myuint8=0xa,myint=1000", buf))
  {
    printf("mpl_param_list_pack_no_prefix() failed on length or value\n");
    if(buf != NULL)
      free(buf);
    return -1;
  }

  mpl_param_list_destroy(&param_list_p);
  free(buf);

  return 0;
}


static int tc_unpack_param_list_default_param_set(void)
{
  mpl_list_t *unpacked_param_list_p = NULL;
  char buf[100];

  strcpy(buf, "myuint8=0xa,myint=1000,test.mystring=hallo,tull.myint=33");

  unpacked_param_list_p = mpl_param_list_unpack_param_set(buf, TEST_PARAM_SET_ID);
  if (unpacked_param_list_p == NULL)
  {
    printf("mpl_param_list_unpack_param_set() failed\n");
    return -1;
  }

  if (!MPL_PARAM_PRESENT_IN_LIST(test_paramid_myint,unpacked_param_list_p))
  {
    printf("myint not in list\n");
    mpl_param_list_destroy(&unpacked_param_list_p);
    return -1;
  }

  if (MPL_GET_PARAM_VALUE_FROM_LIST(int,test_paramid_myint,unpacked_param_list_p) != 1000)
  {
    printf("wrong int value\n");
    mpl_param_list_destroy(&unpacked_param_list_p);
    return -1;
  }
  printf("myint\n");

  if (!MPL_PARAM_PRESENT_IN_LIST(test_paramid_myuint8,unpacked_param_list_p))
  {
    printf("myuint8 not in list\n");
    mpl_param_list_destroy(&unpacked_param_list_p);
    return -1;
  }

  if (MPL_GET_PARAM_VALUE_FROM_LIST(uint8_t,test_paramid_myuint8,unpacked_param_list_p) != 0xa)
  {
    printf("wrong uint8 value\n");
    mpl_param_list_destroy(&unpacked_param_list_p);
    return -1;
  }
  printf("myuint8\n");

  if (!MPL_PARAM_PRESENT_IN_LIST(test_paramid_mystring,unpacked_param_list_p))
  {
    printf("mystring not in list\n");
    mpl_param_list_destroy(&unpacked_param_list_p);
    return -1;
  }

  if (strcmp(MPL_GET_PARAM_VALUE_PTR_FROM_LIST(char*,test_paramid_mystring,unpacked_param_list_p), "hallo"))
  {
    printf("wrong string value\n");
    mpl_param_list_destroy(&unpacked_param_list_p);
    return -1;
  }
  printf("test.mystring\n");

  if (!MPL_PARAM_PRESENT_IN_LIST(tull_paramid_myint,unpacked_param_list_p))
  {
    printf("tull.myint not in list\n");
    mpl_param_list_destroy(&unpacked_param_list_p);
    return -1;
  }

  if (MPL_GET_PARAM_VALUE_FROM_LIST(int,tull_paramid_myint,unpacked_param_list_p) != 33)
  {
    printf("wrong int value\n");
    mpl_param_list_destroy(&unpacked_param_list_p);
    return -1;
  }
  printf("tull.myint\n");

  mpl_param_list_destroy(&unpacked_param_list_p);

  return 0;
}


static int tc_list_append(void)
{
  int myint = 1000;
  uint8_t myuint8 = 10;
  uint16_t myuint16 = 100;
  uint32_t myuint32 = 2000;
  char mystring[] = "hallo der";
  bool mybool = true;
  uint8_t mybool8 = 0;
  int i;

  mpl_list_t *list1_p = NULL;
  mpl_list_t *list2_p = NULL;
  mpl_list_t *list_p = NULL;
  mpl_param_element_t* param_elem1_p;

  (void) mpl_add_param_to_list(&list1_p, test_paramid_myint, &myint);
  (void) mpl_add_param_to_list(&list1_p, test_paramid_myuint8, &myuint8);
  (void) mpl_add_param_to_list(&list1_p, test_paramid_myuint16, &myuint16);
  (void) mpl_add_param_to_list(&list1_p, test_paramid_myuint32, &myuint32);

  printf("1: list1: %zu, list2: %zu\n", mpl_list_len(list1_p), mpl_list_len(list2_p));

  mpl_list_append( &list1_p, list2_p );

  printf("2: list1: %zu, list2: %zu\n", mpl_list_len(list1_p), mpl_list_len(list2_p));

  if ((mpl_list_len(list1_p) != 4) || (mpl_list_len(list2_p) != 0))
  {
    printf("1: List length error\n");
    mpl_param_list_destroy(&list1_p);
    return -1;
  }

  list2_p = NULL;

  printf("3: list1: %zu, list2: %zu\n", mpl_list_len(list1_p), mpl_list_len(list2_p));

  mpl_list_append( &list2_p, list1_p );

  printf("4: list1: %zu, list2: %zu\n", mpl_list_len(list1_p), mpl_list_len(list2_p));

  if (mpl_list_len(list1_p) != mpl_list_len(list2_p))
  {
    printf("2: List length error\n");
    mpl_param_list_destroy(&list1_p);
    return -1;
  }

  list2_p = NULL;

  (void) mpl_add_param_to_list(&list2_p, test_paramid_mystring, &mystring);
  (void) mpl_add_param_to_list(&list2_p, test_paramid_mybool, &mybool);
  (void) mpl_add_param_to_list(&list2_p, test_paramid_mybool8, &mybool8);

  printf("5: list1: %zu, list2: %zu\n", mpl_list_len(list1_p), mpl_list_len(list2_p));

  mpl_list_append( &list1_p, list2_p );

  printf("6: list1: %zu, list2: %zu\n", mpl_list_len(list1_p), mpl_list_len(list2_p));

  if (mpl_list_len(list1_p) != 7)
  {
    printf("3: List length error\n");
    mpl_param_list_destroy(&list1_p);
    return -1;
  }

  i = 0;
  MPL_LIST_FOR_EACH(list1_p, list_p)
  {
    param_elem1_p = MPL_LIST_CONTAINER(list_p, mpl_param_element_t, list_entry);

    printf("%d: %s\n", i, mpl_param_id_get_string(param_elem1_p->id));

    switch (i)
    {
    case 0:
      if (param_elem1_p->id != test_paramid_myuint32)
      {
        printf("Unexpected list element\n");
        mpl_param_list_destroy(&list1_p);
        return -1;
      }
      break;
    case 1:
      if (param_elem1_p->id != test_paramid_myuint16)
      {
        printf("Unexpected list element\n");
        mpl_param_list_destroy(&list1_p);
        return -1;
      }
      break;
    case 2:
      if (param_elem1_p->id != test_paramid_myuint8)
      {
        printf("Unexpected list element\n");
        mpl_param_list_destroy(&list1_p);
        return -1;
      }
      break;
    case 3:
      if (param_elem1_p->id != test_paramid_myint)
      {
        printf("Unexpected list element\n");
        mpl_param_list_destroy(&list1_p);
        return -1;
      }
      break;
    case 4:
      if (param_elem1_p->id != test_paramid_mybool8)
      {
        printf("Unexpected list element\n");
        mpl_param_list_destroy(&list1_p);
        return -1;
      }
      break;
    case 5:
      if (param_elem1_p->id != test_paramid_mybool)
      {
        printf("Unexpected list element\n");
        mpl_param_list_destroy(&list1_p);
        return -1;
      }
      break;
    case 6:
      if (param_elem1_p->id != test_paramid_mystring)
      {
        printf("Unexpected list element\n");
        mpl_param_list_destroy(&list1_p);
        return -1;
      }
      break;
    default:
      break;
    }
    i++;
  }

  mpl_param_list_destroy(&list1_p);

  return 0;
}

static int tc_param_list_find_all(void)
{
  int myint = 1000;
  uint8_t myuint8 = 10;
  uint16_t myuint16 = 100;
  uint32_t myuint32_1 = 110;
  uint32_t myuint32_2 = 120;
  uint32_t myuint32_3 = 130;
  uint32_t myuint32_4 = 140;
  uint32_t myuint32_5 = 150;
  char mystring[] = "hallo der";
  bool mybool = true;
  uint8_t mybool8 = 0;

  mpl_list_t *param_list_p = NULL;
  mpl_list_t *new_list_p = NULL;
  int i;
  mpl_list_t *list_p = NULL;
  mpl_param_element_t* param_elem1_p;

  (void) mpl_add_param_to_list(&param_list_p, test_paramid_myuint32, &myuint32_1);
  (void) mpl_add_param_to_list(&param_list_p, test_paramid_myint, &myint);
  (void) mpl_add_param_to_list(&param_list_p, test_paramid_myuint8, &myuint8);
  (void) mpl_add_param_to_list(&param_list_p, test_paramid_myuint32, &myuint32_2);
  (void) mpl_add_param_to_list(&param_list_p, test_paramid_myuint32, &myuint32_3);
  (void) mpl_add_param_to_list(&param_list_p, test_paramid_myuint16, &myuint16);
  (void) mpl_add_param_to_list(&param_list_p, test_paramid_mystring, &mystring);
  (void) mpl_add_param_to_list(&param_list_p, test_paramid_myuint32, &myuint32_4);
  (void) mpl_add_param_to_list(&param_list_p, test_paramid_mybool, &mybool);
  (void) mpl_add_param_to_list(&param_list_p, test_paramid_mybool8, &mybool8);
  (void) mpl_add_param_to_list(&param_list_p, test_paramid_myuint32, &myuint32_5);

  printf("1: param_list_p: %zu, new_list_p: %zu\n", mpl_list_len(param_list_p), mpl_list_len(new_list_p));

  new_list_p = mpl_param_list_find_all(test_paramid_myuint32, param_list_p);

  printf("2: param_list_p: %zu, new_list_p: %zu\n", mpl_list_len(param_list_p), mpl_list_len(new_list_p));

  if (mpl_list_len(new_list_p) != 5)
  {
    printf("List length error\n");
    mpl_param_list_destroy(&param_list_p);
    mpl_param_list_destroy(&new_list_p);
    return -1;
  }

  i = 0;
  MPL_LIST_FOR_EACH(new_list_p, list_p)
  {
    param_elem1_p = MPL_LIST_CONTAINER(list_p, mpl_param_element_t, list_entry);

    switch (i)
    {
    case 0:
      if (*(int*)param_elem1_p->value_p != myuint32_1)
      {
        printf("Unexpected list element\n");
        mpl_param_list_destroy(&param_list_p);
        mpl_param_list_destroy(&new_list_p);
        return -1;
      }
      printf("match: %d\n", *(int*)param_elem1_p->value_p);
      break;
    case 1:
      if (*(int*)param_elem1_p->value_p != myuint32_2)
      {
        printf("Unexpected list element\n");
        mpl_param_list_destroy(&param_list_p);
        mpl_param_list_destroy(&new_list_p);
        return -1;
      }
      printf("match: %d\n", *(int*)param_elem1_p->value_p);
      break;
    case 2:
      if (*(int*)param_elem1_p->value_p != myuint32_3)
      {
        printf("Unexpected list element\n");
        mpl_param_list_destroy(&param_list_p);
        mpl_param_list_destroy(&new_list_p);
        return -1;
      }
      printf("match: %d\n", *(int*)param_elem1_p->value_p);
      break;
    case 3:
      if (*(int*)param_elem1_p->value_p != myuint32_4)
      {
        printf("Unexpected list element\n");
        mpl_param_list_destroy(&param_list_p);
        mpl_param_list_destroy(&new_list_p);
        return -1;
      }
      printf("match: %d\n", *(int*)param_elem1_p->value_p);
      break;
    case 4:
      if (*(int*)param_elem1_p->value_p != myuint32_5)
      {
        printf("Unexpected list element\n");
        mpl_param_list_destroy(&param_list_p);
        mpl_param_list_destroy(&new_list_p);
        return -1;
      }
      printf("match: %d\n", *(int*)param_elem1_p->value_p);
      break;
    default:
      break;
    }
    i++;
  }

  mpl_param_list_destroy(&param_list_p);
  mpl_param_list_destroy(&new_list_p);
  return 0;
}


static int tc_param_clone_list(void)
{
  int myint = 1000;
  uint8_t myuint8 = 10;
  uint16_t myuint16 = 100;
  uint32_t myuint32 = 110;

  mpl_list_t *param_list_p = NULL;
  mpl_list_t *new_list_p = NULL;
  int i;
  mpl_list_t *list_p = NULL;
  mpl_param_element_t* param_elem1_p;

  (void) mpl_add_param_to_list(&param_list_p, test_paramid_myuint32, &myuint32);
  (void) mpl_add_param_to_list(&param_list_p, test_paramid_myuint16, &myuint16);
  (void) mpl_add_param_to_list(&param_list_p, test_paramid_myuint8, &myuint8);
  (void) mpl_add_param_to_list(&param_list_p, test_paramid_myint, &myint);

  printf("1: param_list_p: %zu, new_list_p: %zu\n", mpl_list_len(param_list_p), mpl_list_len(new_list_p));

  new_list_p = mpl_param_list_clone(param_list_p);

  printf("2: param_list_p: %zu, new_list_p: %zu\n", mpl_list_len(param_list_p), mpl_list_len(new_list_p));

  if (mpl_list_len(new_list_p) != mpl_list_len(param_list_p))
  {
    printf("List length error\n");
    mpl_param_list_destroy(&param_list_p);
    mpl_param_list_destroy(&new_list_p);
    return -1;
  }

  if (mpl_list_len(new_list_p) != 4)
  {
    printf("List length error\n");
    mpl_param_list_destroy(&param_list_p);
    mpl_param_list_destroy(&new_list_p);
    return -1;
  }

  i = 0;
  MPL_LIST_FOR_EACH(new_list_p, list_p)
  {
    param_elem1_p = MPL_LIST_CONTAINER(list_p, mpl_param_element_t, list_entry);

    printf("%d: %s\n", i, mpl_param_id_get_string(param_elem1_p->id));

    switch (i)
    {
    case 3:
      if (param_elem1_p->id != test_paramid_myuint32)
      {
        printf("Unexpected list element\n");
        mpl_param_list_destroy(&param_list_p);
        mpl_param_list_destroy(&new_list_p);
        return -1;
      }
      break;
    case 2:
      if (param_elem1_p->id != test_paramid_myuint16)
      {
        printf("Unexpected list element\n");
        mpl_param_list_destroy(&param_list_p);
        mpl_param_list_destroy(&new_list_p);
        return -1;
      }
      break;
    case 1:
      if (param_elem1_p->id != test_paramid_myuint8)
      {
        printf("Unexpected list element\n");
        mpl_param_list_destroy(&param_list_p);
        mpl_param_list_destroy(&new_list_p);
        return -1;
      }
      break;
    case 0:
      if (param_elem1_p->id != test_paramid_myint)
      {
        printf("Unexpected list element\n");
        mpl_param_list_destroy(&param_list_p);
        mpl_param_list_destroy(&new_list_p);
        return -1;
      }
      break;
    default:
      break;
    }
    i++;
  }

  mpl_param_list_destroy(&param_list_p);
  mpl_param_list_destroy(&new_list_p);
  return 0;
}

static int tc_list_write_read_file(void)
{
  FILE *fp;
  char *param_list_path = CONFIG_FILE;

  int myint = 1000;
  uint8_t myuint8 = 10;
  uint16_t myuint16 = 100;
  uint32_t myuint32 = 2000;
  char mystring[] = "hallo der";
  bool mybool = true;
  uint8_t mybool8 = 0;

  mpl_list_t *param_list_p = NULL;
  mpl_list_t *unpacked_param_list_p = NULL;
  int res;

  (void) mpl_add_param_to_list(&param_list_p, test_paramid_myint, &myint);
  (void) mpl_add_param_to_list(&param_list_p, test_paramid_myuint8, &myuint8);
  (void) mpl_add_param_to_list(&param_list_p, test_paramid_myuint16, &myuint16);
  (void) mpl_add_param_to_list(&param_list_p, test_paramid_myuint32, &myuint32);
  (void) mpl_add_param_to_list(&param_list_p, test_paramid_mystring, &mystring);
  (void) mpl_add_param_to_list(&param_list_p, test_paramid_mybool, &mybool);
  (void) mpl_add_param_to_list(&param_list_p, test_paramid_mybool8, &mybool8);


  fp = fopen(param_list_path, "w");
  if (NULL == fp)
  {
    printf("Could not open config file for writing\n");
    return -1;
  }

  if (mpl_file_write_params_no_prefix(NULL, param_list_p) >= 0)
  {
    printf("file write succeeded\n");
    return -1;
  }
  
  if (mpl_file_write_params_no_prefix(fp, param_list_p) < 0)
  {
    printf("file write failed\n");
    return -1;
  }

  fclose(fp);

  fp = fopen(param_list_path, "r");
  if (NULL == fp)
  {
    printf("Could not open config file for reading\n");
    return -1;
  }

  if (mpl_file_read_params(NULL, &unpacked_param_list_p, TEST_PARAM_SET_ID) >= 0)
  {
    printf("file read failed\n");
    return -1;
  }

  if (mpl_file_read_params(fp, &unpacked_param_list_p, TEST_PARAM_SET_ID) < 0)
  {
    printf("file read failed\n");
    return -1;
  }

  fclose(fp);

  res = mpl_compare_param_lists(param_list_p, unpacked_param_list_p);

  mpl_param_list_destroy(&param_list_p);
  mpl_param_list_destroy(&unpacked_param_list_p);

  return res;
}

static int tc_list_write_read_file_negative(void)
{
  FILE *fp;
  char *param_list_path = CONFIG_FILE;
  mpl_list_t *unpacked_param_list_p = NULL;
  int i;
  uint32_t max_bytes = 512 * 256;

  fp = fopen(param_list_path, "w");
  if (NULL == fp)
  {
    printf("Could not open config file for writing\n");
    return -1;
  }

  for (i = 1; i <= max_bytes; i++)
  {
    fprintf(fp, "a");
    if ((i % 254) == 0)
    {
      fprintf(fp, "\n");
      i++;
    }
  }

  fclose(fp);

  fp = fopen(param_list_path, "r");
  if (NULL == fp)
  {
    printf("Could not open config file for reading\n");
    return -1;
  }

  if (mpl_file_read_params(fp, &unpacked_param_list_p, TEST_PARAM_SET_ID) >= 0)
  {
    printf("file read unexpectedly succeeded\n");
    return -1;
  }

  fclose(fp);


  fp = fopen(param_list_path, "w");
  if (NULL == fp)
  {
    printf("Could not open config file for writing\n");
    return -1;
  }

  for (i = 1; i <= (max_bytes - 1); i++)
  {
    fprintf(fp, "a");
    if ((i % 254) == 0)
    {
      fprintf(fp, "\n");
      i++;
    }
  }

  fclose(fp);

  fp = fopen(param_list_path, "r");
  if (NULL == fp)
  {
    printf("Could not open config file for reading\n");
    return -1;
  }

  if (mpl_file_read_params(fp, &unpacked_param_list_p, TEST_PARAM_SET_ID) < 0)
  {
    printf("file read unexpectedly failed\n");
    return -1;
  }

  fclose(fp);



  fp = fopen(param_list_path, "w");
  if (NULL == fp)
  {
    printf("Could not open config file for writing\n");
    return -1;
  }

  for (i = 1; i <= (max_bytes - 1); i++)
  {
    fprintf(fp, "a");
    if ((i % 255) == 0)
    {
      fprintf(fp, "\n");
      i++;
    }
  }

  fclose(fp);

  fp = fopen(param_list_path, "r");
  if (NULL == fp)
  {
    printf("Could not open config file for reading\n");
    return -1;
  }

  if (mpl_file_read_params(fp, &unpacked_param_list_p, TEST_PARAM_SET_ID) >= 0)
  {
    printf("file read unexpectedly succeeded\n");
    return -1;
  }

  fclose(fp);



  return 0;
}

void log_func(const char *str_p)
{
    printf("TESTLOG: %s\n", str_p);
}

static int tc_write_read_config(void)
{
  FILE *fp;
  char *config_path = CONFIG_FILE;

  int myint = 1000;
  uint8_t myuint8 = 10;
  uint16_t myuint16 = 100;
  uint32_t myuint32 = 2000;
  char mystring[] = "hallo der";
  bool mybool = true;
  uint8_t mybool8 = 0;
  mpl_blacklist_t blacklist = NULL;
  mpl_param_element_t *elem_p;

  mpl_list_t *param_list_p = NULL;
  mpl_list_t *unpacked_param_list_p = NULL;
  int res;

  /* Config */
  (void) mpl_add_param_to_list(&param_list_p, test_paramid_myint, &myint);
  (void) mpl_add_param_to_list(&param_list_p, test_paramid_myuint8, &myuint8);
  (void) mpl_add_param_to_list(&param_list_p, test_paramid_myuint16, &myuint16);
  (void) mpl_add_param_to_list(&param_list_p, test_paramid_myuint32, &myuint32);
  (void) mpl_add_param_to_list(&param_list_p, test_paramid_mystring, &mystring);
  (void) mpl_add_param_to_list(&param_list_p, test_paramid_mybool, &mybool);
  (void) mpl_add_param_to_list(&param_list_p, test_paramid_mybool8, &mybool8);

  /* Blacklist: */
  elem_p = mpl_param_element_create_empty(test_paramid_myuint16);
  mpl_list_add(&blacklist, &elem_p->list_entry);
  elem_p = mpl_param_element_create_empty(test_paramid_myuint8_arr);
  mpl_list_add(&blacklist, &elem_p->list_entry);
  /* Remove from blacklist */
  elem_p = mpl_param_element_create_empty(test_paramid_myuint32);
  mpl_list_add(&blacklist, &elem_p->list_entry);
  mpl_remove_from_blacklist(&blacklist,  test_paramid_myuint32);

  fp = fopen(config_path, "w");
  if (NULL == fp)
  {
    printf("Could not open config file for writing\n");
    return -1;
  }

  if (mpl_config_write_config_bl_fp(NULL, param_list_p, TEST_PARAM_SET_ID, blacklist) >= 0)
  {
    printf("file write suceeded\n");
    return -1;
  }

  if (mpl_config_write_config_bl_fp(fp, param_list_p, TEST_PARAM_SET_ID, blacklist) < 0)
  {
    printf("file write failed\n");
    return -1;
  }

  fclose(fp);

  fp = fopen(config_path, "r");
  if (NULL == fp)
  {
    printf("Could not open config file for reading\n");
    return -1;
  }


  if (mpl_config_read_config_fp(NULL, &unpacked_param_list_p, TEST_PARAM_SET_ID) >= 0)
  {
    printf("file read suceeded\n");
    return -1;
  }
  
  if (mpl_config_read_config_fp(fp, &unpacked_param_list_p, TEST_PARAM_SET_ID) < 0)
  {
    printf("file read failed\n");
    return -1;
  }

  fclose(fp);

  elem_p = mpl_param_list_find(test_paramid_myuint16, param_list_p);
  mpl_list_remove(&param_list_p, &elem_p->list_entry);
  mpl_param_element_destroy(elem_p);
  elem_p = mpl_param_list_find(test_paramid_myuint8_arr, param_list_p);
  mpl_list_remove(&param_list_p, &elem_p->list_entry);
  mpl_param_element_destroy(elem_p);

  res = mpl_compare_param_lists(param_list_p, unpacked_param_list_p);

  if (mpl_config_write_config_bl_func(log_func,
                                      param_list_p,
                                      TEST_PARAM_SET_ID,
                                      blacklist) < 0)
  {
      printf("mpl_config_write_config_bl_func() failed\n");
      return -1;
  }
  
  mpl_param_list_destroy(&param_list_p);
  mpl_param_list_destroy(&unpacked_param_list_p);
  mpl_param_list_destroy(&blacklist);

  return res;
}

static int tc_write_read_blacklist(void)
{
  FILE *fp;
  char *blacklist_path = BLACKLIST_FILE;

  mpl_blacklist_t blacklist = NULL;
  mpl_list_t *unpacked_param_list_p = NULL;
  mpl_param_element_t *elem_p;
  int res;

  /* Blacklist: */
  elem_p = mpl_param_element_create_empty(test_paramid_myuint32);
  mpl_list_add(&blacklist, &elem_p->list_entry);
  elem_p = mpl_param_element_create_empty(test_paramid_myuint16);
  mpl_list_add(&blacklist, &elem_p->list_entry);
  elem_p = mpl_param_element_create_empty(test_paramid_myuint8_arr);
  mpl_list_add(&blacklist, &elem_p->list_entry);

  fp = fopen(blacklist_path, "w");
  if (NULL == fp)
  {
    printf("Could not open blacklist file for writing\n");
    return -1;
  }

  if (mpl_config_write_blacklist_fp(fp, blacklist, TEST_PARAM_SET_ID) < 0)
  {
    printf("file write failed\n");
  }

  fclose(fp);

  fp = fopen(blacklist_path, "r");
  if (NULL == fp)
  {
    printf("Could not open blacklist file for reading\n");
    return -1;
  }


  if (mpl_config_read_config_fp(fp, &unpacked_param_list_p, TEST_PARAM_SET_ID) < 0)
  {
    printf("file read failed\n");
    return -1;
  }

  fclose(fp);

  /* Remove non-configurable entry from blacklist */
  mpl_remove_from_blacklist(&blacklist,  test_paramid_myuint16);

  res = mpl_compare_param_lists(blacklist, unpacked_param_list_p);

  mpl_param_list_destroy(&unpacked_param_list_p);
  mpl_param_list_destroy(&blacklist);

  return res;
}

static int tc_param_tag(void)
{
  int myint = 1000;
  uint8_t myuint8 = 10;
  uint16_t myuint16 = 100;
  uint32_t myuint32_1 = 110;
  uint32_t myuint32_2 = 120;
  uint32_t myuint32_3 = 130;
  uint32_t myuint32_4 = 140;
  uint32_t myuint32_5 = 150;
  char mystring[] = "hallo der";
  bool mybool = true;
  uint8_t mybool8 = 0;

  buf = NULL;
  mpl_list_t *orig_list_p = NULL;
  mpl_list_t *param_list_p = NULL;
  mpl_list_t *tag_list_p = NULL;
  mpl_param_element_t* param_elem_p;


  (void) mpl_add_param_to_list_tag(&orig_list_p, test_paramid_myuint32, 1, &myuint32_1);
  (void) mpl_add_param_to_list(&orig_list_p, test_paramid_myint, &myint);
  (void) mpl_add_param_to_list(&orig_list_p, test_paramid_myuint8, &myuint8);
  (void) mpl_add_param_to_list(&orig_list_p, test_paramid_myuint32, &myuint32_2);
  (void) mpl_add_param_to_list_tag(&orig_list_p, test_paramid_myuint32, 2, &myuint32_3);
  (void) mpl_add_param_to_list(&orig_list_p, test_paramid_myuint16, &myuint16);
  (void) mpl_add_param_to_list(&orig_list_p, test_paramid_mystring, &mystring);
  (void) mpl_add_param_to_list_tag(&orig_list_p, test_paramid_myuint32, 77, &myuint32_4);
  (void) mpl_add_param_to_list(&orig_list_p, test_paramid_mybool, &mybool);
  (void) mpl_add_param_to_list(&orig_list_p, test_paramid_mybool8, &mybool8);
  (void) mpl_add_param_to_list_tag(&orig_list_p, test_paramid_myuint32, 33, &myuint32_5);

  /* Positive */
  buflen = mpl_param_list_pack(orig_list_p,buf,0);
  buf = malloc(buflen + 1);

  if(mpl_param_list_pack(orig_list_p,buf,buflen + 1) != buflen)
  {
    printf("mpl_param_list_pack() failed\n");
    if(buf != NULL)
      free(buf);
    mpl_param_list_destroy(&orig_list_p);
    return -1;
  }

  printf("buf = '%s', buflen = %zu\n", buf, (size_t)buflen);

  param_list_p = mpl_param_list_unpack(buf);

  if (NULL == param_list_p)
  {
    printf("mpl_param_list_unpack() failed\n");
    free(buf);
    mpl_param_list_destroy(&orig_list_p);
    return -1;
  }

  if (mpl_compare_param_lists(packmsg_p,upackmsg_p) < 0)
  {
    printf("unpacked list not equal to original\n");
    free(buf);
    mpl_param_list_destroy(&orig_list_p);
    mpl_param_list_destroy(&param_list_p);
    return -1;
  }

  free(buf);

  tag_list_p = mpl_param_list_find_all_tag(test_paramid_myuint32,
                                           77,
                                           orig_list_p);

  if (tag_list_p == NULL)
  {
    printf("mpl_param_list_find_all_tag() (1)\n");
    mpl_param_list_destroy(&orig_list_p);
    mpl_param_list_destroy(&param_list_p);
    return -1;
  }

  if (mpl_list_len(tag_list_p) != 1)
  {
    printf("mpl_param_list_find_all_tag() (2)\n");
    mpl_param_list_destroy(&orig_list_p);
    mpl_param_list_destroy(&param_list_p);
    mpl_param_list_destroy(&tag_list_p);
    return -1;
  }

  if (mpl_param_list_param_count_tag(test_paramid_myuint32, 77, orig_list_p) != 1) {
    printf("mpl_param_list_find_all_tag() (3)\n");
    mpl_param_list_destroy(&orig_list_p);
    mpl_param_list_destroy(&param_list_p);
    mpl_param_list_destroy(&tag_list_p);
    return -1;
  }

  if (mpl_param_list_param_count(test_paramid_myuint32, orig_list_p) != 5) {
    printf("mpl_param_list_find_all_tag() (4)\n");
    mpl_param_list_destroy(&orig_list_p);
    mpl_param_list_destroy(&param_list_p);
    mpl_param_list_destroy(&tag_list_p);
    return -1;
  }

  mpl_param_list_destroy(&tag_list_p);
  mpl_param_list_destroy(&orig_list_p);

  param_elem_p = mpl_param_list_find_tag(MPL_PARAM_ID_UNDEFINED, 77, param_list_p);
  if ((param_elem_p->id != test_paramid_myuint32) || (*(int*)param_elem_p->value_p != myuint32_4))
  {
    printf("Unexpected list element\n");
    mpl_param_list_destroy(&param_list_p);
    return -1;
  }
  printf("Tag 77 match: %d\n", *(int*)param_elem_p->value_p);

  param_elem_p = mpl_param_list_find_tag(test_paramid_myuint32, 2, param_list_p);
  if (*(int*)param_elem_p->value_p != myuint32_3)
  {
    printf("Unexpected list element\n");
    mpl_param_list_destroy(&param_list_p);
    return -1;
  }
  printf("Tag 2 match: %d\n", *(int*)param_elem_p->value_p);

  param_elem_p = mpl_param_list_find_tag(test_paramid_myuint32, 0, param_list_p);
  if (*(int*)param_elem_p->value_p != myuint32_2)
  {
    printf("Unexpected list element\n");
    mpl_param_list_destroy(&param_list_p);
    return -1;
  }
  printf("Tag 0 match: %d\n", *(int*)param_elem_p->value_p);

  param_elem_p = mpl_param_list_find_tag(test_paramid_myuint32, 1, param_list_p);
  if (*(int*)param_elem_p->value_p != myuint32_1)
  {
    printf("Unexpected list element\n");
    mpl_param_list_destroy(&param_list_p);
    return -1;
  }
  printf("Tag 1 match: %d\n", *(int*)param_elem_p->value_p);

  param_elem_p = mpl_param_list_find_tag(test_paramid_myuint32, 33, param_list_p);
  if (*(int*)param_elem_p->value_p != myuint32_5)
  {
    printf("Unexpected list element\n");
    mpl_param_list_destroy(&param_list_p);
    return -1;
  }
  printf("Tag 33 match: %d\n", *(int*)param_elem_p->value_p);

  param_elem_p = mpl_param_list_find_tag(test_paramid_myuint32, 77, param_list_p);
  if (*(int*)param_elem_p->value_p != myuint32_4)
  {
    printf("Unexpected list element\n");
    mpl_param_list_destroy(&param_list_p);
    return -1;
  }
  printf("Tag 77 match: %d\n", *(int*)param_elem_p->value_p);

  if (mpl_param_element_get_tag(param_elem_p) != 77)
  {
    printf("Unexpected tag value 100\n");
    mpl_param_list_destroy(&param_list_p);
    return -1;
  }

  mpl_param_element_set_tag(param_elem_p, 55);
  if (mpl_param_element_get_tag(param_elem_p) != 55)
  {
    printf("Unexpected tag value 100\n");
    mpl_param_list_destroy(&param_list_p);
    return -1;
  }

  if (mpl_add_param_to_list_tag(&param_list_p, test_paramid_myuint32, 100, &myuint32_5) == 0)
  {
    printf("Unexpected success tag value 100\n");
    mpl_param_list_destroy(&param_list_p);
    return -1;
  }

  if (mpl_add_param_to_list_tag(&param_list_p, test_paramid_myuint32, -1, &myuint32_5) == 0)
  {
    printf("Unexpected success tag value 100\n");
    mpl_param_list_destroy(&param_list_p);
    return -1;
  }

  mpl_param_list_destroy(&param_list_p);

  buf = malloc(strlen("test.myint[99]=5") + 1);
  strcpy(buf, "test.myint[99]=5");
  param_list_p = mpl_param_list_unpack(buf);
  free(buf);

  if (NULL == param_list_p)
  {
    printf("mpl_param_list_unpack() failed\n");
    return -1;
  }

  mpl_param_list_destroy(&param_list_p);

  buf = malloc(strlen("test.myint[100]=5") + 1);
  strcpy(buf, "test.myint[100]=5");
  param_list_p = mpl_param_list_unpack(buf);
  free(buf);

  if (NULL != param_list_p)
  {
    printf("mpl_param_list_unpack() unexpectedly succeeded\n");
    mpl_param_list_destroy(&param_list_p);
    return -1;
  }

  buf = malloc(strlen("test.myint[-1]=5") + 1);
  strcpy(buf, "test.myint[-1]=5");
  param_list_p = mpl_param_list_unpack(buf);
  free(buf);

  if (NULL != param_list_p)
  {
    printf("mpl_param_list_unpack() unexpectedly succeeded\n");
    mpl_param_list_destroy(&param_list_p);
    return -1;
  }

  buf = malloc(strlen("test.myint[ab]=5") + 1);
  strcpy(buf, "test.myint[ab]=5");
  param_list_p = mpl_param_list_unpack(buf);
  free(buf);

  if (NULL != param_list_p)
  {
    printf("mpl_param_list_unpack() unexpectedly succeeded\n");
    mpl_param_list_destroy(&param_list_p);
    return -1;
  }

  packparam.id = test_paramid_myint;
  packparam.value_p = &myint;
  packparam.tag = 100;

  if (mpl_param_pack(&packparam, NULL, 0) != -1)
  {
      printf("mpl_param_pack() unexpectedly succeeded\n");
      return -1;
  }

  return 0;
}


static int tc_int_tuple(void)
{
  mpl_int_tuple_t it, it_copy;
  int size;
  char *value_p;
  char *key_p;
  mpl_param_element_t* cloneparam_p;
  mpl_param_element_t* newparam_p;
  int copy_size;

  it.key = 100;
  it.value = 200;

  /* Find out space need */
  packparam.id = test_paramid_myint_tup;
  packparam.value_p = &it;
  packparam.tag = 0;

  buflen=mpl_param_pack(&packparam, NULL, 0);
  if (buflen<0)
  {
    printf("mpl_param_pack() failed\n");
    return (-1);
  }

  /* Alloc size including '\0' */
  buf = malloc(buflen+1);

  size=mpl_param_pack(&packparam, buf, buflen+1);
  if ((size<0) || (size != buflen))
  {
    printf("mpl_param_pack() failed\n");
    free(buf);
    return (-1);
  }

  printf("buf = '%s', buflen = %zu\n", buf, (size_t)buflen);
  key_p = buf;
  value_p = strstr(buf, "=");
  *value_p = 0;
  value_p++;

  if (mpl_param_unpack(key_p, "2000", &unpackparam_p) == 0)
  {
    printf("mpl_param_unpack() succeeded 1\n");
    mpl_param_element_destroy(unpackparam_p);
    free(buf);
    return (-1);
  }

  if (mpl_param_unpack(key_p, "999:2000", &unpackparam_p) == 0)
  {
    printf("mpl_param_unpack() succeeded 2\n");
    mpl_param_element_destroy(unpackparam_p);
    free(buf);
    return (-1);
  }

  if (mpl_param_unpack(key_p, "2000:999", &unpackparam_p) == 0)
  {
    printf("mpl_param_unpack() succeeded 3\n");
    mpl_param_element_destroy(unpackparam_p);
    free(buf);
    return (-1);
  }

  if (mpl_param_unpack(key_p, "999:999", &unpackparam_p) < 0)
  {
    printf("mpl_param_unpack() failed 1\n");
    free(buf);
    return (-1);
  }
  mpl_param_element_destroy(unpackparam_p);

  if (mpl_param_unpack(key_p, value_p, &unpackparam_p) < 0)
  {
    printf("mpl_param_unpack() failed 2\n");
    free(buf);
    return (-1);
  }

  if ((newparam_p = mpl_param_element_create_int_tuple(test_paramid_myint_tup, 100, 200)) == NULL)
  {
    printf("mpl_param_element_create_int_tuple failed\n");
    free(buf);
    return -1;
  }

  if (0 != mpl_param_element_compare(&packparam, newparam_p))
  {
    printf("param value mismatch (0)\n");
    free(buf);
    mpl_param_element_destroy(newparam_p);
    return -1;
  }

  if (mpl_param_list_int_tuple_find(test_paramid_myint_tup,
                                    100,
                                    200,
                                    &newparam_p->list_entry) != newparam_p)
  {
    printf("mpl_param_list_int_tuple_find()\n");
    free(buf);
    mpl_param_element_destroy(newparam_p);
    return -1;
  }

  mpl_param_element_destroy(newparam_p);



  if (0 != mpl_param_element_compare(&packparam, unpackparam_p))
  {
    printf("param value mismatch (1)\n");
    free(buf);
    return -1;
  }

  it.key++;

  if (0 == mpl_param_element_compare(&packparam, unpackparam_p))
  {
    printf("unexpected param %d value match\n", packparam.id);
    free(buf);
    return -1;
  }
  printf("key\n");

  it.key--;
  it.value++;

  if (0 == mpl_param_element_compare(&packparam, unpackparam_p))
  {
    printf("unexpected param %d value match\n", packparam.id);
    free(buf);
    return -1;
  }
  printf("val\n");

  it.value--;

  mpl_param_element_destroy(unpackparam_p);

  cloneparam_p = mpl_param_element_clone(&packparam);

  if (0 != mpl_param_element_compare(&packparam, cloneparam_p))
  {
    printf("param value mismatch (2)\n");
    free(buf);
    mpl_param_element_destroy(cloneparam_p);
    return -1;
  }
  printf("clone\n");

  mpl_param_element_destroy(cloneparam_p);

  size = sizeof(it_copy.value);

  if ((copy_size = mpl_param_value_copy_out(&packparam, &it_copy.value, 100)) < 0)
  {
    printf("mpl_param_value_copy_out() failed\n");
    free(buf);
    return -1;
  }
  if (copy_size != size)
  {
    printf("mpl_param_value_copy_out() failed to give correct size: %d\n", copy_size);
    free(buf);
    return -1;
  }

  if (it_copy.value != it.value)
  {
    printf("mpl_param_value_copy_out() failed to copy correctly\n");
    free(buf);
    return -1;
  }

  if ((copy_size = mpl_param_value_copy_out(&packparam, &it_copy.value, 0)) != size)
  {
    printf("mpl_param_value_copy_out() failed\n");
    free(buf);
    return -1;
  }
  printf("copy out\n");


  /* Negative: Max value check */
  if (mpl_param_unpack("test.myint_tup", "1000:1001", &unpackparam_p) >= 0)
  {
    printf("mpl_param_unpack() unexpectedly succeeded for max value check\n");
    mpl_param_element_destroy(unpackparam_p);
    return (-1);
  }

  /* Negative: Max value check */
  if (mpl_param_unpack("test.myint_tup", "1001:1000", &unpackparam_p) >= 0)
  {
    printf("mpl_param_unpack() unexpectedly succeeded for max value check\n");
    mpl_param_element_destroy(unpackparam_p);
    return (-1);
  }

  /* Negative: Min value check */
  if (mpl_param_unpack("test.myint_tup", "-1000:-1001", &unpackparam_p) >= 0)
  {
    printf("mpl_param_unpack() unexpectedly succeeded for min value check\n");
    mpl_param_element_destroy(unpackparam_p);
    return (-1);
  }

  /* Negative: Min value check */
  if (mpl_param_unpack("test.myint_tup", "-1001:-1000", &unpackparam_p) >= 0)
  {
    printf("mpl_param_unpack() unexpectedly succeeded for min value check\n");
    mpl_param_element_destroy(unpackparam_p);
    return (-1);
  }


  /* Sanity check */
  if (mpl_param_unpack("test.myint_tup", "500:-500", &unpackparam_p) != 0)
  {
    printf("mpl_param_unpack() unexpectedly failed\n");
    return (-1);
  }
  mpl_param_element_destroy(unpackparam_p);

  free(buf);
  return 0;
}


static int tc_strint_tuple(void)
{
  mpl_strint_tuple_t t, t_copy;
  int size;
  char *value_p;
  char *key_p;
  mpl_param_element_t* cloneparam_p;
  mpl_param_element_t* newparam_p;
  int copy_size;

  t.key_p = malloc(100 * sizeof(char));
  strcpy(t.key_p, "mykey");
  t.value = 200;

  /* Find out space need */
  packparam.id = test_paramid_mystrint_tup;
  packparam.value_p = &t;

  buflen=mpl_param_pack(&packparam, NULL, 0);
  if (buflen<0)
  {
    printf("mpl_param_pack() failed\n");
    free(t.key_p);
    return (-1);
  }

  /* Alloc size including '\0' */
  buf = malloc(buflen+1);

  size=mpl_param_pack(&packparam, buf, buflen+1);
  if ((size<0) || (size != buflen))
  {
    printf("mpl_param_pack() failed\n");
    free(buf);
    free(t.key_p);
    return (-1);
  }

  printf("buf = '%s', buflen = %zu\n", buf, (size_t)buflen);
  key_p = buf;
  value_p = strstr(buf, "=");
  *value_p = 0;
  value_p++;

  if (mpl_param_unpack(key_p, "thekey", &unpackparam_p) == 0)
  {
    printf("mpl_param_unpack() succeeded 1\n");
    mpl_param_element_destroy(unpackparam_p);
    free(buf);
    free(t.key_p);
    return (-1);
  }

  if (mpl_param_unpack(key_p, "thekey:2000", &unpackparam_p) == 0)
  {
    printf("mpl_param_unpack() succeeded 2\n");
    mpl_param_element_destroy(unpackparam_p);
    free(buf);
    free(t.key_p);
    return (-1);
  }

  if (mpl_param_unpack(key_p, "thekey:999", &unpackparam_p) < 0)
  {
    printf("mpl_param_unpack() failed 1\n");
    free(buf);
    free(t.key_p);
    return (-1);
  }
  mpl_param_element_destroy(unpackparam_p);

  printf("key_p = '%s', value_p = %s\n", key_p, value_p);
  if (mpl_param_unpack(key_p, value_p, &unpackparam_p) < 0)
  {
    printf("mpl_param_unpack() failed 2\n");
    free(buf);
    free(t.key_p);
    return (-1);
  }

  if ((newparam_p = mpl_param_element_create_strint_tuple(test_paramid_mystrint_tup, "mykey", 200)) == NULL)
  {
    printf("mpl_param_element_create_strint_tuple failed\n");
    free(buf);
    free(t.key_p);
    return -1;
  }

  if (0 != mpl_param_element_compare(&packparam, newparam_p))
  {
    printf("param value mismatch (0)\n");
    free(buf);
    free(t.key_p);
    mpl_param_element_destroy(newparam_p);
    return -1;
  }

  if (mpl_param_list_strint_tuple_find(test_paramid_mystrint_tup,
                                       "mykey",
                                       200,
                                       &newparam_p->list_entry) != newparam_p)
  {
    printf("mpl_param_list_strint_tuple_find()\n");
    free(buf);
    free(t.key_p);
    mpl_param_element_destroy(newparam_p);
    return -1;
  }

  if (mpl_param_list_strint_tuple_find(test_paramid_mystruint8_tup,
                                       "mykey",
                                       200,
                                       &newparam_p->list_entry) != NULL)
  {
    printf("mpl_param_list_strint_tuple_find()\n");
    free(buf);
    free(t.key_p);
    mpl_param_element_destroy(newparam_p);
    return -1;
  }

  mpl_param_element_destroy(newparam_p);


  if (0 != mpl_param_element_compare(&packparam, unpackparam_p))
  {
    printf("param value mismatch (1)\n");
    free(buf);
    free(t.key_p);
    return -1;
  }

  t.key_p[2] = 'x';

  if (0 == mpl_param_element_compare(&packparam, unpackparam_p))
  {
    printf("unexpected param %d value match\n", packparam.id);
    free(buf);
    free(t.key_p);
    return -1;
  }
  printf("key\n");

  t.key_p[2] = 'k';
  t.value++;

  if (0 == mpl_param_element_compare(&packparam, unpackparam_p))
  {
    printf("unexpected param %d value match\n", packparam.id);
    free(buf);
    free(t.key_p);
    return -1;
  }
  printf("val\n");

  t.value--;

  mpl_param_element_destroy(unpackparam_p);

  cloneparam_p = mpl_param_element_clone(&packparam);

  if (0 != mpl_param_element_compare(&packparam, cloneparam_p))
  {
    printf("param value mismatch (2)\n");
    free(buf);
    free(t.key_p);
    mpl_param_element_destroy(cloneparam_p);
    return -1;
  }
  printf("clone\n");

  mpl_param_element_destroy(cloneparam_p);

  size = sizeof(t_copy.value);

  if ((copy_size = mpl_param_value_copy_out(&packparam, &t_copy.value, 100)) < 0)
  {
    printf("mpl_param_value_copy_out() failed\n");
    free(buf);
    free(t.key_p);
    return -1;
  }
  if (copy_size != size)
  {
    printf("mpl_param_value_copy_out() failed to give correct size: %d\n", copy_size);
    free(buf);
    free(t.key_p);
    return -1;
  }

  if (t_copy.value != t.value)
  {
    printf("mpl_param_value_copy_out() failed to copy correctly\n");
    free(buf);
    free(t.key_p);
    return -1;
  }

  if ((copy_size = mpl_param_value_copy_out(&packparam, &t_copy.value, 0)) != size)
  {
    printf("mpl_param_value_copy_out() failed\n");
    free(buf);
    free(t.key_p);
    return -1;
  }
  printf("copy out\n");

  /* Negative: Max value check */
  if (mpl_param_unpack("test.mystrint_tup", "123456789012345678901:1001", &unpackparam_p) >= 0)
  {
    printf("mpl_param_unpack() unexpectedly succeeded for max value check\n");
    mpl_param_element_destroy(unpackparam_p);
    return (-1);
  }

  /* Negative: Min value check */
  if (mpl_param_unpack("test.mystrint_tup", "123456:-501", &unpackparam_p) >= 0)
  {
    printf("mpl_param_unpack() unexpectedly succeeded for min value check\n");
    mpl_param_element_destroy(unpackparam_p);
    return (-1);
  }


  /* Sanity check */
  if (mpl_param_unpack("test.mystrint_tup", "12345:100", &unpackparam_p) != 0)
  {
    printf("mpl_param_unpack() unexpectedly failed\n");
    return (-1);
  }
  mpl_param_element_destroy(unpackparam_p);


  free(buf);
  free(t.key_p);
  return 0;
}


static int tc_struint8_tuple(void)
{
  mpl_struint8_tuple_t t, t_copy;
  int size;
  char *value_p;
  char *key_p;
  mpl_param_element_t* cloneparam_p;
  mpl_param_element_t* newparam_p;
  int copy_size;

  t.key_p = malloc(100 * sizeof(char));
  strcpy(t.key_p, "mykey");
  t.value = 100;

  /* Find out space need */
  packparam.id = test_paramid_mystruint8_tup;
  packparam.value_p = &t;

  buflen=mpl_param_pack(&packparam, NULL, 0);
  if (buflen<0)
  {
    printf("mpl_param_pack() failed\n");
    free(t.key_p);
    return (-1);
  }

  /* Alloc size including '\0' */
  buf = malloc(buflen+1);

  size=mpl_param_pack(&packparam, buf, buflen+1);
  if ((size<0) || (size != buflen))
  {
    printf("mpl_param_pack() failed\n");
    free(buf);
    free(t.key_p);
    return (-1);
  }

  printf("buf = '%s', buflen = %zu\n", buf, (size_t)buflen);
  key_p = buf;
  value_p = strstr(buf, "=");
  *value_p = 0;
  value_p++;

  if (mpl_param_unpack(key_p, "thekey", &unpackparam_p) == 0)
  {
    printf("mpl_param_unpack() succeeded 1\n");
    mpl_param_element_destroy(unpackparam_p);
    free(buf);
    free(t.key_p);
    return (-1);
  }

  if (mpl_param_unpack(key_p, "thekey/210", &unpackparam_p) == 0)
  {
    printf("mpl_param_unpack() succeeded 2\n");
    mpl_param_element_destroy(unpackparam_p);
    free(buf);
    free(t.key_p);
    return (-1);
  }

  if (mpl_param_unpack(key_p, "thekey/199", &unpackparam_p) < 0)
  {
    printf("mpl_param_unpack() failed 1\n");
    free(buf);
    free(t.key_p);
    return (-1);
  }
  mpl_param_element_destroy(unpackparam_p);

  printf("key_p = '%s', value_p = %s\n", key_p, value_p);
  if (mpl_param_unpack(key_p, value_p, &unpackparam_p) < 0)
  {
    printf("mpl_param_unpack() failed 2\n");
    free(buf);
    free(t.key_p);
    return (-1);
  }

  if ((newparam_p = mpl_param_element_create_struint8_tuple(test_paramid_mystruint8_tup, "mykey", 100)) == NULL)
  {
    printf("mpl_param_element_create_struint8_tuple failed\n");
    free(buf);
    free(t.key_p);
    return -1;
  }

  if (0 != mpl_param_element_compare(&packparam, newparam_p))
  {
    printf("param value mismatch (0)\n");
    free(buf);
    free(t.key_p);
    mpl_param_element_destroy(newparam_p);
    return -1;
  }

  if (mpl_param_list_struint8_tuple_find(test_paramid_mystruint8_tup,
                                         "mykey",
                                         100,
                                         &newparam_p->list_entry) != newparam_p)
  {
    printf("mpl_param_list_struint8_tuple_find()\n");
    free(buf);
    free(t.key_p);
    mpl_param_element_destroy(newparam_p);
    return -1;
  }

  if (mpl_param_list_struint8_tuple_find(test_paramid_mystrint_tup,
                                         "mykey",
                                         100,
                                         &newparam_p->list_entry) != NULL)
  {
    printf("mpl_param_list_struint8_tuple_find()\n");
    free(buf);
    free(t.key_p);
    mpl_param_element_destroy(newparam_p);
    return -1;
  }

  mpl_param_element_destroy(newparam_p);


  if (0 != mpl_param_element_compare(&packparam, unpackparam_p))
  {
    printf("param value mismatch (1)\n");
    free(buf);
    free(t.key_p);
    return -1;
  }

  t.key_p[2] = 'x';

  if (0 == mpl_param_element_compare(&packparam, unpackparam_p))
  {
    printf("unexpected param %d value match\n", packparam.id);
    free(buf);
    free(t.key_p);
    return -1;
  }
  printf("key\n");

  t.key_p[2] = 'k';
  t.value++;

  if (0 == mpl_param_element_compare(&packparam, unpackparam_p))
  {
    printf("unexpected param %d value match\n", packparam.id);
    free(buf);
    free(t.key_p);
    return -1;
  }
  printf("val\n");

  t.value--;

  mpl_param_element_destroy(unpackparam_p);

  cloneparam_p = mpl_param_element_clone(&packparam);

  if (0 != mpl_param_element_compare(&packparam, cloneparam_p))
  {
    printf("param value mismatch (2)\n");
    free(buf);
    free(t.key_p);
    mpl_param_element_destroy(cloneparam_p);
    return -1;
  }
  printf("clone\n");

  mpl_param_element_destroy(cloneparam_p);

  size = sizeof(t_copy.value);

  if ((copy_size = mpl_param_value_copy_out(&packparam, &t_copy.value, 100)) < 0)
  {
    printf("mpl_param_value_copy_out() failed\n");
    free(buf);
    free(t.key_p);
    return -1;
  }
  if (copy_size != size)
  {
    printf("mpl_param_value_copy_out() failed to give correct size: %d\n", copy_size);
    free(buf);
    free(t.key_p);
    return -1;
  }

  if (t_copy.value != t.value)
  {
    printf("mpl_param_value_copy_out() failed to copy correctly\n");
    free(buf);
    free(t.key_p);
    return -1;
  }

  if ((copy_size = mpl_param_value_copy_out(&packparam, &t_copy.value, 0)) != size)
  {
    printf("mpl_param_value_copy_out() failed\n");
    free(buf);
    free(t.key_p);
    return -1;
  }
  printf("copy out\n");

  /* Negative: Max value check */
  if (mpl_param_unpack("test.mystruint8_tup", "123456789012345678901/201", &unpackparam_p) >= 0)
  {
    printf("mpl_param_unpack() unexpectedly succeeded for max value check\n");
    mpl_param_element_destroy(unpackparam_p);
    return (-1);
  }

  /* Negative: Min value check */
  if (mpl_param_unpack("test.mystruint8_tup", "123456/4", &unpackparam_p) >= 0)
  {
    printf("mpl_param_unpack() unexpectedly succeeded for min value check\n");
    mpl_param_element_destroy(unpackparam_p);
    return (-1);
  }


  /* Sanity check */
  if (mpl_param_unpack("test.mystruint8_tup", "12345/100", &unpackparam_p) != 0)
  {
    printf("mpl_param_unpack() unexpectedly failed\n");
    return (-1);
  }
  mpl_param_element_destroy(unpackparam_p);


  free(buf);
  free(t.key_p);
  return 0;
}


static int tc_pack_unpack_param_sint8(void)
{
  sint8_t mysint8 = -5;
  sint8_t mysint8_copy = 0;
  int copy_size;
  int size;
  char* value_p;
  int res;
  sint8_t mysint8_2;
  mpl_param_element_t *par;

  /* Negative */
  packparam.id = mpl_param_last_paramid(TEST_PARAM_SET_ID) + 1;
  packparam.value_p = &mysint8;

  size=mpl_param_pack(&packparam, NULL, 0);
  if (size>0)
  {
    printf("mpl_param_pack() unexpectedly (1) succeeded\n");
    return (-1);
  }

  /* Negative 2 */
  packparam.id = mpl_param_first_paramid(TEST_PARAM_SET_ID) - 1;
  packparam.value_p = &mysint8;

  size=mpl_param_pack(&packparam, NULL, 0);
  if (size>0)
  {
    printf("mpl_param_pack() unexpectedly (2) succeeded\n");
    return (-1);
  }

  /* Negative 3 */
  packparam.id = test_paramid_mysint8 + 0x00030000;
  packparam.value_p = &mysint8;

  size=mpl_param_pack(&packparam, NULL, 0);
  if (size>0)
  {
    printf("mpl_param_pack() unexpectedly (3) succeeded\n");
    return (-1);
  }

  /* Find out space need */
  packparam.id = test_paramid_mysint8;
  packparam.value_p = &mysint8;

  size=mpl_param_pack(&packparam, NULL, 0);
  if (size<0)
  {
    printf("mpl_param_pack() failed\n");
    return (-1);
  }

  /* Alloc size including '\0' */
  buf = malloc(size+1);

  size=mpl_param_pack(&packparam, buf, size+1);
  if (size<0)
  {
    printf("mpl_param_pack() failed\n");
    free(buf);
    return (-1);
  }

  printf("%s\n", buf);

  value_p = strstr(buf, "=");
  if (value_p == NULL)
  {
    printf("No '=' found in packet param\n");
    free(buf);
    return (-1);
  }
  *value_p = '\0';
  value_p++;

  if (mpl_param_unpack(buf, value_p, &unpackparam_p) < 0)
  {
    printf("mpl_param_unpack() failed\n");
    free(buf);
    return (-1);
  }

  free(buf);

  res = ((unpackparam_p->id == test_paramid_mysint8) &&
	 (*(sint8_t*)unpackparam_p->value_p == mysint8));

  if (res != 1)
    return -1;

  if ((copy_size = mpl_param_value_copy_out(unpackparam_p, &mysint8_copy, 100)) < 0)
  {
    printf("mpl_param_value_copy_out() failed\n");
    mpl_param_element_destroy(unpackparam_p);
    return -1;
  }

  if (copy_size != sizeof(mysint8_copy))
  {
    printf("mpl_param_value_copy_out() failed to give correct size\n");
    mpl_param_element_destroy(unpackparam_p);
    return -1;
  }

  printf("mysint8_copy = %d, copy_size = %d\n", mysint8_copy, copy_size );

  if (mysint8_copy != *(sint8_t*)unpackparam_p->value_p)
  {
    printf("mpl_param_value_copy_out() failed to copy correctly\n");
    mpl_param_element_destroy(unpackparam_p);
    return -1;
  }

  if ((copy_size = mpl_param_value_copy_out(unpackparam_p, &mysint8_copy, 0)) != sizeof(mysint8_copy))
  {
    printf("mpl_param_value_copy_out() failed to give correct size\n");
    mpl_param_element_destroy(unpackparam_p);
    return -1;
  }

  mpl_param_element_destroy(unpackparam_p);


  /* Negative */
  if (mpl_param_unpack(buf, value_p, NULL) >= 0)
  {
    printf("mpl_param_unpack() unexpectedly (4) succeeded\n");
    free(buf);
    return (-1);
  }

  /* Out of range */
  if (mpl_param_unpack("test.mysint8", "130", &unpackparam_p) >= 0)
  {
    printf("mpl_param_unpack() unexpectedly succeeded for mysint8=%s\n", "130");
    return (-1);
  }

  /* > max */
  if (mpl_param_unpack("test.mysint8", "70", &unpackparam_p) >= 0)
  {
    printf("mpl_param_unpack() unexpectedly succeeded for mysint8=%s\n", "70");
    return (-1);
  }

  /* MIN */
  mysint8_2 = INT8_MIN;
  par = mpl_param_element_create(test_paramid_mysint8_2, &mysint8_2);
  if (NULL == par)
  {
      printf("mpl_param_element_create(mysint8_2) failed\n");
      return -1;
  }
  if (pack_unpack_one(par, true, TEST_PARAM_SET_ID) < 0)
      return -1;

  mpl_param_element_destroy(par);

  /* MAX */
  mysint8_2 = INT8_MAX;
  par = mpl_param_element_create(test_paramid_mysint8_2, &mysint8_2);
  if (NULL == par)
  {
      printf("mpl_param_element_create(mysint8_2) failed\n");
      return -1;
  }
  if (pack_unpack_one(par, true, TEST_PARAM_SET_ID) < 0)
      return -1;

  mpl_param_element_destroy(par);

  return 0;
}

static int tc_pack_unpack_param_sint16(void)
{
  sint16_t mysint16 = -5;
  sint16_t mysint16_copy = 0;
  int copy_size;
  int size;
  char* value_p;
  int res;
  sint16_t mysint16_2;
  mpl_param_element_t *par;

  /* Negative */
  packparam.id = mpl_param_last_paramid(TEST_PARAM_SET_ID) + 1;
  packparam.value_p = &mysint16;

  size=mpl_param_pack(&packparam, NULL, 0);
  if (size>0)
  {
    printf("mpl_param_pack() unexpectedly (1) succeeded\n");
    return (-1);
  }

  /* Negative 2 */
  packparam.id = mpl_param_first_paramid(TEST_PARAM_SET_ID) - 1;
  packparam.value_p = &mysint16;

  size=mpl_param_pack(&packparam, NULL, 0);
  if (size>0)
  {
    printf("mpl_param_pack() unexpectedly (2) succeeded\n");
    return (-1);
  }

  /* Negative 3 */
  packparam.id = test_paramid_mysint16 + 0x00030000;
  packparam.value_p = &mysint16;

  size=mpl_param_pack(&packparam, NULL, 0);
  if (size>0)
  {
    printf("mpl_param_pack() unexpectedly (3) succeeded\n");
    return (-1);
  }

  /* Find out space need */
  packparam.id = test_paramid_mysint16;
  packparam.value_p = &mysint16;

  size=mpl_param_pack(&packparam, NULL, 0);
  if (size<0)
  {
    printf("mpl_param_pack() failed\n");
    return (-1);
  }

  /* Alloc size including '\0' */
  buf = malloc(size+1);

  size=mpl_param_pack(&packparam, buf, size+1);
  if (size<0)
  {
    printf("mpl_param_pack() failed\n");
    free(buf);
    return (-1);
  }

  printf("%s\n", buf);

  value_p = strstr(buf, "=");
  if (value_p == NULL)
  {
    printf("No '=' found in packet param\n");
    free(buf);
    return (-1);
  }
  *value_p = '\0';
  value_p++;

  if (mpl_param_unpack(buf, value_p, &unpackparam_p) < 0)
  {
    printf("mpl_param_unpack() failed\n");
    free(buf);
    return (-1);
  }

  free(buf);

  res = ((unpackparam_p->id == test_paramid_mysint16) &&
	 (*(sint16_t*)unpackparam_p->value_p == mysint16));

  if (res != 1)
    return -1;

  if ((copy_size = mpl_param_value_copy_out(unpackparam_p, &mysint16_copy, 100)) < 0)
  {
    printf("mpl_param_value_copy_out() failed\n");
    mpl_param_element_destroy(unpackparam_p);
    return -1;
  }

  if (copy_size != sizeof(mysint16_copy))
  {
    printf("mpl_param_value_copy_out() failed to give correct size\n");
    mpl_param_element_destroy(unpackparam_p);
    return -1;
  }

  printf("mysint16_copy = %d, copy_size = %d\n", mysint16_copy, copy_size );

  if (mysint16_copy != *(sint16_t*)unpackparam_p->value_p)
  {
    printf("mpl_param_value_copy_out() failed to copy correctly\n");
    mpl_param_element_destroy(unpackparam_p);
    return -1;
  }

  if ((copy_size = mpl_param_value_copy_out(unpackparam_p, &mysint16_copy, 0)) != sizeof(mysint16_copy))
  {
    printf("mpl_param_value_copy_out() failed to give correct size\n");
    mpl_param_element_destroy(unpackparam_p);
    return -1;
  }

  mpl_param_element_destroy(unpackparam_p);


  /* Negative */
  if (mpl_param_unpack(buf, value_p, NULL) >= 0)
  {
    printf("mpl_param_unpack() unexpectedly (4) succeeded\n");
    free(buf);
    return (-1);
  }

  /* Out of range */
  if (mpl_param_unpack("test.mysint16", "33000", &unpackparam_p) >= 0)
  {
    printf("mpl_param_unpack() unexpectedly succeeded for mysint16=%s\n", "33000");
    return (-1);
  }

  /* > max */
  if (mpl_param_unpack("test.mysint16", "30100", &unpackparam_p) >= 0)
  {
    printf("mpl_param_unpack() unexpectedly succeeded for mysint16=%s\n", "30100");
    return (-1);
  }

  /* MIN */
  mysint16_2 = INT16_MIN;
  par = mpl_param_element_create(test_paramid_mysint16_2, &mysint16_2);
  if (NULL == par)
  {
      printf("mpl_param_element_create(mysint16_2) failed\n");
      return -1;
  }
  if (pack_unpack_one(par, true, TEST_PARAM_SET_ID) < 0)
      return -1;

  mpl_param_element_destroy(par);

  /* MAX */
  mysint16_2 = INT16_MAX;
  par = mpl_param_element_create(test_paramid_mysint16_2, &mysint16_2);
  if (NULL == par)
  {
      printf("mpl_param_element_create(mysint16_2) failed\n");
      return -1;
  }
  if (pack_unpack_one(par, true, TEST_PARAM_SET_ID) < 0)
      return -1;

  mpl_param_element_destroy(par);

  return 0;
}

static int tc_pack_unpack_param_sint32(void)
{
  sint32_t mysint32 = -5;
  sint32_t mysint32_copy = 0;
  int copy_size;
  int size;
  char* value_p;
  int res;
  sint32_t mysint32_2;
  mpl_param_element_t *par;

  /* Negative */
  packparam.id = mpl_param_last_paramid(TEST_PARAM_SET_ID) + 1;
  packparam.value_p = &mysint32;

  size=mpl_param_pack(&packparam, NULL, 0);
  if (size>0)
  {
    printf("mpl_param_pack() unexpectedly (1) succeeded\n");
    return (-1);
  }

  /* Negative 2 */
  packparam.id = mpl_param_first_paramid(TEST_PARAM_SET_ID) - 1;
  packparam.value_p = &mysint32;

  size=mpl_param_pack(&packparam, NULL, 0);
  if (size>0)
  {
    printf("mpl_param_pack() unexpectedly (2) succeeded\n");
    return (-1);
  }

  /* Negative 3 */
  packparam.id = test_paramid_mysint32 + 0x00030000;
  packparam.value_p = &mysint32;

  size=mpl_param_pack(&packparam, NULL, 0);
  if (size>0)
  {
    printf("mpl_param_pack() unexpectedly (3) succeeded\n");
    return (-1);
  }

  /* Find out space need */
  packparam.id = test_paramid_mysint32;
  packparam.value_p = &mysint32;

  size=mpl_param_pack(&packparam, NULL, 0);
  if (size<0)
  {
    printf("mpl_param_pack() failed\n");
    return (-1);
  }

  /* Alloc size including '\0' */
  buf = malloc(size+1);

  size=mpl_param_pack(&packparam, buf, size+1);
  if (size<0)
  {
    printf("mpl_param_pack() failed\n");
    free(buf);
    return (-1);
  }

  printf("%s\n", buf);

  value_p = strstr(buf, "=");
  if (value_p == NULL)
  {
    printf("No '=' found in packet param\n");
    free(buf);
    return (-1);
  }
  *value_p = '\0';
  value_p++;

  if (mpl_param_unpack(buf, value_p, &unpackparam_p) < 0)
  {
    printf("mpl_param_unpack() failed\n");
    free(buf);
    return (-1);
  }

  free(buf);

  res = ((unpackparam_p->id == test_paramid_mysint32) &&
	 (*(sint32_t*)unpackparam_p->value_p == mysint32));

  if (res != 1)
    return -1;

  if ((copy_size = mpl_param_value_copy_out(unpackparam_p, &mysint32_copy, 100)) < 0)
  {
    printf("mpl_param_value_copy_out() failed\n");
    mpl_param_element_destroy(unpackparam_p);
    return -1;
  }

  if (copy_size != sizeof(mysint32_copy))
  {
    printf("mpl_param_value_copy_out() failed to give correct size\n");
    mpl_param_element_destroy(unpackparam_p);
    return -1;
  }

  printf("mysint32_copy = %d, copy_size = %d\n", mysint32_copy, copy_size );

  if (mysint32_copy != *(sint32_t*)unpackparam_p->value_p)
  {
    printf("mpl_param_value_copy_out() failed to copy correctly\n");
    mpl_param_element_destroy(unpackparam_p);
    return -1;
  }

  if ((copy_size = mpl_param_value_copy_out(unpackparam_p, &mysint32_copy, 0)) != sizeof(mysint32_copy))
  {
    printf("mpl_param_value_copy_out() failed to give correct size\n");
    mpl_param_element_destroy(unpackparam_p);
    return -1;
  }

  mpl_param_element_destroy(unpackparam_p);


  /* Negative */
  if (mpl_param_unpack(buf, value_p, NULL) >= 0)
  {
    printf("mpl_param_unpack() unexpectedly (4) succeeded\n");
    free(buf);
    return (-1);
  }

  /* > max */
  if (mpl_param_unpack("test.mysint32", "6000", &unpackparam_p) >= 0)
  {
    printf("mpl_param_unpack() unexpectedly succeeded for mysint32=%s\n", "6000");
    return (-1);
  }

  /* MIN */
  mysint32_2 = INT32_MIN;
  par = mpl_param_element_create(test_paramid_mysint32_2, &mysint32_2);
  if (NULL == par)
  {
      printf("mpl_param_element_create(mysint32_2) failed\n");
      return -1;
  }
  if (pack_unpack_one(par, true, TEST_PARAM_SET_ID) < 0)
      return -1;

  mpl_param_element_destroy(par);

  /* MAX */
  mysint32_2 = INT32_MAX;
  par = mpl_param_element_create(test_paramid_mysint32_2, &mysint32_2);
  if (NULL == par)
  {
      printf("mpl_param_element_create(mysint32_2) failed\n");
      return -1;
  }
  if (pack_unpack_one(par, true, TEST_PARAM_SET_ID) < 0)
      return -1;

  mpl_param_element_destroy(par);

  return 0;
}

static int tc_param_list_add(void)
{
  int myint = 1000;
  uint8_t myuint8 = 10;
  uint16_t myuint16 = 100;
  uint32_t myuint32 = 2000;
  uint64_t myuint64 = 4000;
  sint8_t mysint8 = -10;
  sint16_t mysint16 = -100;
  sint32_t mysint32 = -2000;
  bool mybool = true;
  uint8_t mybool8 = 1;
  test_my_enum_t myenum = test_my_enum_val2;
  uint8_t myenum8 = test_my_enum8_smallval2;
  uint16_t myenum16 = test_my_enum16_val2;
  uint32_t myenum32 = test_my_enum32_val2;
  sint8_t mysenum8 = test_my_senum8_val2;
  sint16_t mysenum16 = test_my_senum16_val2;
  sint32_t mysenum32 = test_my_senum32_val2;

  mpl_list_t *param_list_p = NULL;
  mpl_list_t *param_list2_p = NULL;

  (void) mpl_add_param_to_list(&param_list_p, test_paramid_myint, &myint);
  (void) mpl_add_param_to_list(&param_list_p, test_paramid_myuint8, &myuint8);
  (void) mpl_add_param_to_list(&param_list_p, test_paramid_myuint16, &myuint16);
  (void) mpl_add_param_to_list(&param_list_p, test_paramid_myuint32, &myuint32);
  (void) mpl_add_param_to_list(&param_list_p, test_paramid_myuint64, &myuint64);
  (void) mpl_add_param_to_list(&param_list_p, test_paramid_mysint8, &mysint8);
  (void) mpl_add_param_to_list(&param_list_p, test_paramid_mysint16, &mysint16);
  (void) mpl_add_param_to_list(&param_list_p, test_paramid_mysint32, &mysint32);
  (void) mpl_add_param_to_list(&param_list_p, test_paramid_mybool, &mybool);
  (void) mpl_add_param_to_list(&param_list_p, test_paramid_mybool8, &mybool8);
  (void) mpl_add_param_to_list(&param_list_p, test_paramid_my_enum, &myenum);
  (void) mpl_add_param_to_list(&param_list_p, test_paramid_my_enum8, &myenum8);
  (void) mpl_add_param_to_list(&param_list_p, test_paramid_my_enum16, &myenum16);
  (void) mpl_add_param_to_list(&param_list_p, test_paramid_my_enum32, &myenum32);
  (void) mpl_add_param_to_list(&param_list_p, test_paramid_my_senum8, &mysenum8);
  (void) mpl_add_param_to_list(&param_list_p, test_paramid_my_senum16, &mysenum16);
  (void) mpl_add_param_to_list(&param_list_p, test_paramid_my_senum32, &mysenum32);

  (void) mpl_param_list_add_int(&param_list2_p, test_paramid_myint, myint);
  (void) mpl_param_list_add_uint8(&param_list2_p, test_paramid_myuint8, myuint8);
  (void) mpl_param_list_add_uint16(&param_list2_p, test_paramid_myuint16, myuint16);
  (void) mpl_param_list_add_uint32(&param_list2_p, test_paramid_myuint32, myuint32);
  (void) mpl_param_list_add_uint64(&param_list2_p, test_paramid_myuint64, myuint64);
  (void) mpl_param_list_add_sint8(&param_list2_p, test_paramid_mysint8, mysint8);
  (void) mpl_param_list_add_sint16(&param_list2_p, test_paramid_mysint16, mysint16);
  (void) mpl_param_list_add_sint32(&param_list2_p, test_paramid_mysint32, mysint32);
  (void) mpl_param_list_add_bool(&param_list2_p, test_paramid_mybool, mybool);
  (void) mpl_param_list_add_bool8(&param_list2_p, test_paramid_mybool8, mybool8);
  (void) mpl_param_list_add_enum(&param_list2_p, test_paramid_my_enum, myenum);
  (void) mpl_param_list_add_enum8(&param_list2_p, test_paramid_my_enum8, myenum8);
  (void) mpl_param_list_add_enum16(&param_list2_p, test_paramid_my_enum16, myenum16);
  (void) mpl_param_list_add_enum32(&param_list2_p, test_paramid_my_enum32, myenum32);
  (void) mpl_param_list_add_signed_enum8(&param_list2_p, test_paramid_my_senum8, mysenum8);
  (void) mpl_param_list_add_signed_enum16(&param_list2_p, test_paramid_my_senum16, mysenum16);
  (void) mpl_param_list_add_signed_enum32(&param_list2_p, test_paramid_my_senum32, mysenum32);

  printf("list1(%p) length: %zu\n", param_list_p, mpl_list_len(param_list_p));
  printf("list2(%p) length: %zu\n", param_list2_p, mpl_list_len(param_list2_p));


  if (mpl_compare_param_lists(param_list_p, param_list2_p))
  {
    printf("parameter list mismatch\n");
    mpl_param_list_destroy(&param_list_p);
    mpl_param_list_destroy(&param_list2_p);
    return -1;
  }

  mpl_param_list_destroy(&param_list_p);
  mpl_param_list_destroy(&param_list2_p);
  return 0;
}

int finished = 0;
#define NUM_THREADS 25
pthread_mutex_t mut;
pthread_cond_t cond;
int retval[NUM_THREADS];
int thread_worker_retval;

void *thread_entry_func(void *arg_p)
{
  int error = *((int*)arg_p);
  int error2;

  mpl_set_errno(error);

  pthread_mutex_lock(&mut);
  finished++;
  //printf("Waiting for other threads, *arg = %d\n", error);
  pthread_cond_wait(&cond, &mut);
  pthread_mutex_unlock(&mut);

  printf("Thread exiting, *arg = %d\n", error);

  if ((error2 = mpl_get_errno()) != error)
  {
    printf("Unexpected errno = %d (expected errno = %d)\n", error2, error);
    retval[error-1] = -1;
  }
  else
  {
    printf("Expected errno = %d (OK)\n", error2);
    retval[error-1] = 0;
  }

  pthread_exit(&retval[error-1]);

  return NULL;
}

static bool thread_worker_done = false;
void *thread_worker_func(void *arg_p)
{
  int i;
  pthread_t t[NUM_THREADS];
  int error[NUM_THREADS];

  thread_worker_retval = 0;

  printf("thread_worker_func , create threads\n");
  for (i = 0; i < NUM_THREADS; i++)
  {
      error[i] = i + 1;
  }

  pthread_mutex_init(&mut, NULL);
  pthread_cond_init(&cond, NULL);

  for (i = 0; i < NUM_THREADS; i++)
  {
    if (pthread_create(&t[i], NULL, thread_entry_func, &error[i]) != 0)
    {
      printf("pthread_create() failed\n");
      thread_worker_retval = -1;
      goto exit_return;
    }
  }

  printf("thread_worker_func , all threads created\n");

  while (finished < NUM_THREADS)
  {
    printf("Waiting for %d threads...\n", NUM_THREADS - finished);
    usleep(1000);
  }
  printf("%d threads finished\n", finished);
  finished = 0;

  pthread_mutex_lock(&mut);
  pthread_cond_broadcast(&cond);
  pthread_mutex_unlock(&mut);

  for (i = 0; i < NUM_THREADS; i++)
  {
      int *tmpret;
      if (pthread_join(t[i], (void**)&tmpret) == 0)
      {
          printf("Joined thread #%d, status = %d\n", i + 1, *tmpret);
      }
      thread_worker_retval = (thread_worker_retval || *tmpret);
  }

exit_return:
  pthread_mutex_destroy(&mut);
  pthread_cond_destroy(&cond);

  thread_worker_done = true;
  pthread_exit(&thread_worker_retval);
  return NULL;
}

static int tc_threads(void)
{
  pthread_t t;
  int *ret;

  pthread_create(&t, NULL, thread_worker_func, NULL);

#ifndef MPL_OSE_TEST
  pthread_join(t, (void**)&ret);
#else
  while(!thread_worker_done)
      usleep(1000);
  ret = &thread_worker_retval;
#endif
  return *ret;
}
static int tc_errno(void)
{
  int error;

  mpl_set_errno(E_MPL_FAILED_OPERATION);
  if ((error = mpl_get_errno()) != E_MPL_FAILED_OPERATION)
  {
    printf("Unexpected errno = %d\n", error);
    return -1;
  }

  mpl_set_errno(E_MPL_INVALID_PARAMETER);
  if ((error = mpl_get_errno()) != E_MPL_INVALID_PARAMETER)
  {
    printf("Unexpected errno = %d\n", error);
    return -1;
  }

  return 0;
}

static int  tc_sizeof_zero(void)
{
    if (mpl_param_id_sizeof_param_value(test_paramid_myint_tup) != sizeof(mpl_int_tuple_t))
    {
        return -1;
    }

    if (mpl_param_id_sizeof_param_value(test_paramid_mystrint_tup) != 0)
    {
        return -1;
    }

    if (mpl_param_id_sizeof_param_value(test_paramid_mystruint8_tup) != 0)
    {
        return -1;
    }

    if (mpl_param_id_sizeof_param_value(test_paramid_myuint8_arr) != 0)
    {
        return -1;
    }

    if (mpl_param_id_sizeof_param_value(test_paramid_mynewbag) != 0)
    {
        return -1;
    }

    if (mpl_param_id_sizeof_param_value(test_paramid_myint) != sizeof(int))
    {
        return -1;
    }

    if (mpl_param_id_sizeof_param_value(6) != 0)
    {
        return -1;
    }

    return 0;
}

int tc_misc_invalid(void)
{
    int myint = 100;
    int copy_int;

    /* Invalid parameter set */
    packparam.id = 8;
    packparam.value_p = &myint;

    if (mpl_param_value_copy_out(&packparam, &copy_int, sizeof(int)) == 0)
    {
        printf("mpl_param_value_copy_out() (1)\n");
        return -1;
    }

    /* No value */
    packparam.id = test_paramid_myint;
    packparam.value_p = NULL;

    if (mpl_param_value_copy_out(&packparam, &copy_int, sizeof(int)) == 0)
    {
        printf("mpl_param_value_copy_out() (2)\n");
        return -1;
    }

    if (mpl_param_allow_get_bl(9, NULL) == true)
    {
        printf("mpl_param_allow_get_bl()\n");
        return -1;
    }

    if (mpl_param_allow_set_bl(9, NULL) == true)
    {
        printf("mpl_param_allow_set_bl()\n");
        return -1;
    }

    if (mpl_param_allow_config_bl(9, NULL) == true)
    {
        printf("mpl_param_allow_config_bl()\n");
        return -1;
    }

    if (mpl_param_element_create_empty_tag(10, 0) != NULL)
    {
        printf("mpl_param_element_create_empty_tag()\n");
        return -1;
    }

    if (mpl_param_element_create_tag(11, 0, &myint) != NULL)
    {
        printf("mpl_param_element_create_tag()\n");
        return -1;
    }

    if (mpl_param_element_create_n_tag(11, 0, &myint, sizeof(myint)) != NULL)
    {
        printf("mpl_param_element_create_n_tag()\n");
        return -1;
    }

    packparam.id = test_paramid_myint;
    packparam.value_p = &myint;
    if (mpl_param_element_set_tag(&packparam, 100) == 0)
    {
        printf("mpl_param_element_set_tag()\n");
        return -1;
    }

    packparam.id = 12;
    if (mpl_param_element_clone(&packparam) != NULL)
    {
        printf("mpl_param_element_clone()\n");
        return -1;
    }

    packparam.id = 13;
    if (mpl_param_element_compare(&packparam, &packparam) == 0)
    {
        printf("mpl_param_element_compare()\n");
        return -1;
    }

    if (mpl_param_element_get_default_bl(14, NULL) != NULL)
    {
        printf("mpl_param_element_get_default_bl()\n");
        return -1;
    }

    if (mpl_param_element_create_empty_tag(tull_paramid_my_virtual_int_par, 0) != NULL)
    {
        printf("mpl_param_element_create_empty_tag(virtual parameter)\n");
        return -1;
    }
    return 0;
}


static int tc_param_type_bag(void)
{
  int myint = 1000;
  uint8_t myuint8 = 10;
  uint16_t myuint16 = 100;
  uint32_t myuint32 = 2000;
  uint64_t myuint64 = 4000;
  sint8_t mysint8 = -10;
  mpl_list_t *param_list0_p = NULL;
  mpl_list_t *param_list1_p = NULL;
  mpl_list_t *param_list2_p = NULL;
  int buflen;
  int buflen2;
  char *buf;
  char *buf2;
  char *b_p;
  int copy_size;
  mpl_list_t *mylist_copy;
  char *str;

  (void) mpl_add_param_to_list(&param_list1_p, test_paramid_myint, &myint);
  (void) mpl_add_param_to_list(&param_list1_p, test_paramid_myuint16, &myuint16);
  (void) mpl_add_param_to_list(&param_list1_p, test_paramid_myuint64, &myuint64);

  (void) mpl_add_param_to_list(&param_list2_p, test_paramid_myuint32, &myuint32);
  (void) mpl_add_param_to_list(&param_list2_p, test_paramid_myuint64, &myuint64);
  if (mpl_add_param_to_list_tag(&param_list2_p, test_paramid_mylist2, 1, param_list1_p) < 0)
      return -1;
  (void) mpl_add_param_to_list(&param_list2_p, test_paramid_mysint8, &mysint8);

  (void)mpl_add_param_to_list(&param_list0_p, test_paramid_myint, &myint);
  if (mpl_add_param_to_list(&param_list0_p, test_paramid_mylist2, param_list2_p) >= 0)
  {
      printf("mpl_add_param_to_list(), unexpected success\n");
      return -1;
  }
  if (mpl_add_param_to_list(&param_list0_p, test_paramid_mylist1, param_list2_p) < 0)
      return -1;
  (void) mpl_add_param_to_list(&param_list0_p, test_paramid_myuint8, &myuint8);
  (void) mpl_add_param_to_list(&param_list0_p, test_paramid_myuint16, &myuint16);

  packparam.id = test_paramid_mylist1;
  packparam.value_p = param_list0_p;

  buflen=mpl_param_pack(&packparam, NULL, 0);
  if (buflen<0)
  {
    printf("mpl_param_pack() failed\n");
    return (-1);
  }

  /* Alloc size including '\0' */
  buf = malloc(buflen+1);
  if (mpl_param_pack(&packparam, buf, buflen+1) != buflen)
  {
    printf("mpl_param_pack() failed\n");
    free(buf);
    return (-1);
  }

  printf("buf = '%s', strlen(buf) = %zu, buflen = %d\n", buf, strlen(buf), buflen);
/*   b_p = strstr(buf, "[1]"); */
/*   b_p++; */
/*   *b_p = '2'; */

  b_p = strstr(buf, "=");
  if (b_p == NULL)
  {
    printf("No '=' found in packet param\n");
    free(buf);
    return (-1);
  }
  *b_p = '\0';
  b_p++;

  if (mpl_param_unpack("test.mylist1", b_p, &unpackparam_p) < 0)
  {
    printf("mpl_param_unpack() failed\n");
    free(buf);
    return (-1);
  }

  if (0 != mpl_param_element_compare(&packparam, unpackparam_p))
  {
    printf("param %d value mismatch\n", packparam.id);
    free(buf);
    mpl_param_element_destroy(unpackparam_p);
    return -1;
  }

  buflen2=mpl_param_pack(unpackparam_p, NULL, 0);
  if (buflen2<0)
  {
    printf("mpl_param_pack() failed\n");
    return (-1);
  }

  buf2 = malloc(buflen2 + 1);

  buflen2=mpl_param_pack(unpackparam_p, buf2, buflen2+1);
  if (buflen2<0)
  {
    printf("mpl_param_pack() failed\n");
    free(buf);
    free(buf2);
    return (-1);
  }


  printf("buf2 = '%s', buflen2 = %d\n", buf2, buflen2);


  if ((copy_size = mpl_param_value_copy_out(unpackparam_p, &mylist_copy, 100)) < 0)
  {
    printf("mpl_param_value_copy_out() failed\n");
    mpl_param_element_destroy(unpackparam_p);
    return -1;
  }

  if (copy_size != sizeof(mylist_copy))
  {
    printf("mpl_param_value_copy_out() failed to give correct size\n");
    mpl_param_element_destroy(unpackparam_p);
    return -1;
  }

  printf("mylist_copy = %p, unpackparam_p->value_p = %p, copy_size = %d\n", mylist_copy, (mpl_list_t*)unpackparam_p->value_p, copy_size );

  if (mpl_compare_param_lists(param_list0_p, mylist_copy) != 0)
  {
    printf("mpl_param_value_copy_out(): copied list differs\n");
    mpl_param_element_destroy(unpackparam_p);
    return -1;
  }

  mpl_dbg_param_list_print(mylist_copy);
  if (mylist_copy != (mpl_list_t*)unpackparam_p->value_p)
  {
    printf("mpl_param_value_copy_out() failed to copy correctly\n");
    mpl_param_element_destroy(unpackparam_p);
    return -1;
  }

  if ((copy_size = mpl_param_value_copy_out(unpackparam_p, &mylist_copy, 0)) != sizeof(mylist_copy))
  {
    printf("mpl_param_value_copy_out() failed to give correct size\n");
    mpl_param_element_destroy(unpackparam_p);
    return -1;
  }


  free(buf);
  free(buf2);
  mpl_param_element_destroy(unpackparam_p);
  mpl_param_list_destroy(&param_list0_p);
  mpl_param_list_destroy(&param_list1_p);
  mpl_param_list_destroy(&param_list2_p);

  /* Negative */
  if (mpl_param_unpack("test.mylist1", "{test.myint=12}", NULL) >= 0)
  {
    printf("mpl_param_unpack() unexpectedly succeeded\n");
    free(buf);
    return (-1);
  }

  if (mpl_param_unpack("test.mylist1", "{test.myint=12", &unpackparam_p) >= 0)
  {
    printf("mpl_param_unpack() unexpectedly succeeded for '%s'\n", "{test.myint=12");
    return (-1);
  }

  if (mpl_param_unpack("test.mylist1", "test.myint=12}", &unpackparam_p) >= 0)
  {
    printf("mpl_param_unpack() unexpectedly succeeded for '%s'\n", "test.myint=12}");
    return (-1);
  }

  if (mpl_param_unpack("test.mylist1", "{}", &unpackparam_p) != 0)
  {
    printf("mpl_param_unpack() failed for '%s'\n", "{}");
    return (-1);
  }

  if (unpackparam_p->value_p != NULL)
  {
    printf("unpackparam_p->value_p != NULL for '%s'\n", "{}");
    return (-1);
  }
  mpl_param_element_destroy(unpackparam_p);
  if (mpl_param_unpack("test.mylist1", "{ }", &unpackparam_p) != 0)
  {
    printf("mpl_param_unpack() failed for '%s'\n", "{}");
    return (-1);
  }
  if (unpackparam_p->value_p != NULL)
  {
    printf("unpackparam_p->value_p != NULL for '%s'\n", "{}");
    return (-1);
  }
  mpl_param_element_destroy(unpackparam_p);

  /* Bags in bag */
  str = "{tull.mybag2={tull.myint=10},tull.mybag3={tull.myint=20}}";
  if (mpl_param_unpack("tull.mybag1", str, &unpackparam_p) < 0)
  {
    printf("mpl_param_unpack() unexpectedly failed for bags in bag 1\n");
    mpl_param_element_destroy(unpackparam_p);
    return (-1);
  }

  buflen2=mpl_param_pack(unpackparam_p, NULL, 0);
  if (buflen2<0)
  {
    printf("mpl_param_pack() failed\n");
    return (-1);
  }

  buf2 = malloc(buflen2 + 1);

  buflen2=mpl_param_pack(unpackparam_p, buf2, buflen2+1);
  if (buflen2<0)
  {
    printf("mpl_param_pack() failed\n");
    free(buf);
    free(buf2);
    return (-1);
  }

  printf("buf2 = '%s', buflen2 = %d, str = '%s', strlen(str) = %d\n", buf2, buflen2, str, (int)strlen(str) );

  if ((strlen(str) + strlen("tull.mybag1=")) != strlen(buf2)) {
    printf("string lengths differ (1)\n");
    free(buf);
    free(buf2);
    return (-1);
  }

  free(buf2);
  mpl_param_element_destroy(unpackparam_p);

  str = "{tull.mybag2={tull.myint=10},tull.mybag4={tull.mybag3={tull.myint=30}},tull.mybag3={tull.myint=20}}";
  if (mpl_param_unpack("tull.mybag1", str, &unpackparam_p) < 0)
  {
    printf("mpl_param_unpack() unexpectedly failed for bags in bag 2\n");
    mpl_param_element_destroy(unpackparam_p);
    return (-1);
  }
  buflen2=mpl_param_pack(unpackparam_p, NULL, 0);
  if (buflen2<0)
  {
    printf("mpl_param_pack() failed\n");
    return (-1);
  }

  buf2 = malloc(buflen2 + 1);

  buflen2=mpl_param_pack(unpackparam_p, buf2, buflen2+1);
  if (buflen2<0)
  {
    printf("mpl_param_pack() failed\n");
    free(buf);
    free(buf2);
    return (-1);
  }

  printf("buf2 = '%s', buflen2 = %d\n", buf2, buflen2);

  if ((strlen(str) + strlen("tull.mybag1=")) != strlen(buf2)) {
    printf("string lengths differ (2)\n");
    free(buf);
    free(buf2);
    return (-1);
  }

  free(buf2);
  mpl_param_element_destroy(unpackparam_p);

  /* Negative: Max value check */
  if (mpl_param_unpack("test.mylist2", "{test.myint[1]=1,test.myint[2]=2,test.myint[3]=3,test.myint[4]=4}", &unpackparam_p) >= 0)
  {
    printf("mpl_param_unpack() unexpectedly succeeded for min value check\n");
    mpl_param_element_destroy(unpackparam_p);
    return (-1);
  }

  /* Negative: Min value check */
  if (mpl_param_unpack("test.mylist2", "{test.myint[1]=1}", &unpackparam_p) >= 0)
  {
    printf("mpl_param_unpack() unexpectedly succeeded for min value check\n");
    mpl_param_element_destroy(unpackparam_p);
    return (-1);
  }

  /* Sanity check */
  if (mpl_param_unpack("test.mylist2", "{test.myint[1]=1,test.myint[2]=2,test.myint[3]=3}", &unpackparam_p) != 0)
  {
    printf("mpl_param_unpack() unexpectedly failed for mylist2\n");
    return (-1);
  }

  mpl_param_element_destroy(unpackparam_p);
  return 0;
}


static int
tc_pack_unpack_param_addr(void)
{
  uint32_t myuint32 = 2000;
  void *myaddr = &myuint32;
  void *myaddr_copy = NULL;
  int copy_size;
  int size;
  char* value_p;
  mpl_param_element_t* clone_p;

  packparam.id = test_paramid_myaddr;
  packparam.value_p = &myaddr;

  /* Find out space need */
  size=mpl_param_pack(&packparam, NULL, 0);
  if (size<0)
  {
    printf("mpl_param_pack() failed\n");
    return (-1);
  }

  /* Alloc size including '\0' */
  buf = malloc(size+1);

  size=mpl_param_pack(&packparam, buf, size+1);
  if (size<0)
  {
    printf("mpl_param_pack() failed\n");
    free(buf);
    return (-1);
  }

  printf("myaddr = %p, buf = '%s'\n", myaddr, buf);

  value_p = strstr(buf, "=");
  if (value_p == NULL)
  {
    printf("No '=' found in packet param\n");
    free(buf);
    return (-1);
  }
  *value_p = '\0';
  value_p++;

  if (mpl_param_unpack(buf, value_p, &unpackparam_p) < 0)
  {
    printf("mpl_param_unpack() failed\n");
    free(buf);
    return (-1);
  }

  free(buf);

  printf("myaddr = %p, *unpackparam_p->value_p = %p\n",
         myaddr,
         *(void**)unpackparam_p->value_p);

  if ((unpackparam_p->id != packparam.id) ||
      (*(void**)unpackparam_p->value_p != myaddr))
  {
    mpl_param_element_destroy(unpackparam_p);
    return -1;
  }

  clone_p = mpl_param_element_clone(unpackparam_p);
  if (NULL == clone_p)
  {
    printf("mpl_param_element_clone() failed\n");
    mpl_param_element_destroy(unpackparam_p);
    return (-1);
  }

  if (*(void**)clone_p->value_p != *(void**)unpackparam_p->value_p)
  {
    printf("mpl_param_element_clone() failed to clone correctly\n");
    mpl_param_element_destroy(clone_p);
    mpl_param_element_destroy(unpackparam_p);
    return (-1);
  }

  mpl_param_element_destroy(clone_p);

  if ((copy_size = mpl_param_value_copy_out(unpackparam_p, &myaddr_copy, 100)) < 0)
  {
    printf("mpl_param_value_copy_out() failed\n");
    mpl_param_element_destroy(unpackparam_p);
    return -1;
  }

  if (copy_size != sizeof(myaddr_copy))
  {
    printf("mpl_param_value_copy_out() failed to give correct size\n");
    mpl_param_element_destroy(unpackparam_p);
    return -1;
  }

  printf("myaddr_copy = %p, copy_size = %u\n", myaddr_copy, copy_size );

  if (myaddr_copy != *(void**)unpackparam_p->value_p)
  {
    printf("mpl_param_value_copy_out() failed to copy correctly\n");
    mpl_param_element_destroy(unpackparam_p);
    return -1;
  }

  if ((copy_size = mpl_param_value_copy_out(unpackparam_p, &myaddr_copy, 0)) != sizeof(myaddr_copy))
  {
    printf("mpl_param_value_copy_out() failed to give correct size\n");
    mpl_param_element_destroy(unpackparam_p);
    return -1;
  }

  mpl_param_element_destroy(unpackparam_p);

  /* Negative: Non address */
  if (mpl_param_unpack("test.myaddr", "test", &unpackparam_p) >= 0)
  {
    printf("mpl_param_unpack() unexpectedly succeeded for Non integer\n");
    mpl_param_element_destroy(unpackparam_p);
    return (-1);
  }
  return 0;
}

static int tc_enum_with_values(void)
{
  test_my_enum4_t myenum4;
  test_my_enum4_t myenum4_copy;
  test_my_enum4_t values[] = {
      test_my_enum4_val1,
      test_my_enum4_val2,
      test_my_enum4_val3,
      test_my_enum4_val4,
      test_my_enum4_val5,
      test_my_enum4_val6
  };
  int i;
  int size;
  char* value_p;
  int res=0;
  int copy_size;

  for (i = 0; i < 6; i++) {
    myenum4 = values[i];
      
    packparam.id = test_paramid_my_enum4;
    packparam.value_p = &myenum4;

    /* Find out space need */
    size=mpl_param_pack(&packparam, NULL, 0);
    if (size<0)
    {
      printf("mpl_param_pack() failed (1) for myenum4=%d\n", myenum4);
      return (-1);
    }

    /* Alloc size including '\0' */
    buf = malloc(size+1);

    size=mpl_param_pack(&packparam, buf, size+1);
    if (size<0)
    {
      printf("mpl_param_pack() failed (2) for myenum4=%d\n", myenum4);
      free(buf);
      return (-1);
    }

    printf("pack: %s\n", buf);

    value_p = strstr(buf, "=");
    if (value_p == NULL)
    {
      printf("No '=' found in packet param for myenum4=%d\n", myenum4);
      free(buf);
      return (-1);
    }
    *value_p = '\0';
    value_p++;

    if (mpl_param_unpack(buf, value_p, &unpackparam_p) < 0)
    {
      printf("mpl_param_unpack() failed (3) for myenum4=%d\n", myenum4);
      free(buf);
      return (-1);
    }

    free(buf);

    res = ((unpackparam_p->id == test_paramid_my_enum4) &&
	   (*(test_my_enum4_t*)unpackparam_p->value_p == myenum4));
    mpl_param_element_destroy(unpackparam_p);

    if (res != 1)
      return -1;
  }

  /* Numeric value */
  if (mpl_param_unpack("test.my_enum4", "33", &unpackparam_p) < 0)
  {
    printf("mpl_param_unpack() failed (4) for myenum4=%s\n", "1");
    return (-1);
  }

  res = ((unpackparam_p->id == test_paramid_my_enum4) &&
         (*(test_my_enum4_t*)unpackparam_p->value_p == 33));
  mpl_param_element_destroy(unpackparam_p);

  if (res != 1)
    return -1;

  /* Symbolic value */
  if (mpl_param_unpack("test.my_enum4", "val3", &unpackparam_p) < 0)
  {
    printf("mpl_param_unpack() failed (4) for myenum4=%s\n", "1");
    return (-1);
  }

  res = ((unpackparam_p->id == test_paramid_my_enum4) &&
         (*(test_my_enum4_t*)unpackparam_p->value_p == 33));
  mpl_param_element_destroy(unpackparam_p);

  if (res != 1)
    return -1;

  /* Copy out */
  if (mpl_param_unpack("test.my_enum4", "val2", &unpackparam_p) < 0)
  {
    printf("mpl_param_unpack() failed (4) for myenum4=%s\n", "val2");
    return (-1);
  }

  if ((copy_size = mpl_param_value_copy_out(unpackparam_p, &myenum4_copy, 100)) < 0)
  {
    printf("mpl_param_value_copy_out() failed\n");
    mpl_param_element_destroy(unpackparam_p);
    return -1;
  }

  if (copy_size != sizeof(myenum4_copy))
  {
    printf("mpl_param_value_copy_out() failed to give correct size\n");
    mpl_param_element_destroy(unpackparam_p);
    return -1;
  }

  printf("myenum4_copy = %d, copy_size = %d\n", myenum4_copy, copy_size );

  if (myenum4_copy != *(test_my_enum4_t*)unpackparam_p->value_p)
  {
    printf("mpl_param_value_copy_out() failed to copy correctly\n");
    mpl_param_element_destroy(unpackparam_p);
    return -1;
  }

  mpl_param_element_destroy(unpackparam_p);

  /* Out of range */
  if (mpl_param_unpack("test.my_enum4", "10", &unpackparam_p) >= 0)
  {
    printf("mpl_param_unpack() unexpectedly succeeded for myenum4=%s\n", "10");
    return (-1);
  }

  return 0;
}

static int tc_integer_ranges(void)
{
  int val;
  uint8_t uint8val;
  int range_id;
    
  /* Positive: Inside range */
  if (mpl_param_unpack("test.my_ranged_int1", "10", &unpackparam_p) < 0)
  {
    printf("mpl_param_unpack() unexpectedly failed for range check\n");
    return (-1);
  }
  mpl_param_element_destroy(unpackparam_p);

  if (mpl_param_unpack("test.my_ranged_int_child1", "10", &unpackparam_p) < 0)
  {
    printf("mpl_param_unpack() unexpectedly failed for range check\n");
    return (-1);
  }
  mpl_param_element_destroy(unpackparam_p);
  
  if (mpl_param_unpack("test.my_ranged_int_child2", "-1", &unpackparam_p) < 0)
  {
    printf("mpl_param_unpack() unexpectedly failed for range check\n");
    return (-1);
  }
  mpl_param_element_destroy(unpackparam_p);
  
  if (mpl_param_unpack("test.my_ranged_int1", "20", &unpackparam_p) < 0)
  {
    printf("mpl_param_unpack() unexpectedly failed for range check\n");
    mpl_param_element_destroy(unpackparam_p);
    return (-1);
  }
  mpl_param_element_destroy(unpackparam_p);
  
  if (mpl_param_unpack("test.my_ranged_int3", "-95", &unpackparam_p) < 0)
  {
    printf("mpl_param_unpack() unexpectedly failed for range check\n");
    mpl_param_element_destroy(unpackparam_p);
    return (-1);
  }
  mpl_param_element_destroy(unpackparam_p);
  
  /* Negative: Outside range */
  if (mpl_param_unpack("test.my_ranged_int1", "9", &unpackparam_p) >= 0)
  {
    printf("mpl_param_unpack() unexpectedly succeeded for range check\n");
    mpl_param_element_destroy(unpackparam_p);
    return (-1);
  }

  if (mpl_param_unpack("test.my_ranged_int1", "21", &unpackparam_p) >= 0)
  {
    printf("mpl_param_unpack() unexpectedly succeeded for range check\n");
    mpl_param_element_destroy(unpackparam_p);
    return (-1);
  }

  if (mpl_param_unpack("test.my_ranged_int_child1", "9", &unpackparam_p) >= 0)
  {
    printf("mpl_param_unpack() unexpectedly succeeded for range check\n");
    mpl_param_element_destroy(unpackparam_p);
    return (-1);
  }

  if (mpl_param_unpack("test.my_ranged_int_child1", "21", &unpackparam_p) >= 0)
  {
    printf("mpl_param_unpack() unexpectedly succeeded for range check\n");
    mpl_param_element_destroy(unpackparam_p);
    return (-1);
  }

  if (mpl_param_unpack("test.my_ranged_int_child2", "-6", &unpackparam_p) >= 0)
  {
    printf("mpl_param_unpack() unexpectedly succeeded for range check\n");
    mpl_param_element_destroy(unpackparam_p);
    return (-1);
  }

  if (mpl_param_unpack("test.my_ranged_int_child2", "6", &unpackparam_p) >= 0)
  {
    printf("mpl_param_unpack() unexpectedly succeeded for range check\n");
    mpl_param_element_destroy(unpackparam_p);
    return (-1);
  }

  /* Positive: Inside range */
  if (mpl_param_unpack("test.my_ranged_uint8", "10", &unpackparam_p) < 0)
  {
    printf("mpl_param_unpack() unexpectedly failed for range check\n");
    return (-1);
  }
  mpl_param_element_destroy(unpackparam_p);
  
  if (mpl_param_unpack("test.my_ranged_uint8", "70", &unpackparam_p) < 0)
  {
    printf("mpl_param_unpack() unexpectedly failed for range check\n");
    mpl_param_element_destroy(unpackparam_p);
    return (-1);
  }
  mpl_param_element_destroy(unpackparam_p);
  
  if (mpl_param_unpack("test.my_ranged_uint8", "40", &unpackparam_p) < 0)
  {
    printf("mpl_param_unpack() unexpectedly failed for range check\n");
    mpl_param_element_destroy(unpackparam_p);
    return (-1);
  }
  mpl_param_element_destroy(unpackparam_p);
  
  /* Negative: Outside range */
  if (mpl_param_unpack("test.my_ranged_uint8", "4", &unpackparam_p) >= 0)
  {
    printf("mpl_param_unpack() unexpectedly succeeded for range check\n");
    mpl_param_element_destroy(unpackparam_p);
    return (-1);
  }

  if (mpl_param_unpack("test.my_ranged_uint8", "21", &unpackparam_p) >= 0)
  {
    printf("mpl_param_unpack() unexpectedly succeeded for range check\n");
    mpl_param_element_destroy(unpackparam_p);
    return (-1);
  }

  if (mpl_param_unpack("test.my_ranged_uint8", "69", &unpackparam_p) >= 0)
  {
    printf("mpl_param_unpack() unexpectedly succeeded for range check\n");
    mpl_param_element_destroy(unpackparam_p);
    return (-1);
  }

  if (mpl_param_unpack("test.my_ranged_uint8", "81", &unpackparam_p) >= 0)
  {
    printf("mpl_param_unpack() unexpectedly succeeded for range check\n");
    mpl_param_element_destroy(unpackparam_p);
    return (-1);
  }

  if (mpl_param_unpack("test.my_ranged_uint8", "29", &unpackparam_p) >= 0)
  {
    printf("mpl_param_unpack() unexpectedly succeeded for range check\n");
    mpl_param_element_destroy(unpackparam_p);
    return (-1);
  }

  if (mpl_param_unpack("test.my_ranged_uint8", "41", &unpackparam_p) >= 0)
  {
    printf("mpl_param_unpack() unexpectedly succeeded for range check\n");
    mpl_param_element_destroy(unpackparam_p);
    return (-1);
  }

  /* Range ids */
  val = 10;
  range_id = mpl_param_value_get_range_id(test_paramid_my_ranged_int3, &val);
  if (range_id != TST_NUMBER_RANGE_ID(ANONYMOUS))
  {
      printf("mpl_param_value_get_range_id() failed for value %d, returned %d\n", val, range_id);
      return (-1);
  }
  printf("val = %d, range_id = %d, range=%s\n",
         val,
         range_id,
         TST_NUMBER_RANGE_NAME_PTR(range_id));
  
  val = 75;
  range_id = mpl_param_value_get_range_id(test_paramid_my_ranged_int3, &val);
  if (range_id != TST_NUMBER_RANGE_ID(range1))
  {
      printf("mpl_param_value_get_range_id() failed for value %d, returned %d\n", val, range_id);
      return (-1);
  }
  printf("val = %d, range_id = %d, range=%s\n",
         val,
         range_id,
         TST_NUMBER_RANGE_NAME_PTR(range_id));
  
  val = -40;
  range_id = mpl_param_value_get_range_id(test_paramid_my_ranged_int3, &val);
  if (range_id != TST_NUMBER_RANGE_ID(ANONYMOUS))
  {
      printf("mpl_param_value_get_range_id() failed for value %d, returned %d\n", val, range_id);
      return (-1);
  }
  printf("val = %d, range_id = %d, range=%s\n",
         val,
         range_id,
         TST_NUMBER_RANGE_NAME_PTR(range_id));
  
  val = 105;
  range_id = mpl_param_value_get_range_id(test_paramid_my_ranged_int3, &val);
  if (range_id != TST_NUMBER_RANGE_ID(range2))
  {
      printf("mpl_param_value_get_range_id() failed for value %d, returned %d\n", val, range_id);
      return (-1);
  }
  printf("val = %d, range_id = %d, range=%s\n",
         val,
         range_id,
         TST_NUMBER_RANGE_NAME_PTR(range_id));
  
  val = -95;
  range_id = mpl_param_value_get_range_id(test_paramid_my_ranged_int3, &val);
  if (range_id != TST_NUMBER_RANGE_ID(range2))
  {
      printf("mpl_param_value_get_range_id() failed for value %d, returned %d\n", val, range_id);
      return (-1);
  }
  printf("val = %d, range_id = %d, range=%s\n",
         val,
         range_id,
         TST_NUMBER_RANGE_NAME_PTR(range_id));

  uint8val = 71;
  range_id = mpl_param_value_get_range_id(test_paramid_my_ranged_uint8, &uint8val);
  if (range_id != TST_NUMBER_RANGE_ID(range1))
  {
      printf("mpl_param_value_get_range_id() failed for value %d, returned %d\n", uint8val, range_id);
      return (-1);
  }
  printf("uint8val = %d, range_id = %d, range=%s\n",
         uint8val,
         range_id,
         TST_NUMBER_RANGE_NAME_PTR(range_id));

  uint8val = 1;
  range_id = mpl_param_value_get_range_id(test_paramid_my_ranged_uint8, &uint8val);
  if (range_id != -1)
  {
      printf("mpl_param_value_get_range_id() succeeded for value %d, returned %d\n", uint8val, range_id);
      return (-1);
  }
  printf("uint8val = %d, range_id = %d, range=%s\n",
         uint8val,
         range_id,
         TST_NUMBER_RANGE_NAME_PTR(range_id));

  return 0;
}


int pack_unpack_one(mpl_param_element_t *packparam_p, bool has_value, int param_set_id)
{
    char *buf;
    int buflen;
    char *sep_p;

    if (param_set_id > 0)
        buflen=mpl_param_pack_no_prefix(packparam_p, NULL, 0);
    else
        buflen=mpl_param_pack(packparam_p, NULL, 0);

    if (buflen<0)
    {
        printf("mpl_param_pack() failed\n");
        return (-1);
    }

    /* Alloc size including '\0' */
    buf = malloc(buflen+1);
    if (param_set_id > 0) {
        if (mpl_param_pack_no_prefix(packparam_p, buf, buflen+1) != buflen)
        {
            printf("mpl_param_pack() failed\n");
            free(buf);
            return (-1);
        }
    }
    else {
        if (mpl_param_pack(packparam_p, buf, buflen+1) != buflen)
        {
            printf("mpl_param_pack() failed\n");
            free(buf);
            return (-1);
        }
    }

    printf("buf = '%s', strlen(buf) = %zu, buflen = %d\n", buf, strlen(buf), buflen);
    sep_p = strchr(buf, '=');
    if (sep_p) {
        *sep_p = 0;
        sep_p++;
    }
    else {
        if (has_value) {
            printf("No value found!\n");
            free(buf);
            return -1;
        }
    }

    /* Unpack one field */
    if (param_set_id > 0) {
        if (mpl_param_unpack_param_set(buf, sep_p, &unpackparam_p, param_set_id) < 0)
        {
            printf("mpl_param_unpack() unexpectedly failed for field unpack\n");
            free(buf);
            return (-1);
        }
    }
    else {
        if (mpl_param_unpack(buf, sep_p, &unpackparam_p) < 0)
        {
            printf("mpl_param_unpack() unexpectedly failed for field unpack\n");
            free(buf);
            return (-1);
        }
    }

    printf("Orig id: %d\n", packparam_p->id);
    printf("Orig tag: %d\n", packparam_p->tag);
    printf("Unpacked id: %d\n", unpackparam_p->id);
    printf("Unpacked tag: %d\n", unpackparam_p->tag);

    if (mpl_param_element_compare(packparam_p, unpackparam_p) != 0)
    {
        printf("param %d value mismatch\n", packparam_p->id);
        free(buf);
        mpl_param_element_destroy(unpackparam_p);
        return -1;
    }

    printf("Unpack and compare ok: '%s' '%s'\n", buf, sep_p);

    mpl_param_element_destroy(unpackparam_p);
    free(buf);
    return 0;
}

static void checkRet(int check_ret, mpl_list_t *result_list_p)
{
    if (check_ret != 0) {
        char *buf;
        int len;
        mpl_pack_options_t options = MPL_PACK_OPTIONS_DEFAULT;
        options.message_delimiter = '\n';
        options.force_field_pack_mode = true;

        printf("checkBag returned %d\n", check_ret);
        len = mpl_param_list_pack_extended(result_list_p,
                                           NULL,
                                           0,
                                           &options
                                          );
        buf = calloc(1, len + 1);
        if (buf != NULL) {
            mpl_param_list_pack_extended(result_list_p,
                                         buf,
                                         len+1,
                                         &options
                                        );
            printf("result_list:\n%s\n", buf);
            free(buf);
        }
    }
}

static int tc_bag_fields(void)
{
    mpl_bag_t *bag_p = NULL;
    mpl_bag_t *bag_clone_p = NULL;
    mpl_bag_t *l1_bag_p = NULL;
    int i;
    char *s;
    int i1 = 100;
    int i2 = 200;
    int i3 = 300;
    int i4 = 400;
    bool b = true;
    char *s1 = "hello there";
    char *s2 = "hello again";
    char *s3 = "bye bye";
    uint8_t u8a[] = {11,12,13,14,15};
    mpl_uint8_array_t u8_arr = {5, u8a};
    uint16_t u16a[] = {1,2,3,4,5};
    mpl_uint16_array_t u16_arr = {5, u16a};
    uint32_t u32a[] = {6,7,8,9,10};
    mpl_uint32_array_t u32_arr = {5, u32a};
    char *st_key = "my string tup key";
    char *st_val = "my string tup val";
    mpl_string_tuple_t strtup = {st_key, st_val};
    int it_key = 50;
    int it_val = 60;
    mpl_int_tuple_t ittup = {it_key, it_val};
    char *sit_key = "my strint tuple key";
    int sit_val = 70;
    mpl_strint_tuple_t sittup = {sit_key, sit_val};
    char *s8t_key = "my struint8 tuple key";
    int s8t_val = 80;
    mpl_struint8_tuple_t s8ttup = {s8t_key, s8t_val};
    mpl_list_t *result_list_p = NULL;
    int check_ret;
    int len;

    mpl_list_t *tmp_p;
    TST_ENUM_VAR_DECLARE_INIT(my_enum3,e3,val3);
    TLL_ENUM_VAR_DECLARE_INIT(my_enum4,e4,val8);
    TLL_ENUM_VAR_DECLARE_INIT(my_enum2,e2,val4);

    packparam.id = mpl_param_get_bag_field_id(test_paramid_mynewbag, TST_FIELD_INDEX(mynewbag, i1));
    packparam.context = test_paramid_mynewbag;
    packparam.id_in_context = TST_FIELD_INDEX(mynewbag, i1);
    packparam.value_p = &i1;
    packparam.tag = 77;

    if (pack_unpack_one(&packparam, true, -1) < 0)
        return-1;
    memset(&packparam, 0, sizeof(packparam));

    TST_ADD_mynewbag_i1(&bag_p,111);
    TST_ADD_mynewbag_i2(&bag_p,i2);
    TST_ADD_mynewbag_b(&bag_p,b);
    TST_ADD_mynewbag_s_TAG(&bag_p,s1,1);
    TST_ADD_mynewbag_s_TAG(&bag_p,s2,2);
    TST_ADD_mynewbag_s_TAG(&bag_p,s3,3);

    packparam.id = test_paramid_mynewbag;
    packparam.value_p = bag_p;
    if (pack_unpack_one(&packparam, true, -1) < 0)
        return-1;

    mpl_param_list_destroy(&bag_p);
    bag_p = NULL;
    memset(&packparam, 0, sizeof(packparam));

    TLL_ADD_BAG_FIELD_CHILD_TAG(&bag_p, mynewbag, e1, TST_PARAM_ID(my_enum3), &e3, 1);
    TLL_ADD_mynewbag_i3(&bag_p,i3);
    TLL_ADD_mynewbag_i4(&bag_p,i4);
    TLL_ADD_mynewbag_i1(&bag_p,i1);
    TLL_ADD_mynewbag_i2(&bag_p,i2);
    TLL_ADD_mynewbag_b(&bag_p,b);
    TLL_ADD_ENUM_FROM_VALUE(&bag_p,my_enum,val2);
    TLL_ADD_mynewbag_s_TAG(&bag_p,s1,1);
    TLL_ADD_mynewbag_s_TAG(&bag_p,s2,2);
    TLL_ADD_mynewbag_s_TAG(&bag_p,s3,3);

    TLL_ADD_INT(&l1_bag_p, myint, 500);
    TLL_ADD_ENUM_FROM_VALUE(&l1_bag_p, my_enum3, val3);
    TLL_ADD_mychild_list2_e4_ENUM(&l1_bag_p, val8);
    TLL_ADD_mynewbag_l1_CHILD(&bag_p,mychild_list2,l1_bag_p);

    MPL_LIST_FOR_EACH(bag_p, tmp_p) {
        mpl_param_element_t *elem_p;
        elem_p = mpl_param_element_clone(MPL_LIST_CONTAINER(tmp_p,
                                                            mpl_param_element_t,
                                                            list_entry));
        pack_unpack_one(elem_p,true, -1);
        MPL_PARAM_ELEMENT_CLEAR_FIELD_INFO(elem_p);
        pack_unpack_one(elem_p,true, -1);
        mpl_param_element_destroy(elem_p);
    }

    packparam.id = tull_paramid_mynewbag;
    packparam.value_p = bag_p;

    if (pack_unpack_one(&packparam, true, -1) < 0)
        return -1;

    if (pack_unpack_one(&packparam, true, TULL_PARAM_SET_ID) < 0)
        return -1;

    mpl_param_list_destroy(&bag_p);
    bag_p = NULL;
    mpl_param_list_destroy(&l1_bag_p);
    l1_bag_p = NULL;
    memset(&packparam, 0, sizeof(packparam));

    TLL_ADD_mynewbag_e1_CHILD_TAG(&bag_p,my_enum2,3,22);
    TLL_ADD_mynewbag_i3(&bag_p,i3);
    TLL_ADD_mynewbag_i4(&bag_p,i4);
    TLL_ADD_mynewbag_i1(&bag_p,i1);
    TLL_ADD_mynewbag_i2(&bag_p,i2);
    TLL_ADD_mynewbag_b(&bag_p,b);
    TLL_ADD_ENUM_FROM_VALUE(&bag_p,my_enum,val2);
    TLL_ADD_mynewbag_s_TAG(&bag_p,s1,1);
    TLL_ADD_mynewbag_s_TAG(&bag_p,s2,2);
    TLL_ADD_mynewbag_s_TAG(&bag_p,s3,3);
    TLL_ADD_mynewbag_st(&bag_p, &strtup);
    TLL_ADD_mynewbag_it(&bag_p, &ittup);
    TLL_ADD_mynewbag_sit(&bag_p, &sittup);
    TLL_ADD_mynewbag_s8t(&bag_p, &s8ttup);
    TLL_ADD_mynewbag_a8(&bag_p, &u8_arr);
    TLL_ADD_mynewbag_a16(&bag_p, &u16_arr);
    TLL_ADD_mynewbag_a32(&bag_p, &u32_arr);

    TLL_ADD_myparent_list_i(&l1_bag_p, 500);
    TLL_ADD_ENUM_FROM_VALUE(&l1_bag_p, my_enum3, val3);
    TLL_ADD_mychild_list2_e4(&l1_bag_p, e4);
    TLL_ADD_mynewbag_l1_CHILD(&bag_p,mychild_list2,l1_bag_p);
    mpl_param_list_destroy(&l1_bag_p);

    MPL_LIST_FOR_EACH(bag_p, tmp_p) {
        mpl_param_element_t *elem_p;
        elem_p = mpl_param_element_clone(MPL_LIST_CONTAINER(tmp_p,
                                                            mpl_param_element_t,
                                                            list_entry));
        pack_unpack_one(elem_p,true, -1);
        MPL_PARAM_ELEMENT_CLEAR_FIELD_INFO(elem_p);
        pack_unpack_one(elem_p,true, -1);
        mpl_param_element_destroy(elem_p);
    }

    packparam.id = tull_paramid_mynewbag;
    packparam.value_p = bag_p;

    if (pack_unpack_one(&packparam, true, -1) < 0)
        return -1;

    if (pack_unpack_one(&packparam, true, TULL_PARAM_SET_ID) < 0)
        return -1;



    if (!TLL_mynewbag_e1_EXISTS_TAG(bag_p,22)) {
        printf("Field exists failed: e1\n");
        return -1;
    }

    e2 = TLL_GET_mynewbag_e1_TAG(bag_p,22);
    printf("Field get: e2: %d\n", e2);
    if (e2 != 3) {
        printf("Field get failed: %d != %d\n", e2, 3);
        return -1;
    }



    if (!TLL_mynewbag_i1_EXISTS(bag_p)) {
        printf("Field exists failed: i1\n");
        return -1;
    }

    i = TLL_GET_mynewbag_i1(bag_p);
    printf("Field get: i1: %d\n", i1);
    if (i != i1) {
        printf("Field get failed: %d != %d\n", i, i1);
        return -1;
    }

    if (!TLL_mynewbag_i2_EXISTS(bag_p)) {
        printf("Field exists failed: i2\n");
        return -1;
    }

    i = TLL_GET_mynewbag_i2(bag_p);
    printf("Field get: i2: %d\n", i2);
    if (i != i2) {
        printf("Field get failed: %d != %d\n", i, i2);
        return -1;
    }

    if (!TLL_mynewbag_s_EXISTS_TAG(bag_p, 2)) {
        printf("Field exists failed: s[2]\n");
        return -1;
    }

    s = TLL_GET_mynewbag_s_PTR_TAG(bag_p, 2);
    printf("Field get: s[2]: %s\n", s);
    if (strcmp(s, s2)) {
        printf("Field get failed: %s != %s\n", s, s2);
        return -1;
    }

    printf("st.key = %s, st.val = %s\n",
           TLL_GET_mynewbag_st_PTR(bag_p)->key_p,
           TLL_GET_mynewbag_st_PTR(bag_p)->value_p
          );
    if (strcmp(TLL_GET_mynewbag_st_PTR(bag_p)->key_p, st_key))
        return -1;
    if (strcmp(TLL_GET_mynewbag_st_PTR(bag_p)->value_p, st_val))
        return -1;

    printf("it.key = %d, it.val = %d\n",
           TLL_GET_mynewbag_it_PTR(bag_p)->key,
           TLL_GET_mynewbag_it_PTR(bag_p)->value
          );
    if (TLL_GET_mynewbag_it_PTR(bag_p)->key != it_key)
        return -1;
    if (TLL_GET_mynewbag_it_PTR(bag_p)->value != it_val)
        return -1;

    printf("sit.key = %s, sit.val = %d\n",
           TLL_GET_mynewbag_sit_PTR(bag_p)->key_p,
           TLL_GET_mynewbag_sit_PTR(bag_p)->value
          );
    if (strcmp(TLL_GET_mynewbag_sit_PTR(bag_p)->key_p,sit_key))
        return -1;
    if (TLL_GET_mynewbag_sit_PTR(bag_p)->value != sit_val)
        return -1;

    printf("s8t.key = %s, s8t.val = %d\n",
           TLL_GET_mynewbag_s8t_PTR(bag_p)->key_p,
           TLL_GET_mynewbag_s8t_PTR(bag_p)->value
          );
    if (strcmp(TLL_GET_mynewbag_s8t_PTR(bag_p)->key_p,s8t_key))
        return -1;
    if (TLL_GET_mynewbag_s8t_PTR(bag_p)->value != s8t_val)
        return -1;

    if (memcmp(TLL_GET_mynewbag_a8_PTR(bag_p)->arr_p,u8_arr.arr_p,u8_arr.len))
        return -1;

    if (memcmp(TLL_GET_mynewbag_a16_PTR(bag_p)->arr_p,u16_arr.arr_p,u16_arr.len * 2))
        return -1;

    if (memcmp(TLL_GET_mynewbag_a32_PTR(bag_p)->arr_p,u32_arr.arr_p,u32_arr.len * 4))
        return -1;

    if (TLL_GET_mynewbag_b(bag_p) != true)
        return -1;


    printf("Bag ok\n");
    check_ret = tull_checkBag_mynewbag(&packparam, &result_list_p);
    checkRet(check_ret, result_list_p);
    mpl_param_list_destroy(&result_list_p);
    if (check_ret != 0) {
        mpl_param_list_destroy(&result_list_p);
        return -1;
    }

    bag_p = packparam.value_p;
    bag_clone_p = mpl_param_list_clone(bag_p);

    printf("Mandatory parameter missing\n");
    TLL_REMOVE(bag_clone_p, my_enum);
    packparam.value_p = bag_clone_p;
    check_ret = tull_checkBag_mynewbag(&packparam, &result_list_p);
    if (check_ret == 0) {
        mpl_param_list_destroy(&result_list_p);
        return -1;
    }
    checkRet(check_ret, result_list_p);
    mpl_param_list_destroy(&result_list_p);
    mpl_param_list_destroy(&bag_clone_p);

    printf("Mandatory parameter doubled\n");
    bag_clone_p = mpl_param_list_clone(bag_p);
    TLL_ADD_ENUM_FROM_VALUE(&bag_clone_p,my_enum,val1);
    packparam.value_p = bag_clone_p;
    check_ret = tull_checkBag_mynewbag(&packparam, &result_list_p);
    if (check_ret == 0) {
        mpl_param_list_destroy(&result_list_p);
        return -1;
    }
    checkRet(check_ret, result_list_p);
    mpl_param_list_destroy(&result_list_p);
    mpl_param_list_destroy(&bag_clone_p);

    printf("Mandatory field missing\n");
    bag_clone_p = mpl_param_list_clone(bag_p);
    TLL_REMOVE_FIELD(bag_clone_p, mynewbag, i4);
    packparam.value_p = bag_clone_p;
    check_ret = tull_checkBag_mynewbag(&packparam, &result_list_p);
    if (check_ret == 0) {
        mpl_param_list_destroy(&result_list_p);
        return -1;
    }
    checkRet(check_ret, result_list_p);
    mpl_param_list_destroy(&result_list_p);
    mpl_param_list_destroy(&bag_clone_p);

    printf("Mandatory field doubled\n");
    bag_clone_p = mpl_param_list_clone(bag_p);
    TLL_ADD_mynewbag_i4(&bag_clone_p,i4);
    packparam.value_p = bag_clone_p;
    check_ret = tull_checkBag_mynewbag(&packparam, &result_list_p);
    if (check_ret == 0) {
        mpl_param_list_destroy(&result_list_p);
        return -1;
    }
    checkRet(check_ret, result_list_p);
    mpl_param_list_destroy(&result_list_p);
    mpl_param_list_destroy(&bag_clone_p);

    printf("Optional field doubled\n");
    bag_clone_p = mpl_param_list_clone(bag_p);
    TLL_ADD_mynewbag_b(&bag_clone_p,0);
    packparam.value_p = bag_clone_p;
    check_ret = tull_checkBag_mynewbag(&packparam, &result_list_p);
    if (check_ret == 0) {
        mpl_param_list_destroy(&result_list_p);
        return -1;
    }
    checkRet(check_ret, result_list_p);
    mpl_param_list_destroy(&result_list_p);
    mpl_param_list_destroy(&bag_clone_p);

    printf("Array element missing\n");
    bag_clone_p = mpl_param_list_clone(bag_p);
    TLL_REMOVE_FIELD_TAG(bag_clone_p,mynewbag,s,2);
    packparam.value_p = bag_clone_p;
    check_ret = tull_checkBag_mynewbag(&packparam, &result_list_p);
    if (check_ret == 0) {
        mpl_param_list_destroy(&result_list_p);
        return -1;
    }
    checkRet(check_ret, result_list_p);
    mpl_param_list_destroy(&result_list_p);
    mpl_param_list_destroy(&bag_clone_p);

    printf("Extra unknown element\n");
    bag_clone_p = mpl_param_list_clone(bag_p);
    TLL_ADD_ENUM_FROM_VALUE(&bag_clone_p,my_enum3,val4);
    packparam.value_p = bag_clone_p;
    check_ret = tull_checkBag_mynewbag(&packparam, &result_list_p);
    if (check_ret == 0) {
        mpl_param_list_destroy(&result_list_p);
        return -1;
    }
    checkRet(check_ret, result_list_p);
    mpl_param_list_destroy(&result_list_p);
    mpl_param_list_destroy(&bag_clone_p);

    printf("Unnumbered multiples\n");
    bag_clone_p = mpl_param_list_clone(bag_p);
    TLL_ADD_mynewbag_i5_TAG(&bag_clone_p,33,3);
    TLL_ADD_mynewbag_i5_TAG(&bag_clone_p,55,5);
    packparam.value_p = bag_clone_p;
    check_ret = tull_checkBag_mynewbag(&packparam, &result_list_p);
    if (check_ret != 0) {
        mpl_param_list_destroy(&result_list_p);
        return -1;
    }
    checkRet(check_ret, result_list_p);
    mpl_param_list_destroy(&result_list_p);
    mpl_param_list_destroy(&bag_clone_p);

    printf("Bag ok\n");
    bag_clone_p = mpl_param_list_clone(bag_p);
    packparam.value_p = bag_clone_p;
    check_ret = tull_checkBag_mynewbag(&packparam, &result_list_p);
    if (check_ret != 0) {
        mpl_param_list_destroy(&result_list_p);
        return -1;
    }
    checkRet(check_ret, result_list_p);
    mpl_param_list_destroy(&result_list_p);
    mpl_param_list_destroy(&bag_clone_p);

    mpl_param_list_destroy(&bag_p);
    bag_p = NULL;
    memset(&packparam, 0, sizeof(packparam));

    if (mpl_param_get_bag_field_name(test_paramid_mynewbag, TST_FIELD_INDEX(mynewbag, i1)) == NULL) {
        return -1;
    }

    TST_ADD_mynewbag_s_TAG(&bag_p,s1,1);
    TST_ADD_mynewbag_s_TAG(&bag_p,s2,1);
    TST_ADD_mynewbag_s_TAG(&bag_p,s3,1);
    result_list_p = mpl_param_list_find_field_all_tag(TLL_PARAM_ID(mynewbag),
                                                      TLL_FIELD_INDEX(mynewbag, s),
                                                      1,
                                                      bag_p);
    mpl_param_list_destroy(&bag_p);
    len = mpl_list_len(result_list_p);
    mpl_param_list_destroy(&result_list_p);
    if (len != 3) {
        return -1;
    }

    return 0;
}

static int tc_inheritance(void)
{
    int i;
    int child;

    /* Positive */
    if (!mpl_param_id_is_child(test_paramid_my_enum, test_paramid_my_enum2)) {
        printf("mpl_param_id_is_child(test_paramid_my_enum, test_paramid_my_enum2) failed\n");
        return -1;
    }

    if (!mpl_param_id_is_child(test_paramid_my_enum, test_paramid_my_enum3)) {
        printf("mpl_param_id_is_child(test_paramid_my_enum, test_paramid_my_enum3) failed\n");
        return -1;
    }

    if (!mpl_param_id_is_child(test_paramid_my_enum2, test_paramid_my_enum3)) {
        printf("mpl_param_id_is_child(test_paramid_my_enum2, test_paramid_my_enum3) failed\n");
        return -1;
    }

    if (!mpl_param_id_is_child(test_paramid_my_enum2, test_paramid_my_enum4)) {
        printf("mpl_param_id_is_child(test_paramid_my_enum2, test_paramid_my_enum4) failed\n");
        return -1;
    }

    if (!mpl_param_id_is_child(test_paramid_my_enum, tull_paramid_my_enum)) {
        printf("mpl_param_id_is_child(test_paramid_my_enum, tull_paramid_my_enum) failed\n");
        return -1;
    }

    if (!mpl_param_id_is_child(tull_paramid_myparent_list, tull_paramid_mychild_list1)) {
        printf("mpl_param_id_is_child(tull_paramid_myparent_list, tull_paramid_mychild_list1) failed\n");
        return -1;
    }

    if (!mpl_param_id_is_child(tull_paramid_myparent_list, tull_paramid_mychild_list2)) {
        printf("mpl_param_id_is_child(tull_paramid_myparent_list, tull_paramid_mychild_list2) failed\n");
        return -1;
    }

    /* Negative */

    if (mpl_param_id_is_child(tull_paramid_my_enum, test_paramid_my_enum)) {
        printf("mpl_param_id_is_child(tull_paramid_my_enum, test_paramid_my_enum) succeeded\n");
        return -1;
    }

    if (mpl_param_id_is_child(test_paramid_my_enum3, test_paramid_my_enum2)) {
        printf("mpl_param_id_is_child(test_paramid_my_enum2, test_paramid_my_enum3) suceeded\n");
        return -1;
    }

    if (mpl_param_id_is_child(test_paramid_my_enum, test_paramid_my_enum7)) {
        printf("mpl_param_id_is_child(test_paramid_my_enum, test_paramid_my_enum7) suceeded\n");
        return -1;
    }

    if (mpl_param_id_is_child(tull_paramid_mychild_list1, tull_paramid_mychild_list2)) {
        printf("mpl_param_id_is_child(tull_paramid_mychild_list1, tull_paramid_mychild_list2) succeeded\n");
        return -1;
    }

    if (mpl_param_id_is_child(tull_paramid_mychild_list1, tull_paramid_myparent_list)) {
        printf("mpl_param_id_is_child(tull_paramid_mychild_list1, tull_paramid_myparent_list) succeeded\n");
        return -1;
    }

    printf("mpl_param_id_is_child() (no field) OK\n");

    for (i = 0; i < 100; i++) {
        int index;
        child = mpl_param_get_child_id(TST_PARAM_ID(my_enum),i);
        if ((i > 0) && (child == MPL_PARAM_ID_UNDEFINED))
            break;
        index = mpl_param_get_child_index(TST_PARAM_ID(my_enum), child);
        printf("child id = 0x%08x, index = %d, %s.%s\n",
               child,
               index,
               mpl_param_id_get_prefix(child),
               mpl_param_id_get_string(child));
        if ((i > 0) && (index != i)) {
            printf("mpl_param_get_child_index() returned %d for index %d\n",
                   index,
                   i
                  );
            return -1;
        }
    }

    return 0;

}

static int tc_upgrade(void)
{
    mpl_param_element_t *par;
    mpl_test_old_myenum_t myenum = mpl_test_old_myenum_val1;
    char *mystring = "mystring";
    int myint = 100;

    if (mpl_test_old_init() < 0)
    {
        printf("mpl_test_old_init() failed\n");
        return -1;
    }

    if (strcmp(mpl_paramset_prefix(MPL_TEST_OLD_PARAM_SET_ID), "mpl_test_old") != 0)
    {
        printf("mpl_paramset_prefix() returned wrong prefix\n");
        return -1;
    }

    if (mpl_param_first_paramid(MPL_TEST_OLD_PARAM_SET_ID) != mpl_test_old_paramid_myfirst)
    {
        printf("mpl_param_first_paramid() returned wrong id %x\n", mpl_param_first_paramid(MPL_TEST_OLD_PARAM_SET_ID));
        return -1;
    }

    if (mpl_param_last_paramid(MPL_TEST_OLD_PARAM_SET_ID) != mpl_test_old_paramid_mylast)
    {
        printf("mpl_param_last_paramid() returned wrong id %x\n", mpl_param_last_paramid(MPL_TEST_OLD_PARAM_SET_ID));
        return -1;
    }

    par = mpl_param_element_create(mpl_test_old_paramid_myenum, &myenum);
    if (NULL == par)
    {
        printf("mpl_param_element_create(myenum) failed\n");
        return -1;
    }
    if (pack_unpack_one(par, true, MPL_TEST_OLD_PARAM_SET_ID) < 0)
        return -1;

    mpl_param_element_destroy(par);

    par = mpl_param_element_create(mpl_test_old_paramid_mystring, mystring);
    if (NULL == par)
    {
        printf("mpl_param_element_create(mystring) failed\n");
        return -1;
    }
    if (pack_unpack_one(par, true, MPL_TEST_OLD_PARAM_SET_ID) < 0)
        return -1;

    mpl_param_element_destroy(par);

    par = mpl_param_element_create(mpl_test_old_paramid_myint, &myint);
    if (NULL == par)
    {
        printf("mpl_param_element_create(myint) failed\n");
        return -1;
    }
    if (pack_unpack_one(par, true, MPL_TEST_OLD_PARAM_SET_ID) < 0)
        return -1;

    mpl_param_element_destroy(par);

    return 0;

}

void mpl_test(int test_start, int test_stop)
{
  int testcase;
  int result;

  if (0 != test_param_init())
  {
    printf("test_param_init() failed\n");
    return;
  }

  if (0 != tull_param_init())
  {
    printf("tull_param_init() failed\n");
    return;
  }

  for(testcase = test_start; testcase <= test_stop; testcase++)
  {
    switch(testcase)
    {
    case 1:
      result=tc_pack_unpack_req_mycommand();
      break;
    case 2:
      result=tc_pack_unpack_resp_mycommand();
      break;
    case 3:
      result=tc_pack_unpack_event_myevent();
      break;
    case 4:
      result=tc_pack_unpack_buffer_too_small();
      break;
    case 5:
      result=tc_unpack_with_unknown_parameter();
      break;
    case 6:
      result=tc_pack_unsupported_param_id();
      break;
    case 7:
      result=tc_pack_unpack_param_int();
      break;
    case 8:
      result=tc_pack_unpack_param_enum();
      break;
    case 9:
      result=tc_pack_unpack_param_uint8();
      break;
    case 10:
      result=tc_pack_unpack_param_uint32();
      break;
    case 11:
      result=tc_pack_unpack_param_bool();
      break;
    case 12:
      result=tc_pack_unpack_param_string();
      break;
    case 13:
      result=tc_pack_param_illegal();
      break;
    case 14:
      result=tc_unpack_param_illegal();
      break;
    case 15:
      result=tc_list();
      break;
    case 16:
      result=tc_param_get_default();
      break;
    case 17:
      result=tc_param_allow_get();
      break;
    case 18:
      result=tc_param_allow_set();
      break;
    case 19:
      result=tc_param_allow_config();
      break;
    case 20:
      result=tc_clone_param();
      break;
    case 21:
      result=tc_param_id_get_str();
      break;
    case 22:
      result=tc_param_value_get_str();
      break;
    case 23:
      result=tc_get_args_escape_1();
      break;
    case 24:
      result=tc_get_args_escape_2();
      break;
    case 25:
      result=tc_get_args_escape_3();
      break;
    case 26:
      result=tc_compare_param();
      break;
    case 27:
      result=tc_config_read();
      break;
    case 28:
      result=tc_element_create_negative();
      break;
    case 29:
      result=tc_config_negative();
      break;
    case 30:
      result=tc_init_mpl_param();
      break;
    case 31:
      result=tc_pack_unpack_param_default_param_set();
      break;
    case 32:
      result=tc_uint8_array();
      break;
    case 33:
      result=tc_string_tuple();
      break;
    case 34:
      result=tc_param_list_tuple_key_find();
      break;
    case 35:
      result=tc_pack_unpack_param_uint16();
      break;
    case 36:
      result=tc_pack_unpack_param_bool8();
      break;
    case 37:
      result=tc_pack_unpack_param_list();
      break;
    case 38:
      result=tc_list_append();
      break;
    case 39:
      result=tc_param_list_find_all();
      break;
    case 40:
      result=tc_uint16_array();
      break;
    case 41:
      result=tc_pack_unpack_param_wstring();
      break;
    case 42:
      result=tc_uint32_array();
      break;
    case 43:
      result=tc_param_clone_list();
      break;
    case 44:
      result=tc_pack_unpack_param_enum8();
      break;
    case 45:
      result=tc_config_read_bl();
      break;
    case 46:
      result=tc_pack_param_list_no_prefix();
      break;
    case 47:
      result=tc_unpack_param_list_default_param_set();
      break;
    case 48:
      result=tc_list_write_read_file();
      break;
    case 49:
      result=tc_write_read_config();
      break;
    case 50:
      result=tc_param_tag();
      break;
    case 51:
      result=tc_int_tuple();
      break;
    case 52:
      result=tc_strint_tuple();
      break;
    case 53:
      result=tc_get_type();
      break;
    case 54:
      result=tc_get_type_string();
      break;
    case 55:
      result=tc_pack_unpack_param_signed_enum8();
      break;
    case 56:
      result=tc_pack_unpack_param_signed_enum16();
      break;
    case 57:
      result=tc_pack_unpack_param_signed_enum32();
      break;
    case 58:
      result=tc_pack_unpack_param_enum16();
      break;
    case 59:
      result=tc_pack_unpack_param_enum32();
      break;
    case 60:
      result=tc_pack_unpack_param_sint8();
      break;
    case 61:
      result=tc_pack_unpack_param_sint16();
      break;
    case 62:
      result=tc_pack_unpack_param_sint32();
      break;
    case 63:
      result=tc_pack_unpack_param_uint16();
      break;
    case 64:
      result=tc_param_list_add();
      break;
    case 65:
      result=tc_pack_unpack_param_uint64();
      break;
    case 66:
      result=tc_list_write_read_file_negative();
      break;
    case 67:
      result=tc_errno();
      break;
    case 68:
        result= tc_threads();
      break;
    case 69:
      result=tc_struint8_tuple();
      break;
    case 70:
      result=tc_sizeof_zero();
      break;
    case 71:
      result=tc_write_read_blacklist();
      break;
    case 72:
      result=tc_misc_invalid();
      break;
    case 73:
      result=tc_param_type_bag();
      break;
    case 74:
      result=tc_pack_unpack_param_addr();
      break;
    case 75:
      result=tc_byte_stuffing();
      break;
    case 76:
      result=tc_byte_stuffing2();
      break;
    case 77:
      result=tc_byte_stuffing3();
      break;
    case 78:
      result=tc_byte_stuffing4();
      break;
    case 79:
      result=tc_byte_stuffing5();
      break;
    case 80:
      result=tc_enum_with_values();
      break;
    case 81:
      result=tc_integer_ranges();
      break;
    case 82:
      result=tc_bag_fields();
      break;
    case 83:
      result=tc_inheritance();
      break;
    case 84:
      result=tc_upgrade();
      break;
    case 85:
      result = tc_pack_unpack_param_sint64();
      break;
    case 86:
      result = tc_config_merge();
      break;
    case 87:
      result=tc_byte_stuffing6();
      break;
    default:
      printf("\n** unknown TC **\n");
      result=-1;
      break;
    }
    if(result==0)
      pass("Test %d\n",testcase);
    else
      fail("Test %d\n",testcase);
  }

  mpl_param_deinit();

  totals();

}

#ifndef MPL_OSE_TEST
int main(int argc, char **argv)
{
  int test_start;
  int test_stop;

  if(argc == 1)
  {
    test_start = mpl_test_min;
    test_stop = mpl_test_max;
  }
  else if(argc == 3)
  {
    test_start = atoi(argv[1]);
    test_stop = atoi(argv[2]);
  }
  else
  {
    test_start = test_stop = atoi(argv[1]);
  }

  mpl_test(test_start, test_stop);
  return 0;
}
#endif

