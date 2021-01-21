#include "OCR_XML.h"

using namespace tinyxml2;
namespace IISc_KannadaClassifier {

string parseInputXML(string inputxml, vector<CvRect> &textBlocks) {
	XMLparser *xmlParser = new XMLparser();
    bool isTextBlkFound = false;

	xmlParser->readXMLdocument((char *) inputxml.c_str());
	for (unsigned int e = 0; e < xmlParser->elementList.size(); e++) {
		XMLelement *documentElement = xmlParser->elementList[e];


		if (documentElement->name == string("DOCUMENT")) {
			XMLelement *pageElement = documentElement->childs[0];


			if (pageElement->name == string("PAGE")) {
				if (pageElement->childs.size() == 0) {
					cout<< "The PAGE element in inputXml doesn't have any children and this indicates that the preProcessing is not yet done.\n";
					cout<< "Please run the preProcessing modules and then run this OCR\n";
					return "";
				}

				for (int i = pageElement->childs.size() - 1; i >= 0; i--) {
					XMLelement *textBlockElement = pageElement->childs[i];


					if (textBlockElement->name == string("TextBlock") || textBlockElement->name == string(
							"Cropping")) {
                        isTextBlkFound = true;
						string imageName = textBlockElement->childs[0]->content; //take value in inputImageURI
						int xStart, yStart, xEnd, yEnd;
						for (unsigned int j = 0; j < textBlockElement->childs.size(); j++) {
							XMLelement *blockElement = textBlockElement->childs[j];

							if (blockElement->name == string("BLOCK")) {
								for (unsigned int k = 0; k < blockElement->childs.size(); k++) {
									XMLelement *childOfBlock = blockElement->childs[k];
									string childName = childOfBlock->name;
									int childValue = atoi(childOfBlock->content);
									if (childName == string("topLx")) {
										xStart = childValue;

									} else if (childName == string("topLy")) {
										yStart = childValue;

									} else if (childName == string("bottomRx")) {
										xEnd = childValue;

									} else if (childName == string("bottomRy")) {
										yEnd = childValue;

									}
								}
								textBlocks.push_back(cvRect(xStart, yStart, xEnd - xStart + 1, yEnd - yStart
										+ 1));
							}
						}
						//cout<<"reading from text block"<<endl;
						//return imageName;
					}
				}
				for (int i = pageElement->childs.size() - 1; i >= 0; i--) {
					XMLelement *childOfPage = pageElement->childs[i];

					if ((childOfPage->name == string("GraphicsBlock") || childOfPage->name == string(
							"PictureBlock")) && isTextBlkFound == false) {
						cout << "No Text Blocks found\n";
						return "";
					}
				}
				XMLelement *lastChildOfPage = pageElement->childs[pageElement->childs.size() - 1];
				string imageName = "";
				if ((lastChildOfPage->name == string("TextBlock")) || (lastChildOfPage->name == string("PictureBlock")) ||(lastChildOfPage->name == string("GraphicsBlock"))){
                    imageName = lastChildOfPage->childs[0]->content;
                }
				else{
                    imageName = lastChildOfPage->childs[1]->content; //take value in OutputImageURI
                    }

				return imageName;
			}
		}
	}
	cout << "Erroneous inputXML.\n";
	return "";
}

char* itoa(unsigned int num) {
	return (char *) toString(num).c_str();
}

void writeOutputXML(OCR_Page &page, char *inputXml, char *outputXml, string outputFileNamePrefix, char *inputImageUri) {
	XMLparser *xmlParser = new XMLparser;
	xmlParser->readXMLdocument(inputXml);
	for (unsigned int e = 0; e < xmlParser->elementList.size(); e++) {
		XMLelement *documentElement = xmlParser->elementList[0];
		if (documentElement->name == string("DOCUMENT")) {
			XMLelement *pageElement = documentElement->childs[0];
			if (pageElement->name == string("PAGE")) {
				if (!page.isBlockSegmented) {
					XMLelement *textBlockElement = pageElement->makeChildElement("TextBlock");
					textBlockElement->setAttribute("TotalNumber", "1");
					XMLelement *inputImageUriElement = textBlockElement->makeChildElement("InputImageURI");
					inputImageUriElement->setContent(inputImageUri);
					XMLelement *outputImageUriElement = textBlockElement->makeChildElement("OutputImageURI");
					outputImageUriElement->setContent(inputImageUri);
					XMLelement *topmostBlockElement = textBlockElement->makeChildElement("BLOCK");
					topmostBlockElement->setAttribute("Number", "1");
					XMLelement *topmostBlockChild = topmostBlockElement->makeChildElement("topLx");
					topmostBlockChild->setContent("0");
					topmostBlockChild = topmostBlockElement->makeChildElement("topLy");
					topmostBlockChild->setContent("0");
					topmostBlockChild = topmostBlockElement->makeChildElement("bottomRx");
					topmostBlockChild->setContent(itoa(page.img->width));
					topmostBlockChild = topmostBlockElement->makeChildElement("bottomRy");
					topmostBlockChild->setContent(itoa(page.img->height));
				}
				for (int i = pageElement->childs.size() - 1; i >= 0; i--) {
					XMLelement *textBlockElement = pageElement->childs[i];
					if (textBlockElement->name == string("TextBlock") || textBlockElement->name == string(
							"Cropping")) {
						for (unsigned int j = 0; j < textBlockElement->childs.size(); j++) {
							XMLelement *topmostBlockElement = textBlockElement->childs[j];
							if (topmostBlockElement->name == string("BLOCK")) {
								OCR_Block &block = page.blocks[j - 2];
								XMLelement *unicodeElement = topmostBlockElement->makeChildElement("Unicode");
								string outputFileName = outputFileNamePrefix + "_" + toString(j - 1) + ".txt";
								unicodeElement->setAttribute("FileURI", (char *) outputFileName.c_str());
								vector<OCR_Line> &lines = block.lines;
								for (unsigned int l = 0; l < lines.size(); l++) {
									XMLelement *textLineElement = topmostBlockElement->makeChildElement(
											"TextLine");
									XMLelement *textLineBlock = textLineElement->makeChildElement("BLOCK");
									textLineBlock->setAttribute("Number", itoa(l + 1));
									XMLelement *textLineBlockChild = textLineBlock->makeChildElement("topLx");
									textLineBlockChild->setContent(itoa(block.boundingBox.x));
									textLineBlockChild = textLineBlock->makeChildElement("topLy");
									textLineBlockChild->setContent(itoa(lines[l].lineTop));
									textLineBlockChild = textLineBlock->makeChildElement("bottomRx");
									textLineBlockChild->setContent(itoa(block.boundingBox.x
											+ block.boundingBox.width - 1));
									textLineBlockChild = textLineBlock->makeChildElement("bottomRy");
									textLineBlockChild->setContent(itoa(lines[l].lineBottom));
									vector<OCR_Word> &words = lines[l].words;
									for (unsigned int w = 0; w < words.size(); w++) {
										XMLelement *textWordElement = textLineBlock->makeChildElement(
												"TextWord");
										XMLelement *textWordBlock =
												textWordElement->makeChildElement("BLOCK");
										textWordBlock->setAttribute("Number", itoa(w + 1));
										XMLelement *textWordBlockChild = textWordBlock->makeChildElement(
												"topLx");
										textWordBlockChild->setContent(itoa(words[w].xStart));
										textWordBlockChild = textWordBlock->makeChildElement("topLy");
										textWordBlockChild->setContent(itoa(lines[l].lineTop));
										textWordBlockChild = textWordBlock->makeChildElement("bottomRx");
										textWordBlockChild->setContent(itoa(words[w].xEnd));
										textWordBlockChild = textWordBlock->makeChildElement("bottomRy");
										textWordBlockChild->setContent(itoa(lines[l].lineBottom));
									}
								}
							}
						}
					}
				}
			}
		}
	}
	xmlParser->dumpInFile(outputXml);
	//xmlParser->verifyAgainstSchema((char *) string("src" + PATH_SEPARATOR + "OCR_XML_Old.xsd").c_str());
}

wstring vectorToString(vector<wchar_t> &unicodes) {
	wstring output;
	for (unsigned i = 0; i < unicodes.size(); i++) {
		output.push_back(unicodes[i]);
	}
	return output;
}

void writeOcrOutputXML(OCR_Page &page, char* outputXmlPath) {
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
				// wordElement->SetAttribute("unicode", vectorToString(unicodes));
				wordElement->SetAttribute("WordNumber", w + 1);
			}
			lineElement->SetAttribute("LineNumber", l + 1);
		}
		blockElement->SetAttribute("BlockNumber", b + 1);
	}

	doc->SaveFile(outputXmlPath);
}

}
