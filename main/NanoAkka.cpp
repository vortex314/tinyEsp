#include "NanoAkka.h"
//#include <c_types.h>


NanoStats stats;
/*
 _____ _                        _
|_   _| |__  _ __ ___  __ _  __| |
  | | | '_ \| '__/ _ \/ _` |/ _` |
  | | | | | | | |  __/ (_| | (_| |
  |_| |_| |_|_|  \___|\__,_|\__,_|
*/
int Thread::_id=0;

void Thread::createQueue()
{
    _workQueue = xQueueCreate(20, sizeof(Invoker *));
    if ( _workQueue== NULL) WARN("Queue creation failed ");
}

void Thread::start()
{
    xTaskCreate([](void* task) {
        ((Thread*)task)->run();
    }, _name.c_str(), 2000, this, 17, NULL);
}

int Thread::enqueue(Invoker* invoker)
{
    if (_workQueue)
        if (xQueueSend(_workQueue, &invoker, (TickType_t)0) != pdTRUE) {
            stats.threadQueueOverflow++;
            WARN("Thread '%s' queue overflow [%X]",_name.c_str(),invoker);
            return ENOBUFS;
        }
    return 0;
};
int Thread::enqueueFromIsr(Invoker* invoker)
{
    if (_workQueue) {
        if (xQueueSendFromISR(_workQueue, &invoker, (TickType_t)0) != pdTRUE) {
            //  WARN("queue overflow"); // cannot log here concurency issue
            stats.threadQueueOverflow++;
            return ENOBUFS;
        }
    }
    return 0;
};

void Thread::run()
{
    INFO("Thread '%s' started ",_name.c_str());
    while(true) {
        uint64_t now = Sys::millis();
        uint64_t expTime = now + 5000;
        TimerSource *expiredTimer = 0;
        // find next expired timer if any within 5 sec
        for (auto timer : _timers) {
            if (timer->expireTime() < expTime) {
                expTime = timer->expireTime();
                expiredTimer = timer;
            }
        }

        if (expiredTimer && (expTime <= now)) {
            if (expiredTimer) {
                int32_t delta = now -expTime;
				if ( delta>100 ) INFO("Timer[%X] on thread %s already expired by %u",expiredTimer,_name.c_str(),delta);
				if ( delta<0 ) INFO("Timer[%X] on thread %s not yet expired by %u",expiredTimer,_name.c_str(),delta);
                expiredTimer->request();
            }
        } else {
            Invoker *prq;
            int32_t waitTime = pdMS_TO_TICKS(expTime - now) + 1;
            if (waitTime < 0)
                waitTime = 0;
//            uint32_t queueCounter = 0;
//			INFO(" waitTime : %u from %d ",waitTime,(uint32_t)((expTime-now)+1));
            if (xQueueReceive(_workQueue, &prq, (TickType_t)waitTime) == pdTRUE) {
                uint64_t start=Sys::millis();
                prq->invoke();
                uint32_t delta=Sys::millis()-start;
                if ( delta > 50 ) WARN(" slow %d msec invoker [%X]",delta,prq);
            }
            if (expiredTimer) {
 //               INFO(" expiredTimer : %X ",expiredTimer);
               expiredTimer->request();
            }
        }
    }
}
