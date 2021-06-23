/*++
Copyright (c) 2021 Microsoft Corporation

Module Name:

    polysat equality constraints

Author:

    Nikolaj Bjorner (nbjorner) 2021-03-19
    Jakob Rath 2021-04-6

--*/
#pragma once
#include "math/polysat/constraint.h"


namespace polysat {

    class eq_constraint : public constraint {
        pdd m_poly;
    public:
        eq_constraint(constraint_manager& m, unsigned lvl, csign_t sign, pdd const& p, p_dependency_ref const& dep):
            constraint(m, lvl, sign, dep, ckind_t::eq_t), m_poly(p) {
            m_vars.append(p.free_vars());
        }
        ~eq_constraint() override {}
        pdd const & p() const { return m_poly; }
        std::ostream& display(std::ostream& out) const override;
        constraint_ref resolve(solver& s, pvar v) override;
        bool is_always_false() override;
        bool is_currently_false(solver& s) override;
        bool is_currently_true(solver& s) override;
        void narrow(solver& s) override;
        bool forbidden_interval(solver& s, pvar v, eval_interval& out_interval, constraint_ref& out_neg_cond) override;
        inequality as_inequality() const override;

    private:
        constraint_ref eq_resolve(solver& s, pvar v);
        constraint_ref diseq_resolve(solver& s, pvar v);
    };

}
