#pragma once

#include <condition_variable>

#include "Promise.h"

template<typename T>
struct SharedState
{
	bool is_ready;
	bool is_exception;
	T value;
	std::exception exception;

	SharedState() : is_ready( false ), is_exception( false ) {}
};

class IReadyCheckable
{
public:
	virtual bool isReady() = 0;
};

template<typename T>
class MyFuture : public IReadyCheckable
{
public:
	MyFuture( std::shared_ptr<SharedState<T>> _shared_state, std::shared_ptr<std::mutex> _lock, std::shared_ptr<std::condition_variable> _ready_condition_variable )
		: shared_state_( _shared_state ), lock_( _lock ), ready_condition_variable_( _ready_condition_variable )
	{};

	T get()
	{		std::unique_lock<std::mutex> lock( *lock_ );
		ready_condition_variable_->wait( lock, [&]() {return shared_state_->is_ready; } );
		if( shared_state_->is_exception ) {
			throw shared_state_->exception;
		} 
		return shared_state_->value;
	}

	bool tryGet(T& return_value)
	{		std::unique_lock<std::mutex> lock( *lock_ );
		if( shared_state_->is_ready ) {
			if( shared_state_->is_exception ) {
				throw shared_state_->exception;
			}
			return_value = shared_state_->value;
			return true;
		}
		return false;
	}

	bool isReady()
	{
		std::unique_lock<std::mutex> lock( *lock_ );
		return shared_state_->is_ready;
	}

private:
	std::shared_ptr<SharedState<T>> shared_state_;
	std::shared_ptr<std::mutex> lock_;
	std::shared_ptr<std::condition_variable> ready_condition_variable_;
};