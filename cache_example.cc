#include <cache.h>
#include <iostream>
/*
to compile this code:
run: g++ cache_example.cc -lsjqcache -std=c++11 
*/

void print_ret(cache::access_ret ret)
{
    //if (ret ==cache::hit){ std::cout <<"hit"<<std::endl;}
    switch (ret)
    {
    case cache::hit:
        std::cout << "hit" << std::endl;
        break;
    case cache::miss:
        std::cout << "miss" << std::endl;
        break;
    case cache::resfail:
        std::cout << "resfail" << std::endl;
        break;
    default:
        std::cout << "error" << std::endl;
        break;
    }
}
int main()
{
    // 4: 4 way 
    // 1024: 1024 entries
    // lru: lru policy
    // 4 mshr entrys
    // 16 max merge per entry
    cache mcache(4, 1024, cache::lru, 4, 16);
    auto ret = mcache.access(0x1);
    print_ret(ret);
    ret=mcache.access(0x2);
    print_ret(ret);
    mcache.fill(0x1);
    ret=mcache.access(0x1);
    print_ret(ret);

}