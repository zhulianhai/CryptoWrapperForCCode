/***************************************************************************
*            lpconfig.c
*			 配置参数的 读取/保存
****************************************************************************/

#define MAX_LEN 1024

//#include "typedef.h"
#include "Common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>


#define lp_new0(type,n)	(type*)calloc(sizeof(type),n)

#include "LpConfig.h"

#ifdef HAVE_SQLITE3
#define SQLITE3_KEY "dww1982"
#define SQLITE3_KEY_LEN strlen(SQLITE3_KEY)
#include "sqlite3.h"
#endif // HAVE_SQLITE3

#define LISTNODE(_struct_)	\
struct _struct_ *_prev;\
struct _struct_ *_next;

typedef struct _ListNode {
    LISTNODE ( _ListNode )
} ListNode;

typedef void ( *ListNodeForEachFunc ) ( ListNode * );

ListNode *list_node_append ( ListNode *head, ListNode *elem )
{
    ListNode *e = head;
    while ( e->_next != NULL ) {
        e = e->_next;
    }
    e->_next = elem;
    elem->_prev = e;
    return head;
}

ListNode *list_node_remove ( ListNode *head, ListNode *elem )
{
    ListNode *before, *after;
    before = elem->_prev;
    after = elem->_next;
    if ( before != NULL ) {
        before->_next = after;
    }
    if ( after != NULL ) {
        after->_prev = before;
    }
    elem->_prev = NULL;
    elem->_next = NULL;
    if ( head == elem ) {
        return after;
    }
    return head;
}

void list_node_foreach ( ListNode *head, ListNodeForEachFunc func )
{
    for ( ; head != NULL; head = head->_next ) {
        func ( head );
    }
}


#define LIST_PREPEND(e1,e2) (  (e2)->_prev=NULL,(e2)->_next=(e1),(e1)->_prev=(e2),(e2) )
#define LIST_APPEND(head,elem) ((head)==0 ? (elem) : (list_node_append((ListNode*)(head),(ListNode*)(elem)), (head)) )
#define LIST_REMOVE(head,elem)

/* returns void */
#define LIST_FOREACH(head) list_node_foreach((ListNode*)head)

typedef struct _LpItem {
    char *key;
    char *value;
    char *old_value;
    bool_t modified;
} LpItem;

typedef struct _LpSection {
    char *name;
    MSList *items;
} LpSection;

struct _LpConfig {
    char *filename;
#ifdef HAVE_SQLITE3
    sqlite3 *db;
#else
    FILE *file;
#endif // HAVE_SQLITE3
    MSList *sections;
    int modified;
};

#ifdef HAVE_SQLITE3
void optimize_database ( sqlite3 *db )
{
    sqlite3_exec ( db, "PRAGMA synchronous = OFF", 0, 0, 0 ); //如果有定期备份的机制，而且少量数据丢失可接受，用OFF
    sqlite3_exec ( db, "PRAGMA cache_size = 8000", 0, 0, 0 ); //建议改为8000
    sqlite3_exec ( db, "PRAGMA case_sensitive_like=1", 0, 0, 0 ); //搜索中文字串
}

// TRUE: create new table   FALSE: table Exists
bool_t sqlite3_table_new ( sqlite3 *db )
{
    char szSQL[] = "CREATE TABLE [ua_config] ( [section] CHAR(128), [var_name] CHAR(256), [var_value] CHAR(256));";
    int res = sqlite3_exec ( db, szSQL, 0, 0, 0 );

    if ( res == SQLITE_OK ) {
        optimize_database ( db );
    }

    return ( res == SQLITE_OK ) ? TRUE : FALSE;
}

bool_t sqlite3_table_empty ( sqlite3 *db )
{
    char szSQL[] = "DELETE FROM  ua_config;";
    int res = sqlite3_exec ( db, szSQL, 0, 0, 0 );
    return ( res == SQLITE_OK ) ? TRUE : FALSE;
}
#endif
LpItem *lp_item_new ( const char *key, const char *value, bool_t changed )
{
    LpItem *item = lp_new0 ( LpItem, 1 );
    item->key = strdup ( key );
    item->value = strdup ( value );
    item->old_value = strdup ( value );
    item->modified = changed;
    return item;
}

LpSection *lp_section_new ( const char *name )
{
    LpSection *sec = lp_new0 ( LpSection, 1 );
    sec->name = strdup ( name );
    return sec;
}

