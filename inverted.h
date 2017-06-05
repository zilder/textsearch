#include <list>
#include <map>
#include <string.h>

#include "trie.h"

/* Define PrefixTree as Trie */
typedef TrieNode<char, uint32_t> PrefixTree;

/* Postings length type */
typedef uint32_t PSize;



class BulkLoader
{
	using Map = std::map<std::string, std::list<uint32_t> >;
private:
	Map		words;

public:
	void insert(char *text, uint32_t docId);
	void print();
	void write(const char *name);
};

class InvertedIndex
{
private:
	bool			initialized;
	std::ifstream	dict_file;
	std::ifstream	post_file;
	PrefixTree	   *prefixes;
	char		   *name;

private:
	void init();

public:
	InvertedIndex(const char *storage_name);
	~InvertedIndex();
	uint32_t find(std::string key, std::vector<uint32_t> &postings);
};
