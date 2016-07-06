/*
	PARSE

	The only "public" function here is read_code,
	which is just a wrapper around two other more
	substantial functions: tokenize and parse.

	tokenize is a classic finite state automaton
	for "tokenizing" (i.e. determining the basic
	meaningful pieces of) the input code string.
	Because Lisp syntax is so brilliantly simple
	(only parentheses and whitespace are syntactically
	significant), the FSA has only two "states".
	It is implemented entirely with GOTOs, with no
	function calls at all!***

	*** Actually, it uses the C standard library,
	but still, it makes no calls to user-defined
	functions.

	tokenize generates a list of tokens which is
	then passed to parse. parse, in turn, generates
	a basic abstract syntax tree. again, because
	Lisp syntax is so simple, no further analysis
	is needed. parse generates a single Obj, which
	may itself contain a List of Objs (see objects.h
	for details).
*/

/*
	TODO:
		-- fix tokenize for nonlist case
		-- add scanf (or something) to read_code
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "objects.h"
#include "parse.h"

// add scanf?
// void parameter

// for now...
extern char* code;

Obj read_code(void) {

	//char* expr = "(add 3 4)";
	//char* expr = "cat";
	//char* expr = "()";
	//char* expr = "(quote ())";
	//char* expr = "(add)";
	//char* expr = "(add ((addn 3) 4) 7 (mul 5 6))";

	printf("\nLISP CODE: %s\n", code);
	return parse(tokenize(code));
}

// TODO: special case for nonlist expr
Token_list* tokenize(char* expr) {
	State state = READY;

	int length = strlen(expr);
	if (length == 0) return NULL;

	int i = 0;
	char c;

	Token_list* tokens = malloc(sizeof(Token_list)); // initialize to NULL?
	Token_list* tail = tokens;
	tail->next = NULL;

	// temporary variables
	int start;
	int end;
	int sub_length;
	char* text;

	START:
		if (i >= length) {
			if (state == SYMBOL) {
				goto END_TEXT;
			}
			goto DONE;
		}

		c = expr[i];

		// TODO: special case for nonlist expr

		if (c == '(')
			goto OPEN;
		if (c == ')')
			goto CLOSE;
		if (c == ' ' || c == '\n')
			goto WHITESPACE;
		if (isalnum(c))
			goto TEXT;

	OPEN://printf("%d @ %s\n", state, "OPEN");
		if (state == SYMBOL)
			goto END_TEXT;
		tail->token.start = i;
		tail->token.end = i + 1;
		tail->token.id = OP;
		tail->token.text = "OPEN";
		tail->next = malloc(sizeof(Token_list));
		tail = tail->next;
		tail->next = NULL;
		i++;
		goto START;

	CLOSE://printf("%d @ %s\n", state, "CLOSE");
		if (state == SYMBOL)
			goto END_TEXT;
		tail->token.start = i;
		tail->token.end = i + 1;
		tail->token.id = CP;
		tail->token.text = "CLOSE";
		if (i < length - 1) {
			tail->next = malloc(sizeof(Token_list));
			tail = tail->next;
			tail->next = NULL;
		}
		i++;
		goto START;

	WHITESPACE://printf("%d @ %s\n", state, "WHITESPACE");
		if (state == SYMBOL)
			goto END_TEXT;
		i++;
		goto START;

	TEXT://printf("%d @ %s\n", state, "TEXT");
		if (state == READY) {
			state = SYMBOL;
			tail->token.id = SYM;
			tail->token.start = i;
		}
		i++;
		goto START;

	END_TEXT://printf("%d @ %s\n", state, "END_TEXT");
		tail->token.end = i;
		start = tail->token.start;
		end = tail->token.end;
		sub_length = end - start;
		text = malloc(sub_length * sizeof(char));
		strncpy(text, expr + start, sub_length);
		text[sub_length] = '\0';	
		tail->token.text = text;
		//if (i < length - 1) {
			tail->next = malloc(sizeof(Token_list));
			tail = tail->next;
			tail->next = NULL;
		//}
		state = READY;
		goto START;

	DONE:
		return tokens;
}	


Obj parse(Token_list* tokens) {
	if (tokens == NULL) {
		printf("%s\n", "no tokens -- read_from_tokens");
		exit(1);
	}

	Obj obj; 
	Token token = tokens->token;

	// code is a name or number
	if (token.id == SYM) {
		char* text = token.text;
		if (isdigit(text[0])) 
			obj = MKOBJ(NUM, num, atoi(text));
		else 
			obj = MKOBJ(NAME, name, text);
		return obj;
	}

	// code is a list
	List* result = NULL;
	Token_list* remainder = slice_ends(&tokens);

	Token_list* head = NULL;
	Token_list* tail = NULL;

	int op = 0;
	int cp = 0;

	while (remainder) {
		Token first = remainder->token;

		// first item is an atom
		if (first.id == SYM) { 
			// wrap first (Token) in a Token_list to pass to parse
			Token_list dummy;
			dummy.next = NULL;
			dummy.token = first;
			push(parse(&dummy), &result);

				// do we need this?
			// if (remainder->next == NULL)
			// 	return result;
			// else
				remainder = remainder->next;
		}

		// first item is a list
		else {
			op = 1;
			cp = 0;
			head = remainder;
			tail = remainder;
			while (op > cp) { // this will terminate if code is wellformed
				tail = tail->next;
				if (tail->token.id == OP)
					op++;
				if (tail->token.id == CP)
					cp++;
			}
			remainder = tail->next;
			tail->next = NULL;
			push(parse(head), &result);
		} 
	}

	obj = MKOBJ(LIST, list, result);
	return obj;
}


// list manipulation
	// token list
void dock(Token_list** list) {
	if (*list == NULL) return;
	
	else if ((*list)->next == NULL) {
		free(*list);
		*list = NULL;
		return;
	}
	
	else dock(&((*list)->next));
}

Token_list* slice_ends(Token_list** list) {
	Token_list* sliced = (*list)->next;
	free(*list);
	*list = NULL;
	dock(&sliced);
	return sliced;
}

	// obj list
void push(Obj obj, List** list) {
	if (*list == NULL) {
		*list = malloc(sizeof(List));
		(*list)->car = obj;
		(*list)->cdr = NULL;
		return;
	}

	else push(obj, &((*list)->cdr));
}