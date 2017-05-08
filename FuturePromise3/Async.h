#pragma once

#include <list>

#include "Promise.h"
#include "Future.h"

template <typename T>
class MyAsyncTask
{
public:
	MyAsyncTask( std::function<T()> _function ) : function_( _function ) {}

	void operator()()
	{
		try {
			promise_.setValue( function_() );
		} catch( std::exception& e ) {
			promise_.setException( e );
		}
	}

	std::shared_ptr<MyFuture<T>> getFuture() 
	{ 
		return promise_.getFuture(); 
	}

private:
	std::function<T()> function_;
	MyPromise<T> promise_;
};



class MyAsyncExecutor
{
public:
	MyAsyncExecutor( int _threads_number ) : threads_number_( _threads_number ) {}

	~MyAsyncExecutor()
	{
		while( threads_.size() > 0 ) {
			//threads_.front().thread_.join();
			//threads_.pop_front();
			checkFinishedThreads();
		}
	}

	template<typename T>
	std::shared_ptr<MyFuture<T>> execute( std::function<T()> function, bool is_async = true )
	{
		MyAsyncTask<T> task( function );
		std::shared_ptr<MyFuture<T>> task_future = task.getFuture();

		if( is_async && threads_.size() < threads_number_ ) {
			std::thread thread( task );
			std::cout << "Started thread " << thread.get_id() << std::endl;
			threads_.push_back( Thread(std::move(thread), task_future) );
		} else {
			task();
			std::cout << "Sync task" << std::endl;
		}
		checkFinishedThreads();
		return task_future;
	}

	void checkFinishedThreads()
	{
		for( auto it = threads_.begin(); it != threads_.end(); ) {
			if( it->future_->isReady() ) {
				std::cout << "Finished thread " << it->thread_.get_id() << std::endl;
				it->thread_.join();
				threads_.erase( it );
				it = threads_.begin();
			} else {
				it++;
			}
		}
	}

private:
	struct Thread
	{
		std::thread thread_;
		std::shared_ptr<IReadyCheckable> future_;

		Thread( std::thread _thread, std::shared_ptr<IReadyCheckable> _future ) :thread_( std::move(_thread) ), future_( _future ) {}
	};

	int threads_number_;
	std::list<Thread> threads_;
};