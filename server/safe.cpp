#include <iostream>
#include <mutex>
#include <map>
#include <String>
#include "asio.hpp"
using std::shared_ptr;
using std::weak_ptr;

class Stock
{
public:
	Stock(std::string s) : name_(s)
	{
		std::cout << "Stock ctor " << s << std::endl;
	}
	~Stock()
	{
		std::cout << "Stock dtor " << name_ << std::endl;
	}

	const std::string name() const
	{
		return name_;
	}
private:
	std::string name_;
};

class StockFactor : public std::enable_shared_from_this<StockFactor>
{
	
public:
	StockFactor()
	{
		std::cout << "StockFactor ctor " << std::endl;
	}
	~StockFactor()
	{
		std::cout << "StockFactor dtor " << std::endl;
	}

	shared_ptr<Stock> get(std::string n);
private:
	static void weakDeleteCallback(const weak_ptr<StockFactor>& f, Stock* s)
	{
		// shared_ptr防止调用removeStock时StockFactor已经析构
		shared_ptr<StockFactor> temp(f.lock());
		if (temp)
		{
			temp->removeStock(s);
		}
		delete s;
	}
	void removeStock(Stock *);

	mutable std::mutex mutex_;
	std::map<std::string, weak_ptr<Stock>> stocks_;
};

shared_ptr<Stock> StockFactor::get(std::string n)
{
	shared_ptr<Stock> temp;
	const std::lock_guard<std::mutex> lock(mutex_);
	weak_ptr<Stock>& wkStock = stocks_[n];
	temp = wkStock.lock();
	if (!temp)
	{
		temp.reset(new Stock(n), std::bind(&StockFactor::weakDeleteCallback,
			weak_ptr<StockFactor>(shared_from_this()),
			std::placeholders::_1));
		wkStock = temp;
	}

	return temp;
};

void StockFactor::removeStock(Stock* s)
{
	if (s)
	{
		const std::lock_guard<std::mutex> lock(mutex_);
		stocks_.erase(s->name());
	}
};

void testLongLifeFactory()
{
	shared_ptr<StockFactor> factory(new StockFactor);
	{
		shared_ptr<Stock> stock = factory->get("NYSE:IBM");
		shared_ptr<Stock> stock2 = factory->get("NYSE:IBM");
		assert(stock == stock2);
		// stock destructs here
	}
	// factory destructs here
}
void testShortLifeFactory()
{
	shared_ptr<Stock> stock;
	{
		shared_ptr<StockFactor> factory(new StockFactor);
		stock = factory->get("NYSE:IBM");
		shared_ptr<Stock> stock2 = factory->get("NYSE:IBM");
		assert(stock == stock2);
		// factory destructs here
	}
	// stock destructs here
}

//int main()
//{
//
//	testLongLifeFactory();
//	testShortLifeFactory();
//
//}