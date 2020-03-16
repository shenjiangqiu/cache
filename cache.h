#ifndef CACHE_H
#define CACHE_H
#include <vector>
#include <tuple>
#include <memory>
#include <cassert>
#include <map>
#include<iostream>
class cache_entry
{
public:
    enum cache_entry_status
    {
        invalid,
        valid,
        reserved
    };
    cache_entry() : m_status(invalid), m_tag(-1)
    {
    }
    void set_entry(int tag, cache_entry_status status)
    {
        m_tag = tag;
        m_status = status;
    }
    cache_entry_status get_status() { return m_status; }
    int get_tag() { return m_tag; }

private:
    cache_entry_status m_status;
    int m_tag;
};
class mshr
{
    public:
    using t_merge = std::vector<int>;
    using t_array = std::map<int, t_merge>;

public:
    enum mshr_ret
    {
        ok,
        entry_full,
        merge_full
    };
    mshr(int entry = 0, int max_merge = 0) : num_entry(entry),
                                             max_merge(max_merge){


                                             };
    mshr_ret access(int addr);
    void fill(int addr)
    {
        array.erase(addr>>6);
    }
    bool full(int addr){
        if(array.find(addr>>6)!=array.end()){
            return array[addr>>6].size()>=max_merge;//bug <
        }else{
            return array.size()>=num_entry;
        }
    }

private:
    int num_entry;
    int max_merge;
    t_array array;
    friend class cache_debugger;
};
class cache
{
    using t_set = std::vector<cache_entry>;

    using t_array = std::vector<t_set>;

public:
    enum rep_policy
    {
        lru,
        fifo
    };
    enum access_ret
    {
        hit,
        miss,
        resfail
    };

    cache(int way = 0, int set = 0, rep_policy p = lru, int mshr_num = 16, int mshr_maxmerge = 32);
    std::pair<int, int> get_size() { return std::make_pair(num_set, num_way); }
    virtual ~cache(){}
    access_ret access(int addr);
    void fill(int addr)
    {
        auto blockAddr = addr >> 6;
        int set = blockAddr % num_set;
        int tag = blockAddr;
        auto &set_entry = tag_array[set];
        for (auto &entry : set_entry)
        {
            if (entry.get_tag() == tag)
            {
                assert(entry.get_status() == cache_entry::reserved);
                entry.set_entry(tag, cache_entry::valid);
                m_mshr.fill(addr);
            }
        }
    }

private:
    mshr m_mshr;
    t_array tag_array;
    unsigned num_way;
    unsigned num_set;
    unsigned policy;


    friend class cache_debugger;
};

class cache_debugger
{
public:
    static cache_entry get_entry(cache &tcache, int set, int way)
    {
        //std::cout<<"get entry:"<<set<<","<<way<<std::endl;
        return tcache.tag_array[set][way];
    }
    static cache::t_array &get_array(cache &tcache)
    {
        return tcache.tag_array;
    }
    static mshr::t_array get_mshr_array(cache & tcache){
        return tcache.m_mshr.array;
    }
    static mshr::t_array get_mshr_array(mshr & m_mshr){
        return m_mshr.array;
    }
};

#endif