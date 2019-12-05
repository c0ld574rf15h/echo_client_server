exe: echo_server echo_client

echo_server: echo_server.o
	g++ --std=c++11 -o echo_server echo_server.o -lglog

echo_client: echo_client.o
	g++ --std=c++11 -o echo_client echo_client.o -lglog -lpthread

echo_server.o: echo_server.cpp
	g++ -c -o echo_server.o echo_server.cpp

echo_client.o: echo_client.cpp
	g++ -c -o echo_client.o echo_client.cpp

clean:
	rm -f echo_server echo_client *.o