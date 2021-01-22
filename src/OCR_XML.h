#ifndef OCR_XML_H_
#define OCR_XML_H_

#include "OCR_Util.h"
#include "OCR_Page.h"
#include "tinyxml2.h"

namespace IISc_KannadaClassifier {

void readBlocksFromXML(const char* inputXmlPath, vector<CvRect> *textBlocks);
void writeOcrOutputXML(OCR_Page &page, const char* outputXmlPath);

}

#endif /* OCR_XML_H_ */
