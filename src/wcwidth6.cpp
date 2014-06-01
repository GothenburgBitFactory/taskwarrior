/*
 * This is an implementation of wcwidth() and wcswidth() (defined in
 * IEEE Std 1002.1-2001) for Unicode.
 *
 * http://www.opengroup.org/onlinepubs/007904975/functions/wcwidth.html
 * http://www.opengroup.org/onlinepubs/007904975/functions/wcswidth.html
 *
 * In fixed-width output devices, Latin characters all occupy a single
 * "cell" position of equal width, whereas ideographic CJK characters
 * occupy two such cells. Interoperability between terminal-line
 * applications and (teletype-style) character terminals using the
 * UTF-8 encoding requires agreement on which character should advance
 * the cursor by how many cell positions. No established formal
 * standards exist at present on which Unicode character shall occupy
 * how many cell positions on character terminals. These routines are
 * a first attempt of defining such behavior based on simple rules
 * applied to data provided by the Unicode Consortium.
 *
 * For some graphical characters, the Unicode standard explicitly
 * defines a character-cell width via the definition of the East Asian
 * FullWidth (F), Wide (W), Half-width (H), and Narrow (Na) classes.
 * In all these cases, there is no ambiguity about which width a
 * terminal shall use. For characters in the East Asian Ambiguous (A)
 * class, the width choice depends purely on a preference of backward
 * compatibility with either historic CJK or Western practice.
 * Choosing single-width for these characters is easy to justify as
 * the appropriate long-term solution, as the CJK practice of
 * displaying these characters as double-width comes from historic
 * implementation simplicity (8-bit encoded characters were displayed
 * single-width and 16-bit ones double-width, even for Greek,
 * Cyrillic, etc.) and not any typographic considerations.
 *
 * Much less clear is the choice of width for the Not East Asian
 * (Neutral) class. Existing practice does not dictate a width for any
 * of these characters. It would nevertheless make sense
 * typographically to allocate two character cells to characters such
 * as for instance EM SPACE or VOLUME INTEGRAL, which cannot be
 * represented adequately with a single-width glyph. The following
 * routines at present merely assign a single-cell width to all
 * neutral characters, in the interest of simplicity. This is not
 * entirely satisfactory and should be reconsidered before
 * establishing a formal standard in this area. At the moment, the
 * decision which Not East Asian (Neutral) characters should be
 * represented by double-width glyphs cannot yet be answered by
 * applying a simple rule from the Unicode database content. Setting
 * up a proper standard for the behavior of UTF-8 character terminals
 * will require a careful analysis not only of each Unicode character,
 * but also of each presentation form, something the author of these
 * routines has avoided to do so far.
 *
 * http://www.unicode.org/unicode/reports/tr11/
 *
 * Markus Kuhn -- 2007-05-26 (Unicode 5.0)
 *
 * Permission to use, copy, modify, and distribute this software
 * for any purpose and without fee is hereby granted. The author
 * disclaims all warranties with regard to this software.
 *
 * Latest version: http://www.cl.cam.ac.uk/~mgk25/ucs/wcwidth.c
 */

#include <cmake.h>
#include <wchar.h>

struct interval {
    int first;
    int last;
};

/* auxiliary function for binary search in interval table */
static int bisearch(wchar_t ucs, const struct interval *table, int max) {
    int min = 0;
    int mid;

    if (ucs < table[0].first || ucs > table[max].last)
        return 0;
    while (max >= min) {
        mid = (min + max) / 2;
        if (ucs > table[mid].last)
            min = mid + 1;
        else if (ucs < table[mid].first)
            max = mid - 1;
        else
            return 1;
    }

    return 0;
}


/* The following two functions define the column width of an ISO 10646
 * character as follows:
 *
 *    - The null character (U+0000) has a column width of 0.
 *
 *    - Other C0/C1 control characters and DEL will lead to a return
 *      value of -1.
 *
 *    - Non-spacing and enclosing combining characters (general
 *      category code Mn or Me in the Unicode database) have a
 *      column width of 0.
 *
 *    - SOFT HYPHEN (U+00AD) has a column width of 1.
 *
 *    - Other format characters (general category code Cf in the Unicode
 *      database) and ZERO WIDTH SPACE (U+200B) have a column width of 0.
 *
 *    - Hangul Jamo medial vowels and final consonants (U+1160-U+11FF)
 *      have a column width of 0.
 *
 *    - Spacing characters in the East Asian Wide (W) or East Asian
 *      Full-width (F) category as defined in Unicode Technical
 *      Report #11 have a column width of 2.
 *
 *    - All remaining characters (including all printable
 *      ISO 8859-1 and WGL4 characters, Unicode control characters,
 *      etc.) have a column width of 1.
 *
 * This implementation assumes that wchar_t characters are encoded
 * in ISO 10646.
 */

