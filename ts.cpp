#include <stdio.h>
#include <string.h>
#include <map>
#include <string>
#include <iostream>
#include <list>
#include <fstream>

#include "trie.h"

/* Define PrefixTree as Trie */
typedef TrieNode<char, uint32_t> PrefixTree;

/* Postings length type */
typedef uint32_t PSize;


class BulkLoader
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
		TrieNode<char, uint32_t>	trie;
		std::ofstream 				f("dict", std::ofstream::binary);
		char						nullstr[1] = {'\0'};

		for (auto const &p: words)
		{
			const std::list<uint32_t> &postings = p.second;
			PSize				psize = postings.size();

			/* Key parts */
			const std::string	&word = p.first;
			std::string			prefix = word.substr(0, 3);
			const char			*rest;
			uint16_t			rest_len;

			/* Put prefix to trie */
			trie.insert(prefix.c_str(), prefix.length(), (uint32_t) f.tellp());

			/*
			 * If word is long enough then take rest part of the word. Else
			 * it's an empty string
			 */
			rest = word.length() > 3 ? word.c_str() + 3 : nullstr;
			rest_len = word.length() > 3 ? word.length() - 3 : 0;

			/* Write word length and word itself */
			f.write((char *) &rest_len, sizeof(uint16_t));
			if (rest_len > 0)
				f.write(rest, rest_len * sizeof(char)); /* keep in mind that it won't be char in future */

			/* Write postings list */
			f.write((char *) &psize, sizeof(PSize));
			for (auto docId: postings)
				f.write((char *) &docId, sizeof(uint32_t));
		}

		trie.save();
	}
};

class Storage
{
private:
	PrefixTree *prefixes;
public:
	Storage()
	{
		/* Load prefix tree */
		prefixes = PrefixTree::load();
	}

	/*
	 * Retrieve postings array for the given key
	 */
	uint32_t find(std::string key, std::vector<uint32_t> &postings)
	{
		std::string		prefix = key.substr(0, 3);
		uint32_t		offset;

		if (prefixes->find(prefix.c_str(), offset))
		{
			std::ifstream	f("dict", std::ofstream::binary);
			uint16_t		buf_len;
			char 			buffer[32];
			PSize			psize;		/* postings vector size */

			f.seekg(offset);
			f.read((char *) &buf_len, sizeof(uint16_t));
			f.read(buffer, buf_len * sizeof(char));
			f.read((char *) &psize, sizeof(PSize));

			for (int i = 0; i < psize; i++)
			{
				uint32_t value;

				f.read((char *) &value, sizeof(uint32_t));
				postings.push_back(value);
			}

			return psize;
		}
		return 0;
	}
};

void
print_postings(Storage &s, const char *key)
{	
	std::vector<uint32_t> postings;

	s.find(key, postings);
	printf("postings: ");
	for (auto it = postings.begin(); it < postings.end(); it++)
		printf("%u, ", *it);
	printf("\n");
}

int
main()
{
	char *word;
	char phrase1[80];
	char phrase2[80];
	BulkLoader loader;

	strcpy(phrase1, "brown fox jumps over lazy dog");
	strcpy(phrase2, "brown dog saw a fox");

	loader.insert(phrase1, 0);
	loader.insert(phrase2, 1);
	loader.print();
	loader.write();

	Storage s;
	std::vector<uint32_t> postings;

	print_postings(s, "dog");
	print_postings(s, "saw");
	print_postings(s, "lazy");
}


