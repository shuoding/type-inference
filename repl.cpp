#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <utility>
#include <memory>
#include <cctype>
#include <cstdlib>
#include <queue>

struct Token {
	Token() {}
	virtual std::string getType() {
		return "Token";
	}
	virtual ~Token() {}
}

struct N : public Token { // Variable Name
	N(const std::string &val0) : val(val0) {}
	std::string getType() override {
		return "N";
	}
	~N() override {}

	std::string val;
}

struct B : public Token { // Boolean Literal
	B(bool val0) : val(val0) {}
	std::string getType() override {
		return "B";
	}
	~B() override {}

	bool val;
}

struct I : public Token { // Integer Literal
	I(int val0) : val(val0) {}
	std::string getType() override {
		return "I";
	}
	~I() override {}

	int val;
}

struct K : public Token { // Reserved Keyword
	K(const std::string &keyword0) : keyword(keyword0) {}
	std::string getType() override {
		return "K";
	}
	~K() override {}

	std::string keyword;
}

struct Node {
	Node() {}
	virtual std::string getType() {
		return "Node";
	}
	virtual ~Node() {}
}

struct Var : public Node {
	Var(const std::string &val0) : val(val0) {}
	std::string getType() override {
		return "Var";
	}
	~Var() override {}

	std::string val;
}

struct Int : public Node {
	Int(int val0) : val(val0) {}
	std::string getType() override {
		return "Int";
	}
	~Int() override {}

	int val;
}

struct Bool : public Node {
	Bool(bool val0) : val(val0) {}
	std::string getType() override {
		return "Bool";
	}
	~Bool() override {}

	bool val;
}

struct Add : public Node {
	Add(Node *n10, Node *n20) : n1(n10), n2(n20) {}
	std::string getType() override {
		return "Add";
	}
	~Add() override {
		delete n1;
		delete n2;
	}

	Node *n1, *n2;
}

struct Sub : public Node {
	Sub(Node *n10, Node *n20) : n1(n10), n2(n20) {}
	std::string getType() override {
		return "Sub";
	}
	~Sub() override {
		delete n1;
		delete n2;
	}

	Node *n1, *n2;
}

struct Mul : public Node {
	Mul(Node *n10, Node *n20) : n1(n10), n2(n20) {}
	std::string getType() override {
		return "Mul";
	}
	~Mul() override {
		delete n1;
		delete n2;
	}

	Node *n1, *n2;
}

struct Div : public Node {
	Div(Node *n10, Node *n20) : n1(n10), n2(n20) {}
	std::string getType() override {
		return "Div";
	}
	~Div() override {
		delete n1;
		delete n2;
	}

	Node *n1, *n2;
}

struct And : public Node {
	And(Node *n10, Node *n20) : n1(n10), n2(n20) {}
	std::string getType() override {
		return "And";
	}
	~And() override {
		delete n1;
		delete n2;
	}

	Node *n1, *n2;
}

struct Or : public Node {
	Or(Node *n10, Node *n20) : n1(n10), n2(n20) {}
	std::string getType() override {
		return "Or";
	}
	~Or() override {
		delete n1;
		delete n2;
	}

	Node *n1, *n2;
}

struct Not : public Node {
	Not(Node *n0) : n(n0) {}
	std::string getType() override {
		return "Not";
	}
	~Not() override {
		delete n;
	}

	Node *n;
}

struct If : public Node {
	If(Node *n10, Node *n20, Node *n30) : n1(n10), n2(n20), n3(n30) {}
	std::string getType() override {
		return "If";
	}
	~If() override {
		delete n1;
		delete n2;
		delete n3;
	}

	Node *n1, *n2, *n3;
}

struct Let : public Node {
	Let(Node *n10, Node *n20, Node *n30) : n1(n10), n2(n20), n3(n30) {}
	std::string getType() override {
		return "Let";
	}
	~Let() override {
		delete n1;
		delete n2;
		delete n3;
	}

