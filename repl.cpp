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

void die(const std::string &s) {
	std::cerr << s << std::endl;
	std::exit(EXIT_FAILURE);
}

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
					die(std::string("unrecognized character: ") + source[i]);
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
	int number = -1; // the BFS index
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
		die("syntax error: <expr> cannot be empty");
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
			die("syntax error: <expr> cannot start with token " + cur->getLiteral());
		}
	} else {
		die("syntax error: <expr> cannot start with token type" + cur->getType());
	}
}

Node *parseTail(std::queue<Token*> &q) {
	if (q.empty()) {
		die("syntax error: <expr> cannot be (");
	}
	Token *cur = q.front();
	q.pop();
	if (cur->getLiteral() == "-") { // ( - <expr1> <expr2> )
		auto n1 = parseHead(q);
		auto n2 = parseHead(q);
		if (q.empty()) {
			die("syntax error: missing )");
		} else {
			auto r = q.front();
			q.pop();
			if (r->getLiteral() != ")") {
				die("syntax error: missing )");
			}
		}
		return new Sub(n1, n2);
	} else if (cur->getLiteral() == "*") { // ( * <expr1> <expr2> )
		auto n1 = parseHead(q);
		auto n2 = parseHead(q);
		if (q.empty()) {
			die("syntax error: missing )");
		} else {
			auto r = q.front();
			q.pop();
			if (r->getLiteral() != ")") {
				die("syntax error: missing )");
			}
		}
		return new Mul(n1, n2);
	} else if (cur->getLiteral() == "/") { // ( / <expr1> <expr2> )
		auto n1 = parseHead(q);
		auto n2 = parseHead(q);
		if (q.empty()) {
			die("syntax error: missing )");
		} else {
			auto r = q.front();
			q.pop();
			if (r->getLiteral() != ")") {
				die("syntax error: missing )");
			}
		}
		return new Div(n1, n2);
	} else if (cur->getLiteral() == "<") { // ( < <expr1> <expr2> )
		auto n1 = parseHead(q);
		auto n2 = parseHead(q);
		if (q.empty()) {
			die("syntax error: missing )");
		} else {
			auto r = q.front();
			q.pop();
			if (r->getLiteral() != ")") {
				die("syntax error: missing )");
			}
		}
		return new Lt(n1, n2);
	} else if (cur->getLiteral() == "if") { // ( if <expr1> then <expr2> else <expr3> )
		auto n1 = parseHead(q);
		if (q.empty()) {
			die("syntax error: missing 'then'");
		} else {
			auto t = q.front();
			q.pop();
			if (t->getLiteral() != "then") {
				die("syntax error: missing 'then'");
			}
		}
		auto n2 = parseHead(q);
		if (q.empty()) {
			die("syntax error: missing 'else'");
		} else {
			auto t = q.front();
			q.pop();
			if (t->getLiteral() != "else") {
				die("syntax error: missing 'else'");
			}
		}
		auto n3 = parseHead(q);
		if (q.empty()) {
			die("syntax error: missing )");
		} else {
			auto r = q.front();
			q.pop();
			if (r->getLiteral() != ")") {
				die("syntax error: missing )");
			}
		}
		return new If(n1, n2, n3);
	} else if (cur->getLiteral() == "let") { // ( let <variable> = <expr1> in <expr2> )
		auto n1 = parseHead(q);
		if (n1->getType() != "Var") {
			die("syntax error: A variable must follow 'let'.");
		}
		if (q.empty()) {
			die("syntax error: missing =");
		} else {
			auto r = q.front();
			q.pop();
			if (r->getLiteral() != "=") {
				die("syntax error: missing =");
			}
		}
		auto n2 = parseHead(q);
		if (q.empty()) {
			die("syntax error: missing 'in'");
		} else {
			auto r = q.front();
			q.pop();
			if (r->getLiteral() != "in") {
				die("syntax error: missing 'in'");
			}
		}
		auto n3 = parseHead(q);
		if (q.empty()) {
			die("syntax error: missing )");
		} else {
			auto r = q.front();
			q.pop();
			if (r->getLiteral() != ")") {
				die("syntax error: missing )");
			}
		}
		return new Let(n1, n2, n3);
	} else {
		die("syntax error: <expr> cannot start with ( " + cur->getLiteral());
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

template<typename F> void dfs(Node *root, F f) {
	f(root);
	if (root->getType() == "Sub") {
		auto r = dynamic_cast<Sub*>(root);
		if (r == nullptr) {
			die("dynamic_cast failed from Node* to Sub*");
		}
		dfs(r->n1, f);
		dfs(r->n2, f);
	} else if (root->getType() == "Mul") {
		auto r = dynamic_cast<Mul*>(root);
		if (r == nullptr) {
			die("dynamic_cast failed from Node* to Mul*");
		}
		dfs(r->n1, f);
		dfs(r->n2, f);
	} else if (root->getType() == "Div") {
		auto r = dynamic_cast<Div*>(root);
		if (r == nullptr) {
			die("dynamic_cast failed from Node* to Div*");
		}
		dfs(r->n1, f);
		dfs(r->n2, f);
	} else if (root->getType() == "Lt") {
		auto r = dynamic_cast<Lt*>(root);
		if (r == nullptr) {
			die("dynamic_cast failed from Node* to Lt*");
		}
		dfs(r->n1, f);
		dfs(r->n2, f);
	} else if (root->getType() == "If") {
		auto r = dynamic_cast<If*>(root);
		if (r == nullptr) {
			die("dynamic_cast failed from Node* to If*");
		}
		dfs(r->n1, f);
		dfs(r->n2, f);
		dfs(r->n3, f);
	} else if (root->getType() == "Let") {
		auto r = dynamic_cast<Let*>(root);
		if (r == nullptr) {
			die("dynamic_cast failed from Node* to Let*");
		}
		dfs(r->n1, f);
		dfs(r->n2, f);
		dfs(r->n3, f);
	}
}

std::map<std::string, std::string> typecheck(Node *root) {
	// assign numbers
	int counter = 0;
	std::map<std::string, int> variable_number_map;
	auto assign_numbers = [&counter, &variable_number_map](Node *cur) -> void {
		if (cur->getType() == "Var") {
			auto c = dynamic_cast<Var*>(cur);
			if (c == nullptr) {
				die("dynamic_cast failed from Node* to Var*");
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
	// Constraints have the form x = y, where x and y are type variables or -1 representing INT or -2 representing BOOL.
	/*
	
	# Type Constraints ([] represents the whole expression)
	<variable>                               : [] = x
	<integer>                                : [] = INT
	<boolean>                                : [] = BOOL
	( - <expr1> <expr2> )                    : [] = INT, [<expr1>] = INT, [<expr2>] = INT
	( * <expr1> <expr2> )                    : [] = INT, [<expr1>] = INT, [<expr2>] = INT
	( / <expr1> <expr2> )                    : [] = INT, [<expr1>] = INT, [<expr2>] = INT
	( < <expr1> <expr2> )                    : [] = BOOL, [<expr1>] = INT, [<expr2>] = INT
	( if <expr1> then <expr2> else <expr3> ) : [] = [<expr2>], [<expr1>] = BOOL; [<expr2>] = [<expr3>]
	( let <variable> = <expr1> in <expr2> )  : [] = [<expr2>], [<variable>] = [<expr1>]
	
	*/
#define INT (counter)
#define BOOL (counter + 1)
	std::vector<std::pair<int, int>> constraints;
	auto generate_constraints = [&counter, &constraints](Node *cur) -> void {
		if (cur->getType() == "Var") {
			// <variable> : [] = x
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
				die("dynamic_cast failed from Node* to Sub*");
			}
			constraints.push_back(std::make_pair(c->number, INT));
			constraints.push_back(std::make_pair(c->n1->number, INT));
			constraints.push_back(std::make_pair(c->n2->number, INT));
		} else if (cur->getType() == "Mul") {
			// ( * <expr1> <expr2> ) : [] = INT, [<expr1>] = INT, [<expr2>] = INT
			auto c = dynamic_cast<Mul*>(cur);
			if (c == nullptr) {
				die("dynamic_cast failed from Node* to Mul*");
			}
			constraints.push_back(std::make_pair(c->number, INT));
			constraints.push_back(std::make_pair(c->n1->number, INT));
			constraints.push_back(std::make_pair(c->n2->number, INT));
		} else if (cur->getType() == "Div") {
			// ( / <expr1> <expr2> ) : [] = INT, [<expr1>] = INT, [<expr2>] = INT
			auto c = dynamic_cast<Div*>(cur);
			if (c == nullptr) {
				die("dynamic_cast failed from Node* to Div*");
			}
			constraints.push_back(std::make_pair(c->number, INT));
			constraints.push_back(std::make_pair(c->n1->number, INT));
			constraints.push_back(std::make_pair(c->n2->number, INT));
		} else if (cur->getType() == "Lt") {
			// ( < <expr1> <expr2> ) : [] = BOOL, [<expr1>] = INT, [<expr2>] = INT
			auto c = dynamic_cast<Lt*>(cur);
			if (c == nullptr) {
				die("dynamic_cast failed from Node* to Lt*");
			}
			constraints.push_back(std::make_pair(c->number, BOOL));
			constraints.push_back(std::make_pair(c->n1->number, INT));
			constraints.push_back(std::make_pair(c->n2->number, INT));
		} else if (cur->getType() == "If") {
			// ( if <expr1> then <expr2> else <expr3> ) : [] = [<expr2>], [<expr1>] = BOOL; [<expr2>] = [<expr3>]
			auto c = dynamic_cast<If*>(cur);
			if (c == nullptr) {
				die("dynamic_cast failed from Node* to If*");
			}
			constraints.push_back(std::make_pair(c->number, c->n2->number));
			constraints.push_back(std::make_pair(c->n1->number, BOOL));
			constraints.push_back(std::make_pair(c->n2->number, c->n3->number));
		} else if (cur->getType() == "Let") {
			// ( let <variable> = <expr1> in <expr2> ) : [] = [<expr2>], [<variable>] = [<expr1>]
			auto c = dynamic_cast<Let*>(cur);
			if (c == nullptr) {
				die("dynamic_cast failed from Node* to Let*");
			}
			constraints.push_back(std::make_pair(c->number, c->n3->number));
			constraints.push_back(std::make_pair(c->n1->number, c->n2->number));
		} else {
			die("Unknown AST node type!");
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
				die("type error: cannot unify " + t_rx + " and " + t_ry);
			}
		}
	}

	// construct variable-type map
	std::map<std::string, std::string> ret;
	auto add_var = [&counter, &ret, &uf](Node *cur) -> void {
		if (cur->getType() == "Var") {
			auto c = dynamic_cast<Var*>(cur);
			if (c == nullptr) {
				die("dynamic_cast failed from Node* to Var*");
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
		// printAST(root);
		while (!tokens.empty()) {
			auto t = tokens.front();
			delete t;
			tokens.pop();
		}
		auto var_type_map = typecheck(root);
		for (auto p : var_type_map) {
			std::cout << p.first << " :: " << p.second << std::endl;
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
