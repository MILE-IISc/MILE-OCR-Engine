#include "CRNN_Infer.h"

namespace IISc_KannadaClassifier {

    std::unordered_map<string, InferCRNN*> InferCRNN::instanceMap;

    InferCRNN::InferCRNN (string modelPath) {
        this->modelPath = modelPath;
    }

    InferCRNN* InferCRNN::getInstance(string modelPath){
        InferCRNN* instance = InferCRNN::instanceMap[modelPath];
        if (instance == NULL) {
            instance = new InferCRNN(modelPath);
            instanceMap[modelPath] = instance;
        }
        return instance;
    }

    string InferCRNN::infer(IplImage *img) {
        return "";
    }
};