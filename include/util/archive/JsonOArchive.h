// JsonOArchive.h

#ifndef _UTIL_ARCHIVE_JSON_O_ARCHIVE_H_
#define _UTIL_ARCHIVE_JSON_O_ARCHIVE_H_

#include "util/archive/StreamOArchive.h"

#include <boost/type_traits/is_same.hpp>

#include <iostream>
#include <string>
#include <ostream>

namespace util
{
    namespace archive
    {

        class JsonOArchive
            : public StreamOArchive<JsonOArchive>
        {
            friend class StreamOArchive<JsonOArchive>;

        public:
            JsonOArchive(
                std::ostream & os)
                : StreamOArchive<JsonOArchive>(*os.rdbuf())
                , is_save_(true)
                , sub_just_end_(true)
                , os_(os)
            {
                splits.push_back(0);
            }

            /// ������д����������ͱ���
            template <typename T>
            void save(
                T const & t)
            {
                os_ << t;
            }

            void save(
                unsigned char const & t)
            {
                unsigned short t1 = t;
                os_ << t1;
            }

            /// ������д����׼���ַ���
            void save(
                std::string const & t)
            {
                os_ << "'" << t << "'";
            }

            using StreamOArchive<JsonOArchive>::save;

            /// ������д�����飨�Ż���
            void save_array(
                framework::container::Array<char> const & a)
            {
                save_binary((char const *)a.address(), sizeof(char) * a.count());
            }

            /// �ж�ĳ�������Ƿ�����Ż���������л�
            /// ֻ�л��������ܹ�ֱ�����л�����
            template<class T>
            struct use_array_optimization
                : boost::is_same<T, char>
            {
            };

            void save_start(
                std::string const & name)
            {
                os_ << ident_;
                if (splits.back()) {
                    os_ << splits.back();
                } else {
                    splits.back() = ',';
                }
                os_ << name << ":";
                sub_just_end_ = false;
            }

            void save_end(
                std::string const & name)
            {
                if (!sub_just_end_)
                    os_ << std::endl;
                sub_just_end_ = true;
            }

            void sub_start()
            {
                os_ << std::endl;
                splits.push_back(0);
                os_ << ident_ << "{" << std::endl;
                sub_just_end_ = false;
                ident_ += "  ";
            }

            void sub_end()
            {
                os_ << ident_ << "}" << std::endl;
                if (!sub_just_end_)
                    os_ << std::endl;
                splits.pop_back();
                sub_just_end_ = true;
                ident_.erase(ident_.size() - 2);
            }

        private:
            std::vector<char> splits;
            bool is_save_;
            std::string ident_;
            bool sub_just_end_;
            std::ostream & os_;
        };

    } // namespace archive
} // namespace util

SERIALIZATION_USE_ARRAY_OPTIMIZATION(util::archive::JsonOArchive);

#endif // _UTIL_ARCHIVE_JSON_O_ARCHIVE_H_
