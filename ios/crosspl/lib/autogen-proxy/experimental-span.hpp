#ifndef _STD_ExperimentalSpan_HPP_
#define _STD_ExperimentalSpan_HPP_

#include <cstddef>

namespace std {

template <class ElementType>
class span {
public:
    /*** type define ***/

    /*** static function and variable ***/

    /*** class function and variable ***/
    span(ElementType* d, size_t s)
            : d(d)
            , s(s) {
    }

    ElementType* data() const {
        return d;
    }

    size_t size() const {
        return s;
    }

private:
    /*** type define ***/

    /*** static function and variable ***/

    /*** class function and variable ***/
    ElementType* d;
    size_t s;

}; // class CrossPLUtils
}  // namespace std

#endif // _STD_ExperimentalSpan_HPP_
