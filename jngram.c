/*
 * ==============================================================
 *       Filename:  jngram.c
 *    Description:  ngram generator for json keywords
 *        Created:  2024-08-26
 *       Compiler:  gcc
 *         Author:  Y. Wang
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
	"\t-r      print raw keyword ngrams (default: formatted css code).\n";

#define MAX_STRING_LEN 200
#define MAX_KEYWORD_NUM 100 // for one book
#define MAX_KEYWORD_LEN 50
	// TODO: error handling on overstepping these limits

long get_mem_usage(){
	struct rusage myusage;
	getrusage(RUSAGE_SELF, &myusage);
	return myusage.ru_maxrss;
}

struct kwdata {
	char vkeyword[MAX_KEYWORD_NUM][MAX_KEYWORD_LEN];
	int nkeyword;
};

int keyword_exists (struct kwdata *keywords, char *new_keyword) {
	for (int i = 0; i < keywords->nkeyword; i++) {
		if (strcmp(keywords->vkeyword[i], new_keyword) == 0) {
			return 1; // keyword already exists
		}
	}
	return 0; // keyword does not exist
}

void add_keyword(struct kwdata *keywords, char *new_keyword) {
	strcpy(keywords->vkeyword[keywords->nkeyword], new_keyword);
	keywords->nkeyword++;
	/*if (keywords->nkeyword >= MAX_KEYWORD_NUM) {*/
	/*	printf("Keyword array is full, try making MAX_KEYWORD_NUM bigger. ");*/
	/*	return 1;*/
	/*} else if (strlen(new_keyword) > MAX_KEYWORD_LEN) {*/
	/*	printf("Keyword is too long, try making MAX_KEYWORD_LEN bigger. ");*/
	/*	return 2;*/
	/*}*/
	/*else {*/
	/*	strcpy(keywords->vkeyword[keywords->nkeyword], new_keyword);*/
	/*	keywords->nkeyword++;*/
	/*	return 0;*/
	/*}*/
}

void keyword_init(struct kwdata *keywords) {
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
				add_keyword(keywords, token);
		}
		token = strtok(NULL, delim);
	}
}

void keyword_css(const char* id, struct kwdata *keywords, long len) {
	const char *css1 = "input[value='";
	const char *css2 = "' i] ~ #booksearchresults #";
	const char *css3 = "{ display: list-item }";
	for (int i = 0; i < keywords->nkeyword; ++i) {
		char *kw = keywords->vkeyword[i];
		int kw_len = strlen(kw);
		for (int j = kw_len; j >= len; j--) {
			if (j == len && i == keywords->nkeyword-1)
				printf("%s%s%s%s\n%s\n",css1,kw,css2,id,css3);
			else
				printf("%s%s%s%s%s\n",css1,kw,css2,id,",");
			kw[j-1] = '\0';
		}
	}
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
	long min_keyword_len = 3;

	opterr = 0;
	for (int opt; (opt = getopt(argc, argv, "rl:")) != -1; ) {
		char *endptr;
		switch (opt) {
			case 'r':
				raw = true;
				break;
			case 'l':
				min_keyword_len = strtol(optarg, &endptr, 10);
				if (endptr[0] != '\0') {
					fprintf(stderr, "ngram: invalid flag value for -l\n\n%s", usage);
					return 1;
				}
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
	json_object *lcc;


	struct kwdata keywords = {{{0}},0};

	for (int k=0; k<nbooks; k++) {
		keyword_init(&keywords);
		jbook = json_object_array_get_idx(jbooks, k);

		title = json_object_object_get(jbook, "title");
		const char *title_str = json_object_get_string(title);
		keyword_gen(title_str,&keywords,min_keyword_len);

		subtitle = json_object_object_get(jbook, "subtitle");
		const char *subtitle_str = json_object_get_string(subtitle);
		if (subtitle_str[0] != '\0')
			keyword_gen(subtitle_str,&keywords,min_keyword_len);

		author = json_object_object_get(jbook, "author");
		const char *author_str = json_object_get_string(author);
		keyword_gen(author_str,&keywords,min_keyword_len);

		booktags = json_object_object_get(jbook, "booktags");
		int ntags = json_object_array_length(booktags);
		for (int i=0; i < ntags; i++) {
			const char *tag_str = json_object_get_string(json_object_array_get_idx(booktags, i));
			keyword_gen(tag_str,&keywords,min_keyword_len);
		}

		if (!raw) {
			lcc = json_object_object_get(jbook, "lccnumber");
			const char *lcc_str = json_object_get_string(lcc);
			keyword_css(lcc_str,&keywords,min_keyword_len);
		} else {
			for (int i = 0; i < keywords.nkeyword; ++i) {
				char *kw = keywords.vkeyword[i];
				int kw_len = strlen(kw);
				for (int j = kw_len; j >= min_keyword_len; j--) {
					printf("%s\n",kw);
					kw[j-1] = '\0';
				}
			}
		}

	}

	json_object_put(jbooks);

	return 0;
}
