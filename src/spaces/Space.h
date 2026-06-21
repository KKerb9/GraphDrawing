#pragma once

#include <cstddef>
#include <memory>
#include <string>

#include "../core/Embedding.h"
#include "../core/Errors.h"

namespace gd {

class SpaceError : public GraphDrawingError {
public:
	using GraphDrawingError::GraphDrawingError;
};

class Space {
public:
        virtual ~Space() = default;

        static ld determinantBareiss(std::vector<std::vector<ld>> a);

        virtual std::string name() const = 0;

        virtual int32_t dimension() const = 0;

        virtual ld dist(const Pt& a, const Pt& b) const = 0;  // метрика (в теории не обязательно пораждена нормой)

        virtual ld norm(const Pt& vec) const = 0;  // норма

        virtual Pt logMap(const Pt& from, const Pt& to) const = 0;  // переводит to в касательное пространство к точке from

        virtual Pt expMap(const Pt& from, const Pt& tangent) const = 0;  // переводит точку tangent из касательного пространства к from обратно в многообразие

        virtual ld tangentNorm(const Pt& at, const Pt& tangent) const = 0;  // риманова норма касательного вектора tangent в точке at

        virtual Pt normalizePoint(const Pt& p, const std::vector<int32_t>& figSize) const = 0;  // нормирует точку p на границы figSize

        virtual ld volume(const std::vector<int32_t>& figSize) const = 0;

        virtual bool areGeodesicSegmentsCrossing(const Pt& a, const Pt& b, const Pt& c, const Pt& d) const = 0;

        virtual bool isValid(const Pt& c) const = 0;

        static bool areEuclideanSegmentsCrossing2D(const Pt& a, const Pt& b, const Pt& c, const Pt& d);
};

using SpacePtr = std::unique_ptr<Space>;

SpacePtr createSpace(const std::string& spaceName, int32_t dim);

} // namespace gd
