/*
 * # Grammar (LL1)
 * <expr> := <variable> # any non-empty alphabetic sequences except for boolean literals and keywords
 *                      # duplicate variable names are not supported
 *         | <integer> # 0 | 1 | -1 | ...
 *                     # - 1 is invalid. The digits must immediately follow the negative sign.
 *         | <boolean> # true | false
 *         | ( - <expr1> <expr2> )
 *         | ( * <expr1> <expr2> )
 *         | ( / <expr1> <expr2> )
 *         | ( < <expr1> <expr2> )
 *         | ( if <expr1> then <expr2> else <expr3> )
 *         | ( let <variable> = <expr1> in <expr2> )
 *
 * # Type Constraints ([] represents the whole expression)
 * <variable>                               :
 * <integer>                                : [] = INT
 * <boolean>                                : [] = BOOL
 * ( - <expr1> <expr2> )                    : [] = INT, [<expr1>] = [<expr2>] = INT
 * ( * <expr1> <expr2> )                    : [] = INT, [<expr1>] = [<expr2>] = INT
 * ( / <expr1> <expr2> )                    : [] = INT, [<expr1>] = [<expr2>] = INT
 * ( < <expr1> <expr2> )                    : [] = BOOL, [<expr1>] = [<expr2>] = INT
 * ( if <expr1> then <expr2> else <expr3> ) : [] = [<expr2>], [<expr1>] = BOOL, [<expr2>] = [<expr3>]
 * ( let <variable> = <expr1> in <expr2> )  : [] = [<expr2>], [<variable>] = [<expr1>]
 */

/*
 * Other common operators can be easily implemented:
 * (+ a b) := (- a (- 0 b))
 * (&& <expr1> <expr2>) := (if <expr1> then <expr2> else false)
 * (|| <expr1> <expr2>) := (if <expr1> then true else <expr2>)
 * (! <expr>) := (if <expr> then false else true)
 * (<= a b) := (! (< b a))
 * (> a b) := (< b a)
 * (>= a b) := (<= b a)
 * (== a b) := (&& (! (< a b)) (! (< b a)))
 * (!= a b) := (! (== a b))
 * ...
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

// print an error message and quit
void die(const std::string &s) {
	std::cerr << s << std::endl;
	std::exit(EXIT_FAILURE);
}

// ================================================== tokenizing =================================================

struct Token {
	Token() {}
	virtual std::string getType() {
		return "Token";
	}
	virtual std::string getLiteral() {
		return "";
	}
	virtual ~Token() {}
};

// variable name
struct N : public Token {
	N(const std::string &val0) : val(val0) {}
	std::string getType() override {
		return "N";
	}
	std::string getLiteral() override {
		return val;
	}
	~N() override {}

	std::string val;
};

// integer literal
struct I : public Token {
	I(int val0) : val(val0) {}
	std::string getType() override {
		return "I";
	}
	std::string getLiteral() override {
		return std::to_string(val);
	}
	~I() override {}

	int val;
};

// boolean literal
struct B : public Token {
	B(bool val0) : val(val0) {}
	std::string getType() override {
		return "B";
	}
	std::string getLiteral() override {
		if (val) {
			return "true";
		} else {
			return "false";
		}
	}
	~B() override {}

	bool val;
};

// reserved token
struct K : public Token {
	K(const std::string &val0) : val(val0) {}
	std::string getType() override {
		return "K";
	}
	std::string getLiteral() override {
		return val;
	}
	~K() override {}

	std::string val;
};

/*
 * variable names: [a-zA-Z]+
 * boolean literal: true | false
 * integer literal: -?[0-9]+
 * reserved tokens: ( ) - * / < if then else let = in
 */

// is alphabetic or not
bool isa(char ch) {
	return std::isalpha(static_cast<unsigned char>(ch));
}

// is digit or not
bool isd(char ch) {
	return std::isdigit(static_cast<unsigned char>(ch));
}

