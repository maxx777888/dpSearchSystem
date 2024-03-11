#include "EnterInfo.h"

//Constructor
EnterInfo::EnterInfo(const std::string& _file) : fileName(_file)
{

	try 
	{
		file_path.open(fileName);
		if (file_path.is_open()) 
		{
			std::cout << "File Opened!" << std::endl;
			getEnterInfo();
			getServerInfo();
		}	
	}
	catch (const std::ifstream::failure& e) {
		std::cerr << e.what() << std::endl;
	}	
}


bool EnterInfo::isEstablishConnection(std::shared_ptr<pqxx::connection> c)
{
	if (c) {
		std::cout << "Establish connection to DB!" << std::endl;
		return true;
	}
	else {
		std::cout << "Error occure while connecting to DB!" << std::endl;
		return false;
	}
}

void EnterInfo::creatTable()
{
	try
	{
		pqxx::work tx(*conPQXX);
		// Создание таблицы "Документы"
		tx.exec("CREATE TABLE IF NOT EXISTS documents ("
			"id SERIAL PRIMARY KEY, "
			"page_title VARCHAR(255) NOT NULL UNIQUE "
			")");

		// Создание таблицы "Слова"
		tx.exec("CREATE TABLE IF NOT EXISTS words ("
			"id SERIAL PRIMARY KEY, "
			"word VARCHAR(255) NOT NULL UNIQUE "
			")");

		// Создание промежуточной таблицы "Документы_Слова"
		tx.exec("CREATE TABLE IF NOT EXISTS documents_words ("
			"document_id INTEGER NOT NULL, "
			"word_id INTEGER NOT NULL, "
			"app_num INTEGER NOT NULL, "
			"PRIMARY KEY(document_id, word_id), "
			"FOREIGN KEY(document_id) REFERENCES documents(id), "
			"FOREIGN KEY(word_id) REFERENCES words(id)"
			")");

		tx.commit();
	}
	catch (pqxx::sql_error e)
	{
		std::cout << e.what() << std::endl;
	}
}

std::vector<int> EnterInfo::getAllPagesIdList()
{
	pqxx::work w(*conPQXX);
	pqxx::result r = w.exec("SELECT id FROM documents");

	std::vector<int> ids;
	for (const auto& row : r) {
		ids.push_back(row["id"].as<int>());
	}
	return ids;
}

std::vector<std::string> EnterInfo::split_string(const std::string& str)
{	
	std::string readyStr = str;
	std::string cleanString = readyStr.erase(0, 6);
	std::replace(cleanString.begin(), cleanString.end(), '+', ' ');

	cleanString = remove_html_tags(cleanString);
	cleanString = remove_punc(cleanString);
	cleanString = remove_whitespace(cleanString);
	cleanString = make_lower_case(cleanString);
	return remove_duplicates(cleanString);
}

std::string EnterInfo::remove_html_tags(const std::string& html)
{
	std::regex tag_regex("<[^>]*>");
	return std::regex_replace(html, tag_regex, "");
}

std::string EnterInfo::remove_punc(const std::string& text)
{
	std::regex punc_regex("[[:punct:]]");
	return std::regex_replace(text, punc_regex, "");
}

std::string EnterInfo::remove_whitespace(const std::string& text)
{
	std::regex whitespace_regex("[[:space:]]");
	return std::regex_replace(text, whitespace_regex, " ");
}

std::string EnterInfo::make_lower_case(const std::string& text)
{
	boost::locale::generator gen;
	std::locale loc = gen("en_US.UTF-8");
	std::string lower_str;
	for (char c : text) 
	{
		lower_str += std::tolower(c, loc);
	}
	return lower_str;
}

std::vector<std::string> EnterInfo::remove_duplicates(const std::string& text)
{
	std::regex word_regex("[[:alpha:]]+");
	std::sregex_iterator it(text.begin(), text.end(), word_regex);
	std::set<std::string> unique_words;

	while (it != std::sregex_iterator()) 
	{
		std::string word = it->str();
		if (unique_words.insert(word).second) {}
		++it;
	}
	std::vector<std::string> unique_words_vector(unique_words.begin(), unique_words.end());
	return unique_words_vector;
}

