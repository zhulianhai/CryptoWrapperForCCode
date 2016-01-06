#ifndef COMMON__H
#define COMMON__H

#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif


#define ms_list_next(elem) ((elem)->next)

    struct _MSList {
        struct _MSList *next;
        struct _MSList *prev;
        void *data;
    };
    typedef struct _MSList MSList;

    MSList * ms_list_append ( MSList *elem, void * data );
    MSList * ms_list_prepend ( MSList *elem, void * data );
    MSList * ms_list_free ( MSList *elem );
    MSList * ms_list_concat ( MSList *first, MSList *second );
    MSList * ms_list_remove ( MSList *first, void *data );
    int ms_list_size ( const MSList *first );
    void ms_list_for_each ( const MSList *list, void ( *func ) ( void * ) );
    void ms_list_for_each2 ( const MSList *list, void ( *func ) ( void *, void * ), void *user_data );
    MSList *ms_list_remove_link ( MSList *list, MSList *elem );
    MSList *ms_list_find ( MSList *list, void *data );
    MSList *ms_list_find_custom ( MSList *list, int ( *compare_func ) ( const void *, const void* ), void *user_data );
    void * ms_list_nth_data ( const MSList *list, int index );
    int ms_list_position ( const MSList *list, MSList *elem );
    int ms_list_index ( const MSList *list, void *data );
    MSList *ms_list_insert_sorted ( MSList *list, void *data, int ( *compare_func ) ( const void *, const void* ) );
    MSList *ms_list_insert ( MSList *list, MSList *before, void *data );
    MSList *ms_list_copy ( const MSList *list );



#ifdef __cplusplus
}
#endif


#endif
