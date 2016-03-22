/**
* @file semaphore.h
* @brief for mqtt module and zmq msg sync 
* @author europelee, europelee@gmail.com
* @version 0.0.1
* @date 2015-12-22
*/

#ifndef _BINARY_SEMAPHORE_H
#define _BINARY_SEMAPHORE_H

#include <semaphore.h>
#include <errno.h>
#include <assert.h>
#include <time.h>

namespace pigeon {
    class BinarySemaphore {
        public:
            //! ctor
            BinarySemaphore() {
                int ret = sem_init( &my_sem, /*shared among threads*/ 0, 0 );
                assert( ret==0 );
            }
            //! dtor
            ~BinarySemaphore() {
                int ret = sem_destroy( &my_sem );
                assert( ret==0 );
            }
            //! wait/acquire
            void wait() {
                while( sem_wait( &my_sem )!=0 )
                    assert( errno==EINTR);
            }
            
            int timeWait(int sec) {
                struct timespec ts;
                int ret = clock_gettime(CLOCK_REALTIME, &ts); 
                assert(ret != -1);
                ts.tv_sec += sec;
                return sem_timedwait(&my_sem, &ts);
            }

            //! post/release 
            void post() { sem_post( &my_sem ); }
        private:
            sem_t my_sem;
    };
}
#endif
