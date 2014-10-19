#ifndef EVAL_MEASURE_LOADER_H__
#define EVAL_MEASURE_LOADER_H__

#include <string>

namespace travatar {

class EvalMeasure;

class EvalMeasureLoader {

    EvalMeasureLoader(); // = delete;
    EvalMeasureLoader(const EvalMeasureLoader &); // = delete;
    EvalMeasureLoader & operator=(const EvalMeasureLoader &); // = delete;

public:
    // Create measure from string
    static EvalMeasure * CreateMeasureFromString(const std::string & str);

}; // class EvalMeasureLoader

} // namespace travatar

#endif // EVAL_MEASURE_LOADER_H__

