CPPFLAGS := -fPIC -O3 -Wall -g
libsjqcache.so:cache.o
	g++ --shared $^ -o $@
test:test.out
cache_test.o:cache_test.cc  cache.h 
	g++ $(CPPFLAGS) -c -o $@ $<; 
cache.o: cache.cc cache.h
	g++ $(CPPFLAGS) -c -o $@ $<; 
test.out:cache_test.o
	g++ $^ -o $@ -lsjqcache
.PHONY:clean
clean:
	rm -rf *.o *.out

install:
	cp cache.h /usr/include
	cp libsjqcache.so /usr/lib
