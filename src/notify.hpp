#include <sys/types.h>
#include <sys/inotify.h>
#include <unistd.h>

#define EVENT_SIZE    (sizeof (struct inotify_event))
#define EVENT_BUF_LEN (1024 * (EVENT_SIZE + 16))

struct Notifier {

    Notifier(const char *filepath) {
    	notifyHandler = inotify_init1(IN_NONBLOCK);
    	if ( notifyHandler < 0 ) throw std::runtime_error("can't init inotify");
    	watchHandler = inotify_add_watch(notifyHandler, filepath, IN_MODIFY);
    }

    void waitSignal(){
    	while(true) {
        	int length = read(notifyHandler, buffer, EVENT_BUF_LEN);
        	if (length <= 0) usleep(100);
            else return;
        }
    }

	private: 
		int notifyHandler;
    	int watchHandler;
    	char buffer[EVENT_BUF_LEN];
};