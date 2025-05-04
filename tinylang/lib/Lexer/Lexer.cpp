#include "tinylang/Lexer/Lexer.h"

using namespace tinylang;

void KeywordFilter::add_keyword(StringRef keyword, tok::TokenKind token_code)
{
    hash_table.insert(std::make_pair(keyword, token_code));
}

void KeywordFilter::add_keywords()
{
#define KEYWORD(NAME, FLAGS) \
    add_keyword(StringRef(#NAME), tok::kw_##NAME);
#include "tinylang/Basic/TokenKinds.def"
}

namespace charinfo 
{
LLVM_READNONE inline bool is_ASCII(char ch)
{
    return static_cast<unsigned char>(ch) <= 127;
}

LLVM_READNONE inline bool is_vertical_withespace(char ch)
{
    return is_ASCII(ch) && (ch == '\r' || ch == '\n'); // 优化性能 is_ASCII 
}
LLVM_READNONE inline bool is_horizontal_withespace(char ch)
{
    return is_ASCII(ch) && (ch == ' ' || ch == '\t' || ch == '\f' || ch == '\v') ;
}
LLVM_READNONE inline bool is_whitespace(char ch)
{
    return is_horizontal_withespace(ch) || is_vertical_withespace(ch);
}

LLVM_READNONE inline bool is_digit(char ch)
{
    return is_ASCII(ch) && ch >= '0' && ch <= '9';
}
LLVM_READNONE inline bool is_hex_digit(char ch)
{
    return is_ASCII(ch) && (is_digit(ch) || (ch >= 'A' && ch <= 'F'));
}

LLVM_READNONE inline bool is_identifier_head(char ch)
{
    return is_ASCII(ch) && (ch == '_' || (ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z'));
}
LLVM_READNONE inline bool is_identifier_body(char ch)
{
    return is_identifier_head(ch) || is_digit(ch);
}
}

void Lexer::next(Token &result)
{
    // 空白字符
    while (*cur_ptr && charinfo::is_whitespace(*cur_ptr)) {
        ++cur_ptr;
    }

    // 结束
    if (!*cur_ptr)
    {
        result.set_kind(tok::eof);
        return;
    }

    if (charinfo::is_identifier_head(*cur_ptr))
    {
        identifier(result);
        return;
    }
    else if (charinfo::is_digit(*cur_ptr)) 
    {
        number(result);
        return;
    }
    else if (*cur_ptr == '"' || *cur_ptr == '\'')
    {
        string(result);
        return;
    }
    else {
        switch (*cur_ptr) {
#define CASE(ch, tok)                                       \
    case ch:                                                \
        from_token(result, cur_ptr + 1, tok);               \
        break
            
            CASE('=', tok::equal);
            CASE('#', tok::hash);
            CASE('+', tok::plus);
            CASE('-', tok::minus);
            CASE('*', tok::star);
            CASE('/', tok::slash);
            CASE(',', tok::comma);
            CASE('.', tok::period);
            CASE(';', tok::semi);
            CASE(')', tok::r_paren);

#undef CASE
            case '(':
                if (*(cur_ptr + 1) == '*')
                {
                    comment();
                    next(result);
                }
                else from_token(result, cur_ptr + 1, tok::l_paren);
                break;
            case ':':
                if (*(cur_ptr + 1) == '=') from_token(result, cur_ptr + 2, tok::colonequal);
                else from_token(result, cur_ptr + 1, tok::colon);
                break;
            case '<':
                if (*(cur_ptr + 1) == '=') from_token(result, cur_ptr + 2, tok::lessequal);
                else from_token(result, cur_ptr + 1, tok::less);
                break;
            case '>':
                if (*(cur_ptr + 1) == '=') from_token(result, cur_ptr + 2, tok::greaterequal);
                else from_token(result, cur_ptr + 1, tok::greater);
                break;
            default: result.set_kind(tok::unknown);
        }
        return;
    }
}

void Lexer::identifier(Token &result)
{
    const char *start = cur_ptr;
    const char *end = cur_ptr + 1;
    while (charinfo::is_identifier_body(*end)) ++end;

    StringRef name(start, end - start);
    from_token(result, end, key_words.get_keyword(name, tok::identifier));
}

void Lexer::number(Token &result)
{
    const char *end = cur_ptr + 1;
    tok::TokenKind kind = tok::unknown;
    bool is_hex = false;

    while (*end) 
    {
        if (!charinfo::is_hex_digit(*end)) break;
        if (!charinfo::is_digit(*end)) is_hex = true;
        ++end;
    }

    switch (*end) 
    {
        case 'H':
            kind = tok::integer_literal;
            +end;
            break;
        
        default:
            if (is_hex) diags.report(get_loc(), diag::err_hex_digit_in_decimal);
            kind = tok::integer_literal;
            break;
    }

    from_token(result, end, kind);
}

void Lexer::string(Token &result)
{
    const char *start = cur_ptr;
    const char *end = cur_ptr + 1;
    while (*end && *end != *start && !charinfo::is_vertical_withespace(*end)) ++end;    // 处理 "aaa\rbbb" 会出错
    if (charinfo::is_vertical_withespace(*end)) diags.report(get_loc(), diag::err_unterminated_char_or_string);
    from_token(result, end + 1, tok::string_literal);
}

void Lexer::comment()
{
    const char *end = cur_ptr + 2;
    unsigned level = 1;
    while (*end && level) {
        if (*end == '(' && *(end + 1) == '*')
        {
            end += 2;
            level++;
        }
        else if (*end == '*' && *(end + 1) == ')') {
            end += 2;
            level--;
        }
        else ++end;

        if (!*end) diags.report(get_loc(), diag::err_unterminated_block_comment);
    }
    cur_ptr = end;
}

void Lexer::from_token(Token &result, const char *end, tok::TokenKind kind)
{
    size_t token_length = end - cur_ptr;
    result.ptr = cur_ptr;
    result.length = token_length;
    result.kind = kind;
    cur_ptr = end;
}