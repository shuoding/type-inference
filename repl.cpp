/*

# Grammar (LL1)
<expr> := <variable> # any non-empty alphabetic sequences except for boolean literals and keywords
        | <integer>
        | <boolean>
        | ( - <expr1> <expr2> )
        | ( * <expr1> <expr2> )
        | ( / <expr1> <expr2> )
        | ( < <expr1> <expr2> )
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

# Type Constraints ([] represents the whole expression)
<variable>                               : [] = x
<integer>                                : [] = INT
<boolean>                                : [] = BOOL
( - <expr1> <expr2> )                    : [<expr1>] = [<expr2>] = INT, [] = INT
( * <expr1> <expr2> )                    : [<expr1>] = [<expr2>] = INT, [] = INT
( / <expr1> <expr2> )                    : [<expr1>] = [<expr2>] = INT, [] = INT
( < <expr1> <expr2> )                    : [<expr1>] = [<expr2>] = INT, [] = BOOL
( if <expr1> then <expr2> else <expr3> ) : [<expr1>] = BOOL; [<expr2>] = [<expr3>], [] = [<expr2>]
( let <variable> = <expr1> in <expr2> )  : [<variable>] = [<expr1>], [] = [<expr2>]

*/

/*

We only defined - * / < if, because other common operators can be easily implemented by these 5 things:

(+ a b) := (- a (- 0 b))
(&& <expr1> <expr2>) := (if <expr1> then <expr2> else false)
(|| <expr1> <expr2>) := (if <expr1> then true else <expr2>)
(! <expr>) := (if <expr> then false else true)
(<= a b) := (! (< b a))
(> a b) := (< b a)
(>= a b) := (<= b a)
(== a b) := (&& (! (< a b)) (! (< b a)))
(!= a b) := (! (== a b))

*/

#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <utility>
#include <memory>
#include <cctype>
#include <cstdlib>
#include <queue>

// ================================================== tokenizing =================================================

struct Token {
	Token() {}
	virtual std::string getType() { return "Token"; }
	virtual std::string getLiteral() { return ""; }
	virtual ~Token() {}
};

struct N : public Token { // Variable Name
	N(const std::string &val0) : val(val0) {}
	std::string getType() override { return "N"; }
	std::string getLiteral() override { return val; }
	~N() override {}
	std::string val;
};

struct B : public Token { // Boolean Literal
	B(bool val0) : val(val0) {}
	std::string getType() override { return "B"; }
	std::string getLiteral() override { if (val) return "true"; else return "false"; }
	~B() override {}
	bool val;
};

struct I : public Token { // Integer Literal
	I(int val0) : val(val0) {}
	std::string getType() override { return "I"; }
	std::string getLiteral() override { return std::to_string(val); }
	~I() override {}
	int val;
};

struct K : public Token { // Reserved Keyword
	K(const std::string &val0) : val(val0) {}
	std::string getType() override { return "K"; }
	std::string getLiteral() override { return val; }
	~K() override {}
	std::string val;
};

/*

variable names: [a-zA-Z]+
boolean literal: true false
integer literal: -?[0-9]+
reserved keywords: ( ) - * / < if then else let = in

*/

bool isa(char ch) { return std::isalpha(static_cast<unsigned char>(ch)); }

bool isd(char ch) { return std::isdigit(static_cast<unsigned char>(ch)); }

bool iss(char ch) { return std::isspace(static_cast<unsigned char>(ch)); }

