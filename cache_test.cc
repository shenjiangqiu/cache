#define CATCH_CONFIG_MAIN // This tells Catch to provide a main() - only do this in one cpp file
#include "catch.hpp"
#include <cache.h>
TEST_CASE("mshr test")
{
    mshr m_mshr(4, 4);
    //REQUIRE(m_mshr.access(0x1)==mshr::ok);
    SECTION("merge fail test")
    {
        for (int i = 0x0; i < 0x4; i++)
        {
            REQUIRE(m_mshr.access(i) == mshr::ok);
        }
        REQUIRE(m_mshr.access(0x4) == mshr::merge_full);
    }
    SECTION("entry full test")
    {
        for (int i = 0x0; i < 0x4; i++)
        {
            REQUIRE(m_mshr.access(i << 6) == mshr::ok);
        }
        REQUIRE(m_mshr.access(0x4 << 6) == mshr::entry_full);
    }
    SECTION("full test")
    {
        //vvvv   region 1
        for (int i = 0; i < 4; i++)
        {
            m_mshr.access(i);
        }
        for (int i = 1; i < 4; i++)
        {
            m_mshr.access(i << 6);
        }
        REQUIRE(m_mshr.full(0) == true);
        for (int i = 1; i < 4; i++)
        {
            REQUIRE(m_mshr.full(i << 6) == false);
        }
        REQUIRE(m_mshr.full(4 << 6) == true);
        //^^^^   region 1

        SECTION("fill test") //now, 0 is merge full, and 1,2 3 are not full, entry is full
        {
            //region 2
            for (int i = 0; i < 4; i++)
            {
                m_mshr.fill(i << 6);
            }
            REQUIRE(cache_debugger::get_mshr_array(m_mshr).size() == 0);
            SECTION("re-push after fill") //now all empty
            {
                for (int i = 0x0; i < 0x4; i++)
                {
                    REQUIRE(m_mshr.access(i) == mshr::ok);
                }
                REQUIRE(m_mshr.access(0x4) == mshr::merge_full);
            }
        }
        SECTION("all full test after partion full")
        {
            for (int i = 1; i < 4; i++)
            {
                for (int j = 0; j < 0; j++)
                {
                    REQUIRE(m_mshr.access(i << 6) == mshr::ok);
                }
            }
        }
    }
}

TEST_CASE("cache function test")
{
    SECTION("LRU test")
    {
        cache mcache(4, 4, cache::lru, 4, 4);
        SECTION("first test")
        {
            REQUIRE(mcache.access(0x1) == cache::miss);
        }

        //init the cache:
        SECTION(" init accesses")
        {
            //fist we accest the full set:
            for (int i = 0; i < 4; i++)
            {
                REQUIRE(mcache.access(i << 6) == cache::miss);
            }
            for (int i = 0; i < 4; i++)
            {
                REQUIRE(cache_debugger::get_array(mcache)[0][0].get_status() == cache_entry::reserved);
            }
            // now every set have one entry reserved

            SECTION("resfail test")
            {
                REQUIRE(mcache.access(0) == cache::hit);
                REQUIRE(mcache.access(0) == cache::hit);
                REQUIRE(mcache.access(0) == cache::hit);
                REQUIRE(mcache.access(0) == cache::resfail);
                REQUIRE(mcache.access(4 << 6) == cache::resfail);
                REQUIRE(mcache.get_size() == std::make_pair(4, 4));

                SECTION("fill ")
                {
                    mcache.fill(0);
                    REQUIRE(cache_debugger::get_entry(mcache, 0, 0).get_status() == cache_entry::valid);
                }
                SECTION("access all entry in a set")
                { //at this point ,every set have only one entry; but not filled
                    for (int i = 0; i < 4; i++)
                    {
                        REQUIRE(cache_debugger::get_entry(mcache, i, 0).get_status() == cache_entry::reserved);
                        REQUIRE(cache_debugger::get_entry(mcache, i, 0).get_tag() != -1);
                        mcache.fill(i << 6);
                        REQUIRE(cache_debugger::get_entry(mcache, i, 0).get_status() == cache_entry::valid);
                        REQUIRE(cache_debugger::get_entry(mcache, i, 0).get_tag() != -1);
                        for (int j = 1; j < 4; j++)
                        {
                            REQUIRE(cache_debugger::get_entry(mcache, i, j).get_status() == cache_entry::invalid);
                            REQUIRE(cache_debugger::get_entry(mcache, i, j).get_tag() == -1);
                            REQUIRE(mcache.access((i + j * 4) << 6) == cache::miss);
                            mcache.fill((i + j * 4) << 6);
                        }
                        //random pick one entry
                        //REQUIRE(cache_debugger::get_entry(mcache, 2, 3).get_tag() == (2 + 4 * (3 - 3)));
                    }
                    for (int j = 0; j < 4; j++)
                    {
                        for (int k = 0; k < 4; k++)
                        {

                            REQUIRE(cache_debugger::get_entry(mcache, j, k).get_status() == cache_entry::valid);
                            REQUIRE(cache_debugger::get_entry(mcache, j, k).get_tag() == (j + 4 * (3 - k)));
                        }
                    }
                    SECTION("hit accesses")
                    {
                        for (int i = 0; i < 4; i++)
                        {
                            for (int j = 0; j < 4; j++)
                            {
                                REQUIRE(mcache.access(i + j * 4) == cache::hit);
                            }
                        }
                    }
                }
            }
        }
    }
    SECTION("fifo test")
    {
    }
}