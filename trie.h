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

	void insert(const Symbol *string, size_t len, Value v)
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
			symbols.insert(symbols.begin() + i, string[0]);
			nodes.insert(nodes.begin() + i, NULL);
			values.insert(values.begin() + i, 0);
		}

		/* Last symbol? Save value to this node and quit */
		if (len == 1)
		{
				values[i] = v;
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
	bool find(const Symbol *key, Value &value)
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

	static Node *load()
	{
		Node			*node;
		std::ifstream	f("trie.pref", std::ifstream::binary);

		node = read_recursively(f);
		f.close();

		return node;
	}

private:
	void write_recursively(std::ofstream &f, Node *node)
	{
		uint16_t	nelems = node->symbols.size();

		/* Write number of elements first */
		f.write((char *) &nelems, sizeof(uint16_t));

		for (int i = 0; i < node->symbols.size(); i++)
		{
			/* TODO: what maximum number of children could be? */
			uint16_t nchildren = node->nodes[i] ?
									node->nodes[i]->symbols.size() : 0;

			f.write((char *) &node->symbols[i], sizeof(Symbol));
			f.write((char *) &node->values[i], sizeof(Value));

			if (node->nodes[i])
				write_recursively(f, node->nodes[i]);
			else
			{
				/* If there are no children */
				uint16_t zero = 0;
				f.write((char *) &zero, sizeof(uint16_t));
			}
		}
	}

	static Node *read_recursively(std::ifstream &f)
	{
		uint16_t	nelems;
		Node		*node;
		
		f.read((char *) &nelems, sizeof(uint16_t));

		if (nelems == 0)
			return NULL;

		node = new Node();
		for (int i = 0; i < nelems; i++)
		{
			Symbol	s;
			Value	v;
			Node    *n;

			f.read((char *) &s, sizeof(Symbol));
			f.read((char *) &v, sizeof(Value));

			node->symbols.push_back(s);
			node->values.push_back(v);
			node->nodes.push_back(read_recursively(f));
		}

		return node;
	}
};
