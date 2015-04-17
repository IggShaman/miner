#include "lp_problem.h"

namespace lp {

void matrix::reset() {
    cols_.clear();
    rows_.clear();
    values_.clear();
    add(0, 0, 0); // dummy row, used due to the way glp_load_matrix works
}


const char* problem::kErrorMessages[] =
    {"OK",
     "EBADB: 0x01 invalid basis",
     "ESING: 0x02 singular matrix",
     "ECOND: 0x03 ill-conditioned matrix",
     "EBOUND: 0x04 invalid bounds",
     "EFAIL: 0x05 solver failed",
     "EOBJLL: 0x06 objective lower limit reached",
     "EOBJUL: 0x07 objective upper limit reached",
     "EITLIM: 0x08 iteration limit exceeded",
     "ETMLIM: 0x09 time limit exceeded",
     "ENOPFS: 0x0A no primal feasible solution",
     "ENODFS: 0x0B no dual feasible solution",
     "EROOT: 0x0C root LP optimum not provided",
     "ESTOP: 0x0D search terminated by application",
     "EMIPGAP: 0x0E relative mip gap tolerance reached",
     "ENOFEAS: 0x0F no primal/dual feasible solution",
     "ENOCVG: 0x10 no convergence",
     "EINSTAB: 0x11 numerical instability",
     "EDATA: 0x12 invalid data",
     "ERANGE: 0x13 result out of range",
     nullptr
    };


problem::problem() {
    glp_ = glp_create_prob();
    glp_init_smcp(&glp_opt_);
    set_verbose(GLP_MSG_OFF);
}


problem::~problem() {
    glp_delete_prob(glp_);
}


void problem::set_matrix ( const matrix& m ) {
    glp_load_matrix(glp_, m.get_columns().size() - 1, m.get_rows().data(), m.get_columns().data(), m.get_values().data());
}


void problem::dump_solution() {
    xlog << std::fixed << "objective: " << get_objective_value() << "\n";
    for ( int i = 1; i <= get_num_columns(); ++i )
	xlog << "  x[" << i << "]=" << get_column_primal(i) << "\n";
}


std::string problem::dump() { //TODO
    std::ostringstream oss;
    oss << (GLP_MIN == glp_get_obj_dir(glp_) ? "min" : "max")
	<< "[";
    
    int cols = glp_get_num_cols(glp_);
    for ( int i = 1; i <= cols; ++i ) {
	oss << glp_get_obj_coef(glp_, i) << '*' << glp_get_col_name(glp_, i);
	if ( i < cols )
	    oss << " + ";
    }
    oss << "]\n";
    
    int rows = glp_get_num_rows(glp_);
    for ( int r = 1; r <= rows; ++r ) {
	std::ostringstream rss;
	
	{
	    int ind[1 + get_num_columns()];
	    double val[1 + get_num_columns()];
	    int nr = glp_get_mat_row(glp_, r, ind, val);
	    for ( int i = 1; i <= nr; ++i ) {
		rss << val[i] << '*' << glp_get_col_name(glp_, ind[i]);
		if ( i < nr )
		    rss << " + ";
	    }
	}
	
	oss << glp_get_row_name(glp_, r) << ':';
	auto t = glp_get_row_type(glp_, r);
	switch(t) {
	case GLP_FR:
	    oss << "unbounded [" << rss.str() << "]\n";
	    break;
	    
	case GLP_LO:
	    oss << glp_get_row_lb(glp_, r) << " <= " << rss.str() << "\n";
	    break;
	    
	case GLP_UP:
	    oss << rss.str() << " <= " << glp_get_row_ub(glp_, r) << "\n";
	    break;
	    
	case GLP_DB:
	    oss << glp_get_row_lb(glp_, r) << " <= " << rss.str() << " <= " << glp_get_row_ub(glp_, r) << "\n";
	    break;
	    
	case GLP_FX:
	    oss << "FIXED:"  << rss.str() << " = " << glp_get_row_ub(glp_, r) << "\n";
	    break;
	    
	default:
	    xlog << "unhandled column type " << t << "\n";
	    DIE();
	};
    }
    
    return oss.str();
}


bool problem::presolve() {
    glp_opt_.presolve = GLP_ON;
    auto rv = solve();
    glp_opt_.presolve = GLP_OFF;
    return rv;
}

} // namespace lp
