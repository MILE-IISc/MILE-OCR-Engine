#include "OCR_ClassLabel.h"

namespace IISc_KannadaClassifier {

void OCR_ClassLabel::initAttributes() {
	isValidAksharaStart = false;
	isOttu = false;
	isVowelModifier = false;
	isYogaVaaha = false;
	isPartCharacter = false;
	hasVowelModifier = false;
	isEEDheergha = false;
	isArkaaOttu = false;
	isSpecialSymbol = false;
}

OCR_ClassLabel::OCR_ClassLabel() {
	initAttributes();
}

OCR_ClassLabel::OCR_ClassLabel(wchar_t _code) {
	unicodes.push_back(_code);
	initAttributes();
}

OCR_ClassLabel::OCR_ClassLabel(wchar_t _code1, wchar_t _code2) {
	unicodes.push_back(_code1);
	unicodes.push_back(_code2);
	initAttributes();
}

OCR_ClassLabel::OCR_ClassLabel(wchar_t _code1, wchar_t _code2, wchar_t _code3, wchar_t _code4) {
	unicodes.push_back(_code1);
	unicodes.push_back(_code2);
	unicodes.push_back(_code3);
	unicodes.push_back(_code4);
	initAttributes();
}

OCR_ClassLabel::~OCR_ClassLabel() {
}

wchar_t OCR_ClassLabel::getUnicode() {
	return unicodes[0];
}

wchar_t OCR_ClassLabel::getVowelModifier() {
	return unicodes[1];
}

wchar_t OCR_ClassLabel::resolveEEDheergha(wchar_t baseVowelModifier) {
	switch (baseVowelModifier) {
	case 0xCBF:
		return 0xCC0;
	case 0xCC6:
		return 0xCC7;
	case 0xCCA:
		return 0xCCB;
	default:
		return 0xCC0;//return unicodes[1];
	}
}

OCR_ClassLabel whiteSpaceLabel = OCR_ClassLabel(0x20); // white space character

map<string, OCR_ClassLabel> classLabels;

OCR_ClassLabel &getClassLabel(string labelName) {
	map<string, OCR_ClassLabel>::iterator p = classLabels.find(labelName);
	if (p != classLabels.end()) {
		return p->second;
	}
	return whiteSpaceLabel;
}

void addVowel(string labelName, wchar_t labelUnicode) {
	OCR_ClassLabel label = OCR_ClassLabel(labelUnicode);
	label.isValidAksharaStart = true;
	classLabels.insert(make_pair(labelName, label));
}

void addYogavaaha(string labelName, wchar_t labelUnicode) {
	OCR_ClassLabel label = OCR_ClassLabel(labelUnicode);
	label.isYogaVaaha = true;
	classLabels.insert(make_pair(labelName, label));
}

string suffixNames[] = { "aa", "i", "u", "uu", "e", "o", "au" };
wchar_t suffixUnicodes[] = { 0xCBE, 0xCBF, 0xCC1, 0xCC2, 0xCC6, 0xCCA, 0xCCC };

void addConsonantGroup(string baseName, wchar_t baseUnicode) {
	OCR_ClassLabel label = OCR_ClassLabel(baseUnicode);
	label.isValidAksharaStart = true;
	classLabels.insert(make_pair(baseName + "a", label));

	for (int i = 0; i < 7; i++) {
		label = OCR_ClassLabel(baseUnicode, suffixUnicodes[i]);
		label.isValidAksharaStart = true;
		label.hasVowelModifier = true;
		classLabels.insert(make_pair(baseName + suffixNames[i], label));
	}

	label = OCR_ClassLabel(baseUnicode, 0xCCD);
	label.isValidAksharaStart = true;
	label.hasVowelModifier = true; //TODO recheck if hasVowelModifier needs to be true.
	classLabels.insert(make_pair(baseName, label));

	label = OCR_ClassLabel(0xCCD, baseUnicode);
	label.isOttu = true;
	classLabels.insert(make_pair(baseName + "Ottu", label));
}

void addHalegannadaConsonantGroup(string baseName, wchar_t baseUnicode) {
	OCR_ClassLabel label = OCR_ClassLabel(baseUnicode);
	label.isValidAksharaStart = true;
	classLabels.insert(make_pair(baseName + "aOld", label));

	for (int i = 0; i < 7; i++) {
		label = OCR_ClassLabel(baseUnicode, suffixUnicodes[i]);
		label.isValidAksharaStart = true;
		label.hasVowelModifier = true;
		classLabels.insert(make_pair(baseName + suffixNames[i] + "Old", label));
	}

	label = OCR_ClassLabel(baseUnicode, 0xCCD);
	label.isValidAksharaStart = true; //TODO verify as to why this flag is set
	label.hasVowelModifier = true; //TODO recheck if hasVowelModifier needs to be true.
	classLabels.insert(make_pair(baseName + "Old", label));

	//label = OCR_ClassLabel(0xCCD, baseUnicode);
	//label.isVotthu = true;
	//classLabels.insert(make_pair(baseName + "Old" + "Ottu", label));
}

void addDoubleOttuGroup(string baseName, wchar_t firstConsonantUnicode, wchar_t secondConsonantUnicode) {
	OCR_ClassLabel label;
	label = OCR_ClassLabel(0x0CCD, firstConsonantUnicode, 0x0CCD, secondConsonantUnicode);
	label.isOttu = true;
	classLabels.insert(make_pair(baseName, label));
}

void addDisjointVowelModifiers() {
	OCR_ClassLabel label = OCR_ClassLabel(0xCC0); //eeDheergha
	label.unicodes.push_back(0xCC7); // iiDheergha
	label.unicodes.push_back(0xCCB); // ooDheergha
	label.isVowelModifier = true;
	label.isEEDheergha = true;
	classLabels.insert(make_pair("eeDheergha", label));

	label = OCR_ClassLabel(0xCC3);
	label.isVowelModifier = true;
	classLabels.insert(make_pair("rruMatra", label));

	label = OCR_ClassLabel(0xCC4);
	label.isVowelModifier = true;
	classLabels.insert(make_pair("rruuMatra", label));

	label = OCR_ClassLabel(0xCC8);
	label.isVowelModifier = true;
	classLabels.insert(make_pair("aiMatra", label));
}

void addArkaaOttu(string labelName) {
	OCR_ClassLabel label = OCR_ClassLabel(0xCB0, 0xCCD);
	label.isArkaaOttu = true;
	classLabels.insert(make_pair(labelName, label));
}

void addPartCharacter(string labelName, int id) {
	OCR_ClassLabel label = OCR_ClassLabel();
	label.isPartCharacter = true;
	label.partCharacterId = id;
	classLabels.insert(make_pair(labelName, label));
}

void addSpecialSymbol(string labelName, wchar_t labelUnicode) {
	OCR_ClassLabel label = OCR_ClassLabel(labelUnicode);
	label.isSpecialSymbol = true;
	classLabels.insert(make_pair(labelName, label));
}

void addPartCharacters() {
	addPartCharacter("rruPart", 1);
	//addPartCharacter("rruLpart");
	//addPartCharacter("yarruPart");

	addPartCharacter("aaDheergha", 26);
	addPartCharacter("uMatra", 2);
	addPartCharacter("uuMatra", 3);
	//addPartCharacter("auMatra");

	addPartCharacter("ngnjaaPart", 4);
	addPartCharacter("ngnjePart", 5);
	addPartCharacter("ngnjauPart", 6);

	addPartCharacter("j1aPart", 7);
	addPartCharacter("j1aaPart", 8);
	addPartCharacter("j1uPart", 9);
	addPartCharacter("j1uuPart", 10); //change this to yuupart
	addPartCharacter("j1oPart", 11);
	addPartCharacter("j1auPart", 12);
	addPartCharacter("j1MPart", 13);

	addPartCharacter("ymj1aaDheergha", 14);
	//addPartCharacter("ymj1uPart");
	//addPartCharacter("ymj1uuPart");
	addPartCharacter("ymj1auPart", 15);
	//addPartCharacter("ymj1Halanth");

	addPartCharacter("b1apart", 27);

	addPartCharacter("yPart", 16);
	addPartCharacter("yiPart", 17);
	//addPartCharacter("yiMPart");
	addPartCharacter("yePart", 18);
	addPartCharacter("yaEPart", 19);
	addPartCharacter("yaaEPart", 20);
	addPartCharacter("yuEPart", 21);
	addPartCharacter("yuuMEPart", 22);
	addPartCharacter("yuuEPart", 25);
	addPartCharacter("yeEPart", 23);
	addPartCharacter("yoEPart", 24);
	//addPartCharacter("yeMPart");
}

void addSpecialSymbols() {
	// http://www.asciitable.com/
	addSpecialSymbol("apostrophe", 0x27);
	addSpecialSymbol("leftApostrophe", 0x27);
	addSpecialSymbol("rightApostrophe", 0x27);
	addSpecialSymbol("openDquote", 0x22);
	addSpecialSymbol("closeDquote", 0x22);
	addSpecialSymbol("caret", 0x5e);
	addSpecialSymbol("tilde", 0x7e);
	addSpecialSymbol("dash", 0x2d);
	addSpecialSymbol("underscore", 0x5f);
	addSpecialSymbol("comma", 0x2c);
	addSpecialSymbol("dot", 0x2e);

	addSpecialSymbol("lSqBracket", 0x5b);
	addSpecialSymbol("rSqBracket", 0x5d);
	addSpecialSymbol("lCurlyBracket", 0x7b);
	addSpecialSymbol("rCurlyBracket", 0x7d);
	addSpecialSymbol("lParenthesis", 0x28);
	addSpecialSymbol("rParenthesis", 0x29);
	addSpecialSymbol("exclamation", 0x21);
	addSpecialSymbol("pipe", 0x7c);
	addSpecialSymbol("colon", 0x3a);
	addSpecialSymbol("semicolon", 0x3b);

	addSpecialSymbol("atSign", 0x40);
	addSpecialSymbol("hash", 0x23);
	addSpecialSymbol("dollar", 0x24);
	addSpecialSymbol("percent", 0x25);
	addSpecialSymbol("ampersand", 0x26);
	addSpecialSymbol("asterisk", 0x2a);
	addSpecialSymbol("add", 0x2b);
	addSpecialSymbol("lessThan", 0x3c);
	addSpecialSymbol("greaterThan", 0x3e);
	addSpecialSymbol("question", 0x3f);
	addSpecialSymbol("slash", 0x2f);
	addSpecialSymbol("backslash", 0x5c);
	addSpecialSymbol("div", 0xf7);
	addSpecialSymbol("equals", 0x3d);

	addSpecialSymbol("zero", 0x30);
	addSpecialSymbol("one", 0x31);
	addSpecialSymbol("two", 0x32);
	addSpecialSymbol("three", 0x33);
	addSpecialSymbol("four", 0x34);
	addSpecialSymbol("five", 0x35);
	addSpecialSymbol("six", 0x36);
	addSpecialSymbol("seven", 0x37);
	addSpecialSymbol("eight", 0x38);
	addSpecialSymbol("nine", 0x39);

	addSpecialSymbol("sonne", 0xce6);
	addSpecialSymbol("ondu", 0xce7);
	addSpecialSymbol("eraDu", 0xce8);
	addSpecialSymbol("muuru", 0xce9);
	addSpecialSymbol("naalku", 0xcea);
	addSpecialSymbol("aidu", 0xceb);
	addSpecialSymbol("aaru", 0xcec);
	addSpecialSymbol("eeLu", 0xced);
	addSpecialSymbol("enTu", 0xcee);
	addSpecialSymbol("ombattu", 0xcef);
}

map<int, string> baseClassMapping;
map<int, string> ottuClassMapping;
map<int, string> ssClassMapping;

void loadClassMappings(string classMappingsFileName, map<int, string> &classMapping) {
	ifstream in(classMappingsFileName.c_str());
	if (!in) {
		cout << "Cannot open file '" << classMappingsFileName << "' for reading.\n";
		outFileLog << "Cannot open file '" << classMappingsFileName << "' for reading.\n";
		return;
	}
	//Each line of the file contains class-number and class-name separated by a comma.
	int MAX_LINE_LENGTH = 128;
	char str[MAX_LINE_LENGTH];
	while (!in.eof()) {
		in.getline(str, MAX_LINE_LENGTH);
		char *firstToken = strtok(str, ",");
		char *secondToken = strtok(NULL, ",");
		if (firstToken == NULL || secondToken == NULL) {
			continue;
		}
		int classNumber = atoi(firstToken);
		string className(secondToken);
		classMapping.insert(pair<int, string>(classNumber, className));
	}
	in.close();
}

void loadClassLabels() {
	cout << "Loading Kannada Class Labels... ";
	outFileLog << "\nLoading Kannada Class Labels...\n";
	clock_t cBegin = clock();
	addYogavaaha("anuswara", 0xC82);
	addYogavaaha("visarga", 0xC83);

	addVowel("a", 0xC85);
	addVowel("aa", 0xC86);
	addVowel("i", 0xC87);
	addVowel("ii", 0xC88);
	addVowel("u", 0xC89);
	addVowel("uu", 0xC8A);
	addVowel("rru", 0xC8B);
	//addVowel("rruu", 0xC8C);
	addVowel("e", 0xC8E);
	addVowel("ee", 0xC8F);
	addVowel("ai", 0xC90);
	addVowel("o", 0xC92);
	addVowel("oo", 0xC93);
	addVowel("au", 0xC94);

	addConsonantGroup("k", 0xC95);
	addConsonantGroup("k1", 0xC96);
	addConsonantGroup("g", 0xC97);
	addConsonantGroup("g1", 0xC98);
	addConsonantGroup("ng", 0xC99);

	addConsonantGroup("ch", 0xC9A);
	addConsonantGroup("ch1", 0xC9B);
	addConsonantGroup("j", 0xC9C);
	addConsonantGroup("j1", 0xC9D);
	addConsonantGroup("nj", 0xC9E);

	addConsonantGroup("T", 0xC9F);
	addConsonantGroup("T1", 0xCA0);
	addConsonantGroup("D", 0xCA1);
	addConsonantGroup("D1", 0xCA2);
	addConsonantGroup("N", 0xCA3);

	addConsonantGroup("_t", 0xCA4);
	addConsonantGroup("_t1", 0xCA5);
	addConsonantGroup("_d", 0xCA6);
	addConsonantGroup("_d1", 0xCA7);
	addConsonantGroup("_n", 0xCA8);

	addConsonantGroup("p", 0xCAA);
	addConsonantGroup("p1", 0xCAB);
	addConsonantGroup("b", 0xCAC);
	addConsonantGroup("b1", 0xCAD);
	addConsonantGroup("m", 0xCAE);

	addConsonantGroup("y", 0xCAF);
	addConsonantGroup("r", 0xCB0);
	addConsonantGroup("_l", 0xCB2);
	addConsonantGroup("v", 0xCB5);
	addConsonantGroup("_sh", 0xCB6);
	addConsonantGroup("Sh", 0xCB7);
	addConsonantGroup("s", 0xCB8);
	addConsonantGroup("h", 0xCB9);
	addConsonantGroup("L", 0xCB3);

	addHalegannadaConsonantGroup("r", 0xCB1);
	addHalegannadaConsonantGroup("L", 0xCDE);

	addDoubleOttuGroup("krruOttu", 0x0C95, 0x0CC3);
	addDoubleOttuGroup("TrOttu", 0x0C9F, 0x0CB0);
	addDoubleOttuGroup("_trOttu", 0x0CA4, 0x0CB0);
	addDoubleOttuGroup("prOttu", 0x0CAA, 0x0CB0);
	addDoubleOttuGroup("vrOttu", 0x0CB5, 0x0CB0);
	addDoubleOttuGroup("raiOttu", 0x0CB0, 0x0CC8);

	addDisjointVowelModifiers();
	addArkaaOttu("arkaaOttu");
	addPartCharacters();
	addSpecialSymbols();

	loadClassMappings("./etc/Kannada/ClassMappingKanBase.txt", baseClassMapping);
	loadClassMappings("./etc/Kannada/ClassMappingKanOttu.txt", ottuClassMapping);
	loadClassMappings("./etc/Kannada/ClassMappingKanSS.txt", ssClassMapping);
	clock_t cEnd = clock();
	double timeInSecs = (cEnd - cBegin) / (float) CLOCKS_PER_SEC;
	cout << "Class Label loading complete. Time spent = " << timeInSecs << "secs\n";
	outFileLog << "Class Label loading complete. Time spent = " << timeInSecs << "secs\n";
}

/** \brief
 * Combines Part Characters. For example: will combine components va and uMatra to form Ma
 *
 */
string getCombinedCharacter(string previousLabel, int indicator) {
	string combinedLabel;
	string baseComponent;
	switch (indicator) {
	case 2: // uMatra
		// baseComponent=(k|k1|g|g1|ng| ch|ch1|j|j1|nj| T|T1|D|D1|N| t|t1|d|d1|n| |b|b1|m |y|r|l|sh|Sh|s|h|L)
		// (baseComponent+a)+uMatra=baseComponent+u;
		// va+uMatra=ma; vi+uMatra=mi; ve+uMatra=me
		if (previousLabel == "va") {
			combinedLabel = "ma";
		} else if (previousLabel == "vi") {
			combinedLabel = "mi";
		} else if (previousLabel == "ve") {
			combinedLabel = "me";
		} else if (previousLabel == "ma") {
			combinedLabel = "mu";
		} else {
			string baseComponent = previousLabel.substr(0, previousLabel.size() - 1);
			combinedLabel = baseComponent + "u";
		}
		break;
	case 3: // uuMatra
		// baseComponent=(k|k1|g|g1|ng| ch|ch1|j|j1|nj| T|T1|D|D1|N| t|t1|d|d1|n| |b|b1|m |y|r|l|sh|Sh|s|h|L)
		// (baseComponent+a)+uuMatra=baseComponent+uu;
		// (baseComponent(except m,y,j1)+e)+uuMatra=baseComponent+o
		// ve+uuMatra=mo;
		if (previousLabel == "ve") {
			combinedLabel = "mo";
		} else {
			int labelEnd = previousLabel.size() - 1;
			baseComponent = previousLabel.substr(0, labelEnd);
			if (previousLabel[labelEnd] == 'a') {
				combinedLabel = baseComponent + "uu";
			} else {
				combinedLabel = baseComponent + "o";
			}
		}
		break;
	case 26: // aaDheergha
		// (baseComponent(include p,p1 and exclude ng,j1,nj,m)+a)+aaDheergha=baseComponent+aa
		if (previousLabel.size() > 4 && previousLabel.substr(previousLabel.size() - 4, previousLabel.size())
				== "Part") {
			baseComponent = previousLabel.substr(0, previousLabel.size() - 4);
			combinedLabel = baseComponent + "aa";
		} else {
			baseComponent = previousLabel.substr(0, previousLabel.size() - 1);
			combinedLabel = baseComponent + "aa";
		}
		break;
	case 4: // ngnjaaPart
		if (previousLabel == "ng" || previousLabel == "nj") {
			combinedLabel = previousLabel + "aa";
		} else {
			//log error
		}
		break;
	case 5: // ngnjePart
		if (previousLabel == "ng" || previousLabel == "nj") {
			combinedLabel = previousLabel + "e";
		} else {
			//log error
		}
		break;
	case 6: // ngnjauPart
		if (previousLabel == "ng" || previousLabel == "nj") {
			combinedLabel = previousLabel + "au";
		} else {
			//log error
		}
		break;
	case 7: // j1aPart
		if (previousLabel == "ra") {
			combinedLabel = "j1a";
		} else if (previousLabel == "ri") {
			combinedLabel = "j1i";
		} else if (previousLabel == "re") {
			combinedLabel = "j1e";
		} else {
			//log error
		}
		break;
	case 8: // j1aaPart
		if (previousLabel == "ra") {
			combinedLabel = "j1aa";
		} else {
			//log error
		}
		break;
	case 9: // j1uPart
		if (previousLabel == "ra") {
			combinedLabel = "j1u";
		} else {
			//log error
		}
		break;
	case 10: // j1uuPart
		if (previousLabel == "ra") {
			combinedLabel = "j1uu";
		} else {
			//log error
		}
		break;
	case 11: // j1oPart
		if (previousLabel == "re") {
			combinedLabel = "j1o";
		} else {
			//log error
		}
		break;
	case 12: // j1auPart
		if (previousLabel == "ra") {
			combinedLabel = "j1au";
		} else {
			//log error
		}
		break;
	case 14: // ymj1aaDheergha
		if (previousLabel == "va") {
			combinedLabel = "maa";
		} else {
			//log error
		}
		break;
	case 15: // ymj1auPart
		if (previousLabel == "va") {
			combinedLabel = "mau";
		} else {
			//log error
		}
		break;
	case 19: //yaEPart
		if (previousLabel == "anuswara") {
			combinedLabel = "ya";
		} else {
			//Handle this event once rruLpart samples are added
		}
		break;
	case 20: //yaaEPart
		if (previousLabel == "anuswara") {
			combinedLabel = "yaa";
		} else {
			//Handle this event once rruLpart samples are added
		}
		break;
	case 21: //yuEPart
		if (previousLabel == "anuswara") {
			combinedLabel = "yu";
		} else {
			//Handle this event once rruLpart samples are added
		}
		break;
	case 22: //yuuMEPart
		if (previousLabel == "anuswara") {
			combinedLabel = "yuu";
		} else {
			//Handle this event once rruLpart samples are added
		}
		break;
	case 23: //yeEPart
		if (previousLabel == "anuswara") {
			combinedLabel = "ye";
		} else {
			//Handle this event once rruLpart samples are added
		}
		break;
	case 24: //yoEPart
		if (previousLabel == "anuswara") {
			combinedLabel = "yo";
		} else {
			//Handle this event once rruLpart samples are added
		}
		break;
	}
	return combinedLabel;
}

string getCombinedCharacter(string previousLabel, int indicator, string nextLabel) {
	//case 13: j1MPart
	string combinedLabel;
	if (indicator != 13) {
		//log error
	}
	if (previousLabel == "ra") {
		if (nextLabel == "uMatra") {
			combinedLabel = "j1a";
		} else if (nextLabel == "ymj1aaDheergha") {
			combinedLabel = "j1aa";
		} else if (nextLabel == "ymj1auPart") {
			combinedLabel = "j1au";
		} else {
			//log error
		}
	} else if (previousLabel == "ri") {
		if (nextLabel == "uMatra") {
			combinedLabel = "j1i";
		} else {
			//log error
		}
	} else if (previousLabel == "re") {
		if (nextLabel == "uMatra") {
			combinedLabel = "j1e";
		} else if (nextLabel == "uuMatra") {
			combinedLabel = "j1o";
		} else {
			//log error
		}
	} else {
		//log error;
	}
	return combinedLabel;
}

string getCombinedCharacter(int indicator, string nextLabel) {
	string combinedLabel;
	switch (indicator) {
	case 1: // rruPart
		if (nextLabel == "uMatra") {
			combinedLabel = "rru";
		} else {
			//log error
		}
		break;
	case 16: // yPart
		if (nextLabel == "uMatra") {
			combinedLabel = "ya";
		} else if (nextLabel == "ymj1aaDheergha") {
			combinedLabel = "yaa";
		} else if (nextLabel == "ymj1auPart") {
			combinedLabel = "yau";
		} else if (nextLabel == "yuuEPart") {
			combinedLabel = "yuu";
		} else {
			//log error
		}
		break;
	case 17: // yiPart
		if (nextLabel == "uMatra") {
			combinedLabel = "yi";
		} else {
			//log error
		}
		break;
	case 18: // yePart
		if (nextLabel == "uMatra") {
			combinedLabel = "ye";
		} else if (nextLabel == "uuMatra") {
			combinedLabel = "yo";
		} else {
			//log error
		}
		break;
	}
	return combinedLabel;
}

char labels[][50] = { "a", "aa", "i", "ii", "u", "uu","rru","e", "ee","ai"/*10*/, "o", "oo", "au","anuswara","visarga",
			"k","ka","kaa","ki","ku"/*20*/,"kuu","ke","ko","kau",
			"k1a","k1aa","k1i","k1u","k1e","k1o"/*30*/,
			"ga","gaa","gi","gu","guu","ge","go","gau",
			"g1","g1a"/*40*/,"g1aa","g1u","g1e","g1au",
			"ch","cha","chaa","chi","chu","chuu"/*50*/,"che","chee","cho","chau",
			"ch1a","ch1aa","ch1i","ch1o",
			"ja","jaa"/*60*/,"ji","ju","juu","je","jo",
			"nja",
			"T","Ta","Taa","Ti","TiLeft","Tu"/*72*/,"Te","To",
			"T1a",
			"Da","Daa","Di","Du","Duu","De","Do"/*82*/,
			"D1a","D1i",
			"N","Na","Naa","Ni","Ne","Nu",
			"_t","_ta"/*92*/,"_taa","_ti","_tu", "_tuu","_te","_to","_tau",
			"_t1a","_t1i",
			"_d"/*102*/,"_da","_daa","_di","_du","_duu","_de","_do","_dau",
			"_d1a","_d1aa"/*112*/,"_d1i","_d1u","_d1e",
			"_n","_na","_naa","_ni","_nu", "_nuu","_ne"/*122*/,"_no","_nau",
			"p","pa","paa","pi","pu","puu","pe","po"/*132*/,"pau",
			"p1a","p1aa","p1i","p1u","p1uu","p1e",
			"b","ba","baa"/*142*/,"bi","bu", "buu","be","bo","bau",
			"b1a","b1aa","b1i","b1u"/*152*/,"b1uu","b1e","b1o",
			"m","ma","maa","mi","mu","muu","me"/*162*/,"mo","mau",
			"y","ya","yaa","yi","yu","yuu","ye","yo"/*172*/,"yau",
			"r","ra","raa","ri","ru","ruu","re","ro",
			"_l"/*182*/,"_la","_laa","_li","_lu", "_luu","_le","_lo", "_lau",
			"v","va"/*192*/,"vaa","vi","vu", "vuu","ve","vo","vau",
			"_sh","_sha","_shaa"/*202*/,"_shi","_shu","_shuu","_she", "_sho","_shau",
			"Sh","Sha","Shaa","Shi"/*212*/,"Shu","Shuu","She","Shau",
			"s","sa","saa","si","su","suu"/*222*/,"se","so","sau",
			"ha","haa","hi","hu","huu","he","ho"/*232*/,"hau",
			"L","La","Laa","Li","Lu","Luu","Le","Lo",
			"rOld"/*242*/,"raOld","raaOld","riOld","ruOld","reOld","roOld","rauOld",
			"LOld","LaOld","LaaOld"/*252*/,"LiOld","LuOld","LuuOld","LeOld","LoOld","LauOld",
			"rruMatra","aiMatra",
			"kOttu","k1Ottu"/*262*/,"gOttu","g1Ottu","ngOttu",
			"chOttu","ch1Ottu","jOttu","njOttu",
			"TOttu","T1Ottu","DOttu"/*272*/,"NOttu",
			"_tOttu","_t1Ottu","_dOttu","_d1Ottu","_nOttu",
			"pOttu","p1Ottu","bOttu","b1Ottu"/*282*/,"mOttu",
			"yOttu","rOttu","_lOttu","vOttu","ShOttu","sOttu","hOttu","LOttu",
			"arkaaOttu"/*292*/,
			"krruMatra","_trOttu", "vrOttu","prOttu","raiMatra","TrOttu",
			"aaDheergha","eeDheergha","uMatra","uuMatra"/*302*/,"rruPart",
			"yPart","yiPart","yePart","yaEPart","yaaEPart","yuEPart","yuuMEPart","yuuEPart","yeEPart"/*312*/,"yoEPart","ymj1aaDheergha",
			"ondu","eraDu","muuru","naalku","aidu","aaru","eeLu","enTu"/*322*/,"ombattu",
			"one","two","three","four","five","six","seven","eight","nine"/*332*/,
			"lParenthesis","rParenthesis", "lAngleBracket","rAngleBracket","lCurlyBracket","rCurlyBracket","lSqBracket","rSqBracket",
			"apostrophe","leftApostrophe"/*342*/,"rightApostrophe","openDquote","closeDquote","slash","backSlash",
			"add","div","percent","dash","equals"/*352*/,"dollar",
			"exclamation","danda","tilde","comma","dot","semicolon","ampersand","asterisk","atSign"/*362*/,"caret","colon","hash","question","underscore"/*367*/
};

string getLabelName(int svmClass) {
	return string(labels[svmClass - 1]);
}

string getLabelName(int svmClass, int classGroup) {
	map<int, string>::iterator p;
	switch (classGroup) {
	case BASE_GROUP:
		p = baseClassMapping.find(svmClass);
		if (p != baseClassMapping.end()) {
			return p->second;
		}
		break;
	case OTTU_GROUP:
		p = ottuClassMapping.find(svmClass);
		if (p != ottuClassMapping.end()) {
			return p->second;
		}
		break;
	case SPECIAL_SYMBOL_GROUP:
		p = ssClassMapping.find(svmClass);
		if (p != ssClassMapping.end()) {
			return p->second;
		}
		break;
	}
	return "";
}

}
