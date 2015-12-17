#include <sys/types.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

#include <iostream>

#include "CApplication.h"

using namespace std;

void halt(int nSignal)
{
    theApp.stop();
}

int main(int argc, char *argv[])
{
    struct sigaction sigkill_action;
    
    memset(&sigkill_action, 0, sizeof(struct sigaction));
    sigkill_action.sa_handler = halt;
    sigaction(SIGTERM, &sigkill_action, NULL);
    sigaction(SIGINT, &sigkill_action, NULL);
    sigaction(SIGQUIT, &sigkill_action, NULL);
    
    try {
	theApp.readConfig();
	// перход процесса в фоновый режим
	pid_t pid = 0;
#ifndef DEBUG	
	pid = fork();
#endif	
	if (pid == 0) {
	    theApp.run();
	}
	else if (pid > 0) {
	    return 0;
	}
	else {
	    cout << "Failed to create demon process" << endl;
	}
    }
    catch (const exception& e) {
	cout << e.what()  << endl;
    }
    return 0;
}
