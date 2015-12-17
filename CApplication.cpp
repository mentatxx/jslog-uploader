#include <yaml-cpp/yaml.h>
#include <openssl/ssl.h>
#include <curl/curl.h>
#include <json/json.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <time.h>

#include <iostream>
#include <fstream>
#include <stdexcept>

#include "CApplication.h"
#include "CLog.h"

CApplication theApp;

size_t writefunction(void* ptr, size_t size, size_t nmemb, void* stream)
{
  fwrite(ptr, size, nmemb, (FILE*)stream);
  return (nmemb * size);
}

static CURLcode sslctx_function(CURL* curl, void* sslctx, void* parm)
{
  X509_STORE* store;
  X509* cert=NULL;
  BIO* bio;
  char* mypem = /* www.cacert.org */
    "-----BEGIN CERTIFICATE-----\n"\
    "MIIHPTCCBSWgAwIBAgIBADANBgkqhkiG9w0BAQQFADB5MRAwDgYDVQQKEwdSb290\n"\
    "IENBMR4wHAYDVQQLExVodHRwOi8vd3d3LmNhY2VydC5vcmcxIjAgBgNVBAMTGUNB\n"\
    "IENlcnQgU2lnbmluZyBBdXRob3JpdHkxITAfBgkqhkiG9w0BCQEWEnN1cHBvcnRA\n"\
    "Y2FjZXJ0Lm9yZzAeFw0wMzAzMzAxMjI5NDlaFw0zMzAzMjkxMjI5NDlaMHkxEDAO\n"\
    "BgNVBAoTB1Jvb3QgQ0ExHjAcBgNVBAsTFWh0dHA6Ly93d3cuY2FjZXJ0Lm9yZzEi\n"\
    "MCAGA1UEAxMZQ0EgQ2VydCBTaWduaW5nIEF1dGhvcml0eTEhMB8GCSqGSIb3DQEJ\n"\
    "ARYSc3VwcG9ydEBjYWNlcnQub3JnMIICIjANBgkqhkiG9w0BAQEFAAOCAg8AMIIC\n"\
    "CgKCAgEAziLA4kZ97DYoB1CW8qAzQIxL8TtmPzHlawI229Z89vGIj053NgVBlfkJ\n"\
    "8BLPRoZzYLdufujAWGSuzbCtRRcMY/pnCujW0r8+55jE8Ez64AO7NV1sId6eINm6\n"\
    "zWYyN3L69wj1x81YyY7nDl7qPv4coRQKFWyGhFtkZip6qUtTefWIonvuLwphK42y\n"\
    "fk1WpRPs6tqSnqxEQR5YYGUFZvjARL3LlPdCfgv3ZWiYUQXw8wWRBB0bF4LsyFe7\n"\
    "w2t6iPGwcswlWyCR7BYCEo8y6RcYSNDHBS4CMEK4JZwFaz+qOqfrU0j36NK2B5jc\n"\
    "G8Y0f3/JHIJ6BVgrCFvzOKKrF11myZjXnhCLotLddJr3cQxyYN/Nb5gznZY0dj4k\n"\
    "epKwDpUeb+agRThHqtdB7Uq3EvbXG4OKDy7YCbZZ16oE/9KTfWgu3YtLq1i6L43q\n"\
    "laegw1SJpfvbi1EinbLDvhG+LJGGi5Z4rSDTii8aP8bQUWWHIbEZAWV/RRyH9XzQ\n"\
    "QUxPKZgh/TMfdQwEUfoZd9vUFBzugcMd9Zi3aQaRIt0AUMyBMawSB3s42mhb5ivU\n"\
    "fslfrejrckzzAeVLIL+aplfKkQABi6F1ITe1Yw1nPkZPcCBnzsXWWdsC4PDSy826\n"\
    "YreQQejdIOQpvGQpQsgi3Hia/0PsmBsJUUtaWsJx8cTLc6nloQsCAwEAAaOCAc4w\n"\
    "ggHKMB0GA1UdDgQWBBQWtTIb1Mfz4OaO873SsDrusjkY0TCBowYDVR0jBIGbMIGY\n"\
    "gBQWtTIb1Mfz4OaO873SsDrusjkY0aF9pHsweTEQMA4GA1UEChMHUm9vdCBDQTEe\n"\
    "MBwGA1UECxMVaHR0cDovL3d3dy5jYWNlcnQub3JnMSIwIAYDVQQDExlDQSBDZXJ0\n"\
    "IFNpZ25pbmcgQXV0aG9yaXR5MSEwHwYJKoZIhvcNAQkBFhJzdXBwb3J0QGNhY2Vy\n"\
    "dC5vcmeCAQAwDwYDVR0TAQH/BAUwAwEB/zAyBgNVHR8EKzApMCegJaAjhiFodHRw\n"\
    "czovL3d3dy5jYWNlcnQub3JnL3Jldm9rZS5jcmwwMAYJYIZIAYb4QgEEBCMWIWh0\n"\
    "dHBzOi8vd3d3LmNhY2VydC5vcmcvcmV2b2tlLmNybDA0BglghkgBhvhCAQgEJxYl\n"\
    "aHR0cDovL3d3dy5jYWNlcnQub3JnL2luZGV4LnBocD9pZD0xMDBWBglghkgBhvhC\n"\
    "AQ0ESRZHVG8gZ2V0IHlvdXIgb3duIGNlcnRpZmljYXRlIGZvciBGUkVFIGhlYWQg\n"\
    "b3ZlciB0byBodHRwOi8vd3d3LmNhY2VydC5vcmcwDQYJKoZIhvcNAQEEBQADggIB\n"\
    "ACjH7pyCArpcgBLKNQodgW+JapnM8mgPf6fhjViVPr3yBsOQWqy1YPaZQwGjiHCc\n"\
    "nWKdpIevZ1gNMDY75q1I08t0AoZxPuIrA2jxNGJARjtT6ij0rPtmlVOKTV39O9lg\n"\
    "18p5aTuxZZKmxoGCXJzN600BiqXfEVWqFcofN8CCmHBh22p8lqOOLlQ+TyGpkO/c\n"\
    "gr/c6EWtTZBzCDyUZbAEmXZ/4rzCahWqlwQ3JNgelE5tDlG+1sSPypZt90Pf6DBl\n"\
    "Jzt7u0NDY8RD97LsaMzhGY4i+5jhe1o+ATc7iwiwovOVThrLm82asduycPAtStvY\n"\
    "sONvRUgzEv/+PDIqVPfE94rwiCPCR/5kenHA0R6mY7AHfqQv0wGP3J8rtsYIqQ+T\n"\
    "SCX8Ev2fQtzzxD72V7DX3WnRBnc0CkvSyqD/HMaMyRa+xMwyN2hzXwj7UfdJUzYF\n"\
    "CpUCTPJ5GhD22Dp1nPMd8aINcGeGG7MW9S/lpOt5hvk9C8JzC6WZrG/8Z7jlLwum\n"\
    "GCSNe9FINSkYQKyTYOGWhlC0elnYjyELn8+CkcY7v2vcB5G5l1YjqrZslMZIBjzk\n"\
    "zk6q5PYvCdxTby78dOs6Y5nCpqyJvKeyRKANihDjbPIky/qbn3BHLt4Ui9SyIAmW\n"\
    "omTxJBzcoTWcFbLUvFUufQb1nA5V9FrWk9p2rSVzTMVD\n"\
    "-----END CERTIFICATE-----\n";
  /* get a BIO */
  bio=BIO_new_mem_buf(mypem, -1);
  /* use it to read the PEM formatted certificate from memory into an X509
   * structure that SSL can use
   */
  PEM_read_bio_X509(bio, &cert, 0, NULL);
  if (cert == NULL)
    printf("PEM_read_bio_X509 failed...\n");

  /* get a pointer to the X509 certificate store (which may be empty!) */
  store=SSL_CTX_get_cert_store((SSL_CTX *)sslctx);

  /* add our certificate to this store */
  if (X509_STORE_add_cert(store, cert)==0)
    printf("error adding certificate\n");

  /* decrease reference counts */
  X509_free(cert);
  BIO_free(bio);

  /* all set to go */
  return CURLE_OK ;
}

