#include <iostream>
#include <memory>
#include <map>

// Include lexer and AST files
// lexer.cpp
// ast.cpp

// Global token state
static int CurTok;

/// getNextToken - Updates `CurTok` to the next token from the lexer.
int getNextToken() {
    return CurTok = gettok();
}

/// Error handling function
std::unique_ptr<ExprAST> LogError(const char *Str) {
    std::cerr << "Error: " << Str << "\n";
    return nullptr;
}

/// NumberExprAST Parser
std::unique_ptr<ExprAST> ParseNumberExpr() {
    auto Result = std::make_unique<NumberExprAST>(NumVal);
    getNextToken(); // consume the number
    return std::move(Result);
}

/// VariableExprAST Parser
std::unique_ptr<ExprAST> ParseIdentifierExpr() {
    std::string IdName = IdentifierStr;

    getNextToken(); // consume the identifier

    if (CurTok != '(') // Simple variable reference
        return std::make_unique<VariableExprAST>(IdName);

    // Function call
    getNextToken(); // eat '('
    std::vector<std::unique_ptr<ExprAST>> Args;
    if (CurTok != ')') {
        while (true) {
            if (auto Arg = ParseExpression()) {
                Args.push_back(std::move(Arg));
            } else {
                return nullptr;
            }

            if (CurTok == ')')
                break;

            if (CurTok != ',')
                return LogError("Expected ')' or ',' in argument list");
            getNextToken();
        }
    }

    getNextToken(); // consume ')'

    return std::make_unique<CallExprAST>(IdName, std::move(Args));
}

/// Parenthesis Parser
std::unique_ptr<ExprAST> ParseParenExpr() {
    getNextToken(); // eat '('
    auto V = ParseExpression();
    if (!V)
        return nullptr;

    if (CurTok != ')')
        return LogError("Expected ')'");
    getNextToken(); // eat ')'
    return V;
}

/// Primary expression parser
std::unique_ptr<ExprAST> ParsePrimary() {
    switch (CurTok) {
    case tok_identifier:
        return ParseIdentifierExpr();
    case tok_number:
        return ParseNumberExpr();
    case '(':
        return ParseParenExpr();
    default:
        return LogError("Unknown token when expecting an expression");
    }
}

/// Binary operation precedence table
static std::map<char, int> BinopPrecedence;

int GetTokPrecedence() {
    if (!isascii(CurTok))
        return -1;

    int TokPrec = BinopPrecedence[CurTok];
    if (TokPrec <= 0)
        return -1;
    return TokPrec;
}

/// Binary Expression Parser
std::unique_ptr<ExprAST> ParseBinOpRHS(int ExprPrec, std::unique_ptr<ExprAST> LHS) {
    while (true) {
        int TokPrec = GetTokPrecedence();

        if (TokPrec < ExprPrec)
            return LHS;

        int BinOp = CurTok;
        getNextToken(); // eat binary operator

        auto RHS = ParsePrimary();
        if (!RHS)
            return nullptr;

        int NextPrec = GetTokPrecedence();
        if (TokPrec < NextPrec) {
            RHS = ParseBinOpRHS(TokPrec + 1, std::move(RHS));
            if (!RHS)
                return nullptr;
        }

        LHS = std::make_unique<BinaryExprAST>(BinOp, std::move(LHS), std::move(RHS));
    }
}

/// Expression Parser
std::unique_ptr<ExprAST> ParseExpression() {
    auto LHS = ParsePrimary();
    if (!LHS)
        return nullptr;

    return ParseBinOpRHS(0, std::move(LHS));
}

/// Prototype parser
std::unique_ptr<PrototypeAST> ParsePrototype() {
    if (CurTok != tok_identifier)
        return nullptr;

    std::string FnName = IdentifierStr;
    getNextToken();

    if (CurTok != '(')
        return LogError("Expected '(' in prototype");

    std::vector<std::string> ArgNames;
    while (getNextToken() == tok_identifier)
        ArgNames.push_back(IdentifierStr);

    if (CurTok != ')')
        return LogError("Expected ')' in prototype");

    getNextToken(); // eat ')'

    return std::make_unique<PrototypeAST>(FnName, std::move(ArgNames));
}

/// Function definition parser
std::unique_ptr<FunctionAST> ParseDefinition() {
    getNextToken(); // eat 'def'
    auto Proto = ParsePrototype();
    if (!Proto)
        return nullptr;

    if (auto E = ParseExpression())
        return std::make_unique<FunctionAST>(std::move(Proto), std::move(E));
    return nullptr;
}

/// External declaration parser
std::unique_ptr<PrototypeAST> ParseExtern() {
    getNextToken(); // eat 'extern'
    return ParsePrototype();
}

/// Top-level expression parser
std::unique_ptr<FunctionAST> ParseTopLevelExpr() {
    if (auto E = ParseExpression()) {
        auto Proto = std::make_unique<PrototypeAST>("", std::vector<std::string>());
        return std::make_unique<FunctionAST>(std::move(Proto), std::move(E));
    }
    return nullptr;
}

/// Main parsing loop
void MainLoop() {
    while (true) {
        std::cout << "ready> ";
        switch (CurTok) {
        case tok_eof:
            return;
        case ';': // Ignore top-level semicolons
            getNextToken();
            break;
        case tok_def:
            if (ParseDefinition())
                std::cout << "Parsed a function definition.\n";
            else
                getNextToken();
            break;
        case tok_extern:
            if (ParseExtern())
                std::cout << "Parsed an extern.\n";
            else
                getNextToken();
            break;
        default:
            if (ParseTopLevelExpr())
                std::cout << "Parsed a top-level expression.\n";
            else
                getNextToken();
            break;
        }
    }
}

int main() {
    // Install standard binary operator precedence.
    BinopPrecedence['<'] = 10;
    BinopPrecedence['+'] = 20;
    BinopPrecedence['-'] = 20;
    BinopPrecedence['*'] = 40; // highest precedence

    std::cout << "ready> ";
    getNextToken();

    // Run the main loop
    MainLoop();

    return 0;
}
