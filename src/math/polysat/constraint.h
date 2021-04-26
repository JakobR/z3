/*++
Copyright (c) 2021 Microsoft Corporation

Module Name:

    polysat constraints

Author:

    Nikolaj Bjorner (nbjorner) 2021-03-19
    Jakob Rath 2021-04-6

--*/
#pragma once
#include "math/polysat/types.h"

namespace polysat {

    enum ckind_t { eq_t, ule_t, sle_t, bit_t };
    enum csign_t : bool { neg_t = false, pos_t = true };

    class eq_constraint;
    class var_constraint;
    class ule_constraint;

    class constraint {
        friend class var_constraint;
        friend class eq_constraint;
        friend class ule_constraint;
        unsigned         m_level;
        ckind_t          m_kind;
        p_dependency_ref m_dep;
        unsigned_vector  m_vars;
        bool_var         m_bool_var;
        csign_t          m_sign;  ///< sign/polarity
        lbool            m_status = l_undef;  ///< current constraint status, computed from value of m_bool_var and m_sign
        constraint(unsigned lvl, bool_var bvar, csign_t sign, p_dependency_ref& dep, ckind_t k):
            m_level(lvl), m_kind(k), m_dep(dep), m_bool_var(bvar), m_sign(sign) {}
    public:
        static constraint* eq(unsigned lvl, bool_var bvar, csign_t sign, pdd const& p, p_dependency_ref& d);
        static constraint* viable(unsigned lvl, bool_var bvar, csign_t sign, pvar v, bdd const& b, p_dependency_ref& d);
        static constraint* ule(unsigned lvl, bool_var bvar, csign_t sign, pdd const& a, pdd const& b, p_dependency_ref& d);
        virtual ~constraint() {}
        bool is_eq() const { return m_kind == ckind_t::eq_t; }
        bool is_ule() const { return m_kind == ckind_t::ule_t; }
        bool is_sle() const { return m_kind == ckind_t::sle_t; }
        ckind_t kind() const { return m_kind; }
        virtual std::ostream& display(std::ostream& out) const = 0;
        virtual bool propagate(solver& s, pvar v) = 0;
        virtual constraint* resolve(solver& s, pvar v) = 0;
        virtual bool is_always_false() = 0;
        virtual bool is_currently_false(solver& s) = 0;
        virtual bool is_currently_true(solver& s) = 0;
        virtual void narrow(solver& s) = 0;
        eq_constraint& to_eq();
        eq_constraint const& to_eq() const;
        p_dependency* dep() const { return m_dep; }
        unsigned_vector& vars() { return m_vars; }
        unsigned level() const { return m_level; }
        bool_var bvar() const { return m_bool_var; }
        bool sign() const { return m_sign; }
        void assign_eh(bool is_true) { m_status = (is_true ^ !m_sign) ? l_true : l_false; }
        bool is_positive() const { return m_status == l_true; }
        bool is_negative() const { return m_status == l_false; }
    };

    inline std::ostream& operator<<(std::ostream& out, constraint const& c) { return c.display(out); }
        

}
