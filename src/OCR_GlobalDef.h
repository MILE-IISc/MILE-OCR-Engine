#ifndef OCR_GLOBALDEF_H_
#define OCR_GLOBALDEF_H_

namespace IISc_KannadaClassifier {

static const float WORD_GAP_FRACTION = 0.35;
static const float LINE_MIDDLE_FRACTION = 0.35;
static const float OTTU_REFERENCE_BELOWBASELINE_FRACTION = 1.15; //1.15 * BaseLine
static const float PADAM_ASPECT_RATIO = 0.75; //width / ht
static const float SPECIALSYMBOL_ASPECT_RATIO = 0.42; // wid / ht
static const int MIN_OVERLAP_OF_COMPONENTS = 3; //in terms of no. of pixels

// 0.X * baseline
static const float MIN_WIDTH_BASECOMPONENT_FRACTION = 0.4;
static const float MIN_HEIGHT_BASECOMPONENT_FRACTION = 0.65;
static const float MIN_WIDTH_OTTUCOMPONENT_FRACTION = 0.3;
static const float MIN_HEIGHT_OTTUCOMPONENT_FRACTION = 0.25;

static const float MAX_WIDTH_BASECOMPONENT_FRACTION = 1.4;
static const float MAX_HEIGHT_BASECOMPONENT_FRACTION = 1.1;
static const float MAX_WIDTH_OTTUCOMPONENT_FRACTION = 0.6;
static const float MIN_WIDTH_BASE_OOTU_MERGED_COMPONENT_FRACTION = 0.55;
static const float MAX_HEIGHT_OTTUCOMPONENT_FRACTION = 0.5;

//"FRAC" CAN BE CHOSEN SO THAT CHAR SUCH AS PU, VU
static const float THRESHOLD_FOR_PUREBASE_BELOWBASELINE_FRACTION = 0.22;
static const float MIN_WIDTH_THRESHOLD_FOR_PU = 0.22;
static const int CC_MIN_AREA = 36; //utmost the size of a valid dot
static const int NOISE_MAX_AREA = 9;
static const int OTTU_MIN_AREA = 50;
static const float OTTU_MIN_RATIO = 0.23;

static const int BASE_GROUP = 1;
static const int OTTU_GROUP = 2;
static const int SPECIAL_SYMBOL_GROUP = 3;
static const float FRAC_PUNC = 0.175;
static const float DANDA_FACTOR = 0.65;
static const float BDThreshold = 0.2; //Bracket & Danda Threshold (set a value between 0.15 - 0.2)
static const float QEThreshold = 0.25; //Question & Exclamation Threshold (set a value between 0.2  - 0.25   )
static const float MID_FACTOR = 0.5;
static const float DASH_FACTOR = 0.9;

static const float NI_BLACK_CONSTANT = -0.2;

// Skew angles are measured w.r.t. horizontal axis
static const int MAX_SKEW_ANGLE = 15; //in degree
//skew angle range would then be -MAX_SKEW_ANGLE to +MAX_SKEW_ANGLE
static const float SKEW_ANGLE_STEP_SIZE = 0.5;
static const float SKEW_ANGLE_TO_IGNORE = 0.0;

static const double MATH_PI = 3.141593;

// Constants used in the algorithm:
// "Block Segmentation and Text Extraction in Mixed Text/Image Documents - Friedrich M. Wahl, Kwan Y. Wong, and Richard G. Casey"
static const int RLSA_C_H = 300, RLSA_C_V = 500, RLSA_C_ALPHA = 30;
static const double RLSA_C1 = 4, RLSA_C2 = 100, RLSA_C3 = 10, RLSA_C4 = 0.5;
static const double RLSA_C11 = 10, RLSA_C12 = 0.2 /*lower than the value specified in paper*/, RLSA_C13 = 8,
		RLSA_C14 = 60, RLSA_C15 = 10 /*higher than the value specified in paper*/, RLSA_C16 = 2, RLSA_C17 =
				0.5, RLSA_C18 = 0.5;
static const double RLSA_C21 = 3, RLSA_C22 = 3, RLSA_C23 = 5;
static const int RLSA_CC_MIN_AREA = 324;
// Note: Above values are for a 240dpi image (as mentioned in paper), while rest of the constants in this file are for a 300dpi image.

static const bool SAVE_INTERIM_IMAGES = false;
static const bool SAVE_SEGMENTED_COMPONENTS = false;
static const bool USE_LINEAR_SVM = true; //to use linear SVM "LibLinear" for faster execution
static const bool SAVE_LOG_FILE = false;

}

#endif /* OCR_GLOBALDEF_H_ */
