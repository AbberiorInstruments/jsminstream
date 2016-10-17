#include "jsmin.hpp"

// File is a modified version of the following:
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
int jsminbuf::underflow()
{
	if (gptr() < egptr()) // buffer not exhausted
		return traits_type::to_int_type(*gptr());

	start_ = buf_.data();

	if (eback() == buf_.data()) // true when this isn't the first fill
	{
		// Make arrangements for putback characters
		std::memmove(buf_.data(), egptr() - putback_, putback_);
		start_ += putback_;
	}

	// start is now the start of the buffer, proper.
	// Read from fptr_ in to the provided buffer
	current_ = start_;
	max_chars_ = buf_.size() - (start_ - buf_.data());
	size_t n = jsmin();
	if (n == 0)
		return traits_type::eof();

	// Set buffer pointers
	setg(buf_.data(), start_, start_ + n);

	return traits_type::to_int_type(*gptr());
}

int jsminbuf::isAlphanum(int c)
{
	return ((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') ||
		(c >= 'A' && c <= 'Z') || c == '_' || c == '$' || c == '\\' ||
		c > 126);
}

int jsminbuf::get()
{
	int c = theLookahead;
	theLookahead = EOF;
	if (c == EOF) {
		c = sbuf_->sbumpc();
	}
	if (c >= ' ' || c == '\n' || c == EOF) {
		return c;
	}
	if (c == '\r') {
		return '\n';
	}
	return ' ';
}

int jsminbuf::peek()
{
	theLookahead = get();
	return theLookahead;
}

int jsminbuf::next()
{
	int c = get();
	if (c == '/') {
		switch (peek()) {
		case '/':
			for (;;) {
				c = get();
				if (c <= '\n') {
					break;
				}
			}
			break;
		case '*':
			get();
			while (c != ' ') {
				switch (get()) {
				case '*':
					if (peek() == '/') {
						get();
						c = ' ';
					}
					break;
				case EOF:
					error("Unterminated comment.");
				}
			}
			break;
		}
	}
	theY = theX;
	theX = c;
	return c;
}

void jsminbuf::action(int d)
{
	switch (d) {
	case 1:
		put_char(theA);
		if (
			(theY == '\n' || theY == ' ') &&
			(theA == '+' || theA == '-' || theA == '*' || theA == '/') &&
			(theB == '+' || theB == '-' || theB == '*' || theB == '/')
			) {
			put_char(theY);
		}
	case 2:
		theA = theB;
		if (!(theA == '\'' || theA == '"' || theA == '`'))
		{
			action(3);
			break;
		}
		in_string_lit_ = true;
	case 22:
		for (;;) {
			if (current_ - start_ >= int(max_chars_) - 5)
				return;

			put_char(theA);
			theA = get();
			if (theA == theB) {
				break;
			}
			if (theA == '\\') {
				put_char(theA);
				theA = get();
			}
			if (theA == EOF) {
				error("Unterminated string literal.");
			}
		}
		in_string_lit_ = false;
	case 3:
		theB = next();
		if (theB == '/' && (
			theA == '(' || theA == ',' || theA == '=' || theA == ':' ||
			theA == '[' || theA == '!' || theA == '&' || theA == '|' ||
			theA == '?' || theA == '+' || theA == '-' || theA == '~' ||
			theA == '*' || theA == '/' || theA == '{' || theA == '\n'
			)) {
			put_char(theA);
			if (theA == '/' || theA == '*') {
				put_char(' ');
			}
			put_char(theB);
			in_regex_lit_ = true;
	case 33:
			for (;;) {
				theA = get();
				if (theA == '[') {
					for (;;) {
						if (current_ - start_ >= int(max_chars_) - 5)
							return;
						put_char(theA);
						theA = get();
						if (theA == ']') {
							break;
						}
						if (theA == '\\') {
							put_char(theA);
							theA = get();
						}
						if (theA == EOF) {
							error("Unterminated set in Regular Expression literal.");
						}
					}
				}
				else if (theA == '/') {
					switch (peek()) {
					case '/':
					case '*':
						error("Unterminated set in Regular Expression literal.");
					}
					break;
				}
				else if (theA == '\\') {
					put_char(theA);
					theA = get();
				}
				if (theA == EOF) {
					error("Unterminated Regular Expression literal.");
				}
				put_char(theA);
			}
			theB = next();
			in_regex_lit_ = false;
		}
	}
}

size_t jsminbuf::jsmin()
{
	if (!initialized_)
	{
		if (peek() == 0xEF)
		{
			get();
			get();
			get();
		}
		theA = '\n';
		action(3);
		initialized_ = true;
	}

	if (in_string_lit_)
	{
		action(22);
	}
	else if (in_regex_lit_)
	{
		action(33);
	}

	while (current_ - start_ < int(max_chars_) - 5)
	{
		if (theA == EOF)
			break;

		switch (theA)
		{
		case ' ':
			action(isAlphanum(theB) ? 1 : 2);
			break;
		case '\n':
			switch (theB) {
			case '{':
			case '[':
			case '(':
			case '+':
			case '-':
			case '!':
			case '~':
				action(1);
				break;
			case ' ':
				action(3);
				break;
			default:
				action(isAlphanum(theB) ? 1 : 2);
			}
			break;
		default:
			switch (theB) {
			case ' ':
				action(isAlphanum(theA) ? 1 : 3);
				break;
			case '\n':
				switch (theA) {
				case '}':
				case ']':
				case ')':
				case '+':
				case '-':
				case '"':
				case '\'':
				case '`':
					action(1);
					break;
				default:
					action(isAlphanum(theA) ? 1 : 3);
				}
				break;
			default:
				action(1);
				break;
			}
		}
	}
	return current_ - start_;
}
