#ifndef OCR_CLASSLABEL_H_
#define OCR_CLASSLABEL_H_

#include "OCR_Util.h"
#include <vector>
#include <string>
#include <wchar.h>
#include <map>
using std::string;
using std::iterator;
using std::map;
using std::make_pair;
using std::pair;

#include "OCR_GlobalDef.h"

namespace IISc_KannadaClassifier {

class OCR_ClassLabel {
public:
	bool isValidAksharaStart;
	bool isOttu;
	bool isVowelModifier;
	bool isYogaVaaha;
	bool isPartCharacter;
	bool hasVowelModifier;
	bool isEEDheergha;
	bool isArkaaOttu;
	bool isSpecialSymbol;

	vector<wchar_t> unicodes;
	int partCharacterId;

	void initAttributes();
	OCR_ClassLabel();
	OCR_ClassLabel(wchar_t _code);
	OCR_ClassLabel(wchar_t _code1, wchar_t _code2);
	OCR_ClassLabel(wchar_t _code1, wchar_t _code2, wchar_t _code3, wchar_t _code4);
	virtual ~OCR_ClassLabel();
	wchar_t getUnicode();
	wchar_t getVowelModifier();
	static wchar_t resolveEEDheergha(wchar_t baseVowelModifier);
};

extern OCR_ClassLabel whiteSpaceLabel;
OCR_ClassLabel &getClassLabel(string labelName);
void loadClassLabels();
string getCombinedCharacter(string previousLabel, int indicator);
string getCombinedCharacter(string previousLabel, int indicator, string nextLabel);
string getCombinedCharacter(int indicator, string nextLabel);
string getLabelName(int svmClass);
string getLabelName(int svmClass, int classGroup);

}

#endif /* OCR_CLASSLABEL_H_ */