// is whitespace or not
bool iss(char ch) {
	return std::isspace(static_cast<unsigned char>(ch));
}

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
			} else { // Actually, several previous branches can be merged to this one.
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
			case '-': // the subtraction operator or the negative sign
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
			default: // nonnegative digits or other characters
				std::string num;
				while (isd(source[i])) {
					num.push_back(source[i++]);
				}
				if (num.size() == 0) { // other characters
					die(std::string("Token Error: unrecognized character '")
						+ source[i]
						+ std::string("' at position ")
						+ std::to_string(i));
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
		auto t = ts.front();
		std::cout << t->getLiteral() << std::endl;
		ts.pop();
	}
}

// =========================================== parsing ================================================

struct Node {
	Node() {}
	virtual std::string getType() {
		return "Node";
	}
	virtual std::string getLiteral() {
		return "";
	}
	virtual ~Node() {}

	// the pre-order BFS number
	int number = -1;
};

struct Var : public Node {
	Var(const std::string &val0) : val(val0) {}
	std::string getType() override {
		return "Var";
	}
	std::string getLiteral() override {
		return "[Var " + val + "]";
	}
	~Var() override {}

	std::string val;
};

struct Int : public Node {
	Int(int val0) : val(val0) {}
	std::string getType() override {
		return "Int";
	}
	std::string getLiteral() override {
		return "[Int " + std::to_string(val) + "]";
	}
	~Int() override {}

	int val;
};

struct Bool : public Node {
	Bool(bool val0) : val(val0) {}
	std::string getType() override {
		return "Bool";
	}
	std::string getLiteral() override {
		return "[Bool " + std::string(val ? "true" : "false") + "]";
	}
	~Bool() override {}

	bool val;
};

struct Sub : public Node {
	Sub(Node *n10, Node *n20) : n1(n10), n2(n20) {}
	std::string getType() override {
		return "Sub";
	}
	std::string getLiteral() override {
		return "[Sub " + n1->getLiteral() + " " + n2->getLiteral() + "]";
	}
	~Sub() override {
		delete n1;
		delete n2;
	}

	Node *n1, *n2;
};

struct Mul : public Node {
	Mul(Node *n10, Node *n20) : n1(n10), n2(n20) {}
	std::string getType() override {
		return "Mul";
	}
	std::string getLiteral() override {
		return "[Mul " + n1->getLiteral() + " " + n2->getLiteral() + "]";
	}
	~Mul() override {
		delete n1;
		delete n2;
	}

	Node *n1, *n2;
};

struct Div : public Node {
	Div(Node *n10, Node *n20) : n1(n10), n2(n20) {}
	std::string getType() override {
		return "Div";
	}
	std::string getLiteral() override {
		return "[Div " + n1->getLiteral() + " " + n2->getLiteral() + "]";
	}
	~Div() override {
		delete n1;
		delete n2;
	}

	Node *n1, *n2;
};

struct Lt : public Node {
	Lt(Node *n10, Node *n20) : n1(n10), n2(n20) {}
	std::string getType() override {
		return "Lt";
	}
	std::string getLiteral() override {
		return "[Lt " + n1->getLiteral() + " " + n2->getLiteral() + "]";
	}
	~Lt() override {
		delete n1;
		delete n2;
	}

	Node *n1, *n2;
};

struct If : public Node {
	If(Node *n10, Node *n20, Node *n30) : n1(n10), n2(n20), n3(n30) {}
	std::string getType() override {
		return "If";
	}
	std::string getLiteral() override {
		return "[If " + n1->getLiteral() + " " + n2->getLiteral() + " " + n3->getLiteral() + "]";
	}
	~If() override {
		delete n1;
		delete n2;
		delete n3;
	}

	Node *n1, *n2, *n3;
};

struct Let : public Node {
	Let(Node *n10, Node *n20, Node *n30) : n1(n10), n2(n20), n3(n30) {}
	std::string getType() override {
		return "Let";
	}
	std::string getLiteral() override {
		return "[Let " + n1->getLiteral() + " " + n2->getLiteral() + " " + n3->getLiteral() + "]";
	}
	~Let() override {
		delete n1;
		delete n2;
		delete n3;
	}

	Node *n1, *n2, *n3;
};

