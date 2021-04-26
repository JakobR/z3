/*++
Copyright (c) 2021 Microsoft Corporation

Module Name:

    polysat constraints

Author:

    Nikolaj Bjorner (nbjorner) 2021-03-19
    Jakob Rath 2021-04-6

--*/

#include "math/polysat/constraint.h"
#include "math/polysat/solver.h"
#include "math/polysat/log.h"
#include "math/polysat/var_constraint.h"
#include "math/polysat/eq_constraint.h"
#include "math/polysat/ule_constraint.h"

namespace polysat {

    eq_constraint& constraint::to_eq() { 
        return *dynamic_cast<eq_constraint*>(this); 
    }

    eq_constraint const& constraint::to_eq() const { 
        return *dynamic_cast<eq_constraint const*>(this); 
    }

    constraint* constraint::eq(unsigned lvl, bool_var bvar, csign_t sign, pdd const& p, p_dependency_ref& d) {
        return alloc(eq_constraint, lvl, bvar, sign, p, d);
    }

    constraint* constraint::viable(unsigned lvl, bool_var bvar, csign_t sign, pvar v, bdd const& b, p_dependency_ref& d) {
        return alloc(var_constraint, lvl, bvar, sign, v, b, d);
    }

    constraint* constraint::ule(unsigned lvl, bool_var bvar, csign_t sign, pdd const& a, pdd const& b, p_dependency_ref& d) {
        return alloc(ule_constraint, lvl, bvar, sign, a, b, d);
    }

}
