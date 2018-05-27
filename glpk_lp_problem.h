#pragma once

namespace lp {

class matrix {
public:
    using dimension_vector_type = std::vector<int>;
    using value_vector_type = std::vector<double>;
    
    matrix() { reset(); }
    void add(int row, int col, double value);
    const dimension_vector_type& get_rows() const { return rows_; }
    const dimension_vector_type& get_columns() const { return cols_; }
    const value_vector_type& get_values() const { return values_; }
    void reset();
    
private:
    dimension_vector_type rows_;
    dimension_vector_type cols_;
    value_vector_type values_;
};


class problem {
public:
    enum class status {
	kOPT = GLP_OPT,
	kFeasible = GLP_FEAS,
	kInfeasible = GLP_INFEAS,
	kUnbounded = GLP_UNBND,
	kUndefined = GLP_UNDEF,
    };
    
    problem();
    problem(const problem&) = delete;
    problem& operator=(const problem&) = delete;
    ~problem();
    
    void set_name(const char* n) {
        glp_set_prob_name(glp_, n);
    }
    
    // objective
    void set_objective_name(const char* n) { glp_set_obj_name(glp_, n); }
    void set_maximize() { glp_set_obj_dir(glp_, GLP_MAX); }
    void set_minimize() { glp_set_obj_dir(glp_, GLP_MIN); }
    
    // structural (column) variables
    int add_column_variables(int nr) { return glp_add_cols(glp_, nr); }
    void set_column_name(int c, const char* name) { glp_set_col_name(glp_, c, name); }
    void set_column_upper_bounded(int col, double bound) {
        glp_set_col_bnds(glp_, col, GLP_UP, 0, bound);
    }
    void set_column_lower_bounded(int col, double bound) {
        glp_set_col_bnds(glp_, col, GLP_LO, bound, 0);
    }
    void set_column_bounded(int col, double lb, double ub) {
        glp_set_col_bnds(glp_, col, GLP_DB, lb, ub);
    }
    void set_column_fixed_bound(int col, double bound) {
        glp_set_col_bnds(glp_, col, GLP_FX, bound, bound);
    }
    void set_column_unbounded(int col) { glp_set_col_bnds(glp_, col, GLP_FR, 0, 0); }
    void set_objective_coefficient(int col, double v) { glp_set_obj_coef(glp_, col, v); }
    double get_column_primal(int col) { return glp_get_col_prim(glp_, col); }
    double get_column_dual(int col) { return glp_get_col_dual(glp_, col); }
    int get_num_columns() { return glp_get_num_cols(glp_); }
    
    // aux (row) variables
    int add_row_variables(int nr) { return glp_add_rows(glp_, nr); }
    void set_row_name(int row, const char* name) { glp_set_row_name(glp_, row, name); }
    void set_row_upper_bounded(int row, double bound) {
        glp_set_row_bnds(glp_, row, GLP_UP, 0, bound);
    }
    void set_row_lower_bounded(int row, double bound) {
        glp_set_row_bnds(glp_, row, GLP_LO, bound, 0);
    }
    void set_row_bounded(int row, double lb, double ub) {
        glp_set_row_bnds(glp_, row, GLP_DB, lb, ub);
    }
    void set_row_fixed_bound(int row, double bound) {
        glp_set_row_bnds(glp_, row, GLP_FX, bound, bound);
    }
    void set_row_unbounded(int row) { glp_set_row_bnds(glp_, row, GLP_FR, 0, 0); }
    double get_row_primal(int row) { return glp_get_row_prim(glp_, row); }
    double get_row_dual(int row) { return glp_get_row_dual(glp_, row); }
    int get_num_rows() { return glp_get_num_rows(glp_); }
    
    void set_matrix(const matrix&);
    double get_objective_value() { return glp_get_obj_val(glp_); }
    
    bool solve();
    bool presolve();
    static const char* errmsg(int ec) { return kErrorMessages[ec]; }
    static const char* kErrorMessages[];
    void dump_solution();
    status get_status() { return static_cast<status>(glp_get_status(glp_)); }
    const char* last_errmsg() const { return errmsg(last_ec_); }
    
    std::string dump();
    void set_verbose(int v) { glp_opt_.msg_lev = v; }
    
private:
    glp_prob* glp_{};
    glp_smcp glp_opt_;
    int last_ec_{}; // error code for the last call to the solver
};

} // namespace lp

#include "glpk_lp_problem_inl.h"
