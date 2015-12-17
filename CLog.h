#ifndef __CLOG_H
#define __CLOG_H

#include <string>
#include <yaml-cpp/yaml.h>

#define S_ISOK(m) (S_ISLNK(m) || S_ISREG(m))

using namespace std;

enum {forward, backforward};

/**
 * Класс реализующий необходимые операции с файлом журнала
 */
class CLog
{
private:
    // имя контролируемого файла
    string _filename;
    // некий ключ связанный с данным журналом
    string _key;
    // уникальный ключ сессии, связанный с данным журналом
    string _session_guid;
    // время создания файла
    time_t _last_ctime;
    // время модификации файла
    time_t _last_mtime;
    // размер файла на предыдущем цикле
    off_t _size;
    // признак наличия в журнале не обработанных данных
    bool _moreData;
    //
    int _last_pos;
    // сборный статус файла (0-й бит признак создания файла, 1-й бит признак модификации файла)
    int _status;
public:
    static CLog* parse(const YAML::Node &node);
    CLog(const string &filename, const string &key);
    // возращает имя файла
    const string filename() const
    {
	return _filename;
    }
    // возвращает ключ журнала
    const string key() const
    {
	return _key;
    }
    // возвращает ключ сессии, связанный с данным журналом
    const string sessionId() const
    {
	return _session_guid;
    }
    // генерирует новый ключ сессии
    const string newSessionId();
    // Выполняет контроль модификации файла. Выставляет биты модификации
    bool check();
    // Возвращает true, при создании файла
    bool isCreated()
    {
	return _status & 0x1;
    }
    // Возвращает true, при модификации файла
    bool isModified()
    {
	return _status & 0x2;
    }
    // возвращает признак наличия необработанных данных
    bool isMoreData()
    {
	return _status & 0x4;
    }
    // возаращает последнюю запись в журнале
    string getLastRecord() const;
    string getLastRecord2();
    string getLastRecord3();
    
};

#endif
