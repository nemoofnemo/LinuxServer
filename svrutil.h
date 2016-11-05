#ifndef SVRUTIL_H
#define SVRUTIL_H
#include "svrlib.h"

namespace svrutil {
    class SRWLock;
    class Timer;
    class TimeStamp;
    class RandomString;
    class Mutex;
    class LogModule;

    template<typename type>
    class EventDispatcher;

    using std::string;
};

class svrutil::SRWLock {
private:
    pthread_rwlock_t lock;
    int readCount;
    int writeCount;

    //not available
    SRWLock(const SRWLock &) {

    }

    //not available
    void operator=(const SRWLock &) {

    }

public:
    SRWLock() {
        pthread_rwlock_init(&lock,NULL);
        readCount = 0;
        writeCount = 0;
    }

    SRWLock(const pthread_rwlockattr_t * attr) {
        pthread_rwlock_init(&lock,attr);
        readCount = 0;
        writeCount = 0;
    }

    ~SRWLock(){
        pthread_rwlock_destroy(&lock);
    }

    int AcquireExclusive(void) {
        int ret = pthread_rwlock_wrlock(&lock);
        if(ret == 0){
            writeCount++;
        }
        return ret;
    }

    int AcquireShared(void) {
        int ret = pthread_rwlock_rdlock(&lock);
        if(ret == 0){
            readCount++;
        }
        return ret;
    }

    int ReleaseExclusive(void) {
        int ret = pthread_rwlock_unlock(&lock);
        if(ret == 0){
            writeCount--;
        }
        return ret;
    }

    int ReleaseShared(void) {
        int ret = pthread_rwlock_unlock(&lock);
        if(ret == 0){
            readCount--;
        }
        return ret;
    }

    bool TryAcquireExclusive(void) {
        if (pthread_rwlock_trywrlock(&lock) == 0){
            writeCount++;
            return true;
        }
        else {
            return false;
        }
    }

    bool TryAcquireShared(void) {
        if (pthread_rwlock_tryrdlock(&lock) == 0){
            readCount++;
            return true;
        }
        else {
            return false;
        }
    }
};

class svrutil::RandomString {
public:
    static string create(int length = 16) {
        static const char v_dict[64] = {			//62bytes
            0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39,0x41,0x42,0x43,0x44,0x45,
            0x46,0x47,0x48,0x49,0x4a,0x4b,0x4c,0x4d,0x4e,0x4f,0x50,0x51,0x52,0x53,0x54,0x55,
            0x56,0x57,0x58,0x59,0x5a,0x61,0x62,0x63,0x64,0x65,0x66,0x67,0x68,0x69,0x6a,
            0x6b,0x6c,0x6d,0x6e,0x6f,0x70,0x71,0x72,0x73,0x74,0x75,0x76,0x77,0x78,0x79,0x7a };

        char * str = new char[length + 1];

        for (int i = 0; i < length; ++i) {
            str[i] = v_dict[::rand() % 62];
        }
        str[length] = 0;

        return string(str);
    }
};

class svrutil::Mutex {
private:
    pthread_mutex_t * pMutex;
    int count;

    //not available

    Mutex(const Mutex & m){

    }

    void operator=(const Mutex & m){

    }

public:
    Mutex() : pMutex(NULL), count(0){

    }

    int init(const pthread_mutexattr_t * attr = NULL){
        if(pMutex == NULL){
            pMutex = new pthread_mutex_t;
        }
        return pthread_mutex_init(pMutex, attr);
    }

    int destroy(void){
        int ret = pthread_mutex_destroy(pMutex);
        if(ret == 0){
            delete pMutex;
            pMutex = NULL;
            count = 0;
        }
        return ret;
    }

    int lock(void){
        int ret = pthread_mutex_lock(pMutex);
        if( 0 == ret ){
            count++;
        }
        return ret;
    }

    int trylock(void){
        int ret = pthread_mutex_trylock(pMutex);
        if( 0 == ret ){
            count++;
        }
        return ret;
    }