	Node *n1, *n2, *n3;
}

struct AST {
	AST(Node *root0) : root(root0) {}
	~AST() {
		delete root;
	}

	Node *root;
}

struct UnionFind {
	int n;
	std::vector<int> prev;
	UnionFind(int n0) : n(n0) {
		for (int i = 0; i < n0; i++) {
			prev.push_back(i);
		}
	}
	int find(int x) {
		int r = x;
		while (prev[r] != r) {
			r = prev[r];
		}
		int i = x, j;
		while (prev[i] != r) {
			j = prev[i];
			prev[i] = r;
			i = j;
		}
		return r;
	}
	void join(int x, int y) { // The second argument serves as the root.
		prev[find(x)] = find(y);
	}
}

/*

variable names: [a-zA-Z]+
boolean literal: true false
integer literal: -?[0-9]+
reserved keywords: ( ) + - * / && || ! if then else let = in

*/

bool myIsAlpha(char ch) {
	return std::isalpha(static_cast<unsigned char>(ch));
}

bool myIsDigit(char ch) {
	return std::isdigit(static_cast<unsigned char>(ch));
}

bool myIsSpace() {
	return std::isspace(static_cast<unsigned char>(ch));
}

std::queue<Token*> tokenize(const std::string &source) {
	std::queue<Token*> ret;
	int n = source.size();
	int i = 0;
	while (i < n) {
		if (myIsAlpha(source[i])) { // starting with English letters
			std::string word;
			while (i < n && myIsAlpha(source[i])) {
				word.push_back(source[i]);
				i++;
			}
			if (word == "true") {
				ret.push(new B(true));
			} else if (word == "false") {
				ret.push(new B(false));
			} else if (word == "if") {
				ret.push(new K("if"));
			} else if (word == "then") {
				ret.push(new K("then"));
			} else if (word == "else") {
				ret.push(new K("else"));
			} else if (word == "let") {
				ret.push(new K("let"));
			} else if (word == "in") {
				ret.push(new K("in"));
			} else {
				ret.push(new N(word));
			}
		} else { // starting with other characters
			switch (source[i]) {
			case '(':
				ret.push(new K("("));
				i++;
				break;
			case ')':
				ret.push(new K(")"));
				i++;
				break;
			case '+':
				ret.push(new K("+"));
				i++;
				break;
			case '-':
				if (i + 1 < n && myIsDigit(source[i + 1])) {
					i++;
					std::string num = "-";
					while (i < n && myIsDigit(source[i])) {
						num.push_back(source[i]);
						i++;
					}
					ret.push(new I(std::stoi(num)));
				} else {
					ret.push(new K("-"));
					i++;
				}
				break;
			case '*':
				ret.push(new K("*"));
				i++;
				break;
			case '/':
				ret.push(new K("/"));
				i++;
				break;
			case '&':
				if (i + 1 < n && source[i + 1] == '&') 	{
					ret.push(new K("&&"));
					i += 2;
				} else {
					std::cerr << "unrecognized character" << std::endl;
					std::exit(EXIT_FAILURE);
				}
				break;
			case '|':
				if (i + 1 < n && source[i + 1] == '|') {
					ret.push(new K("||"));
					i += 2;
				} else {
					std::cerr << "unrecognized character" << std::endl;
					std::exit(EXIT_FAILURE);
				}
				break;
			case '!':
				ret.push(new K("!"));
				i++;
				break;
			case '=':
				ret.push(new K("="));
				i++;
				break;
			default: // nonnegative digits
				std::string num;
				while (myIsDigit(source[i])) {
					num.push_back(source[i]);
					i++;
				}
				if (num.size() == 0) {
					std::cerr << "unrecognized character" << std::endl;
					std::exit(EXIT_FAILURE);
				} else {
					ret.push(new I(std::stoi(num)));
				}
				break;
			}
		}
	}
	return ret;
}

