/*!
  \file
  \brief Work supplier that parses an input stream
 */
#ifndef WORK_PARSER_HDR
#define WORK_PARSER_HDR

#include <istream>
#include <memory>

namespace worker {
    namespace work_parser {

        /*!
          \brief Implementation of a work file parser

          A work file is an ASCII-encoded text file that contains bash
          scripts, separated by a marker. Each bash script represents
          an individual work item.
         */
        class Work_parser {
            public:
                /*!
                  \brief Work_parser constructor
                  \param work_stream std::shared_ptr<std::istream> to an
                         input stream that contains the work items.
                  \param separator std::string representing the separator
                          between the individual work items.
                 */
                Work_parser(std::shared_ptr<std::istream> work_stream,
                        const std::string& separator) :
                    ifs_ {work_stream}, sep_ {separator}, nr_items_ {0} {
                        parse_next();
                    };

                /*!
                  \brief Work_parser constructor that uses the default
                         work item separator.
                  \param work_stream std::shared_ptr<std::istream> to an
                         input stream that contains the work items.
                 */
                Work_parser(std::shared_ptr<std::istream> work_stream) :
                    Work_parser(work_stream, DEFAULT_SEP) {};

                /*!
                  \brief checks whether the Work_parser has work items left.
                  \return true if the Work_parser has work items left,
                          false otherwise.
                 */
                bool has_next() const { return next_item_.length() > 0; };

                /*!
                  \brief returns the next work item.
                  \return string representing a work item, the empty
                          string if none is left.
                 */
                std::string next();

                /*!
                  \brief returns the number of work items returned so far.
                  \return number of work items the Work_parser has
                          provided so far.
                 */
                size_t nr_items() const { return nr_items_; };

                /*!
                  \brief returns the separator for the Work_parser.
                  \return string representing the separator used by the
                          Work_parser.
                 */
                std::string separator() const { return sep_; };

            private:
                //! default separator
                static const std::string DEFAULT_SEP;
                //! shared pointer to input stream that contains work items
                std::shared_ptr<std::istream> ifs_;
                //! next work item to be returned by next()
                std::string next_item_;
                //! separator used by Work_parser
                std::string sep_;
                //! number of items returned so far
                size_t nr_items_;
                /*!
                  \brief parses the istream and sets _next_item
                 */
                void parse_next();
        };

    }
}

#endif
