#ifndef __CAPPLICATION_H
#define __CAPPLICATION_H

#include <vector>
#include <string>

//#define DEBUG

#define CONFIG_FILE	"/etc/jslog/uploader.conf"
#define DEST_URL 	"https://jslog.me/log"

using namespace std;

class CLog;

class CApplication
{
private:
    vector<CLog*> _logs;
    bool _bStop;
public:
    void readConfig();
    void run();
    void stop();
    string prepareSystemInfoRequest(CLog*) const;
    string prepareErrorRequest(CLog*) const;
};

extern CApplication theApp;

#endif
