// BigEndianBinaryIArchive.h

#ifndef _UTIL_ARCHIVE_BIG_ENDIAN_BINARY_I_ARCHIVE_H_
#define _UTIL_ARCHIVE_BIG_ENDIAN_BINARY_I_ARCHIVE_H_

#include "util/archive/StreamIArchive.h"
#include "util/serialization/Array.h"

#include <framework/system/BytesOrder.h>

#include <boost/type_traits/is_same.hpp>
#include <boost/utility/enable_if.hpp>

#include <istream>

#ifdef BOOST_BIG_ENDIAN

#  include "util/archive/BinaryIArchive.h"

namespace util
{
    namespace archive
    {

        template <
            typename _Elem = char, 
            typename _Traits = std::char_traits<_Elem>
        >
        class BigEndianBinaryIArchive
            : public BinaryIArchive<_Elem, _Traits>
        {
        public:
            BigEndianBinaryIArchive(
                std::basic_istream<_Elem, _Traits> & is)
                : BinaryIArchive<_Elem, _Traits>(*is.rdbuf())
            {
            }

            BigEndianBinaryIArchive(
                std::basic_streambuf<_Elem, _Traits> & buf)
                : BinaryIArchive<_Elem, _Traits>(buf)
            {
            }
        };

    } // namespace archive
} // namespace util

#else // BOOST_BIG_ENDIAN

namespace util
{
    namespace archive
    {

        /**
        ת���ֽ�˳���ģ�壬����˳��->����˳��

        Ϊ���ñ������Զ�����Ӧ��ת������
        ע�⣺û��ʵ��8���ֽ����ݵ��ֽ�˳��ת��
         */ 
        template <typename T, int size = sizeof(T)>
        struct btoh
        {
            static void apply(T &)
            {
            }
        };

        /// 2���ֽ����ݵ��ֽ�˳��ת��
        template <typename T>
        struct btoh<T, 2>
        {
            static void apply(T & t)
            {
                t = (T)framework::system::BytesOrder::big_endian_to_host_short(t);
            }
        };

        /// 4���ֽ����ݵ��ֽ�˳��ת��
        template <typename T>
        struct btoh<T, 4>
        {
            static void apply(T & t)
            {
                t = (T)framework::system::BytesOrder::big_endian_to_host_long(t);
            }
        };

        /// 8���ֽ����ݵ��ֽ�˳��ת��
        template <typename T>
        struct btoh<T, 8>
        {
            static void apply(T & t)
            {
                t = (T)framework::system::BytesOrder::big_endian_to_host_longlong(t);
            }
        };

        template <
            typename _Elem = char, 
            typename _Traits = std::char_traits<_Elem>
        >
        class BigEndianBinaryIArchive
            : public StreamIArchive<BigEndianBinaryIArchive<_Elem, _Traits>, _Elem, _Traits>
        {
            friend class StreamIArchive<BigEndianBinaryIArchive<_Elem, _Traits>, _Elem, _Traits>;

        public:
            BigEndianBinaryIArchive(
                std::basic_istream<_Elem, _Traits> & is)
                : StreamIArchive<BigEndianBinaryIArchive<_Elem, _Traits>, _Elem, _Traits>(*is.rdbuf())
            {
            }

            BigEndianBinaryIArchive(
                std::basic_streambuf<_Elem, _Traits> & buf)
                : StreamIArchive<BigEndianBinaryIArchive<_Elem, _Traits>, _Elem, _Traits>(buf)
            {
            }

        public:
            /// �����ж�������
            template<class T>
            void load(
                T & t)
            {
                this->load_binary((_Elem *)&t, sizeof(T));
                // ִ���ֽ�˳��ת��
                if (this->state()) return;
                btoh<T>::apply(t);
            }

            /// �ж�ĳ�������Ƿ�����Ż�����Ķ�
            /// ֻ��char�����ܹ�ֱ�Ӷ����飬����Ҫת���ֽ�˳��
            template<class T>
            struct use_array_optimization
                : boost::integral_constant<bool, sizeof(T) == 1>
            {
            };

            /// �����飬ֱ�Ӷ�����������ȡ�����char������Ż�ʵ��
            template<class T>
            void load_array(
                framework::container::Array<T> & a, 
                typename boost::enable_if<use_array_optimization<T> >::type * = NULL)
            {
                this->load_binary((_Elem *)a.address(), a.count());
            }
        };

    } // namespace archive
} // namespace util

#endif // BOOST_BIG_ENDIAN

namespace util
{
    namespace serialization
    {
        template <
            typename _Elem, 
            typename _Traits, 
            typename T
        >
        struct use_array_optimization<util::archive::BigEndianBinaryIArchive<_Elem, _Traits>, T>
            : util::archive::BigEndianBinaryIArchive<_Elem, _Traits>::template use_array_optimization<T>
        {
        };
    }
}

#endif // _UTIL_ARCHIVE_BIG_ENDIAN_BINARY_I_ARCHIVE_H_
