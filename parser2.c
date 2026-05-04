#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>

#include "parser.h"
#include "ad.h"
#include "vm.h"
#include "utils.h"
Symbol *owner=NULL;
// Prototipuri pentru a evita eroarea "assuming int"
bool expr();
bool exprAssign();
bool exprOr();
bool exprOrPrim();
bool exprAnd();
bool exprAndA();
bool exprEq();
bool exprEqE();
bool exprRel();
bool exprRelR();
bool exprAdd();
bool exprAddA();
bool exprMul();
bool exprMulM();
bool exprCast();
bool exprUnary();
bool exprPostfix();
bool exprPostfixP();
bool exprPrimary();
bool stm();
bool stmCompound(bool newDomain);
bool fnDef();
bool varDef();
bool structDef();

extern int line;
Token *iTk;		// the iterator in the tokens list
Token *consumedTk;		// the last consumed token

void tkerr(const char *fmt,...){
	fprintf(stderr,"error in line %d: ",iTk->line);
	va_list va;
	va_start(va,fmt);
	vfprintf(stderr,fmt,va);
	va_end(va);
	fprintf(stderr,"\n");
	exit(EXIT_FAILURE);
	}

bool consume(int code){
	//printf("consume(%d) iTk->code=%d line=%d\n", code, iTk->code, iTk->line);

	if(iTk->code==code){
		consumedTk=iTk;
		iTk=iTk->next;
		return true;
		}
	return false;
	}

// typeBase: TYPE_INT | TYPE_DOUBLE | TYPE_CHAR | STRUCT ID
bool typeBase(Type *t){
	t->n=-1;
	if(consume(TYPE_INT)){
		t->tb=TB_INT;
		return true;
		}
	if(consume(TYPE_DOUBLE)){
		t->tb=TB_DOUBLE;
		return true;
		}
	if(consume(TYPE_CHAR)){
		t->tb=TB_CHAR;
		return true;
		}
	if(consume(STRUCT)){
		if(consume(ID)){
			Token *tkName=consumedTk;
			t->tb=TB_STRUCT;
			t->s=findSymbol(tkName->text);
			if(!t->s) 
			{
				tkerr("structura nedefinita: %s", tkName->text);
			}
			return true;
			}
		}
	return false;
	}

//arrayDecl LBRACKET INT? RBRACKET
bool arrayDecl(Type *t)
{
	Token* start = iTk;
	if (consume(LBRACKET))
	{

		if (consume(INT))
		{
			Token *tkSize=consumedTk;
			t->n=tkSize->i;

		}
		else
		{
			t->n=0;
		}
		if (consume(RBRACKET))
		{
			return true;
		}
		else
		{
			tkerr("Nu exista ] in functia arrayDecl\n");
		}
	}
	iTk = start;
	return false;
}

//fnParam:typeBase ID arrayDecl?
bool fnParam()
{
	Type t;
	Token* start = iTk;
	if (typeBase(&t))
	{
		if (consume(ID))
		{
			Token* tkName = consumedTk;
			if (arrayDecl(&t))
			{
				t.n = 0;
			}
			Symbol* param = findSymbolInDomain(symTable, tkName->text);
			if (param)tkerr("symbol redefinition: %s", tkName->text);
			param = newSymbol(tkName->text, SK_PARAM);
			param->type = t;
			param->owner = owner;
			param->paramIdx = symbolsLen(owner->fn.params);
			// parametrul este adaugat atat la domeniul curent, cat si la parametrii fn
			addSymbolToDomain(symTable, param);
			addSymbolToList(&owner->fn.params, dupSymbol(param));
			return true;
		}
		else
			tkerr("Nu exista numele parametrului dupa tip\n");
	}
	iTk = start;
	return false;
}


