#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <queue>
#include <condition_variable>
#include <regex>
#include <boost/locale.hpp>
#include "http_utils.h"
#include <functional>
#include "../EnterInfo.h"


std::mutex mtx;
std::condition_variable cv;
std::queue<std::function<void()>> tasks;
bool exitThreadPool = false;


void threadPoolWorker() {
	std::unique_lock<std::mutex> lock(mtx);
	while (!exitThreadPool || !tasks.empty()) {
		if (tasks.empty()) {
			cv.wait(lock);
		}
		else {
			auto task = tasks.front();
			tasks.pop();
			lock.unlock();
			task();
			lock.lock();
		}
	}
}
void parseLink(const Link& link, int depth, EnterInfo &s)
{
	try {
		
		std::cout << "\x1b[91m" << "Recursion number is " << depth << "\x1b[0m" << std::endl;
		std::this_thread::sleep_for(std::chrono::milliseconds(500));

		std::string html = getHtmlContent(link);

		if (html.size() == 0)
		{
			std::cout << s.getLinkPageName(link) << " #---> ";
			std::cout << "Failed to get HTML Content" << std::endl;
			if (s.insertBlackListPage(s.getLinkPageName(link)))
			{
				std::cout << s.getLinkPageName(link) << " added to Black List " << std::endl;
			}
			
			return;
		} 

		html.erase(std::remove(
			html.begin(),
			html.end(), '\n'),
			html.end());

		std::string documentName = s.getLinkPageName(link);
		
		// TODO: Parse HTML code here on your own
		s.setDataToDB(documentName, html);
		// TO: Collect more links from HTML code and add them to the parser like that:
		
		std::vector<Link> links = { s.extract_links(html) };
		
		if (links.size() != 0)
		{
			std::cout << "\x1b[95m" << "New Links were found = " << links.size() << "\x1b[0m" << std::endl;
			for (Link l : links) {
				std::cout << s.getLinkPageName(l) << std::endl;
			}
			std::cout << "\x1b[95m" << "Printing new links finished" << "\x1b[0m" << std::endl;
		}
		else {
			std::cout << "\x1b[95m" << "No more New links were found." << "\x1b[0m" << std::endl;
		}
		

		if (depth > 0) {
			
			std::lock_guard<std::mutex> lock(mtx);

			size_t count = links.size();
			size_t index = 0;
			for (auto& subLink : links)
			{
				tasks.push([subLink, depth, &s]() { parseLink(subLink, depth - 1, s); });
			}
			cv.notify_one();
		}
	}
	catch (const std::exception& e)
	{
		std::cout << e.what() << std::endl;
	}

}

int main()
{
	try {
		
		EnterInfo spyder("../../../../info_ini.txt");
		if (!spyder.createTable())
		{
			std::cout << "It was not possible to establish a connection with the DB" << std::endl;
		}
			
		int recursion_depth = spyder.getRecurtionDepth();

		int numThreads = std::thread::hardware_concurrency();
		std::vector<std::thread> threadPool;

		for (int i = 0; i < numThreads; ++i) {
			threadPool.emplace_back(threadPoolWorker);
		}

		Link link{ spyder.getStartingPage()};
		
		{
			std::lock_guard<std::mutex> lock(mtx);
			tasks.push([link, recursion_depth, &spyder]() { parseLink(link, recursion_depth - 1, spyder); });
			cv.notify_one();
		}
		std::this_thread::sleep_for(std::chrono::seconds(2));
		{
			std::lock_guard<std::mutex> lock(mtx);
			exitThreadPool = true;
			cv.notify_all();
		}

		for (auto& t : threadPool) {
			t.join();
		}
	}
	catch (const std::exception& e)
	{
		std::cout << e.what() << std::endl;
	}
	return 0;
}
