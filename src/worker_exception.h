#ifndef WORKER_EXCEPTION_HRD
#define WORKER_EXCEPTION_HRD

namespace worker {

    class Worker_exception : public std::exception {
        public:
            Worker_exception(const char* msg) : _msg {msg} {};
            virtual char const* what() const throw() { return _msg; };
        private:
            const char* _msg;
    };

}

#endif