//varDef: typeBase ID arrayDecl? SEMICOLON
bool varDef()
{
	Token* start = iTk;
	Type t;
	if (typeBase(&t))
	{
		if (consume(ID))
		{
			Token *tkName=consumedTk;
			if (arrayDecl(&t)) {
				if(t.n==0)tkerr("a vector variable must have a specified dimension");
			}
			if (consume(SEMICOLON))
			{
				Symbol *var=findSymbolInDomain(symTable,tkName->text);
				if(var)tkerr("symbol redefinition: %s",tkName->text);
				var=newSymbol(tkName->text,SK_VAR);
				var->type=t;
				var->owner=owner;
				addSymbolToDomain(symTable,var);
				if(owner){
					switch(owner->kind){
						case SK_FN:

							var->varIdx=symbolsLen(owner->fn.locals);
							addSymbolToList(&owner->fn.locals,dupSymbol(var));
						break;
						case SK_STRUCT:
							var->varIdx = 0;
							Symbol* m = owner->structMembers;
							while (m) {
								var->varIdx += typeSize(&m->type);
								m = m->next;
							}
							addSymbolToList(&owner->structMembers, dupSymbol(var));
						break;
					}
				}else{
				var->varMem=safeAlloc(typeSize(&t));
				}
				return true;
			}
			else
				tkerr("Nu exista ; la final de variabila\n");
		}
		else
		{
			tkerr("Nu exista un id, un nume de variabila la functia varDef\n");
		}
	}
	iTk = start;
	return false;
}
//structDef: STRUCT ID LACC varDef* RACC SEMICOLON
bool structDef()
{
	Token* start = iTk;
	
	if (consume(STRUCT))
	{
		if (consume(ID))
		{
			Token *tkName=consumedTk;
			if (consume(LACC))
			{
				Symbol *s=findSymbolInDomain(symTable,tkName->text);
				if(s)tkerr("symbol redefinition: %s",tkName->text);
				s=addSymbolToDomain(symTable,newSymbol(tkName->text,SK_STRUCT));
				s->type.tb=TB_STRUCT;
				s->type.s=s;
				s->type.n=-1;
				pushDomain();
				owner=s;
				while (varDef())
				{
				}
					if (consume(RACC))
					{
						if (consume(SEMICOLON))
						{
							owner=NULL;
							dropDomain();
							return true;
						}
						else
						{
							tkerr("Nu exista ; la final	\n ");
						}
					}
					else
					{
						tkerr("Nu exista } in cadrul structului\n");
					}
			}
			/*else
			{
				tkerr("Nu exista { in cadrul structului\n");
			}
			*/
		}
		else
		{
			tkerr(" Nu exista numele structului\n");
		}
	}
	iTk = start;
	return false;
}


/*stm: stmCompound
| IF LPAR expr RPAR stm ( ELSE stm )?
| WHILE LPAR expr RPAR stm
| RETURN expr? SEMICOLON
| expr? SEMICOLON*/

bool stm()
{
	if (stmCompound(true))
	{
		return true;
	}
	else if (consume(IF))
	{
		if (consume(LPAR))
		{
			if (expr())
			{
				if (consume(RPAR))
				{
					if (stm())
					{
						if (consume(ELSE))
						{
							if (!stm())
							{
								tkerr("Lipseste instructiunea dupa else\n");	
							}
						}
						if (iTk->code == LPAR || iTk->code==RPAR) {
							tkerr("Paranteza ( sau ) neasteptata dupa if\n");
						}
						return true;
					}
					else
						tkerr("Nu exista instructiunea dupa conditia if\n");
				}
				else
					tkerr("Nu exista ) dupa expresia if-ului\n");
			}
			
		}
		else
			tkerr("Nu exista (  pentru if\n");
	}
	else if (consume(WHILE))
	{
		if (consume(LPAR))
		{
			if (expr())
			{
				if (consume(RPAR))
				{
					if (stm())
					{
						if (iTk->code == LPAR || iTk->code== RPAR) {
							tkerr("Paranteza ( sau ) neasteptata dupa while\n");
						}
						return true;
					}
					else
						tkerr("nu exista instructiune in while\n");
				}
				tkerr("Nu exista ) in while\n");
			}
			else
				tkerr("Nu exista conditie valida in cadrul while-lului\n");
		}
		else
			tkerr("Nu exista ( in while  \n");
	}
	else if (consume(RETURN))
	{
		if (expr())
		{

		}
		if (consume(SEMICOLON))
		{
			return true;
		}
		else
			tkerr("Nu exista ; dupa return\n");
	}
	else {
		Token* start = iTk;
		if (expr()) {
			if (consume(SEMICOLON))
			{
				return true;
			}
			else
				tkerr("Nu exista ; dupa expresie\n");
		}

		if (consume(SEMICOLON))
		{
			return true;
		}
		iTk = start;
	
	}
	return false;
}

