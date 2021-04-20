/*++
Copyright (c) 2021 Microsoft Corporation

Module Name:

    polysat solver

Abstract:

    Polynomial solver for modular arithmetic.

Author:

    Nikolaj Bjorner (nbjorner) 2021-03-19
    Jakob Rath 2021-04-6

--*/
#pragma once

#include <limits>
#include "util/statistics.h"
#include "math/polysat/constraint.h"
#include "math/polysat/eq_constraint.h"
#include "math/polysat/var_constraint.h"
#include "math/polysat/ule_constraint.h"
#include "math/polysat/justification.h"
#include "math/polysat/trail.h"

namespace polysat {

    class solver {

        struct stats {
            unsigned m_num_decisions;
            unsigned m_num_propagations;
            unsigned m_num_conflicts;
            void reset() { memset(this, 0, sizeof(*this)); }
            stats() { reset(); }
        };

        friend class eq_constraint;
        friend class var_constraint;

        typedef ptr_vector<constraint> constraints;

        reslimit&                m_lim;
        scoped_ptr_vector<dd::pdd_manager> m_pdd;
        scoped_ptr_vector<dd::fdd> m_bits;
        dd::bdd_manager          m_bdd;
        dep_value_manager        m_value_manager;
        small_object_allocator   m_alloc;
        poly_dep_manager         m_dm;
        constraints              m_conflict;
        constraints              m_stash_just;
        var_queue                m_free_vars;
        stats                    m_stats;

        uint64_t                 m_max_conflicts { std::numeric_limits<uint64_t>::max() };
        uint64_t                 m_max_decisions { std::numeric_limits<uint64_t>::max() };

        // Per constraint state
        scoped_ptr_vector<constraint>   m_constraints;
        scoped_ptr_vector<constraint>   m_redundant;

        // Per variable information
        vector<bdd>              m_viable;   // set of viable values.
        vector<rational>         m_value;    // assigned value
        vector<justification>    m_justification; // justification for variable assignment
        vector<constraints>      m_cjust;    // constraints justifying variable range.
        vector<constraints>      m_watch;    // watch list datastructure into constraints.
        unsigned_vector          m_activity; 
        vector<pdd>              m_vars;
        unsigned_vector          m_size;     // store size of variables.

        // search state that lists assigned variables
        vector<std::pair<pvar, rational>> m_search;

        unsigned                 m_qhead { 0 };
        unsigned                 m_level { 0 };

        svector<trail_instr_t>   m_trail;
        unsigned_vector          m_qhead_trail;
        vector<std::pair<pvar, bdd>> m_viable_trail;
        unsigned_vector          m_cjust_trail;


        unsigned_vector          m_base_levels;  // External clients can push/pop scope. 


        void push_viable(pvar v) {
            m_viable_trail.push_back(std::make_pair(v, m_viable[v]));
        }

        void push_qhead() { 
            m_trail.push_back(trail_instr_t::qhead_i);
            m_qhead_trail.push_back(m_qhead);
        }

        void pop_qhead() {
            m_qhead = m_qhead_trail.back();
            m_qhead_trail.pop_back();
        }

        void push_cjust(pvar v, constraint* c) {
            m_cjust[v].push_back(c);        
            m_trail.push_back(trail_instr_t::just_i);
            m_cjust_trail.push_back(v);
        }

        unsigned size(pvar v) const { return m_size[v]; }
        /**
         * check if value is viable according to m_viable.
         */
        bool is_viable(pvar v, rational const& val);

        /**
         * register that val is non-viable for var.
         */
        void add_non_viable(pvar v, rational const& val);

        /**
         * Register all values that are not contained in vals as non-viable.
         */
        void intersect_viable(pvar v, bdd vals);

        /**
         * Add dependency for variable viable range.
         */
        void add_viable_dep(pvar v, p_dependency* dep);

        
        /**
         * Find a next viable value for variable.
         */
        dd::find_t find_viable(pvar v, rational & val);

        /** Log all viable values for the given variable.
         * (Inefficient, but useful for debugging small instances.)
         */
        void log_viable(pvar v);

        /**
         * undo trail operations for backtracking.
         * Each struct is a subclass of trail and implements undo().
         */

        void del_var();

        dd::pdd_manager& sz2pdd(unsigned sz);
        dd::fdd const& sz2bits(unsigned sz);

        void push_level();
        void pop_levels(unsigned num_levels);
        void pop_constraints(scoped_ptr_vector<constraint>& cs);

        void assign_core(pvar v, rational const& val, justification const& j);

        bool is_assigned(pvar v) const { return !m_justification[v].is_unassigned(); }


        bool should_search();

        void propagate(pvar v);
        void propagate(pvar v, rational const& val, constraint& c);
        void erase_watch(pvar v, constraint& c);
        void erase_watch(constraint& c);
        void add_watch(constraint& c);

        void set_conflict(constraint& c);
        void set_conflict(pvar v);

        unsigned_vector m_marks;
        unsigned        m_clock { 0 };
        void reset_marks();
        bool is_marked(pvar v) const { return m_clock == m_marks[v]; }
        void set_mark(pvar v) { m_marks[v] = m_clock; }

        unsigned                 m_conflict_level { 0 };

        constraint* resolve(pvar v);

        bool can_decide() const { return !m_free_vars.empty(); }
        void decide();
        void decide(pvar v);

        void narrow(pvar v);

        p_dependency* mk_dep(unsigned dep) { return dep == null_dependency ? nullptr : m_dm.mk_leaf(dep); }

        bool is_conflict() const { return !m_conflict.empty(); }
        bool at_base_level() const;
        unsigned base_level() const;

        void resolve_conflict();            
        void backtrack(unsigned i, scoped_ptr<constraint>& lemma);
        void report_unsat();
        void revert_decision(pvar v);
        void learn_lemma(pvar v, constraint* c);
        void backjump(unsigned new_level);
        void add_lemma(constraint* c);

        bool invariant();
        bool invariant(scoped_ptr_vector<constraint> const& cs);

    public:

        /**
         * to share chronology we pass an external trail stack.
         * every update to the solver is going to be retractable
         * by pushing an undo action on the trail stack.
         */
        solver(reslimit& lim);

        ~solver();

        /**
         * End-game satisfiability checker.
         */
        lbool check_sat();

        /**
         * retrieve unsat core dependencies
         */
        void unsat_core(unsigned_vector& deps);
        
        /**
         * Add variable with bit-size. 
         */
        pvar add_var(unsigned sz);

        /**
         * Create polynomial terms
         */
        pdd var(pvar v) { return m_vars[v]; }

        /**
         * Add polynomial constraints
         * Each constraint is tracked by a dependency.
         * assign sets the 'index'th bit of var.
         */
        void add_eq(pdd const& p, unsigned dep = null_dependency);
        void add_diseq(pdd const& p, unsigned dep = null_dependency);
        void add_ule(pdd const& p, pdd const& q, unsigned dep = null_dependency);
        void add_ult(pdd const& p, pdd const& q, unsigned dep = null_dependency);
        void add_sle(pdd const& p, pdd const& q, unsigned dep = null_dependency);
        void add_slt(pdd const& p, pdd const& q, unsigned dep = null_dependency);
        

        /**
         * main state transitions.
         */
        bool can_propagate();
        void propagate();

        /**
         * External context managment.
         * Adds so-called user-scope.
         */
        void push();
        void pop(unsigned num_scopes);
       
        std::ostream& display(std::ostream& out) const;

        void collect_statistics(statistics& st) const;

    };

    inline std::ostream& operator<<(std::ostream& out, solver const& s) { return s.display(out); }

}


