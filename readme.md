# jngram

Extracting ngrams from json metadata keywords and producing CSS codes for a CSS-based search engine, see [this](https://gregw.xyz/posts/catalog-your-books/) and [this](https://gregw.xyz/posts/css-search-engine/) blog posts.

See a demo [here](https://gregw.xyz/books/).

# Compile

You need [json-c](https://github.com/json-c/json-c) installed and ready to use:

```sh
gcc -I/usr/include/json-c/ jngram.c -ljson-c -o jngram
```

# Usage

```txt
Usage: jngram [flags] filename

Flags:
        -l num  minimal length of search keywords (default: 3);
        -r      print raw keyword ngrams (default: formatted css code);
        -a      print all ngrams (default: ngrams that include the first letter).
```

# Example

An example json file, `library.json`:

```json
[
	{
		"title": "The Four Loves",
		"subtitle": "",
		"publishyear": "1960",
		"publisher": "HarperOne",
		"author": "C. S. Lewis",
		"lccnumber": "BV4639-L45-2017",
		"_comment": "BV4639 .L45 2017",
		"booktags": [
			"cslewis"
		]
	}
]
```

**By default, ngrams always include the first letter.**

## Formatted CSS codes:

```sh
./jngram library.json
```

```css
#booksearchresults li { display: none }
input[value='the' i] ~ #booksearchresults #BV4639-L45-2017,
input[value='fou' i] ~ #booksearchresults #BV4639-L45-2017,
input[value='four' i] ~ #booksearchresults #BV4639-L45-2017,
input[value='lov' i] ~ #booksearchresults #BV4639-L45-2017,
input[value='love' i] ~ #booksearchresults #BV4639-L45-2017,
input[value='loves' i] ~ #booksearchresults #BV4639-L45-2017,
input[value='lew' i] ~ #booksearchresults #BV4639-L45-2017,
input[value='lewi' i] ~ #booksearchresults #BV4639-L45-2017,
input[value='lewis' i] ~ #booksearchresults #BV4639-L45-2017,
input[value='csl' i] ~ #booksearchresults #BV4639-L45-2017,
input[value='csle' i] ~ #booksearchresults #BV4639-L45-2017,
input[value='cslew' i] ~ #booksearchresults #BV4639-L45-2017,
input[value='cslewi' i] ~ #booksearchresults #BV4639-L45-2017,
input[value='cslewis' i] ~ #booksearchresults #BV4639-L45-2017 { display: list-item }
```

## Raw ngrams:

```sh
./jngram -r library.json
```

```txt
the
fou
four
lov
love
loves
lew
lewi
lewis
csl
csle
cslew
cslewi
cslewis
```

## Raw all ngrams:

```sh
./jngram -ra library.json
```

```txt
the
fou
our
four
lov
...
(middle part omitted)
...
cslew
slewi
lewis
cslewi
slewis
cslewis
```