/*
 * <expr> := <variable> # any non-empty alphabetic sequences except for boolean literals and keywords
 *                      # duplicate variable names are not supported
 *         | <integer> # 0 | 1 | -1 | ...
 *                     # - 1 is invalid. The digits must immediately follow the negative sign.
 *         | <boolean> # true | false
 *         | ( - <expr1> <expr2> )
 *         | ( * <expr1> <expr2> )
 *         | ( / <expr1> <expr2> )
 *         | ( < <expr1> <expr2> )
 *         | ( if <expr1> then <expr2> else <expr3> )
 *         | ( let <variable> = <expr1> in <expr2> )
 */

// mutually recursive functions
Node *parseHead(std::queue<Token*> &q);
Node *parseTail(std::queue<Token*> &q);

Node *parseHead(std::queue<Token*> &q) {
	if (q.empty()) {
		die("Syntax Error: Expressions and subexpressions cannot be empty.");
	}
	Token *cur = q.front();
	q.pop();
	if (cur->getType() == "N") { // <variable>
		return new Var(cur->getLiteral());
	} else if (cur->getType() == "I") { // <integer>
		return new Int(std::stoi(cur->getLiteral()));
	} else if (cur->getType() == "B") { // <boolean>
		if (cur->getLiteral() == "true") {
			return new Bool(true);
		} else {
			return new Bool(false);
		}
	} else if (cur->getType() == "K") { // left parenthesis (
		if (cur->getLiteral() == "(") {
			return parseTail(q);
		} else {
			die("Syntax Error: Expressions and subexpressions cannot start with token " + cur->getLiteral());
		}
	} else { // This branch is for the Token type.
		die("Syntax Error: Expressions and subexpressions cannot start with token type " + cur->getType());
	}
}