    int unlock(void){
        int ret = pthread_mutex_unlock(pMutex);
        if( 0 == ret ){
            count--;
        }
        return ret;
    }

};

class svrutil::Timer {
private:
    struct timeval m_time;

public:
    void start(void){
        gettimeofday(&m_time, NULL);
    }

    //in milliseconds
    long int stop(void){
        struct timeval record;
        gettimeofday(&record, NULL);
        long int timeUse = 1000000 * (record.tv_sec - m_time.tv_sec) + record.tv_usec - m_time.tv_usec;
        return timeUse / 1000;
    }

    long int stopInMicrosecond(void){
        struct timeval record;
        gettimeofday(&record, NULL);
        long int timeUse = 1000000 * (record.tv_sec - m_time.tv_sec) + record.tv_usec - m_time.tv_usec;
        return timeUse;
    }

    long int stopInSecond(void){
        struct timeval record;
        gettimeofday(&record, NULL);
        long int timeUse = 1000000 * (record.tv_sec - m_time.tv_sec) + record.tv_usec - m_time.tv_usec;
        return timeUse / 1000000;
    }
};

class svrutil::TimeStamp {
public:
    static std::string create(void){
        struct tm * ptr = NULL;
        time_t ti;
        char str[32] = {0};
        time(&ti);
        ptr = gmtime(&ti);
        strftime(str, 32, "[%Y/%m/%d %T] ", ptr);
        return string(str);
    }
};

class svrutil::LogModule {
private:
    std::string path;
    Mutex mutex;
    FILE * fp;

    LogModule(){

    }

    LogModule(const LogModule & l){

    }

    void operator=(const LogModule & l){

    }

public:
    LogModule(std::string name, std::string mode = string("w+")) : path(name), fp(NULL) {
        if(name != "console") {
            if((fp = fopen(name.c_str(), mode.c_str())) == NULL) {
                fprintf(stderr, "in LogModule, initialize failed.\n");
                exit(0);
            }
        }

        if(mutex.init() != 0) {
            fprintf(stderr, "in LogModule, mutex initialize failed.\n");
            exit(0);
        }
    }

    ~LogModule(){
        if(fp){
            fclose(fp);
        }
        if(mutex.destroy() != 0) {
             fprintf(stderr, "in LogModule, mutex destroy failed.\n");
        }
    }

    //with timestamp
    int write(const char * str, ...) {
        va_list vl;
        va_start(vl, str);
        int num = 0;

        std::string out("");
        out += TimeStamp::create();
        out += string(str);

        mutex.lock();
        if (path == "console") {
            num = ::vprintf(out.c_str(), vl);
        }
        else {
            num = ::vfprintf(fp, out.c_str(), vl);
        }
        mutex.unlock();

        va_end(vl);
        return num;
    }

    //no timestamp
    int print(const char * str, ...) {
        va_list vl;
        va_start(vl, str);
        int num = 0;

        mutex.lock();
        if (path == "console") {
            num = ::vprintf(str, vl);
        }
        else {
            num = ::vfprintf(fp, str, vl);
        }
        mutex.unlock();

        va_end(vl);
        return num;
    }

    void flush(void){
        if(fp){
            fflush(fp);
        }
    }
};

//dispatcher template
template<typename ArgType>
class svrutil::EventDispatcher {
public:
	const int DEFAULT_MAXTHREAD_NUM = 16;
	const int DEFAULT_SLEEP_TIME = 100;

	enum Status {RUNNING,SUSPEND,HALT};

	template<typename _ArgType>
	class Callback {
	private:
		void operator=(const Callback& cb) {
			//...
		}

		Callback(const Callback & cb) {
			//....
		}

	public:
		Callback() {
			//...
		}

		virtual ~Callback() {

		}

		virtual void run(_ArgType * pArg) {
			
		}
	};

private:
	int maxThreadNum;
	int sleepTime;
	Status status;