//stmCompound: LACC ( varDef | stm )* RACC
bool stmCompound(bool newDomain)
{
	Token* start = iTk;
	if (consume(LACC))
	{
		if (iTk->code == LACC) {
			tkerr("Acolada { neasteptata\n");
		}
		if(newDomain)
		{
			pushDomain();
		}
		for (;;)
		{
			if (varDef())
			{
				if (iTk->code == SEMICOLON) {
					tkerr("Simbol ; neasteptat\n");
				}
			}
			else if (stm())
			{
				if (iTk->code == SEMICOLON) {
					tkerr("Simbol ; neasteptat\n");
				}
			}
			else
			{
				break;
			}

		}
		if (consume(RACC))
		{
			if(newDomain)
			{
				dropDomain();
			}
			
			return true;
		}
		else
			tkerr("Nu exista } la finalul blocului de instructiuni \n");
		
	}
	iTk = start;
	return false;
}


// fnDef ( typeBase | VOID ) ID
//          LPAR(fnParam(COMMA fnParam)*) ? RPAR
//          stmCompound
bool fnDef()
{
	Token* start = iTk;
	Type t;
	if (typeBase(&t))
	{
	}else if (consume(VOID)) {
		t.tb = TB_VOID;
		t.n = -1; // Nu este tablou
	}
	else {
		return false; // Nu este o definiție de funcție
	}
	
		if (consume(ID))
		{
			Token *tkName=consumedTk;
			if (consume(LPAR))
			{
				Symbol *fn=findSymbolInDomain(symTable,tkName->text);
				if(fn)tkerr("symbol redefinition: %s",tkName->text);
				fn=newSymbol(tkName->text,SK_FN);
				fn->type=t;
				addSymbolToDomain(symTable,fn);
				owner=fn;
				pushDomain();
				if (fnParam())
				{
					while (consume(COMMA))
						{
							if (!fnParam())
								tkerr("Nu exista un parametru valid dupa virgula\n");
						}
					

				}
				if (consume(RPAR))
				{
					if (stmCompound(false))
					{
						dropDomain();
						owner=NULL;
						return true;
					}
					else
					{
						tkerr("Nu se satisface conditia de a incepe cu {\n");
						dropDomain();
						owner=NULL;
					}
				}
				else
					tkerr("Nu exista )\n");
			}
			/*else
				tkerr("Nu exista (, la linia %d \n", line);
				*/
		}
		else
			tkerr("Nu exista un nume dupa tipul return, la linia %d \n", line);
	
	iTk = start;
	return false;
}

// unit: ( structDef | fnDef | varDef )* END
bool unit(){
	for(;;){
		if(structDef()){}
		else if(fnDef()){}
		else if(varDef()){}
		else break;
		}
	if(consume(END)){
		return true;
		}
	tkerr("Eroare, simbol neasteptat dupa unit\n");
	return false;
	}
//expr: exprAssign
bool expr()
{
	if (exprAssign())
	{
		return true;
	}
	
	return false;
}


//exprAssign:  exprUnary ASSIGN exprAssign | exprOr
bool exprAssign()
{
	Token* start = iTk;
	if (exprUnary())
	{
		if (consume(ASSIGN))
		{
			if (exprAssign())
			{
				return true;
			}
			else
				tkerr("Expresia este invalida dupa operatorul de assignare\n");
		}
		
	}
	iTk = start;
	if (exprOr())
	{
		return true;
	}
	iTk = start;
	return false;
}


