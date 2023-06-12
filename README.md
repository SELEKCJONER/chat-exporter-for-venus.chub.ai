# chat exporter for venus.chub.ai
This is a c++ program that i made to export chats from venus.chub.ai
Chats must be made public so program can retrieve them through their api without authentication
If you know how to make authentication availibe let me know


my compile command

note i was installing some extra libraries but i think that only library that is not included with mingw64 is nlohmann/json.hpp

g++ -o program venusexporter.cpp -std=c++11 -lws2_32 -lwininet
