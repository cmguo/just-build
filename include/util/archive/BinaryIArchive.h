// BinaryIArchive.h

#ifndef _UTIL_ARCHIVE_BINARY_I_ARCHIVE_H_
#define _UTIL_ARCHIVE_BINARY_I_ARCHIVE_H_

#include "util/archive/StreamIArchive.h"
#include "util/serialization/Array.h"

#include <boost/type_traits/is_fundamental.hpp>
#include <boost/utility/enable_if.hpp>

#include <istream>
#include <string>

namespace util
{
    namespace archive
    {

        template <
            typename _Elem = char, 
            typename _Traits = std::char_traits<_Elem>
        >
        class BinaryIArchive
            : public StreamIArchive<BinaryIArchive<_Elem, _Traits>, _Elem, _Traits>
        {
            friend class StreamIArchive<BinaryIArchive<_Elem, _Traits>, _Elem, _Traits>;
        public:
            BinaryIArchive(
                std::basic_istream<_Elem, _Traits> & is)
                : StreamIArchive<BinaryIArchive<_Elem, _Traits>, _Elem, _Traits>(*is.rdbuf())
            {
            }

            BinaryIArchive(
                std::basic_streambuf<_Elem, _Traits> & buf)
                : StreamIArchive<BinaryIArchive<_Elem, _Traits>, _Elem, _Traits>(buf)
            {
            }

        public:
            /// �����ж�������
            template<class T>
            void load(
                T & t)
            {
                load_binary((_Elem *)&t, sizeof(T));
            }

            using StreamIArchive<BinaryIArchive<_Elem, _Traits>, _Elem, _Traits>::load;

            /// �ж�ĳ�������Ƿ�����Ż���������л�
            /// ֻ�л��������ܹ�ֱ�����л�����
            template<class T>
            struct use_array_optimization
                : boost::is_fundamental<T>
            {
            };

            /// �����ж������飨�Ż���
            template<class T>
            void load_array(
                framework::container::Array<T> & a, 
                typename boost::enable_if<use_array_optimization<T> >::type * = NULL)
            {
                load_binary((_Elem *)a.address(), sizeof(T) * a.count());
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
        struct use_array_optimization<util::archive::BinaryIArchive<_Elem, _Traits>, T>
            : util::archive::BinaryIArchive<_Elem, _Traits>::template use_array_optimization<T>
        {
        };
    }
}

#endif // _UTIL_ARCHIVE_BINARY_I_ARCHIVE_H_
