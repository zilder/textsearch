#include <cassert>
#include <fstream>
#include <stdio.h>
#include <vector>

#define DEBUG 1


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
		int		pos = 0;
		bool	exists = false;

		/* Find key position */
		pos = bsearch(symbols, string[0], &exists);

		/* If node doesn't yes exist insert a new one */
		if (!exists)
		{
			symbols.insert(symbols.begin() + pos, string[0]);
			nodes.insert(nodes.begin() + pos, NULL);
			values.insert(values.begin() + pos, 0);

			#ifdef DEBUG
			debug_check_order();
			#endif
		}

		/* Last symbol? Save value to this node and quit */
		if (len == 1)
		{
				values[pos] = v;
		}
		/* Else traverse further */
		else
		{
			if (!nodes[pos])
				nodes[pos] = new Node();

			/* TODO: replace recursion with loop */
			nodes[pos]->insert(string + 1, len - 1, v);
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
		bool	exists;
		size_t	key_len = strlen(key);
		Node	*cur,
				*next = this;

		while (i < key_len)
		{
			if (!next)	return false;
			cur = next;

			/* If at some point we couldn't find key symbol then quit */
			pos = bsearch(cur->symbols, key[i], &exists);
			if (!exists)
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
	static int bsearch(const std::vector<Symbol> &symbols,
					   const Symbol &val,
					   bool *found)
	{
		int		start = 0,
				end = symbols.size() - 1,
				pos;

		/* Trivial case */
		if (symbols.empty())
		{
			*found = false;
			return 0;
		}

		/* Start binary search */
		while (start <= end)
		{
			pos = start + (end - start) / 2;

			/* Found? */
			if (symbols[pos] == val)
			{
				*found = true;
				return pos;
			}

			/* Final case, quit */
			if (start == end)
				break;

			/* Shift boundaries */
			if (val > symbols[pos])
				start = pos + 1;
			else
				end = pos - 1;
		}

		/*
		 * We didn't find key, but at least we could determine position
		 * where it should go. Here are two options: either key is less than
		 * symbols[pos] meaning 'pos' is right position to insert new node;
		 * or key is greater than symbols[pos] meaning that we should insert
		 * new node right after it
		 */
		if (val > symbols[pos])
			pos++;

		*found = false;
		return pos;
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

	/* Check ordering */
	void debug_check_order()
	{
		if (symbols.size() > 1)
		{
			for (int i = 1; i < symbols.size(); i++)
				assert(symbols[i] > symbols[i - 1]);
		}
	}
};
