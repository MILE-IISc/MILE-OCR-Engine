#include "OCR_XML.h"
#include <stdio.h>
#include "base64.h"

using namespace tinyxml2;
using namespace std;

namespace IISc_KannadaClassifier {

int readBlocksFromXML(XMLDocument &doc, CvRect *textBlocks) {
	XMLElement* blockElement = doc.FirstChildElement("page")->FirstChildElement("block");
	int b = 0;
	while (blockElement != 0) {
		int rowStart = blockElement->IntAttribute("rowStart");
		int rowEnd = blockElement->IntAttribute("rowEnd");
		int colStart = blockElement->IntAttribute("colStart");
		int colEnd = blockElement->IntAttribute("colEnd");
		CvRect *cvRect = &textBlocks[b];
		cvRect->x = colStart;
		cvRect->y = rowStart;
		cvRect->width = colEnd - colStart + 1;
		cvRect->height = rowEnd - rowStart + 1;
		blockElement = blockElement->NextSiblingElement("block");
		b++;
	}
	return b;
}

int readBlocksFromXML(const char* inputXmlPath, CvRect *textBlocks) {
	XMLDocument doc;
	doc.LoadFile(inputXmlPath);
	return readBlocksFromXML(doc, textBlocks);
}

int readBlocksAndImageFromXML(const char *xmlFilePath, CvRect *textBlocks, const char *imageFilePath) {
	XMLDocument doc;
	doc.LoadFile(xmlFilePath);
	XMLElement* imageDataElement = doc.FirstChildElement("page")->FirstChildElement("imageData");
	const char* base64Image = imageDataElement->GetText();
	int base64Length = strlen(base64Image);
	cout << "Read base64 image from XML. Length = " << base64Length << "\n";
	// cout << "----------\n" << base64Image << "\n----------\n";
	size_t imageLength;
	cout << "Calling base64_decode ...\n";
	const unsigned char *image = base64_decode((const unsigned char*)base64Image, base64Length, &imageLength);
	cout << "Decoded image. Length = " << imageLength << "\n";
	int bytesWritten = writeDataToFile(imageFilePath, image, imageLength);
	cout << "Saved image contents to file (bytesWritten = " << bytesWritten << "): " << imageFilePath << "\n";
	return readBlocksFromXML(doc, textBlocks);
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
		CvRect &blockRect = block.boundingBox;
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
				wordElement->SetAttribute("rowStart", line.lineTop + blockRect.y);
				wordElement->SetAttribute("rowEnd", line.lineBottom + blockRect.y);
				wordElement->SetAttribute("colStart", word.xStart + blockRect.x);
				wordElement->SetAttribute("colEnd", word.xEnd + blockRect.x);
			}
			lineElement->SetAttribute("LineNumber", lineNumber++);
			lineElement->SetAttribute("rowStart", line.lineTop + blockRect.y);
			lineElement->SetAttribute("rowEnd", line.lineBottom + blockRect.y);
			lineElement->SetAttribute("colStart", lineColStart + blockRect.x);
			lineElement->SetAttribute("colEnd", lineColEnd + blockRect.x);
		}
		blockElement->SetAttribute("BlockNumber", b + 1);
		blockElement->SetAttribute("type", "Text");
		blockElement->SetAttribute("rowStart", blockRect.y);
		blockElement->SetAttribute("rowEnd", blockRect.y + blockRect.height - 1);
		blockElement->SetAttribute("colStart", blockRect.x);
		blockElement->SetAttribute("colEnd", blockRect.x + blockRect.width - 1);
	}

	doc->SaveFile(outputXmlPath);
}

}
