/*
 * ==============================================================
 *       Filename:  jngram.c
 *    Description:  ngram generator for json keywords
 *        Created:  2024-08-26
 *       Compiler:  gcc
 *         Author:  Y. WANG
 * ==============================================================
 */
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <json.h>
#include <sys/resource.h>

const char *usage = "Usage: jngram [flags] filename\n"
	"\n"
	"Flags:\n"
	"\t-l num  minimal length of search keywords (default: 3);\n"
	"\t-r      print raw keyword ngrams (default: formatted css code);\n"
	"\t-a      print all ngrams (default: ngrams that include the first letter).\n";

#define MAX_STRING_LEN 200
#define MAX_KEYWORD_NUM 100 // for one book
#define MAX_KEYWORD_LEN 50
	// TODO: error handling on overstepping these limits

struct kwdata {
	char vkeyword[MAX_KEYWORD_NUM][MAX_KEYWORD_LEN];
	int nkeyword;
};

int keyword_exists (struct kwdata *keywords, char *new_keyword) {
	for (int i = 0; i < keywords->nkeyword; i++) {
		if (strcmp(keywords->vkeyword[i], new_keyword) == 0) {
			return 1;
		}
	}
	return 0;
}

void keyword_add(struct kwdata *keywords, char *new_keyword) {
	strcpy(keywords->vkeyword[keywords->nkeyword], new_keyword);
	keywords->nkeyword++;
}

void keywords_init(struct kwdata *keywords) {
	memset(keywords->vkeyword, 0, sizeof(keywords->vkeyword));
	keywords->nkeyword = 0;
}

void keyword_gen(const char* str, struct kwdata *keywords, long len){

	char delim[] = " '-(),.0123456789";

	char srcstr[MAX_STRING_LEN];
	strcpy(srcstr, str);

	char *token = strtok(srcstr, delim);
	while(token) {
		int tok_len = strlen(token);
		if (tok_len >= len) {
			for (int i = 0; i < tok_len; ++i) {
				token[i] = tolower((unsigned char)token[i]);
			}
			if (!keyword_exists(keywords, token))
				keyword_add(keywords, token);
		}
		token = strtok(NULL, delim);
	}
}

char *gram_gen(char *keyword, int pos, int len)
{
   char *gram;
   int c;
   gram = malloc(len+1);
   for (c = 0; c < len; c++)
   {
      *(gram+c) = *(keyword+pos);      
      keyword++;  
   }
   *(gram+c) = '\0';
   return gram;
}

int file_exists(const char *fname)
{
    FILE *file;
    if ((file = fopen(fname, "r")))
    {
        fclose(file);
        return 1;
    }
    return 0;
}

int main(int argc, char *argv[])
{
	bool raw = false;
	bool all = false;
	long min_gram_len = 3;

	opterr = 0;
	for (int opt; (opt = getopt(argc, argv, "l:ra")) != -1; ) {
		char *endptr;
		switch (opt) {
			case 'l':
				min_gram_len = strtol(optarg, &endptr, 10);
				if (endptr[0] != '\0') {
					fprintf(stderr, "ngram: invalid flag value for -l\n\n%s", usage);
					return 1;
				}
				break;
			case 'r':
				raw = true;
				break;
			case 'a':
				all = true;
				break;
			case '?':
				fprintf(stderr, "ngram: invalid flag -%c\n\n%s", optopt, usage);
				return 1;
		}
	}

	if (argc == optind) {
		fprintf(stderr, "ngram: json file required as argument \n\n%s", usage);
		return 1;
	} else {
		if(!file_exists(argv[optind])) {
			fprintf(stderr, "ngram: file \"%s\" does not exist\n", argv[optind]);
			return 1;
		}
	}

	if (!raw)
		printf("#booksearchresults li { display: none }\n");

	json_object *jbooks = json_object_from_file(argv[optind]);
	int nbooks = json_object_array_length(jbooks);

	json_object *jbook;
	json_object *title;
	json_object *subtitle;
	json_object *author;
	json_object *booktags;

	struct kwdata keywords = {{{0}},0};

	for (int k=0; k<nbooks; k++) {
		keywords_init(&keywords);
		jbook = json_object_array_get_idx(jbooks, k);

		title = json_object_object_get(jbook, "title");
		const char *title_str = json_object_get_string(title);
		keyword_gen(title_str,&keywords,min_gram_len);

		subtitle = json_object_object_get(jbook, "subtitle");
		const char *subtitle_str = json_object_get_string(subtitle);
		if (subtitle_str[0] != '\0')
			keyword_gen(subtitle_str,&keywords,min_gram_len);

		author = json_object_object_get(jbook, "author");
		const char *author_str = json_object_get_string(author);
		keyword_gen(author_str,&keywords,min_gram_len);

		booktags = json_object_object_get(jbook, "booktags");
		int ntags = json_object_array_length(booktags);
		for (int i=0; i < ntags; i++) {
			const char *tag_str = json_object_get_string(json_object_array_get_idx(booktags, i));
			keyword_gen(tag_str,&keywords,min_gram_len);
		}

		const char *cssid = raw ? "" : json_object_get_string(json_object_object_get(jbook, "lccnumber"));
		const char *css1 = raw ? "" : "input[value='";
		const char *css2 = raw ? "" : "' i] ~ #booksearchresults #";
		const char *css3 = raw ? "" : ",";
		const char *css4 = raw ? "" : "{ display: list-item }";

		for (int i = 0; i < keywords.nkeyword; ++i) {
			char *kw = keywords.vkeyword[i];
			char *gram;
			int kw_len = strlen(kw);
			int pos = 0, gram_len = min_gram_len;
			while (gram_len <= kw_len) {
				while (pos + gram_len <= kw_len) {
					gram = gram_gen(kw, pos, gram_len);
					if (gram_len == kw_len && i == keywords.nkeyword-1)
						printf("%s%s%s%s%s\n",css1,gram,css2,cssid,css4);
					else
						printf("%s%s%s%s%s\n",css1,gram,css2,cssid,css3);
					free(gram);
					if (!all)
						break;
					pos++;
				}
				pos = 0;
				gram_len++;
			}

		}

	}

	json_object_put(jbooks);

	return 0;
}
