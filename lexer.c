#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include<stdlib.h>

#include "lexer.h"
#include "utils.h"

Token *tokens;	// single linked list of tokens
Token *lastTk;		// the last token in list

int line=1;		// the current line in the input file

// adds a token to the end of the tokens list and returns it
// sets its code and line

///definim o functie extract se face pe interval [), imi extrag textul si il pun unde imi trebuie mie
Token *addTk(int code){
	Token *tk=safeAlloc(sizeof(Token));
	tk->code=code;
	tk->line=line;
	tk->next=NULL;
	if(lastTk){
		lastTk->next=tk;
		}else{
		tokens=tk;
		}
	lastTk=tk;
	return tk;
	}

char *extract(const char *begin,const char *end){
	//err("extract needs to be implemented");
	size_t lenght = end - begin;
	char* bigLine = (char *)malloc(sizeof(char)*(lenght+1));
	if (bigLine == NULL)
	{
		err("Eroare la extragere!\n");
	}
	strncpy(bigLine, begin, lenght);
	bigLine[lenght] = '\0';
	return bigLine;
	}
double turnIntoDouble(const char * start, const char *pch)
{
	char* out = extract(start, pch);
	if (!out)
		err("Eroare la extragere!");
	char* endptr;
	double d= strtod(out, &endptr);
	if (out == endptr || *endptr!='\0')
	{
		err("Failed to convert Double from String\n");
	}
	free(out);
	return d;
}
Token *tokenize(const char *pch)
{
	const char *start;
	Token *tk;
	for(;;)
	{
		switch (*pch)
		{
		case ' ':case '\t':pch++; break;
		case '\r':		// handles different kinds of newlines (Windows: \r\n, Linux: \n, MacOS, OS X: \r or \n)
			if (pch[1] == '\n')pch++;
			// fallthrough to \n
		case '\n':
			line++;
			pch++;
			break;
		case '\0':addTk(END); return tokens;
		case ',':addTk(COMMA); pch++; break;
		case ';': addTk(SEMICOLON); pch++; break;
		case '(': addTk(LPAR); pch++; break;
		case ')': addTk(RPAR); pch++; break;
		case '[': addTk(LBRACKET); pch++; break;
		case ']': addTk(RBRACKET); pch++; break;
		case '{': addTk(LACC); pch++; break;
		case '}': addTk(RACC); pch++; break;
			///aici deja adaugam ; si din astea etc
		case '=':
			if (pch[1] == '=') {
				addTk(EQUAL);
				pch += 2;
			}
			else
			{
				addTk(ASSIGN);
				pch++;
			}
			break;
		case '+':  addTk(ADD); pch++; break;
		case '-': addTk(SUB); pch++; break;
		case '*': addTk(MUL); pch++; break;
		case '/':
			if (pch[1] != '/')
			{
				addTk(DIV); pch++;
			}
			else
			{
				while (*pch != '\r' && *pch != '\n' && *pch != '\0')
					pch++;
			}
			break;
		case '.': addTk(DOT); pch++; break;
		case '&':
			if (pch[1] == '&')
			{
				addTk(AND);
				pch += 2;
			}
			else
			{
				err("Eroare, nu se cunoaste al doilea caracter! Trebuie sa avem AND--&&\n");
			}
			break;
		case '|':
			if (pch[1] == '|')
			{
				addTk(OR);
				pch += 2;
			}
			else
			{
				err("Eroare, nu se cunoaste al doilea caracter! Trebuie sa avem OR---||\n");
			}
			break;
		case '!':
			if (pch[1] == '=')
			{
				addTk(NOTEQ);
				pch += 2;
			}
			else
			{
				addTk(NOT);
				pch++;
			}
			break;
		case '<':
			if (pch[1] == '=')
			{
				addTk(LESSEQ);
				pch += 2;
			}
			else
			{
				addTk(LESS);
				pch++;
			}
			break;
		case '>':
			if (pch[1] == '=')
			{
				addTk(GREATEREQ);
				pch += 2;
			}
			else
			{
				addTk(GREATER);
				pch++;
			}
			break;

			//add the other case -> 1char
			//sa adaugam mesaje de eroare
		default:
			if (isalpha(*pch) || *pch == '_') {
				for (start = pch++; isalnum(*pch) || *pch == '_'; pch++) {}
				char* text = extract(start, pch);
				if (strcmp(text, "char") == 0) { addTk(TYPE_CHAR); free(text); }
				///aici adaugam restul din atomC Cuvinte cheie adica
				else if (strcmp(text, "double") == 0) { addTk(TYPE_DOUBLE); free(text); }
				else if (strcmp(text, "else") == 0) {
					addTk(ELSE); free(text);
				}
				else if (strcmp(text, "if") == 0) { addTk(IF); free(text); }
				else if (strcmp(text, "int") == 0) {
					addTk(TYPE_INT); free(text);
				}
				else if (strcmp(text, "return") == 0) { addTk(RETURN); free(text); }
				else if (strcmp(text, "struct") == 0) {
					addTk(STRUCT); free(text);
			}
				else if (strcmp(text, "void") == 0) {
					addTk(VOID); free(text);
				}
				else if (strcmp(text, "while") == 0) {
					addTk(WHILE); free(text);
				}
				else {
					tk = addTk(ID);
					tk->text = text;
				}
			}
			///aici mai trebuie sa facem int double, char, string.
			else if (isdigit(*pch))
			{
				start = pch;
				int is_double = 0;
				while (isdigit(*pch)) pch++;

				if (*pch == '.')
				{
					pch++;
					if (isdigit(*pch))
					{
						is_double = 1;
						while (isdigit(*pch)) pch++;
						
						if (strchr("eE", *pch) != NULL)
						{
							pch++;
							if (strchr("+-", *pch) != NULL)pch++;
							if (isdigit(*pch))
								{
									while (isdigit(*pch)) pch++;
								}
							else
								{
									is_double = 0;
									err("Lipsa parte numerica exponent!\n");///partea numerica
								}
						}
					}
					else
					{
						err("Lipsa parte zecimala!\n");///
					}
				}
				else if (strchr("eE", *pch) != NULL)
				{
					
					pch++;
					if (strchr("+-", *pch) != NULL)pch++;
						if (isdigit(*pch))
						{
							is_double = 1;
							while (isdigit(*pch)) pch++;
						}
						else
						{
							err("Lipsa parte numerica la exponent\n");
						}
				}
				if (is_double)
				{
					tk = addTk(DOUBLE);
					tk->d = turnIntoDouble(start, pch);
				}
				else
				{
					tk = addTk(INT);
					char* out = extract(start, pch);
					tk->i = atoi(out);
					free(out);
				}
			}
			else if (*pch == '\'')
			{
				///const char* start = pch;
				char c;
				pch++;
				if (strchr("\'\\\0", *pch) == NULL)
				{
					c = *pch;
					pch++;
				}
				else if (*pch == '\\') {
					pch++;
					switch (*pch)
					{
					case 'a': c = '\a'; break;
					case 'b': c = '\b'; break;
					case 'f': c = '\f'; break;
					case 'n': c = '\n'; break;
					case 'r': c = '\r'; break;
					case 't': c = '\t'; break;
					case 'v': c = '\v'; break;
					case '\'': c = '\''; break;
					case '\"': c = '\"'; break;
					case '\\': c = '\\'; break;
					case '0': c = '\0'; break;
					default: err("Secventele pentru CHAR!\n");
					}
					pch++;
				}
				else
					err("Secventa invalida pentru CHAR.\n");
				if (*pch == '\'')
				{
					pch++;
					tk = addTk(CHAR);
					tk->c = c;
				}
				else
					err("Lipseste ' de la finalul caracterului\n");
			}
			else if (*pch == '\"')
			{
				pch++;
				const char* outText = pch;
				///int is_string = 0;
				while (*pch != '\"' && *pch != '\0')
				{
					if (*pch == '\\' && pch[1] != '\0')
					{
						pch++;
					}
					pch++;
				}
				if (*pch != '\"')
				{
					err("Sirul nu se termina intr-o maniera buna!!!\n");
				}
				char* finalText = extract(outText, pch);
				tk = addTk(STRING);
				int i = 0;
				const char* c = outText;
				while (c < pch)
				{
					if (*c == '\\')
					{
						c++;
						switch (*c)
						{
						case 'a': finalText[i++] = '\a';
							break;
						case 'b': finalText[i++] = '\b';
							break;
						case 'f': finalText[i++] = '\f';
							break;
						case 'n': finalText[i++] = '\n';
							break;
						case 'r': finalText[i++] = '\r';
							break;
						case 't': finalText[i++] = '\t';
							break;
						case 'v': finalText[i++] = '\v';
							break;
						case '\\': finalText[i++] = '\\';
							break;
						case '\'': finalText[i++] = '\'';
							break;
						case '\"': finalText[i++] = '\"';
							break;
						case '\0': finalText[i++] = '\0';
							break;
						}
					}
					else
					{
						finalText[i++] = *c;
					}
					c++;
				}
				finalText[i] = '\0';
				tk->text = _strdup(finalText);
				free(finalText);
				pch++;
			}
			else
			{
				err("invalid char: %c", *pch);
				pch++;
			}
			break;
		}
		
	}
}
void showTokens(const Token *tokens)
{
	const char* tkNames[] = {
	"ID", "TYPE_CHAR", "TYPE_DOUBLE", "ELSE", "IF", "TYPE_INT", "RETURN", "STRUCT", "VOID", "WHILE",
	"COMMA", "END", "SEMICOLON", "LPAR", "RPAR", "LBRACKET", "RBRACKET", "LACC", "RACC",
	"ASSIGN", "EQUAL", "ADD", "SUB", "MUL", "DIV", "DOT", "AND", "OR", "NOT", "NOTEQ", "LESS", "LESSEQ", "GREATER", "GREATEREQ",
	"INT", "DOUBLE", "CHAR", "STRING"
	};
	for(const Token *tk=tokens;tk;tk=tk->next)
	{
		
			printf("%d\t%s", tk->line, tkNames[tk->code]);
			if (strcmp(tkNames[tk->code], "ID") == 0 || strcmp(tkNames[tk->code], "STRING") == 0)
			{
				printf(":%s", tk->text);
			}
			else if (strcmp(tkNames[tk->code], "INT") == 0)
			{
				printf(":%d", tk->i);
			}
			else if (strcmp(tkNames[tk->code], "DOUBLE") == 0)
			{
				printf(":%.2f", tk->d);
			}
			else if (strcmp(tkNames[tk->code], "CHAR") == 0)
			{
				printf(":%c", tk->c);
			}
			printf("\n");
	}
}
