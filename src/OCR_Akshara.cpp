#include "OCR_Akshara.h"

namespace IISc_KannadaClassifier {

OCR_Akshara::OCR_Akshara(string labelName) {
	labelNames.push_back(labelName);
}

OCR_Akshara::~OCR_Akshara() {
}

void OCR_Akshara::pushLabel(string labelName) {
	labelNames.push_back(labelName);
}

void OCR_Akshara::replaceLastLabel(string labelName) {
	labelNames.pop_back();
	labelNames.push_back(labelName);
}

void OCR_Akshara::emitAksharaUnicode() {
	OCR_ClassLabel &firstLabel = getClassLabel(labelNames[0]);
	if (labelNames.size() > 1 && firstLabel.hasVowelModifier) {
		// Emit unicode for consonant part and store vowel modifier part without omitting it
		wchar_t baseVowelModifier = firstLabel.getVowelModifier();
		unicodes.push_back(firstLabel.getUnicode());
		for (unsigned int i = 1; i < labelNames.size() - 1; i++) {
			OCR_ClassLabel &label = getClassLabel(labelNames[i]);
			if (label.isVowelModifier) {
				if (label.isEEDheergha) {
					baseVowelModifier = OCR_ClassLabel::resolveEEDheergha(baseVowelModifier);
				} else {
					baseVowelModifier = label.getUnicode();
				}
			} else {
				copyVector(label.unicodes, unicodes);
			}
		}
		OCR_ClassLabel &lastLabel = getClassLabel(labelNames[labelNames.size() - 1]);
		if (lastLabel.isYogaVaaha || lastLabel.isArkaaOttu) {
			// Emit stored-vowel-modifier's unicode followed by that of yogaVaaha/arkaaVottu.
			unicodes.push_back(baseVowelModifier);
			copyVector(lastLabel.unicodes, unicodes);
		} else if (lastLabel.isVowelModifier) {
			// Delete (ignore) the earlier stored-vowel-modifier and emit only current label's unicode
			if (lastLabel.isEEDheergha) {
				unicodes.push_back(OCR_ClassLabel::resolveEEDheergha(baseVowelModifier));
			} else {
				unicodes.push_back(lastLabel.getUnicode());
			}
		} else {
			//Emit last label's unicode followed by that of stored-vowel-modifier.
			copyVector(lastLabel.unicodes, unicodes);
			unicodes.push_back(baseVowelModifier);
		}
	} else {
		// Emit unicodes in the same order as appearing in the Akshara
		copyVector(firstLabel.unicodes, unicodes);
		for (unsigned int i = 1; i < labelNames.size(); i++) {
			OCR_ClassLabel &label = getClassLabel(labelNames[i]);
			copyVector(label.unicodes, unicodes);
		}
	}
}

ostream &operator<<(ostream &out, OCR_Akshara &akshara) {
	for (unsigned int i = 0; i < akshara.labelNames.size(); i++) {
		out << akshara.labelNames[i] << " ";
	}
	out << "-> " << std::hex;
	for (unsigned int i = 0; i < akshara.unicodes.size(); i++) {
		out << akshara.unicodes[i] << " ";
	}
	out << std::dec;
	return out;
}

}
