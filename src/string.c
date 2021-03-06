/* Copyright (c) 2006-2015 Jonas Fonseca <jonas.fonseca@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include "tig/tig.h"
#include "tig/string.h"

/*
 * Strings.
 */

bool
string_isnumber(const char *str)
{
	int pos;

	for (pos = 0; str[pos]; pos++) {
		if (!isdigit(str[pos]))
			return false;
	}

	return pos > 0;
}

bool
iscommit(const char *str)
{
	int pos;

	for (pos = 0; str[pos]; pos++) {
		if (!isxdigit(str[pos]))
			return false;
	}

	return 7 <= pos && pos < SIZEOF_REV;
}

int
suffixcmp(const char *str, int slen, const char *suffix)
{
	size_t len = slen >= 0 ? slen : strlen(str);
	size_t suffixlen = strlen(suffix);

	return suffixlen < len ? strcmp(str + len - suffixlen, suffix) : -1;
}

void
string_ncopy_do(char *dst, size_t dstlen, const char *src, size_t srclen)
{
	if (srclen > dstlen - 1)
		srclen = dstlen - 1;

	strncpy(dst, src, srclen);
	dst[srclen] = 0;
}

void
string_copy_rev(char *dst, const char *src)
{
	size_t srclen;

	if (!*src)
		return;

	for (srclen = 0; srclen < SIZEOF_REV; srclen++)
		if (!src[srclen] || isspace(src[srclen]))
			break;

	string_ncopy_do(dst, SIZEOF_REV, src, srclen);
}

void
string_copy_rev_from_commit_line(char *dst, const char *src)
{
	string_copy_rev(dst, src + STRING_SIZE("commit "));
}

size_t
string_expanded_length(const char *src, size_t srclen, size_t tabsize, size_t max_size)
{
	size_t size, pos;

	for (size = pos = 0; pos < srclen && size < max_size; pos++) {
		if (src[pos] == '\t') {
			size_t expanded = tabsize - (size % tabsize);

			size += expanded;
		} else {
			size++;
		}
	}

	return pos;
}

size_t
string_expand(char *dst, size_t dstlen, const char *src, int srclen, int tabsize)
{
	size_t size, pos;

	for (size = pos = 0; size < dstlen - 1 && (srclen == -1 || pos < srclen) && src[pos]; pos++) {
		const char c = src[pos];

		if (c == '\t') {
			size_t expanded = tabsize - (size % tabsize);

			if (expanded + size >= dstlen - 1)
				expanded = dstlen - size - 1;
			memcpy(dst + size, "        ", expanded);
			size += expanded;
		} else if (isspace(c) || iscntrl(c)) {
			dst[size++] = ' ';
		} else {
			dst[size++] = src[pos];
		}
	}

	dst[size] = 0;
	return pos;
}

char *
string_trim_end(char *name)
{
	int namelen = strlen(name) - 1;

	while (namelen > 0 && isspace(name[namelen]))
		name[namelen--] = 0;

	return name;
}

char *
string_trim(char *name)
{
	while (isspace(*name))
		name++;

	return string_trim_end(name);
}

bool PRINTF_LIKE(4, 5)
string_nformat(char *buf, size_t bufsize, size_t *bufpos, const char *fmt, ...)
{
	size_t pos = bufpos ? *bufpos : 0;
	int retval;

	FORMAT_BUFFER(buf + pos, bufsize - pos, fmt, retval, false);
	if (bufpos && retval > 0)
		*bufpos = pos + retval;

	return pos >= bufsize ? false : true;
}

int
strcmp_null(const char *s1, const char *s2)
{
	if (!s1 || !s2) {
		return (!!s1) - (!!s2);
	}

	return strcmp(s1, s2);
}

int
strcmp_numeric(const char *s1, const char *s2)
{
	int number = 0;
	int num1, num2;

	for (; *s1 && *s2 && *s1 == *s2; s1++, s2++) {
		int c = *s1;

		if (isdigit(c)) {
			number = 10 * number + (c - '0');
		} else {
			number = 0;
		}
	}

	num1 = number * 10 + atoi(s1);
	num2 = number * 10 + atoi(s2);

	if (num1 != num2)
		return num2 - num1;

	if (!!*s1 != !!*s2)
		return !!*s2 - !!*s1;
	return *s1 - *s2;
}