std::string EnterInfo::getCleanText(const std::string& text)
{
	std::string str = remove_html_tags(text);
	str = remove_punc(str);
	str = remove_whitespace(str);
	str = get_ready_to_rank_text(str);
	str = make_lower_case(str);

	return str;
}

bool EnterInfo::is_valid_word(const std::string& word)
{
	return word.size() >= 3 && word.size() <= 32;
}

std::string EnterInfo::get_ready_to_rank_text(const std::string& text)
{
	std::vector<std::string> words;
	std::regex word_regex("[[:alpha:]]+");
	std::sregex_iterator it(text.begin(), text.end(), word_regex);
	while (it != std::sregex_iterator()) 
	{
		std::string word = it->str();
		if (is_valid_word(word)) 
		{
			words.push_back(word);
		}
		++it;
	}
	std::string result;
	for (const auto& word : words) 
	{
		result += word + " ";
	}
	
	if (!result.empty()) 
	{
		result.pop_back();
	}
	return result;
}

std::map<std::string, int> EnterInfo::words_count(const std::string& text)
{
	std::regex word_regex("[[:alpha:]]+");
	std::sregex_iterator it(text.begin(), text.end(), word_regex);

	std::map<std::string, int> word_counts;
	for (const std::string& word : arr_unique_words) 
	{
		word_counts[word] = 0;
	}

	while (it != std::sregex_iterator()) 
	{
		std::string word = it->str();
		if (word_counts.count(word)) {
			word_counts[word]++;
		}
		++it;
	}
	return word_counts;
}

void EnterInfo::getServerInfo()
{
	starting_page = makeLink(dataFromIniFileVector.at(5));
	max_recursion = std::stoi(dataFromIniFileVector.at(6));
	server_port_number = std::stoi(dataFromIniFileVector.at(7));
}

std::string EnterInfo::enterToDBString()
{
	std::string readyToEnter = dataFromIniFileVector.at(0) + " " +
		dataFromIniFileVector.at(1) + " " +
		dataFromIniFileVector.at(2) + " " +
		dataFromIniFileVector.at(3) + " " +
		dataFromIniFileVector.at(4);

	return readyToEnter;
}

std::vector<std::string> EnterInfo::getEnterInfo()
{
	if (file_path.is_open())
	{
		while (!file_path.eof())
		{
			std::string str;
			std::getline(file_path, str);
			dataFromIniFileVector.push_back(str);
		}
	}
	return dataFromIniFileVector;
}

//Destructor
EnterInfo::~EnterInfo()
{
	if (file_path.is_open())
	{
		file_path.close();
		std::cout << "File Closed!" << std::endl;
		std::cout << std::endl;
	}
}

Link EnterInfo::getStartingPage()
{
	return starting_page;
}

bool EnterInfo::createTable()
{
	conPQXX = std::make_shared<pqxx::connection>(enterToDBString());
	if (isEstablishConnection(conPQXX))
	{
		creatTable();
		return true;
	}
	else {
		return false;
	}
}

bool EnterInfo::insertDoc(std::string page)
{
	try
	{
		pqxx::work tx(*conPQXX);
		tx.exec("INSERT INTO documents(page_title) VALUES('" + page + "')");
		tx.commit();
		return true;
	}
	catch (const pqxx::unique_violation& e) {
		return false;
	}
}

bool EnterInfo::insertWord(std::string word)
{
	try
	{
		pqxx::work tx(*conPQXX);
		tx.exec("INSERT INTO words(word) VALUES('" + word + "')");
		tx.commit();
		return true;
	}
	catch (const pqxx::unique_violation& e) {
		return false;
	}
}

int EnterInfo::getDocId(std::string page)
{
	// Выполнение запроса
	pqxx::work w(*conPQXX);
	pqxx::result r = w.exec("SELECT id FROM documents WHERE page_title = '" + page + "'");

	// Получение ID
	if (r.size() == 1) {
		int id = r[0][0].as<int>();
		return id;
	}
	else {
		return 0;
	}

}

