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
	
	Link getStartingPage();//���������� ��������� �������� � ������� ��������������� �����
	bool createTable();//������� ����������� � �� � ��� ������������� ������� �������

	//������������ html ������ �������� � ���������� � �� 
	void setDataToDB(std::string page_name, std::string html_data);
	//���������� ������ ������� �� �������� �� ����������� ������ ������������ ���� 
	std::vector<std::string> getSearchResult(std::string search_str);

	//����������� ������ �� html ������ � ���������� ������ ������
	std::vector<Link> extract_links(std::string html);

	std::string getLinkPageName(Link link);//��������� ������ Link � ������

	int getRecurtionDepth();//���������� ������� ��������
	unsigned short getPortNumber();

private:
	//����������
	std::ifstream file_path;
	std::string fileName;
	Link starting_page;//������ �� ��������� ��������
	int max_recursion = 0;//���������� ������� ��������
	std::shared_ptr<pqxx::connection> conPQXX; //��������� �� ���������� ����������� � ��
	std::vector <std::string> dataFromIniFileVector; //������ ������ ������ �� �����
	const int MAX_LINKS = 100; //������������ ���-��  ������, ������� ����� �������� 
	const int MAX_SEARCH_WORDS = 4;//������������ ���-�� ���� ������������ � ������
	const int MAX_SEARCH_RESULTS = 10;//������������ ���-�� ������� ��������� �� ����� 
	unsigned short server_port_number = 1111; //���� ��� �������
	std::map<std::string, int> rank_words_ready_to_send;

	//������ ��������� ������
	void getServerInfo();//�������� ������ �� ����� �� ������������ ������
	std::string enterToDBString(); //������� ����������� ������ ��� ����� � ��
	std::vector<std::string> getEnterInfo(); //�������� � ������ ������ �� �����

	//������ ��� ������ � ��
	bool isEstablishConnection(std::shared_ptr<pqxx::connection> c);//���������� true ���� ���� ����� � ��
	void creatTable(); //������� ������� � ��
	std::vector<int> getAllPagesIdList();//��������� ��� ������������ id � ������� documents
	int getWordRank(int page, int word);//���������� ������� ��� ����������� ����� �� ��������
	int getDocId(std::string page);//���������� id ����� �������� � ������� ��� 0 ���� ����� �������� ���
	int getWordId(std::string word);//���������� id ����� ����� � ������� ��� 0 ���� ������ ����� ���
	std::string getDocNameById(int page_id);//���������� �������� �������� �� id
	bool insertDoc(std::string page);//��������� ����� ��������
	bool insertWord(std::string word);//��������� ����� �����
	bool insertWordNumData(int page, int word, int q);//��������� ���-�� ��� ����� ����� ����������� �� ��������
	void insertWordVectorToDB(std::vector<std::string> v);//��������� ����� � ������� Words
	void insertMainData(std::string name, std::map<std::string, int> m);//��������� ������ � ������� documents_words
	std::vector<std::string> getPageTitles();//���������� ������ ���� ���������� �������

	//����������� ������ 
	std::vector<std::string> split_string(const std::string& str);//��������� ������ � ������ ����
	std::string remove_html_tags(const std::string& html);//�������� ������ �� HTML-�����
	std::string remove_punc(const std::string& text);//�������� ������ �� ������ ����������
	std::string remove_whitespace(const std::string& text);//�������� ����� �� ������ �����
	std::string make_lower_case(const std::string& text);//��������� ������ � ������ �������
	std::vector<std::string> remove_duplicates(const std::string& text);//�������� ���������� � ������
	std::string getCleanText(const std::string& text);//�������� ������ �� �� �������� ��������
	bool is_valid_word(const std::string& word);//�������� �� ���������� ���� � ������
	std::string get_ready_to_rank_text(const std::string& text);
	//������ std::map �� ������ � ���-�� ���������� ����� ����� � ������
	std::map<std::string, int> words_count(const std::string& text);
	std::vector<std::string> arr_unique_words;//������ ���������� ���� ������� ��� ���������� � ��
	std::vector<std::string> getReadySearchString(const std::string& str);//������� ������ ������
	//������ ������ � ��������������� ��������� �������
	std::vector<std::string> getOrderedListOfPages(std::vector<std::string> v_search);
	// ��������������� URL-������ �� �������
	std::vector<std::string> filter_https(std::vector<std::string> urls);
	//������� ��� ����� ����� �������
	std::vector<std::string> remove_after_space(std::vector<std::string> words);
	//���������� ������ ����� � ������ �������� Link
	std::vector<Link> to_links(std::vector<std::string> urls);
	//���������� ������ � ������ Link
	Link makeLink(std::string str);
	//���������� true ���� ������ �� �������� ��� ��������� � �������
	bool isNewLinkFound(std::string page, std::vector<std::string> page_words);

};
