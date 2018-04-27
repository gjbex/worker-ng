#ifndef WORKER_EXCEPTION_HRD
#define WORKER_EXCEPTION_HRD

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

}

#endif
