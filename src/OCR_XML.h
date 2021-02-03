#ifndef OCR_XML_H_
#define OCR_XML_H_

#include "OCR_Util.h"
#include "OCR_Page.h"
#include "tinyxml2.h"

namespace IISc_KannadaClassifier {

int readBlocksFromXML(const char* inputXmlPath, CvRect *textBlocks, double *rotationAngle);
int readBlocksAndImageFromXML(const char *xmlFilePath, CvRect *textBlocks, const char *imageFilePath, double *rotationAngle);
void writeOcrOutputXML(OCR_Page &page, const char* outputXmlPath);

}

#endif /* OCR_XML_H_ */
