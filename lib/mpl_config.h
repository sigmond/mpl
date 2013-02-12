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
 * File name: mpl_config.h
 *
 * Description: MPL config system API declarations
 *
 **************************************************************************/
#ifndef MPL_CONFIG_H
#define MPL_CONFIG_H

/**
 * @file mpl_config.h
 * @brief MPL configuration system
 */

/** @defgroup MPL_CONFIG MPL configuration system
 *  @ingroup MPL
 *  When an MPL parameter is defined it is given a set of properties. The
 *  configuration system uses two of these: "config" and "default".
 *  A configurable parameter must have the "config" option set, and should
 *  also normally have a default value.
 *
 *  A configuration is represented by the type mpl_config_t (which is basically
 *  an MPL parameter list). It can either be created "manually" in C using
 *  MPL functions or it can be loaded from file. The file must contain
 *  "MPL packed text format" parameters. A configuration can also be written
 *  to a file.
 *
 *  An additional feature is the possibility to define "blacklisted"
 *  parameters. This is to limit the parameters allowed in a config more than
 *  what can be expressed using the parameter set definition. A blacklist
 *  (mpl_blacklist_t) can either be created "manually" or loaded from file.
 *  A blacklist can also be written to a file.
 *
 */

/*****************************************************************************
 *
 * Include files
 *
 *****************************************************************************/

#include <stdio.h>
#include "mpl_list.h"
#include "mpl_param.h"

/*****************************************************************************
 *
 * Defines & Type definitions
 *
 *****************************************************************************/

/**
 * @ingroup MPL_CONFIG
 * mpl_config_t
 *
 * Configuration object.
 *
 */
typedef mpl_list_t *mpl_config_t;

/**
 * @ingroup MPL_CONFIG
 * mpl_config_log_func_t
 *
 * A function provided by the user that is supposed to behave
 * more or less like puts().
 *
 */
typedef void (*mpl_config_log_func_t)(const char *logstring_p);

/****************************************************************************
 *
 * Public Functions
 *
 ****************************************************************************/
/**
 * @ingroup MPL_CONFIG
 *
 * mpl_config init
 *
 * Initiate configuration object
 *
 * @param config_p        pointer to configuration
 *
 * @return              0 on success, -1 on error
 *
 **/
int mpl_config_init(mpl_config_t *config_p);


/**
 * @ingroup MPL_CONFIG
 *
 * mpl_config_read_config
 *
 * Read config from file given by path
 *
 * @param config_path         path to the config file.
 * @param config_p            pointer to configuration
 * @param param_set_id        default parameter set id
 *
 * @return              0 on success, -1 on error
 *
 **/
#define mpl_config_read_config(config_path, config_p, param_set_id)\
    mpl_config_read_config_bl(config_path, config_p, param_set_id, NULL)

/**
 * @ingroup MPL_CONFIG
 *
 * mpl_config_read_config_bl
 *
 * Read config from file given by path
 * filtered through a blacklist
 *
 * @param config_path         path to the config file
 * @param config_p            pointer to configuration
 * @param param_set_id        default parameter set id
 * @param blacklist           parameters present in the blacklist will
 *                            be skipped
 *
 * @return              0 on success, -1 on error
 *
 **/
int mpl_config_read_config_bl(char* config_path,
                              mpl_config_t *config_p,
                              int param_set_id,
                              mpl_blacklist_t blacklist);

/**
 * @ingroup MPL_CONFIG
 *
 * mpl_config_read_config_fp
 *
 * Read config from an opened file
 *
 * @param fp                  a config file pointer open for reading
 * @param config_p            pointer to configuration
 * @param param_set_id        default parameter set id
 *
 * @return              0 on success, -1 on error
 *
 **/
#define mpl_config_read_config_fp(fp, config_p, param_set_id) \
    mpl_config_read_config_bl_fp(fp, config_p, param_set_id, NULL)

/**
 * @ingroup MPL_CONFIG
 *
 * mpl_config_read_config_bl_fp
 *
 * Read config from an opened
 * file filtered through a blacklist
 *
 * @param fp                  a config file pointer open for reading
 * @param config_p            pointer to configuration
 * @param param_set_id        default parameter set id
 * @param blacklist           parameters present in the blacklist will
 *                            be skipped
 *
 * @return              0 on success, -1 on error
 *
 **/