bool_t lp_item_modified ( LpItem *item )
{
    if ( strcmp ( item->value, item->old_value ) || item->modified ) {
        return TRUE;
    } else {
        return FALSE;
    }
}


void lp_item_destroy ( void *pitem )
{
    LpItem *item = ( LpItem * ) pitem;
    free ( item->key );
    free ( item->value );
    free ( item->old_value );
    free ( item );
}

void lp_section_destroy ( LpSection *sec )
{
    free ( sec->name );
    ms_list_for_each ( sec->items, lp_item_destroy );
    ms_list_free ( sec->items );
    free ( sec );
}

void lp_section_add_item ( LpSection *sec, LpItem *item )
{
    sec->items = ms_list_append ( sec->items, ( void * ) item );
}

void lp_config_add_section ( LpConfig *lpconfig, LpSection *section )
{
    lpconfig->sections = ms_list_append ( lpconfig->sections, ( void * ) section );
}

void lp_config_remove_section ( LpConfig *lpconfig, LpSection *section )
{
#ifdef HAVE_SQLITE3
    char szSQL[128];
    sprintf ( szSQL, "DELETE FROM ua_config WHERE  section = '%s';", section->name );

    if ( !lpconfig->db ) {
        if ( sqlite3_open ( lpconfig->filename, &lpconfig->db ) == SQLITE_OK ) {
            sqlite3_key ( lpconfig->db, SQLITE3_KEY, SQLITE3_KEY_LEN );

            sqlite3_exec ( lpconfig->db, szSQL, 0, 0, 0 );
            ms_message ( "lp_config_remove_section: sqlite3_exec %s", szSQL );
            sqlite3_close ( lpconfig->db );
        }
        lpconfig->db = NULL;
    } else {
        sqlite3_exec ( lpconfig->db, szSQL, 0, 0, 0 );
        ms_message ( "lp_config_remove_section: sqlite3_exec %s", szSQL );
    }

#endif
    lpconfig->sections = ms_list_remove ( lpconfig->sections, ( void * ) section );
    lp_section_destroy ( section );
}

static bool_t is_first_char ( const char *start, const char *pos )
{
    const char *p;
    for ( p = start; p < pos; p++ ) {
        if ( *p != ' ' ) {
            return FALSE;
        }
    }
    return TRUE;
}

#ifdef HAVE_SQLITE3

/************************************************************************/
/*
第一步 查询 section 总条目
SELECT DISTINCT section FROM ua_config;
第二步根据每个 section 查询出 var_name, var_value
SELECT var_name, var_value FROM ua_config WHERE section = 'sound';*/
/************************************************************************/
void lp_config_parse ( LpConfig *lpconfig )
{
    int nrow = 0, ncolumn = 0;
    char **sectionResult; //二维数组存放结果
    int nIndex = 0;
    LpSection *cur = NULL;

    char szSQL[] = "SELECT DISTINCT section FROM ua_config;";
    if ( sqlite3_get_table ( lpconfig->db, szSQL , &sectionResult , &nrow , &ncolumn , 0 ) == SQLITE_OK ) {
        int index = ncolumn;
        int i, j;
        for ( i = 0; i < nrow; i++ ) {
            for ( j = 0; j < ncolumn; j++ ) {
                if ( strcmp ( sectionResult[j], "section" ) == 0 ) {
                    int itemnrow = 0, itemncolumn = 0;
                    char **itemResult; //二维数组存放结果
                    int itemIndex = 0;
                    char szSQLitem[128];

                    cur = lp_section_new ( sectionResult[index] );
                    lp_config_add_section ( lpconfig, cur );

                    sprintf ( szSQLitem, "SELECT var_name, var_value FROM ua_config WHERE section = '%s';", sectionResult[index] );

                    if ( sqlite3_get_table ( lpconfig->db, szSQLitem , &itemResult , &itemnrow , &itemncolumn , 0 ) == SQLITE_OK ) {
                        int idx = itemncolumn;
                        int k, l;
                        char *val_name;
                        char *val_value;
                        val_name = val_value = NULL;

                        for ( k = 0; k < itemnrow; k++ ) {
                            for ( l = 0; l < itemncolumn; l++ ) {
                                if ( strcmp ( itemResult[l], "var_name" ) == 0 ) {
                                    val_name = itemResult[idx];
                                }

                                if ( strcmp ( itemResult[l], "var_value" ) == 0 ) {
                                    val_value = itemResult[idx];
                                }

                                ++idx;
                            }
                            ms_message ( "lp_config_parse section: %s, var_name: %s, var_value: %s", cur->name, val_name, val_value );
                            lp_section_add_item ( cur, lp_item_new ( val_name, val_value, FALSE ) );

                        }

                        sqlite3_free_table ( itemResult );
                    }

                }
                ++index;
            }
        }

        sqlite3_free_table ( sectionResult );
    }


}

