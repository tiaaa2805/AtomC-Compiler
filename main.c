#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include<stdlib.h>
#include "lexer.h"
#include "utils.h"
#include "parser.h"
#include "ad.h"
///#include "vm.h"
int main()
{
	const char *fileName = "C:\\Users\\tijan\\OneDrive\\Desktop\\AtomC\\LFTC-Tot\\tests\\testparser.c";
	char *lines = loadFile(fileName);

	Token* pTokens = tokenize(lines); // Preia rezultatul functiei tokenize
	/*pushDomain();
	parse(pTokens);
	showDomain(symTable, "global");
	dropDomain();
	
	
	*/
parse(pTokens);
	printf("Parsing successful!\n");
	///showTokens(tokens);
	free(lines);
	free(pTokens);
	return 0;
}