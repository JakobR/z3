/*++
Copyright (c) 2021 Microsoft Corporation

Module Name:

    polysat conflict

Author:

    Nikolaj Bjorner (nbjorner) 2021-03-19
    Jakob Rath 2021-04-6

--*/
#pragma once
#include "math/polysat/constraint.h"

namespace polysat {

    /// Represents a conflict as core (~negation of clause).
    ///
    /// TODO: can probably move some clause_builder functionality into this class.
    class conflict_core {
        // constraint_ref_vector m_constraints;
        vector<constraint_literal> m_constraints;
        bool m_needs_model = true;  ///< True iff the conflict depends on the current variable assignment. (If so, additional constraints must be added to the final learned clause.)

    public:
        vector<constraint_literal> const& constraints() const {
            return m_constraints;
        }

        bool empty() const {
            return m_constraints.empty();
        }

        void reset() {
            m_constraints.reset();
            m_needs_model = true;
        }

        // for bailing out with a conflict at the base level
        void set(std::nullptr_t) {
            SASSERT(empty());
            m_constraints.push_back({});
            m_needs_model = false;
        }

        // void set(
        
        // TODO: set core from conflicting units
        // TODO: set clause

        // TODO: use iterator instead (this method is needed for marking so duplicates don't necessarily have to be skipped)
        unsigned_vector vars(constraint_manager const& cm) const {
            unsigned_vector vars;
            for (auto const& c : m_constraints)
                vars.append(c->vars());
            return vars;
        }

        std::ostream& display(std::ostream& out) const;
    };

    inline std::ostream& operator<<(std::ostream& out, conflict_core const& c) { return c.display(out); }

}
