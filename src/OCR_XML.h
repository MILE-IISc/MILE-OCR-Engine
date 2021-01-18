#ifndef OCR_XML_H_
#define OCR_XML_H_

#include "xmlParser.h"
#include "OCR_Util.h"
#include "OCR_Page.h"

namespace IISc_KannadaClassifier {

string parseInputXML(string inputxml, vector<CvRect> &textBlocks);
void writeOutputXML(OCR_Page &page, char* inputXml, char* outputXml, string outputFileNamePrefix,
		char *inputImageUri);

}

#endif /* OCR_XML_H_ */
