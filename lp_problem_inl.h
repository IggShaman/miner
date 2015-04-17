namespace lp {

inline void matrix::add ( int row, int col, double value ) {
    rows_.push_back(row);
    cols_.push_back(col);
    values_.push_back(value);
}


/*
inline void constraint_vector::add ( int x, double v ) {
    x_.push_back(x);
    values_.push_back(v);
}


inline void constraint_vector::reset ( int reserve ) {
    x_.clear();
    values_.clear();
    
    if ( reserve > 0 ) {
	x_.reserve(reserve);
	values_.reserve(reserve);
    }
    
    add(0, 0); // dummy row; TODO: what for?
}
*/

/*
inline int problem::set_column_fixed_value ( int c, int r, double v ) { //TEST
    if ( !r ) {
	r = add_row_variables(1);
	
	int xx[2];
	xx[1] = c;
	double vv[2];
	vv[1] = 1;
	glp_set_mat_row(glp_, r, 1, xx, vv);
    }
    
    set_row_fixed_bound(r, v);
    return r;
}
*/


inline bool problem::solve() {
    last_ec_ = glp_simplex(glp_, &glp_opt_);
    I_ASSERT(!last_ec_, EX_LOG("ERROR: " << lp::problem::errmsg(last_ec_)));
    return get_status() == status::kOPT;
}


/*XX
inline void problem::delete_row ( int r ) {
    int rr[2];
    rr[1] = r;
    glp_del_rows(glp_, 1, rr);
    
    //glp_del_rows(glp_, 1, int[]({0, r}));
    //glp_set_row_stat(glp_, r, GLP_BS);
    //set_row_unbounded(r);
}
*/

} // namespace lp
