Build instructions:
Go to build dir, and run:
cmake -DBOOST_ROOT:STRING=<your boost root dir> -DBOOST_VER:STRING=1.60.0 ../ && make

Boost library 1.60.0 should be built with below option:
./b2 runtime-link=static variant=release link=static --with-system --with-random --with-date_time --with-regex --with-thread --with-filesystem --with-chrono --with-atomic

