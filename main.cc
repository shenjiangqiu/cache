#include"cache.h"
#include<iostream>
int main(){
    cache m_cache(16,16,cache::lru,16,32);
    auto ret=m_cache.access(0x1);
    std::cout<<ret<<std::endl;
}