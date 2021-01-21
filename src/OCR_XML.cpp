#include "OCR_XML.h"

using namespace tinyxml2;
using namespace std;

namespace IISc_KannadaClassifier {

char* itoa(unsigned int num) {
	return (char *) toString(num).c_str();
}

void writeOcrOutputXML(OCR_Page &page, const char* outputXmlPath) {
	XMLDocument* doc = new XMLDocument();
	doc->InsertEndChild(doc->NewDeclaration(NULL));
	XMLElement* pageElement = doc->NewElement("page");
	pageElement->SetAttribute("xmlns", "http://mile.ee.iisc.ernet.in/schemas/ocr_output");
	pageElement->SetAttribute("skew", page.skewCorrected);
	doc->InsertEndChild(pageElement);

	vector<OCR_Block> &blocks = page.blocks;
	int lineNumber = 1;
	for (unsigned int b = 0; b < blocks.size(); b++) {
		XMLElement* blockElement = pageElement->InsertNewChildElement("block");
		OCR_Block &block = blocks[b];
		vector<OCR_Line> &lines = block.lines;
		for (unsigned int l = 0; l < lines.size(); l++) {
			XMLElement* lineElement = blockElement->InsertNewChildElement("line");
			OCR_Line &line = lines[l];
			vector<OCR_Word> &words = line.words;
			int lineColStart = 0, lineColEnd = 0;
			for (unsigned int w = 0; w < words.size(); w++) {
				XMLElement* wordElement = lineElement->InsertNewChildElement("word");
				OCR_Word &word = words[w];
				if (w == 0) {
					lineColStart = word.xStart;
				}
				if (w == words.size() - 1) {
					lineColEnd = word.xEnd;
				}
				vector<OCR_Akshara> &aksharas = word.aksharas;
				vector<wchar_t> unicodes;
				for (unsigned int a = 0; a < aksharas.size(); a++) {
					copyVector(aksharas[a].unicodes, unicodes);
				}
				wordElement->SetAttribute("unicode", toString(unicodes).c_str());
				wordElement->SetAttribute("WordNumber", w + 1);
				wordElement->SetAttribute("rowStart", line.lineTop);
				wordElement->SetAttribute("rowEnd", line.lineBottom);
				wordElement->SetAttribute("colStart", word.xStart);
				wordElement->SetAttribute("colEnd", word.xEnd);
			}
			lineElement->SetAttribute("LineNumber", lineNumber++);
			lineElement->SetAttribute("rowStart", line.lineTop);
			lineElement->SetAttribute("rowEnd", line.lineBottom);
			lineElement->SetAttribute("colStart", lineColStart);
			lineElement->SetAttribute("colEnd", lineColEnd);
		}
		blockElement->SetAttribute("BlockNumber", b + 1);
		blockElement->SetAttribute("type", "Text");
		blockElement->SetAttribute("rowStart", block.boundingBox.y);
		blockElement->SetAttribute("rowEnd", block.boundingBox.y + block.boundingBox.height - 1);
		blockElement->SetAttribute("colStart", block.boundingBox.x);
		blockElement->SetAttribute("colEnd", block.boundingBox.x + block.boundingBox.width - 1);
	}

	doc->SaveFile(outputXmlPath);
}

}