int mpl_config_read_config_bl_fp(FILE *fp,
                              mpl_config_t *config_p,
                              int param_set_id,
                              mpl_blacklist_t blacklist);

/**
 * @ingroup MPL_CONFIG
 *
 * mpl_config_write_config_bl_fp
 *
 * Write config to an opened file
 *
 * @param fp                  a config file pointer open for writing
 * @param config              configuration
 * @param param_set_id        default parameter set
 * @param blacklist           blacklisted parameters
 *
 * @return              0 on success, -1 on error
 *
 **/
int mpl_config_write_config_bl_fp(FILE *fp,
                                  mpl_config_t config,
                                  int param_set_id,
                                  mpl_blacklist_t blacklist);

/**
 * @ingroup MPL_CONFIG
 *
 * mpl_config_write_config_bl_func
 *
 * Write config using function pointer
 * The provided function is supposed to work like puts().
 *
 * @param func                pointer to function (puts() lookalike)
 * @param config              configuration
 * @param default_param_set_id the default parameter set
 * @param blacklist           blacklisted parameters
 *
 * @return              0 on success, -1 on error
 *
 **/
int mpl_config_write_config_bl_func(mpl_config_log_func_t func,
                                    mpl_config_t config,
                                    int default_param_set_id,
                                    mpl_blacklist_t blacklist);


/**
 * @ingroup MPL_CONFIG
 *
 * mpl_config_read_blacklist
 *
 * Read blacklisted parameters from file given by path
 *
 *
 * @param blacklist_path      path to the blacklist file
 * @param blacklist_p         pointer to blacklist
 * @param param_set_id        default parameter set id
 *
 * @return              0 on success
 *                      -1 on open file error
 *                      -2 on error when reading parameter value in the file
 *
 * @note The file needs only to list the parameter
 *       names (no equal sign or value), one per line.
 **/
#define mpl_config_read_blacklist(blacklist_path, blacklist_p, param_set_id) \
    mpl_config_read_config_bl(blacklist_path, blacklist_p, param_set_id, NULL)

/**
 * @ingroup MPL_CONFIG
 *
 * mpl_config_read_blacklist_fp
 *
 * Read blacklisted parameters from opened file
 *
 *
 * @param fp                  file pointer open for reading
 * @param blacklist_p         pointer to blacklist
 * @param param_set_id        default parameter set id
 *
 * @return             0 on success
 *                     -1 on open file error
 *                     -2 on error when read parameter value in the file
 *
 * @note The file needs only to list the parameter
 *       names (no equal sign or value), one per line.
 **/
#define mpl_config_read_blacklist_fp(fp, blacklist_p, param_set_id) \
    mpl_config_read_config_bl_fp(fp, blacklist_p, param_set_id, NULL)

/**
 * @ingroup MPL_CONFIG
 *
 * mpl_config_write_blacklist_fp
 *
 * Write blacklist file to an opened file
 *
 * @param fp                  a config file pointer open for writing
 * @param blacklist           blacklist to write
 * @param param_set_id        default parameter set id
 *
 * @return              0 on success, -1 on error
 *
 **/
int mpl_config_write_blacklist_fp(FILE *fp,
                                  mpl_blacklist_t blacklist,
                                  int param_set_id);

/**
 * @ingroup MPL_CONFIG
 *
 * mpl_config_get_para
 *
 * Get a parameter element for the given parameter id
 *
 * For parameter that is not found in the list, a default
 * value will be used
 *
 * @param param_id            parameter id to find
 * @param config_p            pointer to configuration
 *
 * @return      pointer to parameter element or NULL when not found
 *
 **/
#define mpl_config_get_para(param_id, config_p) \
    mpl_config_get_para_bl(param_id, config_p, NULL)

