#pragma once

// jsminbuf is basically a rip-off of Douglas Crockford's work. License 
// and credits therefore see below.

/* jsmin.c
2013-03-29

Copyright (c) 2002 Douglas Crockford  (www.crockford.com)

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

The Software shall be used for Good, not Evil.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include <vector>
#include <algorithm>
#include <istream>

class jsminbuf : public std::streambuf
{
	std::streambuf* sbuf_;
	std::vector<char> buf_;
public:
	jsminbuf(std::streambuf* sbuf, size_t putback = 1)
		: theLookahead(EOF)
		, theX(EOF)
		, theY(EOF)
		, theA(EOF)
		, theB(EOF)
		, initialized_(false)
		, sbuf_(sbuf)
		, buf_(1024)
		, putback_(std::max(size_t(1), putback))
		, in_string_lit_(false)
		, in_regex_lit_(false)
	{
		auto end = buf_.data() + buf_.size();
		setg(end, end, end);
	}

	int underflow();

	void error(const char *descr)
	{
		throw std::exception(descr);
	}
	void put_char(char c)
	{
		*current_++ = c;
	}

	int	isAlphanum(int c);
	int get();
	int	peek();
	int	next();
	void action(int d);
	size_t jsmin();

	bool initialized_;
	char theLookahead;
	char theX;
	char theY;
	char theA;
	char theB;
	char * current_;
	char * start_;
	const size_t putback_;
	size_t max_chars_;

	bool in_string_lit_;
	bool in_regex_lit_;
};

struct jsminstream_base
{
	jsminbuf sbuf_;
	jsminstream_base(std::streambuf* sbuf) : sbuf_(sbuf)
	{
	}
};

class ijsminstream : virtual jsminstream_base, public std::istream
{
public:
	ijsminstream(std::streambuf* sbuf)
		: jsminstream_base(sbuf)
		, std::ios(&this->sbuf_)
		, std::istream(&this->sbuf_)
	{
	}
};