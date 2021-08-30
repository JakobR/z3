/*++
Copyright (c) 2021 Microsoft Corporation

Module Name:

    Conflict explanation using forbidden intervals as described in
    "Solving bitvectors with MCSAT: explanations from bits and pieces"
    by S. Graham-Lengrand, D. Jovanovic, B. Dutertre.

Author:

    Nikolaj Bjorner (nbjorner) 2021-03-19
    Jakob Rath 2021-04-6

--*/
#pragma once
#include "math/polysat/constraint.h"
#include "math/polysat/solver.h"

namespace polysat {

    class forbidden_intervals {

    public:
        static bool explain(solver& s, vector<signed_constraint> const& conflict, pvar v, clause_ref& out_lemma);

    };
}
