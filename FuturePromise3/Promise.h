#pragma once

#include <condition_variable>

#include "Future.h"

template<typename T>
class MyPromise
{
public:
	MyPromise()
		: lock_( new std::mutex ), shared_state_( new SharedState<T>() ),
		is_future_got_( false ), ready_condition_variable_( new std::condition_variable() )
	{};

	void setValue( const T& value ) 
	{
		std::unique_lock<std::mutex> lock( *lock_ );
		if( shared_state_->is_ready ) {
			throw std::exception( "set value: value is already set" );
		}
		shared_state_->is_ready = true;
		shared_state_->value = value;
		ready_condition_variable_->notify_one();
	}

	void setException( const std::exception& exception ) 
	{
		std::unique_lock<std::mutex> lock( *lock_ );
		//std::cout << "Exception is thrown " << exception.what();
		if( shared_state_->is_ready ) {
			throw std::exception( "set exception: value is already set" );
		}
		shared_state_->is_ready = true;
		shared_state_->is_exception = true;
		shared_state_->exception = exception;
		ready_condition_variable_->notify_one();
	}

	std::shared_ptr<MyFuture<T>> getFuture()
	{
		if( is_future_got_ ) {
			throw std::exception( "Future is already got" );
		}
		is_future_got_ = true;
		return std::shared_ptr<MyFuture<T>>( new MyFuture<T>( shared_state_, lock_, ready_condition_variable_ ) );
	}

private:
	std::shared_ptr<SharedState<T>> shared_state_;
	std::shared_ptr<std::mutex> lock_;
	std::shared_ptr<std::condition_variable> ready_condition_variable_;
	bool is_future_got_;
};