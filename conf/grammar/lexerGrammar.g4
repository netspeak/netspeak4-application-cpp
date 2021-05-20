/**
 * This section of the grammar defines all lexer rules necessary for the Query
 * grammar.
 */
lexer grammar lexerGrammar
    ;


//Wildcard Chars
QMARK_TOKEN: '?';
PLUS_TOKEN: '+';
ASTERISK_TOKEN: '*' | DOT_CHAR;
ASSOCIATION_TOKEN: '|';
HASH_TOKEN: '#';
LBRACE_TOKEN: '{';
RBRACE_TOKEN: '}';
LBRACKET_TOKEN: '[';
RBRACKET_TOKEN: ']';
DBLQUOTE_TOKEN: '"';

/**
 * Definition of a WORD and REGEXWORD A WORD is all possibly combination of all
 * chars which are no whitespace or a wildcard A REGEXWORD contains at least
 * one REGEX expression and a second token.
 */

// TODO: This can be simplified
REGEXWORD_TOKEN
    : REGEX+ (WORD_TOKEN | REGEX)+
    | WORD_TOKEN REGEX+ (WORD_TOKEN | REGEX)*
    ;
WORD_TOKEN: (CHAR | WILDCARDSINWORDS | '.')+;


fragment DOT_CHAR: '.' '.'+;


/*
 * Fragments are rule with the only purpose to make the rules easier to read.
 * They wont be in the generated Files as Tokens.
 */
fragment REGEX
    : QMARK_TOKEN
    | ASTERISK_TOKEN
    | REGEXBRACKETS
    | REGEXBRACE
    | DOT_CHAR
    | PLUS_TOKEN
    ;
fragment REGEXBRACKETS: '[' CHAR+ ']';
fragment REGEXBRACE: '{' CHAR+ '}';


fragment WILDCARDSINWORDS
    : BACKSLASH_TOKEN ~(
        '\u0009' // CHARACTER TABULATION
        | '\u000b' // LINE TABULATION
        | '\u000c' // FORM FEED
        | '\u0020' // SPACE
        | '\u00a0' // NON-BREAKING SPACE
        | '\u1680' // OGHAM SPACE MARK
        | '\u2000' // EN QUAD
        | '\u2001' // EM QUAD
        | '\u2002' // EN SPACE
        | '\u2003' // EM SPACE
        | '\u2004' // THREE-PER-EM SPACE
        | '\u2005' // FOUR-PER-EM SPACE
        | '\u2006' // SIX-PER-EM SPACE
        | '\u2007' // FIGURE SPACE
        | '\u2008' // PUNCTUATION SPACE
        | '\u2009' // THIN SPACE
        | '\u200A' // HAIR SPACE
        | '\u200B' // ZERO WIDTH SPACE
        | '\u202F' // NARROW NO-BREAK SPACE
        | '\u3000' // IDEOGRAPHIC SPACE)
        | '\r'
        | '\n'
    )
    ;

BACKSLASH_TOKEN: '\\';


fragment CHAR
    : ~(
        '\u0009' // CHARACTER TABULATION
        | '\u000b' // LINE TABULATION
        | '\u000c' // FORM FEED
        | '\u0020' // SPACE
        | '\u00a0' // NON-BREAKING SPACE
        | '\u1680' // OGHAM SPACE MARK
        | '\u2000' // EN QUAD
        | '\u2001' // EM QUAD
        | '\u2002' // EN SPACE
        | '\u2003' // EM SPACE
        | '\u2004' // THREE-PER-EM SPACE
        | '\u2005' // FOUR-PER-EM SPACE
        | '\u2006' // SIX-PER-EM SPACE
        | '\u2007' // FIGURE SPACE
        | '\u2008' // PUNCTUATION SPACE
        | '\u2009' // THIN SPACE
        | '\u200A' // HAIR SPACE
        | '\u200B' // ZERO WIDTH SPACE
        | '\u202F' // NARROW NO-BREAK SPACE
        | '\u3000' // IDEOGRAPHIC SPACE)
        | '\r'
        | '\n'
        | '?'
        | '+'
        | '*'
        | '#'
        | '{'
        | '\\'
        | '}'
        | '['
        | ']'
        | '"'
        | '|'
        | '.'
    )
    ;

/**
 * This lexer rule is special because of the skip command. This rule matches
 * all possibly whitespace chars from Unicode. The generated tokenstream
 * contains no token which match this rule. This is however important when
 * writing parser rules.
 */
WHITESPACE_TOKEN
    : (
        '\u0009' // CHARACTER TABULATION
        | '\u000b' // LINE TABULATION
        | '\u000c' // FORM FEED
        | '\u0020' // SPACE
        | '\u00a0' // NON-BREAKING SPACE
        | '\u1680' // OGHAM SPACE MARK
        | '\u2000' // EN QUAD
        | '\u2001' // EM QUAD
        | '\u2002' // EN SPACE
        | '\u2003' // EM SPACE
        | '\u2004' // THREE-PER-EM SPACE
        | '\u2005' // FOUR-PER-EM SPACE
        | '\u2006' // SIX-PER-EM SPACE
        | '\u2007' // FIGURE SPACE
        | '\u2008' // PUNCTUATION SPACE
        | '\u2009' // THIN SPACE
        | '\u200A' // HAIR SPACE
        | '\u200B' // ZERO WIDTH SPACE
        | '\u202F' // NARROW NO-BREAK SPACE
        | '\u3000' // IDEOGRAPHIC SPACE)
        | '\r'
        | '\n'
    ) -> skip
    ;