Node *parseTail(std::queue<Token*> &q) {
	if (q.empty()) {
		die("Syntax Error: Expressions and subexpressions cannot be (.");
	}
	Token *cur = q.front();
	q.pop();
	if (cur->getLiteral() == "-") { // ( - <expr1> <expr2> )
		auto n1 = parseHead(q);
		auto n2 = parseHead(q);
		if (q.empty()) {
			die("Syntax Error: missing ) in (- <expr1> <expr2>)");
		} else {
			auto r = q.front();
			q.pop();
			if (r->getLiteral() != ")") {
				die("Syntax Error: missing ) in (- <expr1> <expr2>)");
			}
		}
		return new Sub(n1, n2);
	} else if (cur->getLiteral() == "*") { // ( * <expr1> <expr2> )
		auto n1 = parseHead(q);
		auto n2 = parseHead(q);
		if (q.empty()) {
			die("Syntax Error: missing ) in (* <expr1> <expr2>)");
		} else {
			auto r = q.front();
			q.pop();
			if (r->getLiteral() != ")") {
				die("Syntax Error: missing ) in (* <expr1> <expr2>)");
			}
		}
		return new Mul(n1, n2);
	} else if (cur->getLiteral() == "/") { // ( / <expr1> <expr2> )
		auto n1 = parseHead(q);
		auto n2 = parseHead(q);
		if (q.empty()) {
			die("Syntax Error: missing ) in (/ <expr1> <expr2>)");
		} else {
			auto r = q.front();
			q.pop();
			if (r->getLiteral() != ")") {
				die("Syntax Error: missing ) in (/ <expr1> <expr2>)");
			}
		}
		return new Div(n1, n2);
	} else if (cur->getLiteral() == "<") { // ( < <expr1> <expr2> )
		auto n1 = parseHead(q);
		auto n2 = parseHead(q);
		if (q.empty()) {
			die("Syntax Error: missing ) in (< <expr1> <expr2>)");
		} else {
			auto r = q.front();
			q.pop();
			if (r->getLiteral() != ")") {
				die("Syntax Error: missing ) in (< <expr1> <expr2>)");
			}
		}
		return new Lt(n1, n2);
	} else if (cur->getLiteral() == "if") { // ( if <expr1> then <expr2> else <expr3> )
		auto n1 = parseHead(q);
		if (q.empty()) {
			die("Syntax Error: missing 'then' in (if <expr1> then <expr2> else <expr3>)");
		} else {
			auto t = q.front();
			q.pop();
			if (t->getLiteral() != "then") {
				die("Syntax Error: missing 'then' in (if <expr1> then <expr2> else <expr3>)");
			}
		}
		auto n2 = parseHead(q);
		if (q.empty()) {
			die("Syntax Error: missing 'else' in (if <expr1> then <expr2> else <expr3>)");
		} else {
			auto t = q.front();
			q.pop();
			if (t->getLiteral() != "else") {
				die("Syntax Error: missing 'else' in (if <expr1> then <expr2> else <expr3>)");
			}
		}
		auto n3 = parseHead(q);
		if (q.empty()) {
			die("Syntax Error: missing ) in (if <expr1> then <expr2> else <expr3>)");
		} else {
			auto r = q.front();
			q.pop();
			if (r->getLiteral() != ")") {
				die("Syntax Error: missing ) in (if <expr1> then <expr2> else <expr3>)");
			}
		}
		return new If(n1, n2, n3);
	} else if (cur->getLiteral() == "let") { // ( let <variable> = <expr1> in <expr2> )
		auto n1 = parseHead(q);
		if (n1->getType() != "Var") {
			die("Syntax Error: The token following 'let' must be a variable.");
		}
		if (q.empty()) {
			die("Syntax Error: missing = in (let <variable> = <expr1> in <expr2>)");
		} else {
			auto r = q.front();
			q.pop();
			if (r->getLiteral() != "=") {
				die("Syntax Error: missing = in (let <variable> = <expr1> in <expr2>)");
			}
		}
		auto n2 = parseHead(q);
		if (q.empty()) {
			die("Syntax Error: missing 'in' in (let <variable> = <expr1> in <expr2>)");
		} else {
			auto r = q.front();
			q.pop();
			if (r->getLiteral() != "in") {
				die("Syntax Error: missing 'in' in (let <variable> = <expr1> in <expr2>)");
			}
		}
		auto n3 = parseHead(q);
		if (q.empty()) {
			die("Syntax Error: missing ) in (let <variable> = <expr1> in <expr2>)");
		} else {
			auto r = q.front();
			q.pop();
			if (r->getLiteral() != ")") {
				die("Syntax Error: missing ) in (let <variable> = <expr1> in <expr2>)");
			}
		}
		return new Let(n1, n2, n3);
	} else {
		die("Syntax Error: Expressions and subexpressions cannot start with ( and " + cur->getLiteral());
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

struct UnionFind {
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

	int n;
	std::vector<int> prev;
};

// This is a general function for traversing the AST and applying f to each node.
template<typename F> void dfs(Node *root, F f) {
	f(root);
	if (root->getType() == "Sub") {
		auto r = dynamic_cast<Sub*>(root);
		if (r == nullptr) {
			die("Internal Error: dynamic_cast failed from Node* to Sub*");
		}
		dfs(r->n1, f);
		dfs(r->n2, f);
	} else if (root->getType() == "Mul") {
		auto r = dynamic_cast<Mul*>(root);
		if (r == nullptr) {
			die("Internal Error: dynamic_cast failed from Node* to Mul*");
		}
		dfs(r->n1, f);
		dfs(r->n2, f);
	} else if (root->getType() == "Div") {
		auto r = dynamic_cast<Div*>(root);
		if (r == nullptr) {
			die("Internal Error: dynamic_cast failed from Node* to Div*");
		}
		dfs(r->n1, f);
		dfs(r->n2, f);
	} else if (root->getType() == "Lt") {
		auto r = dynamic_cast<Lt*>(root);
		if (r == nullptr) {
			die("Internal Error: dynamic_cast failed from Node* to Lt*");
		}
		dfs(r->n1, f);
		dfs(r->n2, f);
	} else if (root->getType() == "If") {
		auto r = dynamic_cast<If*>(root);
		if (r == nullptr) {
			die("Internal Error: dynamic_cast failed from Node* to If*");
		}
		dfs(r->n1, f);
		dfs(r->n2, f);
		dfs(r->n3, f);
	} else if (root->getType() == "Let") {
		auto r = dynamic_cast<Let*>(root);
		if (r == nullptr) {
			die("Internal Error: dynamic_cast failed from Node* to Let*");
		}
		dfs(r->n1, f);
		dfs(r->n2, f);
		dfs(r->n3, f);
	}
}

// This function does both type inference and type check.
std::map<std::string, std::string> typecheck(Node *root) {
	// assign numbers to AST nodes
	int counter = 0;
	std::map<std::string, int> variable_number_map;
	auto assign_numbers = [&counter, &variable_number_map](Node *cur) -> void {
		if (cur->getType() == "Var") { // Different occurances of the same variable share the same number.
			auto c = dynamic_cast<Var*>(cur);
			if (c == nullptr) {
				die("Internal Error: dynamic_cast failed from Node* to Var*");
			}
			if (variable_number_map.count(c->val) == 0) {
				c->number = counter++;
				variable_number_map[c->val] = c->number;
			} else {
				c->number = variable_number_map[c->val];
			}
		} else {
			cur->number = counter++;
		}
	};
	dfs(root, assign_numbers);

	// generate constraints
	// Constraints have the form x = y, where x and y are type variables or INT or BOOL.
	/*
	 * # Type Constraints ([] represents the whole expression)
	 * <variable>                               :
	 * <integer>                                : [] = INT
	 * <boolean>                                : [] = BOOL
	 * ( - <expr1> <expr2> )                    : [] = INT, [<expr1>] = INT, [<expr2>] = INT
	 * ( * <expr1> <expr2> )                    : [] = INT, [<expr1>] = INT, [<expr2>] = INT
	 * ( / <expr1> <expr2> )                    : [] = INT, [<expr1>] = INT, [<expr2>] = INT
	 * ( < <expr1> <expr2> )                    : [] = BOOL, [<expr1>] = INT, [<expr2>] = INT
	 * ( if <expr1> then <expr2> else <expr3> ) : [] = [<expr2>], [<expr1>] = BOOL, [<expr2>] = [<expr3>]
	 * ( let <variable> = <expr1> in <expr2> )  : [] = [<expr2>], [<variable>] = [<expr1>]
	 */
#define INT (counter)
#define BOOL (counter + 1)
	std::vector<std::pair<int, int>> constraints;
	// We must capture "counter" in this lambda expression, because the macros INT and BOOL are using "counter".
	auto generate_constraints = [&counter, &constraints](Node *cur) -> void {
		if (cur->getType() == "Var") {
			// <variable> :
			;
		} else if (cur->getType() == "Int") {
			// <integer> : [] = INT
			constraints.push_back(std::make_pair(cur->number, INT));
		} else if (cur->getType() == "Bool") {
			// <boolean> : [] = BOOL
			constraints.push_back(std::make_pair(cur->number, BOOL));
		} else if (cur->getType() == "Sub") {
			// ( - <expr1> <expr2> ) : [] = INT, [<expr1>] = INT, [<expr2>] = INT
			auto c = dynamic_cast<Sub*>(cur);
			if (c == nullptr) {
				die("Internal Error: dynamic_cast failed from Node* to Sub*");
			}
			constraints.push_back(std::make_pair(c->number, INT));
			constraints.push_back(std::make_pair(c->n1->number, INT));
			constraints.push_back(std::make_pair(c->n2->number, INT));
		} else if (cur->getType() == "Mul") {
			// ( * <expr1> <expr2> ) : [] = INT, [<expr1>] = INT, [<expr2>] = INT
			auto c = dynamic_cast<Mul*>(cur);
			if (c == nullptr) {
				die("Internal Error: dynamic_cast failed from Node* to Mul*");
			}
			constraints.push_back(std::make_pair(c->number, INT));
			constraints.push_back(std::make_pair(c->n1->number, INT));
			constraints.push_back(std::make_pair(c->n2->number, INT));
		} else if (cur->getType() == "Div") {
			// ( / <expr1> <expr2> ) : [] = INT, [<expr1>] = INT, [<expr2>] = INT
			auto c = dynamic_cast<Div*>(cur);
			if (c == nullptr) {
				die("Internal Error: dynamic_cast failed from Node* to Div*");
			}
			constraints.push_back(std::make_pair(c->number, INT));
			constraints.push_back(std::make_pair(c->n1->number, INT));
			constraints.push_back(std::make_pair(c->n2->number, INT));
		} else if (cur->getType() == "Lt") {
			// ( < <expr1> <expr2> ) : [] = BOOL, [<expr1>] = INT, [<expr2>] = INT
			auto c = dynamic_cast<Lt*>(cur);
			if (c == nullptr) {
				die("Internal Error: dynamic_cast failed from Node* to Lt*");
			}
			constraints.push_back(std::make_pair(c->number, BOOL));
			constraints.push_back(std::make_pair(c->n1->number, INT));
			constraints.push_back(std::make_pair(c->n2->number, INT));
		} else if (cur->getType() == "If") {
			// ( if <expr1> then <expr2> else <expr3> ) : [] = [<expr2>], [<expr1>] = BOOL, [<expr2>] = [<expr3>]
			auto c = dynamic_cast<If*>(cur);
			if (c == nullptr) {
				die("Internal Error: dynamic_cast failed from Node* to If*");
			}
			constraints.push_back(std::make_pair(c->number, c->n2->number));
			constraints.push_back(std::make_pair(c->n1->number, BOOL));
			constraints.push_back(std::make_pair(c->n2->number, c->n3->number));
		} else if (cur->getType() == "Let") {
			// ( let <variable> = <expr1> in <expr2> ) : [] = [<expr2>], [<variable>] = [<expr1>]
			auto c = dynamic_cast<Let*>(cur);
			if (c == nullptr) {
				die("Internal Error: dynamic_cast failed from Node* to Let*");
			}
			constraints.push_back(std::make_pair(c->number, c->n3->number));
			constraints.push_back(std::make_pair(c->n1->number, c->n2->number));
		} else {
			die("Internal Error: Unknown AST node type!");
		}
	};
	dfs(root, generate_constraints);

	// a helper function
	auto is_type_variable = [&counter](int x) -> bool {
		// 0, 1, ..., counter - 1 are type variables.
		// counter is INT.
		// counter + 1 is BOOL.
		return x < counter;
	};

	// solve constraints
	UnionFind uf(counter + 2);
	for (auto p : constraints) {
		int x = p.first;
		int y = p.second;
		int rx = uf.find(x);
		int ry = uf.find(y);
		if (is_type_variable(rx) && is_type_variable(ry)) {
			uf.join(rx, ry);
		} else if (is_type_variable(rx)) { // always choose the proper type as the root
			uf.join(rx, ry);
		} else if (is_type_variable(ry)) { // always choose the proper type as the root
			uf.join(ry, rx);
		} else {
			if (rx == ry) {
				uf.join(rx, ry);
			} else {
				std::string t_rx = (rx == INT) ? "INT" : "BOOL";
				std::string t_ry = (ry == INT) ? "INT" : "BOOL";
				die("Type Error: cannot unify " + t_rx + " and " + t_ry);
			}
		}
	}

	// construct variable-type map
	std::map<std::string, std::string> ret;
	auto add_var = [&counter, &ret, &uf](Node *cur) -> void {
		if (cur->getType() == "Var") {
			auto c = dynamic_cast<Var*>(cur);
			if (c == nullptr) {
				die("Internal Error: dynamic_cast failed from Node* to Var*");
			}
			std::string t;
			if (uf.find(c->number) == INT) {
				t = "INT";
			} else if (uf.find(c->number) == BOOL) {
				t = "BOOL";
			} else {
				t = "GENERICS-" + std::to_string(uf.find(c->number));
			}
			ret[c->val] = t;
		}
	};
	dfs(root, add_var);
	return ret;
#undef INT
#undef BOOL
}

int main() {
	std::string line;
	while (true) {
		std::cout << "...> " << std::flush;
		getline(std::cin, line);
		auto tokens = tokenize(line);
		auto ast_root = parse(tokens);
		while (!tokens.empty()) { // release the tokens
			auto t = tokens.front();
			delete t;
			tokens.pop();
		}
		auto variable_type_map = typecheck(ast_root);
		for (auto p : variable_type_map) {
			std::cout << p.first << " :: " << p.second << std::endl;
		}
		delete ast_root; // release the AST
	}
}
