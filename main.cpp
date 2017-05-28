#include <stdio.h>

#include "inverted.h"


void
print_postings(InvertedIndex &s, const char *key)
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

	// strcpy(phrase1, "brown fox jumps over lazy dog");
	// strcpy(phrase2, "brown dog saw a fox");
	strcpy(phrase1, "cannot predict this");
	strcpy(phrase2, "i would prefer");

	loader.insert(phrase1, 0);
	loader.insert(phrase2, 1);
	loader.print();
	loader.write("123");

	InvertedIndex s("123");
	std::vector<uint32_t> postings;

	// print_postings(s, "dog");
	// print_postings(s, "saw");
	// print_postings(s, "lazy");
	print_postings(s, "predict");
	print_postings(s, "prefer");
}


