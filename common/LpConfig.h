/***************************************************************************
 *            lpconfig.h
 *
 *  Thu Mar 10 15:02:49 2011
 *  Copyright  2011  MingWeiYu
 *  Email yumigwei@126.com
 ****************************************************************************/

#ifndef LPCONFIG_H
#define LPCONFIG_H

#ifdef LPCONFIG_EXPORTS
#define LPCONFIG_API __declspec(dllexport)
#else
#ifdef LPCONFIG_IMPORTS
#define LPCONFIG_API __declspec(dllimport)
#else
#define LPCONFIG_API extern
#endif
#endif

/**
 * The LpConfig object is used to manipulate a configuration file.
 *
 * @ingroup misc
 * The format of the configuration file is a .ini like format:
 * - sections are defined in []
 * - each section contains a sequence of key=value pairs.
 *
 * Example:
 * @code
 * [sound]
 * echocanceler=1
 * playback_dev=ALSA: Default device
 *
 * [video]
 * enabled=1
 * @endcode
**/
typedef struct _LpConfig LpConfig;

typedef unsigned char bool_t;


#ifndef FALSE
#define FALSE               0
#endif

#ifndef TRUE
#define TRUE                1
#endif



#ifdef __cplusplus
extern "C" {
#endif

    LPCONFIG_API LpConfig * lp_config_new ( const char *filename );
    LPCONFIG_API int lp_config_read_file ( LpConfig *lpconfig, const char *filename );
    /**
     * Retrieves a configuration item as a string, given its section, key, and default value.
     *
     * @ingroup misc
     * The default value string is returned if the config item isn't found.
    **/
    LPCONFIG_API const char *lp_config_get_string ( LpConfig *lpconfig, const char *section, const char *key, const char *default_string );
    LPCONFIG_API int lp_config_read_file ( LpConfig *lpconfig, const char *filename );
    /**
     * Retrieves a configuration item as an integer, given its section, key, and default value.
     *
     * @ingroup misc
     * The default integer value is returned if the config item isn't found.
    **/
    LPCONFIG_API int lp_config_get_int ( LpConfig *lpconfig,const char *section, const char *key, int default_value );
    LPCONFIG_API int lp_config_read_file ( LpConfig *lpconfig, const char *filename );
    /**
     * Retrieves a configuration item as a float, given its section, key, and default value.
     *
     * @ingroup misc
     * The default float value is returned if the config item isn't found.
    **/
    LPCONFIG_API float lp_config_get_float ( LpConfig *lpconfig,const char *section, const char *key, float default_value );
    /**
     * Sets a string config item
     *
     * @ingroup misc
    **/
    LPCONFIG_API void lp_config_set_string ( LpConfig *lpconfig,const char *section, const char *key, const char *value );
    /**
     * Sets an integer config item
     *
     * @ingroup misc
    **/
    LPCONFIG_API void lp_config_set_int ( LpConfig *lpconfig,const char *section, const char *key, int value );
    /**
     * Writes the config file to disk.
     *
     * @ingroup misc
    **/
    LPCONFIG_API int lp_config_sync ( LpConfig *lpconfig );
    /**
     * Returns 1 if a given section is present in the configuration.
     *
     * @ingroup misc
    **/
    LPCONFIG_API int lp_config_has_section ( LpConfig *lpconfig, const char *section );
    /**
     * Removes every pair of key,value in a section and remove the section.
     *
     * @ingroup misc
    **/
    LPCONFIG_API void lp_config_clean_section ( LpConfig *lpconfig, const char *section );
    /*tells whether uncommited (with lp_config_sync()) modifications exist*/
    LPCONFIG_API int lp_config_needs_commit ( const LpConfig *lpconfig );
    LPCONFIG_API void lp_config_destroy ( LpConfig *cfg );

#ifdef __cplusplus
}
#endif

#endif
