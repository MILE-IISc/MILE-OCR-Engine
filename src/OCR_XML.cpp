#include "OCR_XML.h"

using namespace tinyxml2;
using namespace std;

namespace IISc_KannadaClassifier {

char* itoa(unsigned int num) {
	return (char *) toString(num).c_str();
}

void writeOcrOutputXML(OCR_Page &page, const char* outputXmlPath) {
	XMLDocument* doc = new XMLDocument();
	XMLElement* pageElement = doc->NewElement("page");
	doc->InsertEndChild(pageElement);

	vector<OCR_Block> &blocks = page.blocks;
	for (unsigned int b = 0; b < blocks.size(); b++) {
		XMLElement* blockElement = pageElement->InsertNewChildElement("block");
		vector<OCR_Line> &lines = blocks[b].lines;
		for (unsigned int l = 0; l < lines.size(); l++) {
			XMLElement* lineElement = blockElement->InsertNewChildElement("line");
			vector<OCR_Word> &words = lines[l].words;
			for (unsigned int w = 0; w < words.size(); w++) {
				XMLElement* wordElement = lineElement->InsertNewChildElement("word");
				vector<OCR_Akshara> &aksharas = words[w].aksharas;
				vector<wchar_t> unicodes;
				for (unsigned int a = 0; a < aksharas.size(); a++) {
					copyVector(aksharas[a].unicodes, unicodes);
				}
				wordElement->SetAttribute("unicode", toString(unicodes).c_str());
				wordElement->SetAttribute("WordNumber", w + 1);
			}
			lineElement->SetAttribute("LineNumber", l + 1);
		}
		blockElement->SetAttribute("BlockNumber", b + 1);
	}

	doc->SaveFile(outputXmlPath);
}

}
