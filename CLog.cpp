#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>

#include <iostream>
#include <fstream>

#include "CLog.h"

CLog* CLog::parse(const YAML::Node &node)
{
    string key;
    string filename;
    
    try {
	node["key"] >> key;
        node["filename"] >> filename;
        return new CLog(filename, key);
    }
    catch (...) {
        return NULL;
    }
}

CLog::CLog(const string &filename, const string &key)
{
    _filename = filename;
    _key = key;
    _session_guid = "";
    _last_ctime = 0;
    _last_mtime = 0;
    _size = 0;
    _status = 0;
    _last_pos = 0;
    _moreData = false;
}

bool CLog::check()
{
    struct stat statBuff;

    _status = 0;

    // при получении данных о файле произошла ошибка или файл не кашерный
    if (stat(_filename.c_str(), &statBuff) < 0 || !S_ISOK(statBuff.st_mode)) {
	// считаем, что файла просто нет
	_last_ctime = 0;
	return false;
    }
    // кашерный файл создан
    else if (_last_ctime == 0) {
	_last_ctime = statBuff.st_ctime;
	_size = statBuff.st_size;
	_status |= 0x1;
	_last_pos = _size;
    }

    // проверка признака модификации файла
    if (statBuff.st_mtime != _last_mtime) {
	_last_mtime = statBuff.st_mtime;
	
	// проверка изменения размера файла
	if (statBuff.st_size > _size) {
	    _status |= 0x2;
	    _size = statBuff.st_size;
	}
    }
    else if (_moreData) {
	_status |= 0x4;
    }
    
    return _status != 0;
}

string CLog::getLastRecord() const
{
    string result = "";
    ifstream in(_filename.c_str());
    
    if (in.is_open()) {
	in.seekg(0, std::ios_base::end);
	char ch = ' ';
	while(ch != '\n') {
	    in.seekg(-2, std::ios_base::cur);
	    if ((int)in.tellg() <= 0) {
		in.seekg(0);
		break;
	    }
	    in.get(ch);
	}
	getline(in, result);
	in.close();
    }
    
    return result;
}

int searchNewLine(ifstream &in, int dir)
{
    char ch;
    in.clear();
    while((ch = in.get()) != '\n') {
	if (dir == backforward) {
	    in.seekg(-2, std::ios_base::cur);
	    
	    if (in.tellg() <= 0) {
		in.clear();
		return 0;
    	    }
    	}
        else if (dir == forward) {
    	    if (in.eof()) {
    		in.clear();
    		return -1;
    	    }
    	}
    }
    in.clear();
    return (int)in.tellg();
}

string getData(ifstream &in, int start_pos, int end_pos)
{
    if (end_pos <= start_pos)
	throw runtime_error("Nothing to read");

    int needToRead = end_pos - start_pos;
    
    if (needToRead > 254)
	throw runtime_error("Data line too long: max 254 chars");

    char buff[255];
    
    in.seekg(start_pos);
    int readed = in.readsome(buff, needToRead);
    buff[readed - 1] = '\x0';
    
    return string(buff);
}

string CLog::getLastRecord2()
{
    ifstream in(_filename.c_str());
    
    if (in.is_open()) {
	in.seekg(_last_pos, std::ios_base::beg);
	int start_pos = searchNewLine(in, backforward);
	in.seekg(-1, std::ios_base::end);
	int end_pos = searchNewLine(in, backforward) - 1;
	try {
	    string res = getData(in, start_pos, end_pos);
	    _last_pos += res.length();
	    return res;
	}
	catch (runtime_error e) {
	    return "";
	}
    }
    return "";
}

string CLog::getLastRecord3()
{
    ifstream in(_filename.c_str());
    
    if (in.is_open()) {
	in.seekg(_last_pos, std::ios_base::beg);
	int start_pos = searchNewLine(in, backforward);
	in.seekg(_last_pos, std::ios_base::beg);
	int end_pos = searchNewLine(in, forward);
	try {
	    string res = getData(in, start_pos, end_pos);
	    _last_pos = end_pos;
	    if (_last_pos < _size)
		_moreData = true;
	    else
		_moreData = false;
	    return res;
	}
	catch(runtime_error e) {
	    return "";
	}
    }
    return "";
}

const string CLog::newSessionId()
{
    char buff[255];
    
    srand(time(NULL));
    sprintf(buff, "%x%x-%x-%X-%x-%x%x%x", rand(), 
	rand(), 
	rand(), 
	((rand() & 0x0fff) | 0x4000),
	rand() % 0x3fff + 0x8000,
	rand(), rand(), rand()
    );
    _session_guid = buff;
    return _session_guid;
}
