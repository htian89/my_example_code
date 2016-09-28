/*
 * utils.h
 *
 *  Created on: 2014年12月17日
 *      Author: root
 */

#ifndef UTILS_H_
#define UTILS_H_

#include <string>
#include <boost/log/trivial.hpp>

#define DEBUG
#ifdef DEBUG
#define LOG_DEBUG(x) BOOST_LOG_TRIVIAL(debug) << "-[" << BOOST_CURRENT_FUNCTION << "] : " << x
#else
#define LOG_DEBUG(x) \
    (void)(0)
#endif

#define LOG_TRACE(x) BOOST_LOG_TRIVIAL(trace) << "-[" << BOOST_CURRENT_FUNCTION << "] : " << x
#define LOG_WARN(x) BOOST_LOG_TRIVIAL(warning) << "-[" << BOOST_CURRENT_FUNCTION << "] : " << x
#define LOG_ERR(x) BOOST_LOG_TRIVIAL(error) << "-[" << BOOST_CURRENT_FUNCTION << "] : " << x

#define FW_MULTILINE_MACRO_BEGIN do {

#ifdef _MS_WINDOWS
    #define FW_MULTILINE_MACRO_END \
        } __pragma(warning(push)) __pragma(warning(disable:4127)) while (0) __pragma(warning(pop))
#else
    #define FW_MULTILINE_MACRO_END } while(0)
#endif

#define CHECK_THROW(expression, ExpectedExceptionType) \
    FW_MULTILINE_MACRO_BEGIN \
        try{ \
            try { \
                expression; \
            } catch(const std::exception & _exc) { \
                LOG_ERR(_exc.what()); \
                throw; \
            } \
        }\
        catch (ExpectedExceptionType &_exc) { \
                LOG_ERR(_exc.what()); } \
    FW_MULTILINE_MACRO_END

#define VERIFY_THROWS(expression, exception) CHECK_THROW(expression, exception)

#define VERIFY_THROW(expression, ...) \
            try { \
                expression; \
            } catch(const std::exception & _exc) { \
                LOG_ERR(_exc.what()); \
                __VA_ARGS__;}

class vclient_exception : public std::exception
{
private:
    std::string _message;
public:
    vclient_exception() {}
    vclient_exception(const char * const message) : _message(message) {}
    vclient_exception(const std::string message) : _message(message) {}

    // Must be narrow string because it derives from std::exception
#if  __cplusplus >= 201103L
    const char* what() const noexcept
#else
    const char* what() throw()
#endif
    {
        return _message.c_str();
    }

#if  __cplusplus >= 201103L
    ~vclient_exception()  noexcept {}
#else
    ~vclient_exception() throw() {}
#endif
};

#endif /* UTILS_H_ */
