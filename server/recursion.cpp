#define _SILENCE_CXX17_ALLOCATOR_VOID_DEPRECATION_WARNING
#define _SILENCE_ALL_CXX17_DEPRECATION_WARNINGS
#include <iostream>
#include <vector>
#include <mutex>
#include <assert.h>
#include <vector>

typedef std::vector<int> IntList;
typedef std::shared_ptr<IntList> IntListPtr;
std::mutex g_mutex;
IntListPtr g_ints = std::make_shared<IntList>();

void traverse()
{
	IntListPtr ints;
	{
		std::lock_guard<std::mutex> lock(g_mutex);
		ints = g_ints;
		assert(!g_ints.unique());
	}

	for (std::vector<int>::const_iterator it = ints->begin();
		it != ints->end(); ++it)
	{
		printf(" %d\n", *it);
		// do something
	}
}

void post(const int& i)
{
	printf("post\n");
	std::lock_guard<std::mutex> lock(g_mutex);
	if (!g_ints.unique())
	{
		g_ints.reset(new IntList(*g_ints));
		printf("copy the whole list\n");
	}
	assert(g_ints.unique());
	g_ints->push_back(i);
}

class a
{
public:
	a():b(1),v1(new std::vector<int>(3, 4))
	{
		std::cout << "ctor" << std::endl;
	}
	std::shared_ptr<std::vector<int>> get()
	{
		return v1;
	}
	void printV1() const
	{
		for (std::vector<int>::const_iterator it = v1->begin(); it != v1->end(); it++)
		{
			std::cout << " " << *it << std::endl;
		}
	}
	void printUseCount() const
	{
		std::cout << "use count" << v1.use_count() << std::endl;
	}
	int b;
	std::shared_ptr<std::vector<int>> v1;
};

int main()
{
	//int a = 23;
	//post(a);
	//traverse();
	a a1;
	a1.printV1();
	a1.printUseCount();
	std::cout << "------------------" << std::endl;
	std::shared_ptr<std::vector<int>> b = a1.get();
	std::cout << "use count" << b.use_count() << std::endl;
	a1.printUseCount();
	b->push_back(545);

	for (std::vector<int>::const_iterator it = b->begin(); it != b->end(); it++)
	{
		std::cout << " " << *it << std::endl;
	}
	std::cout << "------------------" << std::endl;
	a1.printV1();

}