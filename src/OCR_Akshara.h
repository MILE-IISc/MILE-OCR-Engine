#ifndef OCR_AKSHARA_H_
#define OCR_AKSHARA_H_

#include "OCR_Util.h"
#include "OCR_ClassLabel.h"
#include <string>
#include <fstream>
using std::string;
using std::ostream;

namespace IISc_KannadaClassifier {

class OCR_Akshara {
public:
	vector<string> labelNames;
	vector<wchar_t> unicodes;
	bool isInvalid;
	OCR_Akshara(string labelName);
	virtual ~OCR_Akshara();
	void pushLabel(string labelName);
	void replaceLastLabel(string labelName);
	void emitAksharaUnicode();
};
ostream &operator<<(ostream &out, OCR_Akshara &akshara);

}

#endif /* OCR_AKSHARA_H_ */
