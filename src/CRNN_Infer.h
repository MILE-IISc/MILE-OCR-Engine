#ifndef CRNN_INFER_H_
#define CRNN_INFER_H_

#include <fstream>
#include <unordered_map>
#include <string>
#include <opencv/cv.h>
#include <tensorflow/cc/saved_model/loader.h>

using std::unordered_map;
using std::string;

using tensorflow::SavedModelBundle;
using tensorflow::SessionOptions;
using tensorflow::RunOptions;

namespace IISc_KannadaClassifier {

class CRNNModelConfig {
public:
	int width = 300;
	int height = 80;
	int timeSteps = 75;
};

class InferCRNN {
private:
    InferCRNN (string modelPath);
    static unordered_map<string, InferCRNN*> instanceMap;  
    string modelPath;
    SavedModelBundle model;
    SessionOptions session_options;
    RunOptions run_options;
    CRNNModelConfig modelConfig;

public:
    static InferCRNN* getInstance(string modelPath);
    string infer(IplImage *img);
};

};

#endif /* CRNN_INFER_H_ */
