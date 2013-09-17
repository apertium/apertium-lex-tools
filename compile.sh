g++ irstlm_ranker.cpp -I /usr/local/include/lttoolbox-3.2/ -I /usr/local/include/apertium-3.2/ -I /usr/include/libxml2/ -I /usr/local/irstlm/include -llttoolbox3 -lxml2 -lapertium3 -lz -lirstlm -lz -lpcre -L/usr/local/lib -L/usr/local/irstlm/lib/ -o irstlm-ranker

