#ifndef CRNN_INFER_H_
#define CRNN_INFER_H_

#include <fstream>
#include <unordered_map>
#include <string>
#include <opencv/cv.h>
using std::unordered_map;
using std::string;

namespace IISc_KannadaClassifier {

class InferCRNN {
private:
    InferCRNN (string modelPath);
    static unordered_map<string, InferCRNN*> instanceMap;  
    string modelPath;

public:
    static InferCRNN* getInstance(string modelPath);
    string infer(IplImage *img);
};

};

#endif /* CRNN_INFER_H_ */