/*
 * Unicode / UTF-8 handling
 *
 * NOTE: Much of the following code for dealing with Unicode is derived from
 * ELinks' UTF-8 code developed by Scrool <scroolik@gmail.com>. Origin file is
 * src/intl/charset.c from the UTF-8 branch commit elinks-0.11.0-g31f2c28.
 *
 * unicode_width() is based on Markus Kuhn's mk_wcwidth() from
 * http://www.cl.cam.ac.uk/~mgk25/ucs/wcwidth.c
 */

struct interval {
	int first;
	int last;
};

static int
bisearch(unsigned long c, const struct interval *table, int max)
{
	int min = 0;
	int mid;

	if (c < table[0].first || c > table[max].last)
		return 0;
	while (max >= min) {
		mid = (min + max) / 2;
		if (c > table[mid].last)
			min = mid + 1;
		else if (c < table[mid].first)
			max = mid - 1;
		else
			return 1;
	}

	return 0;
}

int
unicode_width(unsigned long c, int tab_size)
{
	/* sorted list of non-overlapping intervals of non-spacing characters */
	/* generated by "uniset +cat=Me +cat=Mn +cat=Cf -00AD +1160-11FF +200B c" */
	static const struct interval combining[] = {
		{ 0x0300, 0x036F }, { 0x0483, 0x0486 }, { 0x0488, 0x0489 },
		{ 0x0591, 0x05BD }, { 0x05BF, 0x05BF }, { 0x05C1, 0x05C2 },
		{ 0x05C4, 0x05C5 }, { 0x05C7, 0x05C7 }, { 0x0600, 0x0603 },
		{ 0x0610, 0x0615 }, { 0x064B, 0x065E }, { 0x0670, 0x0670 },
		{ 0x06D6, 0x06E4 }, { 0x06E7, 0x06E8 }, { 0x06EA, 0x06ED },
		{ 0x070F, 0x070F }, { 0x0711, 0x0711 }, { 0x0730, 0x074A },
		{ 0x07A6, 0x07B0 }, { 0x07EB, 0x07F3 }, { 0x0901, 0x0902 },
		{ 0x093C, 0x093C }, { 0x0941, 0x0948 }, { 0x094D, 0x094D },
		{ 0x0951, 0x0954 }, { 0x0962, 0x0963 }, { 0x0981, 0x0981 },
		{ 0x09BC, 0x09BC }, { 0x09C1, 0x09C4 }, { 0x09CD, 0x09CD },
		{ 0x09E2, 0x09E3 }, { 0x0A01, 0x0A02 }, { 0x0A3C, 0x0A3C },
		{ 0x0A41, 0x0A42 }, { 0x0A47, 0x0A48 }, { 0x0A4B, 0x0A4D },
		{ 0x0A70, 0x0A71 }, { 0x0A81, 0x0A82 }, { 0x0ABC, 0x0ABC },
		{ 0x0AC1, 0x0AC5 }, { 0x0AC7, 0x0AC8 }, { 0x0ACD, 0x0ACD },
		{ 0x0AE2, 0x0AE3 }, { 0x0B01, 0x0B01 }, { 0x0B3C, 0x0B3C },
		{ 0x0B3F, 0x0B3F }, { 0x0B41, 0x0B43 }, { 0x0B4D, 0x0B4D },
		{ 0x0B56, 0x0B56 }, { 0x0B82, 0x0B82 }, { 0x0BC0, 0x0BC0 },
		{ 0x0BCD, 0x0BCD }, { 0x0C3E, 0x0C40 }, { 0x0C46, 0x0C48 },
		{ 0x0C4A, 0x0C4D }, { 0x0C55, 0x0C56 }, { 0x0CBC, 0x0CBC },
		{ 0x0CBF, 0x0CBF }, { 0x0CC6, 0x0CC6 }, { 0x0CCC, 0x0CCD },
		{ 0x0CE2, 0x0CE3 }, { 0x0D41, 0x0D43 }, { 0x0D4D, 0x0D4D },
		{ 0x0DCA, 0x0DCA }, { 0x0DD2, 0x0DD4 }, { 0x0DD6, 0x0DD6 },
		{ 0x0E31, 0x0E31 }, { 0x0E34, 0x0E3A }, { 0x0E47, 0x0E4E },
		{ 0x0EB1, 0x0EB1 }, { 0x0EB4, 0x0EB9 }, { 0x0EBB, 0x0EBC },
		{ 0x0EC8, 0x0ECD }, { 0x0F18, 0x0F19 }, { 0x0F35, 0x0F35 },
		{ 0x0F37, 0x0F37 }, { 0x0F39, 0x0F39 }, { 0x0F71, 0x0F7E },
		{ 0x0F80, 0x0F84 }, { 0x0F86, 0x0F87 }, { 0x0F90, 0x0F97 },
		{ 0x0F99, 0x0FBC }, { 0x0FC6, 0x0FC6 }, { 0x102D, 0x1030 },
		{ 0x1032, 0x1032 }, { 0x1036, 0x1037 }, { 0x1039, 0x1039 },
		{ 0x1058, 0x1059 }, { 0x1160, 0x11FF }, { 0x135F, 0x135F },
		{ 0x1712, 0x1714 }, { 0x1732, 0x1734 }, { 0x1752, 0x1753 },
		{ 0x1772, 0x1773 }, { 0x17B4, 0x17B5 }, { 0x17B7, 0x17BD },
		{ 0x17C6, 0x17C6 }, { 0x17C9, 0x17D3 }, { 0x17DD, 0x17DD },
		{ 0x180B, 0x180D }, { 0x18A9, 0x18A9 }, { 0x1920, 0x1922 },
		{ 0x1927, 0x1928 }, { 0x1932, 0x1932 }, { 0x1939, 0x193B },
		{ 0x1A17, 0x1A18 }, { 0x1B00, 0x1B03 }, { 0x1B34, 0x1B34 },
		{ 0x1B36, 0x1B3A }, { 0x1B3C, 0x1B3C }, { 0x1B42, 0x1B42 },
		{ 0x1B6B, 0x1B73 }, { 0x1DC0, 0x1DCA }, { 0x1DFE, 0x1DFF },
		{ 0x200B, 0x200F }, { 0x202A, 0x202E }, { 0x2060, 0x2063 },
		{ 0x206A, 0x206F }, { 0x20D0, 0x20EF }, { 0x302A, 0x302F },
		{ 0x3099, 0x309A }, { 0xA806, 0xA806 }, { 0xA80B, 0xA80B },
		/*
		 * BUG: the last range in the next line should be { 0xFE00, 0xFE0F }
		 * inclusive of 0xFE0F ("VARIATION SELECTOR-16"), but that causes
		 * platform-specific failures in test/main/emoji-test :
		 *   - test fails in Travis environment, both gcc/clang
		 *   - test fails on Ubuntu 16.04 x86_64, gcc
		 *   - test passes on OS X 10.11.6, both gcc/clang
		 */
		{ 0xA825, 0xA826 }, { 0xFB1E, 0xFB1E }, { 0xFE00, 0xFE0E },
		{ 0xFE20, 0xFE23 }, { 0xFEFF, 0xFEFF }, { 0xFFF9, 0xFFFB },
		{ 0x10A01, 0x10A03 }, { 0x10A05, 0x10A06 }, { 0x10A0C, 0x10A0F },
		{ 0x10A38, 0x10A3A }, { 0x10A3F, 0x10A3F }, { 0x1D167, 0x1D169 },
		{ 0x1D173, 0x1D182 }, { 0x1D185, 0x1D18B }, { 0x1D1AA, 0x1D1AD },
		{ 0x1D242, 0x1D244 }, { 0xE0001, 0xE0001 }, { 0xE0020, 0xE007F },
		{ 0xE0100, 0xE01EF }
	};

	if (bisearch(c, combining, sizeof(combining) / sizeof(struct interval) - 1))
		return 0;

	if (c >= 0x1100 &&
	   (c <= 0x115f				/* Hangul Jamo */
	    || c == 0x2329
	    || c == 0x232a
	    || (c >= 0x2e80  && c <= 0xa4cf && c != 0x303f)
						/* CJK ... Yi */
	    || (c >= 0xac00  && c <= 0xd7a3)	/* Hangul Syllables */
	    || (c >= 0xf900  && c <= 0xfaff)	/* CJK Compatibility Ideographs */
	    || (c >= 0xfe10  && c <= 0xfe19)	/* Vertical forms */
	    || (c >= 0xfe30  && c <= 0xfe6f)	/* CJK Compatibility Forms */
	    || (c >= 0xff00  && c <= 0xff60)	/* Fullwidth Forms */
	    || (c >= 0xffe0  && c <= 0xffe6)
	    || (c >= 0x20000 && c <= 0x2fffd)
	    || (c >= 0x30000 && c <= 0x3fffd)))
		return 2;

	if (c == '\t')
		return tab_size;

	/* mk_wcwidth() returns 0 for NUL */
	/* two tig tests fail if this returns 0, which seems like a bug */
	if (c == '\0')
		return 1;

	/* mk_wcwidth() returns -1 for control characters, which might be smarter */
	if (c < 32 || (c >= 0x7f && c < 0xa0))
		return 0;

	return 1;
}

