#ifndef WORK_PARSER_HDR
#define WORK_PARSER_HDR

#include <istream>
#include <memory>

class Work_parser {
    public:
        Work_parser(std::shared_ptr<std::istream> ifs,
                    const std::string& sep) :
            _ifs {ifs}, _sep {sep}, _nr_items {0} {
                parse_next();
            };
        Work_parser(std::shared_ptr<std::istream> ifs) :
            Work_parser(ifs, DEFAULT_SEP) {};
        bool has_next() const { return _next_item.length() > 0; };
        std::string next();
        size_t nr_items() const { return _nr_items; };
        std::string separator() const { return _sep; };
    private:
        static const std::string DEFAULT_SEP;
        std::shared_ptr<std::istream> _ifs;
        std::string _next_item;
        std::string _sep;
        size_t _nr_items;
        void parse_next();
};

#endif
