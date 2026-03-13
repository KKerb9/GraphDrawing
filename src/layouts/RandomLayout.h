#pragma once

#include "layouts/Layout.h"

namespace gd {

class RandomLayout : public LayoutAlgorithm {
public:
        RandomLayout();

        void computeLayout(Embedding& emb, const Space& space) const override;
};

} // namespace gd