/**
 * @ingroup MPL_CONFIG
 *
 * mpl_config_get_para_tag
 *
 * Get a parameter element for the given parameter
 * id and tag
 *
 * For parameter that is not found in the list, a default
 * value will be used
 *
 * @param param_id            parameter id to find
 * @param tag                 tag to look for
 * @param config_p            pointer to configuration
 *
 * @return      pointer to parameter element or NULL when not found
 *
 **/
#define mpl_config_get_para_tag(param_id, tag, config_p) \
    mpl_config_get_para_bl_tag(param_id, tag, config_p, NULL)

/**
 * @ingroup MPL_CONFIG
 *
 * mpl_config_get_para_bl
 *
 * Get a parameter element for the given parameter
 * id filtered through a blacklist
 *
 * For parameter that is not found in the list, a default
 * value will be used
 *
 * @param param_id            parameter id to find
 * @param config_p            pointer to configuration
 * @param blacklist           parameters present in the blacklist will be
 *                            skipped
 *
 * @return      pointer to parameter element or NULL when not found
 *
 **/
mpl_param_element_t* mpl_config_get_para_bl(mpl_param_element_id_t param_id,
                                            mpl_config_t *config_p,
                                            mpl_blacklist_t blacklist);

/**
 * @ingroup MPL_CONFIG
 *
 * mpl_config_get_para_bl_tag
 *
 * Get a parameter element for the given
 * parameter id and tag filtered through a
 * blacklist
 *
 * For parameter that is not found in the list, a
 * default value will be used
 *
 * @param param_id            parameter id to find
 * @param tag                 tag to look for
 * @param config_p            pointer to configuration
 * @param blacklist           parameters present in the blacklist will be
 *                            skipped
 *
 * @return      pointer to parameter element or NULL when not found
 *
 **/
mpl_param_element_t*
    mpl_config_get_para_bl_tag(mpl_param_element_id_t param_id,
                               int tag,
                               mpl_config_t *config_p,
                               mpl_blacklist_t blacklist);

/**
 * @ingroup MPL_CONFIG
 *
 * mpl_config_tuple_key_get_para
 *
 * Get a parameter element for the given
 * parameter id using a given key or
 * wildcard (fallback/default)
 *
 * For parameter that is not found in the list, a
 * default value will be used
 *
 * @param param_id            parameter id to find
 * @param key_p               key string to search for
 * @param wildcard_p          wildcard string to search for if key not found
 * @param config_p            pointer to configuration
 *
 * @return      pointer to parameter element or NULL when not found
 *
 **/
#define mpl_config_tuple_key_get_para(param_id, key_p, wildcard_p, config_p) \
    mpl_config_tuple_key_get_para_bl(param_id, \
                                     key_p, \
                                     wildcard_p, \
                                     config_p, \
                                     NULL)

/**
 * @ingroup MPL_CONFIG
 *
 * mpl_config_tuple_key_get_para_bl
 *
 * Get a parameter element for the given
 * parameter id using a given key or
 * wildcard (fallback/default) filtered
 * through a blacklist
 *
 * For parameter that is not found in the list, a default
 * value will be used
 *
 * @param param_id            parameter id to find
 * @param key_p               key string to search for
 * @param wildcard_p          wildcard string to search for if key not found
 * @param config_p            pointer to configuration
 * @param blacklist           parameters present in the blacklist will be
 *                            skipped
 *
 * @return      pointer to parameter element or NULL when not found
 *
 **/
mpl_param_element_t*
    mpl_config_tuple_key_get_para_bl(mpl_param_element_id_t param_id,
                                     char *key_p,
                                     char *wildcard_p,
                                     mpl_config_t *config_p,
                                     mpl_blacklist_t blacklist);

/**
 * @ingroup MPL_CONFIG
 *
 * mpl_config_reset
 *
 * Destroy the configuration object
 *
 * @param config_p            pointer to configuration
 *
 **/
void mpl_config_reset(mpl_config_t *config_p);


/**
 * @ingroup MPL_CONFIG
 *
 * mpl_config_merge
 *
 * Merge a configuration into another (overwriting existing elements)
 *
 * @param to_p            pointer to existing configuration
 * @param from            new configuration
 *
 * @return              0 on success, -1 on error
 **/
int mpl_config_merge(mpl_config_t *to_p, mpl_config_t from);


#endif
