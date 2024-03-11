#pragma once

#include <fstream>
#include <string>
#include <vector>
#include <iostream>
#include <algorithm>
#include <iterator>
#include <set>
#include <map>
#include <sstream>
#include <regex>
#include <boost/locale.hpp>
#include "pqxx/pqxx"
#include "spider/link.h"




class EnterInfo
{
public:

	struct Page_Rank_Struct
	{
		int id;
		int value;
	};



	//Constructor
	EnterInfo(const std::string& _file);
	EnterInfo() = delete;
	//Destructor
	~EnterInfo();
	
	Link getStartingPage();//Возвращает стартовую страницу с которой начитначинается поиск
	bool createTable();//Создает подключение к БД и при необходимости создает таблицу

	//Обрабытывает html данные страницы и отправляет в БД 
	void setDataToDB(std::string page_name, std::string html_data);
	//Возвращает вектор страниц по убыванию по результатам поиска ранжирования слов 
	std::vector<std::string> getSearchResult(std::string search_str);

	//вытаскивает ссылки из html данных и возвращает вектор ссылок
	std::vector<Link> extract_links(std::string html);

	std::string getLinkPageName(Link link);//Переводит объект Link в строку

	int getRecurtionDepth();//Возвращает глубину рекурсии
	unsigned short getPortNumber();

private:
	//Переменные
	std::ifstream file_path;
	std::string fileName;
	Link starting_page;//Ссылка на стартовую страницу
	int max_recursion = 0;//Переменная глубины рекурсии
	std::shared_ptr<pqxx::connection> conPQXX; //Указатель на переменную подключения к БД
	std::vector <std::string> dataFromIniFileVector; //Вектор хранит данные из файла
	const int MAX_LINKS = 100; //Максимальное кол-во  ссылок, которые можно получить 
	const int MAX_SEARCH_WORDS = 4;//Максимальное кол-во слов учавствующих в поиске
	const int MAX_SEARCH_RESULTS = 10;//Максимальное кол-во страниц выводимых на экран 
	unsigned short server_port_number = 1111; //Порт для сервера
	std::map<std::string, int> rank_words_ready_to_send;

	//Методы обработки файлов
	void getServerInfo();//Получает данные из файла из конструктора класса
	std::string enterToDBString(); //Создает необходимую строку для связи с БД
	std::vector<std::string> getEnterInfo(); //Собирает в вектор данные из файла

	//Методы для работы с БД
	bool isEstablishConnection(std::shared_ptr<pqxx::connection> c);//Возвращает true если есть связь с БД
	void creatTable(); //Создаем таблицы в БД
	std::vector<int> getAllPagesIdList();//Возращает все существующие id в таблице documents
	int getWordRank(int page, int word);//Возвращает сколько раз встретилось слово на странице
	int getDocId(std::string page);//Возвращает id номер страницы в таблице или 0 если такой страницы нет
	int getWordId(std::string word);//Возвращает id номер слова в таблице или 0 если такого слова нет
	std::string getDocNameById(int page_id);//Возвращает название страницы по id
	bool insertDoc(std::string page);//Добавляет новые страницы
	bool insertWord(std::string word);//Добавляет новые слова
	bool insertWordNumData(int page, int word, int q);//Добавляет кол-во раз когда слово встречается на странице
	void insertWordVectorToDB(std::vector<std::string> v);//Добавляем слова в таблицу Words
	void insertMainData(std::string name, std::map<std::string, int> m);//Добавляет данные в таблицу documents_words
	std::vector<std::string> getPageTitles();//Возвращает вектор всех записанных страниц

	//Технические методы 
	std::vector<std::string> split_string(const std::string& str);//Разбивает строку в вектор слов
	std::string remove_html_tags(const std::string& html);//Отчищает строку от HTML-тегов
	std::string remove_punc(const std::string& text);//Отчищает строку от знаков препинания
	std::string remove_whitespace(const std::string& text);//Отчищает текст от пустых строк
	std::string make_lower_case(const std::string& text);//Переводит строку в нижний регистр
	std::vector<std::string> remove_duplicates(const std::string& text);//Удаление дубликатов в тексте
	std::string getCleanText(const std::string& text);//Отчищает строку от от ненужных символов
	bool is_valid_word(const std::string& word);//Проверка на валидность слов в тексте
	std::string get_ready_to_rank_text(const std::string& text);
	//Содает std::map со словом и кол-во повторений этого слова в тексте
	std::map<std::string, int> words_count(const std::string& text);
	std::vector<std::string> arr_unique_words;//Вектор уникальных слов готовый для добавления в БД
	std::vector<std::string> getReadySearchString(const std::string& str);//Готовит строку поиска
	//Выдает вектор с отсортированным названием страниц
	std::vector<std::string> getOrderedListOfPages(std::vector<std::string> v_search);
	// Отфильтровывает URL-адреса из вектора
	std::vector<std::string> filter_https(std::vector<std::string> urls);
	//Удаляет все слова после пробела
	std::vector<std::string> remove_after_space(std::vector<std::string> words);
	//Превращает вектор строк в вектор объектов Link
	std::vector<Link> to_links(std::vector<std::string> urls);
	//Превращает строку в объект Link
	Link makeLink(std::string str);
	//Возвращает true если ссылка на страницу уже добавлена в таблицу
	bool isNewLinkFound(std::string page, std::vector<std::string> page_words);

};
