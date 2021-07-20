#ifndef WORKER_EXCEPTION_HDR
#define WORKER_EXCEPTION_HDR

#include <exception>

namespace worker {

    /*!
      \brief Exception to be used as base class for all worker exceptions
     */
    class Worker_exception : public std::exception {
        /*!
          \brief Exception constructor.
          \param message std:string that specifies the specific
                 inforation about the condition that triggered
                 the exception.
         */
        public:
            Worker_exception(const char* msg) : _msg {msg} {};
            virtual char const* what() const throw() { return _msg; };
        private:
            const char* _msg;
    };

    enum class Error : int {
        cli_option = 1,
        file = 2,
        socket = 3,
        unexpected = 4
    };

    void exit(Error err);

}

#endif