void CApplication::readConfig()
{
    ifstream in(CONFIG_FILE);

    if (!in.is_open())
	throw runtime_error("Failed to read config file!");

    YAML::Node doc;
    
    try {
	YAML::Parser parser(in);
	parser.GetNextDocument(doc);
	
	for (unsigned nI = 0; nI < doc.size(); nI++) {
	    _logs.push_back(CLog::parse(doc[nI]["project"]));
	}
    }
    catch(...) {
	throw runtime_error("Failed to parse config file");
    }
    in.close();
}

void CApplication::run()
{
    vector<CLog*>::iterator iter;
    struct stat statBuff;
    struct timespec sleep_interval;

    // при старте проверяем доступность файлов
    for (iter = _logs.begin(); iter != _logs.end(); ++iter) {
	// не смогли получить данные о файле, файл не символьная ссылка и не обычный файл
	if (stat((*iter)->filename().c_str(), &statBuff) < 0 || !S_ISOK(statBuff.st_mode))
	{
	    cout << "Файл " << (*iter)->filename() << " не найден или не может быть обработан!" << endl ;
	}
    }
    // время приостановки процесса на 1 сек.
    sleep_interval.tv_sec = 1;
    sleep_interval.tv_nsec = 0;
    
    curl_global_init(CURL_GLOBAL_ALL);
    
    // инициализация CURL
    CURL *curl = curl_easy_init();
    if (!curl)
	throw runtime_error("Failed to init CURL");

    struct curl_slist *headers = NULL;
    
    headers = curl_slist_append(headers, "Accept: application/json");
    headers = curl_slist_append(headers, "Content-Type: application/json");
    
    curl_easy_setopt(curl, CURLOPT_URL, DEST_URL);
    curl_easy_setopt(curl, CURLOPT_VERBOSE, 0L);
    curl_easy_setopt(curl, CURLOPT_HEADER, 0L);
    curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L);
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, *writefunction);
#ifdef DEBUG
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, stdout);
    curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, *writefunction);
    curl_easy_setopt(curl, CURLOPT_HEADERDATA, stderr);