/*exprOr: exprOr OR exprAnd | exprAnd

exprOr: exprAnd exprOrPrim
exprOrPrim: Or exprAnd exprOrPrim|e*/
bool exprOr()
{
	if (exprAnd())
	{
		if (exprOrPrim())
		{
			return true;
		}
	
	}
	return false;
}
bool exprOrPrim()
{
	Token* start = iTk;
	if (consume(OR))
	{
		if (exprAnd())
		{
			if (exprOrPrim())
			{
				return true;
			}
		}
		else
			tkerr("Nu exista expresie dupa operatorul de tip Or\n");
	}
	iTk = start;
	return true;
}

/*
exprAnd: exprAnd AND exprEq | exprEq

exprAnd:exprEq exprAndA
exprAndA: AND exprEq exprAndA|e
*/
bool exprAnd()
{
	if (exprEq())
	{
		if (exprAndA())
		{
			return true;
		}
		
	}
	return false;
}
bool exprAndA()
{
	Token* start = iTk;
	if (consume(AND))
	{
		if (exprEq())
		{
			if (exprAndA())
			{
				return true;
			}
			
		}
		else
			tkerr("Nu exista operator dupa operatorul '&&' \n");
	}
	iTk = start;
	return true;
}

/*
exprEq: exprEq ( EQUAL | NOTEQ ) exprRel | exprRel

exprEq:exprRel exprEqE
exprEqE: ( EQUAL | NOTEQ ) exprRel exprEqE|e

*/
bool exprEq()
{
	if (exprRel())
	{
		if (exprEqE())
		{
			return true;
		}
	
	}
	return false;
}
bool exprEqE()
{
	Token* start = iTk;
		if (consume(EQUAL) || consume(NOTEQ))
		{
			if (exprRel())
			{
				if (exprEqE())
					return true;
				
			}
			else {
				tkerr("Nu exista operandul dupa operatorul de egalitate - '==' si cek de negalitate '!=' \n");
			}
		}
	iTk = start;
	return true;
}

/*exprRel: exprRel ( LESS | LESSEQ | GREATER | GREATEREQ ) exprAdd | exprAdd

exprRel:exprAdd exprRelR
exprRelR:( LESS | LESSEQ | GREATER | GREATEREQ ) exprAdd exprRelR| e
*/
bool exprRel()
{
	if (exprAdd())
	{
		if (exprRelR())
		{
			return true;
		}
	
	}
	return false;
}
bool exprRelR()
{
	Token* start = iTk;
	if (consume(LESS) || consume(LESSEQ) || consume(GREATER) || consume(GREATEREQ))
	{
		if (exprAdd())
		{
			if (exprRelR())
			{
				return true;
			}
		}
		else
			tkerr("Nu exista un operand de comparatie \n ");
	}
	iTk = start;
	return true;
}

/*exprAdd: exprAdd ( ADD | SUB ) exprMul | exprMul

exprAdd: exprMul exprAddA
exprAddA: ( ADD | SUB ) exprMul exprAddA|e*/
bool exprAdd()
{
	if (exprMul())
	{
		if (exprAddA())
		{
			return true;
		}
		
	}
	return false;
}
bool exprAddA()
{
	Token* start = iTk;
	if (consume(ADD) || consume(SUB))
	{
		if (exprMul())
		{
			if (exprAddA())
			{
				return true;
			}
			
		}
		else
			tkerr(" Nu exista operand dupa operatorul de adunare + sau cel de scadere -\n");
	}
	iTk = start;
	return true;
}

/*exprMul: exprMul ( MUL | DIV ) exprCast | exprCast

exprMul: exprCast exprMulM
exprMulM: ( MUL | DIV ) exprCast exprMulM| e*/
bool exprMul()
{
	if (exprCast())
	{
		if (exprMulM())
		{
			return true;
		}
		
	}
	return false;
}
bool exprMulM()
{
	Token* start = iTk;
	if (consume(MUL) || consume(DIV))
	{
		if (exprCast())
		{
			if (exprMulM())
			{
				return true;
			}
			
		}
		else
			tkerr("Nu exista operand dupa operatorul de inmultire * si nici dupa cel de impartire :\n");
	}
	iTk = start;
	return true;
}


