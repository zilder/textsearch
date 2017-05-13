#include <stdio.h>
#include <vector>
#include <fstream>

template<class Symbol, class Value>
class TrieNode
{
using Node = TrieNode<Symbol, Value>;

protected:
	std::vector<Symbol>	symbols;	/* sorted vector of symbols */
	std::vector<Node *>	nodes;		/* same order as symbols */
	std::vector<Value>	values;		/* same order as symbols */
	/* TODO: add nulls vector */

public:
	~TrieNode() { /* TODO */ }

	void insert(Symbol *string, size_t len, Value v)
	{
		int		i = 0;
		bool	exists = false;

		/* TODO: rewrite using binary search */
		for (; i < symbols.size(); i++)
		{
			if (string[0] <= symbols[i])
			{
				exists = (string[0] == symbols[i]);
				break;
			}
		}

		/* If node doesn't yes exist insert a new one */
		if (!exists)
		{
			//Node *child = new Node();

			symbols.insert(symbols.begin() + i, string[0]);
			nodes.insert(nodes.begin() + i, NULL);
			values.insert(values.begin() + i, 0);
		}

		/* Last symbol? Save value to this node and quit */
		if (len == 1)
		{
				values[i] = v;
				//this->value = v;
		}
		/* Else traverse further */
		else
		{
			if (!nodes[i])
				nodes[i] = new Node();
			nodes[i]->insert(string + 1, len - 1, v);
		}	
	}

	/*
	 * Find value by given key. Returns true if key was found and false
	 * otherwise
	 */
	bool find(Symbol *key, Value &value)
	{
		int		i = 0,
				j;
		bool	exists = false;
		size_t	key_len = strlen(key);
		Node	*cur,
				*next = this;

		while (i < key_len)
		{
			if (!next)	return false;
			cur = next;

			/* TODO: rewrite using binary search */
			for (j = 0; j < cur->symbols.size(); j++)
			{
				if (key[i] <= cur->symbols[j])
				{
					exists = (key[i] == cur->symbols[j]);
					break;
				}
			}

			/* If at some point we couldn't find key symbol then quit */
			if (!exists)
				return false;

			/* Get the next node */
			next = cur->nodes[j];
			i++;
		}

		value = cur->values[j];
		return true;
	}

	void save()
	{
		std::ofstream f("trie.pref", std::ofstream::binary);

		write_recursively(f, this);
		f.close();
	}

private:
	void write_recursively(std::ofstream &f, Node *node)
	{
		for (int i = 0; i < node->symbols.size(); i++)
		{
			/* TODO: what maximum number of children could be? */
			uint16_t nchildren = node->nodes[i] ?
									node->nodes[i]->symbols.size() : 0;

			f.write((char *) &node->symbols[i], sizeof(Symbol));
			f.write((char *) &node->values[i], sizeof(Value));
			f.write((char *) &nchildren, sizeof(uint16_t));

			if (node->nodes[i])
				write_recursively(f, node->nodes[i]);
		}
	}
};

int
main()
{
	TrieNode<char, uint32_t> trie;
	char word1[32];
	char word2[32];
	char word3[32];
	uint32_t value;

	printf("Trie test\n");

	strcpy(word1, "abc");
	strcpy(word2, "ass");
	strcpy(word3, "asd");
	trie.insert(word1, 3, 123);
	trie.insert(word2, 3, 456);

	if (trie.find(word1, value))
		printf("Yes! Value is: %u", value);
	else
		printf("Something's wrong");

	trie.save();
}