#else // else HAVE_SQLITE3
void lp_config_parse ( LpConfig *lpconfig )
{
    char tmp[MAX_LEN];
    LpSection *cur = NULL;
    if ( lpconfig->file == NULL ) {
        return;
    }
    while ( fgets ( tmp, MAX_LEN, lpconfig->file ) != NULL ) {
        char *pos1, *pos2;
        pos1 = strchr ( tmp, '[' );
        if ( pos1 != NULL && is_first_char ( tmp, pos1 ) ) {
            pos2 = strchr ( pos1, ']' );
            if ( pos2 != NULL ) {
                int nbs;
                char secname[MAX_LEN];
                secname[0] = '\0';
                /* found section */
                *pos2 = '\0';
                nbs = sscanf ( pos1 + 1, "%s", secname );
                if ( nbs == 1 ) {
                    if ( strlen ( secname ) > 0 ) {
                        cur = lp_section_new ( secname );
                        lp_config_add_section ( lpconfig, cur );
                    }
                } else {
                    //ms_warning("parse error!");
                }
            }
        } else {
            pos1 = strchr ( tmp, '=' );
            if ( pos1 != NULL ) {
                char key[MAX_LEN];
                key[0] = '\0';

                *pos1 = '\0';
                if ( sscanf ( tmp, "%s", key ) > 0 ) {

                    pos1++;
                    pos2 = strchr ( pos1, '\n' );
                    if ( pos2 == NULL ) {
                        pos2 = pos1 + strlen ( pos1 );
                    } else {
                        *pos2 = '\0'; /*replace the '\n' */
                        pos2--;
                    }
                    /* remove ending white spaces */
                    for ( ; pos2 > pos1 && *pos2 == ' '; pos2-- ) {
                        *pos2 = '\0';
                    }
                    if ( pos2 - pos1 >= 0 ) {
                        /* found a pair key,value */
                        if ( cur != NULL ) {
                            lp_section_add_item ( cur, lp_item_new ( key, pos1, FALSE ) );
                            /*printf("Found %s %s=%s\n",cur->name,key,pos1);*/
                        } else {
                            //ms_warning("found key,item but no sections");
                        }
                    }
                }
            }
        }
    }
}
#endif // HAVE_SQLITE3


int lp_config_read_file ( LpConfig *lpconfig, const char *filename )
{
    FILE *f = fopen ( filename, "r" );
    if ( f != NULL ) {
        lp_config_parse ( lpconfig );
        fclose ( f );
        return 0;
    }
    //ms_warning("Fail to open file %s",filename);
    return -1;
}


LpConfig *lp_config_new ( const char *filename )
{
    LpConfig *lpconfig = lp_new0 ( LpConfig, 1 );
    if ( filename != NULL ) {
        lpconfig->filename = strdup ( filename );
#ifdef HAVE_SQLITE3
        lpconfig->db = NULL;
        if ( sqlite3_open ( filename, &lpconfig->db ) == SQLITE_OK ) {
            sqlite3_key ( lpconfig->db, SQLITE3_KEY, SQLITE3_KEY_LEN );

            if ( !sqlite3_table_new ( lpconfig->db ) ) {
                ;
            }
            lp_config_parse ( lpconfig );

            sqlite3_close ( lpconfig->db );
        } else {
            //ms_warning("Can't open database: %s\n", sqlite3_errmsg(lpconfig->db));
        }
        lpconfig->db = NULL;
#else
#ifdef _MSC_VER
        lpconfig->file = fopen ( lpconfig->filename, "a+b" );
#else
        lpconfig->file = fopen ( lpconfig->filename, "rw" );
#endif // _MSC_VER
        if ( lpconfig->file != NULL ) {
            lp_config_parse ( lpconfig );
            fclose ( lpconfig->file );

            /* make existing configuration files non-group/world-accessible */
            //if (chmod(filename, S_IRUSR | S_IWUSR) == -1)
            //	ms_warning("unable to correct permissions on "
            //	  	  "configuration file: %s",
            //		   strerror(errno));
            lpconfig->file = NULL;
            lpconfig->modified = 0;
        }
#endif // HAVE_SQLITE3

    }
    return lpconfig;
}