	std::list<std::pair<pthread_t, void*>> threadList;
	std::list<std::pair<std::string, ArgType*>> eventList;
	std::map<std::string, Callback<ArgType>*> callbackMap;
	svrutil::SRWLock lock;

	static void * workThread(void * pArg) {
		EventDispatcher * pDispatcher = (EventDispatcher*)pArg;
		string name;
		ArgType * pArgType = NULL;
		std::map<std::string, Callback<ArgType>*>::iterator it;
		bool runFlag = false;

		while (pDispatcher->status != HALT) {
			//status
			if (pDispatcher->status == SUSPEND) {
				usleep(100000);
				continue;
			}

			//get event
			pDispatcher->lock.AcquireExclusive();
			if (pDispatcher->eventList.size() > 0) {
				name = pDispatcher->eventList.front().first;
				pArgType = pDispatcher->eventList.front().second;
				pDispatcher->eventList.pop_front();
			}
			else {
				pDispatcher->lock.ReleaseExclusive();
				usleep(pDispatcher->sleepTime * 1000);
				continue;
			}
			pDispatcher->lock.ReleaseExclusive();

			//process event
			pDispatcher->lock.AcquireShared();
			it = pDispatcher->callbackMap.find(name);
			if (it != pDispatcher->callbackMap.end()) {
				runFlag = true;
			}
			pDispatcher->lock.ReleaseShared();

			if (runFlag) {
				it->second->run(pArgType);
				runFlag = false;
			}
		}

		return NULL;
	}

	//not available

	void operator=(const EventDispatcher & ED) {
		//...
	}

	EventDispatcher() {

	}

	EventDispatcher(const EventDispatcher & ED) {

	}
	
public:
	EventDispatcher(int max = DEFAULT_MAXTHREAD_NUM) {
		if (max < 1) {
			max = 1;
		}

		maxThreadNum = max;
		sleepTime = DEFAULT_SLEEP_TIME;
		status = RUNNING;

		for (int i = 0; i < maxThreadNum; ++i) {
			pthread_t thread;
            if(pthread_create(&thread, NULL, workThread, this) == 0){
			    threadList.push_back(std::pair<HANDLE, void*>(thread, NULL));
            }
		}

		Sleep(200);
	}

	virtual ~EventDispatcher() {
		status = HALT;
		std::list<std::pair<pthread_t, void*>>::iterator it = threadList.begin();
		while (it != threadList.end()) {
			void * pret = NULL;
            pthread_join(it->first, &pret);
			++it;
		}
		usleep(200000);
	}

	bool addCallback(const std::string & name, Callback<ArgType> * pCallback) {
		if (name.size() == 0 || pCallback == NULL) {
			return false;
		}

		lock.AcquireExclusive();
		std::map<std::string, Callback<ArgType>*>::iterator it = callbackMap.find(name);
		if (it == callbackMap.end()) {
			callbackMap.insert(std::pair<string, Callback<ArgType>*>(name, pCallback));
		}
		lock.ReleaseExclusive();

		return true;
	}

	bool removeCallback(const std::string & name) {
		if (name.size() == 0) {
			return false;
		}

		lock.AcquireExclusive();
		std::map<std::string, Callback<ArgType>*>::iterator it = callbackMap.find(name);
		if (it != callbackMap.end()) {
			callbackMap.erase(it);
		}
		lock.ReleaseExclusive();

		return true;
	}

	bool submitEvent(const std::string & name,ArgType * pArg) {
		lock.AcquireExclusive();
		std::map<std::string, Callback<ArgType>*>::iterator it = callbackMap.find(name);
		if (it != callbackMap.end()) {
			eventList.push_back(std::pair<std::string, ArgType*>(name, pArg));
		}
		lock.ReleaseExclusive();
		return true;
	}

	bool setStatus(Status st) {
		if (st == RUNNING || st == SUSPEND) {
			status = st;
			return true;
		}
		else {
			return false;
		}
	}

};


#endif // SVRUTIL_H
