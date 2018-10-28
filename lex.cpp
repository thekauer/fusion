#include "lex.h"

/*constexpr*/ u64 hash(const char const * s) {
	u64 hash = offset;
	for (size_t i = 0; i < strlen(s); i++) {
		hash ^= s[i];
		hash *= prime;
	}
	return hash;
}





#define add_op(e) tokens.push_back(Lexem((e),nullptr));




void Lexer::tabcount() {
	while ((code[i] == ' ' || code[i] == '\t') && i<len)
	{
		tcount++;
		i++;
	}
	if (ltcount < tcount) {
		add_op(Gi);
	}
	if (tcount < ltcount) {
		add_op(Li);
	}


}


void Lexer::is_op() {
	int l = 0;
	if (i < len - 1) {
		switch (code[i])
		{

		case '(': l = Lp; break;
		case ')': l = Rp; break;
		case '[': l = Lb; break;
		case ']': l = Rb; break;
		case '{': l = Lc; break;
		case '}': l = Rc; break;


		case ':':
			if (code[i + 1] == ':') { l = DoubleDoubleDot; i++; }
			else { l = DoubleDot; }
			break;



		case '.':
			if (code[i + 1] != '.') {
				l = Dot;
				break;
			}
			else {
				if (code[i + 1] == '.') {
					l = Range;
					if (i < len - 2 && code[i + 2] == '.') { l = Elipsis; i++; }
					i++;
					break;
				}
			}




		case '&': l = And;
			if (code[i + 1] == '&') { l = AndAnd; i++; }
			break;
		case '|': l = Or;
			if (code[i + 1] == '|') { l = OrOr; i++; }
			break;
		case '<': l = Less;
		case '>': l = Greater;
		case '+': l = Add;
		case '-': l = Sub;
		case '/': l = Div;
		case '%': l = Modulus;
		case '~': l = Neg;
		case '!': l = Not;
		case '^': l = Xor;
		case '=': l = Eq;

			if (code[i + 1] == '=') {
				l += EqOffset; /*eq version */
				i++;
			}
			break;


		default:
			break;
		}
		i++;
	}
	add_op(((_Lexem)l));
}

void Lexer::is_kw_or_id() {
	u64 hash = offset;
	if (code[i] <= 'Z' && code[i] >= 'a') {
		hash ^= code[i];
		hash *= prime;
		if (i < len - 1) {
			i++;
			while (i < len && code[i] >= '0' && code[i] <= 'Z') {
				hash ^= code[i];
				hash *= prime;
				i++;
			}
		}
	}

	std::map<u64, int>::const_iterator k = keywords.find(hash);

	if (k != keywords.cend()) {
		tokens.push_back(Lexem(Kw, new Keyword((Keyword)k->second)));
	}
	else {
		tokens.push_back(Lexem(Id, new u64(hash)));
	}



}


void Lexer::is_char() {
	if (code[i] == '\'') {
		if (i < len - 2) {
			if (code[i + 2] == '\'') {
				tokens.push_back(Lexem(LitChar, new char(code[i + 1])));
			}
		}
		i++; //jump over ending '
	}
}

void Lexer::is_string() {
	if (code[i] == '\"') {
		std::string s = "";
		if (i < len - 1) {
			i++;
			while (i < len && code[i] != '\"') {
				s += code[i];
				i++;
			}
		}
		i++; // jump over the ending "
		tokens.push_back(Lexem(LitString, new std::string(s)));
	}
}

void Lexer::is_hex() {
	if (i < len - 1) {
		if (code[i] == '0' && code[i + 1] == 'x') {
			std::string s = "";
			if (i < len - 2) {
				i += 2;

				while (i < len && isalnum(code[i])) {
					s += code[i];
					i++;
				}
			}

			tokens.push_back(Lexem(LitHex, new long(strtol(s.c_str(), NULL, 16))));
		}
	}
}
void Lexer::is_num() {
	if ((isdigit(code[i]) || code[i] == '-') && i<len - 1) {
		std::string s;
		s += code[i];
		i++;
		bool f = false;
		while (isdigit(code[i]) || code[i] == '.') {
			s += code[i];
			if (code[i] == '.') f = true;
			i++;
		}
		if (f)
			tokens.push_back(Lexem(LitFloat, new float(atof(s.c_str()))));
		else
			tokens.push_back(Lexem(LitInt, new float(atoi(s.c_str()))));

	}
}

void Lexer::is_true() {
	if (i < len - 4) {
		if (code[i] == 't'  &&
			code[i + 1] == 'r'  &&
			code[i + 2] == 'u'  &&
			code[i + 3] == 'e'
			) tokens.push_back(Lexem(LitBool, new bool(true)));
		i += 4;
	}
}

void Lexer::is_false() {
	if (i < len - 5) {
		if (code[i] == 'f'	&&
			code[i + 1] == 'a'	&&
			code[i + 2] == 'l'	&&
			code[i + 3] == 's'	&&
			code[i + 4] == 'e'
			) tokens.push_back(Lexem(LitBool, new bool(false)));
		i += 5;
	}
}

bool Lexer::get_int(int& res) {
	std::string s;
	if ((isdigit(code[i]) || code[i] == '-') && i<len - 1) {
		s += code[i];
		i++;
		while (i < len) {
			s += code[i];
			i++;
		}



		if (i < len - 1 && code[i + 1] == '.') {
			i++;
			s += '.';
			while (i < len && isdigit(code[i])) {
				s += code[i];
				i++;
			}
			tokens.push_back(Lexem(LitFloat, new float(atof(s.c_str()))));
			return false;
		}



		res = atoi(s.c_str());
		return true;
	}
	return false;
}

void Lexer::is_range() {
	int n = 0, start, end, step;
	Range_t::Inf_e inf;

	if ((i < len - 1) && (code[i] == code[i + 1] && code[i + 1] == '.')) {
		i += 2;
		if (get_int(n)) {
			end = n;
			inf = Range_t::RNoStart;
			if (i < len - 1 && code[i + 1] == ':') {
				i++;
				if (i < len - 1) {
					if (get_int(n)) {
						step = n;
					}
					else {
						/*ERROR EXPECTED INTEGER*/
					}
				}
			}
		}
		else {
			inf = Range_t::RInf;
		}
	}

	if (get_int(n)) {
		start = n;
		inf = Range_t::RNoEnd;
		if (i < len - 2 && code[i + 1] == '.' && code[i + 2] == '.') {
			i += 2;
			if (i < len - 1 && isdigit(code[i + 1])) {
				i++;
				if (get_int(n)) {
					end = n;
					inf = Range_t::RNot;
					if (i < len - 1 && code[i + 1] == ':') {
						i++;
						if (i < len - 1 && isdigit(code[i + 1])) {
							i++;
							if (get_int(n)) {
								step = n;
							}
							else {
								/*ERROR EXPECTED INTEGER*/
							}
						}
					}
				}
			}

		}
	}
}

void Lexer::is_lit() {
	is_range();
	return;
	is_char();
	return;
	is_string();
	return;
	is_hex();
	return;
	is_num();
	return;
	is_true();
	return;
	is_false();
	return;
}


void Lexer::lex() {
	while (i < len) {
		while (code[i] != '\n' && i < len) {
			tabcount();

			is_lit(); //MUST GO FIRST
			is_op();
			is_kw_or_id();





			add_op(N);
			i++;
		}

		i++;
	}
}


std::vector<Lexem> Lexer::GetTokens() {
	return tokens;
}

#undef add_op