void lp_item_set_value ( LpItem *item, const char *value )
{
    free ( item->value );
    item->value = strdup ( value );
    item->modified = lp_item_modified ( item );
}


void lp_config_destroy ( LpConfig *lpconfig )
{
    if ( lpconfig->filename != NULL ) {
        free ( lpconfig->filename );
    }
    ms_list_for_each ( lpconfig->sections, ( void ( * ) ( void * ) ) lp_section_destroy );
    ms_list_free ( lpconfig->sections );
    free ( lpconfig );
}

LpSection *lp_config_find_section ( LpConfig *lpconfig, const char *name )
{
    LpSection *sec;
    MSList *elem;
    /*printf("Looking for section %s\n",name);*/
    for ( elem = lpconfig->sections; elem != NULL; elem = ms_list_next ( elem ) ) {
        sec = ( LpSection * ) elem->data;
        if ( strcmp ( sec->name, name ) == 0 ) {
            /*printf("Section %s found\n",name);*/
            return sec;
        }
    }
    return NULL;
}

LpItem *lp_section_find_item ( LpSection *sec, const char *name )
{
    MSList *elem;
    LpItem *item;
    /*printf("Looking for item %s\n",name);*/
    for ( elem = sec->items; elem != NULL; elem = ms_list_next ( elem ) ) {
        item = ( LpItem * ) elem->data;
        if ( strcmp ( item->key, name ) == 0 ) {
            /*printf("Item %s found\n",name);*/
            return item;
        }
    }
    return NULL;
}

void lp_section_remove_item ( LpSection *sec, LpItem *item )
{
    sec->items = ms_list_remove ( sec->items, ( void * ) item );
    lp_item_destroy ( item );
}

const char *lp_config_get_string ( LpConfig *lpconfig, const char *section, const char *key, const char *default_string )
{
    LpSection *sec;
    LpItem *item;
    sec = lp_config_find_section ( lpconfig, section );
    if ( sec != NULL ) {
        item = lp_section_find_item ( sec, key );
        if ( item != NULL ) {
            return item->value;
        }
    }
    return default_string;
}

int lp_config_get_int ( LpConfig *lpconfig, const char *section, const char *key, int default_value )
{
    const char *str = lp_config_get_string ( lpconfig, section, key, NULL );
    if ( str != NULL ) {
        return atoi ( str );
    } else {
        return default_value;
    }
}

float lp_config_get_float ( LpConfig *lpconfig, const char *section, const char *key, float default_value )
{
    const char *str = lp_config_get_string ( lpconfig, section, key, NULL );
    float ret = default_value;
    if ( str == NULL ) {
        return default_value;
    }
    sscanf ( str, "%f", &ret );
    return ret;
}

void lp_config_set_string ( LpConfig *lpconfig, const char *section, const char *key, const char *value )
{
    LpItem *item;
    LpSection *sec = lp_config_find_section ( lpconfig, section );
    if ( sec != NULL ) {
        item = lp_section_find_item ( sec, key );
        if ( item != NULL ) {
            if ( value != NULL ) {
                lp_item_set_value ( item, value );
            } else {
                lp_section_remove_item ( sec, item );
            }
        } else {
            if ( value != NULL ) {
                lp_section_add_item ( sec, lp_item_new ( key, value, TRUE ) );
            }
        }
    } else if ( value != NULL ) {
        sec = lp_section_new ( section );
        lp_config_add_section ( lpconfig, sec );
        lp_section_add_item ( sec, lp_item_new ( key, value, TRUE ) );
    }
    lpconfig->modified++;
}
/*#define snprintf _snprintf*/
void lp_config_set_int ( LpConfig *lpconfig, const char *section, const char *key, int value )
{
    char tmp[30];
    snprintf ( tmp, 30, "%i", value );
    lp_config_set_string ( lpconfig, section, key, tmp );
    lpconfig->modified++;
}