/*exprCast: LPAR typeBase arrayDecl? RPAR exprCast | exprUnary*/
bool exprCast()
{
	Token* start = iTk;
	if (consume(LPAR))
	{
		Type t;
		if (typeBase(&t))
		{
			if (arrayDecl(&t)) {}
			if (consume(RPAR))
			{
				if (exprCast())
					return true;
				else
					tkerr("Expresie invalida dupa cast\n");
			}
			else
				tkerr("Nu exista ) dupa titpul de date pentru cast\n");
		}
		iTk = start;
	}
	else if (exprUnary())
	{
		return true;
	}
	iTk = start;
	return false;
}

//exprUnary: ( SUB | NOT ) exprUnary | exprPostfix
bool exprUnary()
{
	if (consume(SUB) || consume(NOT))
	{
		if (exprUnary())
		{
			return true;
		}
		else
			tkerr("Nu exista expresie dupa operatorul unar\n");
	}
	else if (exprPostfix())
	{
		return true;
	}
	return false;
}

/*exprPostfix: exprPostfix LBRACKET expr RBRACKET
| exprPostfix DOT ID
| exprPrimary

exprPostfix: exprPrimary exprPostfixP
exprPostfixP: LBRACKET expr RBRACKET exprPostfixP | DOT ID exprPostfixP| e*/
bool exprPostfix()
{
	if (exprPrimary())
	{
		if (exprPostfixP())
		{
			return true;
		}
	}
	return false;
}
bool exprPostfixP()
{
	Token* start = iTk;
	if (consume(LBRACKET))
	{
		if (expr())
		{
			if (consume(RBRACKET))
			{
				if (exprPostfixP())
				{
					return true;
				}
				
			}
			else tkerr("Lipseste ] la indexarea tabloului\n");
			if(iTk->code== LBRACKET || iTk->code == DOT ) {
				tkerr("Simbol [ sau . neasteptat\n");
			}
		}
		else tkerr("Lipseste [ la indexarea tabloului \n");
		if(iTk->code== RBRACKET || iTk->code == DOT ) {
			tkerr("Simbol ] sau . neasteptat\n");
		}
	}
	else if (consume(DOT))
	{
		if (consume(ID))
		{
			if (exprPostfixP())
			{
				return true;
			}
		}
		else
			tkerr("Lipseste numele membrului dupa . \n");
	}
	iTk = start;
	return true;
}

/*exprPrimary: ID ( LPAR ( expr ( COMMA expr )* )? RPAR )?
| INT | DOUBLE | CHAR | STRING | LPAR expr RPAR*/
bool exprPrimary()
{
	Token* start = iTk;
	if (consume(ID))
	{
		if (consume(LPAR))
		{
			if (expr())
			{
				while (consume(COMMA))
					{
						if (!expr())
						{
							tkerr("Lipseste expresia de dupa , , in apelul functiei \n");
						}
					}
				if (iTk->code == COMMA || iTk->code == SEMICOLON) {
					tkerr("Simbol , sau ; neasteptat\n");
				}
			}
			if (consume(RPAR))
			{
				return true;
			}
			else
				tkerr("Nu are paranteza ) la apelul functiei.\n");
		}
		return true;
	}
	else if (consume(INT))
	{
		return true;
	}
	else if (consume(DOUBLE))
	{
		return true;
	}
	else if(consume(CHAR))
	{ 
		return true;
	}
	else if (consume(STRING))
	{
		return true;
	}
	else if (consume(LPAR))
	{
		if (expr())
		{
			if (consume(RPAR))
			{
				return true;
			}
			else tkerr("Nu exista ) dupa expresie \n");
		}
		else tkerr("Nu exista o expresie valida dupa (  \n");
	}
		if (consume(RPAR)) {
			tkerr("Paranteza ) neasteptata\n");
		}
	
	iTk = start;
	return false;
}
void parse(Token *tokens){
	iTk=tokens;
	if(!unit())tkerr("syntax error");
	}
