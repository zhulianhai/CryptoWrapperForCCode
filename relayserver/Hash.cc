/*
 * =====================================================================================
 *
 *       Filename:  Hash.cc
 *
 *    Description:
 *
 *        Version:  1.0
 *        Created:  07/30/2014 03:09:37 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (),
 *        Company:
 *
 * =====================================================================================
 */

#include <syslog.h>
#include <stdio.h>
#include "Hash.h"
#include <iostream>


Hash::Hash()
{
    hashtable_.clear();
}
Hash::~Hash()
{
}

void Hash::HashInsert ( uint64_t addr, void *buffer )
{
    hashtable_[addr] = buffer;
}

void *Hash::HashSearch ( uint64_t search_key )
{
    map_t::iterator it = hashtable_.find ( search_key );
    if ( it != hashtable_.end() ) {
        return it->second;
    }

    return NULL;
}

void Hash::HashDelKey ( uint64_t delkey )
{
    hashtable_.erase ( delkey );
}

void Hash::Clear()
{
    hashtable_.clear();
}

void Hash::HashTravel()
{
    map_t::iterator it = hashtable_.begin();
    while ( it != hashtable_.end() ) {
        std::cout << it->first << ":" << it->second << std::endl;
        it++;
    }

}

bool Hash::HashIsempty()
{
    return hashtable_.empty();
}


