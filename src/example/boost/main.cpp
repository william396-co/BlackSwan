#include <iostream>

#include "work_guard.h"
#include "io_context.h"
#include "steady_timer.h"
#include "async_accept.h"

//refernece page https://hicalio.cn/posts/%E6%B7%B1%E5%85%A5%E5%AD%A6%E4%B9%A0boost.asio_%E5%85%A5%E9%97%A8%E7%AF%87/

int main(){

    //io_context_test();
    //work_guard_test();
    async_accept_test();

    return 0;
}
