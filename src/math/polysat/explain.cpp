/*++
Copyright (c) 2021 Microsoft Corporation

Module Name:

    Conflict explanation / resolution

Author:

    Nikolaj Bjorner (nbjorner) 2021-03-19
    Jakob Rath 2021-04-6

--*/
#include "math/polysat/explain.h"
#include "math/polysat/log.h"
#include "math/polysat/solver.h"

namespace polysat {

    constraint_manager& explainer::cm() { return s().m_constraints; }

    signed_constraint ex_polynomial_superposition::resolve1(pvar v, signed_constraint c1, signed_constraint c2) {
            // c1 is true, c2 is false
            SASSERT(c1.is_currently_true(s()));
            SASSERT(c2.is_currently_false(s()));
            LOG("c1: " << c1);
            LOG("c2: " << c2);
            pdd a = c1->to_eq().p();
            pdd b = c2->to_eq().p();
            pdd r = a;
            if (!a.resolve(v, b, r) && !b.resolve(v, a, r))
                return {};
            unsigned const lvl = std::max(c1->level(), c2->level());
            signed_constraint c = cm().eq(lvl, r);
            LOG("resolved: " << c << "        currently false? " << c.is_currently_false(s()));
            if (!c.is_currently_false(s()))
                return {};
            return c;
    }

    bool ex_polynomial_superposition::is_positive_equality_over(pvar v, signed_constraint const& c) {
        return c.is_positive() && c->is_eq() && c->contains_var(v);
    }

    // TODO(later): check superposition into disequality again (see notes)
    // true = done, false = abort, undef = continue
    lbool ex_polynomial_superposition::try_explain1(pvar v, conflict_core& core) {
        for (auto it1 = core.begin(); it1 != core.end(); ++it1) {
            signed_constraint c1 = *it1;
            if (!is_positive_equality_over(v, c1))
                continue;
            if (!c1.is_currently_true(s()))
                continue;

            for (auto it2 = core.begin(); it2 != core.end(); ++it2) {
                signed_constraint c2 = *it2;
                if (!is_positive_equality_over(v, c2))
                    continue;
                if (!c2.is_currently_false(s()))
                    continue;

                signed_constraint c = resolve1(v, c1, c2);
                if (!c)
                    continue;
                vector<signed_constraint> premises;
                premises.push_back(c1);
                premises.push_back(c2);
                core.replace(c2, c, std::move(premises));
                if (!c->contains_var(v)) {
                    core.keep(c);
                    core.remove_var(v);
                    return l_true;
                } else
                    return l_undef;
            }
        }
        return l_false;
    }

    bool ex_polynomial_superposition::try_explain(pvar v, conflict_core& core) {
        LOG_H3("Trying polynomial superposition...");
        lbool result = l_undef;
        while (result == l_undef)
            result = try_explain1(v, core);
        LOG("success? " << result);
        return result;
    }

}
