#include<iostream>
#include <thread>
#include "binarysemaphore.h"
#include <memory>
pigeon::BinarySemaphore semObj;

void threadfun1() {
    static int c1 = 0;
    while(true) {
        semObj.wait();
        c1++;        
        std::cout<<"recv signal now!  "<< c1 <<std::endl;
    }
}

void threadfun2() {
    static int c2 = 0;
    while(true) {
        //std::this_thread::sleep_for(std::chrono::microseconds(10));
        if (c2 >= 1000000)
            break;
        
        semObj.post();
        c2++;
    }
}
int main() {
    //semObj = std::unique_ptr<pigeon::BinarySemaphore>(new pigeon::BinarySemaphore());
    std::thread t1(threadfun1);
    std::thread t2(threadfun2);
    t1.join();
    t2.join();
    return 0;

}
