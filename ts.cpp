#include <stdio.h>
#include <string.h>
#include <map>
#include <string>
#include <iostream>
#include <list>
#include <fstream>
#include <algorithm>

#include "trie.h"

/* Define PrefixTree as Trie */
typedef TrieNode<char, uint32_t> PrefixTree;

/* Postings length type */
typedef uint32_t PSize;

#define MAX_PREFIX_LEN	3


typedef struct
{
	uint16_t	separator : 1;	/* Is record a separator? */
	uint16_t	length : 15;	/* String length */
} DictRecord;

#define InitSeparator(r)	\
	do {					\
		(r).separator = 1;	\
		(r).length = 0;		\
	} while (0)

#define InitDictRecord(r, l)	\
	do {					\
		(r).separator = 0;	\
		(r).length = (l);	\
	} while (0)


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

	void write(const char *name)
	{
		TrieNode<char, uint32_t>	trie;
		char						filename[32];
		std::ofstream 				fdict;	/* dictionary file */
		std::ofstream				fpost;	/* postings file */
		char						nullstr[1] = {'\0'};
		char						prefix[MAX_PREFIX_LEN] = {'\0'};
		bool						prefix_changed = false;
		uint8_t						prefix_len;

		sprintf(filename, "%s.dict", name);
		fdict.open(filename, std::ofstream::binary);
		sprintf(filename, "%s.post", name);
		fpost.open(filename, std::ofstream::binary);

		for (auto const &p: words)
		{
			const std::list<uint32_t> &postings = p.second;
			PSize				psize = postings.size();

			/* Key parts */
			const std::string	&word = p.first;
			const char			*rest;
			uint16_t			rest_len;
			DictRecord			record;
			size_t				poffset;

			/*
			 * Do we need to put another prefix into trie? And if we do
			 * then what prefix should it be and what is its length?
			 */
			prefix_changed = false;
			prefix_len = MAX_PREFIX_LEN;
			for (uint8_t i = 0; i < MAX_PREFIX_LEN; i++)
			{
				char cur = (i < word.length()) ? word[i] : '\0';

				if (cur != prefix[i])
					prefix_changed = true;
				prefix[i] = cur;

				if (i >= word.length())
					prefix_len = std::min(prefix_len, i);
			}

			/* Put prefix to trie */
			if (prefix_changed)
			{
				// uint32_t	zero;

				/*
				 * Write a separator - zero length word with zero length
				 * postings list
				 */
				// fdict.write((char *) &zero, sizeof(uint16_t));
				// fdict.write((char *) &zero, sizeof(PSize));
				InitSeparator(record);
				fdict.write((char *) &record, sizeof(DictRecord));

				/* Put prefix to the trie */
				trie.insert(prefix, prefix_len, (uint32_t) fdict.tellp());
			}

			/*
			 * If word is long enough then take rest part of the word. Else
			 * it's an empty string
			 */
			if (prefix_len < MAX_PREFIX_LEN)
			{
				rest = nullstr;
				rest_len = 0;
			}
			else
			{
				rest = word.c_str() + 3;
				rest_len = word.length() - MAX_PREFIX_LEN;
			}

			/* Write word and offset in postings list*/
			InitDictRecord(record, rest_len);
			// fdict.write((char *) &rest_len, sizeof(uint16_t));
			fdict.write((char *) &record, sizeof(DictRecord));
			if (rest_len > 0)
				fdict.write(rest, rest_len * sizeof(char)); /* keep in mind that it won't be char in future */
			poffset = fpost.tellp();
			fdict.write((char *) &poffset, sizeof(size_t));

			/* Write postings list */
			fpost.write((char *) &psize, sizeof(PSize));
			for (auto docId: postings)
				fpost.write((char *) &docId, sizeof(uint32_t));
		}

		trie.save(name);
	}
};

class Storage
{
private:
	PrefixTree	*prefixes;
	char		*name;
public:
	Storage(const char *storage_name)
	{
		name = (char *) malloc(strlen(storage_name));	/* name + .dict */
		strcpy(name, storage_name);

		/* Load prefix tree */
		prefixes = PrefixTree::load(storage_name);
	}

	~Storage()
	{
		free(name);
	}

	/*
	 * Retrieve postings array for the given key
	 */
	uint32_t find(std::string key, std::vector<uint32_t> &postings)
	{
		char			filename[32];
		std::string		prefix = key.substr(0, MAX_PREFIX_LEN);
		uint32_t		dict_offset,
						post_offset;

		if (prefixes->find(prefix.c_str(), dict_offset))
		{
			/* TODO: should we open file in the constructor? */
			std::ifstream	fdict,
							fpost;
			// uint16_t		buf_len;
			char 			buffer[32]; /* TODO */
			PSize			psize;		/* postings vector size */
			DictRecord		record;

			sprintf(filename, "%s.dict", this->name);
			fdict.open(filename, std::ofstream::binary);

			sprintf(filename, "%s.post", this->name);
			fpost.open(filename, std::ofstream::binary);

			fdict.seekg(dict_offset);
			// f.read((char *) &buf_len, sizeof(uint16_t));
			fdict.read((char *) &record, sizeof(DictRecord));
			fdict.read(buffer, record.length * sizeof(char));
			fdict.read((char *) &post_offset, sizeof(uint32_t));

			fpost.seekg(post_offset);
			fpost.read((char *) &psize, sizeof(PSize));

			for (int i = 0; i < psize; i++)
			{
				uint32_t value;

				fpost.read((char *) &value, sizeof(uint32_t));
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
	loader.write("123");

	Storage s("123");
	std::vector<uint32_t> postings;

	print_postings(s, "dog");
	print_postings(s, "saw");
	print_postings(s, "lazy");
}