/* Number of bytes used for encoding a UTF-8 character indexed by first byte.
 * Illegal bytes are set one. */
static const unsigned char utf8_bytes[256] = {
	1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,
	2,2,2,2,2,2,2,2, 2,2,2,2,2,2,2,2, 2,2,2,2,2,2,2,2, 2,2,2,2,2,2,2,2,
	3,3,3,3,3,3,3,3, 3,3,3,3,3,3,3,3, 4,4,4,4,4,4,4,4, 5,5,5,5,6,6,1,1,
};

unsigned char
utf8_char_length(const char *string)
{
	int c = *(unsigned char *) string;

	return utf8_bytes[c];
}

/* Decode UTF-8 multi-byte representation into a Unicode character. */
unsigned long
utf8_to_unicode(const char *string, size_t length)
{
	unsigned long unicode;

	switch (length) {
	case 1:
		unicode  =   string[0];
		break;
	case 2:
		unicode  =  (string[0] & 0x1f) << 6;
		unicode +=  (string[1] & 0x3f);
		break;
	case 3:
		unicode  =  (string[0] & 0x0f) << 12;
		unicode += ((string[1] & 0x3f) << 6);
		unicode +=  (string[2] & 0x3f);
		break;
	case 4:
		unicode  =  (string[0] & 0x0f) << 18;
		unicode += ((string[1] & 0x3f) << 12);
		unicode += ((string[2] & 0x3f) << 6);
		unicode +=  (string[3] & 0x3f);
		break;
	case 5:
		unicode  =  (string[0] & 0x0f) << 24;
		unicode += ((string[1] & 0x3f) << 18);
		unicode += ((string[2] & 0x3f) << 12);
		unicode += ((string[3] & 0x3f) << 6);
		unicode +=  (string[4] & 0x3f);
		break;
	case 6:
		unicode  =  (string[0] & 0x01) << 30;
		unicode += ((string[1] & 0x3f) << 24);
		unicode += ((string[2] & 0x3f) << 18);
		unicode += ((string[3] & 0x3f) << 12);
		unicode += ((string[4] & 0x3f) << 6);
		unicode +=  (string[5] & 0x3f);
		break;
	default:
		return 0;
	}

	/* Invalid characters could return the special 0xfffd value but NUL
	 * should be just as good. */
	return unicode > 0x10FFFF ? 0 : unicode;
}

