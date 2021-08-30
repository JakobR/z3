/*++
Copyright (c) 2021 Microsoft Corporation

Module Name:

    polysat conflict

Author:

    Nikolaj Bjorner (nbjorner) 2021-03-19
    Jakob Rath 2021-04-6

--*/

#include "math/polysat/conflict_core.h"
#include "math/polysat/solver.h"
#include "math/polysat/log.h"
#include "math/polysat/log_helper.h"

namespace polysat {

    std::ostream& conflict_core::display(std::ostream& out) const {
        bool first = true;
        for (auto c : m_constraints) {
            if (first)
                first = false;
            else
                out << "  ;  ";
            out << c;
        }
        if (m_needs_model)
            out << "  ;  + current model";
        return out;
    }

    void conflict_core::set(std::nullptr_t) {
        SASSERT(empty());
        m_constraints.push_back({});
        m_needs_model = false;
    }

    void conflict_core::set(signed_constraint c) {
        LOG("Conflict: " << c);
        SASSERT(empty());
        m_constraints.push_back(std::move(c));
        m_needs_model = true;
    }

    void conflict_core::set(pvar v, vector<signed_constraint> const& cjust_v) {
        LOG("Conflict for v" << v << ": " << cjust_v);
        SASSERT(empty());
        NOT_IMPLEMENTED_YET();
        m_conflict_var = v;
        m_constraints.append(cjust_v);
        if (cjust_v.empty())
            m_constraints.push_back({});
        m_needs_model = true;
    }

}