//此处替换成sqlite3 insert
#ifdef HAVE_SQLITE3
void lp_item_write ( LpItem *item, sqlite3 *db, const char *section )
{
    char szSQL[128];
    if ( lp_item_modified ( item ) ) {
        sprintf ( szSQL,
                  "DELETE FROM ua_config WHERE  section = '%s' AND var_name = '%s' AND var_value = '%s';",
                  section, item->key, item->old_value );

        if ( sqlite3_exec ( db, szSQL, 0, 0, 0 ) != SQLITE_OK ) {
            ms_warning ( "lp_item_write,write section to database erro: %s\n", sqlite3_errmsg ( db ) );
        }

        sprintf ( szSQL,
                  "INSERT INTO [ua_config] values('%s', '%s', '%s');",
                  section, item->key, item->value );

        ms_message ( "lp_item_write section: %s", szSQL );

        if ( sqlite3_exec ( db, szSQL, 0, 0, 0 ) != SQLITE_OK ) {
            ms_warning ( "lp_item_write,write section to database erro: %s\n", sqlite3_errmsg ( db ) );
        }
    } else {
        ms_message ( "lp_item_write section: %s, var_name: %s, var_value: %s not changed, skip !", section, item->key, item->value );
    }
}
#else
void lp_item_write ( LpItem *item, FILE *file )
{
    fprintf ( file, "%s=%s\n", item->key, item->value );
}
#endif // HAVE_SQLITE3

//此处替换成更新和写入sqlite3设置
#ifdef HAVE_SQLITE3
void ms_list_for_each3 ( const MSList *list, void ( *func ) ( void *, void *, void * ), void *user_data, void *user_data2 )
{
    for ( ; list != NULL; list = list->next ) {
        func ( list->data, user_data, user_data2 );
    }
}
void lp_section_write ( LpSection *sec, sqlite3 *db )
{
    ms_list_for_each3 ( sec->items, ( void ( * ) ( void *, void *, void * ) ) lp_item_write, ( void * ) db, ( void * ) sec->name );
}
#else
void lp_section_write ( LpSection *sec, FILE *file )
{
    fprintf ( file, "[%s]\n", sec->name );
    ms_list_for_each2 ( sec->items, ( void ( * ) ( void *, void * ) ) lp_item_write, ( void * ) file );
    fprintf ( file, "\n" );
}
#endif // HAVE_SQLITE

int lp_config_sync ( LpConfig *lpconfig )
{
#ifdef HAVE_SQLITE3
    lpconfig->db = NULL;
    if ( lpconfig->filename == NULL ) {
        return -1;
    }

    if ( sqlite3_open ( lpconfig->filename, &lpconfig->db ) == SQLITE_OK ) {
        sqlite3_key ( lpconfig->db, SQLITE3_KEY, SQLITE3_KEY_LEN );

        if ( !sqlite3_table_new ( lpconfig->db ) ) {
            //sqlite3_table_empty(lpconfig->db);
            //ms_warning("Database update: %s\n", sqlite3_errmsg(lpconfig->db));
        }

        ms_list_for_each2 ( lpconfig->sections, ( void ( * ) ( void *, void * ) ) lp_section_write, ( void * ) lpconfig->db );
        sqlite3_close ( lpconfig->db );
    } else {
        ms_warning ( "Can't open database: %s\n", sqlite3_errmsg ( lpconfig->db ) );
    }
    lpconfig->db = NULL;
#else
    FILE *file;
    if ( lpconfig->filename == NULL ) {
        return -1;
    }
#ifndef WIN32
    /* don't create group/world-accessible files */
    ( void ) umask ( S_IRWXG | S_IRWXO );
#endif

#ifdef _MSC_VER
    file = fopen ( lpconfig->filename, "w+b" );
#else
    file = fopen ( lpconfig->filename, "w" );
#endif // _MSC_VER

    if ( file == NULL ) {
//		ms_warning("Could not write %s !",lpconfig->filename);
        return -1;
    }
    ms_list_for_each2 ( lpconfig->sections, ( void ( * ) ( void *, void * ) ) lp_section_write, ( void * ) file );
    fclose ( file );
#endif // HAVE_SQLITE3
    lpconfig->modified = 0;
    return 0;
}

int lp_config_has_section ( LpConfig *lpconfig, const char *section )
{
    if ( lp_config_find_section ( lpconfig, section ) != NULL ) {
        return 1;
    }
    return 0;
}

void lp_config_clean_section ( LpConfig *lpconfig, const char *section )
{
    LpSection *sec = lp_config_find_section ( lpconfig, section );
    if ( sec != NULL ) {
        lp_config_remove_section ( lpconfig, sec );
    }
    lpconfig->modified++;
}

int lp_config_needs_commit ( const LpConfig *lpconfig )
{
    return lpconfig->modified > 0;
}
