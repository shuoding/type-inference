#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <utility>

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

Node parseHead(const std::string &source) {
}

Node parseTail(const std::string &source) {
}

AST parse(const std::string &source) {
}

std::pair<bool, std::map<std::string, std::string>> typecheck(const AST &ast) {
}

std::pair<std::string, std::pair<bool, int>> eval(const AST &ast) {
}

int main() {
	std::string line;
	while (true) {
		getline(std::cin, line);
		AST ast = parse(line);
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
