/*
 * =====================================================================================
 *
 *       Filename:  Hash.h
 *
 *    Description:
 *
 *        Version:  1.0
 *        Created:  07/30/2014 03:01:53 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (),
 *        Company:
 *
 * =====================================================================================
 */
#ifndef HASH_H_
#define HASH_H_


#include <ext/hash_map>
#include <iostream>
#include <stdint.h>

using namespace __gnu_cxx;

namespace __gnu_cxx
{
struct my_hash {
    size_t operator() ( const uint64_t &x ) const {
        return x;
    }

};
struct my_compare {
    bool operator() ( const uint64_t &x, const uint64_t &y ) const {
        return x == y;
    }
};

}

class Hash
{
public:
    Hash();
    void HashInsert ( uint64_t, void * );
    void *HashSearch ( uint64_t ); //if new  ,return NULL
    void HashDelKey ( uint64_t );
    void HashTravel();
    void Clear();
    bool HashIsempty();
    ~Hash();
private:
    typedef hash_map<uint64_t , void *, my_hash, my_compare> map_t;
    map_t hashtable_;
};



#endif