int mk_wcwidth(wchar_t ucs)
{
    /* sorted list of non-overlapping intervals of non-spacing characters */
    /* generated by "uniset +cat=Me +cat=Mn +cat=Cf -00AD +1160-11FF +200B c" */
    /* "uniset cat:Me + cat:Mn + cat:Cf - U+00AD + U+1160..U+11FF + U+200B" */
    static const struct interval combining[] = {
        { 0x0300, 0x036F }, { 0x0483, 0x0489 }, { 0x0591, 0x05BD },
        { 0x05BF, 0x05BF }, { 0x05C1, 0x05C2 }, { 0x05C4, 0x05C5 },
        { 0x05C7, 0x05C7 }, { 0x0600, 0x0604 }, { 0x0610, 0x061A },
        { 0x064B, 0x065F }, { 0x0670, 0x0670 }, { 0x06D6, 0x06DD },
        { 0x06DF, 0x06E4 }, { 0x06E7, 0x06E8 }, { 0x06EA, 0x06ED },
        { 0x070F, 0x070F }, { 0x0711, 0x0711 }, { 0x0730, 0x074A },
        { 0x07A6, 0x07B0 }, { 0x07EB, 0x07F3 }, { 0x0816, 0x0819 },
        { 0x081B, 0x0823 }, { 0x0825, 0x0827 }, { 0x0829, 0x082D },
        { 0x0859, 0x085B }, { 0x08E4, 0x08FE }, { 0x0900, 0x0902 },
        { 0x093A, 0x093A }, { 0x093C, 0x093C }, { 0x0941, 0x0948 },
        { 0x094D, 0x094D }, { 0x0951, 0x0957 }, { 0x0962, 0x0963 },
        { 0x0981, 0x0981 }, { 0x09BC, 0x09BC }, { 0x09C1, 0x09C4 },
        { 0x09CD, 0x09CD }, { 0x09E2, 0x09E3 }, { 0x0A01, 0x0A02 },
        { 0x0A3C, 0x0A3C }, { 0x0A41, 0x0A42 }, { 0x0A47, 0x0A48 },
        { 0x0A4B, 0x0A4D }, { 0x0A51, 0x0A51 }, { 0x0A70, 0x0A71 },
        { 0x0A75, 0x0A75 }, { 0x0A81, 0x0A82 }, { 0x0ABC, 0x0ABC },
        { 0x0AC1, 0x0AC5 }, { 0x0AC7, 0x0AC8 }, { 0x0ACD, 0x0ACD },
        { 0x0AE2, 0x0AE3 }, { 0x0B01, 0x0B01 }, { 0x0B3C, 0x0B3C },
        { 0x0B3F, 0x0B3F }, { 0x0B41, 0x0B44 }, { 0x0B4D, 0x0B4D },
        { 0x0B56, 0x0B56 }, { 0x0B62, 0x0B63 }, { 0x0B82, 0x0B82 },
        { 0x0BC0, 0x0BC0 }, { 0x0BCD, 0x0BCD }, { 0x0C3E, 0x0C40 },
        { 0x0C46, 0x0C48 }, { 0x0C4A, 0x0C4D }, { 0x0C55, 0x0C56 },
        { 0x0C62, 0x0C63 }, { 0x0CBC, 0x0CBC }, { 0x0CBF, 0x0CBF },
        { 0x0CC6, 0x0CC6 }, { 0x0CCC, 0x0CCD }, { 0x0CE2, 0x0CE3 },
        { 0x0D41, 0x0D44 }, { 0x0D4D, 0x0D4D }, { 0x0D62, 0x0D63 },
        { 0x0DCA, 0x0DCA }, { 0x0DD2, 0x0DD4 }, { 0x0DD6, 0x0DD6 },
        { 0x0E31, 0x0E31 }, { 0x0E34, 0x0E3A }, { 0x0E47, 0x0E4E },
        { 0x0EB1, 0x0EB1 }, { 0x0EB4, 0x0EB9 }, { 0x0EBB, 0x0EBC },
        { 0x0EC8, 0x0ECD }, { 0x0F18, 0x0F19 }, { 0x0F35, 0x0F35 },
        { 0x0F37, 0x0F37 }, { 0x0F39, 0x0F39 }, { 0x0F71, 0x0F7E },
        { 0x0F80, 0x0F84 }, { 0x0F86, 0x0F87 }, { 0x0F8D, 0x0F97 },
        { 0x0F99, 0x0FBC }, { 0x0FC6, 0x0FC6 }, { 0x102D, 0x1030 },
        { 0x1032, 0x1037 }, { 0x1039, 0x103A }, { 0x103D, 0x103E },
        { 0x1058, 0x1059 }, { 0x105E, 0x1060 }, { 0x1071, 0x1074 },
        { 0x1082, 0x1082 }, { 0x1085, 0x1086 }, { 0x108D, 0x108D },
        { 0x109D, 0x109D }, { 0x1160, 0x11FF }, { 0x135D, 0x135F },
        { 0x1712, 0x1714 }, { 0x1732, 0x1734 }, { 0x1752, 0x1753 },
        { 0x1772, 0x1773 }, { 0x17B4, 0x17B5 }, { 0x17B7, 0x17BD },
        { 0x17C6, 0x17C6 }, { 0x17C9, 0x17D3 }, { 0x17DD, 0x17DD },
        { 0x180B, 0x180D }, { 0x18A9, 0x18A9 }, { 0x1920, 0x1922 },
        { 0x1927, 0x1928 }, { 0x1932, 0x1932 }, { 0x1939, 0x193B },
        { 0x1A17, 0x1A18 }, { 0x1A56, 0x1A56 }, { 0x1A58, 0x1A5E },
        { 0x1A60, 0x1A60 }, { 0x1A62, 0x1A62 }, { 0x1A65, 0x1A6C },
        { 0x1A73, 0x1A7C }, { 0x1A7F, 0x1A7F }, { 0x1B00, 0x1B03 },
        { 0x1B34, 0x1B34 }, { 0x1B36, 0x1B3A }, { 0x1B3C, 0x1B3C },
        { 0x1B42, 0x1B42 }, { 0x1B6B, 0x1B73 }, { 0x1B80, 0x1B81 },
        { 0x1BA2, 0x1BA5 }, { 0x1BA8, 0x1BA9 }, { 0x1BAB, 0x1BAB },
        { 0x1BE6, 0x1BE6 }, { 0x1BE8, 0x1BE9 }, { 0x1BED, 0x1BED },
        { 0x1BEF, 0x1BF1 }, { 0x1C2C, 0x1C33 }, { 0x1C36, 0x1C37 },
        { 0x1CD0, 0x1CD2 }, { 0x1CD4, 0x1CE0 }, { 0x1CE2, 0x1CE8 },
        { 0x1CED, 0x1CED }, { 0x1CF4, 0x1CF4 }, { 0x1DC0, 0x1DE6 },
        { 0x1DFC, 0x1DFF }, { 0x200B, 0x200F }, { 0x202A, 0x202E },
        { 0x2060, 0x2064 }, { 0x206A, 0x206F }, { 0x20D0, 0x20F0 },
        { 0x2CEF, 0x2CF1 }, { 0x2D7F, 0x2D7F }, { 0x2DE0, 0x2DFF },
        { 0x302A, 0x302D }, { 0x3099, 0x309A }, { 0xA66F, 0xA672 },
        { 0xA674, 0xA67D }, { 0xA69F, 0xA69F }, { 0xA6F0, 0xA6F1 },
        { 0xA802, 0xA802 }, { 0xA806, 0xA806 }, { 0xA80B, 0xA80B },
        { 0xA825, 0xA826 }, { 0xA8C4, 0xA8C4 }, { 0xA8E0, 0xA8F1 },
        { 0xA926, 0xA92D }, { 0xA947, 0xA951 }, { 0xA980, 0xA982 },
        { 0xA9B3, 0xA9B3 }, { 0xA9B6, 0xA9B9 }, { 0xA9BC, 0xA9BC },
        { 0xAA29, 0xAA2E }, { 0xAA31, 0xAA32 }, { 0xAA35, 0xAA36 },
        { 0xAA43, 0xAA43 }, { 0xAA4C, 0xAA4C }, { 0xAAB0, 0xAAB0 },
        { 0xAAB2, 0xAAB4 }, { 0xAAB7, 0xAAB8 }, { 0xAABE, 0xAABF },
        { 0xAAC1, 0xAAC1 }, { 0xAAEC, 0xAAED }, { 0xAAF6, 0xAAF6 },
        { 0xABE5, 0xABE5 }, { 0xABE8, 0xABE8 }, { 0xABED, 0xABED },
        { 0xFB1E, 0xFB1E }, { 0xFE00, 0xFE0F }, { 0xFE20, 0xFE26 },
        { 0xFEFF, 0xFEFF }, { 0xFFF9, 0xFFFB }, { 0x101FD, 0x101FD },
        { 0x10A01, 0x10A03 }, { 0x10A05, 0x10A06 }, { 0x10A0C, 0x10A0F },
        { 0x10A38, 0x10A3A }, { 0x10A3F, 0x10A3F }, { 0x11001, 0x11001 },
        { 0x11038, 0x11046 }, { 0x11080, 0x11081 }, { 0x110B3, 0x110B6 },
        { 0x110B9, 0x110BA }, { 0x110BD, 0x110BD }, { 0x11100, 0x11102 },
        { 0x11127, 0x1112B }, { 0x1112D, 0x11134 }, { 0x11180, 0x11181 },
        { 0x111B6, 0x111BE }, { 0x116AB, 0x116AB }, { 0x116AD, 0x116AD },
        { 0x116B0, 0x116B5 }, { 0x116B7, 0x116B7 }, { 0x16F8F, 0x16F92 },
        { 0x1D167, 0x1D169 }, { 0x1D173, 0x1D182 }, { 0x1D185, 0x1D18B },
        { 0x1D1AA, 0x1D1AD }, { 0x1D242, 0x1D244 }, { 0xE0001, 0xE0001 },
        { 0xE0020, 0xE007F }, { 0xE0100, 0xE01EF },
    };

    /* test for 8-bit control characters */
    if (ucs == 0)
        return 0;
    if (ucs < 32 || (ucs >= 0x7f && ucs < 0xa0))
        return -1;

    /* binary search in table of non-spacing characters */
    if (bisearch(ucs, combining,
                 sizeof(combining) / sizeof(struct interval) - 1))
        return 0;

    /* if we arrive here, ucs is not a combining or C0/C1 control character */

    return 1 +
           (ucs >= 0x1100 &&
            (ucs <= 0x115f ||                    /* Hangul Jamo init. consonants */
             ucs == 0x2329 || ucs == 0x232a ||
             (ucs >= 0x2e80 && ucs <= 0xa4cf &&
              ucs != 0x303f) ||                  /* CJK ... Yi */
             (ucs >= 0xac00 && ucs <= 0xd7a3) || /* Hangul Syllables */
             (ucs >= 0xf900 && ucs <= 0xfaff) || /* CJK Compatibility Ideographs */
             (ucs >= 0xfe10 && ucs <= 0xfe19) || /* Vertical forms */
             (ucs >= 0xfe30 && ucs <= 0xfe6f) || /* CJK Compatibility Forms */
             (ucs >= 0xff00 && ucs <= 0xff60) || /* Fullwidth Forms */
             (ucs >= 0xffe0 && ucs <= 0xffe6)
#ifndef CYGWIN
                                              ||
             (ucs >= 0x20000 && ucs <= 0x2fffd) ||
             (ucs >= 0x30000 && ucs <= 0x3fffd)
#endif
            )
           );
}

int mk_wcswidth(const wchar_t *pwcs, size_t n)
{
    int w, width = 0;

    for (; *pwcs && n-- > 0; pwcs++)
        if ((w = mk_wcwidth(*pwcs)) < 0)
            return -1;
        else
            width += w;

    return width;
}