int EnterInfo::getWordId(std::string word)
{
	pqxx::work w(*conPQXX);
	pqxx::result r = w.exec("SELECT id FROM words WHERE word = '" + word + "'");

	if (r.size() == 1) {
		int id = r[0][0].as<int>();
		return id;
	}
	else {
		return 0;
	}
}

std::string EnterInfo::getDocNameById(int page_id)
{
	std::string sql = "SELECT page_title FROM documents WHERE id = ";
	sql += std::to_string(page_id);

	pqxx::work w(*conPQXX);
	pqxx::result r = w.exec(sql);

	if (r.size() == 1) {
		return r[0]["page_title"].as<std::string>();
	}
	else {
		return "An error has occurred, the id '" + std::to_string(page_id) + "' does not exist in the database";
	}
}

bool EnterInfo::insertWordNumData(int page, int word, int q)
{
	try
	{
		pqxx::work tx(*conPQXX);
		tx.exec("INSERT INTO documents_words(document_id, word_id, app_num) VALUES(" + std::to_string(page) + ", "
			"" + std::to_string(word) + ", " + std::to_string(q) + ")");
		tx.commit();
		return true;
	}
	catch (const pqxx::unique_violation& e) {
		return false;
	}
}

int EnterInfo::getWordRank(int page, int word)
{
	pqxx::work w(*conPQXX);
	std::string query = "SELECT app_num FROM documents_words "
		"WHERE document_id = " + std::to_string(page) + " AND word_id = " + std::to_string(word);
	pqxx::result r = w.exec(query);

	if (r.size() == 1) {
		int id = r[0][0].as<int>();
		return id;
	}
	else {
		return 0;
	}
}

void EnterInfo::insertWordVectorToDB(std::vector<std::string> v)
{
	bool noProblem = true;
	for (const auto& word : v) {
		if (!insertWord(word))
		{
			noProblem = false;
		}
	}
	if (noProblem == true) {
		std::cout << "All words added succesfully" << std::endl;
	}
}

void EnterInfo::insertMainData(std::string name, std::map<std::string, int> m)
{
	bool noProblem = true;
	int doc_id = getDocId(name);
	for (const auto& pair : m)
	{
		int word_id = getWordId(pair.first);
		if (insertWordNumData(doc_id, word_id, pair.second))
		{
			noProblem = false;
		}
	}
	if (noProblem == true) {
		std::cout << "All data added succesfully" << std::endl;
	}
}

std::vector<std::string> EnterInfo::filter_https(std::vector<std::string> urls)
{
	std::vector<std::string> filtered_urls;
	for (std::string url : urls) 
	{
		if (url.find("https:") == 0 || url.find("http:") == 0) 
		{
			filtered_urls.push_back(url);
		}
	}
	return filtered_urls;
}

std::vector<std::string> EnterInfo::remove_after_space(std::vector<std::string> words)
{
	std::vector<std::string> updated_words;

	for (std::string word : words) 
	{
		size_t pos = word.find(" ");

		if (pos != std::string::npos) 
		{
			updated_words.push_back(word.substr(0, pos));
		}
		else {
			updated_words.push_back(word);
		}
	}
	return updated_words;
}

std::vector<Link> EnterInfo::to_links(std::vector<std::string> urls)
{
	std::vector<Link> links;
	for (std::string url : urls)
	{
		links.push_back(makeLink(url));
	}
	return links;
}

Link EnterInfo::makeLink(std::string str)
{
	std::string protocol_str = str.substr(0, str.find(":"));
	std::string host_and_query = str.substr(str.find(":") + 3);

	ProtocolType protocol = ProtocolType::HTTP;
	if (protocol_str == "https") {
		protocol = ProtocolType::HTTPS;
	}

	size_t pos = host_and_query.find("/");
	std::string hostName = host_and_query.substr(0, pos);
	std::string query = host_and_query.substr(pos);

	return Link{ protocol, hostName, query };
}

std::vector<std::string> EnterInfo::getPageTitles()
{
	std::vector<std::string> words;
	pqxx::work tx(*conPQXX);
	pqxx::result result = tx.exec_params("SELECT page_title FROM documents");
	for (const auto& row : result) 
	{
		std::stringstream ss(row["page_title"].c_str());
		std::string word;

		while (ss >> word) {
			words.push_back(word);
		}
	}
	tx.commit();
	return words;
}

