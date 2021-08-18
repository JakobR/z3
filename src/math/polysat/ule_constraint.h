/*++
Copyright (c) 2021 Microsoft Corporation

Module Name:

    polysat unsigned <= constraint

Author:

    Nikolaj Bjorner (nbjorner) 2021-03-19
    Jakob Rath 2021-04-6

--*/
#pragma once
#include "math/polysat/constraint.h"


namespace polysat {

    class ule_constraint : public constraint {
        pdd m_lhs;
        pdd m_rhs;
    public:
        ule_constraint(constraint_manager& m, unsigned lvl, pdd const& l, pdd const& r):
            constraint(m, lvl, ckind_t::ule_t), m_lhs(l), m_rhs(r) {
            m_vars.append(l.free_vars());
            for (auto v : r.free_vars())
                if (!m_vars.contains(v))
                    m_vars.push_back(v);
        }
        ~ule_constraint() override {}
        pdd const& lhs() const { return m_lhs; }
        pdd const& rhs() const { return m_rhs; }
        std::ostream& display(std::ostream& out, lbool status) const override;
        bool is_always_false(bool is_positive, pdd const& lhs, pdd const& rhs);
        bool is_always_false(bool is_positive) override;
        bool is_currently_false(solver& s, bool is_positive) override;
        bool is_currently_true(solver& s, bool is_positive) override;
        void narrow(solver& s, bool is_positive) override;
        bool forbidden_interval(solver& s, bool is_positive, pvar v, eval_interval& out_interval, constraint_literal_ref& out_neg_cond) override;
        inequality as_inequality(bool is_positive) const override;
        unsigned hash() const override;
        bool operator==(constraint const& other) const override;
    };

}
