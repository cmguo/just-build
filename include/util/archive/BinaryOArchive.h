// BinaryOArchive.h

#ifndef _UTIL_ARCHIVE_BINARY_O_ARCHIVE_H_
#define _UTIL_ARCHIVE_BINARY_O_ARCHIVE_H_

#include "util/archive/StreamOArchive.h"
#include "util/serialization/Array.h"

#include <boost/type_traits/is_fundamental.hpp>
#include <boost/utility/enable_if.hpp>

#include <ostream>
#include <string>

namespace util
{
    namespace archive
    {

        template <
            typename _Elem = char, 
            typename _Traits = std::char_traits<_Elem>
        >
        class BinaryOArchive
            : public StreamOArchive<BinaryOArchive<_Elem, _Traits>, _Elem, _Traits>
        {
            friend class StreamOArchive<BinaryOArchive<_Elem, _Traits>, _Elem, _Traits>;
            friend struct SaveAccess;

        public:
            BinaryOArchive(
                std::basic_ostream<_Elem, _Traits> & os)
                : StreamOArchive<BinaryOArchive<_Elem, _Traits>, _Elem, _Traits>(*os.rdbuf())
            {
            }

            BinaryOArchive(
                std::basic_streambuf<_Elem, _Traits> & buf)
                : StreamOArchive<BinaryOArchive<_Elem, _Traits>, _Elem, _Traits>(buf)
            {
            }

        public:
            /// ������д����������ͱ���
            template <typename T>
            void save(
                T const & t)
            {
                save_binary((_Elem const *)&t, sizeof(T));
            }

            using StreamOArchive<BinaryOArchive<_Elem, _Traits>, _Elem, _Traits>::save;

            /// �ж�ĳ�������Ƿ�����Ż���������л�
            /// ֻ�л��������ܹ�ֱ�����л�����
            template<class T>
            struct use_array_optimization
                : boost::is_fundamental<T>
            {
            };

            /// ������д�����飨�Ż���
            template<class T>
            void save_array(
                framework::container::Array<T> const & a, 
                typename boost::enable_if<use_array_optimization<T> >::type * = NULL)
            {
                save_binary((_Elem const *)a.address(), sizeof(T) * a.count());
            }
        };

    } // namespace archive
} // namespace util

namespace util
{
    namespace serialization
    {
        template <
            typename _Elem, 
            typename _Traits, 
            typename T
        >
        struct use_array_optimization<util::archive::BinaryOArchive<_Elem, _Traits>, T>
            : util::archive::BinaryOArchive<_Elem, _Traits>::template use_array_optimization<T>
        {
        };
    }
}

#endif // _UTIL_ARCHIVE_BINARY_O_ARCHIVE_H_
