#ifndef __I_UTIL_H_
#define __I_UTIL_H_

namespace i {

inline const char* strip_path ( const char* s ) {
    auto rv = rindex(s, '/');
    return !rv ? s : rv + 1;
}


class log_stream {
public:
    explicit log_stream ( std::ostream* os, bool stderr_copy ) : os_(os), stderr_copy_(stderr_copy) {}
    
    ~log_stream() {
	std::string s = oss_.str();
	if ( !s.length() )
	    return;
	
	if ( '\n' != *s.rbegin() )
	    s += '\n';
	
	if ( stderr_copy_ )
	    std::cerr << s;
	*os_ << s << std::flush;
    }
    
    std::ostringstream& operator()() { return oss_; }
    
private:
    std::ostream* os_;
    bool stderr_copy_;
    std::ostringstream oss_;
};


class exception : public std::runtime_error {
public:
    exception ( const std::string& message ) : runtime_error(message) {}
};

} // namespace i

#define errlog i::log_stream(&std::cerr, false)() << __PRETTY_FUNCTION__ << ' ' << i::strip_path(__FILE__) << '(' <<  __LINE__ << "): ERROR: "
#define xlog i::log_stream(&std::cerr, false)() << __PRETTY_FUNCTION__ << ' ' << i::strip_path(__FILE__) << '(' <<  __LINE__ << "): "

#define EX_LOG(V) static_cast<std::ostringstream&>(std::ostringstream() << __PRETTY_FUNCTION__ << ' ' << i::strip_path(__FILE__) << '(' <<  __LINE__ << "): " << V).str()
#define I_ASSERT(OP, LOG) if ( !(OP) ) { auto s = (LOG); std::cerr << s; throw i::exception(s); }
#define I_FAIL(LOG) { auto s = EX_LOG(LOG); std::cerr << s; throw i::exception(s); }

inline void DIE() {
    *((char*)nullptr) = 0;
    exit(-1); // just in case
}

#endif // __I_UTIL_H_
