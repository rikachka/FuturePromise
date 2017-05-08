#pragma once

#include "Promise.h"
#include "Future.h"
#include "Async.h"

template <typename T>
class MyChainTask : public IFutureTask<T>
{
public:
	MyChainTask( std::shared_ptr<IFutureTask> _task_first, std::shared_ptr<IFutureTask> _task_second )
		: task_first_(_task_first), task_second_(_task_second), 
		future_first_( _task_first->getFuture() ), future_second_( _task_second->getFuture() ), 
		is_chain_(true)
	{}

	MyChainTask( std::shared_ptr<IFutureTask> _task_first)
		: task_first_( _task_first ), future_first_( _task_first->getFuture() ), is_chain_(false)
	{}

	void operator()()
	{
		try {
			(*(task_first_.get()))();
			T value = future_first_->get();
			if( !is_chain_ ) {
				std::cout << "first: " << value << std::endl;
				promise_.setValue( value );
			}
		} catch( std::exception& e ) {
			std::cout << "first: " << e.what() << std::endl;
			promise_.setException( e );
			return;
		}

		if( is_chain_ ) {
			try {
				(*(task_second_.get()))();
				T value = future_second_->get();
				std::cout << "second: " << value << std::endl;
				promise_.setValue( value );
			} catch( std::exception& e ) {
				std::cout << "second: " << e.what() << std::endl;
				promise_.setException( e );
			}
		}
	}

	std::shared_ptr<MyFuture<T>> getFuture()
	{
		if( !is_future_got_ ) {
			future_ = promise_.getFuture();
		}
		is_future_got_ = true;
		return future_;
	}

private:
	std::shared_ptr<IFutureTask> task_first_;
	std::shared_ptr<IFutureTask> task_second_;
	std::shared_ptr<MyFuture<T>> future_first_;
	std::shared_ptr<MyFuture<T>> future_second_;
	MyPromise<T> promise_;
	bool is_chain_;
	std::shared_ptr<MyFuture<T>> future_;
	bool is_future_got_ = false;
};



template <typename T>
class MyChainExecutor
{
public:
	MyChainExecutor() :chain_task_( nullptr ) {}

	std::shared_ptr<MyFuture<T>> set( std::function<T()> function )
	{
		std::shared_ptr<MyAsyncTask<T>> task (new MyAsyncTask<T>( function ));
		chain_task_ = std::shared_ptr<MyChainTask<T>>( new MyChainTask<T>( task ));
		return chain_task_->getFuture();
	}

	std::shared_ptr<MyFuture<T>> then( std::function<T()> function )
	{
		std::shared_ptr<MyAsyncTask<T>> task (new MyAsyncTask<T>( function ));
		chain_task_ = std::shared_ptr<MyChainTask<T>>(new MyChainTask<T>( chain_task_, task ));
		return chain_task_->getFuture();
	}

	void execute()
	{
		if( chain_task_ ) {
			(*(chain_task_.get()))();
		}
	}

private:
	std::shared_ptr<MyChainTask<T>> chain_task_;
};