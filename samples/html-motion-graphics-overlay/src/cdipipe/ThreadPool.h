#pragma once

#include <boost/asio/thread_pool.hpp>
#include <boost/asio/post.hpp>

namespace CdiTools
{
    class ThreadPool
    {
    public:
        ThreadPool(std::size_t num_threads) : pool_{ num_threads } {}
        ~ThreadPool()
        {
            pool_.join();
        }

        template<typename CompletionToken>
        void post(CompletionToken&& token)
        {
            boost::asio::post(pool_, token);
        }

        static ThreadPool& instance() {
            static ThreadPool theInstance(std::thread::hardware_concurrency());

            return theInstance;
        }

    private:
        boost::asio::thread_pool pool_;
    };
}