#endif
    curl_easy_setopt(curl, CURLOPT_SSLCERTTYPE,"PEM");
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, false);
    curl_easy_setopt(curl, CURLOPT_SSL_CTX_FUNCTION, *sslctx_function);
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "POST");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    
    _bStop = false;
    // цикл контроля за файлами
    while(!_bStop) {
	for (iter = _logs.begin(); iter != _logs.end(); ++iter) {
	    // проверяем изменилось ли состояние файла
	    if ((*iter)->check()) {
		string postData;
		// журнал был впервые или вновь создан или приложение только что запущено
		if ((*iter)->isCreated()) {
		    (*iter)->newSessionId();
		    postData = prepareSystemInfoRequest(*iter);
		}
		// журнал бы изменен с момента последней проверки или имеются необработанные данные
		else if ((*iter)->isModified() || (*iter)->isMoreData()) {
		    postData = prepareErrorRequest(*iter);
		}
		if (postData.length() > 0){
		    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postData.c_str());
#ifdef DEBUG
    cerr << "CApplication::run: " << "JSON data" << postData.c_str() << endl;
#endif
		    CURLcode curlCode = curl_easy_perform(curl);
		    if (curlCode != CURLE_OK)
			cerr << "curl_easy_perform() failed: " << curl_easy_strerror(curlCode) << endl;
		}
	    }
	}
	// заснуть на некоторое время
	nanosleep(&sleep_interval, NULL);
    }
    curl_easy_cleanup(curl);
}

string CApplication::prepareSystemInfoRequest(CLog *log) const
{
    json_object* res 		= json_object_new_object();
    json_object* key 		= json_object_new_string(log->key().c_str());
    json_object* time 		= json_object_new_int(::time(NULL));
    json_object* sessionId 	= json_object_new_string(log->sessionId().c_str());
    json_object* type 		= json_object_new_string("systemInfo");
    json_object* hostId 	= json_object_new_string(log->key().c_str());
    json_object* data 		= json_object_new_object();
    json_object* userAgent 	= json_object_new_string("Log uploder");
    json_object* platform 	= json_object_new_string("Linux");
    json_object* version 	= json_object_new_string("");
    
    json_object_object_add(data, "userAgent", userAgent);
    json_object_object_add(data, "platform", platform);
    json_object_object_add(data, "version", version);
    json_object_object_add(res, "key", key);
    json_object_object_add(res, "time", time);
    json_object_object_add(res, "sessionId", sessionId);
    json_object_object_add(res, "type", type);
    json_object_object_add(res, "hostId", hostId);
    json_object_object_add(res, "data", data);
    
    return json_object_to_json_string(res);
}

string CApplication::prepareErrorRequest(CLog *log) const
{
    json_object* res 		= json_object_new_object();
    json_object* key		= json_object_new_string(log->key().c_str());
    json_object* time		= json_object_new_int(::time(NULL));
    json_object* sessionId	= json_object_new_string(log->sessionId().c_str());
    json_object* type		= json_object_new_string("error");
    json_object* hostId		= json_object_new_string(log->key().c_str());
    json_object* data		= json_object_new_string(log->getLastRecord3().c_str());

    if (strlen(json_object_get_string(data)) > 0) {    
	json_object_object_add(res, "key", key);
	json_object_object_add(res, "time", time);
	json_object_object_add(res, "sessionId", sessionId);
	json_object_object_add(res, "type", type);
	json_object_object_add(res, "hostId", hostId);
	json_object_object_add(res, "data", data);
    
	return json_object_to_json_string(res);
    }
    return "";
}

void CApplication::stop()
{
    curl_global_cleanup();
    _bStop = true;
}
