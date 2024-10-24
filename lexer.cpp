#include <string>
#include <vector>
#include <cctype>
#include <iostream>
// Tokens to be defined
// Keyword,
// Identifier,
// Literal,
// Operator,
// Punctuation,
// EndOfFile,
// Unknown


enum Token
{
  tok_eof = -1,

  // commands
  tok_def = -2,
  tok_extern = -3,

  // primary
  tok_identifier = -4,
  tok_number = -5,
};

static std::string IdentifierStr; // Filled in if tok_identifier
static double NumVal;             // Filled in if tok_number

/// gettok - Return the next token from standard input.
static int gettok()
{
  static int LastChar = ' ';
  // Skip any whitespace.
  while (isspace(LastChar))
    LastChar = getchar();
if (isalpha(LastChar))
{ // identifier: [a-zA-Z][a-zA-Z0-9]*
  IdentifierStr = LastChar;
  while (isalnum((LastChar = getchar())))
    IdentifierStr += LastChar;

  if (IdentifierStr == "def")
    return tok_def;
  if (IdentifierStr == "extern")
    return tok_extern;
  return tok_identifier;
}
// Note that this code sets the ‘IdentifierStr’ global whenever it lexes an identifier. Also, since language keywords are matched by the same loop, we handle them here inline. Numeric values are similar:
if (isdigit(LastChar) || LastChar == '.')
{ // Number: [0-9.]+
  std::string NumStr;
  do
  {
    NumStr += LastChar;
    LastChar = getchar();
  } while (isdigit(LastChar) || LastChar == '.');

  NumVal = strtod(NumStr.c_str(), 0);
  return tok_number;
}

// Comments(Skipped by lexer)
if (LastChar == '#') {
  // Comment until end of line.
  do
    LastChar = getchar();
  while (LastChar != EOF && LastChar != '\n' && LastChar != '\r');

  if (LastChar != EOF)
    return gettok();
}
// Check for end of file.  Don't eat the EOF.
if (LastChar == EOF)
  return tok_eof;

// Otherwise, just return the character as its ascii value.
int ThisChar = LastChar;
LastChar = getchar();
return ThisChar;
}
