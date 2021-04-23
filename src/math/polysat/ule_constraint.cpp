/*++
Copyright (c) 2021 Microsoft Corporation

Module Name:

    polysat unsigned <= constraints

Author:

    Nikolaj Bjorner (nbjorner) 2021-03-19
    Jakob Rath 2021-04-6

--*/

#include "math/polysat/constraint.h"
#include "math/polysat/solver.h"
#include "math/polysat/log.h"

namespace polysat {

    std::ostream& ule_constraint::display(std::ostream& out) const {
        return out << m_lhs << " <=u " << m_rhs;
    }

    bool ule_constraint::propagate(solver& s, pvar v) {
        LOG_H3("Propagate " << s.m_vars[v] << " in " << *this);
        SASSERT(!vars().empty());
        unsigned idx = 0;
        if (vars()[idx] != v)
            idx = 1;
        SASSERT(v == vars()[idx]);
        // find other watch variable.
        for (unsigned i = vars().size(); i-- > 2; ) {
            if (!s.is_assigned(vars()[i])) {
                std::swap(vars()[idx], vars()[i]);
                return true;
            }
        }

        narrow(s);
        return false;
    }

    constraint* ule_constraint::resolve(solver& s, pvar v) {
        return nullptr;
    }

    void ule_constraint::narrow(solver& s) {
        LOG("Assignment: " << s.m_search);
        auto p = lhs().subst_val(s.m_search);
        LOG("Substituted LHS: " << lhs() << " := " << p);
        auto q = rhs().subst_val(s.m_search);
        LOG("Substituted RHS: " << rhs() << " := " << q);

        if (is_always_false(p, q)) {
            s.set_conflict(*this);
            return;
        }
        if (p.is_val() && q.is_val()) {
            SASSERT(p.val() <= q.val());
            return;
        }

        pvar v = null_var;
        rational a, b, c, d;
        if (p.is_unilinear() && q.is_unilinear() && p.var() == q.var()) {
            // a*x + b <=u c*x + d
            v = p.var();
            a = p.hi().val();
            b = p.lo().val();
            c = q.hi().val();
            d = q.lo().val();
        }
        else if (p.is_unilinear() && q.is_val()) {
            // a*x + b <=u d
            v = p.var();
            a = p.hi().val();
            b = p.lo().val();
            c = rational::zero();
            d = q.val();
        }
        else if (p.is_val() && q.is_unilinear()) {
            // b <=u c*x + d
            v = q.var();
            a = rational::zero();
            b = p.val();
            c = q.hi().val();
            d = q.lo().val();
        }
        if (v != null_var) {
            bddv const& x = s.var2bits(v).var();
            bdd xs = (a * x + b <= c * x + d);
            s.push_cjust(v, this);
            s.intersect_viable(v, xs);

            rational val;
            if (s.find_viable(v, val) == dd::find_t::singleton) {
                s.propagate(v, val, *this);
            }

            return;
        }

        // TODO: other cheap constraints possible?
    }

    bool ule_constraint::is_always_false(pdd const& lhs, pdd const& rhs) {
        // TODO: other conditions (e.g. when forbidden interval would be full)
        return lhs.is_val() && rhs.is_val() && !(lhs.val() <= rhs.val());
    }

    bool ule_constraint::is_always_false() {
        return is_always_false(lhs(), rhs());
    }

    bool ule_constraint::is_currently_false(solver& s) {
        auto p = lhs().subst_val(s.m_search);
        auto q = rhs().subst_val(s.m_search);
        return is_always_false(p, q);
    }

}