bool EnterInfo::isNewLinkFound(std::string page, std::vector<std::string> page_words)
{
	return std::find(page_words.begin(), page_words.end(), page) != page_words.end();
}

void EnterInfo::setDataToDB(std::string page_name, std::string html_data)
{
	if (insertDoc(page_name))
	{
		std::cout << "The new page "<< page_name <<" was added to DB." << std::endl;
	}
	std::string cleanText = getCleanText(html_data);
	arr_unique_words = remove_duplicates(cleanText);
	rank_words_ready_to_send = words_count(cleanText);

	insertWordVectorToDB(arr_unique_words);
	insertMainData(page_name, rank_words_ready_to_send);
}

std::vector<std::string> EnterInfo::getSearchResult(std::string search_str)
{
	std::vector<std::string> db_result = getReadySearchString(search_str);
	db_result = getOrderedListOfPages(db_result);
	if (db_result.size() > MAX_SEARCH_RESULTS) 
	{
		std::vector<std::string> result;
		std::copy_n(db_result.begin(), MAX_SEARCH_RESULTS, std::back_inserter(result));
		return result;
	}
	else {
		return db_result;
	}	
}

std::vector<Link> EnterInfo::extract_links(std::string html)
{
	std::regex link_regex("<a href=\"(.*?)\"", std::regex::icase);
	std::vector<std::string> old_links = getPageTitles();
	std::vector<std::string> new_links;

	std::smatch match;
	while (std::regex_search(html, match, link_regex) && new_links.size() < MAX_LINKS) 
	{
		std::string link = match[1].str();
		if (!isNewLinkFound(link, old_links))
		{
			new_links.push_back(link);
		}
		html.erase(match.position(), match.length());
	}

	new_links = filter_https(new_links);
	new_links = remove_after_space(new_links);

	return to_links(new_links);
}

std::string EnterInfo::getLinkPageName(Link link)
{
	std::string url;
	switch (link.protocol) 
	{
	case ProtocolType::HTTP:
		url += "http://";
		break;
	case ProtocolType::HTTPS:
		url += "https://";
		break;
	}
	url += link.hostName;
	if (!link.query.empty()) 
	{
		url += link.query;
	} else { 
		url += "/"; }

	return url;
}

int EnterInfo::getRecurtionDepth()
{
	return max_recursion;
}

unsigned short EnterInfo::getPortNumber()
{
	return server_port_number;
}

std::vector<std::string> EnterInfo::getReadySearchString(const std::string& str)
{
	std::vector<std::string> words = split_string(str);
	if (words.size() <= MAX_SEARCH_WORDS) { return words; }
	else
	{
		std::vector<std::string> participation_search_words;
		for (size_t i = 0; i < MAX_SEARCH_WORDS; ++i)
		{
			participation_search_words.push_back(words[i]);
		}
		return participation_search_words;
	}
}

std::vector<std::string> EnterInfo::getOrderedListOfPages(std::vector<std::string> v_search)
{
	std::vector<std::string> answer;
	std::vector<Page_Rank_Struct> rank_result_vector;
	std::vector<int>page_id = getAllPagesIdList();

	for (const auto& id : page_id)
	{
		int rank = 0;	
		for (const auto& word : v_search)
		{
			int word_id = getWordId(word);//Получили id слова
			rank += getWordRank(id, word_id);
		}
		if (rank > 0) 
		{
			rank_result_vector.push_back({ id, rank });
		}
		
	}

	std::stable_sort(rank_result_vector.begin(), rank_result_vector.end(),
		[](const Page_Rank_Struct& a, const Page_Rank_Struct& b) 
		{
			return a.value > b.value;
		});

	if (rank_result_vector.size() == 0) 
	{
		answer.push_back("NO_RESULTS");
		return answer;
	}
	else {

		for (const auto& pair : rank_result_vector) 
		{
			answer.push_back(getDocNameById(pair.id));
		}
		return answer;
	}

}
