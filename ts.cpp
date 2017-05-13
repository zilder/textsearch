#include <stdio.h>
#include <string.h>

#include <map>
#include <string>
#include <iostream>
#include <list>
#include <fstream>

//void insert(Dictionary &dict, char *text, uint32_t docId);


class Dictionary
{
	using Map = std::map<std::string, std::list<uint32_t> >;
private:
	Map words;
public:
	void insert(char *text, uint32_t docId)
	{
		char *saved;
		char *word;

		word = strtok_r(text, " ", &saved);
		while (word)
		{
			words[word].push_back(docId);
			word = strtok_r(NULL, " ", &saved);
		}
	}

	void print()
	{
		for (auto const &p: words)
		{
			std::cout << p.first << ": [";
			for (auto docId: p.second)
			{
				std::cout << docId << ',';
			}
			std::cout << ']' << std::endl;
		}
	}

	void write()
	{
		std::ofstream outfile("dict", std::ofstream::binary);

		for (auto const &p: words)
		{
			const std::string &word = p.first;
			const std::list<uint32_t> &postings = p.second;
			uint16_t word_len = word.length();
			uint32_t postings_len = postings.size();
			size_t buflen = sizeof(word_len) +
					word_len * sizeof(char) + 
					sizeof(postings_len) +
					postings_len * sizeof(uint32_t);
			char buffer[buflen];
			char *cur = buffer;

			/* Write word length and word itself */
			memcpy(cur, (char *) &word_len, sizeof(uint16_t));
			cur += sizeof(uint16_t);
			memcpy(cur, word.c_str(), word.length());
			cur += word.length();

			/* Write postings list */
			memcpy(cur, (char *) &postings_len, sizeof(uint32_t));
			cur += sizeof(uint32_t);
			for (auto docId: postings)
			{
				memcpy(cur, (char *) &docId, sizeof(uint32_t));
				cur += sizeof(uint32_t);
			}
			outfile.write(buffer, buflen);
		}	
	}
};

int
main()
{
	char *word;
	char phrase1[80];
	char phrase2[80];
	Dictionary dict;

	strcpy(phrase1, "brown fox jumps over lazy dog");
	strcpy(phrase2, "brown dog saw a fox");

	/*
	word = strtok_r(phrase, " ", &saved);
	while (word)
	{
		dict[word] = 0;
		word = strtok_r(NULL, " ", &saved);
	}
	*/
	dict.insert(phrase1, 0);
	dict.insert(phrase2, 1);
	dict.print();
	dict.write();

	/*
	for (auto const &p: dict)
	{
		std::cout << p.first << ": [";
		for (auto docId: p.second)
		{
			std::cout << docId << ',';
		}
		std::cout << ']' << std::endl;
	}
	*/
}


