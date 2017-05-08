// FuturePromise3.cpp: определяет точку входа для консольного приложения.
//

#include "stdafx.h"
#include "conio.h"
#include <thread>
#include <iostream>
#include <stdlib.h>
#include <vector>

#include "Promise.h"
#include "Future.h"
#include "Async.h"
#include "Chain.h"

void test_future_promise_get_value()
{
	std::cout << "===TEST_FUTURE_PROMISE_GET_VALUE===" << std::endl;
	MyPromise<int> prom;										// create promise
	std::shared_ptr<MyFuture<int>> fut = prom.getFuture();		// engagement with future
	std::thread th1( 
		[]( auto fut ) { std::cout << "Value: " << fut->get() << std::endl; },
		fut 
	);															// send future to new thread
	std::cout << "Value is not set yet" << std::endl;
	prom.setValue( 10 );										// fulfill promise (synchronizes with getting the future)
	std::cout << "Value is set" << std::endl;
	th1.join();
	std::cout << std::endl;
}

void test_future_promise_try_get_exception()
{
	std::cout << "===TEST_FUTURE_PROMISE_TRY_GET_EXCEPTION===" << std::endl;
	MyPromise<int> prom;										// create promise
	std::shared_ptr<MyFuture<int>> fut = prom.getFuture();		// engagement with future
	std::thread th1(
		[]( std::shared_ptr<MyFuture<int>> fut ) {
			try {
				int i = 0;
				while( !fut->tryGet(i) ) {
					std::cout << "Waiting " << i++ << std::endl;
					std::this_thread::sleep_for( std::chrono::seconds( 1 ) );
				}
			} catch( std::exception& e ) {
				std::cout << "Exception: " << e.what() << std::endl;
			}
		},
		fut
	);															// send future to new thread
	std::cout << "Exception is not set yet" << std::endl;
	std::this_thread::sleep_for( std::chrono::seconds( 5 ) );
	prom.setException( std::exception("Exception in promise") );// fulfill promise (synchronizes with getting the future)
	std::cout << "Exception is set" << std::endl;
	th1.join();
	std::cout << std::endl;
}

int sum( int max )
{
	int sum = 0;
	for( int i = 1; i < max; i++ ) {
		sum += i;
	}
	std::this_thread::sleep_for( std::chrono::seconds( rand() % 5 ) );
	return sum;
}

void test_async()
{
	std::cout << "===TEST_ASYNC===" << std::endl;
	srand( 1 );
	MyAsyncExecutor executor( 5 );
	std::vector<std::shared_ptr<MyFuture<int>>> results;
	std::cout << "Async tasks: " << std::endl;
	for( int i = 0; i < 10; i++ ) {
		results.push_back( executor.execute<int>( std::bind( sum, rand() ) ) );
	}
	std::cout << "Sync tasks: " << std::endl;
	for( int i = 0; i < 3; i++ ) {
		results.push_back( executor.execute<int>( std::bind( sum, rand() ), false ) );
	}
	for( int i = 0; i < results.size(); i++ ) {
		std::cout << results[i]->get() << std::endl;
	}
	std::cout << std::endl;
}

void test_chain()
{
	std::cout << "===TEST_CHAIN===" << std::endl;
	MyChainExecutor<int> executor;
	std::vector<std::shared_ptr<MyFuture<int>>> results;
	results.push_back( executor.set( std::bind( sum, rand() ) ) );
	for( int i = 0; i < 5; i++ ) {
		results.push_back( executor.then( std::bind( sum, rand() ) ) );
	}
	executor.execute();
	for( int i = 0; i < results.size(); i++ ) {
		std::cout << results[i]->get() << std::endl;
	}
	std::cout  << std::endl;
}

int throw_exception()
{
	throw std::exception( "Test exception" );
	return 0;
}

void test_chain_exception()
{
	std::cout << "===TEST_CHAIN_EXCEPTION===" << std::endl;
	MyChainExecutor<int> executor;
	std::vector<std::shared_ptr<MyFuture<int>>> results;
	results.push_back( executor.set( std::bind( sum, rand() ) ) );
	for( int i = 0; i < 1; i++ ) {
		results.push_back( executor.then( std::bind( sum, rand() ) ) );
	}
	results.push_back( executor.then( throw_exception ) );
	for( int i = 0; i < 1; i++ ) {
		results.push_back( executor.then( std::bind( sum, rand() ) ) );
	}
	executor.execute();
	for( int i = 0; i < results.size(); i++ ) {
		std::cout << i << ": ";
		if( results[i]->isReady() ) {
			try {
				std::cout << results[i]->get();
			} catch( std::exception e ) {
				std::cout << "exception: " << e.what();
			}
		}
		std::cout << std::endl;
	}
	std::cout << std::endl;
}

int main()
{
	test_future_promise_get_value();
	test_future_promise_try_get_exception();
	test_async();
	test_chain();
	test_chain_exception();
	_getch();
	return 0;
}

