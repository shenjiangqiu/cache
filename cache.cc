#include "cache.h"

cache::cache(int way, int set, rep_policy p,
             int mshr_num, int mshr_maxmerge) : num_way(way),
                                                num_set(set),
                                                policy(p),
                                                tag_array(t_array(set, t_set(way))),
                                                m_mshr(mshr_num, mshr_maxmerge)
{
}
cache::access_ret cache::access(int addr)
{
    auto blockAddr = addr >> 6;
    int set = blockAddr % num_set;
    int tag = blockAddr;
    if (policy == lru)
    {
        auto &set_entry = tag_array[set];
        auto it = set_entry.begin();
        for (auto &entry : set_entry)
        {
            if (entry.get_status() == cache_entry::invalid)
            {
                //to the first place,and push to mshr
                if (m_mshr.access(addr) != mshr::ok)
                {
                    return resfail;
                }
                entry.set_entry(blockAddr, cache_entry::reserved);
                if (it != set_entry.begin())
                {
                    //move to begin;
                    auto temp = entry;
                    while (it != set_entry.begin())
                    {
                        it->set_entry(std::prev(it)->get_tag(), std::prev(it)->get_status());
                        it--;
                    }
                    it->set_entry(temp.get_tag(), temp.get_status());
                }
                return miss;
            }
            if (entry.get_tag() == tag)
            {
                if (entry.get_status() == cache_entry::valid)
                {
                    //to the first place;
                    if (it != set_entry.begin())
                    {
                        //move to begin;
                        auto temp = entry;
                        while (it != set_entry.begin())
                        {
                            it->set_entry(std::prev(it)->get_tag(), std::prev(it)->get_status());
                            it--;
                        }
                        it->set_entry(temp.get_tag(), temp.get_status());
                    }
                    return hit;
                }
                else //reserved
                {
                    if (m_mshr.access(addr) != mshr::ok)
                    {
                        return resfail;
                    }

                    if (it != set_entry.begin())
                    {
                        //move to begin;
                        auto temp = entry;
                        while (it != set_entry.begin())
                        {
                            it->set_entry(std::prev(it)->get_tag(), std::prev(it)->get_status());
                            it--;
                        }
                        it->set_entry(temp.get_tag(), temp.get_status());
                    }

                    //to the first place; and push to mshr
                    return miss;
                }
            }
            it++;
        }
        if (m_mshr.access(addr) != mshr::ok)
        {
            return resfail;
        }
        set_entry.pop_back();

        cache_entry entry;
        entry.set_entry(addr >> 6, cache_entry::reserved);
        set_entry.insert(set_entry.begin(), entry);
        return miss;
    }
    else
    {
        auto &set_entry = tag_array[set];
        auto it = set_entry.begin();
        for (auto &entry : set_entry)
        {
            if (entry.get_status() == cache_entry::invalid)
            {
                //to the first place,and push to mshr
                if (m_mshr.access(addr) != mshr::ok)
                {
                    return resfail;
                }
                entry.set_entry(blockAddr, cache_entry::reserved);

                return miss;
            }
            if (entry.get_tag() == tag)
            {
                if (entry.get_status() == cache_entry::valid)
                {
                    return hit;
                }
                else
                {
                    if (m_mshr.access(addr) != mshr::ok)
                    {
                        return resfail;
                    }

                    //to the first place; and push to mshr
                    return hit;
                }
            }
            it++;
        }
        //not found valid and tag
        if (m_mshr.access(addr) != mshr::ok)
        {
            return resfail;
        }
        cache_entry entry;
        entry.set_entry(addr >> 6, cache_entry::reserved);
        set_entry.pop_back();
        set_entry.insert(set_entry.begin(), entry);
        return miss;
    }
}

//start mshr

mshr::mshr_ret mshr::access(int addr)
{
    auto blockAddr = addr >> 6;//bug
    if (array.find(blockAddr) != array.end())
    {
        if (array[blockAddr].size() >= max_merge)
        {
            return merge_full;
        }
        array[blockAddr].push_back(addr);
        return ok;
    }
    else
    {
        if (array.size() >= num_entry)
        {
            return entry_full;
        }
        array.insert(std::make_pair(blockAddr, std::vector<int>()));
        array[blockAddr].push_back(addr);
        return ok;
    }
}