std::queue<Token*> tokenize(const std::string &source) {
	std::queue<Token*> ret;
	int n = source.size();
	int i = 0;
	while (i < n) {
		if (iss(source[i])) { // ignore all whitespace characters
			i++;
			continue;
		}
		if (isa(source[i])) { // starting with English letters
			std::string word;
			while (i < n && isa(source[i])) {
				word.push_back(source[i++]);
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
			case '-': // operator or negative sign
				if (i + 1 < n && isd(source[i + 1])) {
					i++;
					std::string num = "-";
					while (i < n && isd(source[i])) {
						num.push_back(source[i++]);
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
			case '<':
				ret.push(new K("<"));
				i++;
				break;
			case '=':
				ret.push(new K("="));
				i++;
				break;
			default: // nonnegative digits
				std::string num;
				while (isd(source[i])) {
					num.push_back(source[i++]);
				}
				if (num.size() == 0) {
					std::cerr << "unrecognized character: " << source[i] << std::endl;
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

void printTokens(const std::queue<Token*> &tokens) {
	std::queue<Token*> ts = tokens;
	while (!ts.empty()) {
		auto t  = ts.front();
		std::cout << t->getLiteral() << std::endl;
		ts.pop();
	}
}

// =========================================== parsing ================================================

struct Node {
	Node() {}
	virtual std::string getType() { return "Node"; }
	virtual std::string getLiteral() { return ""; }
	virtual ~Node() {}
};

struct Var : public Node {
	Var(const std::string &val0) : val(val0) {}
	std::string getType() override { return "Var"; }
	std::string getLiteral() override { return "[Var " + val + "]"; }
	~Var() override {}
	std::string val;
};

struct Int : public Node {
	Int(int val0) : val(val0) {}
	std::string getType() override { return "Int"; }
	std::string getLiteral() override { return "[Int " + std::to_string(val) + "]"; }
	~Int() override {}
	int val;
};

struct Bool : public Node {
	Bool(bool val0) : val(val0) {}
	std::string getType() override { return "Bool"; }
	std::string getLiteral() override { return "[Bool " + std::string(val ? "true" : "false") + "]"; }
	~Bool() override {}
	bool val;
};

struct Sub : public Node {
	Sub(Node *n10, Node *n20) : n1(n10), n2(n20) {}
	std::string getType() override { return "Sub"; }
	std::string getLiteral() override { return "[Sub " + n1->getLiteral() + " " + n2->getLiteral() + "]"; }
	~Sub() override { delete n1; delete n2; }
	Node *n1, *n2;
};

struct Mul : public Node {
	Mul(Node *n10, Node *n20) : n1(n10), n2(n20) {}
	std::string getType() override { return "Mul"; }
	std::string getLiteral() override { return "[Mul " + n1->getLiteral() + " " + n2->getLiteral() + "]"; }
	~Mul() override { delete n1; delete n2; }
	Node *n1, *n2;
};

struct Div : public Node {
	Div(Node *n10, Node *n20) : n1(n10), n2(n20) {}
	std::string getType() override { return "Div"; }
	std::string getLiteral() override { return "[Div " + n1->getLiteral() + " " + n2->getLiteral() + "]"; }
	~Div() override { delete n1; delete n2; }
	Node *n1, *n2;
};

struct Lt : public Node {
	Lt(Node *n10, Node *n20) : n1(n10), n2(n20) {}
	std::string getType() override { return "Lt"; }
	std::string getLiteral() override { return "[Lt " + n1->getLiteral() + " " + n2->getLiteral() + "]"; }
	~Lt() override { delete n1; delete n2; }
	Node *n1, *n2;
};

struct If : public Node {
	If(Node *n10, Node *n20, Node *n30) : n1(n10), n2(n20), n3(n30) {}
	std::string getType() override { return "If"; }
	std::string getLiteral() override { return "[If " + n1->getLiteral() + " " + n2->getLiteral() + " " + n3->getLiteral() + "]"; }
	~If() override { delete n1; delete n2; delete n3; }
	Node *n1, *n2, *n3;
};

struct Let : public Node {
	Let(Node *n10, Node *n20, Node *n30) : n1(n10), n2(n20), n3(n30) {}
	std::string getType() override { return "Let"; }
	std::string getLiteral() override { return "[Let " + n1->getLiteral() + " " + n2->getLiteral() + " " + n3->getLiteral() + "]"; }
	~Let() override { delete n1; delete n2; delete n3; }
	Node *n1, *n2, *n3;
};

/*

<expr> := <variable> # any non-empty alphabetic sequences except for boolean literals and keywords :: Var
        | <integer> :: Int
        | <boolean> :: Bool
        | ( - <expr1> <expr2> ) :: Sub
        | ( * <expr1> <expr2> ) :: Mul
        | ( / <expr1> <expr2> ) :: Div
        | ( < <expr1> <expr2> ) :: Lt
        | ( if <expr1> then <expr2> else <expr3> ) :: If
        | ( let <variable> = <expr1> in <expr2> ) :: Let
 */

Node *parseHead(std::queue<Token*> &q);
Node *parseTail(std::queue<Token*> &q);

Node *parseHead(std::queue<Token*> &q) {
	if (q.empty()) {
		std::cerr << "syntax error: <expr> cannot be empty" << std::endl;
		std::exit(EXIT_FAILURE);
	}
	Token *cur = q.front();
	q.pop();
	if (cur->getType() == "N") { // Var
		return new Var(cur->getLiteral());
	} else if (cur->getType() == "I") { // Int
		return new Int(std::stoi(cur->getLiteral()));
	} else if (cur->getType() == "B") { // Bool
		if (cur->getLiteral() == "true") {
			return new Bool(true);
		} else {
			return new Bool(false);
		}
	} else if (cur->getType() == "K") { // (
		if (cur->getLiteral() == "(") {
			return parseTail(q);
		} else {
			std::cerr << "syntax error: <expr> cannot start with token " << cur->getLiteral() << std::endl;
			std::exit(EXIT_FAILURE);
		}
	} else {
		std::cerr << "syntax error: <expr> cannot start with token type" << cur->getType() << std::endl;
		std::exit(EXIT_FAILURE);
	}
}

Node *parseTail(std::queue<Token*> &q) {
	if (q.empty()) {
		std::cerr << "syntax error: <expr> cannot be (" << std::endl;
		std::exit(EXIT_FAILURE);
	}
	Token *cur = q.front();
	q.pop();
	if (cur->getLiteral() == "-") { // ( - <expr1> <expr2> )
		auto n1 = parseHead(q);
		auto n2 = parseHead(q);
		if (q.empty()) {
			std::cerr << "syntax error: missing )" << std::endl;
			std::exit(EXIT_FAILURE);
		} else {
			auto r = q.front();
			q.pop();
			if (r->getLiteral() != ")") {
				std::cerr << "syntax error: missing )" << std::endl;
				std::exit(EXIT_FAILURE);
			}
		}
		return new Sub(n1, n2);
	} else if (cur->getLiteral() == "*") { // ( * <expr1> <expr2> )
		auto n1 = parseHead(q);
		auto n2 = parseHead(q);
		if (q.empty()) {
			std::cerr << "syntax error: missing )" << std::endl;
			std::exit(EXIT_FAILURE);
		} else {
			auto r = q.front();
			q.pop();
			if (r->getLiteral() != ")") {
				std::cerr << "syntax error: missing )" << std::endl;
				std::exit(EXIT_FAILURE);
			}
		}
		return new Mul(n1, n2);
	} else if (cur->getLiteral() == "/") { // ( / <expr1> <expr2> )
		auto n1 = parseHead(q);
		auto n2 = parseHead(q);
		if (q.empty()) {
			std::cerr << "syntax error: missing )" << std::endl;
			std::exit(EXIT_FAILURE);
		} else {
			auto r = q.front();
			q.pop();
			if (r->getLiteral() != ")") {
				std::cerr << "syntax error: missing )" << std::endl;
				std::exit(EXIT_FAILURE);
			}
		}
		return new Div(n1, n2);
	} else if (cur->getLiteral() == "<") { // ( < <expr1> <expr2> )
		auto n1 = parseHead(q);
		auto n2 = parseHead(q);
		if (q.empty()) {
			std::cerr << "syntax error: missing )" << std::endl;
			std::exit(EXIT_FAILURE);
		} else {
			auto r = q.front();
			q.pop();
			if (r->getLiteral() != ")") {
				std::cerr << "syntax error: missing )" << std::endl;
				std::exit(EXIT_FAILURE);
			}
		}
		return new Lt(n1, n2);
	} else if (cur->getLiteral() == "if") { // ( if <expr1> then <expr2> else <expr3> )
		auto n1 = parseHead(q);
		if (q.empty()) {
			std::cerr << "syntax error: missing 'then'" << std::endl;
			std::exit(EXIT_FAILURE);
		} else {
			auto t = q.front();
			q.pop();
			if (t->getLiteral() != "then") {
				std::cerr << "syntax error: missing 'then'" << std::endl;
				std::exit(EXIT_FAILURE);
			}
		}
		auto n2 = parseHead(q);
		if (q.empty()) {
			std::cerr << "syntax error: missing 'else'" << std::endl;
			std::exit(EXIT_FAILURE);
		} else {
			auto t = q.front();
			q.pop();
			if (t->getLiteral() != "else") {
				std::cerr << "syntax error: missing 'else'" << std::endl;
				std::exit(EXIT_FAILURE);
			}
		}
		auto n3 = parseHead(q);
		if (q.empty()) {
			std::cerr << "syntax error: missing )" << std::endl;
			std::exit(EXIT_FAILURE);
		} else {
			auto r = q.front();
			q.pop();
			if (r->getLiteral() != ")") {
				std::cerr << "syntax error: missing )" << std::endl;
				std::exit(EXIT_FAILURE);
			}
		}
		return new If(n1, n2, n3);
	} else if (cur->getLiteral() == "let") { // ( let <variable> = <expr1> in <expr2> )
		auto n1 = parseHead(q);
		if (n1->getType() != "Var") {
			std::cerr << "syntax error: A variable must follow 'let'." << std::endl;
			std::exit(EXIT_FAILURE);
		}
		if (q.empty()) {
			std::cerr << "syntax error: missing =" << std::endl;
			std::exit(EXIT_FAILURE);
		} else {
			auto r = q.front();
			q.pop();
			if (r->getLiteral() != "=") {
				std::cerr << "syntax error: missing =" << std::endl;
				std::exit(EXIT_FAILURE);
			}
		}
		auto n2 = parseHead(q);
		if (q.empty()) {
			std::cerr << "syntax error: missing 'in'" << std::endl;
			std::exit(EXIT_FAILURE);
		} else {
			auto r = q.front();
			q.pop();
			if (r->getLiteral() != "in") {
				std::cerr << "syntax error: missing 'in'" << std::endl;
				std::exit(EXIT_FAILURE);
			}
		}
		auto n3 = parseHead(q);
		if (q.empty()) {
			std::cerr << "syntax error: missing )" << std::endl;
			std::exit(EXIT_FAILURE);
		} else {
			auto r = q.front();
			q.pop();
			if (r->getLiteral() != ")") {
				std::cerr << "syntax error: missing )" << std::endl;
				std::exit(EXIT_FAILURE);
			}
		}
		return new Let(n1, n2, n3);
	} else {
		std::cerr << "syntax error: <expr> cannot start with ( " << cur->getLiteral() << std::endl;
		std::exit(EXIT_FAILURE);
	}
}

Node *parse(const std::queue<Token*> &tokens) {
	std::queue<Token*> q = tokens;
	return parseHead(q);
}

void printAST(Node *root) {
	std::cout << root->getLiteral() << std::endl;
}

// =========================================== type inference and type check ==========================================

/*
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
};

std::pair<bool, std::map<std::string, std::string>> typecheck(const AST &ast) {
}

*/

// ============================================== evaluation ==========================================

/*

std::pair<std::string, std::pair<bool, int>> eval(const AST &ast) {
}

*/

int main() {
	std::string line;
	while (true) {
		getline(std::cin, line);
		auto tokens = tokenize(line);
		auto root = parse(tokens);
		printAST(root);
		while (!tokens.empty()) {
			auto t = tokens.front();
			delete t;
			tokens.pop();
		}
		delete root;
		/*
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
		*/
	}
}
