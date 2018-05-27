namespace lp {

inline void matrix::add(int row, int col, double value) {
    rows_.push_back(row);
    cols_.push_back(col);
    values_.push_back(value);
}


inline bool problem::solve() {
    last_ec_ = glp_simplex(glp_, &glp_opt_);
    //I_ASSERT(!last_ec_, EX_LOG("ERROR: " << lp::problem::errmsg(last_ec_)));
    return get_status() == status::kOPT;
}

} // namespace lp