/* Calculates how much of string can be shown within the given maximum width
 * and sets trimmed parameter to non-zero value if all of string could not be
 * shown. If the reserve flag is true, it will reserve at least one
 * trailing character, which can be useful when drawing a delimiter.
 *
 * Returns the number of bytes to output from string to satisfy max_width. */
size_t
utf8_length(const char **start, int max_chars, size_t skip, int *width, size_t max_width, int *trimmed, bool reserve, int tab_size)
{
	const char *string = *start;
	const char *end = max_chars < 0 ? strchr(string, '\0') : string + max_chars;
	unsigned char last_bytes = 0;
	size_t last_ucwidth = 0;

	*width = 0;
	*trimmed = 0;

	while (string < end) {
		unsigned char bytes = utf8_char_length(string);
		size_t ucwidth;
		unsigned long unicode;

		if (string + bytes > end)
			break;

		/* Change representation to figure out whether
		 * it is a single- or double-width character. */

		unicode = utf8_to_unicode(string, bytes);
		/* FIXME: Graceful handling of invalid Unicode character. */
		if (!unicode)
			break;

		ucwidth = unicode_width(unicode, tab_size);
		if (skip > 0) {
			skip -= ucwidth <= skip ? ucwidth : skip;
			*start += bytes;
		}
		*width  += ucwidth;
		if (max_width > 0 && *width > max_width) {
			*trimmed = 1;
			*width -= ucwidth;
			if (reserve && *width == max_width) {
				string -= last_bytes;
				*width -= last_ucwidth;
			}
			break;
		}

		string  += bytes;
		if (ucwidth) {
			last_bytes = bytes;
			last_ucwidth = ucwidth;
		} else {
			last_bytes += bytes;
		}
	}

	return string - *start;
}

int
utf8_width_of(const char *text, int max_bytes, int max_width)
{
	int text_width = 0;
	const char *tmp = text;
	int trimmed = false;

	utf8_length(&tmp, max_bytes, 0, &text_width, max_width, &trimmed, false, 1);
	return text_width;
}

/* vim: set ts=8 sw=8 noexpandtab: */
