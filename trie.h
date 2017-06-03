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
				pos;
		bool	exists = false;
		size_t	key_len = strlen(key);
		Node	*cur,
				*next = this;

		while (i < key_len)
		{
			if (!next)	return false;
			cur = next;

			/* If at some point we couldn't find key symbol then quit */
			if ((pos = bsearch(cur->symbols, key[i])) < 0)
				return false;

			/* Get the next node */
			next = cur->nodes[pos];
			i++;
		}

		value = cur->values[pos];
		return true;
	}

	void save(const char *name)
	{
		char			fname[32];
		std::ofstream	f;

		sprintf(fname, "%s.pref", name);
		f.open(fname, std::ofstream::binary);

		write_recursively(f, this);
		f.close();
	}

	static Node *load(const char *name)
	{
		Node			*node;
		char			fname[32];
		std::ifstream	f;

		sprintf(fname, "%s.pref", name);
		f.open(fname, std::ifstream::binary);

		node = read_recursively(f);
		f.close();

		return node;
	}

private:
	static int bsearch(std::vector<Symbol> &symbols, const Symbol &val)
	{
		int		start = 0,
				end = symbols.size(),
				pos;

		while (start < end)
		{
			pos = (end - start) / 2;

			/* Found? */
			if (symbols[pos] == val)
				return pos;

			/* Final case, quit */
			if (start == end)
				break;

			/* Shift boundaries */
			if (symbols[pos] > val)
				start = pos + 1;
			else
				end = pos - 1;
		}

		return -1;
	}

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