/*

<expr> := <variable> # any non-empty alphabetic sequences except for boolean literals and keywords :: Var
        | <integer> :: Int
        | <boolean> :: Bool
        | ( + <expr1> <expr2> ) :: Add
        | ( - <expr1> <expr2> ) :: Sub
        | ( * <expr1> <expr2> ) :: Mul
        | ( / <expr1> <expr2> ) :: Div
        | ( && <expr1> <expr2> ) :: And
        | ( || <expr1> <expr2> ) :: Or
        | ( ! <expr> ) :: Not
        | ( if <expr1> then <expr2> else <expr3> ) :: If
        | ( let <variable> = <expr1> in <expr2> ) :: Let
 */

Node *parseHead(std::queue<Token*> &q) {
	Token *cur = q.top();
	q.pop();
	if (cur->getType() == "N") { // Var
	} else if (cur->getType() == "I") { // Int
	} else if (cur->getType() == "B") { // Bool
	} else if (cur->getType() == "K") { // (
	} else {
		std::cerr << "ERROR1" << std::endl;
		std::exit(EXIT_FAILURE);
	}
}

Node *parseTail(std::queue<Token*> &q) {
}

AST parse(const std::string &source) {
	std::queue<Token*> q = tokenize(source);
	return AST(parseHead(q));
}

std::pair<bool, std::map<std::string, std::string>> typecheck(const AST &ast) {
}

std::pair<std::string, std::pair<bool, int>> eval(const AST &ast) {
}

int main() {
	std::string line;
	while (true) {
		getline(std::cin, line);
		AST ast = parse(tokenize(line));
		auto p1 = typecheck(ast);
		if (p1.first) {
			auto typeMap = p1.second;
			for (auto p2 : typeMap) {
				std::cout << p2.first << " :: " << p2.second << std::endl;
			}
			auto p3 = eval(ast);
			if (p3.first == "BOOL") {
				std::cout << "Got BOOL value: " << p3.second.first << std::endl;
			} else {
				std::cout << "Got INT value: " << p3.second.second << std::endl;
			}
		} else {
			std::cerr << "Typecheck failed!" << std::endl;
		}
	}
}

/*

# Grammar (LL1)
<expr> := <variable> # any non-empty alphabetic sequences except for boolean literals and keywords
        | <integer>
        | <boolean>
        | ( + <expr1> <expr2> )
        | ( - <expr1> <expr2> )
        | ( * <expr1> <expr2> )
        | ( / <expr1> <expr2> )
        | ( && <expr1> <expr2> )
        | ( || <expr1> <expr2> )
        | ( ! <expr> )
        | ( if <expr1> then <expr2> else <expr3> )
        | ( let <variable> = <expr1> in <expr2> )

# Base Values
<integer> := 0 | 1 | -1 | ... # - 1 is invalid.
<boolean> := true | false

# Type Variables
[<variable>]
[<integer>]
[<boolean>]
[<expr>]

# Type Constraints
<variable>                               : [<variable>]
<integer>                                : [<integer>] = INT
<boolean>                                : [<boolean>] = BOOL
( + <expr1> <expr2> )                    : [<expr1>] = [<expr2>] = INT
( - <expr1> <expr2> )                    : [<expr1>] = [<expr2>] = INT
( * <expr1> <expr2> )                    : [<expr1>] = [<expr2>] = INT
( / <expr1> <expr2> )                    : [<expr1>] = [<expr2>] = INT
( && <expr1> <expr2> )                   : [<expr1>] = [<expr2>] = BOOL
( || <expr1> <expr2> )                   : [<expr1>] = [<expr2>] = BOOL
( ! <expr> )                             : [<expr>] = BOOL
( if <expr1> then <expr2> else <expr3> ) : [<expr1>] = BOOL; [<expr2>] = [<expr3>]
( let <variable> = <expr1> in <expr2> )  : [<variable>] = [<expr1>]

*/
