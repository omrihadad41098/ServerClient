#include <stdlib.h>
#include "../include/event.h"
#include "../include/StompProtocol.h"
#include <iostream>
#include <string>
#include <map>
#include <vector>
using std::string;
using std::map;
using std::vector;

int main(int argc, char *argv[]) {
	// TODO: implement the STOMP client
	
	std::mutex mutex;
	StompProtocol protocol(mutex);
	protocol.keyboardTask();
	return 0;
}